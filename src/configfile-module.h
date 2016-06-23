#ifndef __WMUD_MODULE_CONFIGFILE_H__
#define __WMUD_MODULE_CONFIGFILE_H__

#include "wmud-session.h"

typedef enum {
	WMUD_CONF_LOG_NONE,
	WMUD_CONF_LOG_SYSLOG,
	WMUD_CONF_LOG_FILE,
	WMUD_CONF_LOG_CONSOLE
} wMUDConfigurationLogging;

typedef enum {
	WMUD_CONF_DAEMONIZE_YES,
	WMUD_CONF_DAEMONIZE_NO,
	WMUD_CONF_DAEMONIZE_FORCE
} wMUDConfigurationDaemonize;

typedef struct {
	/* A GSList of _wMUDConfigurationInterfaces */
	GSList *interfaces;

	wMUDConfigurationLogging log_dest;
	gchar *logfile;
	gboolean log_found;

	wMUDConfigurationLogging debug_log_dest;
	gchar *debug_logfile;
	gboolean debug_log_found;

	wMUDConfigurationLogging info_log_dest;
	gchar *info_logfile;
	gboolean info_log_found;

	wMUDConfigurationLogging warning_log_dest;
	gchar *warning_logfile;
	gboolean warning_log_found;

	wMUDConfigurationLogging error_log_dest;
	gchar *error_logfile;
	gboolean error_log_found;

	wMUDConfigurationDaemonize daemonize;

	gboolean chat_enabled;
	gboolean dbus_enabled;

	gchar *modules_dir;

	gchar *statesave_module;
	gchar **protocol_modules;

	GSList *statesave_parameters;
	GSList *protocol_parameters;
} wMUDConfiguration;

typedef struct {
	wMUDSessionType type;
	gchar *name;
	GInetAddr *inetaddr;
	guint timeout;
} wMUDConfigurationInterface;

typedef struct {
	gchar *name;
	GSList *datalist;
} wMUDConfigurationGroup;

typedef struct {
	gchar *key;
	gchar *value;
} wMUDConfigurationValue;

#endif /* __WMUD_MODULE_CONFIGFILE_H__ */

