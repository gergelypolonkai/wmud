#include <glib.h>
#include <gnet.h>

#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "wmud.h"
#include "logger.h"
#include "sessions.h"
#include "networking.h"
#include "configfile.h"
#include "commands.h"
#include "irc.h"

#define LISTEN_PORT 4000

GSList *_wmud_active_interfaces = NULL;

/**
 * _wmud_telnet_event:
 * @connection: the #GConn on which this event occured
 * @event: type of event
 * @user_data: user data arrived with this function call. Currently it is not used
 *
 * Callback function which gets called whenever a new event occurs on a telnet-type socket
 */
static void
_wmud_telnet_event(GConn *connection, GConnEvent *event, gpointer user_data)
{
	gchar *command;
	GError *error = NULL;

	//Context;

	switch (event->type)
	{
		case GNET_CONN_READABLE:
			command = NULL;
			gsize term_pos;
			gsize length;

			switch (g_io_channel_read_line(connection->iochannel, &command, &length, &term_pos, &error))
			{
				case G_IO_STATUS_ERROR:
					wmud_log_error("Telnet read error: %s", error->message);
					break;
				case G_IO_STATUS_NORMAL:
					wmud_log_debug("Accepted data");
					break;
				case G_IO_STATUS_EOF:
					wmud_log_info("Client disconnected");
					wmud_connection_disconnect(connection);
					break;
				case G_IO_STATUS_AGAIN:
					wmud_log_debug("Telnet read terminated, trying again");
					break;
				default:
					break;
			}
			if (command)
			{
				*(command + term_pos) = 0;
				wmud_log_debug("Got command: %s", command);
				wmud_process_command(connection, command);
				*(command + term_pos) = '\n';
				g_free(command);
			}
			break;
		case GNET_CONN_TIMEOUT:
			wmud_log_info("Telnet connection timeout");
			wmud_connection_disconnect(connection);
			break;
		case GNET_CONN_ERROR:
			wmud_log_error("Unprocessed connection event CONN_ERROR");
			break;
		case GNET_CONN_CONNECT:
			wmud_log_error("Unprocessed connection event CONN_CONNECT");
			break;
		case GNET_CONN_CLOSE:
			wmud_log_error("Unprocessed connection event CONN_CLOSE");
			break;
		case GNET_CONN_READ:
			wmud_log_error("Unprocessed connection event CONN_READ");
			break;
		case GNET_CONN_WRITE:
			wmud_log_error("Unprocessed connection event CONN_WRITE");
			break;
		case GNET_CONN_WRITABLE:
			wmud_log_error("Unprocessed connection event CONN_WRITABLE");
			break;
		default:
			wmud_log_error("Got an unknown connction event from GNet!");
			break;
	}

}

static void
_wmud_irc_event(GConn *connection, GConnEvent *event, gpointer user_data)
{
	gchar *command;
	GError *error = NULL;

	switch (event->type)
	{
		case GNET_CONN_READABLE:
			command = NULL;
			gsize term_pos;
			gsize length;

			switch (g_io_channel_read_line(connection->iochannel, &command, &length, &term_pos, &error))
			{
				case G_IO_STATUS_ERROR:
					wmud_log_error("IRC read error: %s", error->message);
					break;
				case G_IO_STATUS_NORMAL:
					wmud_log_debug("Accepted data");
					break;
				case G_IO_STATUS_EOF:
					wmud_log_info("Client disconnected");
					wmud_connection_disconnect(connection);
					break;
				case G_IO_STATUS_AGAIN:
					wmud_log_debug("IRC read terminated, trying again");
					break;
				default:
					break;
			}
			if (command)
			{
				*(command + term_pos) = 0;
				wmud_log_debug("Got command: %s", command);
				wmud_process_irc_command(connection, command);
				*(command + term_pos) = '\n';
				g_free(command);
			}
			break;
		case GNET_CONN_TIMEOUT:
			wmud_log_info("IRC connection timeout");
			wmud_connection_disconnect(connection);
			break;
		case GNET_CONN_ERROR:
			wmud_log_error("Unprocessed connection event CONN_ERROR");
			break;
		case GNET_CONN_CONNECT:
			wmud_log_error("Unprocessed connection event CONN_CONNECT");
			break;
		case GNET_CONN_CLOSE:
			wmud_log_error("Unprocessed connection event CONN_CLOSE");
			break;
		case GNET_CONN_READ:
			wmud_log_error("Unprocessed connection event CONN_READ");
			break;
		case GNET_CONN_WRITE:
			wmud_log_error("Unprocessed connection event CONN_WRITE");
			break;
		case GNET_CONN_WRITABLE:
			wmud_log_error("Unprocessed connection event CONN_WRITABLE");
			break;
		default:
			wmud_log_error("Got an unknown connction event from GNet!");
			break;
	}

}

void
wmud_connection_send(GConn *connection, gchar *fmt, ...)
{
	va_list args;
	gchar *buffer,
	      *send_buffer;
	gsize len;
	GError *error = NULL;
	GIOStatus status;

	va_start(args, fmt);
	Context buffer = g_strdup_vprintf(fmt, args);
	va_end(args);

	Context send_buffer = g_strdup_printf("%s%s", buffer, wmud_connection_get_linebreak(connection));
	Context g_free(buffer);

	/* TODO: Error checking! */
	Context;
	switch ((status = g_io_channel_write_chars(connection->iochannel, send_buffer, -1, &len, &error)))
	{
		case G_IO_STATUS_ERROR:
			wmud_log_error("Error during g_io_channel_write_chars(): %s", error->message);
			break;
		case G_IO_STATUS_NORMAL:
			wmud_log_debug("Message sent");
			break;
		case G_IO_STATUS_EOF:
			wmud_log_warning("EOF during g_io_channel_write_chars()???");
			break;
		case G_IO_STATUS_AGAIN:
			wmud_log_warning("Resource temporarily unavailable while sending message to client");
			break;
		default:
			wmud_log_error("Unknown return value from g_io_channel_write_chars()!");
			break;
	}
	if (error != NULL)
	{
		g_error_free(error);
		error = NULL;
	}
	Context g_free(send_buffer);

	switch ((status = g_io_channel_flush(connection->iochannel, &error)))
	{
		case G_IO_STATUS_ERROR:
			wmud_log_error("Error during g_io_channel_flush(): %s", error->message);
			break;
		case G_IO_STATUS_NORMAL:
			wmud_log_debug("IOChannel flushed");
			break;
		case G_IO_STATUS_EOF:
			wmud_log_warning("EOF during g_io_channel_flush()???");
			break;
		case G_IO_STATUS_AGAIN:
			wmud_log_warning("Resource temporarily unavailable while sending message to client");
			break;
		default:
			wmud_log_error("Unknown return value from g_io_channel_flush()!");
			break;
	}

	if (error != NULL)
	{
		g_error_free(error);
		error = NULL;
	}
	Context;
}

const gchar *
wmud_connection_get_linebreak(GConn *connection)
{
	wMUDSession *session;

	Context session = wmud_session_for_connection(connection);

	if (session)
	{
		switch (wmud_session_get_session_type(session))
		{
			case WMUD_SESSION_TYPE_TELNET:
				Context return "\r\n";
				break;
			case WMUD_SESSION_TYPE_IRC:
				Context return "\r\n";
				break;
			default:
				wmud_log_warning("Looling for linebreak for unknown session type");
				return "";
		}
	}
	else
	{
		Context;
	}

	return "";
}

/**
 * _wmud_accept_telnet:
 * @server: the #GServer object that has an incoming connection
 * @connection: The #GConn created to hold the new connection
 * @user_data: user data arrived with this function call. Currently it is not used
 *
 * Callback function which gets called whenever a new connection arrived on a server socket
 */
static void
_wmud_accept_telnet(GServer *server, GConn *connection, gpointer user_data)
{
	wMUDConfigurationInterface *interface = (wMUDConfigurationInterface *)user_data;
	/* If the connection is not created for some reason, log an error and return */
	if (connection == NULL)
	{
		wmud_log_error("Error while accepting connection");
		return;
	}

	/* TODO: Disconnect if the IP is banned */

	/* Log a message that a new connection has arrived */
	wmud_log_info("Accepted telnet connection from [%s]", gnet_inetaddr_get_canonical_name(connection->inetaddr));

	/* Set the parameters of this connection */
	/* The connection should be buffered, so it is easier to handle */
	g_io_channel_set_buffered(connection->iochannel, TRUE);
	/* The function to call, whenever an event occurs */
	gnet_conn_set_callback(connection, _wmud_telnet_event, NULL);
	/* Watch for readable events */
	gnet_conn_set_watch_readable(connection, TRUE);
	/* Timeout interval */
	gnet_conn_timeout(connection, interface->timeout * 1000);

	Context;

	if (!wmud_new_session(connection, WMUD_SESSION_TYPE_TELNET))
	{
		Context;
		g_io_channel_write_chars(connection->iochannel, "Unable to create a session for you. If you experience this problem for more than once, please contact us!", -1, NULL, NULL);
		gnet_conn_disconnect(connection);
	}

	Context;
}

static void
_wmud_accept_irc(GServer *server, GConn *connection, gpointer user_data)
{
	wMUDConfigurationInterface *interface = (wMUDConfigurationInterface *)user_data;

	if (connection == NULL)
	{
		wmud_log_error("Error while accepting connection");
		return;
	}

	/* TODO: Disconnect if the IP is banned */

	/* Log a message that a new connection has arrived */
	wmud_log_info("Accepted IRC connection from [%s]", gnet_inetaddr_get_canonical_name(connection->inetaddr));

	/* Set the parameters of this connection */
	/* The connection should be buffered, so it is easier to handle */
	g_io_channel_set_buffered(connection->iochannel, TRUE);
	/* The function to call, whenever an event occurs */
	gnet_conn_set_callback(connection, _wmud_irc_event, NULL);
	/* Watch for readable events */
	gnet_conn_set_watch_readable(connection, TRUE);
	/* Timeout interval */
	gnet_conn_timeout(connection, interface->timeout * 1000);

	Context;

	if (!wmud_new_session(connection, WMUD_SESSION_TYPE_TELNET))
	{
		Context;
		g_io_channel_write_chars(connection->iochannel, "Unable to create a session for you. If you experience this problem for more than once, please contact us!", -1, NULL, NULL);
		gnet_conn_disconnect(connection);
	}

	Context;
}

static void
_wmud_create_interface(gpointer data, gpointer user_data)
{
	GServer *server;
	wMUDConfigurationInterface *interface = (wMUDConfigurationInterface *)data;

	wmud_log_debug("Creating new interface '%s'", interface->name);
	switch (interface->type)
	{
		case WMUD_SESSION_TYPE_TELNET:
			server = gnet_server_new(interface->inetaddr, gnet_inetaddr_get_port(interface->inetaddr), _wmud_accept_telnet, interface);
			_wmud_active_interfaces = g_slist_append(_wmud_active_interfaces, (gpointer)server);
			break;
		case WMUD_SESSION_TYPE_IRC:
			server = gnet_server_new(interface->inetaddr, gnet_inetaddr_get_port(interface->inetaddr), _wmud_accept_irc, interface);
			_wmud_active_interfaces = g_slist_append(_wmud_active_interfaces, (gpointer)server);
			break;
		default:
			wmud_log_error("Unknown type of interface!");
			break;
	}
}

void
wmud_connection_disconnect(GConn *connection)
{
		gnet_conn_disconnect(connection);
}

gpointer
wmud_networking_thread(gpointer data)
{
	wMUDThreadData *thread_data = (wMUDThreadData *)data;

	Context;
	wmud_log_info("Initializing network layer...");

	/* g_main_context_get_thread_default() is only available since GLib 2.22;
	 * use g_main_context_new() otherwise
	 */
#if (((GLIB_MAJOR_VERSION == 2) && (GLIB_MINOR_VERSION >= 22)) || (GLIB_MAJOR_VERSION > 2))
	thread_data->main_context = g_main_context_get_thread_default();
#else
	thread_data->main_context = g_main_context_new();
#endif
	thread_data->main_loop = g_main_loop_new(thread_data->main_context, FALSE);

	/* Do the real initialization work here */

	Context g_slist_foreach(wmud_configuration->interfaces, _wmud_create_interface, NULL);

	/* End of initialization */

	wmud_log_info("Network layer initialized");

	Context;
	thread_data->running = TRUE;
	g_main_loop_run(thread_data->main_loop);

	wmud_log_info("Network layer shutting down");

	wmud_destroy_all_sessions("Server is shutting down.");

	g_main_loop_unref(thread_data->main_loop);
	thread_data->main_loop = NULL;

	thread_data->running = FALSE;

	Context;

	return NULL;
}

