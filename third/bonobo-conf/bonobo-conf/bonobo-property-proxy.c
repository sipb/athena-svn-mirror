/*
 * bonobo-property-proxy.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include "bonobo-property-proxy.h"

typedef struct {
	BonoboPBProxy *pbp;
} ProxyCallbackData;

static CORBA_char *
impl_Bonobo_Property_getName (PortableServer_Servant servant,
			      CORBA_Environment *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;

	return CORBA_string_dup (ps->property_name);
}

static CORBA_TypeCode
impl_Bonobo_Property_getType (PortableServer_Servant servant,
			      CORBA_Environment *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;

	return bonobo_pbproxy_prop_type (ps->pbp, ps->property_name, ev);
}

static CORBA_any *
impl_Bonobo_Property_getValue (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;

	return bonobo_pbproxy_get_value (ps->pbp, ps->property_name, ev);
}

static void
impl_Bonobo_Property_setValue (PortableServer_Servant servant,
				const CORBA_any       *any,
				CORBA_Environment     *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;

	bonobo_pbproxy_set_value (ps->pbp, ps->property_name, any, ev);
}

static CORBA_any *
impl_Bonobo_Property_getDefault (PortableServer_Servant servant,
				 CORBA_Environment *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;

	return bonobo_pbproxy_get_default (ps->pbp, ps->property_name, ev);
}

static CORBA_char *
impl_Bonobo_Property_getDocString (PortableServer_Servant servant,
				   CORBA_Environment *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;
	
	return bonobo_pbproxy_get_doc_string (ps->pbp, ps->property_name, ev);
}

static CORBA_long
impl_Bonobo_Property_getFlags (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;

	return bonobo_pbproxy_get_flags (ps->pbp, ps->property_name, ev);
}

static Bonobo_EventSource_ListenerId
impl_Bonobo_Property_addListener (PortableServer_Servant servant,
				  const Bonobo_Listener  l,
				  CORBA_Environment     *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;
	Bonobo_Unknown corba_es = BONOBO_OBJREF (ps->pbp->es);
	Bonobo_EventSource_ListenerId id = 0;
	char *mask;

	mask = g_strdup_printf ("Bonobo/Property:change:%s", 
				ps->property_name);

	id = Bonobo_EventSource_addListenerWithMask (corba_es, l, mask, ev); 

	g_free (mask);

	return id;
}

static void
impl_Bonobo_Property_removeListener (PortableServer_Servant servant,
				     const Bonobo_EventSource_ListenerId id,
				     CORBA_Environment     *ev)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;
	Bonobo_Unknown corba_es = BONOBO_OBJREF (ps->pbp->es);

	Bonobo_EventSource_removeListener (corba_es, id, ev); 
	return;
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
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;

	if (!strcmp (repoid, "IDL:Bonobo/Property:1.0"))
		return bonobo_transient_create_objref (ps->transient,
			"IDL:Bonobo/Property:1.0", ps->property_name, ev);
	else
		return CORBA_OBJECT_NIL;
}

static POA_Bonobo_Unknown__epv *
bonobo_config_property_get_unknown_epv (void)
{
	POA_Bonobo_Unknown__epv *epv;

	epv = g_new0 (POA_Bonobo_Unknown__epv, 1);

	epv->ref            = impl_Bonobo_Property_ref;
	epv->unref          = impl_Bonobo_Property_unref;
	epv->queryInterface = impl_Bonobo_Property_queryInterface;

	return epv;
}

static POA_Bonobo_Property__epv *
bonobo_property_get_epv (void)
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

static POA_Bonobo_Property__vepv *
bonobo_property_get_vepv (void)
{
	static POA_Bonobo_Property__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_Bonobo_Property__vepv, 1);

	vepv->Bonobo_Unknown_epv = bonobo_config_property_get_unknown_epv ();
	vepv->Bonobo_Property_epv = bonobo_property_get_epv ();

	return vepv;
}

static PortableServer_Servant
bonobo_property_create_servant (PortableServer_POA  poa,
				BonoboTransient    *bt,
				char               *name,
				void               *callback_data)
{
        PropertyProxyServant *servant;
        CORBA_Environment     ev;
	ProxyCallbackData    *cd = (ProxyCallbackData *)callback_data;

        CORBA_exception_init (&ev);

        servant = g_new0 (PropertyProxyServant, 1);
        ((POA_Bonobo_Property *)servant)->vepv = bonobo_property_get_vepv ();
        servant->property_name = g_strdup (name);
	servant->pbp = cd->pbp;
	servant->transient = bt;
        POA_Bonobo_Property__init ((PortableServer_Servant) servant, &ev);

	return servant;
}

static void
bonobo_property_destroy_servant (PortableServer_Servant servant, 
				 void *callback_data)
{
	PropertyProxyServant *ps = (PropertyProxyServant *) servant;
        CORBA_Environment ev;

        g_free (ps->property_name);
        CORBA_exception_init (&ev);
        POA_Bonobo_Property__fini (servant, &ev);
        CORBA_exception_free (&ev);
	g_free (servant);
}

static void
transient_destroy_cb (GtkObject         *object,
		      ProxyCallbackData *cd)
{
	g_free (cd);
}

BonoboTransient *
bonobo_property_proxy_transient (BonoboPBProxy *pbp)
{
	ProxyCallbackData *cd;
	BonoboTransient *bt;

	g_return_val_if_fail (pbp != NULL, NULL);

	cd = g_new0 (ProxyCallbackData, 1);

	cd->pbp = pbp;

	bt = bonobo_transient_new (NULL, bonobo_property_create_servant, 
				   bonobo_property_destroy_servant, cd);
	
	gtk_signal_connect (GTK_OBJECT (bt), "destroy",
			    GTK_SIGNAL_FUNC (transient_destroy_cb), cd);

	return bt;
}
