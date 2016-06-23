#ifndef __WMUD_CORE_IRC_H__
#define __WMUD_CORE_IRC_H__

#include <glib.h>
#include <gnet.h>

void wmud_process_irc_command(GConn *connection, gchar *command);

#endif /* __WMUD_CORE_IRC_H__ */

