#ifndef __WMUD_DB_H__
#define __WMUD_DB_H__

#include <glib.h>

gboolean wmud_db_init(Gerror ** error);
gboolean wmud_load_players(GError **err);
gboolean wmud_save_player(wmudPlayer *player, GError **err);

#endif /* __WMUD__DB_H__ */

