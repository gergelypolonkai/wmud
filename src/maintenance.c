/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * maintenance.c: game maintenance and self-check functions
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "wmud-types.h"
#include "maintenance.h"
#include "main.h"
#include "players.h"

/**
 * wmud_maintenance_check_new_players:
 * @player: #wmudPLayer structure of the player record to check
 * @user_data: not used
 *
 * Callback called from within the maintenance loop. Checks if the player has
 * an unset password, and generate one for them, if so.
 */
void
wmud_maintenance_check_new_players(wmudPlayer *player, gpointer user_data)
{
	if (player->cpassword == NULL)
	{
		gchar *pw,
		      *salt,
		      *cpw;
		GString *full_salt;

		pw = wmud_random_string(8);
		salt = wmud_random_string(8);
		full_salt = g_string_new("$1$");
		g_string_append(full_salt, salt);
		cpw = g_strdup(crypt(pw, full_salt->str));

		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Player %s has no"
				" password set", player->player_name);
		g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "New password will be %s", pw);
		player->cpassword = cpw;
		/* TODO: Send e-mail about the new password. Upon completion,
		 * set it in the database */

		g_free(pw);
		g_free(salt);
		g_string_free(full_salt, TRUE);
	}
}

/**
 * wmud_maintenance:
 * @user_data: not used
 *
 * Timeout source function for maintenance tasks
 */
gboolean
wmud_maintenance(gpointer user_data)
{
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Starting maintenance...");
	/* Run through the player list, and generate a random password for each
	 * newly registered player */
	g_slist_foreach(players, (GFunc)wmud_maintenance_check_new_players, NULL);

	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Finished maintenance...");

	return TRUE;
}

/**
 * maint_thread_func:
 * @main_loop: the main loop to be associated with the maintenance thread
 *
 * The maintenance thread's main function.
 *
 * Return value: This function always returns %NULL.
 */
gpointer
maint_thread_func(GMainLoop *maint_loop)
{
	g_main_loop_run(maint_loop);

	return NULL;
}

void
wmud_maintenance_init(void)
{
	GSource *timeout_source;
	GMainLoop *maint_loop;
	GMainContext *maint_context;
	GError *err = NULL;

	/* Create the maintenance context and main loop */
	maint_context = g_main_context_new();
	maint_loop = g_main_loop_new(maint_context, FALSE);

	/* Create the timeout source which will do the maintenance tasks */
	timeout_source = g_timeout_source_new_seconds(3);
	g_source_set_callback(timeout_source, wmud_maintenance, NULL, NULL);
	g_source_attach(timeout_source, maint_context);
	g_source_unref(timeout_source);

	g_thread_create((GThreadFunc)maint_thread_func, maint_loop, FALSE, &err);
}

