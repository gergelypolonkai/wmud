/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * networking.h: basic networking function headers, variables and defines
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

#include "wmud_types.h"

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
 * TELNET_ECHO:
 *
 * Telnet ECHO code
 */
#define TELNET_ECHO '\x01'

extern GSList *clients;

gboolean wmud_networking_init(guint port_number, GError **err);
void wmud_client_send(wmudClient *client, const gchar *fmt, ...);
void wmud_client_start_login(wmudClient *client);
void wmud_client_interpret_newplayer_answer(wmudClient *client);
void wmud_client_interpret_newplayer_email(wmudClient *client);

#endif
