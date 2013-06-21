/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * wmudplayer.c: the WmudPlayer GObject type
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
#include "wmudplayer.h"

#include <crypt.h>

#include "players.h"

#define WMUD_PLAYER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WMUD_TYPE_PLAYER, WmudPlayerPrivate))

struct _WmudPlayerPrivate
{
	guint32 id;
	gchar *player_name;
	gchar *cpassword;
	gchar *email;
	gint fail_count;
	gboolean registered;
};

G_DEFINE_TYPE(WmudPlayer, wmud_player, G_TYPE_OBJECT);

static void
wmud_player_dispose(GObject *gobject)
{
	//WmudPlayer *self = WMUD_PLAYER(gobject);

	G_OBJECT_CLASS(wmud_player_parent_class)->dispose(gobject);
}

static void
wmud_player_finalize(GObject *gobject)
{
	WmudPlayer *self = WMUD_PLAYER(gobject);

	if (self->priv->player_name)
		g_free(self->priv->player_name);

	if (self->priv->cpassword)
		g_free(self->priv->cpassword);

	if (self->priv->email)
		g_free(self->priv->email);

	G_OBJECT_CLASS(wmud_player_parent_class)->finalize(gobject);
}

static void
wmud_player_class_init(WmudPlayerClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = wmud_player_dispose;
	gobject_class->finalize = wmud_player_finalize;

	g_type_class_add_private(klass, sizeof(WmudPlayerPrivate));
}

static void
wmud_player_init(WmudPlayer *self)
{
	self->priv = WMUD_PLAYER_GET_PRIVATE(self);
}

WmudPlayer *
wmud_player_new(void)
{
	return g_object_new(WMUD_TYPE_PLAYER, NULL, NULL);
}

void
wmud_player_set_cpassword(WmudPlayer *self, const gchar *cpassword)
{
	if (self->priv->cpassword)
		g_free(self->priv->cpassword);

	self->priv->cpassword = g_strdup(cpassword);
}

gchar *
wmud_player_get_cpassword(WmudPlayer *self)
{
	return self->priv->cpassword;
}

void
wmud_player_set_registered(WmudPlayer *self, gboolean registered)
{
	self->priv->registered = registered;
}

gboolean
wmud_player_get_registered(WmudPlayer *self)
{
	return self->priv->registered;
}

void
wmud_player_set_player_name(WmudPlayer *self, const gchar *name)
{
	self->priv->player_name = g_strdup(name);
}

gchar *
wmud_player_get_player_name(WmudPlayer *self)
{
	return self->priv->player_name;
}

void
wmud_player_reset_fail_count(WmudPlayer *self)
{
	self->priv->fail_count = 0;
}

void
wmud_player_increase_fail_count(WmudPlayer *self)
{
	self->priv->fail_count++;
}

gint
wmud_player_get_fail_count(WmudPlayer *self)
{
	return self->priv->fail_count;
}

void
wmud_player_set_email(WmudPlayer *self, const gchar *email)
{
	if (self->priv->email)
		g_free(self->priv->email);

	self->priv->email = g_strdup(email);
}

gchar *
wmud_player_get_email(WmudPlayer *self)
{
	return self->priv->email;
}

void
wmud_player_set_id(WmudPlayer *self, guint32 id)
{
	self->priv->id = id;
}

guint32
wmud_player_get_id(WmudPlayer *self)
{
	return self->priv->id;
}

/**
 * wmud_player_dup:
 * @player: the object structure to duplicate
 *
 * Duplicates a #WmudPlayer object.
 *
 * Return value: the new, duplicated player object.
 */
WmudPlayer *
wmud_player_dup(WmudPlayer *self)
{
	WmudPlayer *new_player;

	if (!self)
		return NULL;

	new_player = wmud_player_new();
	new_player->priv->id = self->priv->id;
	new_player->priv->player_name = g_strdup(self->priv->player_name);
	new_player->priv->cpassword = g_strdup(self->priv->cpassword);
	new_player->priv->email = g_strdup(self->priv->email);

	return new_player;
}

/**
 * wmud_player_password_valid:
 * @player:   The client to be authenticated
 * @password: The password in clear text
 *
 * Tries to authenticate a client with the name stored in the player object,
 * and the password provided.
 *
 * Return value: %TRUE if the password is valid, %FALSE otherwise.
 */
gboolean
wmud_player_password_valid(WmudPlayer *player, const gchar *password)
{
	return (g_strcmp0(crypt(password, player->priv->cpassword), player->priv->cpassword) == 0);
}

