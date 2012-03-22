#include <glib.h>
#include <sqlite3.h>

#include "main.h"
#include "db.h"
#include "players.h"

sqlite3 *dbh = NULL;

gboolean
wmud_db_init(GError **err)
{
	GString *db_file = g_string_new(WMUD_STATEDIR);
	int sqlite_code;

	g_string_append_printf(db_file, "/%s", database_file);

	if ((sqlite_code = sqlite3_open(db_file->str, &dbh)) != SQLITE_OK)
	{
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_CANTOPEN, "Can not open databsae file (%s): %s", db_file->str, sqlite3_errmsg(dbh));

		return FALSE;
	}

	return TRUE;
}

gboolean
wmud_db_players_load(GError **err)
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
		g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Bad query in wmud_db_players_load(): %s", sqlite3_errmsg(dbh));

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
			player->registering = (player->cpassword == NULL);

			g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Loaded player _%s_", player->player_name);

			players = g_slist_prepend(players, player);
		}
		else if (sqlite_code == SQLITE_DONE)
		{
			break;
		}
		else
		{
			g_set_error(err, WMUD_DB_ERROR, WMUD_DB_ERROR_BADQUERY, "Query error in wmud_db_players_load(): %s", sqlite3_errmsg(dbh));
			return FALSE;
		}
	}

	sqlite3_finalize(sth);

	return FALSE;
}

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

