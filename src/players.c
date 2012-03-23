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

GSList *players = NULL;

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

wmudPlayer *
wmud_player_exists(gchar *player_name)
{
	GSList *player_elem;
	
	if ((player_elem = g_slist_find_custom(players, player_name, (GCompareFunc)find_player_by_name)) == NULL)
		return NULL;

	return player_elem->data;

}

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

