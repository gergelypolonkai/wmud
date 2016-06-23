#ifndef __WMUD_WORLD_H__
#define __WMUD_WORLD_H__

#include <glib-object.h>

#define WMUD_TYPE_WORLD            wmud_world_get_type()
#define WMUD_WORLD(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WMUD_TYPE_WORLD, wMUDWorld))
#define WMUD_IS_WORLD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WMUD_TYPE_WORLD))
#define WMUD_WORLD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WMUD_TYPE_WORLD, wMUDWorldClass))
#define WMUD_IS_WORLD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WMUD_TYPE_WORLD))
#define WMUD_WORLD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WMUD_TYPE_WORLD, wMUDWorldClass))

typedef struct _wMUDWorld        wMUDWorld;
typedef struct _wMUDWorldClass   wMUDWorldClass;
typedef struct _wMUDWorldPrivate wMUDWorldPrivate;

struct _wMUDWorld
{
	GObject parent_object;
	
	wMUDWorldPrivate *priv;
};

struct _wMUDWorldClass
{
	GObjectClass parent_class;
};

GType wmud_world_get_type(void);

wMUDWorld *wmud_world_new(void);

#endif /* __WMUD_WORLD_H__ */

