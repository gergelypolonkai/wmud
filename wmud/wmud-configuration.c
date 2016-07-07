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
    gboolean *trainable_abilities;
    gboolean *reborn;
} WmudConfigurationPrivate;

// The following defines are the default values for configurable
// items. These can be changed in run time!

// If there is only one race, should the race changing step be hidden?
#define DEFAULT_HIDE_SINGLE_RACE FALSE

// If there is only one class, should the class changing step be hidden?
#define DEFAULT_HIDE_SINGLE_CLASS FALSE

// If the owner of the house is not present to this many (real world)
// days, their house can be occupied (traps and the like may still
// trigger, though! If this is set to 0, houses cannot be occupied.
#define DEFAULT_HOUSE_OCCUPY_TIME 0

// The minimum number of deities that must be choosen during character
// generation. If the number of available deities is less than this,
// it will be decremented runtime!
#define DEFAULT_MINIMUM_DEITIES 0

// TRUE if clans can declare war on each other (ie. PvP)
#define DEFAULT_CLAN_WARS FALSE

// The maximum size of a group
#define DEFAULT_MAXIMUM_GROUP_SIZE 5

// If TRUE, abilities (Strength, Intelligence, etc.) are trainable
#define DEFAULT_TRAINABLE_ABILITIES FALSE

// If TRUE, characters can choose to reborn (ie. reroll)
#define DEFAULT_REBORN FALSE

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
                                   priv->file_name,
                                   G_KEY_FILE_NONE,
                                   error)) {
        return;
    }
}

#define wmud_new(type, var, value) \
    var = g_new(type, 1);            \
    *(var) = value;

WmudConfiguration *
wmud_configuration_new(void)
{
    WmudConfiguration *configuration = g_object_new(
            WMUD_TYPE_CONFIGURATION, NULL);
    WmudConfigurationPrivate *priv = wmud_configuration_get_instance_private(
            configuration);

    wmud_new(gboolean, priv->hide_single_race, DEFAULT_HIDE_SINGLE_RACE);
    wmud_new(gboolean, priv->hide_single_class, DEFAULT_HIDE_SINGLE_CLASS);
    wmud_new(guint, priv->house_occupy_time, DEFAULT_HOUSE_OCCUPY_TIME);
    wmud_new(guint, priv->minimum_deities, DEFAULT_MINIMUM_DEITIES);
    wmud_new(gboolean, priv->clan_wars, DEFAULT_CLAN_WARS);
    wmud_new(guint, priv->maximum_group_size, DEFAULT_MAXIMUM_GROUP_SIZE);
    wmud_new(gboolean, priv->trainable_abilities, DEFAULT_TRAINABLE_ABILITIES);
    wmud_new(gboolean, priv->reborn, DEFAULT_REBORN);

    return configuration;
}

gchar *
wmud_configuration_get_filename(WmudConfiguration *configuration)
{
    WmudConfigurationPrivate *priv = wmud_configuration_get_instance_private(
            configuration);

    return priv->file_name;
}
