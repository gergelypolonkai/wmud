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

