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
#include "interpreter.h"

/**
 * SECTION:world
 * @short_description: World loading and building functions
 * @title: Game world manipulation
 *
 */

/**
 * wmud_world_check_planes:
 * @planes: a #GSList of wmudPlane structs
 * @err: a #GError where this function can send back error messages
 *
 * Check for duplicate plane names in the @planes list.
 *
 * Return value: %TRUE if there are no duplicate names, %FALSE otherwise
 */
gboolean
wmud_world_check_planes(GSList *planes, GError **err)
{
	return FALSE;
}

/**
 * wmud_world_check_planets:
 * @planets: a #GSList of wmudPlanet structs
 * @err: a #GError where this function can send back error messages
 *
 * Check for duplicate planet names in the @planets list.
 *
 * Return value: %TRUE if there are no duplicate names, %FALSE otherwise
 */
gboolean
wmud_world_check_planets(GSList *planets, GError **err)
{
	return FALSE;
}

/**
 * wmud_world_check_areas:
 * @areas: a #GSList of wmudArea structs
 * @err: a #GError where this function can send back error messages
 *
 * Check for duplicate area names in the @areas list.
 *
 * Return value: %TRUE if there are no duplicate names, %FALSE otherwise
 */
gboolean
wmud_world_check_areas(GSList *areas, GError **err)
{
	return FALSE;
}

/**
 * wmud_world_check_exits:
 * @exits: a #GSList of wmudExit structs
 * @directions: a #GSList of wmudDirection structs
 * @rooms: a #GSList of wmudRoom structs
 * @err: a #GError where this function can send back error messages
 *
 * Check if both endpoints of exits point to a valid direction, and lead to an
 * existing room.
 *
 * Return value: %TRUE if all the exits are valid, %FALSE otherwise
 */
gboolean
wmud_world_check_exits(GSList *exits, GSList *directions, GSList *rooms, GError **err)
{
	return FALSE;
}

/**
 * wmud_world_assoc_planets_planes:
 * @planets: a #GSList of wmudPlanets
 * @planes: a #GSList of wmudPlanes
 * @err: a #GError where this function can send back error messages
 *
 * Associates planets with planes by adding the required planes to the
 * wmudPlanets' planes list.
 *
 * Return value: %FALSE if there are planets without planes, or if a planet has
 * a plane database record that points to a nonexistant plane; %TRUE otherwise.
 */
gboolean
wmud_world_assoc_planets_planes(GSList *planets, GSList *planes, GError **err)
{
	return FALSE;
}

/**
 * wmud_world_assoc_rooms_areas:
 * @rooms: a #GSList of wmudRooms
 * @areas: a #GSList of wmudAreas
 * @err: a #GError where this function can send back error messages
 *
 * Associates rooms with areas by adding the required areas to the wmudRooms'
 * area list and the rooms to the wmudAreas' rooms list.
 *
 * Return value: %FALSE if there are any associations between a nonexistant
 * area or a nonexistant room.
 */
gboolean
wmud_world_assoc_rooms_areas(GSList *rooms, GSList *areas, GError **err)
{
	return FALSE;
}

/**
 * wmud_world_assoc_rooms_planets:
 * @rooms: a #GSList of wmudRooms
 * @planets: a #GSList of wmudPlanets
 * @err: a #GError where this function can send back error messages
 *
 * Associates rooms with planets, and through them with planes.
 *
 * Return value: %FALSE if a room has no planes associated with them, %TRUE
 * otherwise.
 */
gboolean
wmud_world_assoc_rooms_planets(GSList *rooms, GSList *planets, GError **err)
{
	return FALSE;
}

/**
 * wmud_world_assoc_exits_rooms:
 * @exits: a #GSList of wmudExits
 * @rooms: a #GSList of wmudRooms
 * @err: a #GError where this function can send back error messages
 *
 * Associate exits with rooms and vice versa.
 */
void
wmud_world_assoc_exits_rooms(GSList *exits, GSList *rooms, GError **err)
{
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

	/* Check if the loaded directions conform to the rules */
	g_clear_error(&in_err);
	wmud_interpreter_check_directions(directions, &in_err);

	/* Load planes from the database */
	g_clear_error(&in_err);
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

