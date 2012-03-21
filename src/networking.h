#ifndef __WMUD_NETWORKING_H__
# define __WMUD_NETWORKING_H__

#include <glib.h>
#include <gio/gio.h>

typedef enum {
	WMUD_CLIENT_STATE_FRESH,     /* Newly connected clients. We are waiting for
	                              * a player name */
	WMUD_CLIENT_STATE_PASSWAIT,  /* Player name entered, waiting for password */
	WMUD_CLIENT_STATE_MENU,      /* Logged in players, waiting in the main menu.
	                              * We are waiting for a menu item to be
	                              * chosen.*/
	WMUD_CLIENT_STATE_INGAME,    /* Player is in-game */
	WMUD_CLIENT_STATE_QUITWAIT   /* Waiting for answer for the quit question */
} wmudClientState;

typedef struct _wmudClient {
	GSocket *socket;
	GString *buffer;
	wmudClientState state;
	gboolean authenticated;
	wmudPlayer *player;
} wmudClient;

extern GSList *clients;

gboolean wmud_networking_init(guint port_number);
void wmud_client_send(wmudClient *client, const gchar *fmt, ...);
void wmud_client_start_login(wmudClient *client);

#endif
