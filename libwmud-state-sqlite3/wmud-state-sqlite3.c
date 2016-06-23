#include <glib.h>
#include <sqlite3.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "module.h"

static gchar *state_file = NULL;
static sqlite3 *statesave_connection = NULL;

gboolean
wmud_statesave_sqlite3_is_statesave(void)
{
	return TRUE;
}

static gint
_wmud_statesave_sqlite3_find_config_group(gconstpointer list_data, gconstpointer lookup_data)
{
	wMUDConfigurationGroup *group = (wMUDConfigurationGroup *)list_data;
	gchar *name_to_find = (gchar *)lookup_data;

	Context return g_utf8_collate(group->name, name_to_find);
}

static void
_wmud_statesave_sqlite3_parse_config(gpointer data, gpointer userdata)
{
	wMUDConfigurationValue *parameter = (wMUDConfigurationValue *)data;

	if (g_utf8_collate("state file", parameter->key) == 0)
	{
		state_file = g_strdup(parameter->value);
	}
}

gboolean
wmud_statesave_sqlite3_load(wMUDConfiguration *config)
{
	GSList *statesave_params;
	wMUDConfigurationGroup *config_group;

	wmud_log_debug("Initializing SQLite3 state saving module...");

	if (!sqlite3_threadsafe())
	{
		wmud_log_error("SQLite3 library is not compiled in a thread-safe manner.");
		return FALSE;
	}

	Context statesave_params = g_slist_find_custom(config->statesave_parameters, "sqlite3", _wmud_statesave_sqlite3_find_config_group);

	if (statesave_params == NULL)
	{
		wmud_log_error("Cannot find group [statesave sqlite3] in configfile!");
		return FALSE;
	}

	config_group = (wMUDConfigurationGroup *)(statesave_params->data);
	g_slist_foreach(config_group->datalist, _wmud_statesave_sqlite3_parse_config, NULL);

	if (state_file == NULL)
	{
		wmud_log_error("Cannot find state file parameter in configuration file (should be under group [statesave sqlite3]");
		return FALSE;
	}

	wmud_log_debug("Will save state into SQLite3 file %s", state_file);

	switch (sqlite3_open(state_file, &statesave_connection))
	{
		case SQLITE_OK:
			wmud_log_info("State file opened successfully");
			break;
		default:
			wmud_log_error("Unprocessed return value from sqlite3_open()!");
			return FALSE;
			break;
	}

	return TRUE;
}

void
wmud_statesave_sqlite3_unload(void)
{
	sqlite3_close(statesave_connection);
}

