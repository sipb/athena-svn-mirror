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

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static GtkObjectClass *bonobo_ui_container_parent_class;

POA_Bonobo_UIContainer__vepv bonobo_ui_container_vepv;

struct _BonoboUIContainerPrivate {
	BonoboUIEngine *engine;
	int             flags;
};

#define WIN_DESTROYED 0x1

static BonoboUIEngine *
get_engine (PortableServer_Servant servant)
{
	BonoboUIContainer *container;

	container = BONOBO_UI_CONTAINER (bonobo_object_from_servant (servant));
	g_return_val_if_fail (container != NULL, NULL);

	if (container->priv->engine == NULL) {
		if (!container->priv->flags & WIN_DESTROYED)
			g_warning ("Trying to invoke CORBA method "
				   "on unbound UIContainer");
		return NULL;
	} else
		return container->priv->engine;
}

static void
impl_Bonobo_UIContainer_registerComponent (PortableServer_Servant   servant,
					   const CORBA_char        *component_name,
					   const Bonobo_Unknown     object,
					   CORBA_Environment       *ev)
{
	BonoboUIEngine *engine = get_engine (servant);

	bonobo_ui_engine_register_component (engine, component_name, object);
}

static void
impl_Bonobo_UIContainer_deregisterComponent (PortableServer_Servant servant,
					     const CORBA_char      *component_name,
					     CORBA_Environment     *ev)
{
	BonoboUIEngine *engine = get_engine (servant);

	if (!engine)
		return;

	bonobo_ui_engine_deregister_component (engine, component_name);
}

static void
impl_Bonobo_UIContainer_setNode (PortableServer_Servant   servant,
				 const CORBA_char        *path,
				 const CORBA_char        *xml,
				 const CORBA_char        *component_name,
				 CORBA_Environment       *ev)
{
	BonoboUIEngine *engine = get_engine (servant);
	BonoboUIError   err;
	BonoboUINode   *node;

/*	fprintf (stderr, "Merging :\n%s\n", xml);*/

	if (!xml)
		err = BONOBO_UI_ERROR_BAD_PARAM;
	else {
		node = bonobo_ui_node_from_string (xml);
		
		if (!node)
			err = BONOBO_UI_ERROR_INVALID_XML;
		else
			err = bonobo_ui_engine_xml_merge_tree (
				engine, path, node, component_name);
	}

	if (err) {
		if (err == BONOBO_UI_ERROR_INVALID_PATH)
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
	BonoboUIEngine *engine = get_engine (servant);
	CORBA_char *xml;

	xml = bonobo_ui_engine_xml_get (engine, path, nodeOnly);
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
	BonoboUIEngine *engine = get_engine (servant);
	BonoboUIError err;

	if (!engine)
		return;

/*	g_warning ("Node remove '%s' for '%s'", path, component_name);*/

	err = bonobo_ui_engine_xml_rm (engine, path, component_name);

	if (err)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_UIContainer_InvalidPath, NULL);
}

static CORBA_boolean
impl_Bonobo_UIContainer_exists (PortableServer_Servant servant,
				const CORBA_char      *path,
				CORBA_Environment     *ev)
{
	BonoboUIEngine *engine = get_engine (servant);

	return bonobo_ui_engine_xml_node_exists (engine, path);
}

static void
impl_Bonobo_UIContainer_setObject (PortableServer_Servant servant,
				   const CORBA_char      *path,
				   const Bonobo_Unknown   control,
				   CORBA_Environment     *ev)
{
	BonoboUIEngine *engine = get_engine (servant);
	BonoboUIError err;

	err = bonobo_ui_engine_object_set (engine, path, control, ev);

	if (err)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_UIContainer_InvalidPath, NULL);
}

static Bonobo_Unknown
impl_Bonobo_UIContainer_getObject (PortableServer_Servant servant,
				   const CORBA_char      *path,
				   CORBA_Environment     *ev)
{
	BonoboUIEngine *engine = get_engine (servant);
	BonoboUIError err;
	Bonobo_Unknown object;

	err = bonobo_ui_engine_object_get (engine, path, &object, ev);

	if (err)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_UIContainer_InvalidPath, NULL);

	return object;
}

static void
impl_Bonobo_UIContainer_freeze (PortableServer_Servant   servant,
				CORBA_Environment       *ev)
{
	BonoboUIEngine *engine = get_engine (servant);

	bonobo_ui_engine_freeze (engine);
}

static void
impl_Bonobo_UIContainer_thaw (PortableServer_Servant   servant,
			      CORBA_Environment       *ev)
{
	BonoboUIEngine *engine = get_engine (servant);

	bonobo_ui_engine_thaw (engine);
}

static void
bonobo_ui_container_destroy (GtkObject *object)
{
	bonobo_ui_container_parent_class->destroy (object);
}

static void
bonobo_ui_container_finalize (GtkObject *object)
{
	BonoboUIContainer *container = (BonoboUIContainer *) object;

	g_free (container->priv);
	container->priv = NULL;

	bonobo_ui_container_parent_class->finalize (object);
}

static void
bonobo_ui_container_init (GtkObject *object)
{
	BonoboUIContainer *container = (BonoboUIContainer *) object;

	container->priv = g_new0 (BonoboUIContainerPrivate, 1);
}

static void
bonobo_ui_container_class_init (BonoboUIContainerClass *klass)
{
	GtkObjectClass              *gtk_class = (GtkObjectClass *) klass;
	POA_Bonobo_UIContainer__epv *epv = &klass->epv;

	bonobo_ui_container_parent_class = gtk_type_class (PARENT_TYPE);
	
	gtk_class->destroy  = bonobo_ui_container_destroy;
	gtk_class->finalize = bonobo_ui_container_finalize;

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
}

BONOBO_X_TYPE_FUNC_FULL (BonoboUIContainer, 
			   Bonobo_UIContainer,
			   PARENT_TYPE,
			   bonobo_ui_container);

BonoboUIContainer *
bonobo_ui_container_new (void)
{
	return gtk_type_new (BONOBO_UI_CONTAINER_TYPE);
}

static void
blank_engine (GtkObject *win, BonoboUIContainer *container)
{
	container->priv->engine = NULL;
	container->win          = NULL;
	container->priv->flags |= WIN_DESTROYED;
}

void
bonobo_ui_container_set_engine (BonoboUIContainer *container,
				BonoboUIEngine    *engine)
{
	g_return_if_fail (BONOBO_IS_UI_CONTAINER (container));

	container->priv->engine = engine;
	bonobo_ui_engine_set_ui_container (
		engine, BONOBO_OBJECT (container));

	gtk_signal_connect_while_alive (
		GTK_OBJECT (engine), "destroy",
		(GtkSignalFunc) blank_engine,
		container, GTK_OBJECT (container));
}

BonoboUIEngine *
bonobo_ui_container_get_engine (BonoboUIContainer *container)
{
	g_return_val_if_fail (BONOBO_IS_UI_CONTAINER (container), NULL);

	return container->priv->engine;
}

void
bonobo_ui_container_set_win (BonoboUIContainer *container,
			     BonoboWindow      *win)
{
	g_return_if_fail (BONOBO_IS_UI_CONTAINER (container));

	container->win = win;

	bonobo_ui_container_set_engine (
		container, bonobo_window_get_ui_engine (win));

	gtk_signal_connect_while_alive (
		GTK_OBJECT (win), "destroy",
		(GtkSignalFunc) blank_engine,
		container, GTK_OBJECT (container));
}

BonoboWindow *
bonobo_ui_container_get_win (BonoboUIContainer *container)
{
	g_return_val_if_fail (BONOBO_IS_UI_CONTAINER (container), NULL);

	g_warning ("bonobo_ui_container_get_win is deprecated");

	return container->win;
}
