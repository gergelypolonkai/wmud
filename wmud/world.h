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

extern GQuark WMUD_WORLD_ERROR;
typedef enum {
	WMUD_WORLD_ERROR_DUPPLANE,
	WMUD_WORLD_ERROR_DUPPLANET,
	WMUD_WORLD_ERROR_DUPAREA,
	WMUD_WORLD_ERROR_DUPROOM
} wmudWorldError;

typedef struct _wmudPlane {
	guint id;
	gchar *name;
} wmudPlane;

typedef struct _wmudPlanet {
	guint id;
	gchar *name;
} wmudPlanet;

typedef struct _wmudArea {
	guint id;
	gchar *name;
} wmudArea;

typedef struct _wmudRoom {
	guint id;
	guint area_id;
	gchar *name;
	gchar *distant_description;
	gchar *close_description;
} wmudRoom;

typedef struct _wmudDirection {
	guint id;
	gchar *short_name;
	gchar *name;
} wmudDirection;

gboolean wmud_world_check_planes(GSList *planes, GError **err);
gboolean wmud_world_check_planets(GSList *planets, GError **err);
gboolean wmud_world_check_areas(GSList *areas, GError **err);
gboolean wmud_world_check_rooms(GSList *rooms, GError **err);
gboolean wmud_world_check_exits(GSList *exits, GSList *directions, GSList *rooms, GError **err);

gboolean wmud_world_assoc_planets_planes(GSList *planets, GSList *planes, GError **err);
gboolean wmud_world_assoc_rooms_areas(GSList *rooms, GSList *areas, GError **err);
gboolean wmud_world_assoc_rooms_planets(GSList *rooms, GSList *planets, GError **err);
void wmud_world_assoc_exits_rooms(GSList *exits, GSList *rooms, GError **err);

gboolean wmud_world_load(GError **err);

#endif /* __WMUD_WORLD_H__ */

