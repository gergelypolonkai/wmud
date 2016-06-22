#ifndef __WMUD_CONFIGURATION_H__
#define __WMUD_CONFIGURATION_H__

#include <glib.h>

#define WMUD_CONFIG_ERROR wmud_config_error_quark()
GQuark wmud_config_error_quark();


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
 * @WMUD_CONFIG_ERROR_NOSMTP: Indicates that the config file read doesn't
 *     contain an [SMTP] section
 * @WMUD_CONFIG_ERROR_NOSMTPSERVER: Indicates that the config file read doesn't
 *     contain an SMTP server address
 * @WMUD_CONFIG_ERROR_NOSMTPSENDER: Indicates that the config file read doesn't
 *     contain an SMTP sender address
 * @WMUD_CONFIG_ERROR_NODATABASE: Indicates that the config file read doesn't
 *     contain a [database] section
 *
 * Error codes returned by configuration file parsing functions.
 */
typedef enum {
    WMUD_CONFIG_ERROR_NOGLOBAL,
    WMUD_CONFIG_ERROR_BADPORT,
    WMUD_CONFIG_ERROR_NOWORLD,
    WMUD_CONFIG_ERROR_NOEMAIL,
    WMUD_CONFIG_ERROR_REUSE,
    WMUD_CONFIG_ERROR_NOSMTP,
    WMUD_CONFIG_ERROR_NOSMTPSERVER,
    WMUD_CONFIG_ERROR_NOSMTPSENDER,
    WMUD_CONFIG_ERROR_NODATABASE
} wmudConfigError;

/**
 * ConfigData:
 * @port: the port number of the game interface to listen on
 * @database_dsn: the data source name for the world associated with this
 *     configuration
 * @admin_email: the world administrator's e-mail address
 * @smtp_server: the SMTP server address to send mails through
 * @smtp_tls: defines wether a TLS connection should be used for the SMTP
 *     server
 * @smtp_username: the username to use while authenticating to the SMTP server
 * @smtp_password: the password to use while authenticating to the SMTP server
 * @smtp_sender: the sender address who sends the outgoing mails
 */
typedef struct _ConfigData {
    guint    port;
    gchar    *database_dsn;
    gchar    *admin_email;
    gchar    *smtp_server;
    gboolean smtp_tls;
    gchar    *smtp_username;
    gchar    *smtp_password;
    gchar    *smtp_sender;
} ConfigData;

extern ConfigData *active_config;

gboolean wmud_config_init(ConfigData **config_data,
                          GError     **err);

#endif /* __WMUD_CONFIGURATION_H__ */
