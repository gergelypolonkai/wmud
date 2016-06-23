#ifndef __WMUD_CORE_NETWORKING_H__
#define __WMUD_CORE_NETWORKING_H__

#include <glib.h>
#include <gnet.h>

gpointer wmud_networking_thread(gpointer data);
void wmud_connection_disconnect(GConn *connection);
void wmud_connection_send(GConn *connection, gchar *fmt, ...);
const gchar *wmud_connection_get_linebreak(GConn *connection);

#endif /* __WMUD_CORE_NETWORKING_H__ */

