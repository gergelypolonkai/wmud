#include <glib.h>

#include "protocols.h"

guint wmud_current_protocol_number = 0;

GQuark
wmud_register_protocol(gchar *name, GError **error)
{
	gchar *full_protocol_name;

	if (name == NULL)
	{
		return g_quark_from_string(NULL);
	}

	full_protocol_name = g_strdup_printf("WMUD PROTO %s", name);

	if (g_quark_try_string(full_protocol_name) != 0)
	{
		/* Quark already exists => protocol is already registered */
		if (*error)
		{
			*error = g_error_new(WMUD_ERROR_PROTOCOL, WMUD_ERROR_PROTOCOL_EXISTS, "Protocol %s is already registered", name);
		}

		return 0;
	}

	return g_quark_from_string(full_protocol_name);
}

