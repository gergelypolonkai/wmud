/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * menu.c: menu handling routines
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

#include "menu.h"
#include "db.h"

#include "menu.h"

/**
 * SECTION:menu
 * @short_description: Menu handling
 * @title: Menu handling routines
 *
 */

GHashTable *mcmd_table = NULL;

/**
 * game_menu:
 *
 * The list of menu items to display after a successful login
 */
GSList *game_menu = NULL;

GQuark
wmud_menu_error_quark()
{
    return g_quark_from_static_string("wmud-menu-error");
}

gboolean
wmud_menu_items_check(GSList *menu_items, GError **err)
{
    /* TODO: Check for duplicate menuchars */
    /* TODO: Check for duplicate menu texts */
    /* TODO: Check for duplicate placements */

    return TRUE;
}

void
menu_item_free(wmudMenu *menu_item)
{
    if (menu_item->text) {
        g_free(menu_item->text);
    }

    if (menu_item->display_text) {
        g_free(menu_item->display_text);
    }

    if (menu_item->display_text_ansi) {
        g_free(menu_item->display_text_ansi);
    }

    if (menu_item->func) {
        g_free(menu_item->func);
    }

    g_free(menu_item);
}

void
wmud_menu_items_free(GSList **menu_items)
{
    if (menu_items) {
#if GLIB_CHECK_VERSION(2, 28, 0)
        g_slist_free_full(*menu_items, (GDestroyNotify)menu_item_free);
#else
        g_slist_foreach(*menu_items, (GFunc)menu_item_free, NULL);
        g_slist_free(*menu_items);
#endif

        *menu_items = NULL;
    }
}

void
menu_item_prepare(wmudMenu *item, GHashTable *cmdtable)
{
    gchar   m1, m2;
    gchar   *a,
            *found = NULL;
    GString *ds, *dsa;

    g_debug("Preparing menu item %s", item->text);
    m1 = g_ascii_tolower(item->menuchar);
    m2 = g_ascii_toupper(item->menuchar);
    for (a = item->text; *a; a++) {
        if ((*a == m1) || (*a == m2)) {
            found = a;

            break;
        }
    }

    if (found) {
        gchar *tmp;

        tmp = g_ascii_strdown(item->text, -1);
        ds  = g_string_new(tmp);
        dsa = g_string_new(tmp);
        g_free(tmp);

        ds->str[found - item->text]  = g_ascii_toupper(item->menuchar);
        dsa->str[found - item->text] = g_ascii_toupper(item->menuchar);
    } else {
        found = item->text;
        ds    = g_string_new(item->text);
        dsa   = g_string_new(item->text);

        g_string_prepend_c(ds, ' ');
        g_string_prepend_c(ds, g_ascii_toupper(item->menuchar));
        g_string_prepend_c(dsa, ' ');
        g_string_prepend_c(dsa, g_ascii_toupper(item->menuchar));
    }

    g_string_insert_c(ds, found - item->text, '(');
    g_string_insert_c(ds, found - item->text + 2, ')');

    g_string_insert(dsa, found - item->text, "\x1b[31;1m");
    g_string_insert(dsa, found - item->text + 8, "\x1b[0m");
    item->display_text      = g_string_free(ds, FALSE);
    item->display_text_ansi = g_string_free(dsa, FALSE);
}

WMUD_MENU_COMMAND(enter_world)
{
}

WMUD_MENU_COMMAND(change_password)
{
}

WMUD_MENU_COMMAND(toggle_colour)
{
}

WMUD_MENU_COMMAND(documentation)
{
}

WMUD_MENU_COMMAND(character_select)
{
}

WMUD_MENU_COMMAND(character_create)
{
}

WMUD_MENU_COMMAND(character_delete)
{
}

WMUD_MENU_COMMAND(chat)
{
}

WMUD_MENU_COMMAND(player_mail)
{
}

WMUD_MENU_COMMAND(colour_test)
{
}

WMUD_MENU_COMMAND(change_email)
{
}

WMUD_MENU_COMMAND(change_name)
{
}

WMUD_MENU_COMMAND(quit)
{
    wmud_client_set_state(client, WMUD_CLIENT_STATE_YESNO);
    wmud_client_set_yesno_callback(client, wmud_client_quitanswer);
    wmud_client_send(client, "Are you sure you want to get back to the real world? [y/N] ");
}

WMUD_MENU_COMMAND(redisplay_menu)
{
    wmud_menu_present(client);
}

gboolean
wmud_menu_init(GSList **menu)
{
    GSList     *menu_items = NULL;
    GError     *in_err     = NULL;
    GHashTable *cmdtable;

    if (!wmud_db_load_menu(&menu_items, &in_err)) {
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Unable to load menu items from the database: %s", in_err->message);
        wmud_menu_items_free(&menu_items);

        return FALSE;
    }

    if (!menu_items) {
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "No menu items were found in the database!");

        return FALSE;
    }

    if (!wmud_menu_items_check(menu_items, &in_err)) {
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Menu items pre-flight check error: %s", in_err->message);
        wmud_menu_items_free(&menu_items);

        return FALSE;
    }

    if (*menu) {
        wmud_menu_items_free(menu);
    }

    *menu = menu_items;

    cmdtable = g_hash_table_new(g_str_hash, g_str_equal);

    g_slist_foreach(*menu, (GFunc)menu_item_prepare, cmdtable);

    g_hash_table_insert(cmdtable, "enter-world", wmud_mcmd_enter_world);
    g_hash_table_insert(cmdtable, "change-password", wmud_mcmd_change_password);
    g_hash_table_insert(cmdtable, "toggle-colour", wmud_mcmd_toggle_colour);
    g_hash_table_insert(cmdtable, "documentation", wmud_mcmd_documentation);
    g_hash_table_insert(cmdtable, "caracter-select", wmud_mcmd_character_select);
    g_hash_table_insert(cmdtable, "character-create", wmud_mcmd_character_create);
    g_hash_table_insert(cmdtable, "character-delete", wmud_mcmd_character_delete);
    g_hash_table_insert(cmdtable, "chat", wmud_mcmd_chat);
    g_hash_table_insert(cmdtable, "player-mail", wmud_mcmd_player_mail);
    g_hash_table_insert(cmdtable, "colour-test", wmud_mcmd_colour_test);
    g_hash_table_insert(cmdtable, "change-email", wmud_mcmd_change_email);
    g_hash_table_insert(cmdtable, "change-name", wmud_mcmd_change_name);
    g_hash_table_insert(cmdtable, "quit", wmud_mcmd_quit);
    g_hash_table_insert(cmdtable, "redisplay-menu", wmud_mcmd_redisplay_menu);

    /* TODO: Free previous hash table, if exists */
    mcmd_table = cmdtable;

    return TRUE;
}

static gint
find_by_menuchar(wmudMenu *item, gchar *menuchar)
{
    if (g_ascii_toupper(*menuchar) == g_ascii_toupper(item->menuchar)) {
        return 0;
    }

    return 1;
}

gchar *
wmud_menu_get_command_by_menuchar(gchar menuchar, GSList *game_menu)
{
    GSList *item;

    if ((item = g_slist_find_custom(game_menu, &menuchar, (GCompareFunc)find_by_menuchar)) != NULL) {
        return ((wmudMenu *)(item->data))->func;
    }

    return NULL;
}

void
wmud_menu_execute_command(WmudClient *client, gchar *command)
{
    wmudMenuCommandFunc func;

    if ((func = g_hash_table_lookup(mcmd_table, command)) == NULL) {
        wmud_client_send(client, "Unknown menu command.\r\n");
    } else {
        func(client);
    }
}

void
send_menu_item(wmudMenu *item, WmudClient *client)
{
    /* TODO: Send ANSI menu item only to ANSI players! */
    wmud_client_send(client, "%s\r\n", item->display_text_ansi);
}

void
wmud_menu_present(WmudClient *client)
{
    g_slist_foreach(game_menu, (GFunc)send_menu_item, client);
    wmud_client_set_state(client, WMUD_CLIENT_STATE_MENU);

    /* TODO: send menu prologue */
}
