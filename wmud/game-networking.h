/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * game-networking.h: basic networking function headers, variables and defines
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
#ifndef __WMUD_NETWORKING_H__
# define __WMUD_NETWORKING_H__

#include <glib.h>

#include "wmudclient.h"

/**
 * TELNET_IAC:
 *
 * Telnet IAC code
 */
#define TELNET_IAC '\xff'
/**
 * TELNET_WONT:
 *
 * Telnet WON'T code
 */
#define TELNET_WONT '\xfc'
/**
 * TELNET_WILL:
 *
 * Telnet WILL code
 */
#define TELNET_WILL '\xfb'
/**
 * TELNET_DO:
 *
 * Telnet DO code
 */
#define TELNET_DO '\xfd'
/**
 * TELNET_DONT:
 *
 * Telnet DONT code
 */
#define TELNET_DONT '\xfe'
/**
 * TELNET_ECHO:
 *
 * Telnet ECHO code
 */
#define TELNET_ECHO '\x01'

extern GSList *clients;

gboolean wmud_networking_init(guint        port_number,
                              GMainContext *game_context,
                              GSList       *menu_items,
                              GError       **err);
void wmud_client_quitanswer(WmudClient *client,
                            gboolean   answer);
void wmud_client_newchar_answer(WmudClient *client,
                                gboolean   answer);

#endif
