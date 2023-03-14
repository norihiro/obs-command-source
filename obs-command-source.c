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
#include <signal.h>
#include <stdlib.h>
#endif

struct command_source
{
	char *cmd_show;
	char *cmd_hide;
	char *cmd_activate;
	char *cmd_deactivate;
	char *cmd_previewed;
	char *cmd_unpreviewed;
#ifndef _WIN32
	int sig_show;
	int sig_activate;
	int sig_preview;
#endif

	bool is_shown;
	bool is_preview, was_preview;

	obs_source_t *self;

#ifndef _WIN32
	pid_t pid_show;
	pid_t pid_activate;
	pid_t pid_preview;
	DARRAY(pid_t) running_pids;
#else
// For Windows, just send dummy information to reuse the code for now.
// TODO: Implement to hold hProcess and kill it.
#define pid_show self
#define pid_activate self
#define pid_preview self
#endif // not _WIN32
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-command-source", "en-US")

#ifndef _WIN32
static void setenv_if(const char *name, const char *val)
{
	if (val)
		setenv(name, val, 1);
}

static void setenv_int(const char *name, int val)
{
	char s[16];
	snprintf(s, sizeof(s), "%d", val);
	s[sizeof(s) - 1] = 0;
	setenv(name, s, 1);
}
#endif

static void fork_exec(const char *cmd, struct command_source *s,
#ifndef _WIN32
		      pid_t *pid_sig
#else
		      void *unused
#endif
)
{
#ifdef _WIN32
	UNUSED_PARAMETER(unused);
	PROCESS_INFORMATION pi = {0};
	STARTUPINFO si = {sizeof(STARTUPINFO)};
	char *p = bstrdup(cmd);
	CreateProcess(NULL, p, NULL, NULL, FALSE, BELOW_NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	bfree(p);
#else
	obs_source_t *current_src = obs_frontend_get_current_scene();
	obs_source_t *preview_src = NULL;
	if (obs_frontend_preview_program_mode_active())
		preview_src = obs_frontend_get_current_preview_scene();

	pid_t pid = fork();
	if (!pid) {
		setenv_if("OBS_CURRENT_SCENE", obs_source_get_name(current_src));
		setenv_if("OBS_PREVIEW_SCENE", obs_source_get_name(preview_src));
		setenv_if("OBS_SOURCE_NAME", obs_source_get_name(s->self));
		setenv_int("OBS_TRANSITION_DURATION", obs_frontend_get_transition_duration());

		execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
		_exit(1);
	}
	else if (pid != -1) {
		if (pid_sig) {
			if (*pid_sig)
				da_push_back(s->running_pids, pid_sig);
			*pid_sig = pid;
		}
		else
			da_push_back(s->running_pids, &pid);
	}

	obs_source_release(current_src);
	obs_source_release(preview_src);
#endif
}

static void check_notify_preview(struct command_source *s);
static void on_preview_scene_changed(enum obs_frontend_event event, void *param);

static void cmdsrc_show(void *data)
{
	struct command_source *s = data;
	if (s->cmd_show) {
		fork_exec(s->cmd_show, s, &s->pid_show);
	}

	check_notify_preview(s);
	if (!s->is_shown)
		obs_frontend_add_event_callback(on_preview_scene_changed, data);
	s->is_shown = true;
}

#ifndef _WIN32
static void cmdsrc_kill(const struct command_source *s, pid_t pid, int sig)
{
	blog(LOG_DEBUG, "source '%s' sending signal %d to PID %d", obs_source_get_name(s->self), sig, pid);
	kill(pid, sig);
}
#endif

static void cmdsrc_hide(void *data)
{
	struct command_source *s = data;
	if (s->cmd_hide) {
		fork_exec(s->cmd_hide, s, NULL);
	}

#ifndef _WIN32
	if (s->pid_show && s->sig_show)
		cmdsrc_kill(s, s->pid_show, s->sig_show);
#endif

	check_notify_preview(s);
	if (s->is_shown)
		obs_frontend_remove_event_callback(on_preview_scene_changed, data);
	s->is_shown = false;
}

static void cmdsrc_activate(void *data)
{
	struct command_source *s = data;
	if (s->cmd_activate) {
		fork_exec(s->cmd_activate, s, &s->pid_activate);
	}
}

static void cmdsrc_deactivate(void *data)
{
	struct command_source *s = data;
	if (s->cmd_deactivate) {
		fork_exec(s->cmd_deactivate, s, NULL);
	}

#ifndef _WIN32
	if (s->pid_activate && s->sig_activate)
		cmdsrc_kill(s, s->pid_activate, s->sig_activate);
#endif
}

static inline void cmdsrc_previewed(struct command_source *s)
{
	if (s->cmd_previewed)
		fork_exec(s->cmd_previewed, s, &s->pid_preview);
}

static inline void cmdsrc_unpreviewed(struct command_source *s)
{
	if (s->cmd_unpreviewed)
		fork_exec(s->cmd_unpreviewed, s, NULL);

#ifndef _WIN32
	if (s->pid_preview && s->sig_preview)
		cmdsrc_kill(s, s->pid_preview, s->sig_preview);
#endif
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
	obs_source_t *preview_soure = obs_frontend_get_current_preview_scene();
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
	default:
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
	obs_data_set_default_int(settings, "sig", SIGTERM);
#endif
}

static obs_properties_t *command_source_get_properties(void *unused)
{
	UNUSED_PARAMETER(unused);
	obs_properties_t *props;
#ifndef _WIN32
	obs_property_t *prop;
#endif

	props = obs_properties_create();

	obs_properties_add_text(props, "cmd_show", obs_module_text("shown"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_hide", obs_module_text("hidden"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_activate", obs_module_text("activated"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_deactivate", obs_module_text("deactivated"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_previewed", obs_module_text("shown-in-preview"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(props, "cmd_unpreviewed", obs_module_text("hidden-from-preview"), OBS_TEXT_DEFAULT);

#ifndef _WIN32
	obs_properties_add_bool(props, "sigen_show", obs_module_text("prop-sig-show"));
	obs_properties_add_bool(props, "sigen_activate", obs_module_text("prop-sig-active"));
	obs_properties_add_bool(props, "sigen_preview", obs_module_text("prop-sig-preview"));
	prop = obs_properties_add_list(props, "sig", obs_module_text("prop-sig"), OBS_COMBO_TYPE_LIST,
				       OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(prop, "SIGABRT", SIGABRT);
	obs_property_list_add_int(prop, "SIGINT", SIGINT);
	obs_property_list_add_int(prop, "SIGKILL", SIGKILL);
	obs_property_list_add_int(prop, "SIGTERM", SIGTERM);
	obs_property_list_add_int(prop, "SIGHUP", SIGHUP);
	obs_property_list_add_int(prop, "SIGUSR1", SIGUSR1);
	obs_property_list_add_int(prop, "SIGUSR2", SIGUSR2);
#endif

	return props;
}

static void command_source_destroy(void *data)
{
	struct command_source *s = data;

	if (s->is_shown)
		obs_frontend_remove_event_callback(on_preview_scene_changed, data);

	if (s->cmd_show)
		bfree(s->cmd_show);
	if (s->cmd_hide)
		bfree(s->cmd_hide);
	if (s->cmd_activate)
		bfree(s->cmd_activate);
	if (s->cmd_deactivate)
		bfree(s->cmd_deactivate);
	if (s->cmd_previewed)
		bfree(s->cmd_previewed);
	if (s->cmd_unpreviewed)
		bfree(s->cmd_unpreviewed);
#ifndef _WIN32
	da_free(s->running_pids);
#endif // not _WIN32

	bfree(s);
}

static inline char *bstrdup_nonzero(const char *s)
{
	if (!*s)
		return NULL;
	return bstrdup(s);
}

static void command_source_update(void *data, obs_data_t *settings)
{
	struct command_source *s = data;

	if (s->cmd_show)
		bfree(s->cmd_show);
	if (s->cmd_hide)
		bfree(s->cmd_hide);
	if (s->cmd_activate)
		bfree(s->cmd_activate);
	if (s->cmd_deactivate)
		bfree(s->cmd_deactivate);
	if (s->cmd_previewed)
		bfree(s->cmd_previewed);
	if (s->cmd_unpreviewed)
		bfree(s->cmd_unpreviewed);
	s->cmd_show = bstrdup_nonzero(obs_data_get_string(settings, "cmd_show"));
	s->cmd_hide = bstrdup_nonzero(obs_data_get_string(settings, "cmd_hide"));
	s->cmd_activate = bstrdup_nonzero(obs_data_get_string(settings, "cmd_activate"));
	s->cmd_deactivate = bstrdup_nonzero(obs_data_get_string(settings, "cmd_deactivate"));
	s->cmd_previewed = bstrdup_nonzero(obs_data_get_string(settings, "cmd_previewed"));
	s->cmd_unpreviewed = bstrdup_nonzero(obs_data_get_string(settings, "cmd_unpreviewed"));
#ifndef _WIN32
	int sig = obs_data_get_int(settings, "sig");
	s->sig_show = obs_data_get_bool(settings, "sigen_show") ? sig : 0;
	s->sig_activate = obs_data_get_bool(settings, "sigen_activate") ? sig : 0;
	s->sig_preview = obs_data_get_bool(settings, "sigen_preview") ? sig : 0;
#endif
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
static bool cmdsrc_waitpid(struct command_source *s, pid_t pid)
{
	int wstatus = 0;
	pid_t ret = waitpid(pid, &wstatus, WNOHANG);
	if (ret == pid) {
		char st[32] = {0};
		if (WIFSIGNALED(wstatus))
			snprintf(st, sizeof(st) - 1, " by signal %d", WTERMSIG(wstatus));
		else if (WCOREDUMP(wstatus))
			snprintf(st, sizeof(st) - 1, " with core dump");
		else if (WIFEXITED(wstatus))
			snprintf(st, sizeof(st) - 1, " with status %d", WEXITSTATUS(wstatus));
		blog(LOG_DEBUG, "source '%s': PID %d exited%s", obs_source_get_name(s->self), pid, st);
		return true;
	}
	return false;
}

static void cmdsrc_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);
	struct command_source *s = data;

	if (s->pid_show) {
		if (cmdsrc_waitpid(s, s->pid_show))
			s->pid_show = 0;
	}

	if (s->pid_activate) {
		if (cmdsrc_waitpid(s, s->pid_activate))
			s->pid_activate = 0;
	}

	if (s->pid_preview) {
		if (cmdsrc_waitpid(s, s->pid_preview))
			s->pid_preview = 0;
	}

	for (size_t i = 0; i < s->running_pids.num; i++) {
		if (cmdsrc_waitpid(s, s->running_pids.array[i])) {
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
