/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * game-networking.c: basic networking functions
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
#include "game-networking.h"
#include "interpreter.h"
#include "players.h"
#include "db.h"
#include "configuration.h"
#include "menu.h"

/**
 * SECTION:game-networking
 * @short_description: Game related networking code
 *
 * Functions to handle game connections. Each connection has a #GSocket and a
 * #GSource, associated with the game thread's #GMainContext.
 */

struct AcceptData {
	GMainContext *context;
	GSocketListener *listener;
};

/**
 * clients:
 * The full #GSList of the currently connected #wmudClient structs.
 */
GSList *clients = NULL;

/**
 * game_menu:
 * The list of menu items to display after a successful login
 */
static GSList *game_menu = NULL;

static GRegex *email_regex = NULL;

void wmud_client_interpret_newplayer_email(wmudClient *client);
void wmud_client_interpret_newplayer_mailconfirm(wmudClient *client);

/**
 * wmud_client_close:
 * @client: the client whose connection should be dropped
 * @send_goodbye: if set to %TRUE, we will send a nice good-bye message to the
 *                client before dropping the connection
 *
 * Closes a client connection. If send_goodbye is set to %TRUE, a good-bye
 * message will be sent to the client.
 */
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

void
send_menu_item(wmudMenu *item, wmudClient *client)
{
	/* TODO: Send ANSI menu item only to ANSI players! */
	wmud_client_send(client, "%s\r\n", item->display_text_ansi);
}

/**
 * wmud_client_callback:
 * @client: the socket of the client on which the data arrived
 * @condition: the condition available on the client socket
 * @client: the wmudClient structure of the client
 *
 * Processes incoming client data, and client hangup
 *
 * Return value: %FALSE if the connection is closed after this call, %TRUE
 * otherwise
 */
static gboolean
wmud_client_callback(GSocket *client_socket, GIOCondition condition, wmudClient *client)
{
	GError *err = NULL;

	if (condition & G_IO_HUP)
	{
		wmud_client_close(client, FALSE);
		return FALSE;
	}
	else if ((condition & G_IO_IN) || (condition & G_IO_PRI))
	{
		gssize len;
		gchar *buf2;
		gchar *buf = g_malloc0(sizeof(gchar) * (MAX_RECV_LEN + 1));

		if ((len = g_socket_receive(client_socket, buf, MAX_RECV_LEN, NULL, &err)) == 0)
		{
			g_free(buf);
			wmud_client_close(client, FALSE);

			return FALSE;
		}

		buf2 = buf;
		while (TRUE)
		{
			char *r = strchr((char *)buf2, '\r'),
			     *n = strchr((char *)buf2, '\n');

			if (r || n)
			{
				gint i,
				     sloc = -1;

				if ((r < n) && r)
				{
					if (client->buffer->len > 0)
						g_string_append_len(client->buffer, buf2, (r - buf2));
					else
						g_string_overwrite_len(client->buffer, 0, buf2, (r - buf2));
					buf2 = r;
				}
				else if (n)
				{
					if (client->buffer->len > 0)
						g_string_append_len(client->buffer, buf2, (n - buf2));
					else
						g_string_overwrite_len(client->buffer, 0, buf2, (n - buf2));
					buf2 = n;
				}

				/* Remove telnet codes from the string */
				for (i = 0; i < client->buffer->len; i++)
				{
					guchar c = (client->buffer->str)[i];

					if ((c >= 240) || (c == 1))
					{
						if (sloc == -1)
							sloc = i;
					}
					else
					{
						if (sloc != -1)
						{
							g_string_erase(client->buffer, sloc, i - sloc);
							sloc = -1;
						}
					}
				}
				if (sloc != -1)
				{
					g_string_erase(client->buffer, sloc, -1);
				}

				switch (client->state)
				{
					case WMUD_CLIENT_STATE_FRESH:
						if (*(client->buffer->str))
						{
							wmudPlayer *player;

							if ((player = wmud_player_exists(client->buffer->str)) != NULL)
							{
								g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Trying to login with playername '%s'", client->buffer->str);
								if (player->cpassword == NULL)
								{
									wmud_client_send(client, "Your registration is not finished yet.\r\n");
									wmud_client_close(client, TRUE);
								}
								else
								{
									client->state = WMUD_CLIENT_STATE_PASSWAIT;
									client->player = player;
									wmud_client_send(client, "Please provide us your password: %c%c%c", TELNET_IAC, TELNET_WILL, TELNET_ECHO);
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
						break;
					case WMUD_CLIENT_STATE_PASSWAIT:
						if (*(client->buffer->str))
						{
							if (wmud_player_auth(client))
							{
								wmud_client_send(client, "%c%c%c\r\nLogin"
										" successful.\r\n", TELNET_IAC,
										TELNET_WONT, TELNET_ECHO);
								client->authenticated = TRUE;
								if (client->player->fail_count > 0)
								{
									wmud_client_send(client, "There %s %d failed login attempt%s with your account since your last visit\r\n", (client->player->fail_count == 1) ? "was" : "were", client->player->fail_count, (client->player->fail_count == 1) ? "" : "s");
								}
								/* TODO: send MOTD */
								/* TODO: send menu items */
								g_slist_foreach(game_menu, (GFunc)send_menu_item, client);
								client->state = WMUD_CLIENT_STATE_MENU;
								/* TODO: send menu prologue */
							}
							else
							{
								wmud_client_send(client, "%c%c%cThis"
										" password doesn't seem to be valid."
										" Let's try it again...\r\nBy what"
										" name would you like to be called? ",
										TELNET_IAC, TELNET_WONT, TELNET_ECHO);
								client->state = WMUD_CLIENT_STATE_FRESH;
								client->player->fail_count++;
								client->login_try_count++;
								if (client->login_try_count == 3)
								{
									wmud_client_send(client, "You are"
											" trying these bad passwords for"
											" too many times. Please stop"
											" that!\r\n");
									wmud_client_close(client, TRUE);
									/* TODO: Increase IP fail count, and ban IP if it's too high */
								}
								/* TODO: Increase and save login fail count */
								client->player = NULL;
							}
						}
						else
						{
							wmud_client_send(client, "\r\nEmpty passwords are"
									" not valid.\r\nTry again: ");
						}
						break;
					case WMUD_CLIENT_STATE_MENU:
						{
							gchar *menu_command;
							
							if ((menu_command = wmud_menu_get_command_by_menuchar(*(client->buffer->str), game_menu)) != NULL)
							{
								wmud_menu_execute_command(client, menu_command);
							}
							else
							{
								wmud_client_send(client, "Unknown menu command.\r\n");
							}
						}
						break;
					case WMUD_CLIENT_STATE_INGAME:
						wmud_interpret_game_command(client);
						break;
					case WMUD_CLIENT_STATE_QUITWAIT:
						//wmud_interpret_quit_answer(client);
						break;
					case WMUD_CLIENT_STATE_NEWCHAR:
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
						break;
					case WMUD_CLIENT_STATE_REGISTERING:
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
							wmud_client_send(client, "\r\nSorry, but this e-mail address doesn't seem to be valid to me.\r\n\r\nIf you think this is a valid address, simply press enter to quit, and send an e-mail to %s from that address, so we can fix our e-mail validation code.\r\n\r\nIf you just mistyped your address, type it now: ", active_config->admin_email);
							if (*(client->buffer->str))
								client->bademail = TRUE;
						}
						break;
					case WMUD_CLIENT_STATE_REGEMAIL_CONFIRM:
						if (g_ascii_strcasecmp(client->player->email, client->buffer->str) == 0)
						{
							g_clear_error(&err);
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
						break;
				}
				g_string_erase(client->buffer, 0, -1);

				for (; ((*buf2 == '\r') || (*buf2 == '\n')) && *buf2; buf2++);
				if (!*buf2)
					break;
			}
			else
			{
				if (client->buffer->len > 0)
					g_string_append(client->buffer, buf2);
				else
					g_string_overwrite(client->buffer, 0, buf2);

				break;
			}
		}

		g_free(buf);
	}

	return TRUE;
}

/**
 * game_source_callback:
 * @socket: the listener socket on which the new connection arrived
 * @condition: not used
 * @accept_data: the AcceptData structure of the game listener
 *
 * Callback function to be called when a new connection is available on the
 * game listener socket.
 *
 * Return value: this function always returns %TRUE
 */
gboolean
game_source_callback(GSocket *socket, GIOCondition condition, struct AcceptData *accept_data)
{
	GSocket *client_socket;
	GSource *client_source;
	GError *err = NULL;
	wmudClient *client;
	GSocketAddress *remote_addr;

	/* This function should never return an error. If so, it is a huge bug,
	 * and will trigger a higher level error. */
	client_socket = g_socket_listener_accept_socket(accept_data->listener, NULL, NULL, &err);

	client = g_new0(wmudClient, 1);
	client->socket = client_socket;
	client->buffer = g_string_new("");
	client->state = WMUD_CLIENT_STATE_FRESH;
	clients = g_slist_prepend(clients, client);

	client_source = g_socket_create_source(client_socket, G_IO_IN | G_IO_OUT | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL, NULL);
	client->socket_source = client_source;
	g_source_set_callback(client_source, (GSourceFunc)wmud_client_callback, client, NULL);
	g_source_attach(client_source, accept_data->context);

	g_clear_error(&err);
	if ((remote_addr = g_socket_get_remote_address(client_socket, &err)) != NULL)
	{
		GInetAddress *addr;
		gchar *ip_addr;

		addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(remote_addr));
		ip_addr = g_inet_address_to_string(addr);
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "New game connection from %s", ip_addr);
		g_free(ip_addr);
		g_object_unref(addr);
		g_object_unref(remote_addr);
	}
	else
	{
		if (err)
		{
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "New game connection. The remote address is unknown. This is a bug. Message from upper level: %s", err->message);
		}
		else
		{
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "New game connection. The remote address is unknown. This is a bug.");
		}
	}
	g_clear_error(&err);
	wmud_client_send(client, "By what name shall we call you? ");

	return TRUE;
}

/**
 * wmud_networking_init:
 * @port_number: the port number on which the game listener should listen
 * @game_context: the #GMainContext of the game thread
 * @err: the GError in which possible errors will be reported
 *
 * Initializes the game network listener
 *
 * Return value: Returns %TRUE on success. Upon failure, %FALSE is return, and
 *               err is set accordingly (if not NULL)
 */
gboolean
wmud_networking_init(guint port_number, GMainContext *game_context, GSList *menu_items, GError **err)
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
	if ((game_socket6 = g_socket_new(G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL)) != NULL)
	{
		GInetAddress *inet_address;
		GSocketAddress *address;
		gboolean result;

		inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);
		address = g_inet_socket_address_new(inet_address, port_number);
		g_object_unref(inet_address);

		g_socket_set_listen_backlog(game_socket6, 10);

		result = g_socket_bind(game_socket6, address, TRUE, NULL)
			&& g_socket_listen(game_socket6, NULL);

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
		/* This function should never return error. If so, that would be a really big bug which will trigger a higher level problem for sure */
		g_socket_listener_add_socket(game_listener, game_socket6, NULL, NULL);
	}

	if (need_ipv4_socket)
	{
		if ((game_socket4 = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL)) != NULL)
		{
			GInetAddress *inet_address;
			GSocketAddress *address;
			gboolean result;

			inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
			address = g_inet_socket_address_new(inet_address, port_number);
			g_object_unref(inet_address);

			g_socket_set_listen_backlog(game_socket4, 10);

			result = g_socket_bind(game_socket4, address, TRUE, NULL)
				&& g_socket_listen(game_socket4, NULL);

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
			g_socket_listener_add_socket(game_listener, game_socket4, NULL, NULL);
		}
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

	game_menu = menu_items;

	email_regex = g_regex_new("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}$", G_REGEX_CASELESS, 0, NULL);
	return TRUE;
}

/**
 * wmud_client_send:
 * @client: the client to which the message will be sent
 * @fmt: the printf() style format string of the message
 * @...: optional parameters to the format string
 *
 * Sends a formatted message to a game client
 */
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

