/* -*- mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-plug.c: a Gtk plug wrapper.
 *
 * Author:
 *   Martin Baulig     (martin@home-of-linux.org)
 *   Michael Meeks     (michael@ximian.com)
 *
 * Copyright 2001, Ximian, Inc.
 *                 Martin Baulig.
 */

#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <libgnome/gnome-macros.h>
#include <bonobo/bonobo-i18n.h>
#include <bonobo/bonobo-plug.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-control-internal.h>

struct _BonoboPlugPrivate {
	gboolean forward_events;
};

enum {
	PROP_0,
	PROP_FORWARD_EVENTS
};

GNOME_CLASS_BOILERPLATE (BonoboPlug, bonobo_plug,
			 GObject, GTK_TYPE_PLUG);

/**
 * bonobo_plug_construct:
 * @plug: The #BonoboPlug.
 * @socket_id: the XID of the socket's window.
 *
 * Finish the creation of a #BonoboPlug widget. This function
 * will generally only be used by classes deriving
 * from #BonoboPlug.
 */
void
bonobo_plug_construct (BonoboPlug *plug, guint32 socket_id)
{
	gtk_plug_construct (GTK_PLUG (plug), socket_id);
}

/**
 * bonobo_plug_new:
 * @socket_id: the XID of the socket's window.
 *
 * Create a new plug widget inside the #GtkSocket identified
 * by @socket_id.
 *
 * Returns: the new #BonoboPlug widget.
 */
GtkWidget*
bonobo_plug_new (guint32 socket_id)
{
	BonoboPlug *plug;

	plug = BONOBO_PLUG (g_object_new (bonobo_plug_get_type (), NULL));

	bonobo_plug_construct (plug, socket_id);

	dprintf ("bonobo_plug_new => %p\n", plug);

	return GTK_WIDGET (plug);
}

BonoboControl *
bonobo_plug_get_control (BonoboPlug *plug)
{
	g_return_val_if_fail (BONOBO_IS_PLUG (plug), NULL);

	return plug->control;
}

void
bonobo_plug_set_control (BonoboPlug    *plug,
			 BonoboControl *control)
{
	BonoboControl *old_control;

	g_return_if_fail (BONOBO_IS_PLUG (plug));

	if (plug->control == control)
		return;

	dprintf ("bonobo_plug_set_control (%p, %p) [%p]\n",
		 plug, control, plug->control);

	old_control = plug->control;

	if (control)
		plug->control = g_object_ref (control);
	else
		plug->control = NULL;

	if (old_control) {
		bonobo_control_set_plug (old_control, NULL);
		g_object_unref (old_control);
	}

	if (control)
		bonobo_control_set_plug (control, plug);
}

static gboolean
bonobo_plug_delete_event (GtkWidget   *widget,
			  GdkEventAny *event)
{
	dprintf ("bonobo_plug_delete_event %p\n", widget);

	return FALSE;
}

static void
bonobo_plug_realize (GtkWidget *widget)
{
	BonoboPlug *plug = (BonoboPlug *) widget;

	dprintf ("bonobo_plug_realize %p\n", plug);

	GTK_WIDGET_CLASS (parent_class)->realize (widget);
}

static void
bonobo_plug_unrealize (GtkWidget *widget)
{
	BonoboPlug *plug = (BonoboPlug *) widget;

	dprintf ("bonobo_plug_unrealize %p\n", plug);

	GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
bonobo_plug_map (GtkWidget *widget)
{
	dprintf ("bonobo_plug_map %p at size %d, %d\n",
		 widget, widget->allocation.width,
		 widget->allocation.height);
	GTK_WIDGET_CLASS (parent_class)->map (widget);
}

static void
bonobo_plug_dispose (GObject *object)
{
	BonoboPlug *plug = (BonoboPlug *) object;
	GtkBin *bin_plug = (GtkBin *) object;

	dprintf ("bonobo_plug_dispose %p\n", object);

	if (bin_plug->child) {
		gtk_container_remove (
			&bin_plug->container, bin_plug->child);
		dprintf ("Removing child ...");
	}

	if (plug->control)
		bonobo_plug_set_control (plug, NULL);

	parent_class->dispose (object);
}

static void
bonobo_plug_finalize (GObject *object)
{
	BonoboPlug *plug = (BonoboPlug *) object;

	if (plug->priv)
		g_free (plug->priv);

	parent_class->finalize (object);
}

static void
bonobo_plug_set_property (GObject      *object,
			  guint         param_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
	BonoboPlug *plug;

	g_return_if_fail (BONOBO_IS_PLUG (object));

	plug = BONOBO_PLUG (object);

	switch (param_id) {
	case PROP_FORWARD_EVENTS:
		plug->priv->forward_events = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
bonobo_plug_get_property (GObject    *object,
			  guint       param_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
	BonoboPlug *plug;

	g_return_if_fail (BONOBO_IS_PLUG (object));

	plug = BONOBO_PLUG (object);

	switch (param_id) {
	case PROP_FORWARD_EVENTS:
		g_value_set_boolean (value, plug->priv->forward_events);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
bonobo_plug_size_allocate (GtkWidget     *widget,
			   GtkAllocation *allocation)
{
	dprintf ("bonobo_plug_size_allocate %p: (%d, %d), (%d, %d) %d! %s\n",
		 widget,
		 allocation->x, allocation->y,
		 allocation->width, allocation->height,
		 GTK_WIDGET_TOPLEVEL (widget),
		 GTK_BIN (widget)->child ?
		 g_type_name_from_instance ((gpointer)GTK_BIN (widget)->child):
		 "No child!");

	GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);
}

static void
bonobo_plug_size_request (GtkWidget      *widget,
			  GtkRequisition *requisition)
{
	GTK_WIDGET_CLASS (parent_class)->size_request (widget, requisition);

	dprintf ("bonobo_plug_size_request %p: %d, %d\n",
		 widget, requisition->width, requisition->height);
}

static gboolean
bonobo_plug_expose_event (GtkWidget      *widget,
			  GdkEventExpose *event)
{
	gboolean retval;

	retval = GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);

	dprintf ("bonobo_plug_expose_event %p (%d, %d), (%d, %d)"
		 "%s (%d && %d == %d)\n",
		 widget,
		 event->area.x, event->area.y,
		 event->area.width, event->area.height,
		 GTK_WIDGET_TOPLEVEL (widget) ? "toplevel" : "bin class",
		 GTK_WIDGET_VISIBLE (widget),
		 GTK_WIDGET_MAPPED (widget),
		 GTK_WIDGET_DRAWABLE (widget));

#ifdef DEBUG_CONTROL
	gdk_draw_line (widget->window,
		       widget->style->black_gc,
		       event->area.x + event->area.width,
		       event->area.y,
		       event->area.x, 
		       event->area.y + event->area.height);

	gdk_draw_line (widget->window,
		       widget->style->black_gc,
		       widget->allocation.x,
		       widget->allocation.y,
		       widget->allocation.x + widget->allocation.width,
		       widget->allocation.y + widget->allocation.height);
#endif

	return retval;
}

static gboolean
bonobo_plug_button_event (GtkWidget      *widget,
			  GdkEventButton *event)
{
	XEvent xevent;

	g_return_val_if_fail (BONOBO_IS_PLUG (widget), FALSE);

	if (!BONOBO_PLUG (widget)->priv->forward_events || !GTK_WIDGET_TOPLEVEL (widget))
		return FALSE;

	if (event->type == GDK_BUTTON_PRESS) {
		xevent.xbutton.type = ButtonPress;

		/* X does an automatic pointer grab on button press
		 * if we have both button press and release events
		 * selected.
		 * We don't want to hog the pointer on our parent.
		 */
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
	} else
		xevent.xbutton.type = ButtonRelease;
    
	xevent.xbutton.display     = GDK_WINDOW_XDISPLAY (widget->window);
	xevent.xbutton.window      = GDK_WINDOW_XWINDOW (GTK_PLUG (widget)->socket_window);
	xevent.xbutton.root        = GDK_ROOT_WINDOW (); /* FIXME */
	/*
	 * FIXME: the following might cause
	 *        big problems for non-GTK apps
	 */
	xevent.xbutton.x           = 0;
	xevent.xbutton.y           = 0;
	xevent.xbutton.x_root      = 0;
	xevent.xbutton.y_root      = 0;
	xevent.xbutton.state       = event->state;
	xevent.xbutton.button      = event->button;
	xevent.xbutton.same_screen = TRUE; /* FIXME ? */

	gdk_error_trap_push ();

	XSendEvent (GDK_DISPLAY (),
		    GDK_WINDOW_XWINDOW (GTK_PLUG (widget)->socket_window),
		    False, NoEventMask, &xevent);

	gdk_flush ();
	gdk_error_trap_pop ();

	return TRUE;
}

static void
bonobo_plug_instance_init (BonoboPlug *plug)
{
	BonoboPlugPrivate *priv;

	priv = g_new0 (BonoboPlugPrivate, 1);

	plug->priv = priv;

	priv->forward_events = TRUE;
}

static void
bonobo_plug_class_init (BonoboPlugClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass *) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

	gobject_class->dispose      = bonobo_plug_dispose;
	gobject_class->finalize     = bonobo_plug_finalize;
	gobject_class->set_property = bonobo_plug_set_property;
	gobject_class->get_property = bonobo_plug_get_property;

	widget_class->realize              = bonobo_plug_realize;
	widget_class->unrealize            = bonobo_plug_unrealize;
	widget_class->delete_event         = bonobo_plug_delete_event;
	widget_class->size_request         = bonobo_plug_size_request;
	widget_class->size_allocate        = bonobo_plug_size_allocate;
	widget_class->expose_event         = bonobo_plug_expose_event;
	widget_class->button_press_event   = bonobo_plug_button_event;
	widget_class->button_release_event = bonobo_plug_button_event;
	widget_class->map                  = bonobo_plug_map;

	g_object_class_install_property (
		gobject_class,
		PROP_FORWARD_EVENTS,
		g_param_spec_boolean ("event_forwarding",
				    _("Event Forwarding"),
				    _("Whether X events should be forwarded"),
				      TRUE,
				      G_PARAM_READABLE | G_PARAM_WRITABLE));
}
