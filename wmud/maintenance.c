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
#include <curl/curl.h>
#include <string.h>

#include "wmud-types.h"
#include "maintenance.h"
#include "main.h"
#include "players.h"
#include "configuration.h"

/**
 * SECTION:maintenance-thread
 * @short_description: Runtime maintenance functions
 * @title: Runtime maintenance functions
 *
 */

/**
 * wmud_maintenance_check_players:
 * @player: #wmudPLayer structure of the player record to check
 * @user_data: not used
 *
 * Callback called from within the maintenance loop. Checks if the player has
 * an unset password, and generate one for them, if so.
 */
void
wmud_maintenance_check_players(wmudPlayer *player, gpointer user_data)
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
	g_slist_foreach(players, (GFunc)wmud_maintenance_check_players, NULL);

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

const char *text[] = {
	"To: Polonkai Gergely <polonkai.gergely@brokernet-group.com>\n",
	"Subject: Teszt\n",
	"\n",
	"Hello!\n"
};

struct WriteThis {
	int counter;
};

static size_t
wmud_smtp_read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct WriteThis *pooh = (struct WriteThis *)userp;
	const char *data;

	if (size * nmemb < 1)
	{
		return 0;
	}

	data = text[pooh->counter];

	if (data)
	{
		size_t len = strlen(data);
		memcpy(ptr, data, len);
		pooh->counter++;
		return len;
	}

	return 0;
}

void
wmud_maintenance_init(void)
{
	GSource *timeout_source;
	GMainLoop *maint_loop;
	GMainContext *maint_context;
	CURL *curl;
	CURLM *mcurl;
	gchar *smtp_server_real;
	struct WriteThis pooh = {0};

	curl_global_init(CURL_GLOBAL_DEFAULT);

	if (!(curl = curl_easy_init()))
	{
		g_error("Could not initialize the CURL library!");
	}

	if (!(mcurl = curl_multi_init()))
	{
		g_error("Could not initialize the CURL library!");
	}

	smtp_server_real = g_strconcat("smtp://", active_config->smtp_server, NULL);
	curl_easy_setopt(curl, CURLOPT_URL, smtp_server_real);
	g_free(smtp_server_real);
	if (active_config->smtp_username && active_config->smtp_password)
	{
		curl_easy_setopt(curl, CURLOPT_USERNAME, active_config->smtp_username);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, active_config->smtp_password);
	}
	curl_easy_setopt(curl, CURLOPT_MAIL_FROM, active_config->smtp_sender);
	curl_easy_setopt(curl, CURLOPT_USE_SSL, (CURLUSESSL_ALL && active_config->smtp_tls));
	/* TODO: Maybe these could go into the configuration as well */
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0L);
#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#else
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
#endif
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, wmud_smtp_read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);
	curl_multi_add_handle(mcurl, curl);

	/* Create the maintenance context and main loop */
	maint_context = g_main_context_new();
	maint_loop = g_main_loop_new(maint_context, FALSE);

	/* Create the timeout source which will do the maintenance tasks */
	timeout_source = g_timeout_source_new_seconds(3);
	g_source_set_callback(timeout_source, wmud_maintenance, NULL, NULL);
	g_source_attach(timeout_source, maint_context);
	g_source_unref(timeout_source);

#if GLIB_CHECK_VERSION(2,32,0)
	g_thread_new("maintenance", (GThreadFunc)maint_thread_func, maint_loop);
#else
	g_thread_create((GThreadFunc)maint_thread_func, maint_loop, TRUE, NULL);
#endif
}

