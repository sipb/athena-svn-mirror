/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-event-source.c: Generic event emitter.
 *
 * Author:
 *	Alex Graveley (alex@helixcode.com)
 *	Iain Holmes   (iain@helixcode.com)
 *      docs, Miguel de Icaza (miguel@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-running-context.h>
#include <time.h>

static BonoboObjectClass    *bonobo_event_source_parent_class;
POA_Bonobo_EventSource__vepv bonobo_event_source_vepv;


struct _BonoboEventSourcePrivate {
	GSList  *listeners;  /* CONTAINS: ListenerDesc* */
	gboolean ignore;
	gint     counter;    /* to create unique listener Ids */
};

typedef struct {
	Bonobo_Listener listener;
	Bonobo_EventSource_ListenerId id;
	CORBA_char *event_mask; /* send all events if NULL */
} ListenerDesc;

/*
 * tries to make a unique connection Id. Adding the time make an
 * accidental remove of a listener more unlikely.
 */
static Bonobo_EventSource_ListenerId
create_listener_id (BonoboEventSource *source)
{
	guint32 id;

	source->priv->counter = source->priv->counter++ & 0x0000ffff;

	if (!source->priv->counter) source->priv->counter++;

	id = source->priv->counter | (time(NULL) << 16);

	return id;
}

static inline BonoboEventSource * 
bonobo_event_source_from_servant (PortableServer_Servant servant)
{
	return BONOBO_EVENT_SOURCE (bonobo_object_from_servant (servant));
}

static void
desc_free (ListenerDesc *desc, CORBA_Environment *ev)
{
	if (desc) {
		CORBA_free (desc->event_mask);
		bonobo_object_release_unref (desc->listener, ev);
		g_free (desc);
	}
}

static Bonobo_EventSource_ListenerId
impl_Bonobo_EventSource_addListenerWithMask (PortableServer_Servant servant,
					     const Bonobo_Listener  l,
					     const CORBA_char      *event_mask,
					     CORBA_Environment     *ev)
{
	BonoboEventSource *event_source;
	CORBA_char        *mask_copy = NULL;
	ListenerDesc      *desc;

	g_return_val_if_fail (!CORBA_Object_is_nil (l, ev), 0);

	event_source = bonobo_event_source_from_servant (servant);

	if (event_mask)
		mask_copy = CORBA_string_dup (event_mask);

	if (event_source->priv->ignore) /* Hook for running context */
		bonobo_running_context_ignore_object (l);

	desc = g_new0 (ListenerDesc, 1);
	desc->listener = bonobo_object_dup_ref (l, ev);
	desc->id = create_listener_id (event_source);
	desc->event_mask = mask_copy;

	event_source->priv->listeners = g_slist_prepend (event_source->priv->listeners, desc);

	return desc->id;
}

static Bonobo_EventSource_ListenerId
impl_Bonobo_EventSource_addListener (PortableServer_Servant servant,
				     const Bonobo_Listener  l,
				     CORBA_Environment     *ev)
{
	return impl_Bonobo_EventSource_addListenerWithMask (servant, l, NULL, ev);
}

static void
impl_Bonobo_EventSource_removeListener (PortableServer_Servant servant,
					const Bonobo_EventSource_ListenerId id,
					CORBA_Environment     *ev)
{
	BonoboEventSource *event_source;
	GSList *list;

	event_source = bonobo_event_source_from_servant (servant);

	for (list = event_source->priv->listeners; list; list = list->next) {
		ListenerDesc *desc = (ListenerDesc *) list->data;

		if (desc->id == id) {
			event_source->priv->listeners = 
				g_slist_remove (event_source->priv->listeners,
						desc);
			desc_free (desc, ev);
			return;
		}
	}

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_Bonobo_EventSource_UnknownListener, 
			     NULL);
}

/**
 * bonobo_event_source_notify_listeners:
 * @event_source: the Event Source that will emit the event.
 * @event_name: Name of the event being emmited
 * @value: A CORBA_any value that contains the data that is passed to interested clients
 * @opt_ev: A CORBA_Environment where a failure code can be returned, can be NULL.
 *
 * This will notify all clients that have registered with this EventSource
 * (through the addListener or addListenerWithMask methods) of the availability
 * of the event named @event_name.  The @value CORBA::any value is passed to
 * all listeners.
 *
 * @event_name can not contain comma separators, as commas are used to
 * separate the various event names. 
 */
void
bonobo_event_source_notify_listeners (BonoboEventSource *event_source,
				      const char        *event_name,
				      const CORBA_any   *value,
				      CORBA_Environment *opt_ev)
{
	GSList *list;
	CORBA_Environment ev, *my_ev;

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	for (list = event_source->priv->listeners; list; list = list->next) {
		ListenerDesc *desc = (ListenerDesc *) list->data;

		if (desc->event_mask == NULL || 
		    strstr (desc->event_mask, event_name))
			Bonobo_Listener_event (desc->listener, 
					       event_name, value, my_ev);
	}
	
	if (!opt_ev)
		CORBA_exception_free (&ev);
}

void
bonobo_event_source_notify_listeners_full (BonoboEventSource *event_source,
					   const char        *path,
					   const char        *type,
					   const char        *subtype,
					   const CORBA_any   *value,                          
					   CORBA_Environment *opt_ev)
{
	char *event_name;

	event_name = bonobo_event_make_name (path, type, subtype);

	bonobo_event_source_notify_listeners (event_source, event_name,
					      value, opt_ev);

	g_free (event_name);
}


/**
 * bonobo_event_source_get_epv:
 *
 * Returns: The EPV for the default BonoboEventSource implementation.  
 */
POA_Bonobo_EventSource__epv *
bonobo_event_source_get_epv (void)
{
	POA_Bonobo_EventSource__epv *epv;

	epv = g_new0 (POA_Bonobo_EventSource__epv, 1);

	epv->addListener         = impl_Bonobo_EventSource_addListener;
	epv->addListenerWithMask = impl_Bonobo_EventSource_addListenerWithMask;
	epv->removeListener      = impl_Bonobo_EventSource_removeListener;

	return epv;
}

static void
init_event_source_corba_class (void)
{
	/* The VEPV */
	bonobo_event_source_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_event_source_vepv.Bonobo_EventSource_epv = bonobo_event_source_get_epv ();
}

static void
bonobo_event_source_destroy (GtkObject *object)
{
	BonoboEventSource *event_source;
	GSList            *l;
	CORBA_Environment  ev;
	
	event_source = BONOBO_EVENT_SOURCE (object);

	CORBA_exception_init (&ev);

	for (l = event_source->priv->listeners; l; l = l->next)
		desc_free (l->data, &ev);

	CORBA_exception_free (&ev);

	g_slist_free (event_source->priv->listeners);
	g_free (event_source->priv);

	GTK_OBJECT_CLASS (bonobo_event_source_parent_class)->destroy (object);
}

static void
bonobo_event_source_init (GtkObject *object)
{
	BonoboEventSource *event_source;

	event_source = BONOBO_EVENT_SOURCE (object);
	event_source->priv = g_new (BonoboEventSourcePrivate, 1);
	event_source->priv->listeners = NULL;
}

static void
bonobo_event_source_class_init (BonoboEventSourceClass *klass)
{
	GtkObjectClass *oclass = (GtkObjectClass *) klass;

	bonobo_event_source_parent_class = 
		gtk_type_class (bonobo_object_get_type ());

	oclass->destroy = bonobo_event_source_destroy;

	init_event_source_corba_class ();
}

/**
 * bonobo_event_source_get_type:
 * 
 * Registers the GtkType for this BonoboObject.
 * 
 * Return value: the type.
 **/
GtkType
bonobo_event_source_get_type (void)
{
        static GtkType type = 0;

        if (!type) {
                GtkTypeInfo info = {
                        "BonoboEventSource",
                        sizeof (BonoboEventSource),
                        sizeof (BonoboEventSourceClass),
                        (GtkClassInitFunc) bonobo_event_source_class_init,
                        (GtkObjectInitFunc) bonobo_event_source_init,
                        NULL, /* reserved 1 */
                        NULL, /* reserved 2 */
                        (GtkClassInitFunc) NULL
                };

                type = gtk_type_unique (bonobo_object_get_type (), &info);
        }

        return type;
}

/**
 * bonobo_event_source_corba_object_create:
 * @object: the object to tie the CORBA object to
 * 
 * creates the CORBA object associated with an event source
 * 
 * Return value: the CORBA handle to this object.
 **/
Bonobo_EventSource
bonobo_event_source_corba_object_create (BonoboObject *object)
{
        POA_Bonobo_EventSource *servant;
        CORBA_Environment ev;

        servant = (POA_Bonobo_EventSource *) g_new0 (BonoboObjectServant, 1);
        servant->vepv = &bonobo_event_source_vepv;

        CORBA_exception_init (&ev);

        POA_Bonobo_EventSource__init ((PortableServer_Servant) servant, &ev);
        if (BONOBO_EX (&ev)) {
                g_free (servant);
                CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

        CORBA_exception_free (&ev);

        return bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_event_source_construct:
 * @event_source: 
 * @corba_event_source: 
 * 
 * constructs an event source.
 * 
 * Return value: the constructed event source or NULL on error.
 **/
BonoboEventSource *
bonobo_event_source_construct (BonoboEventSource  *event_source, 
			       Bonobo_EventSource corba_event_source) 
{
        g_return_val_if_fail (event_source != NULL, NULL);
        g_return_val_if_fail (BONOBO_IS_EVENT_SOURCE (event_source), NULL);
        g_return_val_if_fail (corba_event_source != NULL, NULL);

        bonobo_object_construct (BONOBO_OBJECT (event_source), 
				 corba_event_source);

        return event_source;
}

/**
 * bonobo_event_source_new:
 *
 * Creates a new BonoboEventSource object.  Typically this
 * object will be exposed to clients through CORBA and they
 * will register and unregister functions to be notified
 * of events that this EventSource generates.
 * 
 * To notify clients of an event, use the bonobo_event_source_notify_listeners()
 * function.
 *
 * Returns: A new #BonoboEventSource server object.
 */
BonoboEventSource *
bonobo_event_source_new (void)
{
	BonoboEventSource *event_source;
	Bonobo_EventSource corba_event_source;
	
	event_source = gtk_type_new (BONOBO_EVENT_SOURCE_TYPE);
	corba_event_source = bonobo_event_source_corba_object_create (
		BONOBO_OBJECT (event_source));
	
	if (corba_event_source == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (event_source));
		return NULL;
	}
	
	return bonobo_event_source_construct (
		event_source, corba_event_source);
}

/**
 * bonobo_event_source_ignore_listeners:
 * @event_source: 
 * 
 *  Instructs the event source to de-register any listeners
 * that are added from the global running context.
 **/
void
bonobo_event_source_ignore_listeners (BonoboEventSource *event_source)
{
	g_return_if_fail (BONOBO_IS_EVENT_SOURCE (event_source));

	event_source->priv->ignore = TRUE;
}
