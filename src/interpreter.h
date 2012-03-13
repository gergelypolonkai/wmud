#ifndef __WMUD_INTERPRETER_H__
# define __WMUD_INTERPRETER_H__

#include "networking.h"

gboolean wmud_interpreter_init(void);
void wmud_interpret_game_command(wmudClient *client);

#endif /* __WMUD_INTERPRETER_H__ */

