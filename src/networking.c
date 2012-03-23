/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * networking.c: basic networking functions
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
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "main.h"
#include "networking.h"
#include "interpreter.h"
#include "players.h"
#include "db.h"

struct AcceptData {
	GMainContext *context;
	GSocketListener *listener;
};

GSList *clients;

void wmud_client_interpret_newplayer_email(wmudClient *client);
void wmud_client_interpret_newplayer_mailconfirm(wmudClient *client_data);

void
wmud_client_close(wmudClient *client, gboolean send_goodbye)
{
	if (send_goodbye)
		wmud_client_send(client, "\r\nHave a nice real-world day!\r\n\r\n");

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "Connection closed.");
	g_socket_close(client->socket, NULL);
	clients = g_slist_remove(clients, client);
	wmud_player_free(&(client->player));
	if (client->buffer)
		g_string_free(client->buffer, TRUE);
	g_source_destroy(client->socket_source);
	g_free(client);
}

static gboolean
wmud_client_callback(GSocket *client, GIOCondition condition, wmudClient *client_data)
{
	GError *err = NULL;

	if (condition & G_IO_HUP)
	{
		wmud_client_close(client_data, FALSE);
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
			wmud_client_close(client_data, FALSE);

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
						if (*(client_data->buffer->str))
							wmud_client_start_login(client_data);
						break;
					case WMUD_CLIENT_STATE_PASSWAIT:
						if (*(client_data->buffer->str))
						{
							if (wmud_player_auth(client_data))
							{
								wmud_client_send(client_data, "%c%c%cLogin"
										" successful.\r\n", TELNET_IAC,
										TELNET_WILL, TELNET_ECHO);
								client_data->authenticated = TRUE;
								/* TODO: Send fail count if non-zero */
								client_data->state = WMUD_CLIENT_STATE_MENU;
							}
							else
							{
								wmud_client_send(client_data, "%c%c%cThis"
										" password doesn't seem to be valid."
										" Let's try it again...\r\nBy what"
										" name would you like to be called? ",
										TELNET_IAC, TELNET_WILL, TELNET_ECHO);
								client_data->state = WMUD_CLIENT_STATE_FRESH;
								client_data->login_try_count++;
								if (client_data->login_try_count == 3)
								{
									wmud_client_send(client_data, "You are"
											" trying these bad passwords for"
											" too many times. Please stop"
											" that!\r\n");
									wmud_client_close(client_data, TRUE);
									/* TODO: Increase IP fail count, and ban IP if it's too high */
								}
								/* TODO: Increase and save player fail count */
								client_data->player = NULL;
							}
						}
						else
						{
							wmud_client_send(client_data, "\r\nEmpty passwords are"
									" not valid.\r\nTry again: ");
						}
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
					case WMUD_CLIENT_STATE_NEWCHAR:
						wmud_client_interpret_newplayer_answer(client_data);
						break;
					case WMUD_CLIENT_STATE_REGISTERING:
						wmud_client_interpret_newplayer_email(client_data);
						break;
					case WMUD_CLIENT_STATE_REGEMAIL_CONFIRM:
						wmud_client_interpret_newplayer_mailconfirm(client_data);
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
	client_data->socket_source = client_source;
	g_source_set_callback(client_source, (GSourceFunc)wmud_client_callback, client_data, NULL);
	g_source_attach(client_source, accept_data->context);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "New connection.");
	wmud_client_send(client_data, "By what name shall we call you? ");

	return TRUE;
}

gboolean
wmud_networking_init(guint port_number, GError **err)
{
	struct AcceptData *accept_data;
	GSocketListener *game_listener;
	gboolean need_ipv4_socket = TRUE;
	GSocket *game_socket6,
		*game_socket4;
	GError *in_err = NULL;
	GSource *game_net_source4 = NULL,
		*game_net_source6 = NULL;

	clients = NULL;
	game_listener = g_socket_listener_new();

	/* The following snippet is borrowed from GLib 2.30's gsocketlistener.c
	 * code, to create the necessary sockets to listen on both IPv4 and
	 * IPv6 address */
	if ((game_socket6 = g_socket_new(G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &in_err)) != NULL)
	{
		GInetAddress *inet_address;
		GSocketAddress *address;
		gboolean result;

		inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);
		address = g_inet_socket_address_new(inet_address, port_number);
		g_object_unref(inet_address);

		g_socket_set_listen_backlog(game_socket6, 10);

		result = g_socket_bind(game_socket6, address, TRUE, &in_err)
			&& g_socket_listen(game_socket6, &in_err);

		g_object_unref(address);

		if (!result)
		{
			g_object_unref(game_socket6);
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "Unable to create listener IPv6 socket");
			return FALSE;
		}

		if (g_socket_speaks_ipv4(game_socket6))
			need_ipv4_socket = FALSE;

		game_net_source6 = g_socket_create_source(game_socket6, G_IO_IN, NULL);
		/* TODO: error checking */
		g_socket_listener_add_socket(game_listener, game_socket6, NULL, &in_err);
	}
	/* TODO: else { error checking } */

	if (need_ipv4_socket)
	{
		if ((game_socket4 = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, &in_err)) != NULL)
		{
			GInetAddress *inet_address;
			GSocketAddress *address;
			gboolean result;

			inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
			address = g_inet_socket_address_new(inet_address, port_number);
			g_object_unref(inet_address);

			g_socket_set_listen_backlog(game_socket4, 10);

			result = g_socket_bind(game_socket4, address, TRUE, &in_err)
				&& g_socket_listen(game_socket4, &in_err);

			g_object_unref(address);

			if (!result)
			{
				g_object_unref(game_socket4);
				if (!game_socket6)
					g_object_unref(game_socket6);

				g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "Unable to create listener IPv4 socket!\n");
				return FALSE;
			}

			game_net_source4 = g_socket_create_source(game_socket4, G_IO_IN, NULL);
			/* TODO: error checking */
			g_socket_listener_add_socket(game_listener, game_socket4, NULL, &in_err);
		}
		/* TODO: else { error checking } */
	}
	else
	{
		if (game_socket6 != NULL)
			g_clear_error(&in_err);
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
	wmudPlayer *player;

	if ((player = wmud_player_exists(client->buffer->str)) != NULL)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Trying to login with playername '%s'\n", client->buffer->str);
		if (player->cpassword == NULL)
		{
			wmud_client_send(client, "Your registration is not finished yet.\r\n");
			wmud_client_close(client, TRUE);
		}
		else
		{
			client->state = WMUD_CLIENT_STATE_PASSWAIT;
			client->player = player;
			wmud_client_send(client, "Please provide us your password: %c%c%c", TELNET_IAC, TELNET_WONT, TELNET_ECHO);
		}
	}
	else
	{
		client->player = g_new0(wmudPlayer, 1);
		client->player->player_name = g_strdup(client->buffer->str);
		client->state = WMUD_CLIENT_STATE_NEWCHAR;
		wmud_client_send(client, "Is %s new to this game? [Y/N] ", client->buffer->str);
	}
}

void
wmud_client_interpret_newplayer_answer(wmudClient *client)
{
	if (g_ascii_strcasecmp(client->buffer->str, "n") == 0)
	{
		wmud_client_send(client, "What is your player-name, then? ");
		client->state = WMUD_CLIENT_STATE_FRESH;
	}
	else if (g_ascii_strcasecmp(client->buffer->str, "y") == 0)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Creating new player\n");
		wmud_client_send(client, "Welcome to this MUD!\r\nPlease enter your e-mail address: ");
		client->state = WMUD_CLIENT_STATE_REGISTERING;
	}
	else
	{
		wmud_client_send(client, "Sorry, but for this question I only understand 'Y' or 'N'.\r\nIs %s a new player here? [Y/N] ", client->player->player_name);
	}
}

void
wmud_client_interpret_newplayer_email(wmudClient *client)
{
	/* TODO: Error checking */
	GRegex *email_regex = g_regex_new("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}$", G_REGEX_CASELESS, 0, NULL);

	if (!*(client->buffer->str))
	{
		if (client->bademail)
		{
			wmud_client_close(client, TRUE);
		}
	}

	if (g_regex_match(email_regex, client->buffer->str, 0, NULL))
	{
		client->player->email = g_strdup(client->buffer->str);
		client->state = WMUD_CLIENT_STATE_REGEMAIL_CONFIRM;
		wmud_client_send(client, "It seems to be a valid address to me, but could you write it again? ");
	}
	else
	{
		wmud_client_send(client, "\r\nSorry, but this e-mail address doesn't seem to be valid to me.\r\n\r\nIf you think this is a valid address, simply press enter to quit, and send an e-mail to %s from that address, so we can fix our e-mail validation code.\r\n\r\nIf you just mistyped your address, type it now: ", admin_email);
		if (*(client->buffer->str))
			client->bademail = TRUE;
	}
}

void
wmud_client_interpret_newplayer_mailconfirm(wmudClient *client)
{
	GError *err = NULL;

	if (g_ascii_strcasecmp(client->player->email, client->buffer->str) == 0)
	{
		if (wmud_db_save_player(client->player, &err))
		{
			wmud_client_send(client, "Good. We will generate the password for this player name, and send it to you\r\nvia e-mail. Please come back to us, if you get that code, so you can log\r\nin.\r\n");
			players = g_slist_prepend(players, wmud_player_dup(client->player));
		}
		else
		{
			g_critical("wmud_db_save_player() error: %s", err->message);
			wmud_client_send(client, "There was an error during the database update. Please try again later!\r\n");
		}
		wmud_client_close(client, TRUE);
	}
	else
	{
		g_free(client->player->email);
		client->player->email = NULL;

		wmud_client_send(client, "This is not the same as you entered before.\r\nLet's just try it again: ");
		client->state = WMUD_CLIENT_STATE_REGISTERING;
	}
}

