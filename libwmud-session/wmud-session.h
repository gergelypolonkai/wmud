#ifndef __WMUD_SESSION_H__
#define __WMUD_SESSION_H__

#include <glib.h>
#include <glib-object.h>
#include <gnet.h>

#define WMUD_TYPE_SESSION            wmud_session_get_type()
#define WMUD_SESSION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WMUD_TYPE_SESSION, wMUDSession))
#define WMUD_IS_SESSION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WMUD_TYPE_SESSION))
#define WMUD_SESSION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WMUD_TYPE_SESSION, wMUDSessionClass))
#define WMUD_IS_SESSION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WMUD_TYPE_SESSION))
#define WMUD_SESSION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WMUD_TYPE_SESSION, wMUDSessionClass))

typedef enum
{
	WMUD_SESSION_TYPE_UNKNOWN,
	WMUD_SESSION_TYPE_TELNET,
	WMUD_SESSION_TYPE_IRC,
	WMUD_SESSION_TYPE_HTTP
} wMUDSessionType;

typedef struct _wMUDSession        wMUDSession;
typedef struct _wMUDSessionClass   wMUDSessionClass;
typedef struct _wMUDSessionPrivate wMUDSessionPrivate;

struct _wMUDSession
{
	GObject parent_object;
	
	wMUDSessionPrivate *priv;
};

struct _wMUDSessionClass
{
	GObjectClass parent_class;
};

GType wmud_session_get_type(void);

wMUDSession *wmud_session_new(void);
wMUDSession *wmud_session_new_with_connection(GConn *connection);
gboolean wmud_session_has_connection(wMUDSession *session, GConn *connection);
wMUDSession *wmud_session_ref(wMUDSession *session);
void wmud_session_unref(wMUDSession *session);

GConn *wmud_session_get_connection(wMUDSession *session);
void wmud_session_set_connection(wMUDSession *session, GConn *connection);
wMUDSessionType wmud_session_get_session_type(wMUDSession *session);
void wmud_session_set_session_type(wMUDSession *session, wMUDSessionType type);

#endif /* __WMUD_SESSION_H__ */

