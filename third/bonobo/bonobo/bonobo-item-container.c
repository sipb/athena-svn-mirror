/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-item-container.h: a generic container for monikers.
 *
 * The BonoboItemContainer object represents a document which may have one
 * or more embedded document objects.  To embed an object in the
 * container, create a BonoboClientSite, add it to the container, and
 * then create an object supporting Bonobo::Embeddable and bind it to
 * the client site.  The BonoboItemContainer maintains a list of the client
 * sites which correspond to the objects embedded in the container.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Nat Friedman    (nat@nat.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkwidget.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-item-container.h>
#include <bonobo/bonobo-client-site.h>

enum {
	GET_OBJECT,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };

static BonoboObjectClass *bonobo_item_container_parent_class;

POA_Bonobo_ItemContainer__vepv bonobo_item_container_vepv;

static CORBA_Object
create_bonobo_item_container (BonoboObject *object)
{
	POA_Bonobo_ItemContainer *servant;
	CORBA_Environment ev;
	CORBA_Object o;

	CORBA_exception_init (&ev);

	servant = (POA_Bonobo_ItemContainer *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_item_container_vepv;

	POA_Bonobo_ItemContainer__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), servant, &ev));

	o = PortableServer_POA_servant_to_reference (
		bonobo_poa(), servant, &ev);

	if (o) {
		bonobo_object_bind_to_servant (object, servant);
		CORBA_exception_free (&ev);
		return o;
	} else {
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}
}

static void
bonobo_item_container_destroy (GtkObject *object)
{
	BonoboItemContainer *container = BONOBO_ITEM_CONTAINER (object);

	/*
	 * Destroy all the ClientSites.
	 */
	while (container->client_sites) {
		BonoboClientSite *client_site =
			BONOBO_CLIENT_SITE (container->client_sites->data);

		bonobo_object_unref (BONOBO_OBJECT (client_site));
	}
	
	GTK_OBJECT_CLASS (bonobo_item_container_parent_class)->destroy (object);
}

/*
 * Returns a list of the objects in this container
 */
static Bonobo_ItemContainer_ObjectList *
impl_Bonobo_ItemContainer_enumObjects (PortableServer_Servant servant,
				       CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboItemContainer *container = BONOBO_ITEM_CONTAINER (object);
	Bonobo_ItemContainer_ObjectList *return_list;
	int items;
	GList *l;
	int i;
	
	return_list = Bonobo_ItemContainer_ObjectList__alloc ();
	if (return_list == NULL)
		return NULL;

	items = g_list_length (container->client_sites);
	
	return_list->_buffer = CORBA_sequence_Bonobo_Unknown_allocbuf (items);
	if (return_list->_buffer == NULL){
		CORBA_free (return_list);
		return NULL;
	}
	
	return_list->_length = items;
	return_list->_maximum = items;

	/*
	 * Assemble the list of objects
	 */
	for (i = 0, l = container->client_sites; l; l = l->next, i++) {
		BonoboObjectClient *embeddable =
			bonobo_client_site_get_embeddable (l->data);

		return_list->_buffer [i] = CORBA_Object_duplicate (
			bonobo_object_corba_objref (BONOBO_OBJECT (embeddable)), ev);
	}

	return return_list;
}

static Bonobo_Unknown
impl_Bonobo_ItemContainer_getObjectByName (PortableServer_Servant servant,
					   const CORBA_char      *item_name,
					   CORBA_boolean          only_if_exists,
					   CORBA_Environment     *ev)
{
	Bonobo_Unknown ret;
	
	gtk_signal_emit (
		GTK_OBJECT (bonobo_object_from_servant (servant)),
		signals [GET_OBJECT], item_name, only_if_exists, ev, &ret);

	return ret;
}

/*
 * BonoboItemContainer CORBA vector-class initialization routine
 */

/**
 * bonobo_item_container_get_epv:
 *
 * Returns: The EPV for the default BonoboItemContainer implementation.  
 */
POA_Bonobo_ItemContainer__epv *
bonobo_item_container_get_epv (void)
{
	POA_Bonobo_ItemContainer__epv *epv;

	epv = g_new0 (POA_Bonobo_ItemContainer__epv, 1);

	epv->enumObjects     = impl_Bonobo_ItemContainer_enumObjects;
	epv->getObjectByName = impl_Bonobo_ItemContainer_getObjectByName;

	return epv;
}

static void
corba_container_class_init (void)
{
	/* Init the vepv */
	bonobo_item_container_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_item_container_vepv.Bonobo_ItemContainer_epv = bonobo_item_container_get_epv ();
}

typedef Bonobo_Unknown (*GnomeSignal_POINTER__POINTER_BOOL_POINTER) (
	BonoboItemContainer *item_container,
	CORBA_char *item_name,
	CORBA_boolean only_if_exists,
	CORBA_Environment *ev,
	gpointer func_data);

static void 
gnome_marshal_POINTER__POINTER_BOOL_POINTER (GtkObject * object,
					     GtkSignalFunc func,
					     gpointer func_data,
					     GtkArg * args)
{
	GnomeSignal_POINTER__POINTER_BOOL_POINTER rfunc;
	void **return_val;
	return_val = GTK_RETLOC_POINTER (args[3]);
	rfunc = (GnomeSignal_POINTER__POINTER_BOOL_POINTER) func;
	*return_val = (*rfunc) (BONOBO_ITEM_CONTAINER (object),
				GTK_VALUE_POINTER (args[0]),
				GTK_VALUE_BOOL (args[1]),
				GTK_VALUE_POINTER (args[2]),
				func_data);
}
/*
 * BonoboItemContainer class initialization routine
 */
static void
bonobo_item_container_class_init (BonoboItemContainerClass *container_class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) container_class;

	bonobo_item_container_parent_class = gtk_type_class (bonobo_object_get_type ());

	object_class->destroy = bonobo_item_container_destroy;

	signals [GET_OBJECT] =
		gtk_signal_new  (
			"get_object",
			GTK_RUN_LAST,
			object_class->type,
			0,
			gnome_marshal_POINTER__POINTER_BOOL_POINTER,
			GTK_TYPE_POINTER,
			3,
			GTK_TYPE_POINTER, GTK_TYPE_BOOL, GTK_TYPE_POINTER);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	corba_container_class_init ();
}

/*
 * BonoboItemContainer instance initialization routine
 */
static void
bonobo_item_container_init (BonoboItemContainer *container)
{
}

/**
 * bonobo_item_container_construct:
 * @container: The container object to construct
 * @corba_container: The CORBA object that implements Bonobo::ItemContainer
 *
 * Constructs the @container Gtk object using the provided CORBA
 * object.
 *
 * Returns: The constructed BonoboItemContainer object.
 */
BonoboItemContainer *
bonobo_item_container_construct (BonoboItemContainer  *container,
				 Bonobo_ItemContainer corba_container)
{
	g_return_val_if_fail (container != NULL, NULL);
	g_return_val_if_fail (corba_container != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (BONOBO_IS_ITEM_CONTAINER (container), NULL);
	
	bonobo_object_construct (BONOBO_OBJECT (container), (CORBA_Object) corba_container);

	return container;
}

/**
 * bonobo_item_container_get_type:
 *
 * Returns: The GtkType for the BonoboItemContainer class.
 */
GtkType
bonobo_item_container_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboItemContainer",
			sizeof (BonoboItemContainer),
			sizeof (BonoboItemContainerClass),
			(GtkClassInitFunc) bonobo_item_container_class_init,
			(GtkObjectInitFunc) bonobo_item_container_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_item_container_new:
 *
 * Creates a new BonoboItemContainer object.  These are used to hold
 * client sites.
 *
 * Returns: The newly created BonoboItemContainer object
 */
BonoboItemContainer *
bonobo_item_container_new (void)
{
	BonoboItemContainer *container;
	Bonobo_ItemContainer corba_container;

	container = gtk_type_new (bonobo_item_container_get_type ());
	corba_container = create_bonobo_item_container (BONOBO_OBJECT (container));

	if (corba_container == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (container));
		return NULL;
	}
	
	return bonobo_item_container_construct (container, corba_container);
}

/**
 * bonobo_item_container_get_moniker:
 * @container: A BonoboItemContainer object.
 */
BonoboMoniker *
bonobo_item_container_get_moniker (BonoboItemContainer *container)
{
	g_return_val_if_fail (container != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_ITEM_CONTAINER (container), NULL);

	return container->moniker;
}

static void
bonobo_item_container_client_site_destroy_cb (BonoboClientSite *client_site, gpointer data)
{
	BonoboItemContainer *container = BONOBO_ITEM_CONTAINER (data);

	/*
	 * Remove this client site from our list.
	 */
	container->client_sites = g_list_remove (container->client_sites, client_site);
}

/**
 * bonobo_item_container_add:
 * @container: The object to operate on.
 * @client_site: The client site to add to the container
 *
 * Adds the @client_site to the list of client-sites managed by this
 * container
 */
void
bonobo_item_container_add (BonoboItemContainer *container, BonoboObject *client_site)
{
	g_return_if_fail (container != NULL);
	g_return_if_fail (client_site != NULL);
	g_return_if_fail (BONOBO_IS_OBJECT (client_site));
	g_return_if_fail (BONOBO_IS_ITEM_CONTAINER (container));

	container->client_sites = g_list_prepend (container->client_sites, client_site);

	gtk_signal_connect (GTK_OBJECT (client_site), "destroy",
			    GTK_SIGNAL_FUNC (bonobo_item_container_client_site_destroy_cb), container);
}

/**
 * bonobo_item_container_remove:
 * @container: The object to operate on.
 * @client_site: The client site to remove from the container
 *
 * Removes the @client_site from the @container
 */
void
bonobo_item_container_remove (BonoboItemContainer *container, BonoboObject *client_site)
{
	g_return_if_fail (container != NULL);
	g_return_if_fail (client_site != NULL);
	g_return_if_fail (BONOBO_IS_OBJECT (client_site));
	g_return_if_fail (BONOBO_IS_ITEM_CONTAINER (container));

	container->client_sites = g_list_remove (container->client_sites, client_site);
}

