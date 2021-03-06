/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * interpreter.h: game command interpreter prototypes and variables
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
#ifndef __WMUD_INTERPRETER_H__
# define __WMUD_INTERPRETER_H__

#include "game-networking.h"

/**
 * wmudCommandFunc:
 * @client: the client from whom the command arrived
 * @command: the command itself
 * @token_list: the command arguments
 *
 * Command handler function type
 */
typedef void (*wmudCommandFunc)(WmudClient *client, gchar *command, GSList *token_list);
/**
 * WMUD_COMMAND:
 * @name: the name of the command. Should be in lowercase
 *
 * Shorthand to create the function header for command handlers
 */
#define WMUD_COMMAND(name) void gcmd_ ## name(WmudClient *client, gchar *command, GSList *token_list)

/**
 * wmudCommand:
 * @command: the command itself. Should be in uppercase, but doesn't actually
 *           matter
 * @commandFunc: the command handler function for this command
 *
 * This structure holds the different properties of the in-game commands.
 */
typedef struct _wmudCommand {
    gchar           *command;
    wmudCommandFunc commandFunc;
} wmudCommand;

#define WMUD_INTERPRETER_ERROR wmud_interpreter_error_quark()
GQuark wmud_interpreter_error_quark();

typedef enum {
    WMUD_INTERPRETER_ERROR_DUPCMD
} wmudInterpreterError;

gboolean wmud_interpreter_check_directions(GSList *directions,
                                           GError **err);
void wmud_interpret_game_command(WmudClient *client);

#endif /* __WMUD_INTERPRETER_H__ */
