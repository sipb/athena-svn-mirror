/**
 * bonobo-config-bag.c: config bag object implementation.
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <string.h>

#include "bonobo-config-property.h"

#define PARENT_TYPE (BONOBO_X_OBJECT_TYPE)

static GtkObjectClass        *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_BAG_CLASS (GTK_OBJECT(o)->klass)

#define CONFIG_PROPERTY_FROM_SERVANT(servant) BONOBO_CONFIG_PROPERTY (bonobo_object_from_servant (servant))

struct _BonoboConfigPropertyPrivate {
	gchar                 *name;
	gchar                 *locale;
	Bonobo_ConfigDatabase  db;
	Bonobo_EventSource     es;
};

static void
bonobo_config_property_destroy (GtkObject *object)
{
	BonoboConfigProperty *cp = BONOBO_CONFIG_PROPERTY (object);
	
	if (cp->priv->name)
		g_free (cp->priv->name);

	if (cp->priv->es != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (cp->priv->es, NULL);

	if (cp->priv->db != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (cp->priv->db, NULL);

	g_free (cp->priv);

	parent_class->destroy (object);
}


/* CORBA implementations */

static CORBA_char *
impl_Bonobo_Property_getName (PortableServer_Servant servant,
			      CORBA_Environment *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);

	return CORBA_string_dup (cp->priv->name);
}

static CORBA_TypeCode
impl_Bonobo_Property_getType (PortableServer_Servant  servant,
			      CORBA_Environment      *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);
	CORBA_any            *value;
	CORBA_TypeCode        type;

	value = Bonobo_ConfigDatabase_getValue (cp->priv->db, cp->priv->name, 
						cp->priv->locale, ev);

	if (BONOBO_EX (ev) || !value)
		return TC_null;
 	
	type = (CORBA_TypeCode) CORBA_Object_duplicate 
		((CORBA_Object) value->_type, ev);

	CORBA_free (value);
	
	return type;
}

static CORBA_any *
impl_Bonobo_Property_getValue (PortableServer_Servant  servant,
			       CORBA_Environment      *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);

	return Bonobo_ConfigDatabase_getValue (cp->priv->db, cp->priv->name,
					       cp->priv->locale, ev);
}

static void
impl_Bonobo_Property_setValue (PortableServer_Servant  servant,
			       const CORBA_any        *any,
			       CORBA_Environment      *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);

	Bonobo_ConfigDatabase_setValue (cp->priv->db, cp->priv->name, any, ev);
}

static CORBA_any *
impl_Bonobo_Property_getDefault (PortableServer_Servant servant,
				 CORBA_Environment *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);

	return Bonobo_ConfigDatabase_getDefault (cp->priv->db, cp->priv->name,
						 cp->priv->locale, ev);
}

static CORBA_char *
impl_Bonobo_Property_getDocString (PortableServer_Servant servant,
				   CORBA_Environment *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);
	CORBA_any            *value;
	gchar                *doc = NULL;
	gchar                *path;
	
	path = g_strconcat ("/doc", cp->priv->name, NULL);

	value = Bonobo_ConfigDatabase_getValue (cp->priv->db, path, 
						cp->priv->locale, ev);

	g_free (path);

	if (BONOBO_EX (ev) || !value)
		return NULL;

	if (CORBA_TypeCode_equal (value->_type, TC_string, NULL))
		doc = CORBA_string_dup (*(char **)value->_value);

	CORBA_free (value);

	return doc;
}

static CORBA_long
impl_Bonobo_Property_getFlags (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);
	CORBA_long flags = BONOBO_PROPERTY_READABLE;
	CORBA_boolean writeable;

	writeable = Bonobo_ConfigDatabase__get_writeable (cp->priv->db, ev);
	if (BONOBO_EX (ev))
		return 0;

	if (writeable)
		flags |= BONOBO_PROPERTY_WRITEABLE;

	return flags;
}


static Bonobo_EventSource_ListenerId
impl_Bonobo_Property_addListener (PortableServer_Servant servant,
				  const Bonobo_Listener  l,
				  CORBA_Environment     *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);
	Bonobo_EventSource_ListenerId id = 0;
	char *mask;

	mask = g_strdup_printf ("=Bonobo/Property:change:%s", 
				cp->priv->name);

	if (cp->priv->es != CORBA_OBJECT_NIL)
		id = Bonobo_EventSource_addListenerWithMask (cp->priv->es, l,
							     mask, ev); 

	g_free (mask);

	return id;
}

static void
impl_Bonobo_Property_removeListener (PortableServer_Servant servant,
				     const Bonobo_EventSource_ListenerId id,
				     CORBA_Environment     *ev)
{
	BonoboConfigProperty *cp = CONFIG_PROPERTY_FROM_SERVANT (servant);

	Bonobo_EventSource_removeListener (cp->priv->es, id, ev); 
	return;
}

BonoboConfigProperty *
bonobo_config_property_new (Bonobo_ConfigDatabase db,
			    const gchar *path)
{
	CORBA_Environment ev;
	BonoboConfigProperty *cp;

	g_return_val_if_fail (db != NULL, NULL);
	g_return_val_if_fail (path != NULL, NULL);

	cp = gtk_type_new (BONOBO_CONFIG_PROPERTY_TYPE);
	
	if (path[0] == '/')
		cp->priv->name = g_strdup (path);
	else
		cp->priv->name = g_strconcat ("/", path, NULL);

	cp->priv->db = bonobo_object_dup_ref (db, NULL);

	if (!(cp->priv->locale = g_getenv ("LANG")))
		cp->priv->locale = "";

	CORBA_exception_init(&ev);

	cp->priv->es = Bonobo_Unknown_queryInterface 
		(db, "IDL:Bonobo/EventSource:1.0", &ev);

	CORBA_exception_free (&ev);

	return cp;
}

static void
bonobo_config_property_class_init (BonoboConfigPropertyClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	POA_Bonobo_Property__epv *epv;
	
	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_config_property_destroy;

	epv = &class->epv;

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
bonobo_config_property_init (BonoboConfigProperty *cp)
{
	cp->priv = g_new0 (BonoboConfigPropertyPrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboConfigProperty, 
			 Bonobo_Property,
			 PARENT_TYPE,
			 bonobo_config_property);

