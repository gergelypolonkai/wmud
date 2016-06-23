#include "wmud-world.h"

#define WMUD_WORLD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), WMUD_TYPE_WORLD, wMUDWorldPrivate))

struct _wMUDWorldPrivate
{
	gchar *name;
};

enum
{
	PROP_0,
	PROP_NAME
};

G_DEFINE_TYPE(wMUDWorld, wmud_world, G_TYPE_OBJECT);

static void
wmud_world_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	wMUDWorld *self = WMUD_WORLD(object);

	switch (property_id)
	{
		case PROP_NAME:
			g_free(self->priv->name);
			self->priv->name = g_value_dup_string(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
wmud_world_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	wMUDWorld *self = WMUD_WORLD(object);

	switch (property_id)
	{
		case PROP_NAME:
			g_value_set_string(value, self->priv->name);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void
wmud_world_dispose(GObject *object)
{
	G_OBJECT_CLASS(wmud_world_parent_class)->dispose(object);
}

static void
wmud_world_finalize(GObject *object)
{
	wMUDWorld *self = WMUD_WORLD(object);

	if (self->priv->name)
	{
		g_free(self->priv->name);
	}

	G_OBJECT_CLASS(wmud_world_parent_class)->finalize(object);
}

static void
wmud_world_class_init(wMUDWorldClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	GParamSpec *wmud_world_param_spec;

	gobject_class->set_property = wmud_world_set_property;
	gobject_class->get_property = wmud_world_get_property;
	gobject_class->dispose      = wmud_world_dispose;
	gobject_class->finalize     = wmud_world_finalize;

	g_type_class_add_private(klass, sizeof(wMUDWorldPrivate));

	wmud_world_param_spec = g_param_spec_string("name", "World name", "Set the name of the world", NULL, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
	g_object_class_install_property(gobject_class, PROP_NAME, wmud_world_param_spec);
}

static void
wmud_world_init(wMUDWorld *self)
{
	wMUDWorldPrivate *priv;

	self->priv = priv = WMUD_WORLD_GET_PRIVATE(self);

	priv->name = NULL;
}

wMUDWorld *
wmud_world_new(void)
{
	wMUDWorld *new_world = g_object_new(WMUD_TYPE_WORLD, NULL);

	return new_world;
}

