/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-listener.c: Generic listener interface for callbacks.
 *
 * Authors:
 *	Alex Graveley (alex@helixcode.com)
 *	Mike Kestner  (mkestner@ameritech.net)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-listener.h>

static BonoboObjectClass *bonobo_listener_parent_class;
POA_Bonobo_Listener__vepv bonobo_listener_vepv;

struct _BonoboListenerPrivate {
	BonoboListenerCallbackFn event_callback;
	gpointer                 user_data;
};

enum SIGNALS {
	EVENT_NOTIFY,
	LAST_SIGNAL
};
static guint signals [LAST_SIGNAL] = { 0 };

static void
impl_Bonobo_Listener_event (PortableServer_Servant servant, 
			    const CORBA_char      *event_name, 
			    const CORBA_any       *args, 
			    CORBA_Environment     *ev)
{
	BonoboListener *listener;

	listener = BONOBO_LISTENER (bonobo_object_from_servant (servant));

	if (listener->priv->event_callback)
		listener->priv->event_callback (
			listener, (CORBA_char *) event_name, 
			(CORBA_any *) args, ev,
			listener->priv->user_data);

	gtk_signal_emit (GTK_OBJECT (listener),
			 signals [EVENT_NOTIFY],
			 event_name, args, ev);
}

/**
 * bonobo_listener_get_epv:
 *
 * Returns: The EPV for the default BonoboListener implementation.  
 */
POA_Bonobo_Listener__epv *
bonobo_listener_get_epv (void)
{
	POA_Bonobo_Listener__epv *epv;

	epv = g_new0 (POA_Bonobo_Listener__epv, 1);

	epv->event = impl_Bonobo_Listener_event;

	return epv;
}

static void
init_listener_corba_class (void)
{
	/* The VEPV */
	bonobo_listener_vepv.Bonobo_Unknown_epv  = bonobo_object_get_epv ();
	bonobo_listener_vepv.Bonobo_Listener_epv = bonobo_listener_get_epv ();
}

static void
bonobo_listener_destroy (GtkObject *object)
{
	GTK_OBJECT_CLASS (bonobo_listener_parent_class)->destroy (object);
}

static void
bonobo_listener_init (GtkObject *object)
{
	BonoboListener *listener;

	listener = BONOBO_LISTENER(object);
	listener->priv = g_new (BonoboListenerPrivate, 1);
	listener->priv->event_callback = NULL;
	listener->priv->user_data = NULL;
}

static void
bonobo_listener_class_init (BonoboListenerClass *klass)
{
	GtkObjectClass *oclass = (GtkObjectClass *)klass;

	bonobo_listener_parent_class = 
		gtk_type_class (bonobo_object_get_type ());

	oclass->destroy = bonobo_listener_destroy;

	signals [EVENT_NOTIFY] = gtk_signal_new (
		"event_notify", GTK_RUN_LAST, oclass->type,
		GTK_SIGNAL_OFFSET (BonoboListenerClass, event_notify),
		gtk_marshal_NONE__POINTER_POINTER_POINTER, GTK_TYPE_NONE, 3,
		GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);

	gtk_object_class_add_signals (oclass, signals, LAST_SIGNAL);

	init_listener_corba_class ();
}

/**
 * bonobo_listener_get_type:
 *
 * Returns: the GtkType for a BonoboListener.
 */
GtkType
bonobo_listener_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboListener",
			sizeof (BonoboListener),
			sizeof (BonoboListenerClass),
			(GtkClassInitFunc) bonobo_listener_class_init,
			(GtkObjectInitFunc) bonobo_listener_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_listener_corba_object_create:
 * @object: BonoboObject to initialize
 *
 * This is just a construction utility for BonoboListener objects.
 *
 * Returns: A Bonobo_Listener CORBA Object reference
 */
Bonobo_Listener
bonobo_listener_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_Listener *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Listener *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_listener_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Listener__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_listener_construct:
 * @listener: BonoboListener object.
 * @corba_listener: CORBA servant to be bound to @listener
 *
 * This method is used to allow subclassing of the BonoboListener
 * implementation
 *
 * Returns: NULL on failure;  or the constructed BonoboListener object
 */
BonoboListener *
bonobo_listener_construct (BonoboListener  *listener, 
			   Bonobo_Listener  corba_listener) 
{
        g_return_val_if_fail (listener != NULL, NULL);
        g_return_val_if_fail (BONOBO_IS_LISTENER (listener), NULL);
        g_return_val_if_fail (corba_listener != NULL, NULL);

        bonobo_object_construct (BONOBO_OBJECT (listener), 
				 corba_listener);

        return listener;
}

/**
 * bonobo_listener_new:
 * @event_callback: function to be invoked when an event is emitted by the EventSource.
 * @user_data: data passed to the functioned pointed by @event_call.
 *
 * Creates a generic event listener.  The listener calls the @event_callback 
 * function and emits an "event_notify" signal when notified of an event.  
 * The signal callback should be of the form:
 *
 * <informalexample>
 * <programlisting>
 *	void some_callback (GtkObject *, 
 *                          char *event_name, 
 *                          BonoboArg *event_data, 
 *                          CORBA_Environment *ev, 
 *                          gpointer user_data);
 * </programlisting>
 * </informalexample>
 *
 * You will typically pass the CORBA_Object reference in the return value
 * to an EventSource (by invoking EventSource::addListener).
 *
 * Returns: A BonoboListener object.
 */
BonoboListener*
bonobo_listener_new (BonoboListenerCallbackFn event_callback, 
		     gpointer                 user_data)
{
	BonoboListener *listener;
	Bonobo_Listener corba_listener;

	listener = gtk_type_new (BONOBO_LISTENER_TYPE);

	corba_listener = bonobo_listener_corba_object_create (
		BONOBO_OBJECT (listener));

	if (corba_listener == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (listener));
		return NULL;
	}
	
	listener->priv->event_callback = event_callback;
	listener->priv->user_data = user_data;

	return bonobo_listener_construct (listener, corba_listener);
}


/**
 * bonobo_event_make_name:
 * @idl_path: the IDL part of the event name.
 * @kind: the kind of the event
 * @subtype: an optional subtype
 *
 * Creates an event name. Event names consists of three part. The @idl_path is
 * mainly to create an unique namespace, and should identify the interface 
 * which triggered the event, for example "Bonobo/Property". The @kind denotes
 * what happened, for example "change". Finally you can use the optional 
 * @subtype to make events more specific. All three parts of the name are 
 * joined together separated by colons. "Bonobo/Property:change" or 
 * "Bonobo/Property:change:autosave" are examples of valid event names.
 *
 * Returns: A valid event_name, or NULL on error.
 */
char *
bonobo_event_make_name (const char *idl_path, 
			const char *kind,
			const char *subtype)
{
	g_return_val_if_fail (idl_path != NULL, NULL);
	g_return_val_if_fail (kind != NULL, NULL);
	g_return_val_if_fail (!strchr (idl_path, ':'), NULL);
	g_return_val_if_fail (!strchr (kind, ':'), NULL);
	g_return_val_if_fail (!subtype || !strchr (subtype, ':'), NULL);
	g_return_val_if_fail (strlen (idl_path), NULL);
	g_return_val_if_fail (strlen (kind), NULL);
	g_return_val_if_fail (!subtype || strlen (subtype), NULL);

	if (subtype)
		return g_strconcat (idl_path, ":", kind, ":", 
				    subtype, NULL);
	else
		return g_strconcat (idl_path, ":", kind, NULL);
}

static gboolean
bonobo_event_name_valid (const char *event_name)
{
	gint i = 0, c = 0, l = -1;

	g_return_val_if_fail (event_name != NULL, FALSE);
	g_return_val_if_fail (strlen (event_name), FALSE);

	if (event_name [0] == ':') 
		return FALSE;

	if (event_name [strlen (event_name) - 1] == ':') 
		return FALSE;

	while (event_name [i]) {
		if (event_name [i] == ':') {
			if (l == (i -1))
				return FALSE;
			l = i;
			c++;
		}
		i++;
	}

	if ((c == 1) || (c == 2)) 
		return TRUE;

	return FALSE;
}

static char *
bonobo_event_token (const char *event_name, gint pos)
{
	char **str_array, *res;

	if (!bonobo_event_name_valid (event_name))
		return NULL;

	str_array = g_strsplit (event_name, ":", 3);

	res = g_strdup (str_array [pos]);

	g_strfreev (str_array);

	return res;
}

/**
 * bonobo_event_type:
 * @event_name: the event name
 *
 * The event type consists of the first two parts of the event name, the idl_path
 * combined with the kind.
 *
 * Returns: The event type, or NULL on error.
 */
char *
bonobo_event_type (const char *event_name)
{
	gint i = 0, c = 0;
       
	if (!bonobo_event_name_valid (event_name))
		return NULL;

	while (event_name [i]) { 
		if (event_name [i] == ':') 
			c++;
		if (c == 2) 
			break;
		i++;
	}

	return g_strndup (event_name, i);
}

/**
 * bonobo_event_type:
 * @event_name: the event name
 *
 * Returns: The event subtype, or NULL on error.
 */
char *
bonobo_event_subtype (const char *event_name)
{
	return bonobo_event_token (event_name, 2);
}

/**
 * bonobo_event_kind:
 * @event_name: the event name
 *
 * Returns: The event kind, or NULL on error.
 */
char *
bonobo_event_kind (const char *event_name)
{
	return bonobo_event_token (event_name, 1);
}

/**
 * bonobo_event_idl_path:
 * @event_name: the event name
 *
 * Returns: The event idl path, or NULL on error.
 */
char *
bonobo_event_idl_path (const char *event_name)
{
	return bonobo_event_token (event_name, 0);
}
