/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * wmudworld.h: header file for the WmudWorld object
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

#ifndef __WMUD_WMUDWORLD_H__
#define __WMUD_WMUDWORLD_H__

#include <glib-object.h>
#include <glib.h>

#define WMUD_TYPE_WORLD            (wmud_world_get_type())
#define WMUD_WORLD(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WMUD_TYPE_WORLD, WmudWorld))
#define WMUD_IS_WORLD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WMUD_TYPE_WORLD))
#define WMUD_WORLD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WMUD_TYPE_WORLD, WmudWorldClass))
#define WMUD_IS_WORLD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WMUD_TYPE_WORLD))
#define WMUD_WORLD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WMUT_TYPE_WORLD, WmudWorldClass))

/**
 * WmudWorld:
 *
 * An online wMUD world
 */
typedef struct _WmudWorld WmudWorld;
typedef struct _WmudWorldClass WmudWorldClass;
typedef struct _WmudWorldPrivate WmudWorldPrivate;

struct _WmudWorld
{
	/*<private>*/
	GObject  parent_instance;
	WmudWorldPrivate *priv;
};

struct _WmudWorldClass
{
	GObjectClass parent_class;
};

WmudWorld *wmud_world_new(void);

#endif /* __WMUD_WMUDWORLD_H__ */
