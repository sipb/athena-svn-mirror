#include "component.h"
#include <libbonobo.h>
#include <libart_lgpl/art_affine.h>

struct _SampleComponentPrivate {
	Bonobo_Unknown	server;
	gdouble		x;
	gdouble		y;
	gdouble		width;
	gdouble		height;
};

enum SIGNALS {
	CHANGED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0 };

static GObjectClass *component_parent_class;

static void
sample_component_finalize (GObject *obj)
{
	SampleComponent *comp = SAMPLE_COMPONENT (obj);

	bonobo_object_release_unref (comp->priv->server, NULL);

	g_free (comp->priv);

	component_parent_class->finalize (obj);
}

static void
sample_component_class_init (GObjectClass *klass)
{
	component_parent_class = g_type_class_peek_parent (klass);

	signals [CHANGED] = g_signal_new (
		"changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 
		G_STRUCT_OFFSET (SampleComponentClass, changed),
		NULL, NULL, g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);

	klass->finalize = sample_component_finalize;
}

static void
sample_component_instance_init (GObject *obj)
{
	SampleComponent *comp = SAMPLE_COMPONENT (obj);

	comp->priv = g_new0 (SampleComponentPrivate, 1);

	comp->priv->server = CORBA_OBJECT_NIL;
	comp->priv->x = 0.0;
	comp->priv->y = 0.0;
	comp->priv->height = 0.0;
	comp->priv->width = 0.0;
}

GType
sample_component_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (SampleComponentClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) sample_component_class_init,
			NULL,
			NULL, /* class_data */
			sizeof (SampleComponent),
			0, /* n_preallocs */
			(GInstanceInitFunc) sample_component_instance_init
		};
		
		type = g_type_register_static (G_TYPE_OBJECT, 
					       "SampleComponent",
					       &info, 0);
	}

	return type;
}

SampleComponent *
sample_component_new (gchar *object_id)
{
	SampleComponent *comp;
	Bonobo_Unknown server;
	CORBA_Environment ev;

	comp = g_object_new (SAMPLE_COMPONENT_TYPE, NULL);

	CORBA_exception_init (&ev);
	server = bonobo_activation_activate_from_id (object_id, 0, NULL, &ev);
	if ((server == CORBA_OBJECT_NIL) || BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		g_object_unref (G_OBJECT (comp));
		return NULL;
	}

	comp->priv->server = bonobo_object_dup_ref (server, &ev);
	Bonobo_Unknown_unref (server, &ev);
	CORBA_exception_free (&ev);

	return comp;
}

SampleComponent *
sample_component_new_from_storage (Bonobo_Storage storage)
{
	g_warning ("Nope, not yet.  Storage loading be broke still.");
#if 0
	Bonobo_Unknown persist;
	Bonobo_Unknown prop_stream;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	switch (comp->priv->persist_type) {

	case PSTREAM:
		persist = Bonobo_Unknown_queryInterface (
			comp->server, "IDL:Bonobo/PersistStream:1.0", &ev);

		if (persist != CORBA_OBJECT_NIL) {
			Bonobo_PersistStream_load (persist, subdir, "", &ev);
			Bonobo_Unknown_unref (persist, &ev);
		}
		break;

	case PSTORAGE:
		persist = Bonobo_Unknown_queryInterface (
			comp->server, "IDL:Bonobo/PersistStorage:1.0", &ev);

		if (persist != CORBA_OBJECT_NIL) {
			Bonobo_PersistStorage_load (persist, subdir, "", &ev);
			Bonobo_Unknown_unref (persist, &ev);
		}
		break;

	default:
		g_warning ("Unexpected Persist type encountered.");
		return;
	}

	if (BONOBO_EX (&ev)) {
		char *msg = bonobo_exception_get_text (&ev);
		gnome_warning_dialog (msg);
		g_free (msg);
	}
#endif
	return NULL;
}

gboolean
sample_component_is_dirty (SampleComponent *comp)
{
	CORBA_Environment ev;
	Bonobo_Persist persist;
	gboolean dirty = FALSE;

	g_return_val_if_fail (SAMPLE_IS_COMPONENT (comp), FALSE);

	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_queryInterface (
			comp->priv->server, "IDL:Bonobo/Persist:1.0", &ev);
	if (persist != CORBA_OBJECT_NIL) {
		dirty = Bonobo_Persist_isDirty (persist, &ev);
		Bonobo_Unknown_unref (persist, &ev);
	}

	CORBA_exception_free (&ev);
	return FALSE;
}

void
sample_component_move (SampleComponent *comp, gdouble x, gdouble y)
{
	g_return_if_fail (SAMPLE_IS_COMPONENT (comp));

	comp->priv->x = x;
	comp->priv->y = y;

	g_signal_emit (G_OBJECT (comp), signals [CHANGED], 0);
}

void
sample_component_resize (SampleComponent *comp, gdouble width, gdouble height)
{
	g_return_if_fail (SAMPLE_IS_COMPONENT (comp));

	comp->priv->width = width;
	comp->priv->height = height;

	g_signal_emit (G_OBJECT (comp), signals [CHANGED], 0);
}

void
sample_component_get_affine (SampleComponent *comp, gdouble *aff)
{
	art_affine_identity (aff);

	art_affine_translate (aff, comp->priv->x, comp->priv->y);
}

Bonobo_Unknown
sample_component_get_server (SampleComponent *comp)
{
     return comp->priv->server;
}

