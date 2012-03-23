#ifndef __WMUD_TYPES_H__
#define __WMUD_TYPES_H__

#include <glib.h>
#include <gio/gio.h>

typedef enum {
	WMUD_CLIENT_STATE_FRESH,             /* Newly connected clients. We are waiting for
	                                      * a player name */
	WMUD_CLIENT_STATE_PASSWAIT,          /* Player name entered, waiting for password */
	WMUD_CLIENT_STATE_MENU,              /* Logged in players, waiting in the main menu.
	                                      * We are waiting for a menu item to be
	                                      * chosen.*/
	WMUD_CLIENT_STATE_INGAME,            /* Player is in-game */
	WMUD_CLIENT_STATE_QUITWAIT,          /* Waiting for answer for the quit question */
	WMUD_CLIENT_STATE_NEWCHAR,           /* Waiting for answer for the new
					      * character question */
	WMUD_CLIENT_STATE_REGISTERING,       /* Player registration starts */
	WMUD_CLIENT_STATE_REGEMAIL_CONFIRM   /* Waiting for e-mail address confirmation */
} wmudClientState;

typedef struct _wmudPlayer {
	guint32 id;           /* User ID */
	gchar *player_name;   /* Player login name */
	gchar *cpassword;     /* Crypted password */
	gchar *email;         /* E-mail address */
} wmudPlayer;

typedef struct _wmudClient {
	GSocket *socket;
	GSource *socket_source;
	GString *buffer;
	wmudClientState state;
	gboolean authenticated;
	wmudPlayer *player;
	gboolean bademail;
} wmudClient;

enum {
	WMUD_CONFIG_ERROR_SUCCESS,
	WMUD_CONFIG_ERROR_NOGLOBAL,
	WMUD_CONFIG_ERROR_BADPORT,
	WMUD_CONFIG_ERROR_NOWORLD,
	WMUD_CONFIG_ERROR_NOEMAIL
};

enum {
	WMUD_DB_ERROR_SUCCESS,
	WMUD_DB_ERROR_CANTOPEN,
	WMUD_DB_ERROR_NOINIT,
	WMUD_DB_ERROR_BADQUERY
};

#endif /* __WMUD_TYPES_H__ */

