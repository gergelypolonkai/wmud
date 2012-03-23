/*
 * players.h
 *
 * Copyright (C) 2012 - Gergely POLONKAI
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
#ifndef __WMUD_PLAYERS_H__
#define __WMUD_PLAYERS_H__

#include <glib.h>

#include "wmud_types.h"

extern GSList *players;

gboolean wmud_player_auth(wmudClient *client);
wmudPlayer *wmud_player_exists(gchar *player_name);
void wmud_player_free(wmudPlayer **player);
wmudPlayer *wmud_player_dup(wmudPlayer *player);

#endif /* __WMUD_PLAYERS_H__ */
