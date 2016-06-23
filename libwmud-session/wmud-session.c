/* wMUDSession - wMUD Session handler object
 * Copyright (C) 2010, Gergely Polonkai
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gnet.h>

#include "wmud-session.h"

/** SECTION:objects
 * @short_description: wMUD Session object
 * @title: wMUD Session handler object
 *
 * wMUDSession is an object to store session-related information in
 * wMUD
 */

/* --- macros --- */
#define WMUD_SESSION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WMUD_TYPE_SESSION, wMUDSessionPrivate))

/* --- structures --- */

struct _wMUDSessionPrivate
{
	GConn *connection;
	wMUDSessionType type;
};

/* --- signals --- */

/* --- properties --- */
enum
{
	PROP_0,
	PROP_CONNECTION,
	PROP_TYPE
};

/* --- prototypes --- */
static void wmud_session_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void wmud_session_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void wmud_session_dispose(GObject *object);
static void wmud_session_finalize(GObject *object);
static void wmud_session_class_init(wMUDSessionClass *klass);
static void wmud_session_init(wMUDSession *self);
wMUDSession *wmud_session_new(void);
wMUDSession *wmud_session_new_with_connection(GConn *connection);
GConn *wmud_session_get_connection(wMUDSession *session);
void wmud_session_set_connection(wMUDSession *session, GConn *connection);
wMUDSessionType wmud_session_get_session_type(wMUDSession *session);
void wmud_session_set_session_type(wMUDSession *session, wMUDSessionType type);

/* --- variables --- */

/* --- functions --- */

G_DEFINE_TYPE(wMUDSession, wmud_session, G_TYPE_OBJECT);

#define WMUD_TYPE_SESSION_TYPE       wmud_session_type_get_type()
static GType
wmud_session_type_get_type(void)
{
	static GType wmud_session_type_type = 0;
	static const GEnumValue wmud_session_types[] = {
		{ WMUD_SESSION_TYPE_UNKNOWN, "Unknown session type", "unknown" },
		{ WMUD_SESSION_TYPE_TELNET,  "Telnet session",       "telnet" },
		{ 0, NULL, NULL }
	};

	if (!wmud_session_type_type)
	{
		wmud_session_type_type = g_enum_register_static("wMUDSessionType", wmud_session_types);
	}

	return wmud_session_type_type;
}

static void
wmud_session_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	wMUDSession *self = WMUD_SESSION(object);

	switch (property_id)
	{
		case PROP_CONNECTION:
			if (self->priv->connection)
			{
				gnet_conn_unref(self->priv->connection);
			}
			self->priv->connection = (GConn *)g_value_get_pointer(value);
			if (self->priv->connection)
			{
				gnet_conn_ref(self->priv->connection);
			}
			break;
		case PROP_TYPE:
			self->priv->type = g_value_get_enum(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
wmud_session_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	wMUDSession *self = WMUD_SESSION(object);

	switch (property_id)
	{
		case PROP_CONNECTION:
			g_value_set_pointer(value, self->priv->connection);
			break;
		case PROP_TYPE:
			g_value_set_enum(value, self->priv->type);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
wmud_session_dispose(GObject *object)
{
	G_OBJECT_CLASS(wmud_session_parent_class)->dispose(object);
}

static void
wmud_session_finalize(GObject *object)
{
	wMUDSession *self = WMUD_SESSION(object);

	if (self->priv->connection)
	{
		gnet_conn_disconnect(self->priv->connection);
		gnet_conn_unref(self->priv->connection);
		self->priv->connection = NULL;
	}

	G_OBJECT_CLASS(wmud_session_parent_class)->finalize(object);
}

static void
wmud_session_class_init(wMUDSessionClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	GParamSpec *wmud_session_param_spec;

	gobject_class->set_property = wmud_session_set_property;
	gobject_class->get_property = wmud_session_get_property;
	gobject_class->dispose      = wmud_session_dispose;
	gobject_class->finalize     = wmud_session_finalize;

	g_type_class_add_private(klass, sizeof(wMUDSessionPrivate));

	wmud_session_param_spec = g_param_spec_pointer("connection", "Connection handle", "GConn * handle of the connection", G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
	g_object_class_install_property(gobject_class, PROP_CONNECTION, wmud_session_param_spec);

	wmud_session_param_spec = g_param_spec_enum("type", "SessionType", "Type of the session's connection", WMUD_TYPE_SESSION_TYPE, WMUD_SESSION_TYPE_UNKNOWN, G_PARAM_READWRITE);
	g_object_class_install_property(gobject_class, PROP_TYPE, wmud_session_param_spec);
}

static void
wmud_session_init(wMUDSession *self)
{
	wMUDSessionPrivate *priv;

	self->priv = priv = WMUD_SESSION_GET_PRIVATE(self);

	priv->connection = NULL;
}

/**
 * wmud_session_new:
 *
 * Creates a new wMUDSession object
 *
 * Returns: a new instance of wMUDSession
 */
wMUDSession *
wmud_session_new(void)
{
	wMUDSession *new_session = g_object_new(WMUD_TYPE_SESSION, NULL);

	return new_session;
}

/**
 * wmud_session_new_with_connection:
 * @connection: the connection this session is bound to. This object is
 *  g_unref()'d when the Session object is disposed
 *
 * Returns: a new instance of wMUDSession with the connection property set
 */
wMUDSession *
wmud_session_new_with_connection(GConn *connection)
{
	wMUDSession *new_session = g_object_new(WMUD_TYPE_SESSION, "connection", connection, NULL);
	wmud_session_set_connection(new_session, connection);

	return new_session;
}

GConn *
wmud_session_get_connection(wMUDSession *session)
{
	GConn *connection = NULL;
	GValue value = {0};

	g_value_init(&value, G_TYPE_POINTER);

	wmud_session_get_property(G_OBJECT(session), PROP_CONNECTION, &value, NULL);

	connection = g_value_get_pointer(&value);

	g_value_unset(&value);

	return connection;
}

/**
 * wmud_session_set_connection:
 * @session: the object which should receive the new connection
 * @connection: the connection this session should be bound to. This object is g_unref()'d when the Session object is disposed
 */
void
wmud_session_set_connection(wMUDSession *session, GConn *connection)
{
	GValue value = {0};

	g_value_init(&value, G_TYPE_POINTER);
	g_value_set_pointer(&value, (gpointer)connection);

	wmud_session_set_property(G_OBJECT(session), PROP_CONNECTION, &value, NULL);

	g_value_unset(&value);
}

/**
 * wmud_session_has_connection:
 * @session: the object which should be inspected
 * @connection: the connection we are looking for
 *
 * Checks if the given #wMUDSession has the given #GConn connection
 *
 * Returns: %TRUE, if the connection is owned by this session, %FALSE otherwise
 */
gboolean
wmud_session_has_connection(wMUDSession *session, GConn *connection)
{
	if (session->priv->connection == connection)
		return TRUE;

	return FALSE;
}

wMUDSessionType
wmud_session_get_session_type(wMUDSession *session)
{
	GValue value = {0};
	g_value_init(&value, WMUD_TYPE_SESSION_TYPE);

	wmud_session_get_property(G_OBJECT(session), PROP_TYPE, &value, NULL);

	return g_value_get_enum(&value);
}

void
wmud_session_set_session_type(wMUDSession *session, wMUDSessionType type)
{
	GValue value = {0, };
	g_value_init(&value, WMUD_TYPE_SESSION_TYPE);

	g_value_set_enum(&value, type);
	wmud_session_set_property(G_OBJECT(session), PROP_TYPE, &value, NULL);
}

wMUDSession *
wmud_session_ref(wMUDSession *session)
{
	return WMUD_SESSION(g_object_ref(G_OBJECT(session)));
}

void
wmud_session_unref(wMUDSession *session)
{
	g_object_unref(G_OBJECT(session));
}
