/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * db.h: database handling routine prototypes
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
#ifndef __WMUD_DB_H__
#define __WMUD_DB_H__

#include <glib.h>

#include "wmudplayer.h"

#define WMUD_DB_ERROR wmud_db_error_quark()
GQuark wmud_db_error_quark();

/**
 * wmudDbError:
 * @WMUD_DB_ERROR_CANTOPEN: Database file cannot be opened
 * @WMUD_DB_ERROR_NOINIT: Database system was not initialized
 * @WMUD_DB_ERROR_BADQUERY: Invalid database query
 *
 * Error codes returned by database handling functions.
 */
typedef enum {
	WMUD_DB_ERROR_CANTOPEN,
	WMUD_DB_ERROR_NOINIT,
	WMUD_DB_ERROR_BADQUERY
} wmudDbError;

gboolean wmud_db_init(GError **err);
gboolean wmud_db_load_players(GError **err);
gboolean wmud_db_save_player(WmudPlayer *player, GError **err);
gboolean wmud_db_load_planes(GSList **planes, GError **err);
gboolean wmud_db_load_planets(GSList **planets, GError **err);
gboolean wmud_db_load_directions(GSList **directions, GError **err);
gboolean wmud_db_load_areas(GSList **areas, GError **err);
gboolean wmud_db_load_rooms(GSList **rooms, GError **err);
gboolean wmud_db_load_exits(GSList **exits, GError **err);
gboolean wmud_db_load_planet_planes(GSList **planet_planes, GError **err);
gboolean wmud_db_load_menu(GSList **menu_items, GError **err);

#endif /* __WMUD__DB_H__ */

