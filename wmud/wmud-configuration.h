#ifndef __WMUD_WMUD_CONFIGURATION_H__
#define __WMUD_WMUD_CONFIGURATION_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define WMUD_TYPE_CONFIGURATION         (wmud_configuration_get_type())
#define WMUD_CONFIGURATION(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), WMUD_TYPE_CONFIGURATION, WmudConfiguration))
#define WMUD_CONFIGURATION_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), WMUD_TYPE_CONFIGURATION, WmudConfigurationClass))
#define WMUD_IS_CONFIGURATION(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), WMUD_TYPE_CONFIGURATION))
#define WMUD_IS_CONFIGURATION_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), WMUD_TYPE_CONFIGURATION))
#define WMUD_CONFIGURATION_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), WMUD_TYPE_CONFIGURATION, WmudConfigurationClass))

typedef struct _WmudConfiguration      WmudConfiguration;
typedef struct _WmudConfigurationClass WmudConfigurationClass;

struct _WmudConfiguration {
    /* Parent instance structure */
    GObject parent_instance;

    /* Instance members */
};

struct _WmudConfigurationClass {
    GObjectClass parent_class;
};

GType wmud_configuration_get_type(void) G_GNUC_CONST;

WmudConfiguration *wmud_configuration_new(void);
void wmud_configuration_update_from_cmdline(WmudConfiguration *configuration,
                                            gint *argc,
                                            gchar **argv[],
                                            GError **error);
void wmud_configuration_update_from_file(WmudConfiguration *configuration,
                                         gchar *filename,
                                         GError **error);

gchar *wmud_configuration_get_filename(WmudConfiguration *configuration);

G_END_DECLS

#endif /* __WMUD_WMUD_CONFIGURATION_H__ */
