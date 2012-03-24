/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * main.c: main and uncategorized functions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gio/gio.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "wmud_types.h"
#include "main.h"
#include "game-networking.h"
#include "interpreter.h"
#include "db.h"
#include "players.h"
#include "maintenance.h"

/**
 * SECTION:utils
 * @short_description: Utilities and uncategorized functions
 * @title: Utility and uncategorized functions
 *
 */

/**
 * debug_context_loc:
 *
 * This variable holds the location of the last context marker
 */
struct {
	char *file;
	int line;
} debug_context_loc = {NULL, 0};

/**
 * game_context:
 *
 * the game thread's main context
 */
GMainContext *game_context;
/**
 * elapsed_seconds:
 *
 * the number of seconds elapsed since game boot. May be inaccurate, as it
 * simply gets updated by a timeout function which should run every second
 */
guint32 elapsed_seconds = 0;
/**
 * elapsed_cycle:
 *
 * yes, I'm optimistic. This counter is increased if, for some reason,
 * #elapsed_seconds reaches the maximum value
 */
guint32 elapsed_cycle = 0;
/**
 * main_rand:
 *
 * the main random generator
 */
GRand *main_rand = NULL;
/**
 * WMUD_CONFIG_ERROR:
 *
 * the GQuark for the config error GError
 */
GQuark WMUD_CONFIG_ERROR = 0;
/**
 * WMUD_DB_ERROR:
 *
 * the GQuark for the database error GError
 */
GQuark WMUD_DB_ERROR = 0;
/**
 * port:
 *
 * the port number to listen on
 */
guint port = 0;
/**
 * database_file:
 *
 * the filename of the world database
 */
gchar *database_file = NULL;
/**
 * admin_email:
 *
 * e-mail address of the MUD's administrator
 */
gchar *admin_email = NULL;

/**
 * rl_sec_elapsed:
 * @user_data: non-used pointer to callback's user data
 *
 * Keeps track of elapsed real-world time. It is inaccurate by design, but it
 * doesn't actually matter.
 */
gboolean
rl_sec_elapsed(gpointer user_data)
{
	elapsed_seconds++;
	if (elapsed_seconds == G_MAXUINT32)
	{
		elapsed_seconds = 0;
		elapsed_cycle++;
	}

	if (elapsed_seconds % 30 == 0)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, "Heartbeat");
	}

	return TRUE;
}

/**
 * wmud_random_string:
 * @len: the desired length of the generated random string
 *
 * Generates a random string of %len characters.
 */
gchar *
wmud_random_string(gint len)
{
	gchar *ret = g_malloc0(len + 1);
	gint i;

	for (i = 0; i < len; i++)
	{
		gchar c = 0;
		/* Include only printable characters, but exclude $ because of
		 * salt generation */
		while (!g_ascii_isprint(c) || (c == '$'))
			c = random_number(1, 127);

		ret[i] = c;
	}

	return ret;
}

#ifdef DEBUG
void
/**
 * debug_context:
 * @file: the source file name, where the context marker was found
 * @line: the line number where the context marker was found
 *
 * This function keeps track of the code flow in some way. It can help with
 * debugging, as during a SIGSEGV or such signal this will print out the last
 * place of DebugContext in the code.
 *
 * THIS FUNCTION SHOULD NEVER BE CALLED DIRECTLY!
 */
debug_context(char *file, int line)
{
	if (debug_context_loc.file != NULL)
		g_free(debug_context_loc.file);

	debug_context_loc.file = g_strdup(file);
	debug_context_loc.line = line;
}
/**
 * DebugContext:
 *
 * Marks the current line of the source file with a context marker. Deadly
 * signals should print the place of the last marker.
 */
#define DebugContext debug_context(__FILE__, __LINE__)
#else
#define DebugContext
#endif

/**
 * wmud_type_init:
 *
 * Initializes the wMUD types.
 */
void
wmud_type_init(void)
{
	WMUD_CONFIG_ERROR = g_quark_from_string("wmud_config_error");
	WMUD_DB_ERROR = g_quark_from_string("wmud_db_error");
}

/**
 * wmud_config_init:
 * @err: The GError in which the config handling status should be returned
 *
 * Parses the default configuration file, and sets different variables
 * according to it.
 *
 * Return value: %TRUE if parsing was successful. %FALSE otherwise.
 */
gboolean
wmud_config_init(GError **err)
{
	GString *config_file = g_string_new(WMUD_CONFDIR);
	GKeyFile *config;
	GError *in_err = NULL;

	g_string_append(config_file, "/wmud.conf");

	config = g_key_file_new();
	/* TODO: Error checking */
	g_key_file_load_from_file(config, config_file->str, 0, &in_err);

	if (!g_key_file_has_group(config, "global"))
	{
		g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_NOGLOBAL, "Config file (%s) does not contain a [global] group", config_file->str);
		g_key_file_free(config);
		g_string_free(config_file, TRUE);

		return FALSE;
	}

	g_clear_error(&in_err);
	port = g_key_file_get_integer(config, "global", "port", &in_err);
	if (in_err)
	{
		if (g_error_matches(in_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND))
		{
			port = DEFAULT_PORT;
		}
		else if (g_error_matches(in_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
		{
			g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_BADPORT, "Config file (%s) contains an invalid port number", config_file->str);
			g_key_file_free(config);
			g_string_free(config_file, TRUE);
			port = 0;

			return FALSE;
		}

		return FALSE;
	}

	g_clear_error(&in_err);
	database_file = g_key_file_get_string(config, "global", "world file", &in_err);
	if (in_err)
	{
		if (g_error_matches(in_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND))
		{
			g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_NOWORLD, "Config file (%s) does not contain a world file path", config_file->str);
			g_key_file_free(config);
			g_string_free(config_file, TRUE);
			database_file = NULL;

			return FALSE;
		}
	}

	g_clear_error(&in_err);
	admin_email = g_key_file_get_string(config, "global", "admin email", &in_err);
	if (in_err)
	{
		if (g_error_matches(in_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND))
		{
			g_set_error(err, WMUD_CONFIG_ERROR, WMUD_CONFIG_ERROR_NOEMAIL, "Config file (%s) does not contain an admin e-mail address", config_file->str);
			g_key_file_free(config);
			g_string_free(config_file, TRUE);
			admin_email = NULL;

			return FALSE;
		}
	}

	g_key_file_free(config);
	g_string_free(config_file, TRUE);

	return TRUE;
}

/**
 * game_thread_func:
 * @game_loop: the main loop to be associated with the game thread
 *
 * The game thread's main function.
 *
 * Return value: This function always returns %NULL.
 */
gpointer
game_thread_func(GMainLoop *game_loop)
{
	/* Run the game loop */
	g_main_loop_run(game_loop);

	return NULL;
}

/**
 * main:
 * @argc: The number of arguments on the command line
 * @argv: The command line arguments themselves
 *
 * The Main Function (TM)
 */
int
main(int argc, char **argv)
{
	GMainLoop *game_loop;
	GSource *timeout_source;
	GError *err = NULL;
	GThread *game_thread;

	/* Initialize the thread and type system */
	g_thread_init(NULL);
	g_type_init();
	wmud_type_init();

	/* TODO: Command line parsing */

	/* Initialize random number generator */
	main_rand = g_rand_new();

	/* Create the game context and main loop */
	game_context = g_main_context_new();
	game_loop = g_main_loop_new(game_context, FALSE);

	/* Create the timeout source which keeps track of elapsed real-world
	 * time */
	timeout_source = g_timeout_source_new(1000);
	g_source_set_callback(timeout_source, rl_sec_elapsed, NULL, NULL);
	g_source_attach(timeout_source, game_context);
	g_source_unref(timeout_source);

	/* TODO: Create signal handlers! */

	if (!wmud_config_init(&err))
	{
		if (err)
		{
			g_critical("Config file parsing error: %s", err->message);
		}
		else
		{
			g_critical("Config file parsing error!");
		}

		return 1;
	}

	g_assert(port != 0);
	g_assert(database_file != NULL);

	g_clear_error(&err);
	if (!wmud_db_init(&err))
	{
		if (err)
		{
			g_critical("Database initialization error: %s", err->message);
		}
		else
		{
			g_critical("Database initialization error!");
		}

		return 1;
	}
	g_clear_error(&err);
	if (!wmud_networking_init(port, &err))
	{
		if (err)
		{
			g_critical("Database initialization error: %s", err->message);
		}
		else
		{
			g_critical("Database initialization error!");
		}

		return 1;
	}

	g_clear_error(&err);
	wmud_db_players_load(&err);

	/* Initialization ends here */

	g_clear_error(&err);
	game_thread = g_thread_create((GThreadFunc)game_thread_func, game_loop, TRUE, &err);

	wmud_maintenance_init();

	/* Initialize other threads here */

	g_thread_join(game_thread);

	return 0;
}

