#ifndef __WMUD_CORE_MODULES_H__
#define __WMUD_CORE_MODULES_H__

#include <glib.h>

#include "configfile.h"

void wmud_fill_callable_table(void);
gboolean wmud_load_config_modules(wMUDConfiguration *config);

#endif /* __WMUD_CORE_MODULES_H__ */

