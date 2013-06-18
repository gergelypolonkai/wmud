/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * wmudclient.h: the WmudClient GObject type
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
#ifndef __WMUD_WMUDCLIENT_H__
#define __WMUD_WMUDCLIENT_H__

/**
 * SECTION:wmudclient
 * @short_description: wMUD Client
 * @inclide: wmudclient.h
 *
 * #WmudClient is for storing an active client connection
 **/

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include "wmudclientstate.h"
#include "wmudplayer.h"

#define WMUD_TYPE_CLIENT            (wmud_client_get_type())
#define WMUD_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WMUD_TYPE_CLIENT, WmudClient))
#define WMUD_IS_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WMUD_TYPE_CLIENT))
#define WMUD_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WMUD_TYPE_CLIENT, WmudClientClass))
#define WMUD_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WMUD_TYPE_CLIENT))
#define WMUD_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WMUT_TYPE_CLIENT, WmudClientClass))

/**
 * WmudClient:
 * @parent_instance: the parent GObject instance
 *
 * A connected game client
 */
typedef struct _WmudClient WmudClient;
typedef struct _WmudClientClass WmudClientClass;
typedef struct _WmudClientPrivate WmudClientPrivate;
typedef void (*WmudClientYesnoCallback)(WmudClient *client, gboolean answer);

struct _WmudClient
{
	GObject  parent_instance;

	/*< private >*/
	WmudClientPrivate *priv;
};

struct _WmudClientClass
{
	GObjectClass parent_class;
};

GType wmud_client_get_type(void);
WmudClient *wmud_client_new(void);
void wmud_client_set_socket(WmudClient *client, GSocket *socket);
GSocket *wmud_client_get_socket(WmudClient *client);
GSource *wmud_client_get_socket_source(WmudClient *client);
void wmud_client_send(WmudClient *client, const gchar *fmt, ...);
void wmud_client_close(WmudClient *self, gboolean send_goodbye);
GString *wmud_client_get_buffer(WmudClient *client);
gsize wmud_client_get_buffer_length(WmudClient *client);
void wmud_client_set_state(WmudClient *client, WmudClientState state);
WmudClientState wmud_client_get_state(WmudClient *client);
void wmud_client_set_player(WmudClient *client, WmudPlayer *player);
WmudPlayer *wmud_client_get_player(WmudClient *client);
void wmud_client_set_yesno_callback(WmudClient *client, WmudClientYesnoCallback yesno_callback);
WmudClientYesnoCallback wmud_client_get_yesno_callback(WmudClient *client);
void wmud_client_set_authenticated(WmudClient *client, gboolean authenticated);
void wmud_client_increase_login_fail_count(WmudClient *client);
gint wmud_client_get_login_fail_count(WmudClient *client);
void wmud_client_set_bademail(WmudClient *client, gboolean bademail);
gboolean wmud_client_get_bademail(WmudClient *client);
guint32 wmud_client_get_last_recv_age(WmudClient *client);
void wmud_client_set_context(WmudClient *client, GMainContext *context);

#endif /* __WMUD_WMUDCLIENT_H__ */

