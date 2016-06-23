#ifndef __WMUD_CORE_COMMANDS_H
#define __WMUD_CORE_COMMANDS_H

#include <gnet.h>

void wmud_process_command(GConn *connection, gchar *command);

#endif /* __WMUD_CORE_COMMANDS_H */

