#ifndef __WMUD_MODULE_H__ 
#define __WMUD_MODULE_H__

#include "wmud-module.h"

#define wmud_log(level, filename, linenum, fmt, ...) ((void (*)(wMUDLogLevelType, const char *, const int, const char *, ...))(wMUDCallables[WMUD_CALLABLE_WMUD_LOG]))(level, filename, linenum, fmt, ## __VA_ARGS__)
#define wmud_print_context(filename, linenum) ((void (*)(char *, int))(wMUDCallables[WMUD_CALLABLE_WMUD_PRINT_CONTEXT]))(filename, linenum)

#endif /* __WMUD_MODULE_H__ */

