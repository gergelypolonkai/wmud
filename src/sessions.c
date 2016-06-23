#include <glib.h>
#include <gnet.h>

#include "wmud.h"
#include "sessions.h"
#include "networking.h"
#include "logger.h"

#include "wmud-session.h"

GSList *wmud_session_list = NULL;

gboolean
wmud_new_session(GConn *connection, wMUDSessionType type)
{
	wMUDSession *session;

	Context;

	if ((session = wmud_session_new_with_connection(connection)) == NULL)
	{
		return FALSE;
	}

	Context wmud_session_set_session_type(session, type);

	Context;

	wmud_session_list = g_slist_append(wmud_session_list, session);

	Context;

	return TRUE;
}

static gint
_wmud_find_session_with_connection(gconstpointer list_element, gconstpointer connection)
{
	wMUDSession *session = (wMUDSession *)list_element;
	GConn *conn = (GConn *)connection;

	g_assert(WMUD_IS_SESSION(list_element));

	Context return (wmud_session_has_connection(session, conn)) ? 0 : 1;
}

wMUDSession *
wmud_session_for_connection(GConn *connection)
{
	GSList *temp;

	Context temp = g_slist_find_custom(wmud_session_list, connection, _wmud_find_session_with_connection);

	if (temp)
	{
		return temp->data;
	}

	return NULL;
}

void
wmud_finish_session(wMUDSession *session, gchar *message)
{
	if ((session == NULL) || (!WMUD_IS_SESSION(session)))
	{
		return;
	}
	GConn *connection = wmud_session_get_connection(session);

	if (connection)
	{
		wmud_log_debug("Destroying session with connection");
		if (message)
		{
			wmud_connection_send(connection, message);
		}
		wmud_connection_disconnect(connection);
		wmud_session_set_connection(session, NULL);
	}

	wmud_session_list = g_slist_remove_all(wmud_session_list, session);
	wmud_session_unref(session);
}

static void
_wmud_destroy_session(gpointer sess, gpointer msg)
{
	wMUDSession *session = (wMUDSession *)sess;
	gchar *message = (gchar *)msg;

	wmud_log_debug("Shutting down session %lx", sess);
	wmud_finish_session(session, message);
}

void
wmud_destroy_all_sessions(gchar *message)
{
	g_slist_foreach(wmud_session_list, _wmud_destroy_session, message);
}

