#ifndef __WMUD_CLIENT_STATE_H__
#define __WMUD_CLIENT_STATE_H__

#include <glib-object.h>

/**
 * WmudClientState:
 * @WMUD_CLIENT_STATE_FRESH: Client is newly connected. Waiting for a login
 *     player name
 * @WMUD_CLIENT_STATE_PASSWAIT: Login player name is entered, waiting for a
 *     login password
 * @WMUD_CLIENT_STATE_MENU: Authentication was successful, player is now in the
 *     main game menu
 * @WMUD_CLIENT_STATE_INGAME: Character login was successful, player is now
 *     in-game
 * @WMUD_CLIENT_STATE_YESNO: Player was asked a yes/no question, and we are
 *     waiting for the answer. client.yesNoCallback MUST be set at this point!
 *     TODO: if wmudClient had a prevState field, and there would be some hooks
 *     that are called before and after the client enters a new state, this
 *     could be a three-state stuff, in which the player can enter e.g ? as
 *     the answer, so they would be presented with the question again.
 * @WMUD_CLIENT_STATE_REGISTERING: Registering a new player. Waiting for the
 *     e-mail address to be given
 * @WMUD_CLIENT_STATE_REGEMAIL_CONFIRM: E-mail address entered séms valid,
 *     waiting for confirmation
 *
 * Game client states.
 */
typedef enum {
	WMUD_CLIENT_STATE_FRESH,
	WMUD_CLIENT_STATE_PASSWAIT,
	WMUD_CLIENT_STATE_MENU,
	WMUD_CLIENT_STATE_INGAME,
	WMUD_CLIENT_STATE_YESNO,
	WMUD_CLIENT_STATE_REGISTERING,
	WMUD_CLIENT_STATE_REGEMAIL_CONFIRM
} WmudClientState;


GType wmud_client_state_get_type (void) G_GNUC_CONST;
#define WMUD_TYPE_CLIENT_STATE (wmud_client_state_get_type())

#endif /* __WMUD_CLIENT_STATE_H__ */

