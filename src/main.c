#include <glib.h>
#include <gio/gio.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main.h"
#include "networking.h"

#define MAX_RECV_LEN 1024

struct {
	char *file;
	int line;
} debug_context_loc = {NULL, 0};

GMainContext *game_context;
guint32 elapsed_seconds = 0;
guint32 elapsed_cycle = 0;

gboolean
rl_sec_elapsed(gpointer user_data)
{
	elapsed_seconds++;
	if (elapsed_seconds == G_MAXUINT32)
	{
		elapsed_seconds = 0;
		elapsed_cycle++;
	}
	g_print("%ld RL sec elapsed.\n", elapsed_seconds);

	return TRUE;
}

#ifdef DEBUG
void
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

int
main(int argc, char **argv)
{
	GMainLoop *game_loop;
	GSource *timeout_source;
	guint timeout_id;
	GSocketListener *game_listener;

	g_thread_init(NULL);
	g_type_init();

	g_print("Starting up...\n");

	game_context = g_main_context_new();
	game_loop = g_main_loop_new(game_context, FALSE);

	timeout_source = g_timeout_source_new(1000);
	g_source_set_callback(timeout_source, rl_sec_elapsed, NULL, NULL);
	timeout_id = g_source_attach(timeout_source, game_context);
	g_source_unref(timeout_source);

	game_listener = g_socket_listener_new();

	wmud_networking_init(4000);

	g_print("Startup finished\n");

	g_main_loop_run(game_loop);

	return 0;
}

