/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-running-context.c: A global running interface
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>

#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-running-context.h>

POA_Bonobo_RunningContext__vepv bonobo_running_context_vepv;

typedef struct {
	gboolean    emitted_last_unref;
	GHashTable *objects;
	GHashTable *keys;
} BonoboRunningInfo;

BonoboRunningInfo *bonobo_running_info = NULL;
BonoboObject      *bonobo_running_context = NULL;
BonoboEventSource *bonobo_running_event_source = NULL;

enum {
	LAST_UNREF,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0 };

static void
key_free (gpointer name, gpointer dummy1, gpointer user_data)
{
	g_free (name);
}

static void
running_info_destroy (void)
{
	if (bonobo_running_info) {
		BonoboRunningInfo *ri = bonobo_running_info;

		if (ri->objects)
			g_hash_table_destroy (ri->objects);
		ri->objects = NULL;

		if (ri->keys) {
			g_hash_table_foreach_remove (
				ri->keys, (GHRFunc) key_free, NULL);
			g_hash_table_destroy (ri->keys);
			ri->keys = NULL;
		}
		g_free (ri);
	}
	bonobo_running_info = NULL;

	if (bonobo_running_context)
		bonobo_object_unref (BONOBO_OBJECT (bonobo_running_context));
	bonobo_running_context = NULL;
	bonobo_running_event_source = NULL;
}

static void
check_destroy (BonoboObject *object,
	       gpointer      dummy)
{
	bonobo_running_context = NULL;
	bonobo_running_event_source = NULL;
}

static BonoboRunningInfo *
get_running_info (gboolean create)
{
	if (!bonobo_running_info && create) {
		bonobo_running_info = g_new (BonoboRunningInfo, 1);
		bonobo_running_info->objects = g_hash_table_new (NULL, NULL);
		bonobo_running_info->keys    = g_hash_table_new (g_str_hash, g_str_equal);

		g_atexit (running_info_destroy);
	}

	return bonobo_running_info;
}

static void
check_empty (void)
{
	BonoboRunningInfo *ri = get_running_info (FALSE);

	if (!ri || !bonobo_running_context)
		return;

	if (!ri->emitted_last_unref &&
	    (g_hash_table_size (ri->objects) == 0) &&
	    (g_hash_table_size (ri->keys) == 0)) {

		ri->emitted_last_unref = TRUE;

		gtk_signal_emit (GTK_OBJECT (bonobo_running_context),
				 signals [LAST_UNREF]);

		g_return_if_fail (bonobo_running_event_source != NULL);

		bonobo_event_source_notify_listeners (
			bonobo_running_event_source,
			"bonobo:last_unref", NULL, NULL);
	}
}

void
bonobo_running_context_add_object (CORBA_Object object)
{
	BonoboRunningInfo *ri = get_running_info (TRUE);

	g_hash_table_insert (ri->objects, object, object);
}

void
bonobo_running_context_remove_object (CORBA_Object object)
{
	BonoboRunningInfo *ri = get_running_info (FALSE);

	if (ri) {
		g_hash_table_remove (ri->objects, object);

		check_empty ();
	}
}

void
bonobo_running_context_ignore_object (CORBA_Object object)
{
	BonoboRunningInfo *ri = get_running_info (FALSE);

	if (ri)
		g_hash_table_remove (ri->objects, object);
}

static void
impl_Bonobo_RunningContext_addObject (PortableServer_Servant servant,
				      const CORBA_Object     object,
				      CORBA_Environment     *ev)
{
	bonobo_running_context_add_object (object);
}

static void
impl_Bonobo_RunningContext_removeObject (PortableServer_Servant servant,
					 const CORBA_Object     object,
					 CORBA_Environment     *ev)
{
	bonobo_running_context_remove_object (object);
}

static void
impl_Bonobo_RunningContext_addKey (PortableServer_Servant servant,
				   const CORBA_char      *key,
				   CORBA_Environment     *ev)
{
	char              *key_copy, *old_key;
	BonoboRunningInfo *ri = get_running_info (TRUE);

	old_key = g_hash_table_lookup (ri->keys, key);
	if (old_key) {
		g_free (old_key);
		g_hash_table_remove (ri->keys, key);
	}
	key_copy = g_strdup (key);

	g_hash_table_insert (ri->keys, key_copy, key_copy);
}

static void
impl_Bonobo_RunningContext_removeKey (PortableServer_Servant servant,
				      const CORBA_char      *key,
				      CORBA_Environment     *ev)
{
	BonoboRunningInfo *ri = get_running_info (FALSE);
	char              *old_key;

	if (!ri)
		return;

	old_key = g_hash_table_lookup (ri->keys, key);
	if (old_key)
		g_free (old_key);
	g_hash_table_remove (ri->keys, key);

	check_empty ();
}

/**
 * bonobo_running_context_get_epv:
 *
 * Returns: The EPV for the default BonoboRunningContext implementation. 
 */
static POA_Bonobo_RunningContext__epv *
bonobo_running_context_get_epv (void)
{
	POA_Bonobo_RunningContext__epv *epv;

	epv = g_new0 (POA_Bonobo_RunningContext__epv, 1);

	epv->addObject    = impl_Bonobo_RunningContext_addObject;
	epv->removeObject = impl_Bonobo_RunningContext_removeObject;
	epv->addKey       = impl_Bonobo_RunningContext_addKey;
	epv->removeKey    = impl_Bonobo_RunningContext_removeKey;

	return epv;
}

static void
init_running_context_corba_class (void)
{
	/* The VEPV */
	bonobo_running_context_vepv.Bonobo_Unknown_epv        = bonobo_object_get_epv ();
	bonobo_running_context_vepv.Bonobo_RunningContext_epv = bonobo_running_context_get_epv ();
}

static void
bonobo_running_context_class_init (BonoboObjectClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	init_running_context_corba_class ();

	((BonoboRunningContextClass *)klass)->last_unref = NULL;

	signals [LAST_UNREF] = gtk_signal_new (
		"last_unref", GTK_RUN_FIRST, object_class->type,
		GTK_SIGNAL_OFFSET (BonoboRunningContextClass, last_unref),
		gtk_marshal_NONE__NONE, GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
}

static GtkType
bonobo_running_context_get_type (void)
{
        static GtkType type = 0;

        if (!type) {
                GtkTypeInfo info = {
                        "BonoboRunningContext",
                        sizeof (BonoboRunningContext),
                        sizeof (BonoboRunningContextClass),
                        (GtkClassInitFunc) bonobo_running_context_class_init,
                        (GtkObjectInitFunc) NULL,
                        NULL, /* reserved 1 */
                        NULL, /* reserved 2 */
                        (GtkClassInitFunc) NULL
                };

                type = gtk_type_unique (bonobo_object_get_type (), &info);
        }

        return type;
}

static Bonobo_RunningContext
bonobo_running_context_corba_object_create (BonoboObject *object)
{
        POA_Bonobo_RunningContext *servant;
        CORBA_Environment ev;

        servant = (POA_Bonobo_RunningContext *) g_new0 (BonoboObjectServant, 1);
        servant->vepv = &bonobo_running_context_vepv;

        CORBA_exception_init (&ev);

        POA_Bonobo_RunningContext__init ((PortableServer_Servant) servant, &ev);
        if (BONOBO_EX (&ev)) {
                g_free (servant);
                CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

        CORBA_exception_free (&ev);

        return bonobo_object_activate_servant (object, servant);
}

BonoboObject *
bonobo_running_context_new (void)
{
	BonoboObject *object;
	Bonobo_RunningContext corba_running_context;

	if (bonobo_running_context) {
		bonobo_object_ref (bonobo_running_context);
		return bonobo_running_context;
	}

	object = gtk_type_new (bonobo_running_context_get_type ());

	corba_running_context =
		bonobo_running_context_corba_object_create (object);

	if (corba_running_context == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (object));
		return NULL;
	}

        bonobo_running_context =
		bonobo_object_construct (object, corba_running_context);

	bonobo_running_event_source = bonobo_event_source_new ();
	bonobo_running_context_ignore_object (
		bonobo_object_corba_objref (BONOBO_OBJECT (
			bonobo_running_event_source)));
	bonobo_event_source_ignore_listeners (bonobo_running_event_source);

	bonobo_object_add_interface (BONOBO_OBJECT (bonobo_running_context),
				     BONOBO_OBJECT (bonobo_running_event_source));

	gtk_signal_connect (GTK_OBJECT (bonobo_running_context), "destroy",
			    (GtkSignalFunc) check_destroy, NULL);

	return bonobo_running_context;
}

BonoboObject *
bonobo_context_running_get (void)
{
	return bonobo_running_context_new ();
}
