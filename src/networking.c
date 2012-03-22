#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include <stdarg.h>

#include "main.h"
#include "networking.h"
#include "interpreter.h"
#include "players.h"

#define MAX_RECV_LEN 1024

struct AcceptData {
	GMainContext *context;
	GSocketListener *listener;
};

GSList *clients;

void
client_close(wmudClient *client, gboolean send_goodbye)
{
	GError *err = NULL;
	if (send_goodbye)
	{
		/* TODO: Send some goodbye text */
	}

	g_print("Connection closed.\n");
	/* TODO: Error checking */
	g_socket_close(client->socket, &err);
	clients = g_slist_remove(clients, client);
	if (client->buffer)
		g_string_free(client->buffer, TRUE);
	g_free(client);
}

gboolean
client_callback(GSocket *client, GIOCondition condition, wmudClient *client_data)
{
	GError *err = NULL;

	if (condition & G_IO_HUP)
	{
		client_close(client_data, FALSE);
		return FALSE;
	}
	else if ((condition & G_IO_IN) || (condition & G_IO_PRI))
	{
		gssize len;
		gchar *buf2;
		gchar *buf = g_malloc0(sizeof(gchar) * (MAX_RECV_LEN + 1));

		/* TODO: Error checking */
		if ((len = g_socket_receive(client, buf, MAX_RECV_LEN, NULL, &err)) == 0)
		{
			g_free(buf);
			client_close(client_data, FALSE);
			return FALSE;
		}

		buf2 = buf;
		while (TRUE)
		{
			char *r = strchr((char *)buf2, '\r'),
			     *n = strchr((char *)buf2, '\n');

			if (r || n)
			{
				if ((r < n) && r)
				{
					if (client_data->buffer->len > 0)
						g_string_append_len(client_data->buffer, buf2, (r - buf2));
					else
						g_string_overwrite_len(client_data->buffer, 0, buf2, (r - buf2));
					buf2 = r;
				}
				else if (n)
				{
					if (client_data->buffer->len > 0)
						g_string_append_len(client_data->buffer, buf2, (n - buf2));
					else
						g_string_overwrite_len(client_data->buffer, 0, buf2, (n - buf2));
					buf2 = n;
				}

				switch (client_data->state)
				{
					case WMUD_CLIENT_STATE_FRESH:
						wmud_client_start_login(client_data);
						break;
					case WMUD_CLIENT_STATE_PASSWAIT:
						wmud_player_auth(client_data);
						break;
					case WMUD_CLIENT_STATE_MENU:
						//wmud_client_interpret_menu_command(client_data);
						break;
					case WMUD_CLIENT_STATE_INGAME:
						wmud_interpret_game_command(client_data);
						break;
					case WMUD_CLIENT_STATE_QUITWAIT:
						//wmud_interpret_quit_answer(client_data);
						break;
				}
				g_string_erase(client_data->buffer, 0, -1);

				for (; ((*buf2 == '\r') || (*buf2 == '\n')) && *buf2; buf2++);
				if (!*buf2)
					break;
			}
			else
			{
				if (client_data->buffer->len > 0)
					g_string_append(client_data->buffer, buf2);
				else
					g_string_overwrite(client_data->buffer, 0, buf2);

				break;
			}
		}

		g_free(buf);
	}

	return TRUE;
}

/* game_source_callback()
 *
 * This function is called whenever a new connection is available on the game socket
 */
gboolean
game_source_callback(GSocket *socket, GIOCondition condition, struct AcceptData *accept_data)
{
	GSocket *client_socket;
	GSource *client_source;
	GError *err = NULL;
	wmudClient *client_data;

	/* TODO: Error checking */
	client_socket = g_socket_listener_accept_socket(accept_data->listener, NULL, NULL, &err);

	client_data = g_new0(wmudClient, 1);
	client_data->socket = client_socket;
	client_data->buffer = g_string_new("");
	client_data->state = WMUD_CLIENT_STATE_FRESH;
	clients = g_slist_prepend(clients, client_data);

	client_source = g_socket_create_source(client_socket, G_IO_IN | G_IO_OUT | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL, NULL);
	g_source_set_callback(client_source, (GSourceFunc)client_callback, client_data, NULL);
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

	clients = NULL;
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

void
wmud_client_send(wmudClient *client, const gchar *fmt, ...)
{
	va_list ap;
	GString *buf = g_string_new("");

	va_start(ap, fmt);
	g_string_vprintf(buf, fmt, ap);
	va_end(ap);

	/* TODO: error checking */
	g_socket_send(client->socket, buf->str, buf->len, NULL, NULL);
	g_string_free(buf, TRUE);
}

void
wmud_client_start_login(wmudClient *client)
{
	g_print("Trying to login with playername '%s'\n", client->buffer->str);
}

