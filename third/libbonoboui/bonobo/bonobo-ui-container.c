/*
 * bonobo-ui-container.c: The server side CORBA impl. for BonoboWindow.
 *
 * Author:
 *	Michael Meeks (michael@ximian.com)
 *
 * Copyright 2000,2001 Ximian, Inc.
 */

#include "config.h"

#include <libbonobo.h>
#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-ui-container.h>

#include <gtk/gtksignal.h>

#define PARENT_TYPE BONOBO_TYPE_OBJECT

static BonoboObjectClass *bonobo_ui_container_parent_class;

struct _BonoboUIContainerPrivate {
	BonoboUIEngine *engine;
	int             flags;
};

static BonoboUIEngine *
get_engine (PortableServer_Servant servant)
{
	BonoboUIContainer *container;

	container = BONOBO_UI_CONTAINER (bonobo_object_from_servant (servant));
	g_return_val_if_fail (container != NULL, NULL);

	if (!container->priv->engine)
		g_warning ("Trying to invoke CORBA method "
			   "on unbound UIContainer");

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
		if (xml [0] == '\0')
			err = BONOBO_UI_ERROR_OK;
		else {
			if (!(node = bonobo_ui_node_from_string (xml)))
				err = BONOBO_UI_ERROR_INVALID_XML;

			else
				err = bonobo_ui_engine_xml_merge_tree (
					engine, path, node, component_name);
		}
	}

	if (err) {
		if (err == BONOBO_UI_ERROR_INVALID_PATH)
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_UIContainer_InvalidPath, NULL);
		else
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_UIContainer_MalformedXML, NULL);
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
impl_Bonobo_UIContainer_setAttr (PortableServer_Servant   servant,
				 const CORBA_char        *path,
				 const CORBA_char        *attr,
				 const CORBA_char        *value,
				 const CORBA_char        *component,
				 CORBA_Environment       *ev)
{
	BonoboUIEngine *engine = get_engine (servant);

	/* Ignore the error for speed */
	bonobo_ui_engine_xml_set_prop (
		engine, path, attr, value, component);
}

static CORBA_char *
impl_Bonobo_UIContainer_getAttr (PortableServer_Servant servant,
				 const CORBA_char      *path,
				 const CORBA_char      *attr,
				 CORBA_Environment     *ev)
{
	CORBA_char     *xml;
	BonoboUIEngine *engine = get_engine (servant);
	gboolean        invalid_path = FALSE;

	xml = bonobo_ui_engine_xml_get_prop (
		engine, path, attr, &invalid_path);
	
	if (!xml) {
		if (invalid_path)
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_UIContainer_InvalidPath, NULL);
		else
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_UIContainer_NonExistentAttr, NULL);

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

/*	g_warning ("Node remove '%s' for '%s'", path, component_name); */

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
impl_Bonobo_UIContainer_execVerb (PortableServer_Servant servant,
				  const CORBA_char      *cname,
				  CORBA_Environment     *ev)
{
	BonoboUIEngine *engine = get_engine (servant);

	bonobo_ui_engine_exec_verb (engine, cname, ev);
}

static void
impl_Bonobo_UIContainer_uiEvent (PortableServer_Servant             servant,
				 const CORBA_char                  *id,
				 const Bonobo_UIComponent_EventType type,
				 const CORBA_char                  *state,
				 CORBA_Environment                 *ev)
{
	BonoboUIEngine *engine = get_engine (servant);

	bonobo_ui_engine_ui_event (engine, id, type, state, ev);
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
bonobo_ui_container_dispose (GObject *object)
{
	BonoboUIContainer *container = (BonoboUIContainer *) object;

	bonobo_ui_container_set_engine (container, NULL);

	G_OBJECT_CLASS (bonobo_ui_container_parent_class)->dispose (object);
}

static void
bonobo_ui_container_finalize (GObject *object)
{
	BonoboUIContainer *container = (BonoboUIContainer *) object;

	g_free (container->priv);
	container->priv = NULL;

	G_OBJECT_CLASS (bonobo_ui_container_parent_class)->finalize (object);
}

static void
bonobo_ui_container_init (GObject *object)
{
	BonoboUIContainer *container = (BonoboUIContainer *) object;

	container->priv = g_new0 (BonoboUIContainerPrivate, 1);
}

static void
bonobo_ui_container_class_init (BonoboUIContainerClass *klass)
{
	GObjectClass                *g_class = (GObjectClass *) klass;
	POA_Bonobo_UIContainer__epv *epv = &klass->epv;

	bonobo_ui_container_parent_class = g_type_class_peek_parent (klass);

	g_class->dispose  = bonobo_ui_container_dispose;
	g_class->finalize = bonobo_ui_container_finalize;

	epv->registerComponent   = impl_Bonobo_UIContainer_registerComponent;
	epv->deregisterComponent = impl_Bonobo_UIContainer_deregisterComponent;

	epv->setAttr    = impl_Bonobo_UIContainer_setAttr;
	epv->getAttr    = impl_Bonobo_UIContainer_getAttr;

	epv->setNode    = impl_Bonobo_UIContainer_setNode;
	epv->getNode    = impl_Bonobo_UIContainer_getNode;
	epv->removeNode = impl_Bonobo_UIContainer_removeNode;
	epv->exists     = impl_Bonobo_UIContainer_exists;

	epv->execVerb   = impl_Bonobo_UIContainer_execVerb;
	epv->uiEvent    = impl_Bonobo_UIContainer_uiEvent;

	epv->setObject  = impl_Bonobo_UIContainer_setObject;
	epv->getObject  = impl_Bonobo_UIContainer_getObject;

	epv->freeze     = impl_Bonobo_UIContainer_freeze;
	epv->thaw       = impl_Bonobo_UIContainer_thaw;
}

BONOBO_TYPE_FUNC_FULL (BonoboUIContainer, 
		       Bonobo_UIContainer,
		       PARENT_TYPE,
		       bonobo_ui_container)

/**
 * bonobo_ui_container_new:
 * @void: 
 * 
 * Return value: a newly created BonoboUIContainer
 **/
BonoboUIContainer *
bonobo_ui_container_new (void)
{
	return g_object_new (BONOBO_TYPE_UI_CONTAINER, NULL);
}

/**
 * bonobo_ui_container_set_engine:
 * @container: the container
 * @engine: the engine
 * 
 * Associates the BonoboUIContainer with a #BonoboUIEngine
 * that it will use to handle all the UI merging requests.
 **/
void
bonobo_ui_container_set_engine (BonoboUIContainer *container,
				BonoboUIEngine    *engine)
{
	BonoboUIEngine *old_engine;

	g_return_if_fail (BONOBO_IS_UI_CONTAINER (container));

	if (container->priv->engine == engine)
		return;

	old_engine = container->priv->engine;
	container->priv->engine = engine;

	if (old_engine)
		bonobo_ui_engine_set_ui_container (old_engine, NULL);

	if (engine)
		bonobo_ui_engine_set_ui_container (engine, container);
}

/**
 * bonobo_ui_container_get_engine:
 * @container: the UI container
 * 
 * Get the associated #BonoboUIEngine
 * 
 * Return value: the engine
 **/
BonoboUIEngine *
bonobo_ui_container_get_engine (BonoboUIContainer *container)
{
	g_return_val_if_fail (BONOBO_IS_UI_CONTAINER (container), NULL);

	return container->priv->engine;
}
