/*
 * bonobo-ui-container.c: The server side CORBA impl. for BonoboWindow.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include "config.h"
#include <gnome.h>
#include <bonobo.h>
#include <liboaf/liboaf.h>

#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-win.h>
#include <bonobo/bonobo-ui-container.h>

POA_Bonobo_UIContainer__vepv bonobo_ui_container_vepv;

#define WIN_DESTROYED 0x1

static BonoboWindow *
bonobo_ui_container_from_servant (PortableServer_Servant servant)
{
	BonoboUIContainer *container;

	container = BONOBO_UI_CONTAINER (bonobo_object_from_servant (servant));
	g_return_val_if_fail (container != NULL, NULL);

	if (container->win == NULL) {
		if (!container->flags & WIN_DESTROYED)
			g_warning ("Trying to invoke CORBA method "
				   "on unbound UIContainer");
		return NULL;
	} else
		return container->win;
}

static void
impl_Bonobo_UIContainer_registerComponent (PortableServer_Servant   servant,
					   const CORBA_char        *component_name,
					   const Bonobo_Unknown     object,
					   CORBA_Environment       *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);

	bonobo_window_register_component (win, component_name, object);
}

static void
impl_Bonobo_UIContainer_deregisterComponent (PortableServer_Servant servant,
					     const CORBA_char      *component_name,
					     CORBA_Environment     *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);

	if (!win)
		return;

	bonobo_window_deregister_component (win, component_name);
}

static void
impl_Bonobo_UIContainer_setNode (PortableServer_Servant   servant,
				 const CORBA_char        *path,
				 const CORBA_char        *xml,
				 const CORBA_char        *component_name,
				 CORBA_Environment       *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);
	BonoboUIXmlError err;

	err = bonobo_window_xml_merge (win, path, xml, component_name);

	if (err) {
		if (err == BONOBO_UI_XML_INVALID_PATH)
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_UIContainer_InvalidPath, NULL);
		else
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_UIContainer_MalFormedXML, NULL);
	}
}

static CORBA_char *
impl_Bonobo_UIContainer_getNode (PortableServer_Servant servant,
				 const CORBA_char      *path,
				 const CORBA_boolean    nodeOnly,
				 CORBA_Environment     *ev)
{
	BonoboWindow  *win = bonobo_ui_container_from_servant (servant);
	CORBA_char *xml;

	xml = bonobo_window_xml_get (win, path, nodeOnly);
	if (!xml) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_UIContainer_InvalidPath, NULL);
		return NULL;
	}

	return xml;
}

static void
impl_Bonobo_UIContainer_removeNode (PortableServer_Servant servant,
				    const CORBA_char      *path,
				    const CORBA_char      *component_name,
				    CORBA_Environment     *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);
	BonoboUIXmlError err;

	if (!win)
		return;

/*	g_warning ("Node remove '%s' for '%s'", path, component_name);*/

	err = bonobo_window_xml_rm (win, path, component_name);

	if (err)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_UIContainer_InvalidPath, NULL);
}

static CORBA_boolean
impl_Bonobo_UIContainer_exists (PortableServer_Servant servant,
				const CORBA_char      *path,
				CORBA_Environment     *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);

	return bonobo_window_xml_node_exists (win, path);
}

static void
impl_Bonobo_UIContainer_setObject (PortableServer_Servant servant,
				   const CORBA_char      *path,
				   const Bonobo_Unknown   control,
				   CORBA_Environment     *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);
	BonoboUIXmlError err;

	err = bonobo_window_object_set (win, path, control, ev);

	if (err)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_UIContainer_InvalidPath, NULL);
}

static Bonobo_Unknown
impl_Bonobo_UIContainer_getObject (PortableServer_Servant servant,
				   const CORBA_char      *path,
				   CORBA_Environment     *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);
	BonoboUIXmlError err;
	Bonobo_Unknown object;

	err = bonobo_window_object_get (win, path, &object, ev);

	if (err)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_UIContainer_InvalidPath, NULL);

	return object;
}

static void
impl_Bonobo_UIContainer_freeze (PortableServer_Servant   servant,
	     CORBA_Environment       *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);

	bonobo_window_freeze (win);
}

static void
impl_Bonobo_UIContainer_thaw (PortableServer_Servant   servant,
	     CORBA_Environment       *ev)
{
	BonoboWindow *win = bonobo_ui_container_from_servant (servant);

	bonobo_window_thaw (win);
}

static void
bonobo_ui_container_destroy (GtkObject *object)
{
	BonoboUIContainer *container = (BonoboUIContainer *) object;

	if (container->win)
		gtk_signal_disconnect_by_data (
			GTK_OBJECT (container->win), container);
}

/**
 * bonobo_ui_container_get_epv:
 *
 * Returns: The EPV for the default BonoboUIContainer implementation.  
 */
POA_Bonobo_UIContainer__epv *
bonobo_ui_container_get_epv (void)
{
	POA_Bonobo_UIContainer__epv *epv;

	epv = g_new0 (POA_Bonobo_UIContainer__epv, 1);

	epv->registerComponent   = impl_Bonobo_UIContainer_registerComponent;
	epv->deregisterComponent = impl_Bonobo_UIContainer_deregisterComponent;

	epv->setNode    = impl_Bonobo_UIContainer_setNode;
	epv->getNode    = impl_Bonobo_UIContainer_getNode;
	epv->removeNode = impl_Bonobo_UIContainer_removeNode;
	epv->exists     = impl_Bonobo_UIContainer_exists;

	epv->setObject  = impl_Bonobo_UIContainer_setObject;
	epv->getObject  = impl_Bonobo_UIContainer_getObject;

	epv->freeze     = impl_Bonobo_UIContainer_freeze;
	epv->thaw       = impl_Bonobo_UIContainer_thaw;

	return epv;
}

static void
bonobo_ui_container_class_init (GtkObjectClass *klass)
{
	bonobo_ui_container_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_ui_container_vepv.Bonobo_UIContainer_epv = bonobo_ui_container_get_epv ();

	klass->destroy = bonobo_ui_container_destroy;
}

/**
 * bonobo_ui_container_get_type:
 *
 * Returns: The GtkType for the BonoboUIContainer class.
 */
GtkType
bonobo_ui_container_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboUIContainer",
			sizeof (BonoboUIContainer),
			sizeof (BonoboUIContainerClass),
			(GtkClassInitFunc) bonobo_ui_container_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_ui_container_corba_object_create:
 * @object: The GtkObject that will wrap the CORBA object.
 *
 * Creates an activates the CORBA object that is wrcontainered
 * by the BonoboObject @object.
 *
 * Returns: An activated object reference to the created object or
 * %CORBA_OBJECT_NIL in case of failure.
 */
Bonobo_UIContainer
bonobo_ui_container_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_UIContainer *servant;
	CORBA_Environment ev;
	
	servant = (POA_Bonobo_UIContainer *)g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_ui_container_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_UIContainer__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}
	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

BonoboUIContainer *
bonobo_ui_container_construct (BonoboUIContainer  *container,
			       Bonobo_UIContainer  corba_container)
{
	g_return_val_if_fail (container != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_UI_CONTAINER (container), NULL);
	g_return_val_if_fail (corba_container != CORBA_OBJECT_NIL, NULL);

	bonobo_object_construct (BONOBO_OBJECT (container), corba_container);
	
	return container;
}

BonoboUIContainer *
bonobo_ui_container_new (void)
{
	Bonobo_UIContainer corba_container;
	BonoboUIContainer *container;

	container = gtk_type_new (BONOBO_UI_CONTAINER_TYPE);

	corba_container = bonobo_ui_container_corba_object_create (BONOBO_OBJECT (container));
	if (corba_container == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (container));
		return NULL;
	}
	
	return bonobo_ui_container_construct (container, corba_container);
}

static void
blank_win (GtkObject *win, BonoboUIContainer *container)
{
	container->win = NULL;
	container->flags |= WIN_DESTROYED;
}

void
bonobo_ui_container_set_win (BonoboUIContainer *container,
			     BonoboWindow      *win)
{
	g_return_if_fail (BONOBO_IS_UI_CONTAINER (container));

	container->win = win;

	bonobo_window_set_ui_container (win, BONOBO_OBJECT (container));

	gtk_signal_connect (GTK_OBJECT (win), "destroy",
			    (GtkSignalFunc) blank_win, container);
}

BonoboWindow *
bonobo_ui_container_get_win (BonoboUIContainer *container)
{
	g_return_val_if_fail (BONOBO_IS_UI_CONTAINER (container), NULL);

	return container->win;
}
