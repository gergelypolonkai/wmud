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

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include "wmudplayer.h"

#define WMUD_TYPE_CLIENT            (wmud_client_get_type())
#define WMUD_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WMUD_TYPE_CLIENT, WmudClient))
#define WMUD_IS_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WMUD_TYPE_CLIENT))
#define WMUD_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WMUD_TYPE_CLIENT, WmudClientClass))
#define WMUD_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WMUD_TYPE_CLIENT))
#define WMUD_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WMUT_TYPE_CLIENT, WmudClientClass))

/**
 * WmudClient:
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

/**
 * WmudClientState:
 * @WMUD_CLIENT_STATE_FRESH: Client is newly connected. Waiting for a login
 *     player name
 * @WMUD_CLIENT_STATE_PASSWAIT: Login player name is entered, waiting for a
 *     login password
 * @WMUD_CLIENT_STATE_MENU: Authentication was successful, player is now in the
 *     main game menu
 * @WMUD_CLIENT_STATE_INGAME: Character login was successful, player is now
 *     in-game
 * @WMUD_CLIENT_STATE_YESNO: Player was asked a yes/no question, and we are
 *     waiting for the answer. client.yesNoCallback MUST be set at this point!
 *     TODO: if wmudClient had a prevState field, and there would be some hooks
 *     that are called before and after the client enters a new state, this
 *     could be a three-state stuff, in which the player can enter e.g ? as
 *     the answer, so they would be presented with the question again.
 * @WMUD_CLIENT_STATE_REGISTERING: Registering a new player. Waiting for the
 *     e-mail address to be given
 * @WMUD_CLIENT_STATE_REGEMAIL_CONFIRM: E-mail address entered séms valid,
 *     waiting for confirmation
 *
 * Game client states.
 */
typedef enum {
	WMUD_CLIENT_STATE_FRESH,
	WMUD_CLIENT_STATE_PASSWAIT,
	WMUD_CLIENT_STATE_MENU,
	WMUD_CLIENT_STATE_INGAME,
	WMUD_CLIENT_STATE_YESNO,
	WMUD_CLIENT_STATE_REGISTERING,
	WMUD_CLIENT_STATE_REGEMAIL_CONFIRM
} WmudClientState;

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

