/**
 * bonobo-subproperty.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include <bonobo/bonobo-arg.h>

#include "bonobo-subproperty.h"


#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

#define GET_PROPERTY(servant) BONOBO_SUB_PROPERTY(bonobo_object_from_servant (servant))

/* Parent object class in GTK hierarchy */
static BonoboObjectClass *bonobo_sub_property_parent_class;

struct _BonoboSubPropertyPrivate {
	BonoboPEditor             *editor;
	gchar                     *name;
	CORBA_any                 *value;
	int                        index;
	BonoboEventSource         *es;
	BonoboSubPropertyChangeFn  change_fn;
};

static CORBA_char *
impl_Bonobo_Property_getName (PortableServer_Servant servant,
			      CORBA_Environment *ev)
{
	BonoboSubProperty *prop = GET_PROPERTY (servant);
	
	return CORBA_string_dup (prop->priv->name);
}

static CORBA_TypeCode
impl_Bonobo_Property_getType (PortableServer_Servant servant,
			      CORBA_Environment *ev)
{
	BonoboSubProperty *prop = GET_PROPERTY (servant);
	CORBA_TypeCode tc;

	tc =  prop->priv->value->_type;

	return (CORBA_TypeCode) CORBA_Object_duplicate ((CORBA_Object) tc, ev);
}

static CORBA_any *
impl_Bonobo_Property_getValue (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	BonoboSubProperty *prop = GET_PROPERTY (servant);

	return bonobo_arg_copy (prop->priv->value);
}

static void
impl_Bonobo_Property_setValue (PortableServer_Servant servant,
			       const CORBA_any       *any,
			       CORBA_Environment     *ev)
{
	BonoboSubProperty *prop = GET_PROPERTY (servant);

	if (!bonobo_arg_type_is_equal (any->_type, prop->priv->value->_type,
				       NULL))
		return;

	if (bonobo_arg_is_equal (prop->priv->value, any, NULL)) 
		return;
    
	prop->priv->change_fn (prop->priv->editor, any, prop->priv->index);
}

static CORBA_any *
impl_Bonobo_Property_getDefault (PortableServer_Servant servant,
				 CORBA_Environment *ev)
{
	return NULL;
}

static CORBA_char *
impl_Bonobo_Property_getDocString (PortableServer_Servant servant,
				   CORBA_Environment *ev)
{
	return NULL;
}

static CORBA_long
impl_Bonobo_Property_getFlags (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	return 0;
}

static Bonobo_EventSource_ListenerId
impl_Bonobo_Property_addListener (PortableServer_Servant servant,
				  const Bonobo_Listener  l,
				  CORBA_Environment     *ev)
{
	BonoboSubProperty *prop = GET_PROPERTY (servant);
	Bonobo_Unknown corba_es = BONOBO_OBJREF (prop->priv->es);
	Bonobo_EventSource_ListenerId id = 0;
	char *mask;

	mask = g_strdup_printf ("Bonobo/Property:change:%s", 
				prop->priv->name);

	id = Bonobo_EventSource_addListenerWithMask (corba_es, l, mask, ev); 

	g_free (mask);

	return id;
}

static void
impl_Bonobo_Property_removeListener (PortableServer_Servant servant,
				     const Bonobo_EventSource_ListenerId id,
				     CORBA_Environment     *ev)
{
	BonoboSubProperty *prop = GET_PROPERTY (servant);
	Bonobo_Unknown corba_es = BONOBO_OBJREF (prop->priv->es);

	Bonobo_EventSource_removeListener (corba_es, id, ev); 
	return;
}

static void
bonobo_sub_property_destroy (GtkObject *object)
{
	BonoboSubProperty *prop = BONOBO_SUB_PROPERTY (object);

	if (prop->priv->name)
		g_free (prop->priv->name);

	if (prop->priv->value)
		CORBA_free (prop->priv->value);

	bonobo_object_unref (BONOBO_OBJECT(prop->priv->editor));

	g_free (prop->priv);

	GTK_OBJECT_CLASS (bonobo_sub_property_parent_class)->destroy (object);
}

static void
bonobo_sub_property_class_init (BonoboSubPropertyClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *)klass;
	POA_Bonobo_Property__epv *epv;

	bonobo_sub_property_parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy  = bonobo_sub_property_destroy;
	
	epv = &klass->epv;

	epv->getName        = impl_Bonobo_Property_getName;
	epv->getType        = impl_Bonobo_Property_getType;
	epv->getValue       = impl_Bonobo_Property_getValue;
	epv->setValue       = impl_Bonobo_Property_setValue;
	epv->getDefault     = impl_Bonobo_Property_getDefault;
	epv->getDocString   = impl_Bonobo_Property_getDocString;
	epv->getFlags       = impl_Bonobo_Property_getFlags;
	epv->addListener    = impl_Bonobo_Property_addListener;
	epv->removeListener = impl_Bonobo_Property_removeListener;
}

static void
bonobo_sub_property_init (BonoboSubProperty *property)
{
	property->priv = g_new0 (BonoboSubPropertyPrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboSubProperty, 
			 Bonobo_Property,
			 PARENT_TYPE,
			 bonobo_sub_property);

BonoboSubProperty *
bonobo_sub_property_new (BonoboPEditor            *editor,
			 gchar                    *name, 
			 CORBA_any                *value,
			 int                        index,
			 BonoboEventSource         *es,
			 BonoboSubPropertyChangeFn  change_fn)
{
	BonoboSubProperty *prop;

	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (editor != NULL, NULL);
	g_return_val_if_fail (value != NULL, NULL);
	g_return_val_if_fail (es != NULL, NULL);
	g_return_val_if_fail (change_fn != NULL, NULL);

	prop = gtk_type_new (bonobo_sub_property_get_type ());

	bonobo_object_ref (BONOBO_OBJECT (editor));

	bonobo_object_ref (BONOBO_OBJECT (es));

	prop->priv->es = es;
	prop->priv->editor = editor;
	prop->priv->name = g_strdup (name);
	prop->priv->value =  bonobo_arg_copy (value);
	prop->priv->index =  index;
	prop->priv->change_fn = change_fn;

	return prop;
}

void
bonobo_sub_property_set_value (BonoboSubProperty *prop,
			       CORBA_any *value)
{
	char *mask;

	if (bonobo_arg_is_equal (prop->priv->value, value, NULL)) 
		return;

	mask = g_strdup_printf ("Bonobo/Property:change:%s", 
				prop->priv->name);

	if (prop->priv->value)
		CORBA_free (prop->priv->value);

	prop->priv->value = bonobo_arg_copy (value);

	bonobo_event_source_notify_listeners (prop->priv->es,
					      mask, prop->priv->value, NULL);
	g_free (mask);
}

