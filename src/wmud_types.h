#ifndef __WMUD_TYPES_H__
#define __WMUD_TYPES_H__

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

typedef struct _wmudPlayer {
	guint32 id;          /* User ID */
	gchar *player_name;  /* Player login name */
	gchar *cpassword;    /* Crypted password */
} wmudPlayer;

typedef struct _wmudClient {
	GSocket *socket;
	GString *buffer;
	wmudClientState state;
	gboolean authenticated;
	wmudPlayer *player;
} wmudClient;

enum {
	WMUD_CONFIG_ERROR_SUCCESS,
	WMUD_CONFIG_ERROR_NOGLOBAL,
	WMUD_CONFIG_ERROR_BADPORT,
	WMUD_CONFIG_ERROR_NOWORLD
};
#endif /* __WMUD_TYPES_H__ */

