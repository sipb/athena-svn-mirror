/* -*- mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-socket.c: a Gtk+ socket wrapper
 *
 * Authors:
 *   Martin Baulig     (martin@home-of-linux.org)
 *   Michael Meeks     (michael@ximian.com)
 *
 * Copyright 2001, Ximian, Inc.
 *                 Martin Baulig.
 */
#include "config.h"
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkdnd.h>
#include <bonobo/bonobo-socket.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-control-frame.h>
#include <bonobo/bonobo-control-internal.h>
#include <libgnome/gnome-macros.h>


/* Used to turn on any socket sizing
 * bits layered over gtk we have */
#undef DEBUG_RAW_GTK

/* Private part of the BonoboSocket structure */
typedef struct {
	/* Signal handler ID for the toplevel's GtkWindow::set_focus() */
	gulong set_focus_id;

	/* Whether a descendant of us has the focus.  If this is the case, it
	 * means that we are out-of-process.
	 */
	guint descendant_has_focus : 1;
} BonoboSocketPrivate;

GNOME_CLASS_BOILERPLATE (BonoboSocket, bonobo_socket,
			 GObject, GTK_TYPE_SOCKET);

static void
bonobo_socket_finalize (GObject *object)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;

	dprintf ("bonobo_socket_finalize %p\n", object);

	socket = BONOBO_SOCKET (object);
	priv = socket->priv;

	priv->descendant_has_focus = FALSE;

	g_free (priv);
	socket->priv = NULL;

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

gboolean
bonobo_socket_disposed (BonoboSocket *socket)
{
	return (socket->frame == NULL);
}

static void
bonobo_socket_dispose (GObject *object)
{
	BonoboSocket *socket = (BonoboSocket *) object;
	BonoboSocketPrivate *priv;

	dprintf ("bonobo_socket_dispose %p\n", object);

	priv = socket->priv;

	if (socket->frame) {
		bonobo_socket_set_control_frame (socket, NULL);
		g_assert (socket->frame == NULL);
	}

	if (priv->set_focus_id) {
		g_assert (socket->socket.toplevel != NULL);
		g_signal_handler_disconnect (socket->socket.toplevel, priv->set_focus_id);
		priv->set_focus_id = 0;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
bonobo_socket_realize (GtkWidget *widget)
{
	BonoboSocket *socket;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_SOCKET (widget));

	socket = BONOBO_SOCKET (widget);

	dprintf ("bonobo_socket_realize %p\n", widget);

	GNOME_CALL_PARENT (GTK_WIDGET_CLASS, realize, (widget));

	if (socket->frame) {
		g_object_ref (socket->frame);
		bonobo_control_frame_get_remote_window (socket->frame, NULL);
		g_object_unref (socket->frame);
	}

	g_assert (GTK_WIDGET_REALIZED (widget));
}

static void
bonobo_socket_unrealize (GtkWidget *widget)
{
	dprintf ("unrealize %p\n", widget);

	g_assert (GTK_WIDGET_REALIZED (widget));
	g_assert (GTK_WIDGET (widget)->window);

	/* To stop evilness inside Gtk+ */
	GTK_WIDGET_UNSET_FLAGS (widget, GTK_REALIZED);

	GNOME_CALL_PARENT (GTK_WIDGET_CLASS, unrealize, (widget));
}

static gboolean
bonobo_socket_expose_event (GtkWidget      *widget,
			    GdkEventExpose *event)
{
	gboolean retval;

	retval = GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);

	dprintf ("bonobo_socket_expose_event %p (%d, %d), (%d, %d)\n",
		 widget,
		 event->area.x, event->area.y,
		 event->area.width, event->area.height);

#ifdef DEBUG_CONTROL
	gdk_draw_line (widget->window,
		       widget->style->black_gc,
		       event->area.x, event->area.y,
		       event->area.x + event->area.width,
		       event->area.y + event->area.height);
#endif

	return retval;
}

static void
bonobo_socket_state_changed (GtkWidget   *widget,
			     GtkStateType previous_state)
{
	BonoboSocket *socket = BONOBO_SOCKET (widget);

	if (!socket->frame)
		return;

	if (!bonobo_control_frame_get_autostate (socket->frame))
		return;

	bonobo_control_frame_control_set_state (
		socket->frame, GTK_WIDGET_STATE (widget));
}

/* Callback for GtkWindow::set_focus().  We watch the focused widget in this way. */
static void
toplevel_set_focus_cb (GtkWindow *window, GtkWidget *focus, gpointer data)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;
	GtkWidget *socket_widget;
	gboolean descendant_had_focus;
	gboolean should_autoactivate;

	socket = BONOBO_SOCKET (data);
	priv = socket->priv;

	g_assert (socket->socket.toplevel == GTK_WIDGET (window));

	socket_widget = GTK_WIDGET (socket);

	descendant_had_focus = priv->descendant_has_focus;

	should_autoactivate = (socket->socket.plug_widget	/* Only in the in-process case */
			       && socket->frame			/* We need an auto-activatable frame */
			       && bonobo_control_frame_get_autoactivate (socket->frame));

	/* If a descendant of ours is focused then possibly activate its
	 * control, unless there are intermediate sockets between us --- they
	 * should take care of that themselves.
	 */

	if (focus && gtk_widget_get_ancestor (focus, GTK_TYPE_SOCKET) == socket_widget) {
		priv->descendant_has_focus = TRUE;

		if (!descendant_had_focus && should_autoactivate)
			bonobo_control_frame_control_activate (socket->frame);
	} else {
		priv->descendant_has_focus = FALSE;

		if (descendant_had_focus && should_autoactivate)
			bonobo_control_frame_control_deactivate (socket->frame);
	}
}

/* GtkWidget::hierarchy_changed() handler.  We have to monitor our toplevel so
 * that we can connect to its GtkWindow::set_focus() signal, so that we can keep
 * track of the currently focused widget.
 */
static void
bonobo_socket_hierarchy_changed (GtkWidget *widget, GtkWidget *previous_toplevel)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;

	socket = BONOBO_SOCKET (widget);
	priv = socket->priv;

	if (priv->set_focus_id) {
		g_assert (socket->socket.toplevel != NULL);
		g_signal_handler_disconnect (socket->socket.toplevel, priv->set_focus_id);
		priv->set_focus_id = 0;
	}

	(* GTK_WIDGET_CLASS (parent_class)->hierarchy_changed) (widget, previous_toplevel);

	if (socket->socket.toplevel && GTK_IS_WINDOW (socket->socket.toplevel))
		priv->set_focus_id = g_signal_connect_after (socket->socket.toplevel, "set_focus",
							     G_CALLBACK (toplevel_set_focus_cb), socket);
}

/* NOTE: This will only get called in the out-of-process case.  GTK+ only sends
 * focus-in/out events to leaf widgets, not their ancestors.
 */
static gint
bonobo_socket_focus_in (GtkWidget     *widget,
			GdkEventFocus *focus)
{
	BonoboSocket *socket = BONOBO_SOCKET (widget);

	if (socket->frame &&
	    bonobo_control_frame_get_autoactivate (socket->frame))
		bonobo_control_frame_control_activate (socket->frame);
	else
		dprintf ("No activate on focus in");

	return GTK_WIDGET_CLASS (parent_class)->focus_in_event (widget, focus);
}

/* NOTE: This will only get called in the out-of-process case.  GTK+ only sends
 * focus-in/out events to leaf widgets, not their ancestors.
 */
static gint
bonobo_socket_focus_out (GtkWidget     *widget,
			 GdkEventFocus *focus)
{
	BonoboSocket *socket = BONOBO_SOCKET (widget);

	if (socket->frame &&
	    bonobo_control_frame_get_autoactivate (socket->frame))
		bonobo_control_frame_control_deactivate (socket->frame);
	else
		dprintf ("No de-activate on focus out");

	return GTK_WIDGET_CLASS (parent_class)->focus_out_event (widget, focus);
}

static void
bonobo_socket_size_allocate (GtkWidget     *widget,
			     GtkAllocation *allocation)
{
	GtkSocket *socket = (GtkSocket *) widget;

	dprintf ("bonobo_socket_size_allocate %p: (%d, %d), (%d, %d), %p, %p\n",
		 widget, allocation->x, allocation->y,
		 allocation->width, allocation->height,
		 socket->plug_widget, socket->plug_window);

	GNOME_CALL_PARENT (GTK_WIDGET_CLASS, size_allocate, (widget, allocation));
}

static void
bonobo_socket_size_request (GtkWidget      *widget,
			    GtkRequisition *requisition)
{
	BonoboSocket *socket = (BonoboSocket *) widget;
	GtkSocket    *gtk_socket = (GtkSocket *) widget;

	dprintf ("pre bonobo_socket_size_request %p: realized %d, %s frame, %d %d\n",
		 widget, GTK_WIDGET_REALIZED (widget) ? 1:0,
		 socket->frame ? "has" : "no",
		 gtk_socket->is_mapped, gtk_socket->have_size);

#ifndef DEBUG_RAW_GTK
	if (GTK_WIDGET_REALIZED (widget) ||
	    !socket->frame ||
	    (gtk_socket->is_mapped && gtk_socket->have_size))
#endif
		GNOME_CALL_PARENT (GTK_WIDGET_CLASS, size_request,
				   (widget, requisition));

#ifndef DEBUG_RAW_GTK
	else if (gtk_socket->have_size &&
		 GTK_WIDGET_VISIBLE (gtk_socket)) {

		requisition->width = gtk_socket->request_width;
		requisition->height = gtk_socket->request_height;

	} else {
		CORBA_Environment tmp_ev, *ev;

		CORBA_exception_init ((ev = &tmp_ev));

		bonobo_control_frame_size_request (
			socket->frame, requisition, ev);

		if (!BONOBO_EX (ev)) {
			gtk_socket->have_size = TRUE;
			gtk_socket->request_width = requisition->width;
			gtk_socket->request_height = requisition->height;
		}

		CORBA_exception_free (ev);
	}
#endif

	dprintf ("bonobo_socket_size_request %p: %d, %d\n",
		 widget, requisition->width, requisition->height);
}

static void
bonobo_socket_show (GtkWidget *widget)
{
	dprintf ("bonobo_socket_show %p\n", widget);

	/* We do a check_resize here, since if we're in-proc we
	 * want to force a size_allocate on the contained GtkPlug,
	 * before we go and map it (waiting for the idle resize),
	 * since idle can be held off for a good while, and cause
	 * extreme ugliness and flicker */
	gtk_container_check_resize (GTK_CONTAINER (widget));

	GNOME_CALL_PARENT (GTK_WIDGET_CLASS, show, (widget));
}

static void
bonobo_socket_show_all (GtkWidget *widget)
{
	/* Do nothing - we don't want this to
	 * propagate to an in-proc plug */
}

static gboolean
bonobo_socket_plug_removed (GtkSocket *socket)
{
	dprintf ("bonobo_socket_plug_removed %p\n", socket);

	return TRUE;
}

static void
bonobo_socket_class_init (BonoboSocketClass *klass)
{
	GObjectClass   *gobject_class;
	GtkWidgetClass *widget_class;
	GtkSocketClass *socket_class;

	gobject_class = (GObjectClass *) klass;
	widget_class  = (GtkWidgetClass *) klass;
	socket_class  = (GtkSocketClass *) klass;

	gobject_class->finalize = bonobo_socket_finalize;
	gobject_class->dispose  = bonobo_socket_dispose;

	widget_class->realize           = bonobo_socket_realize;
	widget_class->unrealize         = bonobo_socket_unrealize;
	widget_class->state_changed     = bonobo_socket_state_changed;
	widget_class->hierarchy_changed = bonobo_socket_hierarchy_changed;
	widget_class->focus_in_event    = bonobo_socket_focus_in;
	widget_class->focus_out_event   = bonobo_socket_focus_out;
	widget_class->size_request    	= bonobo_socket_size_request;
	widget_class->size_allocate   	= bonobo_socket_size_allocate;
	widget_class->expose_event    	= bonobo_socket_expose_event;
	widget_class->show            	= bonobo_socket_show;
	widget_class->show_all        	= bonobo_socket_show_all;

	socket_class->plug_removed = bonobo_socket_plug_removed;
}

static void
bonobo_socket_instance_init (BonoboSocket *socket)
{
	BonoboSocketPrivate *priv;

	priv = g_new0 (BonoboSocketPrivate, 1);
	socket->priv = priv;
}

/**
 * bonobo_socket_new:
 *
 * Create a new empty #BonoboSocket.
 *
 * Returns: A new #BonoboSocket.
 */
GtkWidget*
bonobo_socket_new (void)
{
	return g_object_new (bonobo_socket_get_type (), NULL);
}

BonoboControlFrame *
bonobo_socket_get_control_frame (BonoboSocket *socket)
{
	g_return_val_if_fail (BONOBO_IS_SOCKET (socket), NULL);

	return socket->frame;
}

void
bonobo_socket_set_control_frame (BonoboSocket       *socket,
				 BonoboControlFrame *frame)
{
	BonoboControlFrame *old_frame;

	g_return_if_fail (BONOBO_IS_SOCKET (socket));

	if (socket->frame == frame)
		return;

	old_frame = socket->frame;

	if (frame)
		socket->frame = BONOBO_CONTROL_FRAME (
			bonobo_object_ref (BONOBO_OBJECT (frame)));
	else
		socket->frame = NULL;

	if (old_frame) {
		bonobo_control_frame_set_socket (old_frame, NULL);
		bonobo_object_unref (BONOBO_OBJECT (old_frame));
	}

	if (frame)
		bonobo_control_frame_set_socket (frame, socket);
}

void
bonobo_socket_add_id (BonoboSocket   *socket,
		      GdkNativeWindow xid)
{
	GtkSocket *gtk_socket = (GtkSocket *) socket;

	gtk_socket_add_id (gtk_socket, xid);

	/* The allocate didn't get through even to the in-proc case,
	 * so do it again */
	if (gtk_socket->plug_widget) {
		GtkAllocation child_allocation;

		child_allocation.x = 0;
		child_allocation.y = 0;
		child_allocation.width = GTK_WIDGET (gtk_socket)->allocation.width;
		child_allocation.height = GTK_WIDGET (gtk_socket)->allocation.height;
		
		gtk_widget_size_allocate (gtk_socket->plug_widget,
					  &child_allocation);
	}
}
