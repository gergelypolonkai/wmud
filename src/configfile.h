#ifndef __WMUD_CORE_CONFIGFILE_H__
#define __WMUD_CORE_CONFIGFILE_H__

#include <glib.h>

#include "wmud-session.h"

#include "configfile-module.h"

extern wMUDConfiguration *wmud_configuration;

gboolean wmud_configfile_read(gchar *filename, wMUDConfiguration **configuration, GError **error);
void wmud_configuration_free(wMUDConfiguration **configuration);

#endif /* __WMUD_CORE_CONFIGFILE_H__ */

