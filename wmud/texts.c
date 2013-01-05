/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * texts.c: static text and template handling functions
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
#include <gio/gio.h>

#include "game-networking.h"
#include "texts.h"

static const gchar *text_files[] = {
	"motd",
	NULL
};
GHashTable *text_table = NULL;

void
wmud_texts_init(void)
{
	int i;
	gchar *texts_dir = g_strconcat(WMUD_STATEDIR, "/texts/", NULL);

	text_table = g_hash_table_new(g_str_hash, g_str_equal);

	for (i = 0; i < g_strv_length((gchar **)text_files); i++) {
		GFile *tf;
		GFileInfo *tfi;
		GError *err = NULL;
		guint64 tfs;
		gchar *contents;
		gsize length;

		gchar *text_file = g_strconcat(texts_dir, text_files[i], NULL);
		g_debug("Loading text file %s from %s", text_files[i], text_file);
		tf = g_file_new_for_path(text_file);
		tfi = g_file_query_info(tf, G_FILE_ATTRIBUTE_STANDARD_SIZE, G_FILE_QUERY_INFO_NONE, NULL, &err);

		if (err) {
			g_warning("Error loading %s: %s", text_files[i], err->message);
			continue;
		}

		tfs = g_file_info_get_attribute_uint64(tfi, G_FILE_ATTRIBUTE_STANDARD_SIZE);

		contents = g_malloc0(tfs + 1);

		g_clear_error(&err);

		if (!g_file_load_contents(tf, NULL, &contents, &length, NULL, &err)) {
			g_object_unref(tfi);
			g_object_unref(tf);
			continue;
		}

		g_hash_table_insert(text_table, (char *)text_files[i], contents);

		g_object_unref(tfi);
		g_object_unref(tf);
		g_free(text_file);
	}

	g_free(texts_dir);
}

void
wmud_text_send_to_client(gchar *text_name, WmudClient *client)
{
	gchar *text = g_hash_table_lookup(text_table, text_name);
	wmud_client_send(client, "%s\r\n", text);
}
