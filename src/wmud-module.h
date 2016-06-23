#ifndef __WMUD_MODULE_WMUD_H__
#define __WMUD_MODULE_WMUD_H__

#include <glib.h>

#include "protocols-module.h"
#include "configfile-module.h"
#include "logger-module.h"

enum _wMUDCallableNum {
	WMUD_CALLABLE_WMUD_LOG,
#ifdef DEBUG
	WMUD_CALLABLE_WMUD_PRINT_CONTEXT,
#endif /* DEBUG */
	WMUD_CALLABLE_LAST
};

extern gpointer wMUDCallables[];

#if DEBUG
#define Context wmud_print_context(__FILE__, __LINE__);
#else
#define Context
#endif

#endif /* __WMUD_MODULE_WMUD_H__ */

