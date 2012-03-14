#ifndef __WMUD_NETWORKING_H__
# define __WMUD_NETWORKING_H__

#include  <glib.h>
#include <gio/gio.h>

typedef enum {
	WMUD_CLIENT_STATE_MENU
} wmudClientState;

typedef struct _wmudClient {
	GSocket *socket;
	GString *buffer;
	wmudClientState state;
} wmudClient;

extern GSList *clients;

gboolean wmud_networking_init(guint port_number);
void wmud_client_send(wmudClient *client, const gchar *fmt, ...);

#endif
