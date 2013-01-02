/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * menu.h: menu handling routines
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
#ifndef __WMUD_MENU_H__
#define __WMUD_MENU_H__

#include "game-networking.h"
#include <glib.h>

/**
 * wmudMenu:
 * @id: The databas ID of the menu item
 * @menuchar: The highlighted character of the menu item. Players must enter it
 *            in the menu to activate the menu item
 * @need_active_char: If %TRUE, the menu item will be displayed only if the
 *                    player has a character selected
 * @placement: The order number of the menu item. The whole menu is sorted
 *             based on this field
 * @text: The text to be displayed to the player. It gets processed by
 *        wmud_menu_process_item() to highlight the menuchar
 * @display_text: The actual text that gets displayed to the player
 * @display_text_ansi: The actual text that gets displayed to the player when
 *                     ANSI colour mode is enabled
 * @func: the menu item's function. See menu.c for the list of possible values.
 */
typedef struct _wmudMenu {
	guint id;
	gchar menuchar;
	gboolean need_active_char;
	guint placement;
	gchar *text;
	gchar *display_text;
	gchar *display_text_ansi;
	gchar *func;
} wmudMenu;

typedef void (*wmudMenuCommandFunc)(wmudClient *client);
/**
 * WMUD_MENU_COMMAND:
 * @name: the name of the command. Should be in lowercase
 *
 * Shorthand to create the function header for menu command handlers
 */
#define WMUD_MENU_COMMAND(name) void wmud_mcmd_ ## name(wmudClient *client)

GSList *game_menu;

#define WMUD_MENU_ERROR wmud_menu_error_quark()
GQuark wmud_menu_error_quark();
gboolean wmud_menu_init(GSList **menu);
gboolean wmud_menu_items_check(GSList *menu_items, GError **err);
void wmud_menu_items_free(GSList **menu_items);
gchar *wmud_menu_get_command_by_menuchar(gchar menuchar, GSList *game_menu);
void wmud_menu_execute_command(wmudClient *client, gchar *command);
void wmud_menu_present(wmudClient *client);

#endif /* __WMUD_MENU_H__ */

