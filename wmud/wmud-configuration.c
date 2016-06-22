#include "wmud-configuration.h"

typedef struct _WmudConfigurationPrivate {
    gchar *file_name;
    guint port;
    gchar *database_dsn;
    gchar *admin_email;
    gchar *smtp_server;
    gboolean smtp_tls;
    gchar *smtp_username;
    gchar *smtp_password;
    gchar *smtp_sender;
} WmudConfigurationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(WmudConfiguration,
                           wmud_configuration,
                           G_TYPE_KEY_FILE);

static void
wmud_configuration_finalize(GObject *gobject)
{
    g_signal_handlers_destroy(gobject);
    G_OBJECT_CLASS(wmud_configuration_parent_class)->finalize(gobject);
}

static void
wmud_configuration_class_init(WmudConfigurationClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->finalize = wmud_configuration_finalize;
}

static void
wmud_configuration_init(WmudConfiguration *configuration)
{
}

WmudConfiguration *
wmud_configuration_new(gchar *filename)
{}
