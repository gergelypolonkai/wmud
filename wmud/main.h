/* wMUD - Yet another MUD codebase by W00d5t0ck
 * Copyright (C) 2012 - Gergely POLONKAI
 *
 * main.h: main and uncategorized functions
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
#ifndef __WMUD_MAIN_H__
# define __WMUD_MAIN_H__

# include <glib.h>

extern GMainContext *game_context;
extern guint32 elapsed_seconds;

/**
 * random_number:
 * @min: Minimum value for random number
 * @max: Maximum value for random number
 *
 * Generates a random number between min and max
 */
#define random_number(min, max) g_random_int_range((min), (max) + 1)

gchar *wmud_random_string(gint len);

#endif /* __WMUD_MAIN_H__ */

