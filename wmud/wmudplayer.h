/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * wmudplayer.h: the WmudPlayer GObject type
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __WMUD_WMUDPLAYER_H__
#define __WMUD_WMUDPLAYER_H__

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>

#define WMUD_TYPE_PLAYER            (wmud_player_get_type())
#define WMUD_PLAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WMUD_TYPE_PLAYER, WmudPlayer))
#define WMUD_IS_PLAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WMUD_TYPE_PLAYER))
#define WMUD_PLAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WMUD_TYPE_PLAYER, WmudPlayerClass))
#define WMUD_IS_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WMUD_TYPE_PLAYER))
#define WMUD_PLAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WMUT_TYPE_PLAYER, WmudPlayerClass))

typedef struct _WmudPlayer        WmudPlayer;
typedef struct _WmudPlayerClass   WmudPlayerClass;
typedef struct _WmudPlayerPrivate WmudPlayerPrivate;

struct _WmudPlayer {
    /*< private >*/
    GObject parent_instance;

    /*< private >*/
    WmudPlayerPrivate *priv;
};

struct _WmudPlayerClass {
    GObjectClass parent_class;
};

GType wmud_player_get_type(void);
WmudPlayer *wmud_player_new(void);
void wmud_player_set_cpassword(WmudPlayer  *player,
                               const gchar *cpassword);
gchar *wmud_player_get_cpassword(WmudPlayer *player);
void wmud_player_set_registered(WmudPlayer *player,
                                gboolean   registered);
gboolean wmud_player_get_registered(WmudPlayer *player);
void wmud_player_set_player_name(WmudPlayer  *player,
                                 const gchar *name);
gchar *wmud_player_get_player_name(WmudPlayer *player);
gint wmud_player_get_fail_count(WmudPlayer *player);
void wmud_player_increase_fail_count(WmudPlayer *player);
void wmud_player_reset_fail_count(WmudPlayer *player);
void wmud_player_set_email(WmudPlayer  *player,
                           const gchar *email);
gchar *wmud_player_get_email(WmudPlayer *player);
void wmud_player_set_id(WmudPlayer *player,
                        guint32    id);
guint32 wmud_player_get_id(WmudPlayer *player);
WmudPlayer *wmud_player_dup(WmudPlayer *player);
gboolean wmud_player_password_valid(WmudPlayer  *player,
                                    const gchar *password);

#endif /* __WMUD_WMUDPLAYER_H__ */
