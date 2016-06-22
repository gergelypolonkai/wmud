/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * players.c: player structure related functions
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "game-networking.h"
#include "players.h"

/**
 * SECTION:player
 * @short_description: Player database handling
 *
 * Functions to handle player database records
 */

/**
 * players:
 *
 * GSList of all loaded players. Stores #wmudPlayer structures.
 */
GSList *players = NULL;

static gint
find_player_by_name(WmudPlayer *player, gchar *player_name)
{
    return g_ascii_strcasecmp(wmud_player_get_player_name(player), player_name);
}

/**
 * wmud_player_exists:
 * @player_name: The login name of the player to check
 *
 * Check if the player with the given name already exists.
 *
 * Return value: the WmudPlayer object for the given player name, or %NULL if
 *               it can not be found.
 */
WmudPlayer *
wmud_player_exists(gchar *player_name)
{
    GSList *player_elem;

    if ((player_elem = g_slist_find_custom(players, player_name, (GCompareFunc)find_player_by_name)) == NULL) {
        return NULL;
    }

    return player_elem->data;
}
