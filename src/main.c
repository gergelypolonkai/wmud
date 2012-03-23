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
#include "networking.h"
#include "interpreter.h"
#include "db.h"
#include "players.h"

struct {
	char *file;
	int line;
} debug_context_loc = {NULL, 0};

GMainContext *game_context;
guint32 elapsed_seconds = 0;
guint32 elapsed_cycle = 0;
GRand *main_rand = NULL;
GQuark WMUD_CONFIG_ERROR = 0;
GQuark WMUD_DB_ERROR = 0;
guint port = 0;
gchar *database_file = NULL;
gchar *admin_email = NULL;

/* rl_sec_elapsed()
 *
 * This function keeps track of elapsed real-world time. It is inaccurate by
 * design, but it doesn't actually matter.
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

void
wmud_maintenance_check_new_players(wmudPlayer *player, gpointer user_data)
{
	if (player->cpassword == NULL)
	{
		gchar *pw,
		      *salt,
		      *cpw;
		GString *full_salt;

		pw = wmud_random_string(8);
		salt = wmud_random_string(8);
		full_salt = g_string_new("$1$");
		g_string_append(full_salt, salt);
		cpw = g_strdup(crypt(pw, full_salt->str));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Player %s has no"
				" password set", player->player_name);
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "New password will be %s", pw);
		player->cpassword = cpw;
		/* TODO: Send e-mail about the new password. Upon completion,
		 * set it in the database */

		g_free(pw);
		g_free(salt);
		g_string_free(full_salt, TRUE);
	}
}

gboolean
wmud_maintenance(gpointer user_data)
{
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Starting maintenance...");
	/* Run through the player list, and generate a random password for each
	 * newly registered player */
	g_slist_foreach(players, (GFunc)wmud_maintenance_check_new_players, NULL);

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Finished maintenance...");

	return TRUE;
}

#ifdef DEBUG
void
/* debug_context()
 *
 * This function keeps track of the code flow in some way. It can help with
 * debugging, as during a SIGSEGV or such signal this will print out the last
 * place of DebugContext in the code.
 */
debug_context(char *file, int line)
{
	if (debug_context_loc.file != NULL)
		g_free(debug_context_loc.file);

	debug_context_loc.file = g_strdup(file);
	debug_context_loc.line = line;
}
#define DebugContext debug_context(__FILE__, __LINE__)
#else
#define DebugContext
#endif

void
wmud_type_init(void)
{
	WMUD_CONFIG_ERROR = g_quark_from_string("wmud_config_error");
	WMUD_DB_ERROR = g_quark_from_string("wmud_db_error");
}

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

gpointer
game_thread_func(GMainLoop *game_loop)
{
	/* Run the game loop */
	g_main_loop_run(game_loop);

	return NULL;
}

gpointer
maint_thread_func(GMainLoop *maint_loop)
{
	g_main_loop_run(maint_loop);

	return NULL;
}

int
main(int argc, char **argv)
{
	GMainLoop *game_loop;
	GSource *timeout_source;
	guint timeout_id;
	GError *err = NULL;
	GThread *game_thread,
		*maint_thread;
	GMainContext *maint_context;
	GMainLoop *maint_loop;

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

	/* Create the maintenance context and main loop */
	maint_context = g_main_context_new();
	maint_loop = g_main_loop_new(maint_context, FALSE);

	/* Create the timeout source which keeps track of elapsed real-world
	 * time */
	timeout_source = g_timeout_source_new(1000);
	g_source_set_callback(timeout_source, rl_sec_elapsed, NULL, NULL);
	timeout_id = g_source_attach(timeout_source, game_context);
	g_source_unref(timeout_source);

	/* Create the timeout source which will do the maintenance tasks */
	timeout_source = g_timeout_source_new_seconds(3);
	g_source_set_callback(timeout_source, wmud_maintenance, NULL, NULL);
	timeout_id = g_source_attach(timeout_source, maint_context);
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

	g_clear_error(&err);
	maint_thread = g_thread_create((GThreadFunc)maint_thread_func, maint_loop, FALSE, &err);

	/* Initialize other threads here */

	g_thread_join(game_thread);

	return 0;
}

