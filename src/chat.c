#include <glib.h>

#include "wmud.h"
#include "logger.h"

gpointer
wmud_chat_thread(gpointer data)
{
	wMUDThreadData *thread_data = (wMUDThreadData *)data;

	Context wmud_log_info("Initializing chat layer...");
	/* g_main_context_get_thread_default() is only available since GLib 2.22;
	 * use g_main_context_new() otherwise
	 */
#if (((GLIB_MAJOR_VERSION == 2) && (GLIB_MINOR_VERSION >= 22)) || (GLIB_MAJOR_VERSION > 2))
	thread_data->main_context = g_main_context_get_thread_default();
#else
	thread_data->main_context = g_main_context_new();
#endif
	thread_data->main_loop = g_main_loop_new(thread_data->main_context, FALSE);

	/* Do the real initialization work here */

	/* End of initialization */

	wmud_log_info("Chat layer initialized");

	Context;
	thread_data->running = TRUE;
	g_main_loop_run(thread_data->main_loop);

	wmud_log_info("Chat layer shutting down");

	g_main_loop_unref(thread_data->main_loop);
	thread_data->main_loop = NULL;

	thread_data->running = FALSE;

	Context;

	return NULL;
}
