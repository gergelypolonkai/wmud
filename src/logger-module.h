#ifndef __WMUD_MODULE_LOGGER_H__
#define __WMUD_MODULE_LOGGER_H__

typedef enum
{
	WMUD_LOG_DEBUG,
	WMUD_LOG_INFO,
	WMUD_LOG_WARN,
	WMUD_LOG_ERROR
} wMUDLogLevelType;

#define wmud_log_info(x, ...)    wmud_log(WMUD_LOG_INFO,  __FILE__, __LINE__, x, ## __VA_ARGS__)
#define wmud_log_warning(x, ...) wmud_log(WMUD_LOG_WARN,  __FILE__, __LINE__, x, ## __VA_ARGS__)
#define wmud_log_error(x, ...)   wmud_log(WMUD_LOG_ERROR, __FILE__, __LINE__, x, ## __VA_ARGS__)
#if DEBUG
#define wmud_log_debug(x, ...)   wmud_log(WMUD_LOG_DEBUG, __FILE__, __LINE__, x, ## __VA_ARGS__)
#else /* ! DEBUG */
#define wmud_log_debug(x, ...)
#endif /* DEBUG */

#endif /* __WMUD_MODULE_LOGGER_H__ */

