#ifndef __WMUD_MAIN_H__
# define __WMUD_MAIN_H__

# include <glib.h>

extern GMainContext *game_context;
extern guint32 elapsed_seconds;
extern GRand *main_rand;

#define random_number(a, b) g_rand_int_range(main_rand, (a), (b) + 1)

#endif /* __WMUD_MAIN_H__ */

