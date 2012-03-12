#ifndef __WMUD_NETWORKING_H__
# define __WMUD_NETWORKING_H__

typedef struct _wmudClient {
	GSocket *socket;
	GString *buffer;
} wmudClient;

extern GSList *clients;

gboolean wmud_networking_init(guint port_number);

#endif
