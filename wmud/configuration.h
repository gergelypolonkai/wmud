#ifndef __WMUD_CONFIGURATION_H__
#define __WMUD_CONFIGURATION_H__

#include <glib.h>

extern GQuark WMUD_CONFIG_ERROR;

/**
 * wmudConfigError:
 * @WMUD_CONFIG_ERROR_NOGLOBAL: Indicates that the config file read doesn't
 *     contain a [global] section
 * @WMUD_CONFIG_ERROR_BADPORT: Indicates that the config file contains and
 *     invalid port number
 * @WMUD_CONFIG_ERROR_NOWORLD: Indicates that the config file doesn't contain a
 *     world database file
 * @WMUD_CONFIG_ERROR_NOEMAIL: Indicates that the config file doesn't contain
 *     an administrator e-mail address
 * @WMUD_CONFIG_ERROR_REUSE: configuration data is reused (non-NULL)
 *
 * Error codes returned by configuration file parsing functions.
 */
typedef enum {
	WMUD_CONFIG_ERROR_NOGLOBAL,
	WMUD_CONFIG_ERROR_BADPORT,
	WMUD_CONFIG_ERROR_NOWORLD,
	WMUD_CONFIG_ERROR_NOEMAIL,
	WMUD_CONFIG_ERROR_REUSE
} wmudConfigError;

/**
 * ConfigData:
 * @port: the port number of the game interface to listen on
 * @database_file: the database file of the world associated with this
 *                 configuration
 * @admin_email: the world administrator's e-mail address
 */
typedef struct _ConfigData {
	guint port;
	gchar *database_file;
	gchar *admin_email;
} ConfigData;

extern ConfigData *active_config;

gboolean wmud_config_init(ConfigData **config_data, GError **err);

#endif /* __WMUD_CONFIGURATION_H__ */
