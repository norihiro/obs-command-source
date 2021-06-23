#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/platform.h>
#include <math.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

struct command_source {
	char *cmd_show;
	char *cmd_hide;
	char *cmd_activate;
	char *cmd_deactivate;
	char *cmd_previewed;
	char *cmd_unpreviewed;

	bool is_shown;
	bool is_preview, was_preview;

	obs_source_t *self;

#ifndef _WIN32
	DARRAY(pid_t) running_pids;
#endif // not _WIN32
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-command-source", "en-US")

static void fork_exec(const char *cmd, struct command_source *s)
{
#ifdef _WIN32
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	char *p = bstrdup(cmd);
	CreateProcess(NULL, p, NULL, NULL, FALSE, BELOW_NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	bfree(p);
#else
	pid_t pid = fork();
	if(!pid) {
		execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
	}
	else if(pid!=-1) {
		da_push_back(s->running_pids, &pid);
	}
#endif
}

static void check_notify_preview(struct command_source *s);
static void on_preview_scene_changed(enum obs_frontend_event event, void *param);

static void cmdsrc_show(void *data)
{
	struct command_source *s = data;
	if(s->cmd_show) {
		fork_exec(s->cmd_show, s);
	}

	check_notify_preview(s);
	if (!s->is_shown)
		obs_frontend_add_event_callback(on_preview_scene_changed, data);
	s->is_shown = true;
}

static void cmdsrc_hide(void *data)
{
	struct command_source *s = data;
	if(s->cmd_hide) {
		fork_exec(s->cmd_hide, s);
	}

	check_notify_preview(s);
	if (s->is_shown)
		obs_frontend_remove_event_callback(on_preview_scene_changed, data);
	s->is_shown = false;
}

static void cmdsrc_activate(void *data)
{
	struct command_source *s = data;
	if(s->cmd_activate) {
		fork_exec(s->cmd_activate, s);
	}
}

static void cmdsrc_deactivate(void *data)
{
	struct command_source *s = data;
	if(s->cmd_deactivate) {
		fork_exec(s->cmd_deactivate, s);
	}
}

static inline void cmdsrc_previewed(struct command_source *s)
{
	if (s->cmd_previewed)
		fork_exec(s->cmd_previewed, s);
}

static inline void cmdsrc_unpreviewed(struct command_source *s)
{
	if (s->cmd_unpreviewed)
		fork_exec(s->cmd_unpreviewed, s);
}

static void preview_callback(obs_source_t *parent, obs_source_t *child, void *param)
{
	UNUSED_PARAMETER(parent);
	struct command_source *s = param;
	if (child == s->self)
		s->is_preview = true;
}

static void check_notify_preview(struct command_source *s)
{
	s->is_preview = false;
	obs_source_t* preview_soure = obs_frontend_get_current_preview_scene();
	if (preview_soure) {
		obs_source_enum_active_sources(preview_soure, preview_callback, s);
		obs_source_release(preview_soure);
	}

	if (s->is_preview && !s->was_preview)
		cmdsrc_previewed(s);
	else if (!s->is_preview && s->was_preview)
		cmdsrc_unpreviewed(s);
	s->was_preview = s->is_preview;
}

static void on_preview_scene_changed(enum obs_frontend_event event, void *param)
{
	struct command_source *s = param;
	switch (event) {
		case OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED:
		case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
		case OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED:
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			check_notify_preview(s);
			break;
	}
}

static const char *command_source_name(void *unused)
{
	UNUSED_PARAMETER(unused);

	return obs_module_text("execute-command");
}

static void command_source_get_defaults(obs_data_t *settings)
{
#ifndef _WIN32
	obs_data_set_default_string(settings, "cmd_show", "/bin/echo going to preview");
	obs_data_set_default_string(settings, "cmd_hide", "/bin/echo hiding from preview");
	obs_data_set_default_string(settings, "cmd_activate", "/bin/echo going to program");
	obs_data_set_default_string(settings, "cmd_deactivate", "/bin/echo retiring from program");
#endif // not _WIN32
}

static obs_properties_t *command_source_get_properties(void *unused)
{
	UNUSED_PARAMETER(unused);
	obs_properties_t *props;

	props = obs_properties_create();

	obs_properties_add_text(props, "cmd_show", obs_module_text("Show"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_hide", obs_module_text("Hide"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_activate", obs_module_text("Active"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_deactivate", obs_module_text("Deactive"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_previewed", obs_module_text("Show in preview"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_unpreviewed", obs_module_text("Hide from preview"), OBS_TEXT_DEFAULT);

	return props;
}

static void command_source_destroy(void *data)
{
	struct command_source *s = data;

	if (s->is_shown)
		obs_frontend_remove_event_callback(on_preview_scene_changed, data);

	if (s->cmd_show) bfree(s->cmd_show);
	if (s->cmd_hide) bfree(s->cmd_hide);
	if (s->cmd_activate) bfree(s->cmd_activate);
	if (s->cmd_deactivate) bfree(s->cmd_deactivate);
	if (s->cmd_previewed) bfree(s->cmd_previewed);
	if (s->cmd_unpreviewed) bfree(s->cmd_unpreviewed);
#ifndef _WIN32
	da_free(s->running_pids);
#endif // not _WIN32

	bfree(s);
}

static inline char * bstrdup_nonzero(const char *s)
{
	if (!*s)
		return NULL;
	return bstrdup(s);
}

static void command_source_update(void *data, obs_data_t *settings)
{
	struct command_source *s = data;

	if (s->cmd_show) bfree(s->cmd_show);
	if (s->cmd_hide) bfree(s->cmd_hide);
	if (s->cmd_activate) bfree(s->cmd_activate);
	if (s->cmd_deactivate) bfree(s->cmd_deactivate);
	if (s->cmd_previewed) bfree(s->cmd_previewed);
	if (s->cmd_unpreviewed) bfree(s->cmd_unpreviewed);
	s->cmd_show = bstrdup_nonzero(obs_data_get_string(settings, "cmd_show"));
	s->cmd_hide = bstrdup_nonzero(obs_data_get_string(settings, "cmd_hide"));
	s->cmd_activate = bstrdup_nonzero(obs_data_get_string(settings, "cmd_activate"));
	s->cmd_deactivate = bstrdup_nonzero(obs_data_get_string(settings, "cmd_deactivate"));
	s->cmd_previewed = bstrdup_nonzero(obs_data_get_string(settings, "cmd_previewed"));
	s->cmd_unpreviewed = bstrdup_nonzero(obs_data_get_string(settings, "cmd_unpreviewed"));
}

static void *command_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct command_source *s = bzalloc(sizeof(struct command_source));
	s->self = source;
#ifndef _WIN32
	da_init(s->running_pids);
#endif // not _WIN32

	command_source_update(s, settings);

	return s;
}

#ifndef _WIN32
static void cmdsrc_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);
	struct command_source *s = data;

	for(size_t i = 0; i < s->running_pids.num; i++) {
		pid_t pid = waitpid(s->running_pids.array[i], NULL, WNOHANG);
		if(pid!=0) {
			da_erase(s->running_pids, i);
			i--;
		}
	}
}
#endif // not _WIN32

static struct obs_source_info command_source_info = {
	.id = "command_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.get_name = command_source_name,
	.create = command_source_create,
	.destroy = command_source_destroy,
	.update = command_source_update,
	.show = cmdsrc_show,
	.hide = cmdsrc_hide,
	.activate = cmdsrc_activate,
	.deactivate = cmdsrc_deactivate,
	.get_defaults = command_source_get_defaults,
	.get_properties = command_source_get_properties,
#ifndef _WIN32
	.video_tick = cmdsrc_tick,
#endif // not _WIN32
};

bool obs_module_load()
{
	obs_register_source(&command_source_info);

	return true;
}

void obs_module_unload(void)
{
}
