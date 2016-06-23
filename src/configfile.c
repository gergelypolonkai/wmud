#include <glib.h>
#include <gnet.h>
#include <string.h>

#include "wmud.h"
#include "configfile.h"
#include "logger.h"

struct _wMUDIfaceFinder {
	wMUDSessionType type;
	gchar *name;
};

static gint
_wmud_find_keyed_member(gconstpointer this_group, gconstpointer group_to_find)
{
	wMUDConfigurationGroup *group_data = (wMUDConfigurationGroup *)this_group;
	gchar *name = (gchar *)group_to_find;

	return g_utf8_collate(group_data->name, name);
}

static gint
_wmud_find_iface(gconstpointer iface, gconstpointer iffinder)
{
	wMUDConfigurationInterface *interface = (wMUDConfigurationInterface *)iface;
	gchar *name_to_find = ((struct _wMUDIfaceFinder *)iffinder)->name;
	wMUDSessionType type = ((struct _wMUDIfaceFinder *)iffinder)->type;

	return ((g_utf8_collate(interface->name, name_to_find) == 0) && (interface->type == type)) ? 0 : 1;
}

gboolean
_wmud_config_process_value(wMUDConfiguration *conf, gchar *group, gchar *key, gchar *value)
{
	GPatternSpec *group_ptn;

	if (g_utf8_collate("general", group) == 0)
	{
		if (
				(g_utf8_collate("log", key) == 0)
				|| (g_utf8_collate("debug log", key) == 0)
				|| (g_utf8_collate("info log", key) == 0)
				|| (g_utf8_collate("warning log", key) == 0)
				|| (g_utf8_collate("error log", key) == 0)
		)
		{
			if (g_utf8_collate("none", value) == 0)
			{
				if (g_utf8_collate("log", key) == 0)
				{
					conf->log_found = TRUE;
					conf->log_dest = WMUD_CONF_LOG_NONE;
					if (conf->logfile)
					{
						g_free(conf->logfile);
						conf->logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("debug log", key) == 0)
				{
					conf->debug_log_found = TRUE;
					conf->debug_log_dest = WMUD_CONF_LOG_NONE;
					if (conf->debug_logfile)
					{
						g_free(conf->debug_logfile);
						conf->debug_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("info log", key) == 0)
				{
					conf->info_log_found = TRUE;
					conf->info_log_dest = WMUD_CONF_LOG_NONE;
					if (conf->info_logfile)
					{
						g_free(conf->info_logfile);
						conf->info_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("warning log", key) == 0)
				{
					conf->warning_log_found = TRUE;
					conf->warning_log_dest = WMUD_CONF_LOG_NONE;
					if (conf->warning_logfile)
					{
						g_free(conf->warning_logfile);
						conf->warning_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("error log", key) == 0)
				{
					conf->error_log_found = TRUE;
					conf->error_log_dest = WMUD_CONF_LOG_NONE;
					if (conf->error_logfile)
					{
						g_free(conf->error_logfile);
						conf->error_logfile = NULL;
					}
					return TRUE;
				}
			}
			if (strncmp("file:", value, 5) == 0)
			{
				if (g_utf8_collate("log", key) == 0)
				{
					gchar *temp_file;
					wMUDConfigurationLogging temp_logging;
					gboolean temp_found;

					temp_logging = conf->log_dest;
					temp_file = conf->logfile;
					temp_found = conf->log_found;

					conf->logfile = g_strdup(value + 5);
					if (!*(conf->logfile))
					{
						wmud_log_error("Log file name must contain a string for %s/%s!", group, key);
						conf->logfile = temp_file;
						return FALSE;
					}
					if (temp_file)
					{
						g_free(temp_file);
					}
					conf->log_dest = WMUD_CONF_LOG_FILE;
					conf->log_found = TRUE;
					return TRUE;
				}
				else if (g_utf8_collate("debug log", key) == 0)
				{
					gchar *temp_file;
					wMUDConfigurationLogging temp_logging;
					gboolean temp_found;

					temp_logging = conf->debug_log_dest;
					temp_file = conf->debug_logfile;
					temp_found = conf->debug_log_found;

					conf->debug_logfile = g_strdup(value + 5);
					if (!*(conf->debug_logfile))
					{
						wmud_log_error("Debug log file name must contain a string for %s/%s!", group, key);
						conf->debug_logfile = temp_file;
						return FALSE;
					}
					if (temp_file)
					{
						g_free(temp_file);
					}
					conf->debug_log_dest = WMUD_CONF_LOG_FILE;
					conf->debug_log_found = TRUE;
					return TRUE;
				}
				else if (g_utf8_collate("info log", key) == 0)
				{
					gchar *temp_file;
					wMUDConfigurationLogging temp_logging;
					gboolean temp_found;

					temp_logging = conf->info_log_dest;
					temp_file = conf->info_logfile;
					temp_found = conf->info_log_found;

					conf->info_logfile = g_strdup(value + 5);
					if (!*(conf->info_logfile))
					{
						wmud_log_error("Info log file name must contain a string for %s/%s!", group, key);
						conf->info_log_dest = temp_logging;
						conf->info_logfile = temp_file;
						conf->info_log_found = temp_found;
						return FALSE;
					}
					if (temp_file)
					{
						g_free(temp_file);
					}
					conf->info_log_dest = WMUD_CONF_LOG_FILE;
					conf->info_log_found = TRUE;
					return TRUE;
				}
				else if (g_utf8_collate("warning log", key) == 0)
				{
					gchar *temp_file;
					wMUDConfigurationLogging temp_logging;
					gboolean temp_found;

					temp_logging = conf->warning_log_dest;
					temp_file = conf->warning_logfile;
					temp_found = conf->warning_log_found;

					conf->warning_logfile = g_strdup(value + 5);
					if (!*(conf->warning_logfile))
					{
						wmud_log_error("Warning log file name must contain a string for %s/%s!", group, key);
						conf->warning_logfile = temp_file;
						return FALSE;
					}
					if (temp_file)
					{
						g_free(temp_file);
					}
					conf->warning_log_dest = WMUD_CONF_LOG_FILE;
					conf->warning_log_found = TRUE;
					return TRUE;
				}
				else if (g_utf8_collate("error log", key) == 0)
				{
					gchar *temp_file;
					wMUDConfigurationLogging temp_logging;
					gboolean temp_found;

					temp_logging = conf->error_log_dest;
					temp_file = conf->error_logfile;
					temp_found = conf->error_log_found;

					conf->error_logfile = g_strdup(value + 5);
					if (!*(conf->error_logfile))
					{
						wmud_log_error("Error log file name must contain a string for %s/%s!", group, key);
						conf->error_logfile = temp_file;
						return FALSE;
					}
					if (temp_file)
					{
						g_free(temp_file);
					}
					conf->error_log_dest = WMUD_CONF_LOG_FILE;
					conf->error_log_found = TRUE;
					return TRUE;
				}
			}
			if (
					(g_utf8_collate("syslog", value) == 0)
					|| (
						(conf->daemonize == WMUD_CONF_DAEMONIZE_FORCE)
						&& (g_utf8_collate("console", value) == 0)
					)
			)
			{
				if (g_utf8_collate("log", key) == 0)
				{
					conf->log_dest = WMUD_CONF_LOG_SYSLOG;
					conf->log_found = TRUE;
					if (conf->logfile)
					{
						g_free(conf->logfile);
						conf->logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("debug log", key) == 0)
				{
					conf->debug_log_dest = WMUD_CONF_LOG_SYSLOG;
					conf->debug_log_found = TRUE;
					if (conf->debug_logfile)
					{
						g_free(conf->debug_logfile);
						conf->debug_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("info log", key) == 0)
				{
					conf->info_log_dest = WMUD_CONF_LOG_SYSLOG;
					conf->info_log_found = TRUE;
					if (conf->info_logfile)
					{
						g_free(conf->info_logfile);
						conf->info_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("warning log", key) == 0)
				{
					conf->warning_log_dest = WMUD_CONF_LOG_SYSLOG;
					conf->warning_log_found = TRUE;
					if (conf->warning_logfile)
					{
						g_free(conf->warning_logfile);
						conf->warning_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("error log", key) == 0)
				{
					conf->error_log_dest = WMUD_CONF_LOG_SYSLOG;
					conf->error_log_found = TRUE;
					if (conf->error_logfile)
					{
						g_free(conf->error_logfile);
						conf->error_logfile = NULL;
					}
					return TRUE;
				}
			}
			if ((g_utf8_collate("console", value) == 0) && (conf->daemonize != WMUD_CONF_DAEMONIZE_FORCE))
			{
				if (g_utf8_collate("log", key) == 0)
				{
					conf->log_dest = WMUD_CONF_LOG_CONSOLE;
					conf->log_found = TRUE;
					if (conf->logfile)
					{
						g_free(conf->logfile);
						conf->logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("debug log", key) == 0)
				{
					conf->debug_log_dest = WMUD_CONF_LOG_CONSOLE;
					conf->debug_log_found = TRUE;
					if (conf->debug_logfile)
					{
						g_free(conf->debug_logfile);
						conf->debug_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("info log", key) == 0)
				{
					conf->info_log_dest = WMUD_CONF_LOG_CONSOLE;
					conf->info_log_found = TRUE;
					if (conf->info_logfile)
					{
						g_free(conf->info_logfile);
						conf->info_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("warning log", key) == 0)
				{
					conf->warning_log_dest = WMUD_CONF_LOG_CONSOLE;
					conf->warning_log_found = TRUE;
					if (conf->warning_logfile)
					{
						g_free(conf->warning_logfile);
						conf->warning_logfile = NULL;
					}
					return TRUE;
				}
				else if (g_utf8_collate("error log", key) == 0)
				{
					conf->error_log_dest = WMUD_CONF_LOG_CONSOLE;
					conf->error_log_found = TRUE;
					if (conf->error_logfile)
					{
						g_free(conf->error_logfile);
						conf->error_logfile = NULL;
					}
					return TRUE;
				}
			}
			wmud_log_error("Found the log option with an unknown value of %s", value);
			return FALSE;
		}

		if (g_utf8_collate("daemonize", key) == 0)
		{
			if (g_utf8_collate("yes", value) == 0)
			{
				conf->daemonize = WMUD_CONF_DAEMONIZE_YES;
				return TRUE;
			}
			else if (g_utf8_collate("no", value) == 0)
			{
				conf->daemonize = WMUD_CONF_DAEMONIZE_NO;
				return TRUE;
			}
			else if (g_utf8_collate("force", value) == 0)
			{
				conf->daemonize = WMUD_CONF_DAEMONIZE_FORCE;
				return TRUE;
			}
			wmud_log_error("Daemonize must have the values yes, no or force");
			return FALSE;
		}

		if (g_utf8_collate("chat", key) == 0)
		{
			if (g_utf8_collate("yes", value) == 0)
			{
				conf->chat_enabled = TRUE;
				return TRUE;
			}
			else if (g_utf8_collate("no", value) == 0)
			{
				conf->chat_enabled = FALSE;
				return TRUE;
			}
			wmud_log_error("Chat must have the value yes or no");
			return FALSE;
		}

		if (g_utf8_collate("dbus", key) == 0)
		{
			if (g_utf8_collate("yes", value) == 0)
			{
				conf->dbus_enabled = TRUE;
				return TRUE;
			}
			else if (g_utf8_collate("no", value) == 0)
			{
				conf->dbus_enabled = TRUE;
				return TRUE;
			}
			wmud_log_error("DBus must have the value yes or no");
			return FALSE;
		}

		wmud_log_error("Unknown key (%s) found in configuration file in group [%s]", key, group);
		return FALSE;
	}

	if (g_utf8_collate("modules", group) == 0)
	{
		if (g_utf8_collate("modules dir", key) == 0)
		{
			if (conf->modules_dir)
			{
				g_free(conf->modules_dir);
			}
			conf->modules_dir = g_strdup(value);
			return TRUE;
		}

		if (g_utf8_collate("statesave", key) == 0)
		{
			if (conf->statesave_module)
			{
				g_free(conf->statesave_module);
			}
			conf->statesave_module = g_strdup(value);
			return TRUE;
		}

		if (g_utf8_collate("protocol", key) == 0)
		{
			if (conf->protocol_modules)
			{
				g_strfreev(conf->protocol_modules);
			}

			conf->protocol_modules = g_strsplit(value, ":", 0);
			return TRUE;
		}

		wmud_log_error("Unknown key (%s) found in configuration file in group [%s]", key, group);
		return FALSE;
	}

	group_ptn = g_pattern_spec_new("statesave *");
	if (g_pattern_match(group_ptn, strlen(group), group, NULL))
	{
		GSList *statesave_member;
		wMUDConfigurationGroup *conf_group;
		wMUDConfigurationValue *conf_data;

		statesave_member = g_slist_find_custom(conf->statesave_parameters, group + 10, _wmud_find_keyed_member);
		if (statesave_member == NULL)
		{
			conf_group = g_new0(wMUDConfigurationGroup, 1);
			conf_group->name = g_strdup(group + 10);
			conf_group->datalist = NULL;
			conf->statesave_parameters = g_slist_append(conf->statesave_parameters, conf_group);
		}
		else
		{
			conf_group = (wMUDConfigurationGroup *)(statesave_member->data);
		}

		conf_data = g_new0(wMUDConfigurationValue, 1);
		conf_data->key = g_strdup(key);
		conf_data->value = g_strdup(value);
		conf_group->datalist = g_slist_append(conf_group->datalist, conf_data);

		return TRUE;
	}
	g_pattern_spec_free(group_ptn);

	group_ptn = g_pattern_spec_new("protocol * *");
	if (g_pattern_match(group_ptn, strlen(group), group, NULL))
	{
		GSList *protocol_member;
		wMUDConfigurationGroup *conf_group;
		wMUDConfigurationValue *conf_data;

		protocol_member = g_slist_find_custom(conf->protocol_parameters, group + 10, _wmud_find_keyed_member);
		if (protocol_member == NULL)
		{
			conf_group = g_new0(wMUDConfigurationGroup, 1);
			conf_group->name = g_strdup(group + 10);
			conf_group->datalist = NULL;
			conf->protocol_parameters = g_slist_append(conf->protocol_parameters, conf_group);
		}
		else
		{
			conf_group = (wMUDConfigurationGroup *)(protocol_member->data);
		}

		conf_data = g_new0(wMUDConfigurationValue, 1);
		conf_data->key = g_strdup(key);
		conf_data->value = g_strdup(value);
		conf_group->datalist = g_slist_append(conf_group->datalist, conf_data);

		return TRUE;
	}
	g_pattern_spec_free(group_ptn);

	group_ptn = g_pattern_spec_new("irc *");
	if (g_pattern_match(group_ptn, strlen(group), group, NULL))
	{
		GSList *temp;
		wMUDConfigurationInterface *interface;
		gchar *iface_name;
		struct _wMUDIfaceFinder *finder;

		finder = g_new0(struct _wMUDIfaceFinder, 1);

		finder->type = WMUD_SESSION_TYPE_IRC;
		finder->name = iface_name = g_utf8_offset_to_pointer(group, 4);
		Context;
		temp = g_slist_find_custom(conf->interfaces, finder, _wmud_find_iface);
		Context;
		g_free(finder);

		if (temp == NULL)
		{
			Context;
			interface = (wMUDConfigurationInterface *)g_new0(wMUDConfigurationInterface, 1);
			interface->name = g_strdup(iface_name);
			interface->type = WMUD_SESSION_TYPE_IRC;
			conf->interfaces = g_slist_append(conf->interfaces, interface);
		}
		else
		{
			Context;
			interface = (wMUDConfigurationInterface *)(temp->data);
		}

		if (g_utf8_collate("port", key) == 0)
		{
			guint64 portnumber;
			gchar *endptr;

			portnumber = g_ascii_strtoull(value, &endptr, 10);

			if ((endptr != NULL) && (*endptr != 0))
			{
				wmud_log_error("Error in configuration file. Value of port can only contain numbers in group [%s]!", group);
			}

			if (interface->inetaddr == NULL)
			{
				interface->inetaddr = gnet_inetaddr_new("0.0.0.0", portnumber);
			}
			else
			{
				gnet_inetaddr_set_port(interface->inetaddr, (gint)portnumber);
			}

			g_pattern_spec_free(group_ptn);
			return TRUE;
		}
		wmud_log_error("Unknown key (%s) found in configuration file in group [%s]", key, group);
		g_pattern_spec_free(group_ptn);
		return FALSE;
	}
	g_pattern_spec_free(group_ptn);

	wmud_log_error("Unknown group (%s) found in configuration file", group);
	return FALSE;
}

static void
_wmud_configuration_free_interface(gpointer data, gpointer user_data)
{
	wMUDConfigurationInterface *iface = (wMUDConfigurationInterface *)data;

	if (iface->name)
	{
		g_free(iface->name);
	}
	if (iface->inetaddr)
	{
		gnet_inetaddr_unref(iface->inetaddr);
	}
	g_free(data);
}

static void
_wmud_configuration_free_value(gpointer data, gpointer user_data)
{
	wMUDConfigurationValue *v = (wMUDConfigurationValue *)data;

	if (v->key)
	{
		g_free(v->key);
	}
	if (v->value)
	{
		g_free(v->value);
	}
}

static void
_wmud_configuration_free_parameters(gpointer data, gpointer user_data)
{
	wMUDConfigurationGroup *group = (wMUDConfigurationGroup *)data;

	g_slist_foreach(group->datalist, _wmud_configuration_free_value, NULL);
	g_slist_free(group->datalist);
	if (group->name)
	{
		g_free(group->name);
	}
}

void
wmud_configuration_free(wMUDConfiguration **configuration)
{
	g_slist_foreach((*configuration)->interfaces, _wmud_configuration_free_interface, NULL);
	g_slist_free((*configuration)->interfaces);

	g_slist_foreach((*configuration)->statesave_parameters, _wmud_configuration_free_parameters, NULL);
	g_slist_free((*configuration)->statesave_parameters);

	g_slist_foreach((*configuration)->protocol_parameters, _wmud_configuration_free_parameters, NULL);
	g_slist_free((*configuration)->protocol_parameters);

	if ((*configuration)->logfile)
	{
		g_free((*configuration)->logfile);
	}
	if ((*configuration)->debug_logfile)
	{
		g_free((*configuration)->debug_logfile);
	}
	if ((*configuration)->info_logfile)
	{
		g_free((*configuration)->info_logfile);
	}
	if ((*configuration)->warning_logfile)
	{
		g_free((*configuration)->warning_logfile);
	}
	if ((*configuration)->error_logfile)
	{
		g_free((*configuration)->error_logfile);
	}

	if ((*configuration)->modules_dir)
	{
		g_free((*configuration)->modules_dir);
	}

	if ((*configuration)->statesave_module)
	{
		g_free((*configuration)->statesave_module);
	}

	if ((*configuration)->protocol_modules)
	{
		g_strfreev((*configuration)->protocol_modules);
	}

	g_free((*configuration));

	*configuration = NULL;
}

gboolean
wmud_configfile_read(gchar *filename, wMUDConfiguration **configuration, GError **error)
{
	GKeyFile *configfile;
	GError *file_error = NULL;
	gchar **grouplist,
	      **group;
	wMUDConfiguration *conf;
       
	*configuration = NULL;

	Context;

	if (filename == NULL)
	{
		return FALSE;
	}

	conf = g_new0(wMUDConfiguration, 1);

	wmud_log_debug("Reading configuration file %s", filename);

	configfile = g_key_file_new();

	if (!g_key_file_load_from_file(configfile, filename, G_KEY_FILE_NONE, &file_error))
	{
		if (file_error->domain == G_FILE_ERROR)
		{
			wmud_log_error("Unable to open configuration file (%s): %s", filename, file_error->message);
		}

		if (file_error->domain == G_KEY_FILE_ERROR)
		{
			wmud_log_error("Unable to parse configuration file (%s): %s", filename, file_error->message);
		}

		g_key_file_free(configfile);

		return FALSE;
	}

	grouplist = g_key_file_get_groups(configfile, NULL);

	for (group = grouplist; *group; group++)
	{
		gchar **keylist,
		      **key;
		wmud_log_debug("Processing group: [%s]", *group);

		keylist = g_key_file_get_keys(configfile, *group, NULL, NULL);

		for (key = keylist; *key; key++)
		{
			gchar *value;
			gboolean line_processed;

			value = g_key_file_get_value(configfile, *group, *key, NULL);

			line_processed = _wmud_config_process_value(conf, *group, *key, value);

			g_free(value);

			if (!line_processed)
			{
				g_strfreev(keylist);
				g_strfreev(grouplist);
				g_key_file_free(configfile);

				return FALSE;
			}
		}

		g_strfreev(keylist);
	}

	g_strfreev(grouplist);

	g_key_file_free(configfile);

	/* If one of the logging options not found, fall back to the "log"
	 * option. If neither log can be found, configuration is invalid
	 */
#ifdef DEBUG
	if (!conf->debug_log_found)
	{
		if (!conf->log_found)
		{
			wmud_log_error("Debug log not found in configuration file");
			wmud_configuration_free(&conf);
			return FALSE;
		}
		else
		{
			conf->debug_log_found = TRUE;
			conf->debug_log_dest = conf->log_dest;
			conf->debug_logfile = (conf->logfile == NULL) ? NULL : g_strdup(conf->logfile);
		}
	}
#endif
	if (!conf->info_log_found)
	{
		if (!conf->log_found)
		{
			wmud_log_error("Info log not found in configuration file");
			wmud_configuration_free(&conf);
			return FALSE;
		}
		else
		{
			conf->info_log_found = TRUE;
			conf->info_log_dest = conf->log_dest;
			conf->info_logfile = (conf->logfile == NULL) ? NULL : g_strdup(conf->logfile);
		}
	}
	if (!conf->warning_log_found)
	{
		if (!conf->log_found)
		{
			wmud_log_error("Warning log not found in configuration file");
			wmud_configuration_free(&conf);
			return FALSE;
		}
		else
		{
			conf->warning_log_found = TRUE;
			conf->warning_log_dest = conf->log_dest;
			conf->warning_logfile = (conf->logfile == NULL) ? NULL : g_strdup(conf->logfile);
		}
	}
	if (!conf->error_log_found)
	{
		if (!conf->log_found)
		{
			wmud_log_error("Error log not found in configuration file");
			wmud_configuration_free(&conf);
			return FALSE;
		}
		else
		{
			conf->error_log_found = TRUE;
			conf->error_log_dest = conf->log_dest;
			conf->error_logfile = (conf->logfile == NULL) ? NULL : g_strdup(conf->logfile);
		}
	}

	/* If daemonizing is forced, we cannot log to the console. However,
	 * daemonizing is not supported if DEBUG is turned on
	 */
#ifdef DEBUG
	if ((conf->daemonize == WMUD_CONF_DAEMONIZE_FORCE) || (conf->daemonize == WMUD_CONF_DAEMONIZE_YES))
	{
		wmud_log_warning("Cannot daemonize in DEBUG mode");
		conf->daemonize = WMUD_CONF_DAEMONIZE_NO;
	}
#endif

	if (conf->daemonize == WMUD_CONF_DAEMONIZE_FORCE)
	{
#ifdef DEBUG
		if (conf->debug_log_dest == WMUD_CONF_LOG_CONSOLE)
		{
			wmud_log_warning("Cannot log to console when daemonized. Debug log is falling back to syslog");
			conf->debug_log_dest = WMUD_CONF_LOG_SYSLOG;
		}
#endif
		if (conf->info_log_dest == WMUD_CONF_LOG_CONSOLE)
		{
			wmud_log_warning("Cannot log to console when daemonized. Info log is falling back to syslog");
			conf->info_log_dest = WMUD_CONF_LOG_SYSLOG;
		}
		if (conf->warning_log_dest == WMUD_CONF_LOG_CONSOLE)
		{
			wmud_log_warning("Cannot log to console when daemonized. Warning log is falling back to syslog");
			conf->warning_log_dest = WMUD_CONF_LOG_SYSLOG;
		}
		if (conf->error_log_dest == WMUD_CONF_LOG_CONSOLE)
		{
			wmud_log_warning("Cannot log to console when daemonized. Error log is falling back to syslog");
			conf->error_log_dest = WMUD_CONF_LOG_SYSLOG;
		}
	}

#ifdef DEBUG
	wmud_log_warning("Logging must be sent to the console in debug mode!");
	if (conf->debug_log_dest != WMUD_CONF_LOG_CONSOLE)
	{
		if (conf->debug_logfile)
		{
			g_free(conf->debug_logfile);
			conf->debug_logfile = NULL;
		}
		conf->debug_log_dest = WMUD_CONF_LOG_CONSOLE;
	}
	if (conf->info_log_dest != WMUD_CONF_LOG_CONSOLE)
	{
		if (conf->info_logfile)
		{
			g_free(conf->info_logfile);
			conf->info_logfile = NULL;
		}
		conf->info_log_dest = WMUD_CONF_LOG_CONSOLE;
	}
	if (conf->warning_log_dest != WMUD_CONF_LOG_CONSOLE)
	{
		if (conf->warning_logfile)
		{
			g_free(conf->warning_logfile);
			conf->warning_logfile = NULL;
		}
		conf->warning_log_dest = WMUD_CONF_LOG_CONSOLE;
	}
	if (conf->error_log_dest != WMUD_CONF_LOG_CONSOLE)
	{
		if (conf->error_logfile)
		{
			g_free(conf->error_logfile);
			conf->error_logfile = NULL;
		}
		conf->error_log_dest = WMUD_CONF_LOG_CONSOLE;
	}
#endif

	*configuration = conf;
	return TRUE;
}

