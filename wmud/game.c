/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * game.c: Game Thread related functions
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

#include "main.h"
#include "game.h"

/**
 * SECTION:game-thread
 * @short_description: Game related functions
 * @title: The Game Thread
 *
 * The game thread is supposed to serve game client connections. Also,
 * maintaining the loaded game world is the objective ot this thread.
 *
 * This thread has to serve all the game clients after a connection is
 * estabilished. Player login, menu interaction and play are both the tasks of
 * this thread.
 *
 * The other main objective is to maintain the loaded world. Moving and
 * resetting the mobs, cleaning up areas and many other things belong here.
 */

/**
 * elapsed_ticks:
 *
 * the number of seconds elapsed since game boot. May be inaccurate, as it
 * simply gets updated by a timeout function which should run every second
 */
guint32 elapsed_ticks = 0;

/**
 * elapsed_cycle:
 *
 * yes, I'm optimistic. This counter is increased if, for some reason,
 * #elapsed_seconds reaches the maximum value
 */
guint32 elapsed_cycle = 0;

/**
 *
 * rl_sec_elapsed:
 * @user_data: non-used pointer to callback's user data
 *
 * Keeps track of elapsed real-world time. It is inaccurate by design, but it
 * doesn't actually matter.
 */
gboolean
rl_sec_elapsed(gpointer user_data)
{
	elapsed_ticks++;
	if (elapsed_ticks == G_MAXUINT32) {
		elapsed_ticks = 0;
		elapsed_cycle++;
	}

	if (elapsed_ticks % WMUD_HEARTBEAT_LENGTH == 0) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, "Heartbeat");
	}

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

gboolean
wmud_game_init(GThread **game_thread, GMainContext **game_context)
{
	GMainLoop *game_loop;
	GSource *timeout_source;
	GError *err = NULL;

	/* Create the game context and main loop */
	*game_context = g_main_context_new();
	game_loop = g_main_loop_new(*game_context, FALSE);

	/* Create the timeout source which keeps track of elapsed real-world
	 * time */
	timeout_source = g_timeout_source_new(WMUD_TICK_LENGTH);
	g_source_set_callback(timeout_source, rl_sec_elapsed, NULL, NULL);
	g_source_attach(timeout_source, *game_context);
	g_source_unref(timeout_source);

	g_clear_error(&err);
#if GLIB_CHECK_VERSION(2, 32, 0)
	*game_thread = g_thread_new("game", (GThreadFunc)game_thread_func, game_loop);
#else
	*game_thread = g_thread_create((GThreadFunc)game_thread_func, game_loop, TRUE, &err);
#endif

	return TRUE;
}
