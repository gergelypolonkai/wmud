#ifndef __WMUD_CORE_LOGGER_H__
#define __WMUD_CORE_LOGGER_H__

#include <stdarg.h>
#include <glib.h>

#include "configfile.h"

#include "logger-module.h"
#include "wmud-module.h"

void wmud_log(wMUDLogLevelType level, const char *filename, const int linenum, const char *fmt, ...);
gboolean wmud_logger_init(wMUDConfiguration *config);

#endif /* __WMUD_CORE_LOGGER_H__ */

