/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * world.h: world loading and building functions
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
#ifndef __WMUD_WORLD_H__
#define __WMUD_WORLD_H__

gboolean wmud_world_check_planes(GSList *planes, GError **err);
gboolean wmud_world_check_planets(GSList *planets, GError **err);
gboolean wmud_world_check_areas(GSList *areas, GError **err);
gboolean wmud_world_check_exits(GSList *exits, GError **err);

gboolean wmud_world_assoc_planets_planes(GSList *planets, GSList *planes, GError **err);
gboolean wmud_world_assoc_rooms_areas(GSList *rooms, GSList *areas, GError **err);
gboolean wmud_world_assoc_rooms_planets(GSList *rooms, GSList *planets, GError **err);
gboolean wmud_world_assoc_exits_rooms(GSList *exits, GSList *rooms, GError **err);

#endif /* __WMUD_WORLD_H__ */

