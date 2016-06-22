#ifndef __WMUD_CONFIGURATION_H__
#define __WMUD_CONFIGURATION_H__

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
    GKeyFile parent_instance;

    /* Instance members */
};

struct _WmudConfigurationClass {
    GKeyFileClass parent_class;
};

GType wmud_configuration_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif /* __WMUD_CONFIGURATION_H__ */
