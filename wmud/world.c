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

struct findData {
	GSList *list;
	guint found;
	gchar *last;
};
struct dirCheckData {
	GSList *directions;
	GSList *rooms;
	gboolean sane;
};

GQuark
wmud_world_error_quark()
{
	return g_quark_from_static_string("wmud-world-error");
}

static gint
check_plane_dups2(wmudPlane *plane, gchar *name)
{
	return (g_ascii_strcasecmp(plane->name, name));
}

static void
check_plane_dups1(wmudPlane *plane, struct findData *find_data)
{
	if (find_data->last != plane->name)
	{
		find_data->last = plane->name;
		find_data->found = (find_data->found > 1) ? find_data->found : 0;
	}

	if (g_slist_find_custom(find_data->list, plane->name, (GCompareFunc)check_plane_dups2))
		find_data->found++;
}

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
	struct findData find_data = {planes, 0, NULL};

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Checking plane names for duplicates");

	g_slist_foreach(planes, (GFunc)check_plane_dups1, &find_data);

	if (find_data.found > 1)
		g_set_error(err, WMUD_WORLD_ERROR, WMUD_WORLD_ERROR_DUPPLANE, "Duplicate plane names found. Please check your database!\n");

	return (find_data.found < 2);
}

static gint
check_planet_dups2(wmudPlanet *planet, gchar *name)
{
	return (g_ascii_strcasecmp(planet->name, name));
}

static void
check_planet_dups1(wmudPlanet *planet, struct findData *find_data)
{
	if (find_data->last != planet->name)
	{
		find_data->last = planet->name;
		find_data->found = (find_data->found > 1) ? find_data->found : 0;
	}

	if (g_slist_find_custom(find_data->list, planet->name, (GCompareFunc)check_planet_dups2))
		find_data->found++;
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
	struct findData find_data = {planets, 0, NULL};

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Checking planet names for duplicates");

	g_slist_foreach(planets, (GFunc)check_planet_dups1, &find_data);

	if (find_data.found > 1)
		g_set_error(err, WMUD_WORLD_ERROR, WMUD_WORLD_ERROR_DUPPLANET, "Duplicate planet names found. Please check your database!\n");

	return (find_data.found < 2);
}

static gint
check_area_dups2(wmudArea *area, gchar *name)
{
	return (g_ascii_strcasecmp(area->name, name));
}

static void
check_area_dups1(wmudArea *area, struct findData *find_data)
{
	if (find_data->last != area->name)
	{
		find_data->last = area->name;
		find_data->found = (find_data->found > 1) ? find_data->found : 0;
	}

	if (g_slist_find_custom(find_data->list, area->name, (GCompareFunc)check_area_dups2))
		find_data->found++;
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
	struct findData find_data = {areas, 0, NULL};

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Checking area names for duplicates");

	g_slist_foreach(areas, (GFunc)check_area_dups1, &find_data);

	if (find_data.found > 1)
		g_set_error(err, WMUD_WORLD_ERROR, WMUD_WORLD_ERROR_DUPAREA, "Duplicate area names found. Please check your database!");

	return (find_data.found < 2);
}

static gint
check_room_exit_room(wmudRoom *room, wmudExit *room_exit)
{
	return ((room->id == room_exit->source_room_id) || (room->id == room_exit->destination_room_id)) ? 0 : 1;
}

static gint
check_room_exit_dir(wmudDirection *dir, wmudExit *room_exit)
{
	return (dir->id == room_exit->direction_id) ? 0 : 1;
}

static void
exit_sanity_check(wmudExit *room_exit, struct dirCheckData *dir_check_data)
{
	if (
		!g_slist_find_custom(dir_check_data->rooms, room_exit, (GCompareFunc)check_room_exit_room)
		|| !g_slist_find_custom(dir_check_data->directions, room_exit, (GCompareFunc)check_room_exit_dir)
	)
		dir_check_data->sane = FALSE;
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
	struct dirCheckData dir_check_data = {directions, rooms, TRUE};

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Sanity checking on room exits");

	g_slist_foreach(exits, (GFunc)exit_sanity_check, &dir_check_data);

	return dir_check_data.sane;
}

static gint
check_room_dups2(wmudRoom *room1, wmudRoom *room2)
{
	if (room1->area_id == room2->area_id)
	{
		return g_ascii_strcasecmp(room1->name, room2->name);
	}
	return 1;
}

static void
check_room_dups1(wmudRoom *room, struct findData *find_data)
{
	if (find_data->last != (gchar *)room)
	{
		find_data->last = (gchar *)room;
		find_data->found = (find_data->found > 1) ? find_data->found : 0;
	}

	if (g_slist_find_custom(find_data->list, room, (GCompareFunc)check_room_dups2))
		find_data->found++;
}

/**
 * wmud_world_check_rooms:
 * @rooms: a #GSList of wmudRoom structs
 * @err: a #GError where this function can send back error messages
 *
 * Check rooms.
 *
 * Return value: %TRUE if everything goes well, %FALSE otherwise
 */
gboolean
wmud_world_check_rooms(GSList *rooms, GError **err)
{
	struct findData find_data = {rooms, 0, NULL};

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Checking room consistency");

	g_slist_foreach(rooms, (GFunc)check_room_dups1, &find_data);

	if (find_data.found > 1)
		g_set_error(err, WMUD_WORLD_ERROR, WMUD_WORLD_ERROR_DUPROOM, "Duplicate room names found. Please check your database!");

	return (find_data.found < 2);
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

static void
free_direction(wmudDirection *dir)
{
	if (dir->short_name)
		g_free(dir->short_name);
	if (dir->name)
		g_free(dir->name);
	g_free(dir);
}

void
wmud_world_free_directions(GSList *directions)
{
	if (directions)
	{
#if GLIB_CHECK_VERSION(2, 28, 0)
		g_slist_free_full(directions, (GDestroyNotify)free_direction);
#else
		g_slist_foreach(directions, (GFunc)free_direction, NULL);
		g_slist_free(directions);
#endif
	}
}

static void
free_plane(wmudPlane *plane)
{
	if (plane->name)
		g_free(plane->name);

	g_free(plane);
}

void
wmud_world_free_planes(GSList *planes)
{
	if (planes)
	{
#if GLIB_CHECK_VERSION(2, 28, 0)
		g_slist_free_full(planes, (GDestroyNotify)free_plane);
#else
		g_slist_foreach(planes, (GFunc)free_plane, NULL);
		g_slist_free(planes);
#endif
	}
}

static void
free_planet(wmudPlanet *planet)
{
	if (planet->name)
		g_free(planet->name);

	g_free(planet);
}

void
wmud_world_free_planets(GSList *planets)
{
	if (planets)
	{
#if GLIB_CHECK_VERSION(2, 28, 0)
		g_slist_free_full(planets, (GDestroyNotify)free_planet);
#else
		g_slist_foreach(planets, (GFunc)free_planet, NULL);
		g_slist_free(planets);
#endif
	}
}

static void
free_area(wmudArea *area)
{
	if (area->name)
		g_free(area->name);

	g_free(area);
}

void
wmud_world_free_areas(GSList *areas)
{
	if (areas)
	{
#if GLIB_CHECK_VERSION(2, 28, 0)
		g_slist_free_full(areas, (GDestroyNotify)free_area);
#else
		g_slist_foreach(areas, (GFunc)free_area, NULL);
		g_slist_free(areas);
#endif
	}
}

static void
free_room(wmudRoom *room)
{
	if (room->name)
		g_free(room->name);
	if (room->distant_description)
		g_free(room->distant_description);
	if (room->close_description)
		g_free(room->close_description);

	g_free(room);
}

void
wmud_world_free_rooms(GSList *rooms)
{
	if (rooms)
	{
#if GLIB_CHECK_VERSION(2, 28, 0)
		g_slist_free_full(rooms, (GDestroyNotify)free_room);
#else
		g_slist_foreach(rooms, (GFunc)free_room, NULL);
		g_slist_free(rooms);
#endif
	}
}

void
wmud_world_free_exits(GSList *exits)
{
	if (exits)
	{
#if GLIB_CHECK_VERSION(2, 28, 0)
		g_slist_free_full(exits, (GDestroyNotify)g_free);
#else
		g_slist_foreach(exits, (GFunc)g_free, NULL);
		g_slist_free(exits);
#endif
	}
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

	/* Load directions from the database and check them */
	if (!wmud_db_load_directions(&directions, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Could not load directions from database: %s", in_err->message);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	if (!directions)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "No directions were found in the database!");
		g_clear_error(&in_err);

		return FALSE;
	}
	g_clear_error(&in_err);
	if (!wmud_interpreter_check_directions(directions, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Direction list pre-flight check error: %s", in_err->message);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}

	/* Load planes from the database and check them */
	g_clear_error(&in_err);
	if (!wmud_db_load_planes(&planes, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Could not load planes from database: %s", in_err->message);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	if (!planes)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "No planes were found in the database!");
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	g_clear_error(&in_err);
	if (!wmud_world_check_planes(planes, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Plane list pre-flight check error: %s", in_err->message);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}

	/* Load planets from the database and check them */
	g_clear_error(&in_err);
	if (!wmud_db_load_planets(&planets, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Could not load planets from database: %s", in_err->message);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	if (!planets)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "No planets were found in the database!");
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	g_clear_error(&in_err);
	if (!wmud_world_check_planets(planets, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Planet list pre-flight check error: %s", in_err->message);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}

	/* Load areas from the database and check them */
	g_clear_error(&in_err);
	if (!wmud_db_load_areas(&areas, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Could not load areas from database: %s", in_err->message);
		wmud_world_free_areas(areas);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	if (!areas)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "No areas were found in the database!");
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	g_clear_error(&in_err);
	if (!wmud_world_check_areas(areas, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Area list pre-flight check error: %s", in_err->message);
		wmud_world_free_areas(areas);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}

	/* Load rooms from the database and check them */
	g_clear_error(&in_err);
	if (!wmud_db_load_rooms(&rooms, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Could not load areas from database: %s", in_err->message);
		wmud_world_free_rooms(rooms);
		wmud_world_free_areas(areas);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	if (!rooms)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "No rooms were found in the database!");
		wmud_world_free_areas(areas);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	g_clear_error(&in_err);
	if (!wmud_world_check_rooms(rooms, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Room list pre-flight check error: %s", in_err->message);
		wmud_world_free_rooms(rooms);
		wmud_world_free_areas(areas);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}

	/* Load room exits from the database and check them */
	g_clear_error(&in_err);
	if (!wmud_db_load_exits(&exits, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Could not load exits from database: %s", in_err->message);
		wmud_world_free_exits(exits);
		wmud_world_free_rooms(rooms);
		wmud_world_free_areas(areas);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	if (!exits)
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "No exits were found in the database!");
		wmud_world_free_rooms(rooms);
		wmud_world_free_areas(areas);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}
	g_clear_error(&in_err);
	if (!wmud_world_check_exits(exits, directions, rooms, &in_err))
	{
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Exit list pre-flight check error: %s", in_err->message);
		wmud_world_free_rooms(rooms);
		wmud_world_free_areas(areas);
		wmud_world_free_planets(planets);
		wmud_world_free_planes(planes);
		wmud_world_free_directions(directions);
		g_clear_error(&in_err);

		return FALSE;
	}

	/* World loading finished. Now let's tie the parts together... */

	/* Put the planets on the planes */
	g_clear_error(&in_err);
	wmud_world_assoc_planets_planes(planets, planes, &in_err);

	/* Generate the areas */
	g_clear_error(&in_err);
	wmud_world_assoc_rooms_areas(rooms, areas, &in_err);

	/* Teleport the previously built areas to the planets */
	g_clear_error(&in_err);
	wmud_world_assoc_rooms_planets(rooms, planets, &in_err);

	/* And finally, create the doors between the rooms */
	g_clear_error(&in_err);
	wmud_world_assoc_exits_rooms(exits, rooms, &in_err);

	g_clear_error(&in_err);
	return TRUE;
}

