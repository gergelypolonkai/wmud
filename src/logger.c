#include <glib.h>

#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <libgnomevfs/gnome-vfs.h>

#include "wmud.h"
#include "configfile.h"
#include "logger.h"

enum {
	_LOG_DEST_STDOUT,
	_LOG_DEST_STDERR
};

typedef struct _wmud_message_level {
	int colorcode;
	const char *prefix;
	int debug_dest;
	wMUDConfigurationLogging dest;
	GnomeVFSHandle *logfile;
} wMUDMessageLevelType;

wMUDMessageLevelType wMUDMessageLevels[] = {
	{ 35, "debug", _LOG_DEST_STDERR, WMUD_CONF_LOG_CONSOLE, NULL },  /* LOG_DEBUG */
	{ 32, "info",  _LOG_DEST_STDOUT, WMUD_CONF_LOG_CONSOLE, NULL },  /* LOG_INFO  */
	{ 33, "warn",  _LOG_DEST_STDOUT, WMUD_CONF_LOG_CONSOLE, NULL },  /* LOG_WARN  */
	{ 31, "error", _LOG_DEST_STDERR, WMUD_CONF_LOG_CONSOLE, NULL }   /* LOG_ERROR */
};
#define wmud_message_level_count (sizeof(wmud_message_levels) / sizeof(wmud_message_level))

static void
_wmud_log_message(wMUDLogLevelType level, const char *filename, const int linenum, const char *fmt, va_list args)
{
	char *format,
		 *final_string;
	wMUDMessageLevelType level_info = wMUDMessageLevels[level];
	int syslog_level;
	static char *timestamp = NULL;
	time_t now;

	if (timestamp == NULL)
	{
		timestamp = g_malloc0(17 * sizeof(char));
	}

	switch (level_info.dest)
	{
		case WMUD_CONF_LOG_NONE:
			return;
		case WMUD_CONF_LOG_FILE:
			now = time(NULL);
			Context strftime(timestamp, 16, "%b %e %H:%M:%S", localtime(&now));
			Context gnome_vfs_seek(level_info.logfile, GNOME_VFS_SEEK_END, 0);
			format = g_strdup_printf("%s wMUD[%d]: [%s] %s:%d: %s\n", timestamp, wmud_pid, level_info.prefix, filename, linenum, fmt);
			final_string = g_strdup_vprintf(format, args);
			g_free(format);
			gnome_vfs_write(level_info.logfile, final_string, strlen(final_string), NULL);
			g_free(final_string);
			break;
		case WMUD_CONF_LOG_SYSLOG:
			switch (level)
			{
				case WMUD_LOG_DEBUG:
					syslog_level = LOG_DEBUG;
					break;
				case WMUD_LOG_INFO:
					syslog_level = LOG_INFO;
					break;
				case WMUD_LOG_WARN:
					syslog_level = LOG_WARNING;
					break;
				case WMUD_LOG_ERROR:
					syslog_level = LOG_ERR;
					break;
				default:
					syslog_level = LOG_CRIT;
					break;
			}
			format = g_strdup_printf("[%s] %s:%d: %s\n", level_info.prefix, filename, linenum, fmt);
			vsyslog(LOG_USER | syslog_level, format, args);
			g_free(format);
			break;
		case WMUD_CONF_LOG_CONSOLE:
			format = g_strdup_printf("\x1b[%dm\x1b[1m[%s] %s:%d: %s\x1b[0m\n", level_info.colorcode, level_info.prefix, filename, linenum, fmt);
			vfprintf((level_info.debug_dest == _LOG_DEST_STDERR) ? stderr : stdout, format, args);
			g_free(format);
			break;
	}
}

void
wmud_log(wMUDLogLevelType level, const char *filename, const int linenum, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	_wmud_log_message(level, filename, linenum, fmt, args);
	va_end(args);
}

gboolean
wmud_logger_init(wMUDConfiguration *config)
{
	gchar *full_logfile_uri;
	GnomeVFSURI *log_uri;

	g_assert(config != NULL);

	if (config->debug_log_dest == WMUD_CONF_LOG_FILE)
	{
		GnomeVFSResult state;
		full_logfile_uri = gnome_vfs_get_uri_from_local_path(config->debug_logfile);
		log_uri = gnome_vfs_uri_new(full_logfile_uri);
		g_free(full_logfile_uri);

		switch (gnome_vfs_open_uri(&(wMUDMessageLevels[WMUD_LOG_DEBUG].logfile), log_uri, GNOME_VFS_OPEN_READ | GNOME_VFS_OPEN_WRITE | GNOME_VFS_OPEN_RANDOM))
		{
			case GNOME_VFS_ERROR_NOT_FOUND:
				if ((state = gnome_vfs_create_uri(&(wMUDMessageLevels[WMUD_LOG_DEBUG].logfile), log_uri, GNOME_VFS_OPEN_WRITE, FALSE, 0640)) != GNOME_VFS_OK)
				{
					wmud_log_error("Unable to open debug logfile (%s)", config->debug_logfile);
					wmud_shutdown();
					return FALSE;
				}
				break;
			case GNOME_VFS_OK:
				break;
			default:
				wmud_log_error("Unprocessed return state from gnome_vfs_open_uri()");
				break;
		}
		gnome_vfs_uri_unref(log_uri);
	}
	wMUDMessageLevels[WMUD_LOG_DEBUG].dest = config->debug_log_dest;

	if (config->info_log_dest == WMUD_CONF_LOG_FILE)
	{
		GnomeVFSResult state;
		full_logfile_uri = gnome_vfs_get_uri_from_local_path(config->info_logfile);
		log_uri = gnome_vfs_uri_new(full_logfile_uri);
		g_free(full_logfile_uri);

		switch (gnome_vfs_open_uri(&(wMUDMessageLevels[WMUD_LOG_INFO].logfile), log_uri, GNOME_VFS_OPEN_READ | GNOME_VFS_OPEN_WRITE | GNOME_VFS_OPEN_RANDOM))
		{
			case GNOME_VFS_ERROR_NOT_FOUND:
				if ((state = gnome_vfs_create_uri(&(wMUDMessageLevels[WMUD_LOG_INFO].logfile), log_uri, GNOME_VFS_OPEN_WRITE, FALSE, 0640)) != GNOME_VFS_OK)
				{
					wmud_log_error("Unable to open info logfile (%s)", config->info_logfile);
					wmud_shutdown();
					return FALSE;
				}
				break;
			case GNOME_VFS_OK:
				break;
			default:
				wmud_log_error("Unprocessed return state from gnome_vfs_open_uri()");
				break;
		}
		gnome_vfs_uri_unref(log_uri);
	}
	wMUDMessageLevels[WMUD_LOG_INFO].dest = config->info_log_dest;

	if (config->warning_log_dest == WMUD_CONF_LOG_FILE)
	{
		GnomeVFSResult state;
		full_logfile_uri = gnome_vfs_get_uri_from_local_path(config->warning_logfile);
		log_uri = gnome_vfs_uri_new(full_logfile_uri);
		g_free(full_logfile_uri);

		switch (gnome_vfs_open_uri(&(wMUDMessageLevels[WMUD_LOG_WARN].logfile), log_uri, GNOME_VFS_OPEN_READ | GNOME_VFS_OPEN_WRITE | GNOME_VFS_OPEN_RANDOM))
		{
			case GNOME_VFS_ERROR_NOT_FOUND:
				if ((state = gnome_vfs_create_uri(&(wMUDMessageLevels[WMUD_LOG_WARN].logfile), log_uri, GNOME_VFS_OPEN_WRITE, FALSE, 0640)) != GNOME_VFS_OK)
				{
					wmud_log_error("Unable to open warning logfile (%s)", config->warning_logfile);
					wmud_shutdown();
					return FALSE;
				}
				break;
			case GNOME_VFS_OK:
				break;
			default:
				wmud_log_error("Unprocessed return state from gnome_vfs_open_uri()");
				break;
		}
		gnome_vfs_uri_unref(log_uri);
	}
	wMUDMessageLevels[WMUD_LOG_WARN].dest = config->warning_log_dest;

	if (config->error_log_dest == WMUD_CONF_LOG_FILE)
	{
		GnomeVFSResult state;
		full_logfile_uri = gnome_vfs_get_uri_from_local_path(config->error_logfile);
		log_uri = gnome_vfs_uri_new(full_logfile_uri);
		g_free(full_logfile_uri);

		switch (gnome_vfs_open_uri(&(wMUDMessageLevels[WMUD_LOG_ERROR].logfile), log_uri, GNOME_VFS_OPEN_READ | GNOME_VFS_OPEN_WRITE | GNOME_VFS_OPEN_RANDOM))
		{
			case GNOME_VFS_ERROR_NOT_FOUND:
				if ((state = gnome_vfs_create_uri(&(wMUDMessageLevels[WMUD_LOG_ERROR].logfile), log_uri, GNOME_VFS_OPEN_WRITE, FALSE, 0640)) != GNOME_VFS_OK)
				{
					wmud_log_error("Unable to open error logfile (%s)", config->error_logfile);
					wmud_shutdown();
					return FALSE;
				}
				break;
			case GNOME_VFS_OK:
				break;
			default:
				wmud_log_error("Unprocessed return state from gnome_vfs_open_uri()");
				break;
		}
		gnome_vfs_uri_unref(log_uri);
	}
	wMUDMessageLevels[WMUD_LOG_ERROR].dest = config->error_log_dest;

	if (
			(config->debug_log_dest == WMUD_CONF_LOG_SYSLOG)
			|| (config->info_log_dest == WMUD_CONF_LOG_SYSLOG)
			|| (config->warning_log_dest == WMUD_CONF_LOG_SYSLOG)
			|| (config->error_log_dest == WMUD_CONF_LOG_SYSLOG)
	)
	{
		openlog("wMUD", LOG_PID, LOG_USER);
	}

	return TRUE;
}

