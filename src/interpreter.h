#ifndef __WMUD_INTERPRETER_H__
# define __WMUD_INTERPRETER_H__

#include "networking.h"

typedef void (*wmudCommandFunc)(wmudClient *client, gchar *command, GSList *token_list);
#define WMUD_COMMAND(name) void name(wmudClient *client, gchar *command, GSList *token_list)

typedef struct _wmudCommand {
	gchar *command;
	wmudCommandFunc commandFunc;
} wmudCommand;

gboolean wmud_interpreter_init(void);
void wmud_interpret_game_command(wmudClient *client);

#endif /* __WMUD_INTERPRETER_H__ */

