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
#include <sqlite3.h>

#include "world.h"
#include "main.h"
#include "db.h"
#include "players.h"
#include "configuration.h"

/**
 * SECTION:db
 * @short_description: Database handling
 * @title: Database handling routines
 *
 */

/**
 * WMUD_DB_ERROR:
 *
 * the GQuark for the database error GError
 */
GQuark WMUD_DB_ERROR = 0;
static sqlite3 *dbh = NULL;

/**
 * wmud_db_init:
 * @err: a GError to put error messages in it
 *
 * Initializes the wMUD database system. Checks and opens database files.
 */
gboolean
wmud_db_init(GError **err)
{
	GString *db_file = g_string_new(WMUD_STATEDIR);
	int sqlite_code;

	g_string_append_printf(db_file, "/%s", active_config->database_file);

	if ((sqlite_code = sqlite3_open(db_file->str, &dbh)) != SQLITE_OK)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_CANTOPEN, "Can not open databsae file (%s): %s", db_file->str, sqlite3_errmsg(dbh));

		return FALSE;
	}

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
	sqlite3_stmt *sth = NULL;
	int sqlite_code;

	if (dbh == NULL)
	{
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	if ((sqlite_code = sqlite3_prepare_v2(dbh, "SELECT id, login, password, email FROM players", -1, &sth, NULL)) != SQLITE_OK)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad query in wmud_db_load_players(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	while (1)
	{
		sqlite_code = sqlite3_step(sth);
		if (sqlite_code == SQLITE_ROW)
		{
			wmudPlayer *player = g_new0(wmudPlayer, 1);
			player->id = sqlite3_column_int(sth, 0);
			player->player_name = g_strdup((gchar *)sqlite3_column_text(sth, 1));
			player->cpassword = g_strdup((gchar *)sqlite3_column_text(sth, 2));
			player->email = g_strdup((gchar *)sqlite3_column_text(sth, 3));

			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded player _%s_", player->player_name);

			players = g_slist_prepend(players, player);
		}
		else if (sqlite_code == SQLITE_DONE)
		{
			break;
		}
		else
		{
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Query error in wmud_db_load_players(): %s", sqlite3_errmsg(dbh));
			sqlite3_finalize(sth);
			return FALSE;
		}
	}

	sqlite3_finalize(sth);

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
wmud_db_save_player(wmudPlayer *player, GError **err)
{
	sqlite3_stmt *sth = NULL;
	int sqlite_code;

	if (dbh == NULL)
	{
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	if ((sqlite_code = sqlite3_prepare_v2(dbh, "INSERT INTO players (id, login, password, email) VALUES (NULL, ?, NULL, ?)", -1, &sth, NULL)) != SQLITE_OK)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad query in wmud_db_player_save(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	if ((sqlite_code = sqlite3_bind_text(sth, 1, player->player_name, -1, SQLITE_STATIC)) != SQLITE_OK)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad parameter in wmud_db_player_save(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	if ((sqlite_code = sqlite3_bind_text(sth, 2, player->email, -1, SQLITE_STATIC)) != SQLITE_OK)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad parameter in wmud_db_player_save(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	if ((sqlite_code = sqlite3_step(sth)) != SQLITE_DONE)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Statement cannot be executed in wmud_db_player_save(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	g_clear_error(err);
	return TRUE;
}

gboolean
wmud_db_load_planes(GSList **planes, GError **err)
{
	sqlite3_stmt *sth = NULL;
	int sqlite_code;

	if (dbh == NULL)
	{
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	if ((sqlite_code = sqlite3_prepare_v2(dbh, "SELECT id, name FROM planes", -1, &sth, NULL)) != SQLITE_OK)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad query in wmud_db_load_planes(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	while (1)
	{
		sqlite_code = sqlite3_step(sth);
		if (sqlite_code == SQLITE_ROW)
		{
			wmudPlane *plane = g_new0(wmudPlane, 1);
			plane->id = sqlite3_column_int(sth, 0);
			plane->name = g_strdup((gchar *)sqlite3_column_text(sth, 1));

			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded plane _%s_", plane->name);

			*planes = g_slist_prepend(*planes, plane);
		}
		else if (sqlite_code == SQLITE_DONE)
		{
			break;
		}
		else
		{
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Query error in wmud_db_load_planes(): %s", sqlite3_errmsg(dbh));
			sqlite3_finalize(sth);
			return FALSE;
		}
	}

	sqlite3_finalize(sth);

	return TRUE;
}

gboolean
wmud_db_load_planets(GSList **planets, GError **err)
{
	return FALSE;
}

gboolean
wmud_db_load_directions(GSList **directions, GError **err)
{
	sqlite3_stmt *sth = NULL;
	int sqlite_code;

	if (dbh == NULL)
	{
		if (err)
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_NOINIT, "Database backend not initialized");

		return FALSE;
	}

	if ((sqlite_code = sqlite3_prepare_v2(dbh, "SELECT id, short_name, name FROM directions", -1, &sth, NULL)) != SQLITE_OK)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad query in wmud_db_load_directions(): %s", sqlite3_errmsg(dbh));

		return FALSE;
	}

	while (1)
	{
		sqlite_code = sqlite3_step(sth);
		if (sqlite_code == SQLITE_ROW)
		{
			wmudDirection *dir = g_new0(wmudDirection, 1);
			dir->id = sqlite3_column_int(sth, 0);
			dir->short_name = g_strdup((gchar *)sqlite3_column_text(sth, 1));
			dir->name = g_strdup((gchar *)sqlite3_column_text(sth, 2));

			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded direction _%s_", dir->name);

			*directions = g_slist_prepend(*directions, dir);
		}
		else if (sqlite_code == SQLITE_DONE)
		{
			break;
		}
		else
		{
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Query error in wmud_db_load_players(): %s", sqlite3_errmsg(dbh));
			return FALSE;
		}
	}

	sqlite3_finalize(sth);

	return TRUE;
}

gboolean
wmud_db_load_areas(GSList **areas, GError **err)
{
	return FALSE;
}

gboolean
wmud_db_load_rooms(GSList **rooms, GError **err)
{
	return FALSE;
}

gboolean
wmud_db_load_exits(GSList **exits, GError **err)
{
	return FALSE;
}

