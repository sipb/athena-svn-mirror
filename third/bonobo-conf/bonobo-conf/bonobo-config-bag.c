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
#include <bonobo/bonobo-property.h>
#include <bonobo/bonobo-persist-stream.h>
#include <bonobo/bonobo-transient.h>
#include <string.h>

#include "bonobo-config-bag.h"
#include "bonobo-config-bag-property.h"

#define PARENT_TYPE (BONOBO_X_OBJECT_TYPE)

#define GET_BAG_FROM_SERVANT(servant) BONOBO_CONFIG_BAG (bonobo_object_from_servant (servant))

static GtkObjectClass        *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_BAG_CLASS (GTK_OBJECT(o)->klass)

static Bonobo_PropertyList *
bonono_config_bag_get_properties (BonoboConfigBag   *cb,
				  CORBA_Environment *ev)
{
	Bonobo_PropertyList *plist;
	Bonobo_KeyList *key_list; 
	gchar *pname;
	CORBA_Object obj;
	int i;

	key_list = Bonobo_ConfigDatabase_listKeys (cb->db, cb->path, ev);
	if (BONOBO_EX (ev) || !key_list)
		return NULL;

	plist = Bonobo_PropertyList__alloc ();

	if (key_list->_length == 0)
		return plist;

	plist->_buffer = 
		CORBA_sequence_Bonobo_Property_allocbuf (key_list->_length);
	CORBA_sequence_set_release (plist, TRUE); 

	for (i = 0; i < key_list->_length; i++) {

		pname = key_list->_buffer [i];

		obj = bonobo_transient_create_objref (cb->transient, 
		        "IDL:Bonobo/Property:1.0", pname, ev);
		
		if (BONOBO_EX (ev) || !obj)
			break;
	       
		plist->_buffer [plist->_length++] = obj;
	}

	/* fixme: */
	/* CORBA_free (key_list); */

	return plist;
}

static Bonobo_Property
bonono_config_bag_get_property_by_name (BonoboConfigBag   *cb,
					const CORBA_char  *name,
					CORBA_Environment *ev)
{
	/* we do not allow a slash in the name */
	if (strchr (name, '/')) {

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_PropertyBag_NotFound, NULL);
		
		return CORBA_OBJECT_NIL;
	}

	/* we always return a reference - even if the value does not exist,
	 * so that we can add values. 
	 */
	return bonobo_transient_create_objref (cb->transient, 
					       "IDL:Bonobo/Property:1.0",
					       name, ev); 
}

static Bonobo_PropertyNames *
bonono_config_bag_get_property_names (BonoboConfigBag   *cb,
				      CORBA_Environment *ev)
{
	return Bonobo_ConfigDatabase_listKeys (cb->db, cb->path, ev);
}

static void
bonono_config_bag_set_values (BonoboConfigBag *cb,
			      const Bonobo_PropertySet * set,
			      CORBA_Environment * ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
			     ex_Bonobo_PropertyBag_NotFound, NULL);

	g_warning ("bonono_config_bag_set_values not implemented");
}

static Bonobo_PropertySet *
bonono_config_bag_get_values (BonoboConfigBag *cb,
			      CORBA_Environment * ev)
{
	Bonobo_PropertySet *set;

	g_warning ("bonono_config_bag_get_values not implemented");

	set = Bonobo_PropertySet__alloc ();
	set->_length = 0;

	return set;
}

static void
bonobo_config_bag_destroy (GtkObject *object)
{
	BonoboConfigBag *cb = BONOBO_CONFIG_BAG (object);
	CORBA_Environment ev;

	if (cb->listener_id != 0) {
		CORBA_exception_init (&ev);
		bonobo_event_source_client_remove_listener (cb->db, cb->listener_id, &ev);

		if (BONOBO_EX (&ev))
			g_critical ("Could not remove listener (%s)", bonobo_exception_get_text (&ev));

		CORBA_exception_free (&ev);
	}
	
	if (cb->transient)
		gtk_object_unref (GTK_OBJECT (cb->transient));
	
	if (cb->db) 
		bonobo_object_release_unref (cb->db, NULL);

	g_free (cb->path);

	parent_class->destroy (object);
}


/* CORBA implementations */

static Bonobo_PropertyList *
impl_Bonobo_PropertyBag_getProperties (PortableServer_Servant  servant,
				       CORBA_Environment      *ev)
{
	BonoboConfigBag *cb = GET_BAG_FROM_SERVANT (servant);

	return CLASS (cb)->get_properties (cb, ev);
}

static Bonobo_Property
impl_Bonobo_PropertyBag_getPropertyByName (PortableServer_Servant  servant,
					   const CORBA_char       *name,
					   CORBA_Environment      *ev)
{
	BonoboConfigBag *cb = GET_BAG_FROM_SERVANT (servant);
       
	return CLASS (cb)->get_property_by_name (cb, name, ev);
}

static Bonobo_PropertyNames *
impl_Bonobo_PropertyBag_getPropertyNames (PortableServer_Servant  servant,
					  CORBA_Environment      *ev)
{
	BonoboConfigBag *cb = GET_BAG_FROM_SERVANT (servant);

	return CLASS (cb)->get_property_names (cb, ev);
}

static void                  
impl_Bonobo_PropertyBag_setValues (PortableServer_Servant    servant,
				   const Bonobo_PropertySet *set,
				   CORBA_Environment        *ev)
{
	BonoboConfigBag *cb =  GET_BAG_FROM_SERVANT (servant);

	CLASS (cb)->set_values (cb, set, ev);
}

static Bonobo_PropertySet *
impl_Bonobo_PropertyBag_getValues (PortableServer_Servant  servant,
				    CORBA_Environment     *ev)
{
	BonoboConfigBag *cb = GET_BAG_FROM_SERVANT (servant);

	return CLASS (cb)->get_values (cb, ev);
}

static Bonobo_EventSource
impl_Bonobo_PropertyBag_getEventSource (PortableServer_Servant  servant,
					CORBA_Environment      *ev)
{
	BonoboConfigBag *cb = GET_BAG_FROM_SERVANT (servant);

	return bonobo_object_dup_ref (BONOBO_OBJREF (cb->es), ev);
}

void
notify_cb (BonoboListener    *listener,
	   char              *event_name, 
	   CORBA_any         *any,
	   CORBA_Environment *ev,
	   gpointer           user_data)
{
	BonoboConfigBag *cb = BONOBO_CONFIG_BAG (user_data);
	char *tmp, *ename;

	tmp = bonobo_event_subtype (event_name);
	ename = g_strconcat ("Bonobo/Property:change:", tmp, NULL); 
	g_free (tmp);

	bonobo_event_source_notify_listeners (cb->es, ename, any, NULL);

	g_free (ename);
}

BonoboConfigBag *
bonobo_config_bag_new (Bonobo_ConfigDatabase db,
		       const gchar *path)
{
	BonoboConfigBag *cb;
	char *m;
	int l;

	g_return_val_if_fail (db != NULL, NULL);
	g_return_val_if_fail (path != NULL, NULL);

	cb = gtk_type_new (BONOBO_CONFIG_BAG_TYPE);
	
	if (path[0] == '/')
		cb->path = g_strdup (path);
	else
		cb->path = g_strconcat ("/", path, NULL);

	cb->db = bonobo_object_dup_ref (db, NULL);

	while ((l = strlen (cb->path)) > 1 && cb->path [l - 1] == '/') 
		cb->path [l] = '\0';
	
	if (!(cb->transient = bonobo_config_bag_property_transient (cb))) {
		bonobo_object_unref (BONOBO_OBJECT (cb));
		return NULL;
	}

	if (!(cb->locale = g_getenv ("LANG")))
		cb->locale = "";

	cb->es = bonobo_event_source_new ();

	bonobo_object_add_interface (BONOBO_OBJECT (cb), 
				     BONOBO_OBJECT (cb->es));

	m = g_strconcat ("Bonobo/ConfigDatabase:change", cb->path, ":", NULL);

	cb->listener_id = bonobo_event_source_client_add_listener (db, notify_cb, m, NULL, cb);

	g_free (m);

	return cb;
}

static void
bonobo_config_bag_class_init (BonoboConfigBagClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	POA_Bonobo_PropertyBag__epv *epv;
	
	parent_class = gtk_type_class (PARENT_TYPE);

	class->get_properties = bonono_config_bag_get_properties;
	class->get_property_by_name = bonono_config_bag_get_property_by_name;
	class->get_property_names = bonono_config_bag_get_property_names;
	class->set_values = bonono_config_bag_set_values;
	class->get_values = bonono_config_bag_get_values;

	object_class->destroy = bonobo_config_bag_destroy;

	epv = &class->epv;

	epv->getProperties        = impl_Bonobo_PropertyBag_getProperties;
	epv->getPropertyByName    = impl_Bonobo_PropertyBag_getPropertyByName;
	epv->getPropertyNames     = impl_Bonobo_PropertyBag_getPropertyNames;
	epv->getValues            = impl_Bonobo_PropertyBag_getValues;
	epv->setValues            = impl_Bonobo_PropertyBag_setValues;
	epv->getEventSource       = impl_Bonobo_PropertyBag_getEventSource;
}

static void
bonobo_config_bag_init (BonoboConfigBag *cb)
{
	cb->listener_id = 0;
}

BONOBO_X_TYPE_FUNC_FULL (BonoboConfigBag, 
			 Bonobo_PropertyBag,
			 PARENT_TYPE,
			 bonobo_config_bag);

