#include <glib.h>
#include <gio/gio.h>

#include "main.h"

#define MAX_RECV_LEN 1024

struct AcceptData {
	GMainContext *context;
	GSocketListener *listener;
};

gboolean
client_callback(GSocket *client, GIOCondition condition, struct AcceptData *accept_data)
{
	GError *err = NULL;

	if (condition & G_IO_HUP)
	{
		g_print("Connection closed.\n");
		/* TODO: Error checking */
		g_socket_close(client, &err);

		return FALSE;
	}
	else if ((condition & G_IO_IN) || (condition & G_IO_PRI))
	{
		gssize len;
		gchar *buf = g_malloc0(sizeof(gchar) * (MAX_RECV_LEN + 1));

		/* TODO: Error checking */
		if ((len = g_socket_receive(client, buf, MAX_RECV_LEN, NULL, &err)) == 0)
		{
			g_print("Connection closed.\n");
			/* TODO: Error checking */
			g_socket_close(client, &err);
			g_free(buf);
			return FALSE;
		}
		g_print("Client data arrived (%d bytes): \"%s\"\n", len, buf);
		g_free(buf);
	}

	return TRUE;
}

gboolean
game_source_callback(GSocket *socket, GIOCondition condition, struct AcceptData *accept_data)
{
	GSocket *client_socket;
	GSource *client_source;
	GError *err = NULL;

	/* TODO: Error checking */
	client_socket = g_socket_listener_accept_socket(accept_data->listener, NULL, NULL, &err);
	client_source = g_socket_create_source(client_socket, G_IO_IN | G_IO_OUT | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL, NULL);
	g_source_set_callback(client_source, (GSourceFunc)client_callback, client_socket, NULL);
	g_source_attach(client_source, accept_data->context);
	g_print("New connection.\n");

	return TRUE;
}

gboolean
wmud_networking_init(guint port_number)
{
	struct AcceptData *accept_data;
	GSocketListener *game_listener;
	gboolean need_ipv4_socket = TRUE;
	GSocket *game_socket6,
		*game_socket4;
	GError *err = NULL;
	GSource *game_net_source4 = NULL,
		*game_net_source6 = NULL;

	game_listener = g_socket_listener_new();

	/* The following snippet is borrowed from GLib 2.30's gsocketlistener.c
	 * code, to create the necessary sockets to listen on both IPv4 and
	 * IPv6 address */
	if ((game_socket6 = g_socket_new(G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &err)) != NULL)
	{
		GInetAddress *inet_address;
		GSocketAddress *address;
		gboolean result;

		inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);
		address = g_inet_socket_address_new(inet_address, port_number);
		g_object_unref(inet_address);

		g_socket_set_listen_backlog(game_socket6, 10);

		result = g_socket_bind(game_socket6, address, TRUE, &err)
			&& g_socket_listen(game_socket6, &err);

		g_object_unref(address);

		if (!result)
		{
			g_object_unref(game_socket6);
			g_print("Unable to create listener IPv6 socket!\n");
			return FALSE;
		}

		if (g_socket_speaks_ipv4(game_socket6))
			need_ipv4_socket = FALSE;

		game_net_source6 = g_socket_create_source(game_socket6, G_IO_IN, NULL);
		/* TODO: error checking */
		g_socket_listener_add_socket(game_listener, game_socket6, NULL, &err);
	}
	/* TODO: else { error checking } */

	if (need_ipv4_socket)
	{
		if ((game_socket4 = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &err)) != NULL)
		{
			GInetAddress *inet_address;
			GSocketAddress *address;
			gboolean result;

			inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
			address = g_inet_socket_address_new(inet_address, port_number);
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

				g_print("Unable to create listener IPv4 socket!\n");
				return FALSE;
			}

			game_net_source4 = g_socket_create_source(game_socket4, G_IO_IN, NULL);
			/* TODO: error checking */
			g_socket_listener_add_socket(game_listener, game_socket4, NULL, &err);
		}
		/* TODO: else { error checking } */
	}
	else
	{
		if (game_socket6 != NULL)
			g_clear_error(&err);
		else
			return FALSE;
	}

	accept_data = g_new(struct AcceptData, 1);
	accept_data->listener = game_listener;
	accept_data->context = game_context;

	if (game_net_source6)
	{
		g_source_set_callback(game_net_source6, (GSourceFunc)game_source_callback, (gpointer)accept_data, NULL);
		g_source_attach(game_net_source6, game_context);
	}
	if (game_net_source4)
	{
		g_source_set_callback(game_net_source4, (GSourceFunc)game_source_callback, (gpointer)accept_data, NULL);
		g_source_attach(game_net_source4, game_context);
	}

	return TRUE;
}

