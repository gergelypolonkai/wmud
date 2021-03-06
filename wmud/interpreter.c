/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * interpreter.c: game command interpreter routines
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
#include <string.h>

#include "interpreter.h"
#include "game-networking.h"
#include "main.h"
#include "world.h"

/**
 * SECTION:interpreter
 * @short_description: Game command interpreter
 *
 * Functions to interpret and execute in-game commands
 */

WMUD_COMMAND(quit);

struct findData {
    GSList *list;
    guint  found;
    gchar  *last;
};

static wmudCommand command_list[] = {
    { "quit", gcmd_quit },
    { NULL,   NULL },
};

GQuark
wmud_interpreter_error_quark()
{
    return g_quark_from_static_string("wmud-interpreter-error");
}

/**
 * destroy_string:
 * @string: a GString to destroy
 *
 * Callback function to destroy a list of GStrings
 */
static void
destroy_string(GString *string)
{
    g_string_free(string, TRUE);
}

static gint
check_direction_dups2(wmudDirection *dir1, wmudDirection *dir2)
{
    gint check;

    if ((check = g_ascii_strcasecmp(dir1->short_name, dir2->short_name)) != 0) {
        return check;
    }

    if ((check = g_ascii_strcasecmp(dir1->name, dir2->name)) != 0) {
        return check;
    }

    if ((check = g_ascii_strcasecmp(dir1->short_name, dir2->name)) != 0) {
        return check;
    }

    return g_ascii_strcasecmp(dir1->name, dir2->short_name);
}

static void
check_direction_dups1(wmudDirection *dir, struct findData *find_data)
{
    if (find_data->last != dir->name) {
        find_data->found = (find_data->found > 1) ? find_data->found : 0;
        find_data->last  = dir->name;
    }

    if (g_slist_find_custom(find_data->list, dir, (GCompareFunc)check_direction_dups2)) {
        find_data->found++;
    }
}

static void
check_direction_command(wmudDirection *dir, gboolean *found)
{
    wmudCommand *cmd;

    for (cmd = command_list; cmd->command; cmd++) {
        if (g_ascii_strcasecmp(dir->short_name, cmd->command) == 0) {
            *found = TRUE;

            return;
        }

        if (g_ascii_strcasecmp(dir->name, cmd->command) == 0) {
            *found = TRUE;

            return;
        }
    }
}

/**
 * wmud_interpreter_check_directions:
 * @directions: a #GSList of directions
 * @err: A #GError to store possible errors on failure
 *
 * Checks if the given directions are already registered commands.
 *
 * Return value: If the directions are acceptable at the time of the check, the function returns %TRUE. Otherwise %FALSE is returned, and
 */
gboolean
wmud_interpreter_check_directions(GSList *directions, GError **err)
{
    gboolean        command_found = FALSE;
    struct findData find_data     = {directions, 0, NULL};

    g_slist_foreach(directions, (GFunc)check_direction_command, &command_found);

    if (command_found) {
        g_set_error(err, WMUD_INTERPRETER_ERROR, WMUD_INTERPRETER_ERROR_DUPCMD, "Direction commands are not unique. Please check the database!");
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Direction command are not unique. Please check the database!");
    }

    g_slist_foreach(directions, (GFunc)check_direction_dups1, &find_data);

    if (find_data.found > 1) {
        g_set_error(err, WMUD_INTERPRETER_ERROR, WMUD_INTERPRETER_ERROR_DUPCMD, "Direction commands defined in the database are not unique!");
        g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "Direction commands defined in the databsae are not unique.");

        return FALSE;
    }

    return !command_found;
}

/**
 * wmud_interpret_game_command:
 * @client: the WmudClient whose command should be processed
 *
 * Processes a WmudClient's buffer, and executes the game command if there is
 * one
 */
void
wmud_interpret_game_command(WmudClient *client)
{
    GSList *command_parts = NULL;
    gchar  *a,
           *start,
           *end;
    gchar       str_delim = 0;
    wmudCommand *cmd;
    int         command_parts_count = 0,
                match_count         = 0;
    GSList      *matches            = NULL;

    if (strchr(wmud_client_get_buffer(client)->str, '\r') || strchr(wmud_client_get_buffer(client)->str, '\n')) {
        /* We should NEVER reach this point! */
        g_assert_not_reached();

        return;
    }

    a = wmud_client_get_buffer(client)->str;

    GString *token;

    while (*a) {
        for (start = a; *start; start++) {
            if (!str_delim) {
                if ((*start == '"') || (*start == '\'')) {
                    str_delim = *start;
                    start++;

                    break;
                } else if (g_ascii_isspace(*start) || (!*start)) {
                    break;
                }
            }
        }

        for (end = start; *end; end++) {
            if (!str_delim && strchr("'\" \t", *end)) {
                break;
            } else if (str_delim && (*end == str_delim)) {
                str_delim = 0;

                break;
            } else if (!*end) {
                break;
            }
        }

        if (*start) {
            token         = g_string_new_len(start, end - start);
            command_parts = g_slist_prepend(command_parts, token);
            command_parts_count++;
        }

        a = end;
        if (((*a == '"') || (*a == '\'')) && str_delim) {
            a++;
        }
    }

    if (str_delim) {
        wmud_client_send(client, "You should close quotes of any kind, like %c, shouldn't you?\r\n", str_delim);
#if GLIB_CHECK_VERSION(2, 28, 0)
        g_slist_free_full(command_parts, (GDestroyNotify)destroy_string);
#else
        g_slist_foreach(command_parts, (GFunc)destroy_string, NULL);
        g_slist_free(command_parts);
#endif
        return;
    }

    if (command_parts_count == 0) {
        /* TODO: handle empty command */

        return;
    }

    command_parts = g_slist_reverse(command_parts);

    for (cmd = command_list; cmd->command; cmd++) {
        GString *input = (GString *)(command_parts->data);
        gint    cmp;

        if (((cmp = g_ascii_strncasecmp(input->str, cmd->command, input->len)) == 0) && !cmd->command[input->len]) {
            g_slist_free(matches);
            match_count = 1;
            matches     = NULL;
            matches     = g_slist_prepend(matches, cmd);

            break;
        } else if (cmp == 0) {
            matches = g_slist_prepend(matches, cmd);
            match_count++;
        }
    }

    switch (match_count) {
        case 0:
            switch (random_number(1, 3)) {
                case 1:
                    wmud_client_send(client, "Huh?\r\n");

                    break;
                case 2:
                    wmud_client_send(client, "What?\r\n");

                    break;
                case 3:
                    wmud_client_send(client, "I can hardly understand you...\r\n");

                    break;
            }

            break;
        case 1:
            ((wmudCommand *)(matches->data))->commandFunc(client, ((GString *)(command_parts->data))->str, command_parts->next);

            break;
        default:
            wmud_client_send(client, "This command could mean several things, please try a more exact form!\r\n");
    }

    g_slist_free(matches);
}

/**
 * gcmd_quit:
 *
 * The QUIT game command's handler
 */
WMUD_COMMAND(quit)
{
    wmud_client_send(client, "Are you sure you want to get back to that freaky other reality? [y/N] ");
    wmud_client_set_state(client, WMUD_CLIENT_STATE_YESNO);
    wmud_client_set_yesno_callback(client, wmud_client_quitanswer);
}
