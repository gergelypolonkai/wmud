/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * wmud-types.h: Common wMUD data types
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
#ifndef __WMUD_TYPES_H__
#define __WMUD_TYPES_H__

#include <glib.h>
#include <gio/gio.h>

/**
 * SECTION:types
 * @short_description: wMUD Data Types
 * @title: wMUD's Data Types
 *
 */

/**
 * wmudClientState:
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
} wmudClientState;

/**
 * wmudPlayer:
 * @id: Player's database ID
 * @player_name: Player's login name
 * @cpassword: crypt()ed password of the player. This is NULL for newly
 *     registered players, who have no password generated for them by the
 *     maintenance loop
 * @email: E-mail address of the player
 *
 * The <structname>wmudPlayer</structname> structure contains all information of
 * a player.
 */
typedef struct _wmudPlayer {
	guint32 id;
	gchar *player_name;
	gchar *cpassword;
	gchar *email;
	gint fail_count;
	gboolean registered;
} wmudPlayer;

typedef struct _wmudClient wmudClient;

typedef void (*wmudYesNoCallback)(wmudClient *client, gboolean answer);

/**
 * wmudClient:
 * @socket: The assigned GSocket object
 * @buffer: The client receive buffer. It may hold partial or multiple lines
 *     until processed
 * @state: The state of the client
 * @authenticated: TRUE if the client is an authenticated game player
 * @player: The associatec player structure. It is also used during
 *     registration, so it should be always checked if the player is a saved
 *     database user
 * @bademail: indicates that the entered e-mail address is invalid
 * @socket_source: the #GSource associated with the client socket
 * @login_try_count: the failed login count of the client
 *
 * <structname>wmudClient</structname> contains all properties of a connected
 * game client.
 */
struct _wmudClient {
	GSocket *socket;
	GSource *socket_source;
	GString *buffer;
	wmudClientState state;
	gboolean authenticated;
	wmudPlayer *player;
	gboolean bademail;
	gint login_try_count;
	wmudYesNoCallback yesNoCallback;
};

#endif /* __WMUD_TYPES_H__ */

