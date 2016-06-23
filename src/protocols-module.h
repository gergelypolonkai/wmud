#ifndef __WMUD_MODULE_PROTOCOLS_H__
#define __WMUD_MODULE_PROTOCOLS_H__

#include <glib.h>

enum {
	WMUD_ERROR_PROTOCOL_OK,
	WMUD_ERROR_PROTOCOL_EXISTS
};

#define WMUD_ERROR_PROTOCOL g_quark_from_string("WMUD PROTOCOL ERROR")
GQuark wmud_register_protocol(gchar *name, GError **error);

#endif /* __WMUD_MODULE_PROTOCOLS_H__ */

