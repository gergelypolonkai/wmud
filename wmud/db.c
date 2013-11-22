/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * db.c: database handling routines
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
#include <libgda/libgda.h>
#include <sql-parser/gda-sql-parser.h>

#include "world.h"
#include "main.h"
#include "db.h"
#include "players.h"
#include "configuration.h"
#include "menu.h"
#include "wmudplayer.h"

/**
 * SECTION:db
 * @short_description: Database handling
 * @title: Database handling routines
 *
 * This module handles all the database requests. It currently uses an SQLite3
 * backend, but due to its nature, it could be easily rewritten to use a
 * different (e.g MySQL or PostgreSQL) database.
 */

static GdaConnection *dbh = NULL;
static GdaSqlParser *parser = NULL;

GQuark
wmud_db_error_quark()
{
	return g_quark_from_static_string("wmud-db-error");
}

/**
 * wmud_db_init:
 * @err: a GError to put error messages in it
 *
 * Initializes the wMUD database system. Checks and opens database files.
 */
gboolean
wmud_db_init(GError **err)
{
	GError *local_err = NULL;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Initializing database");

	gda_init();

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Database DSN is \"%s\"", active_config->database_dsn);
	dbh = gda_connection_open_from_string(NULL, active_config->database_dsn, NULL, GDA_CONNECTION_OPTIONS_THREAD_SAFE, &local_err);

	if (dbh == NULL) {
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_CANTOPEN, "Can not open databsae (%s): %s", active_config->database_dsn, local_err->message);

		return FALSE;
	}

	parser = gda_sql_parser_new();

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Database initialization finished.");

	return TRUE;
}

/**
 * wmud_db_load_players:
 * @err: a GError to put error messages in it
 *
 * Loads all player records from the database
 */
gboolean
wmud_db_load_players(GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res;
	GdaDataModelIter *iter;
	GError *local_err = NULL;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading players");
	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT id, login, password, email FROM players", NULL, NULL);

	/* TODO: error checking! */
	if ((res = gda_connection_statement_execute_select(dbh, sth, NULL, &local_err)) == NULL) {
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Unable to load players: %s", local_err->message);
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_SELECT_ERROR, "SELECT error: %s", local_err->message);

		return FALSE;
	}

	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		WmudPlayer *player;

		player = wmud_player_new();

		val = gda_data_model_iter_get_value_at(iter, 0);
		wmud_player_set_id(player, g_value_get_int(val));

		val = gda_data_model_iter_get_value_at(iter, 1);
		wmud_player_set_player_name(player, g_value_get_string(val));

		val = gda_data_model_iter_get_value_at(iter, 2);
		wmud_player_set_cpassword(player, g_value_get_string(val));

		val = gda_data_model_iter_get_value_at(iter, 3);
		wmud_player_set_email(player, g_value_get_string(val));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded player _%s_", wmud_player_get_player_name(player));

		players = g_slist_prepend(players, player);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Finished loading players");

	return FALSE;
}

/**
 * wmud_db_save_player:
 * @player: the player record to save
 * @err: a GError to put error messages in it
 *
 * Saves a player record to the database backend.
 *
 * Return value: %TRUE on success. Upon failure, %FALSE is returned, and err is
 *               set accordingly.
 */
gboolean
wmud_db_save_player(WmudPlayer *player, GError **err)
{
	/*
	sqlite3_stmt *sth = NULL;
	int sqlite_code;

	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	if ((sqlite_code = sqlite3_prepare_v2(dbh, "INSERT INTO players (id, login, password, email) VALUES (NULL, ?, NULL, ?)", -1, &sth, NULL)) != SQLITE_OK) {
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad query in wmud_db_save_player(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	if ((sqlite_code = sqlite3_bind_text(sth, 1, wmud_player_get_player_name(player), -1, SQLITE_STATIC)) != SQLITE_OK) {
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad parameter in wmud_db_save_player(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	if ((sqlite_code = sqlite3_bind_text(sth, 2, wmud_player_get_email(player), -1, SQLITE_STATIC)) != SQLITE_OK) {
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad parameter in wmud_db_save_player(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	if ((sqlite_code = sqlite3_step(sth)) != SQLITE_DONE) {
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Statement cannot be executed in wmud_db_save_player(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	g_clear_error(err);
	return TRUE;
	*/
	return FALSE;
}

gboolean
wmud_db_load_planes(GSList **planes, GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res = NULL;
	GdaDataModelIter *iter;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading planes");
	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT id, name FROM planes", NULL, NULL);
	res = gda_connection_statement_execute_select(dbh, sth, NULL, NULL);
	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		wmudPlane *plane;

		plane = g_new0(wmudPlane, 1);

		val = gda_data_model_iter_get_value_at(iter, 0);
		plane->id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 1);
		plane->name = g_strdup(g_value_get_string(val));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded plane _%s_", plane->name);

		*planes = g_slist_prepend(*planes, plane);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);

	return TRUE;
}

gboolean
wmud_db_load_planets(GSList **planets, GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res = NULL;
	GdaDataModelIter *iter;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading planets");
	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT id, name FROM planets", NULL, NULL);
	res = gda_connection_statement_execute_select(dbh, sth, NULL, NULL);
	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		wmudPlanet *planet;

		planet = g_new0(wmudPlanet, 1);

		val = gda_data_model_iter_get_value_at(iter, 0);
		planet->id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 1);
		planet->name = g_strdup(g_value_get_string(val));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded planet _%s_", planet->name);

		*planets = g_slist_prepend(*planets, planet);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);

	return TRUE;
}

gboolean
wmud_db_load_directions(GSList **directions, GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res = NULL;
	GdaDataModelIter *iter;
	GError *local_err = NULL;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading directions");

	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT id, short_name, name FROM directions", NULL, NULL);

	if ((res = gda_connection_statement_execute_select(dbh, sth, NULL, &local_err)) == NULL) {
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_SELECT_ERROR, "Unable to load directions: %s", local_err->message);

		return FALSE;
	}

	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		wmudDirection *dir;

		dir = g_new0(wmudDirection, 1);

		val = gda_data_model_iter_get_value_at(iter, 0);
		dir->id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 1);
		dir->short_name = g_strdup(g_value_get_string(val));

		val = gda_data_model_iter_get_value_at(iter, 2);
		dir->name = g_strdup(g_value_get_string(val));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded direction _%s_", dir->name);

		*directions = g_slist_prepend(*directions, dir);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);

	return TRUE;
}

gboolean
wmud_db_load_areas(GSList **areas, GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res = NULL;
	GdaDataModelIter *iter;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading areas");
	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT id, name FROM areas", NULL, NULL);
	res = gda_connection_statement_execute_select(dbh, sth, NULL, NULL);
	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		wmudArea *area;

		area = g_new0(wmudArea, 1);

		val = gda_data_model_iter_get_value_at(iter, 0);
		area->id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 1);
		area->name = g_strdup(g_value_get_string(val));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded area _%s_", area->name);

		*areas = g_slist_prepend(*areas, area);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);

	return TRUE;
}

gboolean
wmud_db_load_rooms(GSList **rooms, GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res = NULL;
	GdaDataModelIter *iter;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading rooms");
	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT id, area, name, distant_description, close_description FROM rooms", NULL, NULL);
	res = gda_connection_statement_execute_select(dbh, sth, NULL, NULL);
	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		wmudRoom *room;

		room = g_new0(wmudRoom, 1);

		val = gda_data_model_iter_get_value_at(iter, 0);
		room->id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 1);
		room->area_id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 2);
		room->name = g_strdup(g_value_get_string(val));

		val = gda_data_model_iter_get_value_at(iter, 3);
		room->distant_description = g_strdup(g_value_get_string(val));

		val = gda_data_model_iter_get_value_at(iter, 4);
		room->close_description = g_strdup(g_value_get_string(val));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded room %d/_%s_", room->area_id, room->name);

		*rooms = g_slist_prepend(*rooms, room);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);

	return TRUE;
}

gboolean
wmud_db_load_exits(GSList **exits, GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res = NULL;
	GdaDataModelIter *iter;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading exits");
	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT room_id, direction, other_side FROM room_exits", NULL, NULL);
	res = gda_connection_statement_execute_select(dbh, sth, NULL, NULL);
	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		wmudExit *room_exit;

		room_exit = g_new0(wmudExit, 1);

		val = gda_data_model_iter_get_value_at(iter, 0);
		room_exit->source_room_id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 1);
		room_exit->direction_id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 2);
		room_exit->destination_room_id = g_value_get_int(val);

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded exit %d =%d=> %d", room_exit->source_room_id, room_exit->direction_id, room_exit->destination_room_id);

		*exits = g_slist_prepend(*exits, room_exit);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);

	return TRUE;
}

gboolean
wmud_db_load_planet_planes(GSList **planet_planes, GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res = NULL;
	GdaDataModelIter *iter;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading planet<->plane associations");
	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT planet_id, plane_id FROM planet_planes", NULL, NULL);
	res = gda_connection_statement_execute_select(dbh, sth, NULL, NULL);
	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		wmudPlanetPlaneAssoc *planet_plane;

		planet_plane = g_new0(wmudPlanetPlaneAssoc, 1);

		val = gda_data_model_iter_get_value_at(iter, 0);
		planet_plane->planet_id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 1);
		planet_plane->plane_id = g_value_get_int(val);

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded planet-plane association %d <> %d", planet_plane->planet_id, planet_plane->plane_id);

		*planet_planes = g_slist_prepend(*planet_planes, planet_plane);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);

	return TRUE;
}

gboolean
wmud_db_load_menu(GSList **menu_items, GError **err)
{
	GdaStatement *sth = NULL;
	GdaDataModel *res = NULL;
	GdaDataModelIter *iter;

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loading menu items");
	if (dbh == NULL) {
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	sth = gda_sql_parser_parse_string(parser, "SELECT id, menuchar, need_active_char, placement, display_text, fnctn FROM menu ORDER BY placement", NULL, NULL);
	res = gda_connection_statement_execute_select(dbh, sth, NULL, NULL);
	iter = gda_data_model_create_iter(res);
	gda_data_model_iter_move_next(iter);

	while (gda_data_model_iter_is_valid(iter)) {
		const GValue *val;
		wmudMenu *menu_item;

		menu_item = g_new0(wmudMenu, 1);

		val = gda_data_model_iter_get_value_at(iter, 0);
		menu_item->id = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 1);
		menu_item->menuchar = *(g_value_get_string(val));

		val = gda_data_model_iter_get_value_at(iter, 2);
		menu_item->need_active_char = g_value_get_boolean(val);

		val = gda_data_model_iter_get_value_at(iter, 3);
		menu_item->placement = g_value_get_int(val);

		val = gda_data_model_iter_get_value_at(iter, 4);
		menu_item->text = g_strdup(g_value_get_string(val));

		val = gda_data_model_iter_get_value_at(iter, 5);
		menu_item->func = g_strdup(g_value_get_string(val));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded menu item %d: %s(%c)", menu_item->id, menu_item->text, menu_item->menuchar);

		*menu_items = g_slist_prepend(*menu_items, menu_item);

		gda_data_model_iter_move_next(iter);
	}

	g_object_unref(iter);
	g_object_unref(sth);

	return TRUE;
}

