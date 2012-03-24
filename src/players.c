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

#include "networking.h"
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

/**
 * wmud_player_auth:
 * @client: The client to be authenticated. The authentication password comes
 *          from the client's buffer.
 *
 * Tries to authenticate a client based on the associated player structure, and
 * the password stored in the client's buffer.
 *
 * Return value: %TRUE if the password is valid, %FALSE otherwise.
 */
gboolean
wmud_player_auth(wmudClient *client)
{
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Player entered %s as password", client->buffer->str);

	if (g_strcmp0(crypt(client->buffer->str, client->player->cpassword), client->player->cpassword) == 0)
		return TRUE;

	return FALSE;
}

static gint
find_player_by_name(wmudPlayer *player, gchar *player_name)
{
	return g_ascii_strcasecmp(player->player_name, player_name);
}

/**
 * wmud_player_exists:
 * @player_name: The login name of the player to check
 *
 * Check if the player with the given name already exists.
 *
 * Return value: the wmudPlayer structure for the given player name, or %NULL
 *               if it can not be found.
 */
wmudPlayer *
wmud_player_exists(gchar *player_name)
{
	GSList *player_elem;
	
	if ((player_elem = g_slist_find_custom(players, player_name, (GCompareFunc)find_player_by_name)) == NULL)
		return NULL;

	return player_elem->data;

}

/**
 * wmud_player_dup:
 * @player: the player structure to duplicate
 *
 * Duplicates a #wmudPlayer structure.
 *
 * Return value: the new, duplicated player structure. It must be freed with wmud_player_free().
 */
wmudPlayer *
wmud_player_dup(wmudPlayer *player)
{
	wmudPlayer *new_player;

	if (!player)
		return NULL;

	new_player = g_new0(wmudPlayer, 1);
	new_player->id = player->id;
	new_player->player_name = g_strdup(player->player_name);
	new_player->cpassword = g_strdup(player->cpassword);
	new_player->email = g_strdup(player->email);

	return new_player;
}

/**
 * wmud_player_free:
 * @player: A pointer to the player structure to be freed
 *
 * Frees a #wmudPlayer structure with all its fields, and sets the structure
 * variable to %NULL.
 */
void
wmud_player_free(wmudPlayer **player)
{
	if (!*player)
		return;
	if ((*player)->player_name)
		g_free((*player)->player_name);
	if ((*player)->cpassword)
		g_free((*player)->cpassword);
	if ((*player)->email)
		g_free((*player)->email);
	g_free(*player);
	*player = NULL;
}

