#ifndef __WMUD_NETWORKING_H__
# define __WMUD_NETWORKING_H__

#include <glib.h>

#include "wmud_types.h"

extern GSList *clients;

gboolean wmud_networking_init(guint port_number);
void wmud_client_send(wmudClient *client, const gchar *fmt, ...);
void wmud_client_start_login(wmudClient *client);

#endif
