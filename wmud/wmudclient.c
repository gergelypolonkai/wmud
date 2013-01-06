/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * wmudclient.c: the WmudClient GObject type
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

#include "wmudclient.h"
#include "wmudclientstate.h"
#include "enumtypes.h"
#include "players.h"

/**
 * SECTION:wmudclient
 * @short_description: wMUD Client
 * @inclide: wmudclient.h
 *
 * #WmudClient is for storing an active client connection
 **/

G_DEFINE_TYPE(WmudClient, wmud_client, G_TYPE_OBJECT);

enum {
	SIG_CONNECTED,
	SIG_NET_HUP,
	SIG_NET_RECV,
	SIG_STATE_CHANGE,
	SIG_LAST
};

static guint signals[SIG_LAST] = { 0 };

#define WMUD_CLIENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WMUD_TYPE_CLIENT, WmudClientPrivate))

struct _WmudClientPrivate
{
	GSocket *socket;
	GSource *socket_source;
	GString *buffer;
	WmudClientState state;
	gboolean authenticated;
	WmudPlayer *player;
	gboolean bademail;
	gint login_try_count;
	WmudClientYesnoCallback yesno_callback;
	time_t last_recv;
};

static void
wmud_client_dispose(GObject *gobject)
{
	WmudClient *self = WMUD_CLIENT(gobject);

	if (self->priv->socket) {
		g_object_unref(self->priv->socket);

		self->priv->socket = NULL;
	}

	G_OBJECT_CLASS(wmud_client_parent_class)->dispose(gobject);
}

static void
wmud_client_finalize(GObject *gobject)
{
	WmudClient *self = WMUD_CLIENT(gobject);

	g_string_free(self->priv->buffer, TRUE);
	g_source_destroy(self->priv->socket_source);

	G_OBJECT_CLASS(wmud_client_parent_class)->finalize(gobject);
}

static void net_recv(WmudClient *self)
{
	self->priv->last_recv = time(NULL);
}

static void
wmud_client_class_init(WmudClientClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = wmud_client_dispose;
	gobject_class->finalize = wmud_client_finalize;

	/**
	 * WmudClient::connected:
	 * @client: The client emitting the signal
	 *
	 * Emitted when a new client connection is accepted
	 **/
	signals[SIG_CONNECTED] = g_signal_newv("connected",
	                                       WMUD_TYPE_CLIENT,
	                                       G_SIGNAL_RUN_LAST,
	                                       NULL,
	                                       NULL, NULL,
	                                       NULL,
	                                       G_TYPE_NONE, 0, NULL);

	/**
	 * WmudClient::net-hup:
	 * @client: The client emitting the signal
	 *
	 * Emitted when the remote side closes the connection
	 **/
	signals[SIG_NET_HUP] = g_signal_newv("net-hup",
			                     WMUD_TYPE_CLIENT,
					     G_SIGNAL_RUN_LAST,
					     NULL,
					     NULL, NULL,
					     NULL,
					     G_TYPE_NONE, 0, NULL);
	/**
	 * WmudClient::net-recv:
	 * @client: The client emitting the signal
	 *
	 * Emitted when data is received through the client socket
	 **/
	signals[SIG_NET_RECV] = g_signal_newv("net-recv",
	                                      WMUD_TYPE_CLIENT,
				              G_SIGNAL_RUN_LAST,
					      g_cclosure_new(G_CALLBACK(net_recv), NULL, NULL),
					      NULL, NULL,
					      NULL,
					      G_TYPE_NONE, 0, NULL);

	/**
	 * WmudClient::state-change:
	 * @client: The client emitting the signal
	 * @old_state: The state which we are changing from.
	 * @new_state: The state which we are changing to.
	 *
	 * Emitted when the client changes state
	 **/
	signals[SIG_STATE_CHANGE] = g_signal_new("state-change",
	                                          WMUD_TYPE_CLIENT,
						  G_SIGNAL_RUN_LAST,
						  0,
						  NULL, NULL,
						  NULL,
						  G_TYPE_NONE, 2, WMUD_TYPE_CLIENT_STATE, WMUD_TYPE_CLIENT_STATE);

	g_type_class_add_private(klass, sizeof(WmudClientPrivate));
}

static void
wmud_client_init(WmudClient *self)
{
	self->priv = WMUD_CLIENT_GET_PRIVATE(self);
	self->priv->socket_source = NULL;
	self->priv->state = WMUD_CLIENT_STATE_FRESH;
	self->priv->buffer = g_string_new("");
	self->priv->last_recv = time(NULL);

	g_signal_emit_by_name(self, "connected", G_TYPE_NONE);
}

static gboolean
net_emitter(GSocket *client_socket, GIOCondition condition, WmudClient *self)
{
	if (condition & G_IO_HUP)
		g_signal_emit_by_name(self, "net-close", G_TYPE_NONE);
	else if ((condition & G_IO_IN) || (condition & G_IO_PRI))
		g_signal_emit_by_name(self, "net-recv", G_TYPE_NONE);

	return TRUE;
}

WmudClient *
wmud_client_new(void)
{
	return g_object_new(WMUD_TYPE_CLIENT, NULL, NULL);
}

void
wmud_client_set_socket(WmudClient *self, GSocket *socket)
{
	/* TODO: Check if a socket is already set! */
	self->priv->socket = socket;
	self->priv->socket_source = g_socket_create_source(socket, G_IO_IN | G_IO_OUT | G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL, NULL);

	g_source_set_callback(self->priv->socket_source, (GSourceFunc)net_emitter, self, NULL);
}

GSocket *
wmud_client_get_socket(WmudClient *self)
{
	return self->priv->socket;
}

GSource *
wmud_client_get_socket_source(WmudClient *self)
{
	return self->priv->socket_source;
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
wmud_client_send(WmudClient *self, const gchar *fmt, ...)
{
	va_list ap;
	GString *buf = g_string_new("");

	va_start(ap, fmt);
	g_string_vprintf(buf, fmt, ap);
	va_end(ap);

	/* TODO: error checking */
	g_socket_send(self->priv->socket, buf->str, buf->len, NULL, NULL);
	g_string_free(buf, TRUE);
}

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
wmud_client_close(WmudClient *self, gboolean send_goodbye)
{
	if (send_goodbye)
		wmud_client_send(self, "\r\nHave a nice real-world day!\r\n\r\n");

	g_socket_shutdown(self->priv->socket, TRUE, TRUE, NULL);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "Connection closed.");

	if (self->priv->player && !wmud_player_get_registered(self->priv->player))
		g_object_unref(self->priv->player);

	g_object_unref(self);
}

gsize
wmud_client_get_buffer_length(WmudClient *self)
{
	return self->priv->buffer->len;
}

GString *
wmud_client_get_buffer(WmudClient *self)
{
	return self->priv->buffer;
}

WmudClientState
wmud_client_get_state(WmudClient *self)
{
	return self->priv->state;
}

void
wmud_client_set_state(WmudClient *self, WmudClientState new_state)
{
	WmudClientState old_state = self->priv->state;

	self->priv->state = new_state;

	g_signal_emit_by_name(self, "state-change", old_state, new_state, G_TYPE_NONE);
}

void
wmud_client_set_player(WmudClient *self, WmudPlayer *player)
{
	self->priv->player = player;
}

WmudPlayer *
wmud_client_get_player(WmudClient *self)
{
	return self->priv->player;
}

void
wmud_client_set_yesno_callback(WmudClient *self, WmudClientYesnoCallback yesno_callback)
{
	self->priv->yesno_callback = yesno_callback;
}

WmudClientYesnoCallback
wmud_client_get_yesno_callback(WmudClient *self)
{
	return self->priv->yesno_callback;
}

void
wmud_client_set_authenticated(WmudClient *self, gboolean authenticated)
{
	self->priv->authenticated = authenticated;
}

void
wmud_client_increase_login_fail_count(WmudClient *self)
{
	self->priv->login_try_count++;
}

gint
wmud_client_get_login_fail_count(WmudClient *self)
{
	return self->priv->login_try_count;
}

void
wmud_client_set_bademail(WmudClient *self, gboolean bademail)
{
	self->priv->bademail = bademail;
}

gboolean
wmud_client_get_bademail(WmudClient *self)
{
	return self->priv->bademail;
}

void
wmud_client_set_context(WmudClient *self, GMainContext *context)
{
	g_source_attach(self->priv->socket_source, context);
}

guint32
wmud_client_get_last_recv_age(WmudClient *self)
{
	return (time(NULL) - self->priv->last_recv);
}

