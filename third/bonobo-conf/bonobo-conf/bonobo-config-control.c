/**
 * bonobo-config-control.c: Property control implementation.
 *
 * Author:
 *      based ob bonobo-property-control.c by Iain Holmes <iain@helixcode.com>
 *      modified by Dietmar Maurer <dietmar@ximian.com>
 *
 * Copyright 2001, Ximian, Inc.
 */
#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-ui-container.h>
#include <bonobo-conf/bonobo-property-bag-editor.h>
#include <bonobo-conf/bonobo-property-bag-proxy.h>

#include "bonobo-config-control.h"

typedef struct {
	char *name;
	Bonobo_PropertyBag pb;
	BonoboPBProxy *proxy;
	BonoboConfigControlGetControlFn get_fn;
	gpointer closure;
} PageData;

struct _BonoboConfigControlPrivate {
	GList *page_list;
};

enum {
	ACTION,
	LAST_SIGNAL
};

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static GtkObjectClass *parent_class;

static guint32 signals[LAST_SIGNAL] = { 0 };

#define CC_FROM_SERVANT(servant) \
BONOBO_CONFIG_CONTROL (bonobo_object_from_servant (servant))

static void
config_control_get_prop (BonoboPropertyBag *bag,
			 BonoboArg         *arg,
			 guint              arg_id,
			 CORBA_Environment *ev,
			 gpointer           user_data)
{
	if (arg_id == 0 && arg->_type == BONOBO_ARG_STRING) {
 		BONOBO_ARG_SET_STRING (arg, (char *)user_data);
	}
}

static CORBA_long
impl_Bonobo_PropertyControl__get_pageCount (PortableServer_Servant servant,
					    CORBA_Environment *ev)
{
	BonoboConfigControl *config_control = CC_FROM_SERVANT (servant);

	return g_list_length (config_control->priv->page_list);
}
       
static void
value_modified_cb (BonoboPBProxy *proxy,
		   gpointer       ud)
{
	BonoboConfigControl *config_control = BONOBO_CONFIG_CONTROL (ud);

	bonobo_config_control_changed (config_control, NULL);
}

static Bonobo_Control
impl_Bonobo_PropertyControl_getControl (PortableServer_Servant servant,
					CORBA_long pagenumber,
					CORBA_Environment *ev)
{
	BonoboConfigControl *config_control = CC_FROM_SERVANT (servant);
	BonoboPropertyBag *property_bag;
	BonoboControl *control;
	BonoboUIContainer *uic;
	GtkWidget *w;
	PageData *pd;
	GList *l;

	l =  g_list_nth (config_control->priv->page_list, pagenumber);

	if (!l || !l->data) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PropertyControl_NoPage, NULL);
		return CORBA_OBJECT_NIL;
	}

	pd = (PageData *)l->data;

	if (!pd->get_fn) {
		
		uic = bonobo_ui_container_new ();

		control = bonobo_property_bag_editor_new 
			(BONOBO_OBJREF (pd->proxy), BONOBO_OBJREF(uic), ev);

		bonobo_object_unref (BONOBO_OBJECT (uic));
	} else {

		w = pd->get_fn (config_control, BONOBO_OBJREF (pd->proxy), 
				pd->closure, ev);
		if (BONOBO_EX (ev) || !w)
			return CORBA_OBJECT_NIL;
	
		control = bonobo_control_new (w);
	}

	gtk_signal_connect (GTK_OBJECT (pd->proxy), "modified",  
			    GTK_SIGNAL_FUNC (value_modified_cb), 
			    config_control);
	
	/* fixme: this is the only way to set at title - very bad */ 

	property_bag = bonobo_property_bag_new (config_control_get_prop,
						NULL, pd->name);

	bonobo_property_bag_add (property_bag, "bonobo:title", 0, 
				 BONOBO_ARG_STRING, NULL, NULL, 
				 BONOBO_PROPERTY_READABLE);

	bonobo_object_add_interface (BONOBO_OBJECT (control),
				     BONOBO_OBJECT (property_bag));

	return (Bonobo_Control) CORBA_Object_duplicate 
		(BONOBO_OBJREF (control), ev);
}

static void
impl_Bonobo_PropertyControl_notifyAction (PortableServer_Servant  servant,
					  CORBA_long              pagenumber,
					  Bonobo_PropertyControl_Action action,
					  CORBA_Environment      *ev)
{
	BonoboConfigControl *config_control = CC_FROM_SERVANT (servant);
	GList *l;
	PageData *pd;

	if (pagenumber < 0 || 
	    pagenumber >= g_list_length (config_control->priv->page_list)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PropertyControl_NoPage, NULL);
		return;
	}

	if (action == Bonobo_PropertyControl_APPLY) {
		l = config_control->priv->page_list;
		
		while (l) {
			pd = (PageData *)l->data;

			bonobo_pbproxy_update (pd->proxy);
			l = l->next;
		}
	}
	
	gtk_signal_emit (GTK_OBJECT (config_control), signals [ACTION], 
			 pagenumber, action);
}

static void
bonobo_config_control_destroy (GtkObject *object)
{
	BonoboConfigControl *config_control;
	GList *l;
	PageData *pd;

	config_control = BONOBO_CONFIG_CONTROL (object);
	
	if (config_control->priv != NULL) {

		l = config_control->priv->page_list;

		while (l) {
			pd = (PageData *)l->data;

			g_free (pd->name);
			bonobo_object_release_unref (pd->pb, NULL);

			l = l->next;
		}

		g_list_free (config_control->priv->page_list);
		
		g_free (config_control->priv);
		config_control->priv = NULL;
	}

	parent_class->destroy (object);
}

static void
bonobo_config_control_class_init (BonoboConfigControlClass *klass)
{
	GtkObjectClass *object_class;
	POA_Bonobo_PropertyControl__epv *epv = &klass->epv;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = bonobo_config_control_destroy;

	signals [ACTION] = 
		gtk_signal_new ("action",
				GTK_RUN_FIRST, object_class->type,
				GTK_SIGNAL_OFFSET (BonoboConfigControlClass, 
						   action),
				gtk_marshal_NONE__INT_INT, GTK_TYPE_NONE,
				2, GTK_TYPE_INT, GTK_TYPE_ENUM);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
				     
	parent_class = gtk_type_class (PARENT_TYPE);

	epv->_get_pageCount = impl_Bonobo_PropertyControl__get_pageCount;
	epv->getControl     = impl_Bonobo_PropertyControl_getControl;
	epv->notifyAction   = impl_Bonobo_PropertyControl_notifyAction;
}

static void
bonobo_config_control_init (BonoboConfigControl *config_control)
{
	config_control->priv = g_new0 (BonoboConfigControlPrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboConfigControl, 
			 Bonobo_PropertyControl,
			 PARENT_TYPE,
			 bonobo_config_control);

/**
 * bonobo_config_control_new:
 * @opt_event_source: An optional event source to use to emit events on.
 *
 * Creates a BonoboConfigControl object.
 *
 * Returns: A pointer to a newly created BonoboConfigControl object.
 */
BonoboConfigControl *
bonobo_config_control_new (BonoboEventSource *opt_event_source)
{
	BonoboConfigControl *config_control;

	config_control = gtk_type_new (bonobo_config_control_get_type ());
	
	if (opt_event_source != NULL) {
		bonobo_object_ref (BONOBO_OBJECT (opt_event_source));
		config_control->event_source = opt_event_source;
	} else
		config_control->event_source = bonobo_event_source_new ();
	
	bonobo_object_add_interface (BONOBO_OBJECT (config_control),
	        BONOBO_OBJECT (config_control->event_source));
			
	return config_control;
}

/**
 * bonobo_config_control_changed:
 * @config_control: The BonoboConfigControl that has changed.
 * @opt_ev: An optional CORBA_Environment for exception handling. 
 *
 * Tells the server that a value in the config control has been changed,
 * and that it should indicate this somehow.
 */
void
bonobo_config_control_changed (BonoboConfigControl *config_control,
			       CORBA_Environment   *opt_ev)
{
	CORBA_Environment ev;
	CORBA_any any;
	CORBA_short s;

	g_return_if_fail (config_control != NULL);
	g_return_if_fail (BONOBO_IS_CONFIG_CONTROL (config_control));

	if (opt_ev == NULL)
		CORBA_exception_init (&ev);
	else
		ev = *opt_ev;

	/* fixme: whats that? */
	s = 0;
	any._type = (CORBA_TypeCode) TC_short;
	any._value = &s;

	bonobo_event_source_notify_listeners (config_control->event_source,
					      BONOBO_PROPERTY_CONTROL_CHANGED,
					      &any, &ev);

	if (opt_ev == NULL && BONOBO_EX (&ev)) {
		g_warning ("ERROR: %s", CORBA_exception_id (&ev));
	}

	if (opt_ev == NULL)
		CORBA_exception_free (&ev);
}

void
bonobo_config_control_add_page (BonoboConfigControl             *cc,
				const char                      *name,
				Bonobo_PropertyBag               pb,
				BonoboConfigControlGetControlFn  opt_get_fn,
				gpointer                         closure)
{
	PageData *pd;

	g_return_if_fail (cc != NULL);
	g_return_if_fail (BONOBO_IS_CONFIG_CONTROL (cc));
	g_return_if_fail (name != NULL);
	g_return_if_fail (pb != CORBA_OBJECT_NIL);

	pd = g_new0 (PageData, 1);

	pd->name = g_strdup (name);

	bonobo_object_dup_ref (pb, NULL);
	pd->pb = pb;

	pd->proxy = bonobo_pbproxy_new ();
	bonobo_pbproxy_set_bag (pd->proxy, pb);

	pd->get_fn = opt_get_fn;
	pd->closure = closure;

	cc->priv->page_list = g_list_append (cc->priv->page_list, pd);
}

