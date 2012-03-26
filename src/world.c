/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * world.c: world loading and building functions
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

#include <glib.h>

#include "world.h"
#include "db.h"

/**
 * SECTION:world
 * @short_description: World loading and building functions
 * @title: Game world manipulation
 *
 */

gboolean
wmud_world_check_planes(GSList *planes, GError **err)
{
	return FALSE;
}

gboolean
wmud_world_check_planets(GSList *planets, GError **err)
{
	return FALSE;
}

gboolean
wmud_world_check_areas(GSList *areas, GError **err)
{
	return FALSE;
}

gboolean
wmud_world_check_exits(GSList *exits, GError **err)
{
	return FALSE;
}

gboolean
wmud_world_assoc_planets_planes(GSList *planets, GSList *planes, GError **err)
{
	return FALSE;
}

gboolean
wmud_world_assoc_rooms_areas(GSList *rooms, GSList *areas, GError **err)
{
	return FALSE;
}

gboolean
wmud_world_assoc_rooms_planets(GSList *rooms, GSList *planets, GError **err)
{
	return FALSE;
}

gboolean
wmud_world_assoc_exits_rooms(GSList *exits, GSList *rooms, GError **err)
{
	return FALSE;
}


/**
 * wmud_world_load:
 * @err: a #GError to put error messages into
 *
 * Load the world from the database backend without activating it.
 */
gboolean
wmud_world_load(GError **err)
{
	GSList *planes = NULL,
	       *planets = NULL,
	       *directions = NULL,
	       *areas = NULL,
	       *rooms = NULL,
	       *exits = NULL;
	GError *in_err = NULL;

	/* Load directions from the database */
	wmud_db_load_directions(&directions, &in_err);

	/* Load planes from the database */
	wmud_db_load_planes(&planes, &in_err);

	/* Check if the loaded planes conform to the rules */
	g_clear_error(&in_err);
	wmud_world_check_planes(planes, &in_err);

	/* Load planets from the database */
	g_clear_error(&in_err);
	wmud_db_load_planets(&planets, &in_err);

	/* Check if the planets conform to the rules */
	g_clear_error(&in_err);
	wmud_world_check_planets(planets, &in_err);

	/* Put the planets on the planes */
	g_clear_error(&in_err);
	wmud_world_assoc_planets_planes(planets, planes, &in_err);

	/* Load areas from the database */
	g_clear_error(&in_err);
	wmud_db_load_areas(&areas, &in_err);

	/* Check if the areas conform to the rules */
	g_clear_error(&in_err);
	wmud_world_check_areas(areas, &in_err);

	/* Load rooms from the database */
	g_clear_error(&in_err);
	wmud_db_load_rooms(&rooms, &in_err);

	/* Associate rooms with the areas */
	g_clear_error(&in_err);
	wmud_world_assoc_rooms_areas(rooms, areas, &in_err);

	/* Associate rooms with planets */
	g_clear_error(&in_err);
	wmud_world_assoc_rooms_planets(rooms, planets, &in_err);

	/* Load room exits from the database */
	g_clear_error(&in_err);
	wmud_db_load_exits(&exits, &in_err);

	/* Check if the exits conform to the rules */
	g_clear_error(&in_err);
	wmud_world_check_exits(exits, &in_err);

	/* Associate exits with rooms */
	g_clear_error(&in_err);
	wmud_world_assoc_exits_rooms(exits, rooms, &in_err);

	g_clear_error(&in_err);
	return TRUE;
}

