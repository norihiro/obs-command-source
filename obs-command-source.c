#include <obs-module.h>
#include <util/platform.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>

struct command_source {
	char *cmd_show;
	char *cmd_hide;
	char *cmd_activate;
	char *cmd_deactivate;

	bool is_showing;
	bool is_active;
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-command-source", "en-US")

static void fork_exec(const char *cmd)
{
	if(!fork()) {
		execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
	}
}

static void cmdsrc_show(void *data)
{
	struct command_source *s = data;
	if(s->cmd_show) {
		fork_exec(s->cmd_show);
	}
}

static void cmdsrc_hide(void *data)
{
	struct command_source *s = data;
	if(s->cmd_hide) {
		fork_exec(s->cmd_hide);
	}
}

static void cmdsrc_activate(void *data)
{
	struct command_source *s = data;
	if(s->cmd_activate) {
		fork_exec(s->cmd_activate);
	}
}

static void cmdsrc_deactivate(void *data)
{
	struct command_source *s = data;
	if(s->cmd_deactivate) {
		fork_exec(s->cmd_deactivate);
	}
}

static const char *command_source_name(void *unused)
{
	UNUSED_PARAMETER(unused);

	return obs_module_text("execute-command");
}

static void command_source_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, "cmd_show", "/bin/echo going to preview");
	obs_data_set_default_string(settings, "cmd_hide", "/bin/echo hiding from preview");
	obs_data_set_default_string(settings, "cmd_activate", "/bin/echo going to program");
	obs_data_set_default_string(settings, "cmd_deactivate", "/bin/echo retiring from program");
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

	return props;
}

static void command_source_destroy(void *data)
{
	struct command_source *s = data;

	if (s->cmd_show) bfree(s->cmd_show);
	if (s->cmd_hide) bfree(s->cmd_hide);
	if (s->cmd_activate) bfree(s->cmd_activate);
	if (s->cmd_deactivate) bfree(s->cmd_deactivate);

	bfree(s);
}

static void command_source_update(void *data, obs_data_t *settings)
{
	struct command_source *s = data;

	if (s->cmd_show) bfree(s->cmd_show);
	if (s->cmd_hide) bfree(s->cmd_hide);
	if (s->cmd_activate) bfree(s->cmd_activate);
	if (s->cmd_deactivate) bfree(s->cmd_deactivate);
	s->cmd_show = bstrdup(obs_data_get_string(settings, "cmd_show"));
	s->cmd_hide = bstrdup(obs_data_get_string(settings, "cmd_hide"));
	s->cmd_activate = bstrdup(obs_data_get_string(settings, "cmd_activate"));
	s->cmd_deactivate = bstrdup(obs_data_get_string(settings, "cmd_deactivate"));
}

static void *command_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(source);
	struct command_source *s = bzalloc(sizeof(struct command_source));

	command_source_update(s, settings);

	return s;
}

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
};

bool obs_module_load()
{
	obs_register_source(&command_source_info);

	return true;
}

void obs_module_unload(void)
{
}
