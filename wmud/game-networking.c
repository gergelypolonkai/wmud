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
#include "texts.h"
#include "wmudclient.h"
#include "wmudplayer.h"

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
 *
 * The full #GSList of the currently connected #WmudClient objects.
 */
GSList *clients = NULL;

static GRegex *email_regex = NULL;

void wmud_client_interpret_newplayer_email(WmudClient *client);
void wmud_client_interpret_newplayer_mailconfirm(WmudClient *client);
static void state_fresh(WmudClient *client);
static void state_passwait(WmudClient *client);
static void state_menu(WmudClient *client);
static void state_yesno(WmudClient *client);
static void state_registering(WmudClient *client);
static void state_regemail_confirm(WmudClient *client);

static void
remove_client(WmudClient *client, gboolean send_quitmessage)
{
	clients = g_slist_remove(clients, client);
	wmud_client_close(client, send_quitmessage);
}

static void
hup_client(WmudClient *client)
{
	remove_client(client, FALSE);
}

/**
 * wmud_client_callback:
 * @client: the socket of the client on which the data arrived
 * @condition: the condition available on the client socket
 * @client: the WmudClient structure of the client
 *
 * Processes incoming client data, and client hangup
 */
static void
recv_client(WmudClient *client)
{
	GError *err = NULL;
	GSocket *client_socket;
	gssize len;
	gchar *buf2;
	gchar *buf = g_malloc0(sizeof(gchar) * (MAX_RECV_LEN + 1));

	client_socket = wmud_client_get_socket(client);

	if ((len = g_socket_receive(client_socket, buf, MAX_RECV_LEN, NULL, &err)) == 0) {
		g_free(buf);
		remove_client(client, FALSE);

		return;
	}

	buf2 = buf;
	while (TRUE) {
		char *r = strchr((char *)buf2, '\r'),
		     *n = strchr((char *)buf2, '\n');

		if (r || n) {
			gint i,
			     sloc = -1;

			if ((r < n) && r) {
				if (wmud_client_get_buffer_length(client) > 0)
					g_string_append_len(wmud_client_get_buffer(client), buf2, (r - buf2));
				else
					g_string_overwrite_len(wmud_client_get_buffer(client), 0, buf2, (r - buf2));
				buf2 = r;
			} else if (n) {
				if (wmud_client_get_buffer_length(client) > 0)
					g_string_append_len(wmud_client_get_buffer(client), buf2, (n - buf2));
				else
					g_string_overwrite_len(wmud_client_get_buffer(client), 0, buf2, (n - buf2));
				buf2 = n;
			}

			/* Remove telnet codes from the string */
			for (i = 0; i < wmud_client_get_buffer_length(client); i++) {
				guchar c = (wmud_client_get_buffer(client)->str)[i];

				if ((c >= 240) || (c == 1)) {
					if (sloc == -1)
						sloc = i;
				} else {
					if (sloc != -1) {
						g_string_erase(wmud_client_get_buffer(client), sloc, i - sloc);
						sloc = -1;
					}
				}
			}

			if (sloc != -1)
				g_string_erase(wmud_client_get_buffer(client), sloc, -1);

			switch (wmud_client_get_state(client))
			{
				case WMUD_CLIENT_STATE_FRESH:
					state_fresh(client);
					break;
				case WMUD_CLIENT_STATE_PASSWAIT:
					state_passwait(client);
					break;
				case WMUD_CLIENT_STATE_MENU:
					state_menu(client);
					break;
				case WMUD_CLIENT_STATE_INGAME:
					wmud_interpret_game_command(client);
					break;
				case WMUD_CLIENT_STATE_YESNO:
					state_yesno(client);
					break;
				case WMUD_CLIENT_STATE_REGISTERING:
					state_registering(client);
					break;
				case WMUD_CLIENT_STATE_REGEMAIL_CONFIRM:
					state_regemail_confirm(client);
					break;
			}
			g_string_erase(wmud_client_get_buffer(client), 0, -1);

			for (; ((*buf2 == '\r') || (*buf2 == '\n')) && *buf2; buf2++);

			if (!*buf2)
				break;
		} else {
			if (wmud_client_get_buffer_length(client) > 0)
				g_string_append(wmud_client_get_buffer(client), buf2);
			else
				g_string_overwrite(wmud_client_get_buffer(client), 0, buf2);

			break;
		}
	}

	g_free(buf);
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
	GError *err = NULL;
	GSocketAddress *remote_addr;
	WmudClient *client;

	/* This function should never return an error. If so, it is a huge bug,
	 * and will trigger a higher level error. */
	client_socket = g_socket_listener_accept_socket(accept_data->listener, NULL, NULL, &err);

	client = wmud_client_new();
	wmud_client_set_socket(WMUD_CLIENT(client), client_socket);
	wmud_client_set_context(client, accept_data->context);
	g_signal_connect(client, "net-hup", G_CALLBACK(hup_client), NULL);
	g_signal_connect(client, "net-recv", G_CALLBACK(recv_client), NULL);

	clients = g_slist_prepend(clients, client);

	g_clear_error(&err);

	if ((remote_addr = g_socket_get_remote_address(client_socket, &err)) != NULL) {
		GInetAddress *addr;
		gchar *ip_addr;

		addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(remote_addr));
		ip_addr = g_inet_address_to_string(addr);
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "New game connection from %s", ip_addr);
		g_free(ip_addr);
		g_object_unref(addr);
		g_object_unref(remote_addr);
	} else {
		if (err)
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "New game connection. The remote address is unknown. This is a bug. Message from upper level: %s", err->message);
		else
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "New game connection. The remote address is unknown. This is a bug.");
	}

	g_clear_error(&err);
	wmud_client_send(WMUD_CLIENT(client), "By what name shall we call you? ");

	return TRUE;
}

/**
 * wmud_networking_init:
 * @port_number: the port number on which the game listener should listen
 * @game_context: the #GMainContext of the game thread
 * @menu_items: a #GSList of menu items to present to the client
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
	if ((game_socket6 = g_socket_new(G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL)) != NULL) {
		GInetAddress *inet_address;
		GSocketAddress *address;
		gboolean result;

		inet_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);
		address = g_inet_socket_address_new(inet_address, port_number);
		g_object_unref(inet_address);

		g_socket_set_listen_backlog(game_socket6, 10);

		result =
		         g_socket_bind(game_socket6, address, TRUE, NULL)
		         && g_socket_listen(game_socket6, NULL);

		g_object_unref(address);

		if (!result) {
			g_object_unref(game_socket6);
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "Unable to create listener IPv6 socket");

			return FALSE;
		}

		if (g_socket_speaks_ipv4(game_socket6))
			need_ipv4_socket = FALSE;

		game_net_source6 = g_socket_create_source(game_socket6, G_IO_IN, NULL);

		/* This function should never return error. If so, that would be a
		 * really big bug which will trigger a higher level problem for sure */
		g_socket_listener_add_socket(game_listener, game_socket6, NULL, NULL);
	}

	if (need_ipv4_socket) {
		if ((game_socket4 = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT, NULL)) != NULL) {
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

			if (!result) {
				g_object_unref(game_socket4);

				if (!game_socket6)
					g_object_unref(game_socket6);

				g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "Unable to create listener IPv4 socket!\n");

				return FALSE;
			}

			game_net_source4 = g_socket_create_source(game_socket4, G_IO_IN, NULL);
			g_socket_listener_add_socket(game_listener, game_socket4, NULL, NULL);
		}
	} else {
		if (game_socket6 != NULL)
			g_clear_error(&in_err);
		else
			return FALSE;
	}

	accept_data = g_new(struct AcceptData, 1);
	accept_data->listener = game_listener;
	accept_data->context = game_context;

	if (game_net_source6) {
		g_source_set_callback(game_net_source6, (GSourceFunc)game_source_callback, (gpointer)accept_data, NULL);
		g_source_attach(game_net_source6, game_context);
	}

	if (game_net_source4) {
		g_source_set_callback(game_net_source4, (GSourceFunc)game_source_callback, (gpointer)accept_data, NULL);
		g_source_attach(game_net_source4, game_context);
	}

	game_menu = menu_items;

	email_regex = g_regex_new("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}$", G_REGEX_CASELESS, 0, NULL);

	return TRUE;
}

void
wmud_client_quitanswer(WmudClient *client, gboolean answer)
{
	if (answer) {
		remove_client(client, TRUE);
	} else {
		wmud_client_send(client, "Good boy!\r\n");
		wmud_client_set_state(client, WMUD_CLIENT_STATE_MENU);
	}
}

void
wmud_client_newchar_answer(WmudClient *client, gboolean answer)
{
	if (answer) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Creating new player\n");
		wmud_client_send(client, "Welcome to this MUD!\r\nPlease enter your e-mail address: ");
		wmud_client_set_state(client, WMUD_CLIENT_STATE_REGISTERING);
	} else {
		wmud_client_send(client, "What is your player-name, then? ");
		wmud_client_set_state(client, WMUD_CLIENT_STATE_FRESH);
	}
}

static void
state_fresh(WmudClient *client)
{
	if (*(wmud_client_get_buffer(client)->str)) {
		WmudPlayer *player;

		if ((player = wmud_player_exists(wmud_client_get_buffer(client)->str)) != NULL) {
			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Trying to"
			          " login with playername '%s'",
			          wmud_client_get_buffer(client)->str);

			if (wmud_player_get_cpassword(player) == NULL) {
				wmud_client_send(client, "Your registration is"
				                 " not finished yet.\r\n");
				remove_client(client, TRUE);
			} else {
				wmud_client_set_state(client, WMUD_CLIENT_STATE_PASSWAIT);
				wmud_player_set_registered(player, TRUE);
				wmud_client_set_player(client, player);
				wmud_client_send(client, "Please provide us your password: %c%c%c", TELNET_IAC, TELNET_WILL, TELNET_ECHO);
			}
		} else {
			WmudPlayer *player = wmud_player_new();

			wmud_player_set_player_name(player, wmud_client_get_buffer(client)->str);
			wmud_client_set_player(client, player);
			wmud_client_set_state(client, WMUD_CLIENT_STATE_YESNO);
			wmud_client_set_yesno_callback(client, wmud_client_newchar_answer);
			wmud_client_send(client, "Is %s new to this game? [Y/N] ", wmud_client_get_buffer(client)->str);
		}
	}
}

static void
state_passwait(WmudClient *client)
{
	if (*(wmud_client_get_buffer(client)->str)) {
		WmudPlayer *player;

		player = wmud_client_get_player(client);

		if (wmud_player_password_valid(player, wmud_client_get_buffer(client)->str)) {
			gint fail_count;

			wmud_client_send(client, "%c%c%c\r\nLogin successful."
					 "\r\n", TELNET_IAC, TELNET_WONT,
			                 TELNET_ECHO);
			wmud_client_set_authenticated(client, TRUE);

			if ((fail_count = wmud_player_get_fail_count(player)) > 0) {
				wmud_client_send(client, "There %s %d failed"
				                 " login attempt%s with your"
				                 " account since your last"
				                 " visit\r\n",
						 (fail_count == 1) ? "was" : "were",
						 fail_count,
						 (fail_count == 1) ? "" : "s");
			}

			wmud_text_send_to_client("motd", client);
			wmud_menu_present(client);
		} else {
			wmud_client_send(client, "%c%c%cThis password doesn't"
			                 " seem to be valid. Let's try it again..."
			                 " \r\nBy what name would you like to be"
					 " be called? ", TELNET_IAC,
					 TELNET_WONT, TELNET_ECHO);
			wmud_client_set_state(client, WMUD_CLIENT_STATE_FRESH);
			wmud_player_increase_fail_count(player);
			wmud_client_increase_login_fail_count(client);
			if (wmud_client_get_login_fail_count(client) == 3) {
				wmud_client_send(client, "You are trying "
						 " these bad passwords for"
						 " too many times. Please"
						 " stop that!\r\n");
				remove_client(client, TRUE);

				/* TODO: Increase IP fail count, and ban IP if it's too high */
			}

			/* TODO: Increase and save login fail count */

			wmud_client_set_player(client, NULL);
		}
	} else {
		wmud_client_send(client, "\r\nEmpty passwords are"
			     " not valid.\r\nTry again: ");
	}
}

static void
state_menu(WmudClient *client)
{
	gchar *menu_command;

	if ((menu_command = wmud_menu_get_command_by_menuchar(*(wmud_client_get_buffer(client)->str), game_menu)) != NULL)
		wmud_menu_execute_command(client, menu_command);
	else
		wmud_client_send(client, "Unknown menu command.\r\n");
}

static void
state_yesno(WmudClient *client)
{
	if (g_ascii_strcasecmp(wmud_client_get_buffer(client)->str, "y") == 0)
		(wmud_client_get_yesno_callback(client))(client, TRUE);
	else if (g_ascii_strcasecmp(wmud_client_get_buffer(client)->str, "n") == 0)
		(wmud_client_get_yesno_callback(client))(client, FALSE);
	else
		wmud_client_send(client, "Please enter a 'Y' or 'N'"
			 " character: ");
}

static void
state_registering(WmudClient *client)
{
	if (!*(wmud_client_get_buffer(client)->str) && (wmud_client_get_bademail(client) == TRUE))
		remove_client(client, TRUE);

	if (g_regex_match(email_regex, wmud_client_get_buffer(client)->str, 0, NULL)) {
		wmud_player_set_email(wmud_client_get_player(client), wmud_client_get_buffer(client)->str);
		wmud_client_set_state(client, WMUD_CLIENT_STATE_REGEMAIL_CONFIRM);
		wmud_client_send(client, "It seems to be a valid"
				 " address to me, but could you"
			     " write it again? ");
	} else {
		wmud_client_send(client, "\r\nSorry, but this"
				 "e-mail address doesn't seem to be"
				 " valid to me.\r\n\r\nIf you think"
				 " this is a valid address, simply"
				 " press enter to quit, and send an"
				 " e-mail to %s from that address,"
				 " so we can fix our e-mail"
				 " validation code.\r\n\r\nIf you"
				 " just mistyped your address, type"
			     " it now: ",
				 active_config->admin_email);

		if (*(wmud_client_get_buffer(client)->str))
			wmud_client_set_bademail(client, TRUE);
	}
}

static void
state_regemail_confirm(WmudClient *client)
{
	GError *err = NULL;

	if (g_ascii_strcasecmp(wmud_player_get_email(wmud_client_get_player(client)), wmud_client_get_buffer(client)->str) == 0) {
		if (wmud_db_save_player(wmud_client_get_player(client), &err)) {
			wmud_client_send(client, "\r\nGood. We will generate the password for this player name, and send it to you\r\nvia e-mail. Please come back to us, if you get that code, so you can log\r\nin.\r\n");
			players = g_slist_prepend(players, wmud_player_dup(wmud_client_get_player(client)));
		} else {
			g_critical("wmud_db_save_player() error: %s", err->message);
			wmud_client_send(client, "\r\nThere was an error during the database update. Please try again later!\r\n");
		}

		remove_client(client, TRUE);
	} else {
		wmud_player_set_email(wmud_client_get_player(client), NULL);

		wmud_client_send(client, "This is not the same as you entered before.\r\nLet's just try it again: ");
		wmud_client_set_state(client, WMUD_CLIENT_STATE_REGISTERING);
	}
}

