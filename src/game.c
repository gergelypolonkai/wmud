#include <glib.h>

#include "main.h"
#include "game.h"

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
	timeout_source = g_timeout_source_new(1000);
	g_source_set_callback(timeout_source, rl_sec_elapsed, NULL, NULL);
	g_source_attach(timeout_source, *game_context);
	g_source_unref(timeout_source);

	g_clear_error(&err);
	*game_thread = g_thread_create((GThreadFunc)game_thread_func, game_loop, TRUE, &err);

	return TRUE;
}

