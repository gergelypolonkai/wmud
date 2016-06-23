#ifndef __WMUD_CORE_SESSIONS_H__
#define __WMUD_CORE_SESSIONS_H__

#include <glib.h>
#include <gnet.h>

#include "wmud-session.h"

extern GSList *wmud_session_list;

gboolean wmud_new_session(GConn *connection, wMUDSessionType type);
wMUDSession *wmud_session_for_connection(GConn *connection);
void wmud_finish_session(wMUDSession *session, gchar *message);
void wmud_destroy_all_sessions(gchar *message);

#endif /* __WMUD_CORE_SESSIONS_H__ */

