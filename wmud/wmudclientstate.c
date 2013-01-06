#include "wmudclientstate.h"

GType
wmud_client_state_get_type (void)
{
	static volatile gsize g_define_type_id__volatile = 0;

	if (g_once_init_enter(&g_define_type_id__volatile)) {
		static const GEnumValue values[] = {
			{ WMUD_CLIENT_STATE_FRESH,            "WMUD_CLIENT_STATE_FRESH",            "fresh"            },
			{ WMUD_CLIENT_STATE_PASSWAIT,         "WMUD_CLIENT_STATE_PASSWAIT",         "passwait"         },
			{ WMUD_CLIENT_STATE_MENU,             "WMUD_CLIENT_STATE_MENU",             "menu"             },
			{ WMUD_CLIENT_STATE_INGAME,           "WMUD_CLIENT_STATE_INGAME",           "ingame"           },
			{ WMUD_CLIENT_STATE_YESNO,            "WMUD_CLIENT_STATE_YESNO",            "yesno"            },
			{ WMUD_CLIENT_STATE_REGISTERING,      "WMUD_CLIENT_STATE_REGISTERING",      "registering"      },
			{ WMUD_CLIENT_STATE_REGEMAIL_CONFIRM, "WMUD_CLIENT_STATE_REGEMAIL_CONFIRM", "regemail-confirm" },
			{ 0,                                  NULL,                                 NULL }
		};
		GType g_define_type_id = g_enum_register_static(g_intern_static_string("WmudClientState"), values);
		g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
	}

	return g_define_type_id__volatile;
}

