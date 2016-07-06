#include "wmud-configuration.h"

typedef struct _WmudConfigurationPrivate {
    gchar *file_name;
    GKeyFile *key_file;
    gchar *admin_email;
    gboolean *hide_single_race;
    gboolean *hide_single_class;
    guint *house_occupy_time;
    guint *minimum_deities;
    gboolean *clan_wars;
    guint *maximum_group_size;
    guint *trainable_abilities;
    gboolean *reborn;
} WmudConfigurationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(WmudConfiguration,
                           wmud_configuration,
                           G_TYPE_OBJECT);

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
    WmudConfigurationPrivate *priv = wmud_configuration_get_instance_private(
            configuration);

    priv->key_file            = NULL;
    priv->file_name           = NULL;
    priv->admin_email         = NULL;
    priv->hide_single_race    = NULL;
    priv->hide_single_class   = NULL;
    priv->house_occupy_time   = NULL;
    priv->minimum_deities     = NULL;
    priv->clan_wars           = NULL;
    priv->maximum_group_size  = NULL;
    priv->trainable_abilities = NULL;
    priv->reborn              = NULL;
}

void
wmud_configuration_update_from_cmdline(WmudConfiguration *configuration,
                                       gint *argc,
                                       gchar **argv[],
                                       GError **error)
{
    WmudConfigurationPrivate *priv = wmud_configuration_get_instance_private(
            configuration);
    GOptionEntry entries[] = {
        {
            "config-file", 'c',
            0,
            G_OPTION_ARG_FILENAME,
            &(priv->file_name),
            "The name of the configuration file to parse",
            "FILE"
        },
        {NULL}
    };
    GError *err = NULL;
    GOptionContext *context;

    context = g_option_context_new("- Yet Another MUD Engine");
    g_option_context_add_main_entries(context, entries, NULL);

    if (!g_option_context_parse(context, argc, argv, &err)) {
        g_print("Option parsing failed: %s\n", err->message);

        g_object_unref(configuration);

        // TODO: Update error!

        return;
    }

    g_print("Config file: %s\n", priv->file_name);
}

void
wmud_configuration_update_from_file(WmudConfiguration *configuration,
                                    gchar *filename,
                                    GError **error)
{
    WmudConfigurationPrivate *priv = wmud_configuration_get_instance_private(
            configuration);

    if (filename != NULL) {
        g_free(priv->file_name);
        priv->file_name = g_strdup(filename);
    } else if (priv->file_name == NULL) {
        priv->file_name = g_strdup(WMUD_CONFDIR "/wmud.conf");
    }

    priv->key_file = g_key_file_new();

    if (!g_key_file_load_from_file(priv->key_file,
                                   filename,
                                   G_KEY_FILE_NONE,
                                   error)) {
        return;
    }
}

WmudConfiguration *
wmud_configuration_new(void)
{
    WmudConfiguration *configuration = g_object_new(
            WMUD_TYPE_CONFIGURATION, NULL);

    // TODO: Update with built-in defaults

    return configuration;
}

gchar *
wmud_configuration_get_filename(WmudConfiguration *configuration)
{
    WmudConfigurationPrivate *priv = wmud_configuration_get_instance_private(
            configuration);

    return priv->file_name;
}
