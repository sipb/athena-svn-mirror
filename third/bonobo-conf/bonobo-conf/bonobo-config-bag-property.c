/*
 * bonobo-config-bag-property.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-exception.h>

#include "bonobo-config-bag-property.h"

typedef struct {
	POA_Bonobo_Property   prop;
	char		     *property_name;
	BonoboConfigBag      *cb;
} ConfigBagPropertyServant;

static CORBA_char *
impl_Bonobo_Property_getName (PortableServer_Servant  servant,
			      CORBA_Environment      *ev)
{
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant*) servant;

	return CORBA_string_dup (ps->property_name);
}

static CORBA_TypeCode
impl_Bonobo_Property_getType (PortableServer_Servant  servant,
			      CORBA_Environment      *ev)
{
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant *) servant;
	CORBA_any                *value;
	CORBA_TypeCode            type;
	char                     *path;
	
	path = g_strconcat (ps->cb->path, "/", ps->property_name, NULL);

	value = Bonobo_ConfigDatabase_getValue (ps->cb->db, path, 
						ps->cb->locale, ev);

	g_free (path);

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
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant *) servant;
	CORBA_any                *value;
	gchar                    *path;
	
	path = g_strconcat (ps->cb->path, "/", ps->property_name, NULL);

	value = Bonobo_ConfigDatabase_getValue (ps->cb->db, path, 
						ps->cb->locale, ev);

	g_free (path);
		
	return value;
}

static void
impl_Bonobo_Property_setValue (PortableServer_Servant  servant,
				const CORBA_any       *any,
				CORBA_Environment     *ev)
{
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant *) servant;
	gchar                    *path;

	path = g_strconcat (ps->cb->path, "/", ps->property_name, NULL);

	Bonobo_ConfigDatabase_setValue (ps->cb->db, path, any, ev);

	g_free (path);
}

static CORBA_any *
impl_Bonobo_Property_getDefault (PortableServer_Servant  servant,
				 CORBA_Environment      *ev)
{
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant *) servant;
	CORBA_any                *value;
	gchar                    *path;
	
	path = g_strconcat (ps->cb->path, "/", ps->property_name, NULL);

	value = Bonobo_ConfigDatabase_getDefault (ps->cb->db, path, 
						  ps->cb->locale, ev);

	g_free (path);
		
	return value;
}

static CORBA_char *
impl_Bonobo_Property_getDocString (PortableServer_Servant  servant,
				   CORBA_Environment      *ev)
{
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant *) servant;
	CORBA_any                *value;
	gchar                    *doc = NULL;
	gchar                    *path;
	
	path = g_strconcat ("/doc", ps->cb->path, "/", ps->property_name, 
			    NULL);

	value = Bonobo_ConfigDatabase_getValue (ps->cb->db, path, 
						ps->cb->locale, ev);

	g_free (path);

	if (BONOBO_EX (ev) || !value)
		return NULL;

	if (CORBA_TypeCode_equal (value->_type, TC_string, NULL))
		doc = CORBA_string_dup (*(char **)value->_value);

	CORBA_free (value);

	return doc;
}

static CORBA_long
impl_Bonobo_Property_getFlags (PortableServer_Servant  servant,
			       CORBA_Environment      *ev)
{
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant *) servant;
	CORBA_long                flags = BONOBO_PROPERTY_READABLE;
	CORBA_boolean             writeable;

	writeable = Bonobo_ConfigDatabase__get_writeable (ps->cb->db, ev);
	if (BONOBO_EX (ev))
		return 0;

	if (writeable)
		flags |= BONOBO_PROPERTY_WRITEABLE;

	return flags;
}

static Bonobo_EventSource_ListenerId
impl_Bonobo_Property_addListener (PortableServer_Servant  servant,
				  const Bonobo_Listener   l,
				  CORBA_Environment      *ev)
{
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant *) servant;
	Bonobo_Unknown corba_es = BONOBO_OBJREF (ps->cb->es);
	Bonobo_EventSource_ListenerId id = 0;
	char *mask;

	mask = g_strdup_printf ("=Bonobo/Property:change:%s", 
				ps->property_name);

	id = Bonobo_EventSource_addListenerWithMask (corba_es, l, mask, ev); 

	g_free (mask);

	return id;
}

static void
impl_Bonobo_Property_removeListener (PortableServer_Servant  servant,
				     const Bonobo_EventSource_ListenerId id,
				     CORBA_Environment      *ev)
{
	ConfigBagPropertyServant *ps = (ConfigBagPropertyServant *) servant;
	Bonobo_Unknown corba_es;

	corba_es = BONOBO_OBJREF (ps->cb->es);

	Bonobo_EventSource_removeListener (corba_es, id, ev); 
	return;
}

static POA_Bonobo_Property__epv *
bonobo_config_bag_property_get_epv (void)
{
	static POA_Bonobo_Property__epv *epv = NULL;

	if (epv != NULL)
		return epv;

	epv = g_new0 (POA_Bonobo_Property__epv, 1);

	epv->getName        = impl_Bonobo_Property_getName;
	epv->getType        = impl_Bonobo_Property_getType;
	epv->getValue       = impl_Bonobo_Property_getValue;
	epv->setValue       = impl_Bonobo_Property_setValue;
	epv->getDefault     = impl_Bonobo_Property_getDefault;
	epv->getDocString   = impl_Bonobo_Property_getDocString;
	epv->getFlags       = impl_Bonobo_Property_getFlags;
	epv->addListener    = impl_Bonobo_Property_addListener;
	epv->removeListener = impl_Bonobo_Property_removeListener;

	return epv;
}

static void
impl_Bonobo_Property_ref (PortableServer_Servant servant, 
			  CORBA_Environment *ev)
{
	/* nothing to do */
}

static void
impl_Bonobo_Property_unref (PortableServer_Servant servant, 
			    CORBA_Environment *ev)
{
	/* nothing to do */
}

static CORBA_Object
impl_Bonobo_Property_queryInterface (PortableServer_Servant  servant,
				     const CORBA_char       *repoid,
				     CORBA_Environment      *ev)
{
	ConfigBagPropertyServant *cs = (ConfigBagPropertyServant *) servant;

	if (!strcmp (repoid, "IDL:Bonobo/Property:1.0"))
		return bonobo_transient_create_objref (cs->cb->transient,
			"IDL:Bonobo/Property:1.0", cs->property_name, 
			ev);
	else
		return CORBA_OBJECT_NIL;
}

static POA_Bonobo_Unknown__epv *
bonobo_config_bag_property_get_unknown_epv (void)
{
	POA_Bonobo_Unknown__epv *epv;

	epv = g_new0 (POA_Bonobo_Unknown__epv, 1);

	epv->ref            = impl_Bonobo_Property_ref;
	epv->unref          = impl_Bonobo_Property_unref;
	epv->queryInterface = impl_Bonobo_Property_queryInterface;

	return epv;
}

static POA_Bonobo_Property__vepv *
bonobo_config_bag_property_get_vepv (void)
{
	static POA_Bonobo_Property__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_Bonobo_Property__vepv, 1);
	vepv->Bonobo_Unknown_epv = bonobo_config_bag_property_get_unknown_epv ();
	vepv->Bonobo_Property_epv = bonobo_config_bag_property_get_epv ();

	return vepv;
}

static PortableServer_Servant
bonobo_config_bag_property_create_servant (PortableServer_POA  poa,
				    BonoboTransient    *bt,
				    char               *name,
				    void               *callback_data)
{
        ConfigBagPropertyServant *servant;
        CORBA_Environment        ev;

        CORBA_exception_init (&ev);

        servant = g_new0 (ConfigBagPropertyServant, 1);
        ((POA_Bonobo_Property *)servant)->vepv = 
		bonobo_config_bag_property_get_vepv ();
        servant->property_name = g_strdup (name);
	servant->cb = BONOBO_CONFIG_BAG (callback_data);
        POA_Bonobo_Property__init ((PortableServer_Servant) servant, &ev);

        CORBA_exception_free (&ev);

	return servant;
}

static void
bonobo_config_bag_property_destroy_servant (PortableServer_Servant servant, 
				     void *callback_data)
{
        CORBA_Environment ev;
	
        g_free (((ConfigBagPropertyServant *)servant)->property_name);
        CORBA_exception_init (&ev);
        POA_Bonobo_Property__fini (servant, &ev);
        CORBA_exception_free (&ev);
	g_free (servant);
}

BonoboTransient *
bonobo_config_bag_property_transient (BonoboConfigBag *cb)
{
	return bonobo_transient_new (NULL, bonobo_config_bag_property_create_servant, 
				     bonobo_config_bag_property_destroy_servant, cb);
}
