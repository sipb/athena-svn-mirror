/**
 * bonobo-property-bag-proxy.c: a proxy for property bags
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property.h>
#include <bonobo/bonobo-arg.h>
#include <string.h>

#include "bonobo-property-proxy.h"
#include "bonobo-property-bag-proxy.h"

static GtkObjectClass *parent_class = NULL;

#define BONOBO_PBPROXY_FROM_SERVANT(servant) \
(BONOBO_PBPROXY (bonobo_object_from_servant (servant)))

typedef struct {
	char            *name;
	Bonobo_Property  property;
	Bonobo_Property  pp;         /* property proxy */
	CORBA_any       *value;
	CORBA_any       *new_value;
} PropertyData;

static PropertyData *lookup_property_data (BonoboPBProxy *proxy,
					   const char    *name);

enum {
	MODIFIED_SIGNAL,
	LAST_SIGNAL
};

static guint proxy_signals [LAST_SIGNAL];

static void
bonobo_pbproxy_destroy (GtkObject *object)
{
	BonoboPBProxy *proxy = BONOBO_PBPROXY (object);
	PropertyData  *pd;
	GList         *l;

	l = proxy->pl;

	while (l) {
		pd = (PropertyData *)l->data;
			
		if (pd->name)
			g_free (pd->name);

		if (pd->new_value)
			CORBA_free (pd->new_value);

		if (pd->value)
			CORBA_free (pd->value);

		if (pd->property != CORBA_OBJECT_NIL)
			bonobo_object_release_unref (pd->property, NULL);
		
		g_free (pd);

		l = l->next;
	}


	if (proxy->transient)
		gtk_object_unref (GTK_OBJECT (proxy->transient));

	if (proxy->lid && proxy->bag != CORBA_OBJECT_NIL)
		bonobo_event_source_client_remove_listener (proxy->bag,
							    proxy->lid, NULL);
	if (proxy->bag != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (proxy->bag, NULL);

	proxy->bag = CORBA_OBJECT_NIL;

	parent_class->destroy (object);
}


/* CORBA implementations */

static Bonobo_PropertyList *
impl_Bonobo_PropertyBag_getProperties (PortableServer_Servant  servant,
				       CORBA_Environment      *ev)
{
	BonoboPBProxy       *proxy = BONOBO_PBPROXY_FROM_SERVANT (servant);
	Bonobo_PropertyList *prop_list;
	GList		    *l;
	int		     len;
	int		     i;

	len = g_list_length (proxy->pl);

	prop_list = Bonobo_PropertyList__alloc ();
	prop_list->_length = len;
	
	if (len == 0)
		return prop_list;

	prop_list->_buffer = CORBA_sequence_Bonobo_Property_allocbuf (len);
	
	i = 0;
	for (l = proxy->pl; l != NULL; l = l->next) {
		PropertyData *pd = (PropertyData *)l->data;

		prop_list->_buffer [i] = CORBA_Object_duplicate (pd->pp, ev);
		
		i++;
	}

	return prop_list;
}

static Bonobo_Property
impl_Bonobo_PropertyBag_getPropertyByName (PortableServer_Servant  servant,
					   const CORBA_char       *name,
					   CORBA_Environment      *ev)
{
	BonoboPBProxy *proxy = BONOBO_PBPROXY_FROM_SERVANT (servant);
	PropertyData  *pd;
	Bonobo_Property prop = CORBA_OBJECT_NIL;

	if ((pd = lookup_property_data (proxy, name))) {
		if (pd->pp)
			return CORBA_Object_duplicate (pd->pp, ev);
		return CORBA_OBJECT_NIL;	
	}

	if (proxy->bag != CORBA_OBJECT_NIL) {

		prop = Bonobo_PropertyBag_getPropertyByName (proxy->bag, name,
							     ev);
		if (BONOBO_EX (ev) || prop == CORBA_OBJECT_NIL)
			return CORBA_OBJECT_NIL;
	}

	pd = g_new0 (PropertyData, 1);

	pd->property = prop;
	pd->name = g_strdup (name);
	pd->pp = bonobo_transient_create_objref (proxy->transient, 
						 "IDL:Bonobo/Property:1.0",
						 pd->name, ev);

	proxy->pl = g_list_prepend (proxy->pl, pd);

	return CORBA_Object_duplicate (pd->pp, ev);	
}

static Bonobo_PropertyNames *
impl_Bonobo_PropertyBag_getPropertyNames (PortableServer_Servant  servant,
					  CORBA_Environment      *ev)
{
	BonoboPBProxy        *proxy = BONOBO_PBPROXY_FROM_SERVANT (servant);
	Bonobo_PropertyNames *name_list;
	GList                *l;
	int                   len;
	int		      i;

	len = g_list_length (proxy->pl);
	
	name_list = Bonobo_PropertyNames__alloc ();
	name_list->_length = len;

	if (len == 0)
		return name_list;

	name_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	
	i = 0;
	for (l = proxy->pl; l != NULL; l = l->next) {
		PropertyData *pd = l->data;

		name_list->_buffer [i] = CORBA_string_dup (pd->name);
		i++;
	}

	return name_list;
}

static void                  
impl_Bonobo_PropertyBag_setValues (PortableServer_Servant servant,
				   const Bonobo_PropertySet *set,
				   CORBA_Environment *ev)
{
	BonoboPBProxy *proxy = BONOBO_PBPROXY_FROM_SERVANT (servant);
	int            i;

	for (i = 0; i < set->_length; i++) {
		bonobo_pbproxy_set_value (proxy, set->_buffer [i].name,
					  &set->_buffer [i].value, ev);
		if (BONOBO_EX (ev))
			return;
	}
}

static Bonobo_PropertySet *
impl_Bonobo_PropertyBag_getValues (PortableServer_Servant servant,
				    CORBA_Environment *ev)
{
	BonoboPBProxy *proxy = BONOBO_PBPROXY_FROM_SERVANT (servant);
	Bonobo_PropertySet *set;
	GList		   *l;
	int		    len;
	int		    i;

	len = g_list_length (proxy->pl);

	set = Bonobo_PropertySet__alloc ();
	set->_length = len;

	if (len == 0)
		return set;

	set->_buffer = CORBA_sequence_Bonobo_Pair_allocbuf (len);

	i = 0;
	for (l = proxy->pl; l != NULL; l = l->next) {
		PropertyData *pd = (PropertyData *)l->data;
		BonoboArg *arg;

		set->_buffer [i].name = CORBA_string_dup (pd->name);

		if (pd->new_value)
			arg = bonobo_arg_copy (pd->new_value);
		else if (pd->value)
			arg = bonobo_arg_copy (pd->value);
		else 
			arg = bonobo_arg_new (TC_null);

		set->_buffer [i].value = *arg;

		i++;		
	}

	return set;
}

static Bonobo_EventSource
impl_Bonobo_PropertyBag_getEventSource (PortableServer_Servant servant,
					CORBA_Environment      *ev)
{
	BonoboPBProxy *proxy = BONOBO_PBPROXY_FROM_SERVANT (servant);

	return bonobo_object_dup_ref (BONOBO_OBJREF (proxy->es), ev);
}

static void
value_changed_cb (BonoboListener    *listener,
		  char              *event_name, 
		  CORBA_any         *any,
		  CORBA_Environment *ev,
		  gpointer           user_data)
{
	BonoboPBProxy *proxy = BONOBO_PBPROXY (user_data);
	PropertyData *pd;
	char *name;

	if (!(name = bonobo_event_subtype (event_name)))
		return;

	if (!(pd = lookup_property_data (proxy, name)))
		return;

	if (pd->new_value) {
		CORBA_free (pd->new_value);
		pd->new_value = NULL;
	}

	if (pd->value)
		CORBA_free (pd->value);

	pd->value = bonobo_arg_copy (any);

	bonobo_event_source_notify_listeners_full (proxy->es,
						   "Bonobo/Property",
						   "change", name,
						   any, ev);
	g_free (name);
}

void
bonobo_pbproxy_set_bag (BonoboPBProxy     *proxy,
			Bonobo_PropertyBag bag)
{
	Bonobo_PropertyList *plist;
	CORBA_Environment   ev;
	PropertyData       *pd;
	GList              *l;
	int                 i;

	g_return_if_fail (proxy != NULL);

	if (proxy->lid && proxy->bag != CORBA_OBJECT_NIL)
		bonobo_event_source_client_remove_listener (proxy->bag,
							    proxy->lid, NULL);

	proxy->lid = 0;

	for (l = proxy->pl; l; l = l->next) {
		pd = (PropertyData *)l->data;

		if (pd->property != CORBA_OBJECT_NIL) {
			bonobo_object_release_unref (pd->property, NULL);
			pd->property = CORBA_OBJECT_NIL;
		}
	}

	if (proxy->bag != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (proxy->bag, NULL);

	proxy->bag = CORBA_OBJECT_NIL;

	CORBA_exception_init (&ev);

	if (bag != CORBA_OBJECT_NIL) {

		proxy->bag = bonobo_object_dup_ref (bag, NULL);
	
		proxy->lid = bonobo_event_source_client_add_listener 
			(bag, value_changed_cb, "Bonobo/Property:change:",
			 NULL, proxy);

		plist = Bonobo_PropertyBag_getProperties (bag, &ev);
      	
		if (BONOBO_EX (&ev) || !plist) {
			CORBA_exception_free (&ev);
			return;
		}
      	
		for (i = 0; i < plist->_length; i++) {
			char      *pn;
			CORBA_any *value;

			CORBA_exception_init (&ev);
		
			pn = Bonobo_Property_getName (plist->_buffer [i], &ev);
			if (BONOBO_EX (&ev))
				continue;

			value = Bonobo_Property_getValue (plist->_buffer [i], 
							  &ev);
			if (BONOBO_EX (&ev)) {
				CORBA_free (pn);
				continue;
			}

			bonobo_object_dup_ref (plist->_buffer [i], NULL);

			if (!(pd = lookup_property_data (proxy, pn))) {
				pd = g_new0 (PropertyData, 1);

				pd->property = plist->_buffer [i];
				pd->name = g_strdup (pn);
				pd->value = value;
				pd->pp =  bonobo_transient_create_objref 
					(proxy->transient, 
					 "IDL:Bonobo/Property:1.0", pd->name, 
					 &ev);
				proxy->pl = g_list_prepend (proxy->pl, pd);
			} else {
				pd->property = plist->_buffer [i];

				if (pd->value)
					CORBA_free (pd->value);

				pd->value = value;

				bonobo_event_source_notify_listeners_full
					(proxy->es, "Bonobo/Property", "change", pd->name, value, &ev);
			}

			CORBA_free (pn);
		}

		CORBA_free (plist);
	}

	CORBA_exception_free (&ev);
}

BonoboPBProxy *
bonobo_pbproxy_new ()
{
	BonoboPBProxy      *proxy;

	proxy = gtk_type_new (BONOBO_PBPROXY_TYPE);

	if (!(proxy->es = bonobo_event_source_new ())) {
		bonobo_object_unref (BONOBO_OBJECT (proxy));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (proxy), 
				     BONOBO_OBJECT (proxy->es));
	
	if (!(proxy->transient = bonobo_property_proxy_transient (proxy))) {
		bonobo_object_unref (BONOBO_OBJECT (proxy));
		return NULL;
	}

	return proxy;
}

static void
bonobo_pbproxy_class_init (BonoboPBProxyClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	POA_Bonobo_PropertyBag__epv *epv;
	
	parent_class = gtk_type_class (BONOBO_OBJECT_TYPE);

	object_class->destroy = bonobo_pbproxy_destroy;

	proxy_signals [MODIFIED_SIGNAL] =
                gtk_signal_new ("modified",
                                GTK_RUN_LAST,
                                object_class->type,
                                GTK_SIGNAL_OFFSET (BonoboPBProxyClass, 
						   modified),
                                gtk_marshal_NONE__NONE,
                                GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, proxy_signals, 
				      LAST_SIGNAL);
	epv = &class->epv;

	epv->getProperties        = impl_Bonobo_PropertyBag_getProperties;
	epv->getPropertyByName    = impl_Bonobo_PropertyBag_getPropertyByName;
	epv->getPropertyNames     = impl_Bonobo_PropertyBag_getPropertyNames;
	epv->getValues            = impl_Bonobo_PropertyBag_getValues;
	epv->setValues            = impl_Bonobo_PropertyBag_setValues;
	epv->getEventSource       = impl_Bonobo_PropertyBag_getEventSource;
}

static void
bonobo_pbproxy_init (BonoboPBProxy *cb)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboPBProxy, 
			 Bonobo_PropertyBag,
			 BONOBO_X_OBJECT_TYPE,
			 bonobo_pbproxy);


static PropertyData *
lookup_property_data (BonoboPBProxy     *proxy,
		      const char        *name)
{
	PropertyData *pd;
	GList *l;

	l = proxy->pl;

	while (l) {
		pd = (PropertyData *)l->data;
		
		if (!strcmp (pd->name, name))
			return pd;

		l = l->next;
	}

	return NULL;
}

CORBA_TypeCode    
bonobo_pbproxy_prop_type (BonoboPBProxy     *proxy,
			  const char        *name,
			  CORBA_Environment *ev)
{
	PropertyData *pd;

	if (!(pd = lookup_property_data (proxy, name)))
		return TC_null;

	if (!pd->value)
		return TC_null;

	return (CORBA_TypeCode)
		CORBA_Object_duplicate ((CORBA_Object)pd->value->_type, ev);
}

CORBA_any *
bonobo_pbproxy_get_value (BonoboPBProxy     *proxy,
			  const char        *name,
			  CORBA_Environment *ev)
{
	PropertyData *pd;

	if (!(pd = lookup_property_data (proxy, name))) {
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		return NULL;
	}

	if (pd->new_value)
		return bonobo_arg_copy (pd->new_value);

	if (pd->value)
		return bonobo_arg_copy (pd->value);

	return bonobo_arg_new (TC_null);
}

void              
bonobo_pbproxy_set_value (BonoboPBProxy     *proxy,
			  const char        *name,
			  const CORBA_any   *any,
			  CORBA_Environment *ev)
{
	PropertyData *pd;

	if (!(pd = lookup_property_data (proxy, name))) {
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		return;
	}

	if (!pd->new_value && pd->value && 
	    bonobo_arg_is_equal (pd->value, any, NULL))
		return;

	if (pd->new_value && bonobo_arg_is_equal (pd->new_value, any, NULL))
		return;

	if (pd->new_value)
		CORBA_free (pd->new_value);

	pd->new_value = bonobo_arg_copy (any);

	gtk_signal_emit (GTK_OBJECT (proxy), proxy_signals [MODIFIED_SIGNAL]);

	bonobo_event_source_notify_listeners_full (proxy->es,
						   "Bonobo/Property",
						   "change", name,
						   any, ev);
}

CORBA_any *
bonobo_pbproxy_get_default (BonoboPBProxy     *proxy,
			    const char        *name,
			    CORBA_Environment *ev)
{
	PropertyData *pd;
	
	if (!(pd = lookup_property_data (proxy, name)) || 
	    pd->property == CORBA_OBJECT_NIL) {
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		return NULL;
	}

	return Bonobo_Property_getDefault (pd->property, ev);
}

char *
bonobo_pbproxy_get_doc_string (BonoboPBProxy     *proxy,
			       const char        *name,
			       CORBA_Environment *ev)
{
	PropertyData *pd;

	if (!(pd = lookup_property_data (proxy, name))) { 
		bonobo_exception_set (ev, ex_Bonobo_PropertyBag_NotFound);
		return NULL;
	}

	if (pd->property == CORBA_OBJECT_NIL) 
		return CORBA_string_dup ("Proxy default documentation");

	return Bonobo_Property_getDocString (pd->property, ev);
}

CORBA_long        
bonobo_pbproxy_get_flags (BonoboPBProxy     *proxy,
			  const char        *name,
			  CORBA_Environment *ev)
{
	PropertyData *pd;

	if (!(pd = lookup_property_data (proxy, name)) || 
	    pd->property == CORBA_OBJECT_NIL) {
		return 0;
	}

	return Bonobo_Property_getFlags (pd->property, ev);
}

void
bonobo_pbproxy_update (BonoboPBProxy *proxy)
{
	GList *l;
	PropertyData *pd;
	CORBA_Environment ev;

	for (l = proxy->pl; l != NULL; l = l->next) {
		CORBA_exception_init (&ev);
	
		pd = (PropertyData *)l->data;
		
		if (pd->new_value && pd->property != CORBA_OBJECT_NIL)
			Bonobo_Property_setValue (pd->property, 
						  pd->new_value, &ev);

		CORBA_exception_free (&ev);
	}
}

void
bonobo_pbproxy_revert (BonoboPBProxy *proxy)
{
	/* fixme: */
	g_warning ("bonobo_pbproxy_revert not implemented");
}

