/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-moniker: Object naming abstraction
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-moniker-extender.h>

struct _BonoboMonikerPrivate {
	Bonobo_Moniker parent;

	int            prefix_len;
	char          *prefix;

	char          *name;
};

static GtkObjectClass *bonobo_moniker_parent_class;

POA_Bonobo_Moniker__vepv bonobo_moniker_vepv;

#define CLASS(o) BONOBO_MONIKER_CLASS (GTK_OBJECT (o)->klass)

static inline BonoboMoniker *
bonobo_moniker_from_servant (PortableServer_Servant servant)
{
	return BONOBO_MONIKER (bonobo_object_from_servant (servant));
}

static Bonobo_Moniker
impl_get_parent (PortableServer_Servant servant,
		 CORBA_Environment     *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);

	return CLASS (moniker)->get_parent (moniker, ev);
}

static void
impl_set_parent (PortableServer_Servant servant,
		 const Bonobo_Moniker   value,
		 CORBA_Environment     *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);

	return CLASS (moniker)->set_parent (moniker, value, ev);
}

/**
 * bonobo_moniker_set_parent:
 * @moniker: the moniker
 * @parent: the parent
 * @ev: a corba exception environment
 * 
 *  This sets the monikers parent; a moniker is really a long chain
 * of hierarchical monikers; referenced by the most local moniker.
 * This sets the parent pointer.
 **/
void
bonobo_moniker_set_parent (BonoboMoniker     *moniker,
			   Bonobo_Moniker     parent,
			   CORBA_Environment *ev)
{
	g_return_if_fail (BONOBO_IS_MONIKER (moniker));
	
	if (moniker->priv->parent != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (moniker->priv->parent, ev);

	if (parent != CORBA_OBJECT_NIL)
		moniker->priv->parent =
			bonobo_object_dup_ref (parent, ev);
	else
		moniker->priv->parent = CORBA_OBJECT_NIL;
}

/**
 * bonobo_moniker_get_parent:
 * @moniker: the moniker
 * @ev: a corba exception environment
 * 
 * See bonobo_moniker_set_parent;
 *
 * Return value: the parent of this moniker
 **/
Bonobo_Moniker
bonobo_moniker_get_parent (BonoboMoniker     *moniker,
			   CORBA_Environment *ev)
{
	g_return_val_if_fail (BONOBO_IS_MONIKER (moniker),
			      CORBA_OBJECT_NIL);
	
	if (moniker->priv->parent == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;
	
	return bonobo_object_dup_ref (moniker->priv->parent, ev);
}

/**
 * bonobo_moniker_get_name:
 * @moniker: the moniker
 * 
 * gets the unescaped name of the moniker less the prefix eg
 * file:/tmp/hash\#.gz returns /tmp/hash#.gz
 * 
 * Return value: the string
 **/
const char *
bonobo_moniker_get_name (BonoboMoniker *moniker)
{	
	g_return_val_if_fail (BONOBO_IS_MONIKER (moniker), "");

	if (moniker->priv->name)
		return & moniker->priv->name [
			moniker->priv->prefix_len];

	return "";
}

/**
 * bonobo_moniker_get_name_full:
 * @moniker: the moniker
 * 
 * gets the full unescaped display name of the moniker eg.
 * file:/tmp/hash\#.gz returns file:/tmp/hash#.gz
 * 
 * Return value: the string
 **/
const char *
bonobo_moniker_get_name_full (BonoboMoniker *moniker)
{	
	g_return_val_if_fail (BONOBO_IS_MONIKER (moniker), "");

	if (moniker->priv->name)
		return moniker->priv->name;

	return "";
}

/**
 * bonobo_moniker_get_name_escaped:
 * @moniker: a moniker
 * 
 * Get the full; escaped display name of the moniker eg.
 * file:/tmp/hash\#.gz returns file:/tmp/hash\#.gz
 * 
 * Return value: the string.
 **/
char *
bonobo_moniker_get_name_escaped (BonoboMoniker *moniker)
{
	return bonobo_moniker_util_escape (moniker->priv->name, 0);
}

/**
 * bonobo_moniker_set_name:
 * @moniker: the BonoboMoniker to configure.
 * @name: new name for this moniker.
 * @num_chars: number of characters in name to copy.
 *
 * This functions sets the moniker name in @moniker to be @name.
 */
void
bonobo_moniker_set_name (BonoboMoniker *moniker,
			 const char    *name,
			 int            num_chars)
{
	g_return_if_fail (BONOBO_IS_MONIKER (moniker));
	g_return_if_fail (strlen (name) >= moniker->priv->prefix_len);

	g_free (moniker->priv->name);
	moniker->priv->name = bonobo_moniker_util_unescape (
		name, num_chars);
}

static CORBA_char *
bonobo_moniker_default_get_display_name (BonoboMoniker     *moniker,
					 CORBA_Environment *ev)
{
	CORBA_char *ans, *parent_name;
	char       *tmp;
	
	parent_name = bonobo_moniker_util_get_parent_name (
		bonobo_object_corba_objref (BONOBO_OBJECT (moniker)), ev);

	if (BONOBO_EX (ev))
		return NULL;

	if (!parent_name)
		return CORBA_string_dup (moniker->priv->name);

	if (!moniker->priv->name)
		return parent_name;

	tmp = g_strdup_printf ("%s%s", parent_name, moniker->priv->name);

	if (parent_name)
		CORBA_free (parent_name);

	ans = CORBA_string_dup (tmp);

	g_free (tmp);
	
	return ans;
}

static Bonobo_Moniker
bonobo_moniker_default_parse_display_name (BonoboMoniker     *moniker,
					   Bonobo_Moniker     parent,
					   const CORBA_char  *name,
					   CORBA_Environment *ev)
{
	int i;
	
	g_return_val_if_fail (moniker != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (moniker->priv != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (moniker->priv->prefix != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (strlen (name) >= moniker->priv->prefix_len, CORBA_OBJECT_NIL);

	bonobo_moniker_set_parent (moniker, parent, ev);

	i = bonobo_moniker_util_seek_std_separator (name, moniker->priv->prefix_len);

	bonobo_moniker_set_name (moniker, name, i);

	return bonobo_moniker_util_new_from_name_full (
		bonobo_object_corba_objref (BONOBO_OBJECT (moniker)),
		&name [i], ev);	
}

static CORBA_char *
impl_get_display_name (PortableServer_Servant servant,
		       CORBA_Environment     *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);
	
	return CLASS (moniker)->get_display_name (moniker, ev);
}

static Bonobo_Moniker
impl_parse_display_name (PortableServer_Servant servant,
			 Bonobo_Moniker         parent,
			 const CORBA_char      *name,
			 CORBA_Environment     *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);

	return CLASS (moniker)->parse_display_name (moniker, parent, name, ev);
}

static Bonobo_Unknown
impl_resolve (PortableServer_Servant       servant,
	      const Bonobo_ResolveOptions *options,
	      const CORBA_char            *requested_interface,
	      CORBA_Environment           *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);
	Bonobo_Unknown retval;

	/* Try a standard resolve */
	retval = CLASS (moniker)->resolve (moniker, options,
					   requested_interface, ev);

	/* Try an extender */
	if (!BONOBO_EX (ev) && retval == CORBA_OBJECT_NIL) {
		Bonobo_Unknown extender;
		
		extender = bonobo_moniker_find_extender (
			moniker->priv->prefix,
			requested_interface, ev);
		
		if (BONOBO_EX (ev))
			return CORBA_OBJECT_NIL;

		else if (extender != CORBA_OBJECT_NIL) {
			retval = Bonobo_MonikerExtender_resolve (
				extender,
				bonobo_object_corba_objref (BONOBO_OBJECT (moniker)),
				options, moniker->priv->name,
				requested_interface, ev);

			bonobo_object_release_unref (extender, ev);
		}
	}

	if (!BONOBO_EX (ev) && retval == CORBA_OBJECT_NIL)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound,
				     NULL);

	return retval;
}

/**
 * bonobo_moniker_get_epv:
 *
 * Returns: The EPV for the default Bonobo Moniker implementation.
 */
POA_Bonobo_Moniker__epv *
bonobo_moniker_get_epv (void)
{
	POA_Bonobo_Moniker__epv *epv;

	epv = g_new0 (POA_Bonobo_Moniker__epv, 1);

	epv->_get_parent      = impl_get_parent;
	epv->_set_parent      = impl_set_parent;
	epv->getDisplayName   = impl_get_display_name;
	epv->parseDisplayName = impl_parse_display_name;
	epv->resolve          = impl_resolve;

	return epv;
}

static void
init_moniker_corba_class (void)
{
	/* The VEPV */
	bonobo_moniker_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_moniker_vepv.Bonobo_Moniker_epv = bonobo_moniker_get_epv ();
}

static void
bonobo_moniker_destroy (GtkObject *object)
{
	BonoboMoniker *moniker = BONOBO_MONIKER (object);

	if (moniker->priv->parent != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (moniker->priv->parent, NULL);

	g_free (moniker->priv->prefix);
	g_free (moniker->priv->name);
	g_free (moniker->priv);

	GTK_OBJECT_CLASS (bonobo_moniker_parent_class)->destroy (object);
}

static void
bonobo_moniker_init (GtkObject *object)
{
	BonoboMoniker *moniker = BONOBO_MONIKER (object);

	moniker->priv = g_new (BonoboMonikerPrivate, 1);

	moniker->priv->parent = CORBA_OBJECT_NIL;
	moniker->priv->name   = NULL;
}

static void
bonobo_moniker_class_init (BonoboMonikerClass *klass)
{
	GtkObjectClass *oclass = (GtkObjectClass *)klass;

	bonobo_moniker_parent_class = gtk_type_class (bonobo_object_get_type ());

	oclass->destroy = bonobo_moniker_destroy;

	klass->get_parent = bonobo_moniker_get_parent;
	klass->set_parent = bonobo_moniker_set_parent;
	klass->get_display_name = bonobo_moniker_default_get_display_name;
	klass->parse_display_name = bonobo_moniker_default_parse_display_name;

	init_moniker_corba_class ();
}

/**
 * bonobo_moniker_get_type:
 *
 * Returns: the GtkType for a BonoboMoniker.
 */
GtkType
bonobo_moniker_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMoniker",
			sizeof (BonoboMoniker),
			sizeof (BonoboMonikerClass),
			(GtkClassInitFunc) bonobo_moniker_class_init,
			(GtkObjectInitFunc) bonobo_moniker_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_moniker_corba_object_create:
 * @object: the GtkObject that will wrap the CORBA object
 *
 * Creates and activates the CORBA object that is wrapped by the
 * @object BonoboObject.
 *
 * Returns: An activated object reference to the created object
 * or %CORBA_OBJECT_NIL in case of failure.
 */
Bonobo_Moniker
bonobo_moniker_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_Moniker *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Moniker *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_moniker_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Moniker__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_moniker_construct:
 * @moniker: an un-constructed moniker object.
 * @corba_moniker: a CORBA handle inheriting from Bonobo::Moniker, or
 * CORBA_OBJECT_NIL, in which case a base Bonobo::Moniker is created.
 * @prefix: the prefix name of the moniker eg. 'file:', '!' or 'tar:'
 * 
 *  Constructs a newly created bonobo moniker with the given arguments.
 * 
 * Return value: the constructed moniker or NULL on failure.
 **/
BonoboMoniker *
bonobo_moniker_construct (BonoboMoniker *moniker,
			  Bonobo_Moniker corba_moniker,
			  const char    *prefix)
{
	BonoboMoniker *retval;

	if (!corba_moniker) {
		corba_moniker = bonobo_moniker_corba_object_create (
			BONOBO_OBJECT (moniker));

		if (corba_moniker == CORBA_OBJECT_NIL) {
			bonobo_object_unref (BONOBO_OBJECT (moniker));
			return NULL;
		}
	}

	if (prefix) {
		moniker->priv->prefix = g_strdup (prefix);
		moniker->priv->prefix_len = strlen (prefix);
	}

	retval = BONOBO_MONIKER (bonobo_object_construct (
		BONOBO_OBJECT (moniker), corba_moniker));

	return retval;
}

