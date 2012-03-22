#ifndef __WMUD_DB_H__
#define __WMUD_DB_H__

#include <glib.h>

#include "wmud_types.h"

gboolean wmud_db_init(GError **err);
gboolean wmud_db_players_load(GError **err);
gboolean wmud_db_save_player(wmudPlayer *player, GError **err);

#endif /* __WMUD__DB_H__ */

