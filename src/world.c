#include <glib.h>

#include "wmud-world.h"

#include "wmud.h"
#include "logger.h"

gpointer
wmud_world_thread(gpointer data)
{
	wMUDWorld *world;

	wMUDThreadData *thread_data = (wMUDThreadData *)data;

	wmud_log_info("Initializing world...");

	/* g_main_context_get_thread_default() is only available since GLib 2.22;
	 * use g_main_context_new() otherwise
	 */
#if (((GLIB_MAJOR_VERSION == 2) && (GLIB_MINOR_VERSION >= 22)) || (GLIB_MAJOR_VERSION > 2))
	thread_data->main_context = g_main_context_get_thread_default();
#else
	thread_data->main_context = g_main_context_new();
#endif
	thread_data->main_loop = g_main_loop_new(thread_data->main_context, FALSE);

	/* TODO: Do the real initialization here */

	world = wmud_world_new();

	wmud_log_info("World initialized");

	thread_data->running = TRUE;

	g_main_loop_run(thread_data->main_loop);

	g_main_loop_unref(thread_data->main_loop);

	thread_data->main_loop = NULL;

	wmud_log_info("World layer shutting down...");

	thread_data->running = FALSE;

	return NULL;
}

