/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-widget.c: BonoboWidget object.
 *
 * Authors:
 *   Nat Friedman    (nat@ximian.com)
 *
 * Copyright 1999, 2001 Ximian, Inc.
 * 
 * Bonobo component embedding for hydrocephalic imbeciles.
 *
 * Pure cane sugar.
 *
 * This purpose of BonoboWidget is to make container-side use of
 * Bonobo as easy as pie.  This widget has one function:
 *
 *      Provide a simple wrapper for embedding Controls.  Embedding
 *      controls is already really easy, but BonoboWidget reduces
 *      the work from about 5 lines to 1.  To embed a given control,
 *      just do:
 *
 *        bw = bonobo_widget_new_control ("moniker for control");
 *        gtk_container_add (some_container, bw);
 *
 *      Ta da!
 *
 *      NB. A simple moniker might look like 'file:/tmp/a.jpeg' or
 *      OAFIID:GNOME_Evolution_Calendar_Control
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-widget.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <libgnome/gnome-macros.h>

GNOME_CLASS_BOILERPLATE (BonoboWidget, bonobo_widget,
			 GObject, GTK_TYPE_BIN);

struct _BonoboWidgetPrivate {
	/* Control stuff. */
	BonoboControlFrame *frame;
};

static Bonobo_Unknown
bonobo_widget_launch_component (const char        *moniker,
				const char        *if_name,
				CORBA_Environment *ev)
{
	Bonobo_Unknown component;

	component = bonobo_get_object (moniker, if_name, ev);

	if (BONOBO_EX (ev))
		component = CORBA_OBJECT_NIL;

	if (component == CORBA_OBJECT_NIL)
		return NULL;

	return component;
}


/*
 *
 * Control support for BonoboWidget.
 *
 */
/**
 * bonobo_widget_construct_control_from_objref:
 * @bw: A BonoboWidget to construct
 * @control: A CORBA Object reference to an IDL:Bonobo/Control:1.0
 * @uic: Bonobo_UIContainer for the launched object or CORBA_OBJECT_NIL
 * if there is no need of menu / toolbar merging.
 * @ev: a CORBA exception environment
 *
 * This is a constructor function.  Only usable for wrapping and
 * derivation of new objects.  For normal use, please refer to
 * #bonobo_widget_new_control_from_objref.
 *
 * Returns: A #BonoboWidget (the @bw)
 */
BonoboWidget *
bonobo_widget_construct_control_from_objref (BonoboWidget      *bw,
					     Bonobo_Control     control,
					     Bonobo_UIContainer uic,
					     CORBA_Environment *ev)
{
	GtkWidget *frame_widget;

	/* Create a local ControlFrame for it. */
	bw->priv->frame = bonobo_control_frame_new (uic);

	bonobo_control_frame_bind_to_control (
		bw->priv->frame, control, ev);

	/* Grab the actual widget which visually contains the remote
	 * Control.  This is a GtkSocket, in reality. */
	frame_widget = bonobo_control_frame_get_widget (bw->priv->frame);

	/* Now stick it into this BonoboWidget. */
	gtk_container_add (GTK_CONTAINER (bw), frame_widget);
	gtk_widget_show (frame_widget);

	return bw;
}

/**
 * bonobo_widget_construct_control:
 * @bw: A BonoboWidget to construct
 * @moniker: A Moniker describing the object to be activated 
 * @uic: Bonobo_UIContainer for the launched object or CORBA_OBJECT_NIL
 * if there is no need of menu / toolbar merging.
 * @ev: a CORBA exception environment
 *
 * This is a constructor function.  Only usable for wrapping and
 * derivation of new objects.  For normal use, please refer to
 * #bonobo_widget_new_control.
 *
 * This function will unref the passed in @bw in case it cannot launch
 * the component and return %NULL in such a case.  Otherwise it returns
 * the @bw itself.
 *
 * Returns: A #BonoboWidget or %NULL
 */
BonoboWidget *
bonobo_widget_construct_control (BonoboWidget      *bw,
				 const char        *moniker,
				 Bonobo_UIContainer uic,
				 CORBA_Environment *ev)
{
	BonoboWidget  *widget;
	Bonobo_Control control;

	/* Create the remote Control object. */
	control = bonobo_widget_launch_component (
		moniker, "IDL:Bonobo/Control:1.0", ev);
	if (BONOBO_EX (ev) || control == CORBA_OBJECT_NIL) {
		/* Kill it (it is a floating object) */
		gtk_object_sink (GTK_OBJECT (bw));
		
		return NULL;
	}

	widget = bonobo_widget_construct_control_from_objref (
		bw, control, uic, ev);

	bonobo_object_release_unref (control, ev);

	return widget;
}

typedef struct {
	BonoboWidget       *bw;
	BonoboWidgetAsyncFn fn;
	gpointer            user_data;
	Bonobo_UIContainer  uic;
} async_closure_t;

static void
control_new_async_cb (Bonobo_Unknown     object,
		      CORBA_Environment *ev,
		      gpointer           user_data)
{
	async_closure_t *c = user_data;

	if (BONOBO_EX (ev) || object == CORBA_OBJECT_NIL)
		c->fn (NULL, ev, c->user_data);
	else {
		bonobo_widget_construct_control_from_objref (
			c->bw, object, c->uic, ev);
		c->fn (c->bw, ev, c->user_data);
	}

	g_object_unref (c->bw);
	bonobo_object_release_unref (c->uic, ev);
	g_free (c);
}

/**
 * bonobo_widget_new_control_async:
 * @moniker: A Moniker describing the object to be activated 
 * @uic: Bonobo_UIContainer for the launched object or CORBA_OBJECT_NIL
 * if there is no need of menu / toolbar merging.
 * @fn: a callback function called when the activation has happend
 * @user_data: user data to be passed back to the callback.
 * 
 * This method creates a widget, returns it to the user, and
 * asynchronously activates a control to insert into the widget.
 * 
 * Return value: a (temporarily) empty Widget to be filled with the
 * control later
 **/
GtkWidget *
bonobo_widget_new_control_async (const char         *moniker,
				 Bonobo_UIContainer  uic,
				 BonoboWidgetAsyncFn fn,
				 gpointer            user_data)
{
	BonoboWidget     *bw;
	async_closure_t  *c = g_new0 (async_closure_t, 1);
	CORBA_Environment ev;

	g_return_val_if_fail (fn != NULL, NULL);
	g_return_val_if_fail (moniker != NULL, NULL);

	bw = g_object_new (BONOBO_TYPE_WIDGET, NULL);

	CORBA_exception_init (&ev);

	c->bw = g_object_ref (bw);
	c->fn = fn;
	c->user_data = user_data;
	c->uic = bonobo_object_dup_ref (uic, &ev);

	bonobo_get_object_async (
		moniker, "IDL:Bonobo/Control:1.0", &ev,
		control_new_async_cb, c);

	if (BONOBO_EX (&ev)) {
		control_new_async_cb (CORBA_OBJECT_NIL, &ev, c);
		gtk_widget_destroy (GTK_WIDGET (bw));
		bw = NULL;
	}

	CORBA_exception_free (&ev);

	return (GtkWidget *) bw;
}

/**
 * bonobo_widget_new_control_from_objref:
 * @control: A CORBA Object reference to an IDL:Bonobo/Control:1.0
 * @uic: Bonobo_UIContainer for the launched object or CORBA_OBJECT_NIL
 * if there is no need of menu / toolbar merging.
 *
 * This function is a simple wrapper for easily embedding controls
 * into applications.  This function is used when you have already
 * a CORBA object reference to an IDL:Bonobo/Control:1.0 (the
 * @control) argument. 
 *
 * Returns: the @control wrapped as a #GtkWidget.
 */
GtkWidget *
bonobo_widget_new_control_from_objref (Bonobo_Control     control,
				       Bonobo_UIContainer uic)
{
	BonoboWidget     *bw;
	CORBA_Environment ev;

	g_return_val_if_fail (control != CORBA_OBJECT_NIL, NULL);

	CORBA_exception_init (&ev);

	bw = g_object_new (BONOBO_TYPE_WIDGET, NULL);

	bw = bonobo_widget_construct_control_from_objref (bw, control, uic, &ev);

	if (BONOBO_EX (&ev))
		bw = NULL;

	CORBA_exception_free (&ev);

	return (GtkWidget *) bw;
}

/**
 * bonobo_widget_new_control:
 * @moniker: A Moniker describing the object to be activated 
 * @uic: Bonobo_UIContainer for the launched object or CORBA_OBJECT_NIL
 * if there is no need of menu / toolbar merging.
 *
 * This function is a simple wrapper for easily embedding controls
 * into applications.  It will launch the component identified by @id
 * and will return it as a GtkWidget.
 *
 * Returns: A #GtkWidget that is bound to the Bonobo Control. 
 */
GtkWidget *
bonobo_widget_new_control (const char        *moniker,
			   Bonobo_UIContainer uic)
{
	BonoboWidget *bw;
	CORBA_Environment ev;

	g_return_val_if_fail (moniker != NULL, NULL);

	CORBA_exception_init (&ev);

	bw = g_object_new (BONOBO_TYPE_WIDGET, NULL);

	bw = bonobo_widget_construct_control (bw, moniker, uic, &ev);

	if (BONOBO_EX (&ev)) {
		char *txt;
		g_warning ("Activation exception '%s'",
			   (txt = bonobo_exception_get_text (&ev)));
		g_free (txt);
		bw = NULL;
	}

	CORBA_exception_free (&ev);

	return (GtkWidget *) bw;
}

/**
 * bonobo_widget_get_control_frame:
 * @bonobo_widget: a Bonobo Widget returned by one of the bonobo_widget_new() functions.
 *
 * Every IDL:Bonobo/Control:1.0 needs to be placed inside an
 * IDL:Bonobo/ControlFrame:1.0.  This returns the BonoboControlFrame
 * object that wraps the Control in the @bonobo_widget. 
 * 
 * Returns: The BonoboControlFrame associated with the @bonobo_widget
 */
BonoboControlFrame *
bonobo_widget_get_control_frame (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	return bonobo_widget->priv->frame;
}

/**
 * bonobo_widget_get_ui_container:
 * @bonobo_widget: the #BonoboWidget to query.
 *
 * Returns: the CORBA object reference to the Bonobo_UIContainer
 * associated with the @bonobo_widget.
 */
Bonobo_UIContainer
bonobo_widget_get_ui_container (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	if (!bonobo_widget->priv->frame)
		return CORBA_OBJECT_NIL;

	return bonobo_control_frame_get_ui_container (
		bonobo_widget->priv->frame);
}

Bonobo_Unknown
bonobo_widget_get_objref (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	if (!bonobo_widget->priv->frame)
		return CORBA_OBJECT_NIL;
	else
		return bonobo_control_frame_get_control (bonobo_widget->priv->frame);
}

static void
bonobo_widget_dispose (GObject *object)
{
	BonoboWidget *bw = BONOBO_WIDGET (object);
	BonoboWidgetPrivate *priv = bw->priv;
	
	if (priv->frame) {
		bonobo_object_unref (BONOBO_OBJECT (priv->frame));
		priv->frame = NULL;
	}

	parent_class->dispose (object);
}

static void
bonobo_widget_finalize (GObject *object)
{
	BonoboWidget *bw = BONOBO_WIDGET (object);
	
	g_free (bw->priv);

	parent_class->finalize (object);
}

static void
bonobo_widget_size_request (GtkWidget *widget,
			    GtkRequisition *requisition)
{
	GtkBin *bin;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WIDGET (widget));
	g_return_if_fail (requisition != NULL);

	bin = GTK_BIN (widget);

	if (bin->child && GTK_WIDGET_VISIBLE (bin->child)) {
		GtkRequisition child_requisition;
      
		gtk_widget_size_request (bin->child, &child_requisition);

		requisition->width = child_requisition.width;
		requisition->height = child_requisition.height;
	}
}

static void
bonobo_widget_size_allocate (GtkWidget *widget,
			     GtkAllocation *allocation)
{
	GtkBin *bin;
	GtkAllocation child_allocation;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WIDGET (widget));
	g_return_if_fail (allocation != NULL);

	widget->allocation = *allocation;
	bin = GTK_BIN (widget);

	child_allocation.x = allocation->x;
	child_allocation.y = allocation->y;
	child_allocation.width = allocation->width;
	child_allocation.height = allocation->height;

	if (bin->child)
		gtk_widget_size_allocate (bin->child, &child_allocation);
}

static void
bonobo_widget_remove (GtkContainer *container,
		      GtkWidget    *widget)
{
	BonoboWidget *bw = (BonoboWidget *) container;

	bw->priv->frame = NULL;

	GTK_CONTAINER_CLASS (parent_class)->remove (container, widget);
}

static void
bonobo_widget_class_init (BonoboWidgetClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;
	GtkContainerClass *container_class = (GtkContainerClass *) klass;

	container_class->remove = bonobo_widget_remove;

	widget_class->size_request = bonobo_widget_size_request;
	widget_class->size_allocate = bonobo_widget_size_allocate;

	object_class->finalize = bonobo_widget_finalize;
	object_class->dispose  = bonobo_widget_dispose;
}

static void
bonobo_widget_instance_init (BonoboWidget *bw)
{
	bw->priv = g_new0 (BonoboWidgetPrivate, 1);
}

/**
 * bonobo_widget_set_property:
 * @control: A #BonoboWidget that represents an IDL:Bonobo/Control:1.0
 * @first_prop: first property name to set.
 *
 * This is a utility function used to set a number of properties
 * in the Bonobo Control in @control.
 *
 * This function takes a variable list of arguments that must be NULL
 * terminated.  Arguments come in tuples: a string (for the argument
 * name) and the data type that is to be transfered.  The
 * implementation of the actual setting of the PropertyBag values is
 * done by the bonobo_property_bag_client_setv() function).
 *
 * This only works for BonoboWidgets that represent controls (ie,
 * that were returned by bonobo_widget_new_control_from_objref() or
 * bonobo_widget_new_control().
 */
void
bonobo_widget_set_property (BonoboWidget      *control,
			    const char        *first_prop, ...)
{
	Bonobo_PropertyBag pb;
	CORBA_Environment  ev;

	va_list args;
	va_start (args, first_prop);

	g_return_if_fail (control != NULL);
	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (control->priv != NULL);
	g_return_if_fail (BONOBO_IS_WIDGET (control));

	CORBA_exception_init (&ev);
	
	pb = bonobo_control_frame_get_control_property_bag (
		control->priv->frame, &ev);

	if (BONOBO_EX (&ev))
		g_warning ("Error getting property bag from control");
	else {
		/* FIXME: this should use ev */
		char *err = bonobo_property_bag_client_setv (pb, &ev, first_prop, args);

		if (err)
			g_warning ("Error '%s'", err);
	}

	bonobo_object_release_unref (pb, &ev);

	CORBA_exception_free (&ev);

	va_end (args);
}


/**
 * bonobo_widget_get_property:
 * @control: A #BonoboWidget that represents an IDL:Bonobo/Control:1.0
 * @first_prop: first property name to set.
 *
 * This is a utility function used to get a number of properties
 * in the Bonobo Control in @control.
 *
 * This function takes a variable list of arguments that must be NULL
 * terminated.  Arguments come in tuples: a string (for the argument
 * name) and a pointer where the data will be stored.  The
 * implementation of the actual setting of the PropertyBag values is
 * done by the bonobo_property_bag_client_setv() function).
 *
 * This only works for BonoboWidgets that represent controls (ie,
 * that were returned by bonobo_widget_new_control_from_objref() or
 * bonobo_widget_new_control().
 */
void
bonobo_widget_get_property (BonoboWidget      *control,
			    const char        *first_prop, ...)
{
	Bonobo_PropertyBag pb;
	CORBA_Environment  ev;

	va_list args;
	va_start (args, first_prop);

	g_return_if_fail (control != NULL);
	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (control->priv != NULL);
	g_return_if_fail (BONOBO_IS_WIDGET (control));

	CORBA_exception_init (&ev);
	
	pb = bonobo_control_frame_get_control_property_bag (
		control->priv->frame, &ev);

	if (BONOBO_EX (&ev))
		g_warning ("Error getting property bag from control");
	else {
		/* FIXME: this should use ev */
		char *err = bonobo_property_bag_client_getv (pb, &ev, first_prop, args);

		if (err)
			g_warning ("Error '%s'", err);
	}

	bonobo_object_release_unref (pb, &ev);

	CORBA_exception_free (&ev);

	va_end (args);
}
