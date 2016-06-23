#ifndef __WMUD_CORE_WMUD_H__
#define __WMUD_CODE_WMUD_H__

#include <glib.h>

/* DEBUG is defined in config.h, if present.
 * DEBUG is needed for Context, so let's include config.h here.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>

typedef struct _wMUDThreadData
{
	GMainContext *main_context;
	GMainLoop *main_loop;
	GThread *thread;
	gchar *name;
	gboolean running;
} wMUDThreadData;

extern pid_t wmud_pid;

void wmud_shutdown(void);
#ifdef DEBUG
void wmud_print_context(char *filename, int linenum);
#endif /* DEBUG */

#endif /* __WMUD_CORE_WMUD_H__ */

