#include <glib.h>
#include <gio/gio.h>

struct {
	char *file;
	int line;
} debug_context_loc = {NULL, 0};

gboolean
game_source_callback(gpointer user_data)
{
	return TRUE;
}

gboolean
accept_incoming(GSocketService *service, GSocketConnection *connection, GObject source_object, gpointer user_data)
{
	g_printf("New connection!\n");
	return TRUE;
}

gboolean
rl_sec_elapsed(gpointer user_data)
{
	g_printf("RL sec elapsed.\n");

	return TRUE;
}

void
debug_context(char *file, int line)
{
	if (debug_context_loc.file != NULL)
		g_free(debug_context_loc.file);

	debug_context_loc.file = g_strdup(file);
	debug_context_loc.line = line;
}
#define DebugContext debug_context(__FILE__, __LINE__)

int
main(int argc, char **argv)
{
	GMainContext *game_context;
	GMainLoop *game_loop;
	GSource *timeout_source,
		*game_net_source4 = NULL,
		*game_net_source6 = NULL;
	guint timeout_id;
	GError *err = NULL;
	GSocket *game_socket6,
		*game_socket4;
	gboolean need_ipv4_socket = TRUE;
	GSocketService *game_service;

	g_type_init();

	g_printf("Starting up...\n");

	game_context = g_main_context_new();
	game_loop = g_main_loop_new(game_context, FALSE);

	timeout_source = g_timeout_source_new(1000);
	g_source_set_callback(timeout_source, rl_sec_elapsed, NULL, NULL);
	timeout_id = g_source_attach(timeout_source, game_context);
	g_source_unref(timeout_source);

	game_service = g_threaded_socket_service_new(-1);

	/* The following snippet is borrowed from GLib 2.30's gsocketlistener.c
	 * code, to create the necessary sockets to listen on both IPv4 and
	 * IPv6 address */
	if ((game_socket6 = g_socket_new(G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &err)) != NULL)
	{
		GInetAddress *inet_address;
		GSocketAddress *address;
		gboolean result;

		inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);
		address = g_inet_socket_address_new(inet_address, 4000);
		g_object_unref(inet_address);

		g_socket_set_listen_backlog(game_socket6, 10);

		result = g_socket_bind(game_socket6, address, TRUE, &err)
			&& g_socket_listen(game_socket6, &err);

		g_object_unref(address);

		if (!result)
		{
			g_object_unref(game_socket6);
			g_printf("Unable to create listener IPv6 socket!\n");
			return 1;
		}

		if (g_socket_speaks_ipv4(game_socket6))
			need_ipv4_socket = FALSE;

		game_net_source6 = g_socket_create_source(game_socket6, G_IO_IN, NULL);
		g_socket_listener_add_socket(G_SOCKET_LISTENER(game_service), game_socket6, NULL, &err);
	}

	if (need_ipv4_socket)
	{
		if ((game_socket4 = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &err)) != NULL)
		{
			GInetAddress *inet_address;
			GSocketAddress *address;
			gboolean result;

			inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
			address = g_inet_socket_address_new(inet_address, 4000);
			g_object_unref(inet_address);

			g_socket_set_listen_backlog(game_socket4, 10);

			result = g_socket_bind(game_socket4, address, TRUE, &err)
				&& g_socket_listen(game_socket4, &err);

			g_object_unref(address);

			if (!result)
			{
				g_object_unref(game_socket4);
				if (!game_socket6)
					g_object_unref(game_socket6);

				g_printf("Unable to create listener IPv4 socket!\n");
				return 1;
			}

			game_net_source4 = g_socket_create_source(game_socket4, G_IO_IN, NULL);
			g_socket_listener_add_socket(G_SOCKET_LISTENER(game_service), game_socket4, NULL, &err);
		}
	}
	else
	{
		if (game_socket6 != NULL)
			g_clear_error(&err);
		else
			return 1;
	}

	if (game_net_source6)
	{
		g_source_set_callback(game_net_source6, game_source_callback, NULL, NULL);
		g_source_attach(game_net_source6, game_context);
	}
	if (game_net_source4)
	{
		g_source_set_callback(game_net_source4, game_source_callback, NULL, NULL);
		g_source_attach(game_net_source4, game_context);
	}

	g_socket_service_start(game_service);
	g_signal_connect(G_OBJECT(game_service), "incoming", G_CALLBACK(accept_incoming), NULL);

	g_printf("Startup finished\n");

	g_main_loop_run(game_loop);

	return 0;
}

