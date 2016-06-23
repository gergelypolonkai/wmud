#include <glib.h>
#include <glib-object.h>
#include <gnet.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgnomevfs/gnome-vfs.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "wmud.h"
#include "logger.h"
#include "ansi.h"
#include "networking.h"
#include "world.h"
#include "configfile.h"
#include "chat.h"
#include "modules.h"

/* TODO: Make this configurable via configure, and/or countable based on sysconfdir */
#define DEFAULT_CONFIGFILE "/home/polesz/Projektek/wMUD/data/wmud.conf"

GSList *wmud_running_threads = NULL;
gchar *wmud_option_configfile = NULL;
wMUDConfiguration *wmud_configuration = NULL;
GMainLoop *wmud_main_loop = NULL;
gboolean wmud_is_running = FALSE;

pid_t wmud_pid;

static GOptionEntry option_entries[] = {
	{ "configfile", 'c', G_OPTION_FLAG_FILENAME, G_OPTION_ARG_FILENAME, &wmud_option_configfile, "Configuration file to parse instead of the default one", "filename" },
	{ NULL },
};

#if DEBUG
void
wmud_print_context(char *filename, int linenum)
{
	fprintf(stderr, "\x1b[34m\x1b[1m[DEBUG CONTEXT] %s:%d\x1b[0m\n", filename, linenum);
}
#endif

void
_fatal_signal_handler(int signum)
{
	fprintf(stderr, "Segmentation fault.\n");

	exit(1);
}

void
_terminating_signal_handler(int signum)
{
	switch (signum)
	{
		case SIGINT:
			wmud_shutdown();
			break;
		default:
			return;
	}
}

void
_wmud_shutdown_thread(gpointer data, gpointer user_data)
{
	wMUDThreadData *thread = (wMUDThreadData *)data;

	wmud_log_debug("Shutting down %s thread", thread->name);
	if (thread->main_loop)
	{
		Context;
		wmud_log_debug("Stopping %s thread's main loop", thread->name);
		g_main_loop_quit(thread->main_loop);
		while (1)
		{
			if (!thread->running)
			{
				break;
			}
			usleep(200);
		}
	}

	Context;
}

void
wmud_shutdown(void)
{
	wmud_log_info("Shutting down...");

	Context g_slist_foreach(wmud_running_threads, _wmud_shutdown_thread, NULL);

	if (wmud_main_loop)
	{
		Context g_main_loop_quit(wmud_main_loop);
		g_main_loop_unref(wmud_main_loop);
		wmud_main_loop = NULL;
	}

	wmud_is_running = FALSE;

	Context wmud_log_info("Shutdown complete");
}

void
wmud_parse_options(gint *argc, gchar ***argv)
{
	GOptionContext *options_context;
	GError *error = NULL;

	options_context = g_option_context_new("wMUD server");
	g_option_context_add_main_entries(options_context, option_entries, NULL);

	if (!g_option_context_parse(options_context, argc, argv, &error))
	{
		g_log(NULL, G_LOG_FLAG_FATAL | G_LOG_LEVEL_ERROR, "Failed parsing options: %s\n", error->message);
	}
}

static GThread *
_new_thread(GThreadFunc thread_func, GError **error, gchar *thread_name, gboolean fatal_if_fail)
{
	GThread *thread;
	wMUDThreadData *thread_data;

	/* Create and initialize thread data */
	thread_data = g_new0(wMUDThreadData, 1);

	thread_data->main_loop = NULL;
	thread_data->main_context = NULL;
	thread_data->thread = NULL;
	thread_data->name = g_strdup(thread_name);
	thread_data->running = FALSE;

	if ((thread = g_thread_create(thread_func, thread_data, FALSE, error)) == NULL)
	{
		wmud_log_error("Unable to start %s thread!", thread_name);

		if (fatal_if_fail)
		{
			wmud_shutdown();
		}
		g_free(thread_data);
	}
	else
	{
		thread_data->thread = thread;
		wmud_running_threads = g_slist_append(wmud_running_threads, thread_data);
	}

	return thread;
}

int
main(int argc, char **argv)
{
	GError *error;
	GThread *thread;
	struct sigaction signal_action;

	wmud_pid = getpid();

	wmud_log_info("wMUD v%s starting up", PACKAGE_VERSION);

	signal_action.sa_handler = _fatal_signal_handler;
	sigemptyset(&signal_action.sa_mask);
	signal_action.sa_flags = 0;
	sigaction(SIGSEGV, &signal_action, NULL);

	signal_action.sa_handler = _terminating_signal_handler;
	sigemptyset(&signal_action.sa_mask);
	signal_action.sa_flags = 0;
	sigaction(SIGINT, &signal_action, NULL);

	Context g_type_init();
	Context gnet_init();
	gnome_vfs_init();

	Context wmud_parse_options(&argc, &argv);

	Context;

	/* TODO: Make this configurable via configure, and/or countable based on sysconfdir */
	if (wmud_option_configfile == NULL)
	{
		wmud_option_configfile = g_strdup(DEFAULT_CONFIGFILE);
	}

	if (!wmud_configfile_read(wmud_option_configfile, &wmud_configuration, &error))
	{
		return 1;
	}
	if (wmud_configuration == NULL)
	{
		return 1;
	}
	Context g_free(wmud_option_configfile);
	Context wmud_fill_callable_table();
	Context wmud_load_config_modules(wmud_configuration);
	return 0;

	if (wmud_logger_init(wmud_configuration))
	{
		wmud_log_debug("Logger initialized");
		Context if (!g_thread_supported())
		{
			g_thread_init(NULL);
		}

		Context thread = _new_thread(wmud_networking_thread, &error, "networking", TRUE);
		Context thread = _new_thread(wmud_world_thread, &error, "world", TRUE);
		Context thread = _new_thread(wmud_chat_thread, &error, "chat", TRUE);
		Context;

		wmud_main_loop = g_main_loop_new(NULL, FALSE);

		wmud_is_running = TRUE;

		g_main_loop_run(wmud_main_loop);
	}

	wmud_configuration_free(&wmud_configuration);

	if (wmud_is_running)
	{
		wmud_shutdown();
	}

	Context gnome_vfs_shutdown();

	wmud_log_debug("Good bye...");

	return 0;
}

