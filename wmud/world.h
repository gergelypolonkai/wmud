/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * world.h: world loading and building functions
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
#ifndef __WMUD_WORLD_H__
#define __WMUD_WORLD_H__

#include <glib.h>

#define WMUD_WORLD_ERROR wmud_world_error_quark()
GQuark wmud_world_error_quark();

typedef enum {
	WMUD_WORLD_ERROR_DUPPLANE,
	WMUD_WORLD_ERROR_DUPPLANET,
	WMUD_WORLD_ERROR_DUPAREA,
	WMUD_WORLD_ERROR_DUPROOM,
	WMUD_WORLD_ERROR_BADASSOC,
	WMUD_WORLD_ERROR_BADPLANET
} wmudWorldError;

/**
 * wmudPlane:
 * @id: The database ID of the plane
 * @name: The name of the plane
 */
typedef struct _wmudPlane {
	guint id;
	gchar *name;
} wmudPlane;

/**
 * wmudPlanet:
 * @id: The database ID of the planet
 * @name: The name of the planet
 * @planes: A #GSList of planes this planet is connected to. The data of this
 *          list should never be freed, only the #GSList itself with
 *          g_slist_free()
 */
typedef struct _wmudPlanet {
	guint id;
	gchar *name;
	GSList *planes;
} wmudPlanet;

/**
 * wmudArea:
 * @id: The database ID of the area
 * @name: The name of the area
 * @rooms: a #GSList of rooms in this area
 */
typedef struct _wmudArea {
	guint id;
	gchar *name;
	GSList *rooms;
} wmudArea;

/**
 * wmudRoom:
 * @id: The database ID of the room
 * @area_id: The database ID of the area this room is in
 * @name: The name of the room
 * @distant_description: The description shown when someone looks at the room
 *                       from a distance (e.g from the door next to this one)
 * @close_description: The description shown when someone looks at the room
 *                     when standing in it
 */
typedef struct _wmudRoom {
	guint id;
	guint area_id;
	gchar *name;
	gchar *distant_description;
	gchar *close_description;
	GSList *planes;
	GSList *exits;
} wmudRoom;

/**
 * wmudDirection:
 * @id: The database ID of the direction
 * @short_name: The short version of the direction's command
 * @name: The normal name of the direction's command
 */
typedef struct _wmudDirection {
	guint id;
	gchar *short_name;
	gchar *name;
} wmudDirection;

/**
 * wmudExit:
 * @source_room_id: The database ID of the room from where the given exit opens
 * @direction_id: The database ID of the direction of this exit
 * @destination_room_id: The database ID of the room to where the give exit
 *                       opens
 */
typedef struct _wmudExit {
	guint source_room_id;
	guint direction_id;
	guint destination_room_id;
} wmudExit;

/**
 * wmudRoomExit:
 * @direction: the direction towards which this exit lead
 * @other_side: the room to which this exit leads to
 */
typedef struct _wmudRoomExit {
	wmudDirection *direction;
	wmudRoom *other_side;
} wmudRoomExit;

/**
 * wmudPlanetPlaneAssoc:
 * @planet_id: The database ID of the planet
 * @plane_id: The database ID of the plane @planet is on
 */
typedef struct _wmudPlanetPlaneAssoc {
	guint planet_id;
	guint plane_id;
} wmudPlanetPlaneAssoc;

gboolean wmud_world_check_planes(GSList *planes, GError **err);
gboolean wmud_world_check_planets(GSList *planets, GError **err);
gboolean wmud_world_check_areas(GSList *areas, GError **err);
gboolean wmud_world_check_rooms(GSList *rooms, GError **err);
gboolean wmud_world_check_exits(GSList *exits, GSList *directions, GSList *rooms, GError **err);

gboolean wmud_world_assoc_planets_planes(GSList *planets, GSList *planes, GSList *planet_planes, GError **err);
gboolean wmud_world_assoc_rooms_areas(GSList *rooms, GSList *areas, GError **err);
gboolean wmud_world_assoc_rooms_planets(GSList *rooms, GSList *planets, GError **err);
void wmud_world_assoc_exits_rooms(GSList *exits, GSList *directions, GSList *rooms, GError **err);

gboolean wmud_world_load(GError **err);

#endif /* __WMUD_WORLD_H__ */

