/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * gnome-component-ui.c: Client UI signal multiplexer and verb repository.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <gnome.h>
#include <bonobo.h>
#include <bonobo/bonobo-ui-component.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>

static BonoboObjectClass *bonobo_ui_component_parent_class;
enum {
	EXEC_VERB,
	UI_EVENT,
	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

POA_Bonobo_UIComponent__vepv bonobo_ui_component_vepv;

#define GET_CLASS(c) (BONOBO_UI_COMPONENT_CLASS (GTK_OBJECT (c)->klass))

typedef struct {
	char              *id;
	BonoboUIListenerFn cb;
	gpointer           user_data;
	GDestroyNotify     destroy_fn;
} UIListener;

typedef struct {
	char          *cname;
	BonoboUIVerbFn cb;
	gpointer       user_data;
	GDestroyNotify destroy_fn;
} UIVerb;

struct _BonoboUIComponentPrivate {
	GHashTable        *verbs;
	GHashTable        *listeners;
	char              *name;
	Bonobo_UIContainer container;
};

static inline BonoboUIComponent *
bonobo_ui_from_servant (PortableServer_Servant servant)
{
	return BONOBO_UI_COMPONENT (bonobo_object_from_servant (servant));
}

static gboolean
verb_destroy (gpointer dummy, UIVerb *verb, gpointer dummy2)
{
	if (verb) {
		if (verb->destroy_fn)
			verb->destroy_fn (verb->user_data);
		verb->destroy_fn = NULL;
		g_free (verb->cname);
		g_free (verb);
	}
	return TRUE;
}

static gboolean
listener_destroy (gpointer dummy, UIListener *l, gpointer dummy2)
{
	if (l) {
		if (l->destroy_fn)
			l->destroy_fn (l->user_data);
		l->destroy_fn = NULL;
		g_free (l->id);
		g_free (l);
	}
	return TRUE;
}

static void
ui_event (BonoboUIComponent           *component,
	  const char                  *id,
	  Bonobo_UIComponent_EventType type,
	  const char                  *state)
{
	UIListener *list;

	list = g_hash_table_lookup (component->priv->listeners, id);
	if (list && list->cb)
		list->cb (component, id, type,
			  state, list->user_data);
}

static CORBA_char *
impl_Bonobo_UIComponent_describeVerbs (PortableServer_Servant servant,
				       CORBA_Environment     *ev)
{
	g_warning ("FIXME: Describe verbs unimplemented");
	return CORBA_string_dup ("<NoUIVerbDescriptionCodeYet/>");
}

static void
impl_Bonobo_UIComponent_execVerb (PortableServer_Servant servant,
				  const CORBA_char      *cname,
				  CORBA_Environment     *ev)
{
	BonoboUIComponent *component;
	UIVerb *verb;

	component = bonobo_ui_from_servant (servant);

	bonobo_object_ref (BONOBO_OBJECT (component));
	
/*	g_warning ("TESTME: Exec verb '%s'", cname);*/

	verb = g_hash_table_lookup (component->priv->verbs, cname);
	if (verb && verb->cb)
		verb->cb (component, verb->user_data, cname);
	else
		g_warning ("FIXME: verb '%s' not found, emit exception", cname);

	gtk_signal_emit (GTK_OBJECT (component),
			 signals [EXEC_VERB],
			 cname);

	bonobo_object_unref (BONOBO_OBJECT (component));
}

static void
impl_Bonobo_UIComponent_uiEvent (PortableServer_Servant             servant,
				 const CORBA_char                  *id,
				 const Bonobo_UIComponent_EventType type,
				 const CORBA_char                  *state,
				 CORBA_Environment                 *ev)
{
	BonoboUIComponent *component;

	component = bonobo_ui_from_servant (servant);

/*	g_warning ("TESTME: Event '%s' '%d' '%s'\n", path, type, state);*/

	bonobo_object_ref (BONOBO_OBJECT (component));

	gtk_signal_emit (GTK_OBJECT (component),
			 signals [UI_EVENT], id, type, state);

	bonobo_object_unref (BONOBO_OBJECT (component));
}


void
bonobo_ui_component_add_verb_full (BonoboUIComponent  *component,
				   const char         *cname,
				   BonoboUIVerbFn      fn,
				   gpointer            user_data,
				   GDestroyNotify      destroy_fn)
{
	UIVerb *verb;
	BonoboUIComponentPrivate *priv;

	g_return_if_fail (cname != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));

	priv = component->priv;

	if ((verb = g_hash_table_lookup (priv->verbs, cname))) {
		g_hash_table_remove (priv->verbs, cname);
		verb_destroy (NULL, verb, NULL);
	}

	verb = g_new (UIVerb, 1);
	verb->cname      = g_strdup (cname);
	verb->cb         = fn;
	verb->user_data  = user_data;
	verb->destroy_fn = destroy_fn;

	g_hash_table_insert (priv->verbs, verb->cname, verb);
}

void
bonobo_ui_component_add_verb (BonoboUIComponent  *component,
			      const char         *cname,
			      BonoboUIVerbFn      fn,
			      gpointer            user_data)
{
	bonobo_ui_component_add_verb_full (
		component, cname, fn, user_data, NULL);
}

typedef struct {
	gboolean    by_name;
	const char *name;
	gboolean    by_func;
	gpointer    func;
	gboolean    by_data;
	gpointer    user_data;
} RemoveInfo;

static gboolean
remove_verb (gpointer	key,
	     gpointer	value,
	     gpointer	user_data)
{
	RemoveInfo *info = user_data;
	UIVerb     *verb = value;

	if (info->by_name && info->name &&
	    !strcmp (verb->cname, info->name))
		return verb_destroy (NULL, verb, NULL);

	else if (info->by_func &&
		 (BonoboUIVerbFn) info->func == verb->cb)
		return verb_destroy (NULL, verb, NULL);

	else if (info->by_data &&
		 (BonoboUIVerbFn) info->user_data == verb->user_data)
		return verb_destroy (NULL, verb, NULL);

	return FALSE;
}

void
bonobo_ui_component_remove_verb (BonoboUIComponent  *component,
				 const char         *cname)
{
	RemoveInfo info;

	memset (&info, 0, sizeof (info));

	info.by_name = TRUE;
	info.name = cname;

	g_hash_table_foreach_remove (component->priv->verbs, remove_verb, &info);
}

void
bonobo_ui_component_remove_verb_by_func (BonoboUIComponent  *component,
					 BonoboUIVerbFn      fn)
{
	RemoveInfo info;

	memset (&info, 0, sizeof (info));

	info.by_func = TRUE;
	info.func = (gpointer) fn;

	g_hash_table_foreach_remove (component->priv->verbs, remove_verb, &info);
}

void
bonobo_ui_component_remove_verb_by_data (BonoboUIComponent  *component,
					 gpointer            user_data)
{
	RemoveInfo info;

	memset (&info, 0, sizeof (info));

	info.by_data = TRUE;
	info.user_data = user_data;

	g_hash_table_foreach_remove (component->priv->verbs, remove_verb, &info);
}

void
bonobo_ui_component_add_listener_full (BonoboUIComponent  *component,
				       const char         *id,
				       BonoboUIListenerFn  fn,
				       gpointer            user_data,
				       GDestroyNotify      destroy_fn)
{
	UIListener *list;
	BonoboUIComponentPrivate *priv;

	g_return_if_fail (fn != NULL);
	g_return_if_fail (id != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));

	priv = component->priv;

	if ((list = g_hash_table_lookup (priv->listeners, id))) {
		g_hash_table_remove (priv->listeners, id);
		listener_destroy (NULL, list, NULL);
	}

	list = g_new (UIListener, 1);
	list->cb = fn;
	list->id = g_strdup (id);
	list->user_data = user_data;
	list->destroy_fn = destroy_fn;

	g_hash_table_insert (priv->listeners, list->id, list);	
}

void
bonobo_ui_component_add_listener (BonoboUIComponent  *component,
				  const char         *id,
				  BonoboUIListenerFn  fn,
				  gpointer            user_data)
{
	bonobo_ui_component_add_listener_full (
		component, id, fn, user_data, NULL);
}

static gboolean
remove_listener (gpointer	key,
		 gpointer	value,
		 gpointer	user_data)
{
	RemoveInfo *info = user_data;
	UIListener     *listener = value;

	if (info->by_name && info->name &&
	    !strcmp (listener->id, info->name))
		return listener_destroy (NULL, listener, NULL);

	else if (info->by_func &&
		 (BonoboUIListenerFn) info->func == listener->cb)
		return listener_destroy (NULL, listener, NULL);

	else if (info->by_data &&
		 (BonoboUIListenerFn) info->user_data == listener->user_data)
		return listener_destroy (NULL, listener, NULL);

	return FALSE;
}

void
bonobo_ui_component_remove_listener (BonoboUIComponent  *component,
				     const char         *cname)
{
	RemoveInfo info;

	memset (&info, 0, sizeof (info));

	info.by_name = TRUE;
	info.name = cname;

	g_hash_table_foreach_remove (component->priv->listeners, remove_listener, &info);
}

void
bonobo_ui_component_remove_listener_by_func (BonoboUIComponent  *component,
					     BonoboUIListenerFn      fn)
{
	RemoveInfo info;

	memset (&info, 0, sizeof (info));

	info.by_func = TRUE;
	info.func = (gpointer) fn;

	g_hash_table_foreach_remove (component->priv->listeners, remove_listener, &info);
}

void
bonobo_ui_component_remove_listener_by_data (BonoboUIComponent  *component,
					     gpointer            user_data)
{
	RemoveInfo info;

	memset (&info, 0, sizeof (info));

	info.by_data = TRUE;
	info.user_data = user_data;

	g_hash_table_foreach_remove (component->priv->listeners, remove_listener, &info);
}

static void
bonobo_ui_component_destroy (GtkObject *object)
{
	BonoboUIComponent *comp = (BonoboUIComponent *) object;
	BonoboUIComponentPrivate *priv = comp->priv;

	if (priv) {
		g_hash_table_foreach_remove (
			priv->verbs, (GHRFunc) verb_destroy, NULL);
		g_hash_table_destroy (priv->verbs);
		priv->verbs = NULL;

		g_hash_table_foreach_remove (
			priv->listeners,
			(GHRFunc) listener_destroy, NULL);
		g_hash_table_destroy (priv->listeners);
		priv->listeners = NULL;

		g_free (priv->name);

		g_free (priv);
	}
	comp->priv = NULL;
}

/**
 * bonobo_ui_component_get_epv:
 *
 * Returns: The EPV for the default BonoboUIComponent implementation.  
 */
POA_Bonobo_UIComponent__epv *
bonobo_ui_component_get_epv (void)
{
	POA_Bonobo_UIComponent__epv *epv;

	epv = g_new0 (POA_Bonobo_UIComponent__epv, 1);

	epv->describeVerbs = impl_Bonobo_UIComponent_describeVerbs;
	epv->execVerb      = impl_Bonobo_UIComponent_execVerb;
	epv->uiEvent       = impl_Bonobo_UIComponent_uiEvent;

	return epv;
}

Bonobo_UIComponent
bonobo_ui_component_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_UIComponent *servant;
	CORBA_Environment       ev;

	servant = (POA_Bonobo_UIComponent *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_ui_component_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_UIComponent__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

BonoboUIComponent *
bonobo_ui_component_construct (BonoboUIComponent *ui_component,
			       Bonobo_UIComponent corba_component,
			       const char        *name)
{
	g_return_val_if_fail (corba_component != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (ui_component), NULL);

	ui_component->priv->name = g_strdup (name);

	return BONOBO_UI_COMPONENT (
		bonobo_object_construct (BONOBO_OBJECT (ui_component),
					 corba_component));
}

BonoboUIComponent *
bonobo_ui_component_new (const char *name)
{
	BonoboUIComponent *component;
	Bonobo_UIComponent corba_component;

	component = gtk_type_new (BONOBO_UI_COMPONENT_TYPE);
	if (component == NULL)
		return NULL;

	corba_component = bonobo_ui_component_corba_object_create (
		BONOBO_OBJECT (component));

	if (corba_component == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (component));
		return NULL;
	}

	return BONOBO_UI_COMPONENT (bonobo_ui_component_construct (
		component, corba_component, name));
}

BonoboUIComponent *
bonobo_ui_component_new_default (void)
{
	char              *name;
	BonoboUIComponent *component;

	static int idx = 0;

	name = g_strdup_printf (
		"%s-%s-%d-%d",
		gnome_app_id ? gnome_app_id : "unknown",
		gnome_app_version ? gnome_app_version : "-.-",
		getpid (), idx++);

	component = bonobo_ui_component_new (name);
	
	g_free (name);

	return component;
}

void
bonobo_ui_component_set_name (BonoboUIComponent  *component,
			      const char         *name)
{
	g_return_if_fail (name != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	
	g_free (component->priv->name);
	component->priv->name = g_strdup (name);
}

const char *
bonobo_ui_component_get_name (BonoboUIComponent  *component)
{
	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component), NULL);
	
	return component->priv->name;
}

void
bonobo_ui_component_set (BonoboUIComponent  *component,
			 const char         *path,
			 const char         *xml,
			 CORBA_Environment  *ev)
{
	GET_CLASS (component)->xml_set (component, path, xml, ev);
}

static void
impl_xml_set (BonoboUIComponent  *component,
	      const char         *path,
	      const char         *xml,
	      CORBA_Environment  *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;
	char              *name;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	name = component->priv->name ? component->priv->name : "";

	Bonobo_UIContainer_setNode (container, path, xml,
				     name, real_ev);

	if (BONOBO_EX (real_ev) && !ev)
		g_warning ("Serious exception on node_set '$%s' of '%s' to '%s'",
			   bonobo_exception_get_text (real_ev), xml, path);

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

void
bonobo_ui_component_set_tree (BonoboUIComponent *component,
			      const char        *path,
			      BonoboUINode      *node,
			      CORBA_Environment *ev)
{
	char *str;

	str = bonobo_ui_node_to_string (node, TRUE);	

/*	fprintf (stderr, "Merging '%s'\n", str); */
	
	bonobo_ui_component_set (
		component, path, str, ev);

	bonobo_ui_node_free_string (str);
}

void
bonobo_ui_component_set_translate (BonoboUIComponent  *component,
				   const char         *path,
				   const char         *xml,
				   CORBA_Environment  *ev)
{
	BonoboUINode *node;

	if (!xml)
		return;

	node = bonobo_ui_node_from_string (xml);

	bonobo_ui_util_translate_ui (node);

	bonobo_ui_component_set_tree (component, path, node, ev);

	bonobo_ui_node_free (node);
}

CORBA_char *
bonobo_ui_component_get (BonoboUIComponent *component,
			 const char        *path,
			 gboolean           recurse,
			 CORBA_Environment *ev)
{
	return GET_CLASS (component)->xml_get (component, path, recurse, ev);
}

static CORBA_char *
impl_xml_get (BonoboUIComponent *component,
	      const char        *path,
	      gboolean           recurse,
	      CORBA_Environment *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	CORBA_char *xml;
	Bonobo_UIContainer container;

	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component), NULL);
	container = component->priv->container;
	g_return_val_if_fail (container != CORBA_OBJECT_NIL, NULL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	xml = Bonobo_UIContainer_getNode (container, path, !recurse, real_ev);

	if (BONOBO_EX (real_ev)) {
		if (!ev)
			g_warning ("Serious exception getting node '%s' '$%s'",
				   path, bonobo_exception_get_text (real_ev));
		return NULL;
	}

	return xml;
}

BonoboUINode *
bonobo_ui_component_get_tree (BonoboUIComponent  *component,
			      const char         *path,
			      gboolean            recurse,
			      CORBA_Environment  *ev)
{	
	char *xml;
	BonoboUINode *node;

	xml = bonobo_ui_component_get (component, path, recurse, ev);

	if (!xml)
		return NULL;

	node = bonobo_ui_node_from_string (xml);

	CORBA_free (xml);

	if (!node)
		return NULL;

	bonobo_ui_xml_strip (&node);

	return node;
}

void
bonobo_ui_component_rm (BonoboUIComponent  *component,
			const char         *path,
			CORBA_Environment  *ev)
{
	GET_CLASS (component)->xml_rm (component, path, ev);
}

static void
impl_xml_rm (BonoboUIComponent  *component,
	     const char         *path,
	     CORBA_Environment  *ev)
{
	BonoboUIComponentPrivate *priv;
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	priv = component->priv;

	Bonobo_UIContainer_removeNode (
		container, path, priv->name, real_ev);

	if (!ev && BONOBO_EX (real_ev))
		g_warning ("Serious exception removing path  '%s' '%s'",
			   path, bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}


void
bonobo_ui_component_object_set (BonoboUIComponent  *component,
				const char         *path,
				Bonobo_Unknown      control,
				CORBA_Environment  *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	Bonobo_UIContainer_setObject (container, path, control, real_ev);

	if (!ev && BONOBO_EX (real_ev))
		g_warning ("Serious exception setting object '%s' '%s'",
			   path, bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

Bonobo_Unknown
bonobo_ui_component_object_get (BonoboUIComponent  *component,
				const char         *path,
				CORBA_Environment  *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_Unknown     ret;
	Bonobo_UIContainer container;

	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component),
			      CORBA_OBJECT_NIL);
	container = component->priv->container;
	g_return_val_if_fail (container != CORBA_OBJECT_NIL,
			      CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	ret = Bonobo_UIContainer_getObject (container, path, real_ev);

	if (!ev && BONOBO_EX (real_ev))
		g_warning ("Serious exception getting object '%s' '%s'",
			   path, bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return ret;
}

void
bonobo_ui_component_add_verb_list_with_data (BonoboUIComponent  *component,
					     BonoboUIVerb       *list,
					     gpointer            user_data)
{
	BonoboUIVerb *l;

	g_return_if_fail (list != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));

	for (l = list; l && l->cname; l++) {
		bonobo_ui_component_add_verb (
			component, l->cname, l->cb,
			user_data?user_data:l->user_data);
	}
}

void
bonobo_ui_component_add_verb_list (BonoboUIComponent  *component,
				   BonoboUIVerb       *list)
{
	bonobo_ui_component_add_verb_list_with_data (component, list, NULL);
}

void
bonobo_ui_component_freeze (BonoboUIComponent *component,
			    CORBA_Environment *ev)
{
	GET_CLASS (component)->freeze (component, ev);
}

static void
impl_freeze (BonoboUIComponent *component,
	     CORBA_Environment *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	Bonobo_UIContainer_freeze (container, real_ev);

	if (BONOBO_EX (real_ev) && !ev)
		g_warning ("Serious exception on UI freeze '$%s'",
			   bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

void
bonobo_ui_component_thaw (BonoboUIComponent *component,
			  CORBA_Environment *ev)
{
	GET_CLASS (component)->thaw (component, ev);
}

static void
impl_thaw (BonoboUIComponent *component,
	   CORBA_Environment *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	Bonobo_UIContainer_thaw (container, real_ev);

	if (BONOBO_EX (real_ev) && !ev)
		g_warning ("Serious exception on UI thaw '$%s'",
			   bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

void
bonobo_ui_component_set_prop (BonoboUIComponent  *component,
			      const char         *path,
			      const char         *prop,
			      const char         *value,
			      CORBA_Environment  *opt_ev)
{
	if (prop && (!strcmp (prop, "label") || !strcmp (prop, "tip"))) {
		char *encoded = bonobo_ui_util_encode_str (value);
		GET_CLASS (component)->set_prop (component, path, prop, encoded, opt_ev);
		g_free (encoded);
	} else
		GET_CLASS (component)->set_prop (component, path, prop, value, opt_ev);
}

static void
impl_set_prop (BonoboUIComponent  *component,
	       const char         *path,
	       const char         *prop,
	       const char         *value,
	       CORBA_Environment  *opt_ev)
{
	BonoboUINode *node;
	char *parent_path;
	Bonobo_UIContainer container;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));
	container = component->priv->container;
	g_return_if_fail (container != CORBA_OBJECT_NIL);

	node = bonobo_ui_component_get_tree (
		component, path, FALSE, opt_ev);

	g_return_if_fail (node != NULL);

	bonobo_ui_node_set_attr (node, prop, value);

	parent_path = bonobo_ui_xml_get_parent_path (path);

	bonobo_ui_component_set_tree (
		component, parent_path, node, opt_ev);

	g_free (parent_path);

	bonobo_ui_node_free (node);
}

gchar *
bonobo_ui_component_get_prop (BonoboUIComponent *component,
			      const char        *path,
			      const char        *prop,
			      CORBA_Environment *opt_ev)
{
	char *txt;

	txt = GET_CLASS (component)->get_prop (component, path, prop, opt_ev);
	
	if (prop && (!strcmp (prop, "label") || !strcmp (prop, "tip"))) {
		char *decoded;
		gboolean err;

		decoded = bonobo_ui_util_decode_str (txt, &err);
		if (err)
			g_warning ("Encoding error getting prop '%s' at path '%s'",
				   prop, path);
		g_free (txt);

		return decoded;
	} else
		return txt;
}

static gchar *
impl_get_prop (BonoboUIComponent *component,
	       const char        *path,
	       const char        *prop,
	       CORBA_Environment *opt_ev)
{
	BonoboUINode *node;
	xmlChar *ans;
	gchar   *ret;

	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component), NULL);

	node = bonobo_ui_component_get_tree (
		component, path, FALSE, opt_ev);

	g_return_val_if_fail (node != NULL, NULL);

	ans = bonobo_ui_node_get_attr (node, prop);
	if (ans) {
		ret = g_strdup (ans);
		bonobo_ui_node_free_string (ans);
	} else
		ret = NULL;

	bonobo_ui_node_free (node);

	return ret;
}

gboolean
bonobo_ui_component_path_exists (BonoboUIComponent *component,
				 const char        *path,
				 CORBA_Environment *ev)
{
	return GET_CLASS (component)->exists (component, path, ev);
}

static gboolean
impl_exists (BonoboUIComponent *component,
	     const char        *path,
	     CORBA_Environment *ev)
{
	gboolean ret;
	Bonobo_UIContainer container;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component), FALSE);
	container = component->priv->container;
	g_return_val_if_fail (container != CORBA_OBJECT_NIL, FALSE);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	ret = Bonobo_UIContainer_exists (container, path, real_ev);

	if (BONOBO_EX (real_ev)) {
		ret = FALSE;
		if (!ev)
			g_warning ("Serious exception on path_exists '$%s'",
				   bonobo_exception_get_text (real_ev));
	}

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return ret;
}

void
bonobo_ui_component_set_status (BonoboUIComponent *component,
				const char        *text,
				CORBA_Environment *opt_ev)
{
	if (text == NULL ||
	    text [0] == '\0') { /* Remove what was there to reveal other msgs */
		bonobo_ui_component_rm (component, "/status/main/*", opt_ev);
	} else {
		char *str, *encoded;

		encoded = bonobo_ui_util_encode_str (text);
		str = g_strdup_printf ("<item name=\"main\">%s</item>", encoded);
		g_free (encoded);
		
		bonobo_ui_component_set (component, "/status", str, opt_ev);
		
		g_free (str);
	}
}

void
bonobo_ui_component_unset_container (BonoboUIComponent *component)
{
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));

	if (component->priv->container != CORBA_OBJECT_NIL) {
		CORBA_Environment  ev;
		char              *name;

		bonobo_ui_component_rm (component, "/", NULL);

		CORBA_exception_init (&ev);

		name = component->priv->name ? component->priv->name : "";

		Bonobo_UIContainer_deregisterComponent (
			component->priv->container, name, &ev);
		
		if (BONOBO_EX (&ev))
			g_warning ("Serious exception deregistering component '%s'",
				   bonobo_exception_get_text (&ev));

		CORBA_exception_free (&ev);

		bonobo_object_release_unref (component->priv->container, NULL);
	}

	component->priv->container = CORBA_OBJECT_NIL;
}

void
bonobo_ui_component_set_container (BonoboUIComponent *component,
				   Bonobo_UIContainer container)
{
	Bonobo_UIContainer ref_cont;

	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));

	if (container != CORBA_OBJECT_NIL) {
		Bonobo_UIComponent corba_component;
		char              *name;
		CORBA_Environment  ev;

		ref_cont = 		
			bonobo_object_dup_ref (container, NULL);

		CORBA_exception_init (&ev);

		corba_component = 
			bonobo_object_corba_objref (BONOBO_OBJECT (component));

		name = component->priv->name ? component->priv->name : "";

		Bonobo_UIContainer_registerComponent (
			ref_cont, name, corba_component, &ev);

		if (BONOBO_EX (&ev))
			g_warning ("Serious exception registering component '$%s'",
				   bonobo_exception_get_text (&ev));
		
		CORBA_exception_free (&ev);
	} else
		ref_cont = CORBA_OBJECT_NIL;

	bonobo_ui_component_unset_container (component);

	component->priv->container = ref_cont;
}

Bonobo_UIContainer
bonobo_ui_component_get_container (BonoboUIComponent *component)
{
	g_return_val_if_fail (BONOBO_IS_UI_COMPONENT (component),
			      CORBA_OBJECT_NIL);
	
	return component->priv->container;
}

static void
bonobo_ui_component_class_init (BonoboUIComponentClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	BonoboUIComponentClass *uclass = BONOBO_UI_COMPONENT_CLASS (klass);
	
	bonobo_ui_component_parent_class = gtk_type_class (BONOBO_OBJECT_TYPE);

	object_class->destroy = bonobo_ui_component_destroy;

	uclass->ui_event = ui_event;

	bonobo_ui_component_vepv.Bonobo_Unknown_epv =
		bonobo_object_get_epv ();
	bonobo_ui_component_vepv.Bonobo_UIComponent_epv =
		bonobo_ui_component_get_epv ();

	signals [EXEC_VERB] = gtk_signal_new (
		"exec_verb", GTK_RUN_FIRST,
		object_class->type,
		GTK_SIGNAL_OFFSET (BonoboUIComponentClass, exec_verb),
		gtk_marshal_NONE__STRING,
		GTK_TYPE_NONE, 1, GTK_TYPE_STRING);

	signals [UI_EVENT] = gtk_signal_new (
		"ui_event", GTK_RUN_FIRST,
		object_class->type,
		GTK_SIGNAL_OFFSET (BonoboUIComponentClass, ui_event),
		gtk_marshal_NONE__POINTER_INT_POINTER,
		GTK_TYPE_NONE, 3, GTK_TYPE_STRING, GTK_TYPE_INT,
		GTK_TYPE_STRING);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	uclass->freeze   = impl_freeze;
	uclass->thaw     = impl_thaw;
	uclass->xml_set  = impl_xml_set;
	uclass->xml_get  = impl_xml_get;
	uclass->xml_rm   = impl_xml_rm;
	uclass->set_prop = impl_set_prop;
	uclass->get_prop = impl_get_prop;
	uclass->exists   = impl_exists;

}

static void
bonobo_ui_component_init (BonoboUIComponent *component)
{
	BonoboUIComponentPrivate *priv;

	priv = g_new0 (BonoboUIComponentPrivate, 1);
	priv->verbs = g_hash_table_new (g_str_hash, g_str_equal);
	priv->listeners = g_hash_table_new (g_str_hash, g_str_equal);
	priv->container = CORBA_OBJECT_NIL;

	component->priv = priv;
}

/**
 * bonobo_ui_component_get_type:
 *
 * Returns: the GtkType of the BonoboUIComponent class.
 */
GtkType
bonobo_ui_component_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboUIComponent",
			sizeof (BonoboUIComponent),
			sizeof (BonoboUIComponentClass),
			(GtkClassInitFunc) bonobo_ui_component_class_init,
			(GtkObjectInitFunc) bonobo_ui_component_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

