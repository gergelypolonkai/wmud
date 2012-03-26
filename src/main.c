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

#include "wmud-types.h"
#include "main.h"
#include "game-networking.h"
#include "interpreter.h"
#include "db.h"
#include "players.h"
#include "maintenance.h"
#include "game.h"
#include "configuration.h"

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
 * main:
 * @argc: The number of arguments on the command line
 * @argv: The command line arguments themselves
 *
 * The Main Function (TM)
 */
int
main(int argc, char **argv)
{
	GError *err = NULL;
	GThread *game_thread;
	GMainContext *game_context;

	/* Initialize the thread and type system */
	g_thread_init(NULL);
	g_type_init();
	wmud_type_init();

	/* TODO: Command line parsing */
	/* TODO: Create signal handlers! */

	if (!wmud_config_init(&active_config, &err))
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
	wmud_db_players_load(&err);

	/* Initialization ends here */

	wmud_game_init(&game_thread, &game_context);

	g_clear_error(&err);
	if (!wmud_networking_init(active_config->port, game_context, &err))
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

	wmud_maintenance_init();

	/* Initialize other threads here */

	g_thread_join(game_thread);

	return 0;
}

