#include "wmud-module.h"

gboolean
wmud_protocol_telnet_load(wMUDConfiguration *configuration)
{
	/*
	GSList *temp;
	wMUDConfigurationInterface *interface;
	gchar *iface_name;
	struct _wMUDIfaceFinder *finder;

	finder = g_new0(struct _wMUDIfaceFinder, 1);

	finder->type = WMUD_SESSION_TYPE_TELNET;
	finder->name = iface_name = g_utf8_offset_to_pointer(group, 7);
	Context;
	temp = g_slist_find_custom(conf->interfaces, finder, _wmud_find_iface);
	Context;
	g_free(finder);

	if (temp == NULL)
	{
		Context;
		interface = (wMUDConfigurationInterface *)g_new0(wMUDConfigurationInterface, 1);
		interface->name = g_strdup(iface_name);
		interface->type = WMUD_SESSION_TYPE_TELNET;
		conf->interfaces = g_slist_append(conf->interfaces, interface);
	}
	else
	{
		Context;
		interface = (wMUDConfigurationInterface *)(temp->data);
	}

	if (g_utf8_collate("port", key) == 0)
	{
		guint64 portnumber;
		gchar *endptr;

		portnumber = g_ascii_strtoull(value, &endptr, 10);

		if ((endptr != NULL) && (*endptr != 0))
		{
			wmud_log_error("Error in configuration file. Value of port can only contain numbers in group [%s]!", group);
		}

		if (interface->inetaddr == NULL)
		{
			interface->inetaddr = gnet_inetaddr_new("0.0.0.0", portnumber);
		}
		else
		{
			gnet_inetaddr_set_port(interface->inetaddr, (gint)portnumber);
		}

		g_pattern_spec_free(group_ptn);
		return TRUE;
	}

	if (g_utf8_collate("address", key) == 0)
	{
		GInetAddr *temp = gnet_inetaddr_new(value, 0);

		if (interface->inetaddr)
		{
			gnet_inetaddr_set_port(temp, gnet_inetaddr_get_port(interface->inetaddr));
			gnet_inetaddr_unref(interface->inetaddr);
			interface->inetaddr = NULL;
		}
		interface->inetaddr = temp;

		g_pattern_spec_free(group_ptn);
		return TRUE;
	}

	if (g_utf8_collate("timeout", key) == 0)
	{
		guint64 timeout;
		gchar *endptr;

		timeout = g_ascii_strtoull(value, &endptr, 10);

		if ((endptr != NULL) && (*endptr != 0))
		{
			wmud_log_error("Error in configuration file. Value of timeout can only contain numbers! in group [%s]", group);
		}

		interface->timeout = (guint)timeout;

		g_pattern_spec_free(group_ptn);
		return TRUE;
	}
	*/

	return TRUE;
}

void
wmud_protocol_telnet_unload(void)
{
}

