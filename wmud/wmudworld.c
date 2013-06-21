/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * wmudworld.c: the WmudWorld object
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
#include "wmudworld.h"

/**
 * SECTION:wmudworld
 * @short_description: wMUD World
 * @include: wmudworld.h
 *
 * #WmudWorld is for storing an online world
 */

G_DEFINE_TYPE(WmudWorld, wmud_world, G_TYPE_OBJECT);

#define WMUD_WORLD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WMUD_TYPE_WORLD, WmudWorldPrivate))

struct _WmudWorldPrivate
{
};

static void
wmud_world_dispose(GObject *gobject)
{
	G_OBJECT_CLASS(wmud_world_parent_class)->dispose(gobject);
}

static void
wmud_world_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(wmud_world_parent_class)->finalize(gobject);
}

static void
wmud_world_class_init(WmudWorldClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = wmud_world_dispose;
	gobject_class->finalize = wmud_world_finalize;

	g_type_class_add_private(klass, sizeof(WmudWorldPrivate));
}

static void
wmud_world_init(WmudWorld *self)
{
	self->priv = WMUD_WORLD_GET_PRIVATE(self);
}

WmudWorld *
wmud_world_new(void)
{
	return g_object_new(WMUD_TYPE_WORLD, NULL, NULL);
}
