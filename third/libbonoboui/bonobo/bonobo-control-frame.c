/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Bonobo control frame object.
 *
 * Authors:
 *   Michael Meeks     (michael@ximian.com)
 *   Nat Friedman      (nat@ximian.com)
 *   Federico Mena     (federico@ximian.com)
 *   Miguel de Icaza   (miguel@ximian.com)
 *   George Lebel      (jirka@5z.com)
 *   Maciej Stachowiak (mjs@eazel.com)
 *
 * Copyright 1999, 2001 Ximian, Inc.
 *                 2000 Eazel, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkplug.h>
#include <gtk/gtkbox.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-control-frame.h>
#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>
#include <gdk/gdktypes.h>
#include <gtk/gtkhbox.h>
#include <bonobo/bonobo-socket.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-ui-marshal.h>
#include <bonobo/bonobo-control-internal.h>

enum {
	ACTIVATED,
	ACTIVATE_URI,
	LAST_SIGNAL
};

#define PARENT_TYPE BONOBO_TYPE_OBJECT

static guint control_frame_signals [LAST_SIGNAL];

/* Parent object class in GTK hierarchy */
static GObjectClass *bonobo_control_frame_parent_class;

struct _BonoboControlFramePrivate {
	BonoboControl     *inproc_control;
	Bonobo_Control	   control;
	GtkWidget	  *socket;
	Bonobo_UIContainer ui_container;
	BonoboPropertyBag *propbag;
	gboolean           autoactivate;
	gboolean           autostate;
	gboolean           activated;
};

static void
control_connection_died_cb (gpointer connection,
			    gpointer user_data)
{
	CORBA_Object cntl;
	CORBA_Environment ev;
	BonoboControlFrame *frame = BONOBO_CONTROL_FRAME (user_data);

	g_return_if_fail (frame != NULL);

	dprintf ("The remote control end died unexpectedly");

	CORBA_exception_init (&ev);

	cntl = CORBA_Object_duplicate (frame->priv->control, &ev);

	bonobo_control_frame_bind_to_control (
		frame, CORBA_OBJECT_NIL, NULL);

	CORBA_exception_set_system (
		&ev, ex_CORBA_COMM_FAILURE, CORBA_COMPLETED_YES);

	bonobo_object_check_env (BONOBO_OBJECT (frame), cntl, &ev);

	CORBA_Object_release (cntl, &ev);

	CORBA_exception_free (&ev);
}

static Bonobo_Gdk_WindowId
impl_Bonobo_ControlFrame_getToplevelId (PortableServer_Servant  servant,
					CORBA_Environment      *ev)
{
	BonoboControlFrame *frame;
	GtkWidget          *toplev;
	Bonobo_Gdk_WindowId id;

	frame = BONOBO_CONTROL_FRAME (bonobo_object (servant));

	for (toplev = bonobo_control_frame_get_widget (frame);
	     toplev && toplev->parent;
	     toplev = toplev->parent)
		;

	bonobo_return_val_if_fail (toplev != NULL, NULL, ev);

	if (BONOBO_IS_PLUG (toplev)) { 
		BonoboControl      *control;
		Bonobo_ControlFrame frame;

		control = bonobo_plug_get_control (BONOBO_PLUG (toplev));
		if (!control) {
			g_warning ("No control bound to plug from which to "
				   "get transient parent");
			return CORBA_string_dup ("");
		}

		frame = bonobo_control_get_control_frame (control, ev);
		if (frame == CORBA_OBJECT_NIL) {
			g_warning ("No control frame associated with control from "
				   "which to get transient parent");
			return CORBA_string_dup ("");
		}

		id = Bonobo_ControlFrame_getToplevelId (frame, ev);
	} else
		id = bonobo_control_window_id_from_x11 (
			GDK_WINDOW_XWINDOW (toplev->window));

	return id;
}

static Bonobo_PropertyBag
impl_Bonobo_ControlFrame_getAmbientProperties (PortableServer_Servant servant,
					       CORBA_Environment     *ev)
{
	BonoboControlFrame *frame;

	frame = BONOBO_CONTROL_FRAME (bonobo_object (servant));

	if (!frame->priv->propbag)
		return CORBA_OBJECT_NIL;

	return bonobo_object_dup_ref (
		BONOBO_OBJREF (frame->priv->propbag), ev);
}

static Bonobo_UIContainer
impl_Bonobo_ControlFrame_getUIContainer (PortableServer_Servant  servant,
					 CORBA_Environment      *ev)
{
	BonoboControlFrame *frame = BONOBO_CONTROL_FRAME (
		bonobo_object_from_servant (servant));

	if (frame->priv->ui_container == NULL)
		return CORBA_OBJECT_NIL;

	return bonobo_object_dup_ref (frame->priv->ui_container, ev);
}

/* --- Notifications --- */

static void
impl_Bonobo_ControlFrame_notifyActivated (PortableServer_Servant  servant,
					  const CORBA_boolean     state,
					  CORBA_Environment      *ev)
{
	BonoboControlFrame *frame = BONOBO_CONTROL_FRAME (
		bonobo_object_from_servant (servant));

	g_signal_emit (frame, control_frame_signals [ACTIVATED], 0, state);
}

static void
impl_Bonobo_ControlFrame_queueResize (PortableServer_Servant  servant,
				      CORBA_Environment      *ev)
{
	/* Supposedly Gtk+ handles size negotiation properly for us */
/*	BonoboSocket *socket;
	BonoboControlFrame *frame;

	frame = BONOBO_CONTROL_FRAME (bonobo_object (servant));

	if ((socket = frame->priv->socket)) {
		GTK_SOCKET (socket)->have_size = FALSE;
		gtk_widget_queue_resize (GTK_WIDGET (socket));
		} */
}

static void
impl_Bonobo_ControlFrame_activateURI (PortableServer_Servant  servant,
				       const CORBA_char       *uri,
				       CORBA_boolean           relative,
				       CORBA_Environment      *ev)
{
	BonoboControlFrame *frame = BONOBO_CONTROL_FRAME (bonobo_object_from_servant (servant));

	g_signal_emit (frame,
		       control_frame_signals [ACTIVATE_URI], 0,
		       (const char *) uri, (gboolean) relative);
}

#ifdef DEBUG_CONTROL
static void
dump_geom (GdkWindow *window)
{
	gint x, y, width, height, depth;
	
	gdk_window_get_geometry (window, &x, &y,
				 &width, &height, &depth);
	
	fprintf (stderr, "geom (%d, %d), (%d, %d), %d ",
		 x, y, width, height, depth);
}

static void
dump_gdk_tree (GdkWindow *window)
{
	GList     *l;
	GtkWidget *widget = NULL;

	gdk_window_get_user_data (window, (gpointer) &widget);

	fprintf (stderr, "Window %p (parent %p) ", window,
		 gdk_window_get_parent (window));

	dump_geom (window);

	if (widget) {
		fprintf (stderr, "has widget '%s' %s ",
			 g_type_name_from_instance ((gpointer) widget),
			 GTK_WIDGET_VISIBLE (widget) ? "visible" : "hidden");
	} else
		fprintf (stderr, "No widget ");

	fprintf (stderr, "gdk: %s %s ", 
		 gdk_window_is_visible (window) ? "visible" : "invisible",
		 gdk_window_is_viewable (window) ? "viewable" : "not viewable");
	
	l = gdk_window_peek_children (window);
	fprintf (stderr, "%d children:\n", g_list_length (l));

	for (; l; l = l->next)
		dump_gdk_tree (l->data);

	fprintf (stderr, "\n");
}
#endif

static CORBA_char *
bonobo_control_frame_get_remote_window_id (BonoboControlFrame *frame,
					   CORBA_Environment  *ev)
{
#ifdef HAVE_GTK_MULTIHEAD
	CORBA_char  *retval;
	char        *cookie;
	int          screen;

	screen = gdk_screen_get_number (
			gtk_widget_get_screen (frame->priv->socket));

	cookie = g_strdup_printf ("screen=%d", screen);

	retval = Bonobo_Control_getWindowId (
				frame->priv->control, cookie, ev);

	g_free (cookie);

	return retval;
#else
	return Bonobo_Control_getWindowId (
				frame->priv->control, "", ev);
#endif /* HAVE_GTK_MULTIHEAD */
}

void
bonobo_control_frame_get_remote_window (BonoboControlFrame *frame,
					CORBA_Environment  *opt_ev)
					
{
	CORBA_char *id;
	CORBA_Environment *ev, tmp_ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	dprintf ("bonobo_control_frame_get_remote_window "
		 "%p %p %d %p\n", frame->priv, frame->priv->socket,
		 GTK_WIDGET_REALIZED (frame->priv->socket),
		 frame->priv->control);

	if (!frame->priv || !frame->priv->socket ||
	    !GTK_WIDGET_REALIZED (frame->priv->socket) ||
	    frame->priv->control == CORBA_OBJECT_NIL)
		return;

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	/* Introduce ourselves to the Control. */
	id = bonobo_control_frame_get_remote_window_id (frame, ev);
	if (BONOBO_EX (ev)) {
		dprintf ("getWindowId exception\n");
		bonobo_object_check_env (BONOBO_OBJECT (frame),
					 frame->priv->control, ev);

	} else {
		GdkNativeWindow xid;
		BonoboPlug *plug = NULL;

		xid = bonobo_control_x11_from_window_id (id);
		dprintf ("setFrame id '%s' (=%d)\n", id, xid);
		CORBA_free (id);

		{
			gpointer user_data = NULL;
			if (gdk_window_lookup (xid)) {
				gdk_window_get_user_data (gdk_window_lookup (xid),
							  &user_data);
				plug = user_data;
			}
		}

		/* FIXME: What happens if we have an in-proc CORBA proxy eg.
		 * for a remote X window ? - we need to treat these differently. */

		if (plug && !frame->priv->inproc_control) {
			g_warning ("ARGH - serious ORB screwup");
			frame->priv->inproc_control = bonobo_plug_get_control (plug);
		} else if (!plug && frame->priv->inproc_control) 
			g_warning ("ARGH - different serious ORB screwup");

		bonobo_socket_add_id (BONOBO_SOCKET (frame->priv->socket), xid);
	}		

	if (!opt_ev)
		CORBA_exception_free (ev);
}

/**
 * bonobo_control_frame_construct:
 * @control_frame: The #BonoboControlFrame object to be initialized.
 * @ui_container: A CORBA object for the UIContainer for the container application.
 *
 * Initializes @control_frame with the parameters.
 *
 * Returns: the initialized BonoboControlFrame object @control_frame that implements the
 * Bonobo::ControlFrame CORBA service.
 */
BonoboControlFrame *
bonobo_control_frame_construct (BonoboControlFrame *frame,
				Bonobo_UIContainer  ui_container,
				CORBA_Environment  *ev)
{
	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), NULL);

	bonobo_control_frame_set_ui_container (frame, ui_container, ev);

	return frame;
}

/**
 * bonobo_control_frame_new:
 * @ui_container: The #Bonobo_UIContainer for the container application.
 *
 * Returns: BonoboControlFrame object that implements the
 * Bonobo::ControlFrame CORBA service. 
 */
BonoboControlFrame *
bonobo_control_frame_new (Bonobo_UIContainer ui_container)
{
	CORBA_Environment   ev;
	BonoboControlFrame *frame;

	frame = g_object_new (BONOBO_TYPE_CONTROL_FRAME, NULL);

	CORBA_exception_init (&ev);
	frame = bonobo_control_frame_construct (frame, ui_container, &ev);
	CORBA_exception_free (&ev);

	return frame;
}

static void
bonobo_control_frame_dispose (GObject *object)
{
	BonoboControlFrame *frame = BONOBO_CONTROL_FRAME (object);

	dprintf ("bonobo_control_frame_dispose %p\n", object);

	if (frame->priv->socket)
		bonobo_control_frame_set_socket (frame, NULL);

	bonobo_control_frame_set_propbag (frame, NULL);

	bonobo_control_frame_bind_to_control (
		frame, CORBA_OBJECT_NIL, NULL);

	bonobo_control_frame_set_ui_container (
		frame, CORBA_OBJECT_NIL, NULL);

	bonobo_control_frame_parent_class->dispose (object);
}

static void
bonobo_control_frame_finalize (GObject *object)
{
	BonoboControlFrame *frame = BONOBO_CONTROL_FRAME (object);

	dprintf ("bonobo_control_frame_finalize %p\n", object);

	g_free (frame->priv);
	
	bonobo_control_frame_parent_class->finalize (object);
}

static void
bonobo_control_frame_activated (BonoboControlFrame *frame, gboolean state)
{
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	frame->priv->activated = state;
}

static void
bonobo_control_frame_class_init (BonoboControlFrameClass *klass)
{
	GObjectClass      *object_class = (GObjectClass *) klass;
	POA_Bonobo_ControlFrame__epv *epv = &klass->epv;

	bonobo_control_frame_parent_class = g_type_class_peek_parent (klass);

	control_frame_signals [ACTIVATED] =
		g_signal_new ("activated",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboControlFrameClass, activated),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE, 1,
			      G_TYPE_BOOLEAN);

	
	control_frame_signals [ACTIVATE_URI] =
		g_signal_new ("activate_uri",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboControlFrameClass, activate_uri),
			      NULL, NULL,
			      bonobo_ui_marshal_VOID__STRING_BOOLEAN,
			      G_TYPE_NONE, 2,
			      G_TYPE_STRING, G_TYPE_BOOLEAN);
	
	klass->activated = bonobo_control_frame_activated;

	object_class->finalize = bonobo_control_frame_finalize;
	object_class->dispose  = bonobo_control_frame_dispose;
	
	epv->getToplevelId        = impl_Bonobo_ControlFrame_getToplevelId;
	epv->getAmbientProperties = impl_Bonobo_ControlFrame_getAmbientProperties;
	epv->getUIContainer       = impl_Bonobo_ControlFrame_getUIContainer;

	epv->notifyActivated      = impl_Bonobo_ControlFrame_notifyActivated;
	epv->queueResize          = impl_Bonobo_ControlFrame_queueResize;
	epv->activateURI          = impl_Bonobo_ControlFrame_activateURI;
}

static void
bonobo_control_frame_init (BonoboObject *object)
{
	BonoboControlFrame *frame = BONOBO_CONTROL_FRAME (object);
	BonoboSocket       *socket;

	frame->priv               = g_new0 (BonoboControlFramePrivate, 1);
	frame->priv->autoactivate = FALSE;
	frame->priv->autostate    = TRUE;

	socket = BONOBO_SOCKET (bonobo_socket_new ());
	gtk_widget_show (GTK_WIDGET (socket));
	bonobo_control_frame_set_socket (frame, socket);
}

BONOBO_TYPE_FUNC_FULL (BonoboControlFrame, 
		       Bonobo_ControlFrame,
		       PARENT_TYPE,
		       bonobo_control_frame);

/**
 * bonobo_control_frame_control_activate:
 * @control_frame: The BonoboControlFrame object whose control should be
 * activated.
 *
 * Activates the BonoboControl embedded in @control_frame by calling the
 * activate() #Bonobo_Control interface method on it.
 */
void
bonobo_control_frame_control_activate (BonoboControlFrame *frame)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	/*
	 * Check that this ControLFrame actually has a Control associated
	 * with it.
	 */
	g_return_if_fail (frame->priv->control != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	Bonobo_Control_activate (frame->priv->control, TRUE, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (frame),
			(CORBA_Object) frame->priv->control, &ev);

	}

	CORBA_exception_free (&ev);
}


/**
 * bonobo_control_frame_control_deactivate:
 * @control_frame: The BonoboControlFrame object whose control should be
 * deactivated.
 *
 * Deactivates the BonoboControl embedded in @frame by calling
 * the activate() CORBA method on it with the parameter %FALSE.
 */
void
bonobo_control_frame_control_deactivate (BonoboControlFrame *frame)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	/*
	 * Check that this ControlFrame actually has a Control associated
	 * with it.
	 */
	g_return_if_fail (frame->priv->control != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	Bonobo_Control_activate (frame->priv->control, FALSE, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (frame),
			(CORBA_Object) frame->priv->control, &ev);
	}

	CORBA_exception_free (&ev);
}

/**
 * bonobo_control_frame_set_autoactivate:
 * @frame: A BonoboControlFrame object.
 * @autoactivate: A flag which indicates whether or not the
 * ControlFrame should automatically perform activation on the Control
 * to which it is bound.
 *
 * Modifies the autoactivate behavior of @frame.  If
 * @frame is set to autoactivate, then it will automatically
 * send an "activate" message to the Control to which it is bound when
 * it gets a focus-in event, and a "deactivate" message when it gets a
 * focus-out event.  Autoactivation is off by default.
 */
void
bonobo_control_frame_set_autoactivate (BonoboControlFrame  *frame,
				       gboolean             autoactivate)
{
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	frame->priv->autoactivate = autoactivate;
}


/**
 * bonobo_control_frame_get_autoactivate:
 * @frame: A #BonoboControlFrame object.
 *
 * Returns: A boolean which indicates whether or not @frame is
 * set to automatically activate its Control.  See
 * bonobo_control_frame_set_autoactivate().
 */
gboolean
bonobo_control_frame_get_autoactivate (BonoboControlFrame *frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), FALSE);

	return frame->priv->autoactivate;
}

static Bonobo_Gtk_State
bonobo_control_frame_state_to_corba (const GtkStateType state)
{
	switch (state) {
	case GTK_STATE_NORMAL:
		return Bonobo_Gtk_StateNormal;

	case GTK_STATE_ACTIVE:
		return Bonobo_Gtk_StateActive;

	case GTK_STATE_PRELIGHT:
		return Bonobo_Gtk_StatePrelight;

	case GTK_STATE_SELECTED:
		return Bonobo_Gtk_StateSelected;

	case GTK_STATE_INSENSITIVE:
		return Bonobo_Gtk_StateInsensitive;

	default:
		g_warning ("bonobo_control_frame_state_to_corba: Unknown state: %d", (gint) state);
		return Bonobo_Gtk_StateNormal;
	}
}

/**
 * bonobo_control_frame_control_set_state:
 * @frame: A #BonoboControlFrame object which is bound to a
 * remote #BonoboControl.
 * @state: A #GtkStateType value, specifying the widget state to apply
 * to the remote control.
 *
 * Proxies @state to the control bound to @frame.
 */
void
bonobo_control_frame_control_set_state (BonoboControlFrame  *frame,
					GtkStateType         state)
{
	Bonobo_Gtk_State  corba_state;
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));
	g_return_if_fail (frame->priv->control != CORBA_OBJECT_NIL);

	corba_state = bonobo_control_frame_state_to_corba (state);

	CORBA_exception_init (&ev);

	Bonobo_Control_setState (frame->priv->control, corba_state, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (frame),
			frame->priv->control, &ev);
	}

	CORBA_exception_free (&ev);
}

/**
 * bonobo_control_frame_set_autostate:
 * @frame: A #BonoboControlFrame object.
 * @autostate: Whether or not GtkWidget state changes should be
 * automatically propagated down to the Control.
 *
 * Changes whether or not @frame automatically proxies
 * state changes to its associated control.  The default mode
 * is for the control frame to autopropagate.
 */
void
bonobo_control_frame_set_autostate (BonoboControlFrame  *frame,
				    gboolean             autostate)
{
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	frame->priv->autostate = autostate;
}

/**
 * bonobo_control_frame_get_autostate:
 * @frame: A #BonoboControlFrame object.
 *
 * Returns: Whether or not this control frame will automatically
 * proxy GtkState changes to its associated Control.
 */
gboolean
bonobo_control_frame_get_autostate (BonoboControlFrame *frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), FALSE);

	return frame->priv->autostate;
}


/**
 * bonobo_control_frame_get_ui_container:
 * @frame: A BonoboControlFrame object.

 * Returns: The Bonobo_UIContainer object reference associated with this
 * ControlFrame.  This ui_container is specified when the ControlFrame is
 * created.  See bonobo_control_frame_new().
 */
Bonobo_UIContainer
bonobo_control_frame_get_ui_container (BonoboControlFrame *frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), CORBA_OBJECT_NIL);

	return frame->priv->ui_container;
}

/**
 * bonobo_control_frame_set_ui_container:
 * @frame: A BonoboControlFrame object.
 * @uic: A Bonobo_UIContainer object reference.
 *
 * Associates a new %Bonobo_UIContainer object with this ControlFrame. This
 * is only allowed while the Control is deactivated.
 */
void
bonobo_control_frame_set_ui_container (BonoboControlFrame *frame,
				       Bonobo_UIContainer  ui_container,
				       CORBA_Environment  *opt_ev)
{
	CORBA_Environment *ev, tmp_ev;
	Bonobo_UIContainer old_ui_container;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));
	g_return_if_fail (frame->priv->activated == FALSE);

	old_ui_container = frame->priv->ui_container;

	if (old_ui_container == ui_container)
		return;

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	/* See ui-faq.txt if this dies on you. */
	if (ui_container != CORBA_OBJECT_NIL) {
		g_assert (CORBA_Object_is_a (
			ui_container, "IDL:Bonobo/UIContainer:1.0", ev));

		frame->priv->ui_container = bonobo_object_dup_ref (
			ui_container, ev);
	} else
		frame->priv->ui_container = CORBA_OBJECT_NIL;

	if (old_ui_container)
		bonobo_object_release_unref (old_ui_container, ev);

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);
}

/**
 * bonobo_control_frame_bind_to_control:
 * @frame: A BonoboControlFrame object.
 * @control: The CORBA object for the BonoboControl embedded
 * in this BonoboControlFrame.
 * @opt_ev: Optional exception environment
 *
 * Associates @control with this @frame.
 */
void
bonobo_control_frame_bind_to_control (BonoboControlFrame *frame,
				      Bonobo_Control      control,
				      CORBA_Environment  *opt_ev)
{
	CORBA_Environment *ev, tmp_ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	if (control == frame->priv->control)
		return;

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	g_object_ref (frame);

	if (frame->priv->control != CORBA_OBJECT_NIL) {
		if (!frame->priv->inproc_control)
			ORBit_small_unlisten_for_broken (
				frame->priv->control,
				G_CALLBACK (control_connection_died_cb));

		/* Unset ourselves as the frame */
		Bonobo_Control_setFrame (frame->priv->control,
					 CORBA_OBJECT_NIL, ev);

		if (frame->priv->control != CORBA_OBJECT_NIL)
			bonobo_object_release_unref (frame->priv->control, ev);

		CORBA_exception_free (ev);
	}

	if (control == CORBA_OBJECT_NIL) {
		frame->priv->control = CORBA_OBJECT_NIL;
		frame->priv->inproc_control = NULL;
	} else {
		frame->priv->control = bonobo_object_dup_ref (control, ev);

		frame->priv->inproc_control = (BonoboControl *)
			bonobo_object (ORBit_small_get_servant (control));

		if (!frame->priv->inproc_control)
			bonobo_control_add_listener (
				frame->priv->control,
				G_CALLBACK (control_connection_died_cb),
				frame, ev);

		Bonobo_Control_setFrame (
			frame->priv->control,
			BONOBO_OBJREF (frame), ev);

		bonobo_control_frame_get_remote_window (frame, ev);
	}

	g_object_unref (frame);

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);
}

/**
 * bonobo_control_frame_get_control:
 * @frame: A BonoboControlFrame which is bound to a remote
 * BonoboControl.
 *
 * Returns: The Bonobo_Control CORBA interface for the remote Control
 * which is bound to @frame.  See also
 * bonobo_control_frame_bind_to_control().
 */
Bonobo_Control
bonobo_control_frame_get_control (BonoboControlFrame *frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), CORBA_OBJECT_NIL);

	return frame->priv->control;
}

/**
 * bonobo_control_frame_get_widget:
 * @frame: The BonoboControlFrame whose widget is being requested.a
 *
 * Use this function when you want to embed a BonoboControl into your
 * container's widget hierarchy.  Once you have bound the
 * BonoboControlFrame to a remote BonoboControl, place the widget
 * returned by bonobo_control_frame_get_widget() into your widget
 * hierarchy and the control will appear in your application.
 *
 * Returns: A GtkWidget which has the remote BonoboControl physically
 * inside it.
 */
GtkWidget *
bonobo_control_frame_get_widget (BonoboControlFrame *frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), NULL);

	return frame->priv->socket;
}

/**
 * bonobo_control_frame_set_propbag:
 * @frame: A BonoboControlFrame object.
 * @propbag: A BonoboPropertyBag which will hold @frame's
 * ambient properties.
 *
 * Makes @frame use @propbag for its ambient properties.  When
 * @frame's Control requests the ambient properties, it will
 * get them from @propbag.
 */

void
bonobo_control_frame_set_propbag (BonoboControlFrame  *frame,
				  BonoboPropertyBag   *propbag)
{
	BonoboPropertyBag *old_pb;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));
	g_return_if_fail (propbag == NULL ||
			  BONOBO_IS_PROPERTY_BAG (propbag));

	old_pb = frame->priv->propbag;

	if (old_pb == propbag)
		return;

	frame->priv->propbag = bonobo_object_ref ((BonoboObject *) propbag);
	bonobo_object_unref ((BonoboObject *) old_pb);
}

/**
 * bonobo_control_frame_get_propbag:
 * @frame: A BonoboControlFrame object whose PropertyBag has
 * been set.
 *
 * Returns: The BonoboPropertyBag object which has been associated with
 * @frame.
 */
BonoboPropertyBag *
bonobo_control_frame_get_propbag (BonoboControlFrame  *frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), NULL);

	return frame->priv->propbag;
}

/**
 * bonobo_control_frame_get_control_property_bag:
 * @frame: the control frame
 * @ev: CORBA exception environment
 * 
 * This retrives a Bonobo_PropertyBag reference from its
 * associated Bonobo Control
 *
 * Return value: CORBA property bag reference or CORBA_OBJECT_NIL
 **/
Bonobo_PropertyBag
bonobo_control_frame_get_control_property_bag (BonoboControlFrame *frame,
					       CORBA_Environment  *opt_ev)
{
	Bonobo_PropertyBag pbag;
	Bonobo_Control control;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (frame != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), NULL);

	if (opt_ev)
		real_ev = opt_ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	control = frame->priv->control;

	/* FIXME: we could cache this here - is it called a lot ? */
	pbag = Bonobo_Control_getProperties (control, real_ev);

	if (BONOBO_EX (real_ev)) {
		if (!opt_ev)
			CORBA_exception_free (&tmp_ev);
		pbag = CORBA_OBJECT_NIL;
	}

	return pbag;
}

void
bonobo_control_frame_size_request (BonoboControlFrame *frame,
				   GtkRequisition     *requisition,
				   CORBA_Environment  *opt_ev)
{
	CORBA_Environment      *ev, tmp_ev;
	Bonobo_Gtk_Requisition  req;

	g_return_if_fail (requisition != NULL);
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	if (frame->priv->control == CORBA_OBJECT_NIL) {
		/* We haven't been bound to a control yet, so return "I don't
		 * care about what I get assigned".
		 */
		requisition->width = requisition->height = 1;
		return;
	}

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	req = Bonobo_Control_getDesiredSize (frame->priv->control, ev);

	if (BONOBO_EX (ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (frame),
			(CORBA_Object) frame->priv->control, ev);

		req.width = req.height = 1;
	}

	requisition->width  = req.width;
	requisition->height = req.height;

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);
}

/**
 * bonobo_control_frame_focus:
 * @frame: A control frame.
 * @direction: Direction in which to change focus.
 * 
 * Proxies a #GtkContainer::focus() request to the embedded control.  This is an
 * internal function and it should only really be ever used by the #BonoboSocket
 * implementation.
 * 
 * Return value: TRUE if the child kept the focus, FALSE if focus should be
 * passed on to the next widget.
 **/
gboolean
bonobo_control_frame_focus (BonoboControlFrame *frame,
			    GtkDirectionType    direction)
{
	BonoboControlFramePrivate *priv;
	CORBA_Environment ev;
	gboolean result;
	Bonobo_Gtk_Direction corba_direction;

	g_return_val_if_fail (frame != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), FALSE);

	priv = frame->priv;

	if (priv->control == CORBA_OBJECT_NIL)
		return FALSE;

	switch (direction) {
	case GTK_DIR_TAB_FORWARD:
		corba_direction = Bonobo_Gtk_DirectionTabForward;
		break;

	case GTK_DIR_TAB_BACKWARD:
		corba_direction = Bonobo_Gtk_DirectionTabBackward;
		break;

	case GTK_DIR_UP:
		corba_direction = Bonobo_Gtk_DirectionUp;
		break;

	case GTK_DIR_DOWN:
		corba_direction = Bonobo_Gtk_DirectionDown;
		break;

	case GTK_DIR_LEFT:
		corba_direction = Bonobo_Gtk_DirectionLeft;
		break;

	case GTK_DIR_RIGHT:
		corba_direction = Bonobo_Gtk_DirectionRight;
		break;

	default:
		g_assert_not_reached ();
		return FALSE;
	}

	CORBA_exception_init (&ev);

	result = Bonobo_Control_focus (priv->control, corba_direction, &ev);
	if (BONOBO_EX (&ev)) {
		g_message ("bonobo_control_frame_focus(): Exception while issuing focus "
			   "request: `%s'", bonobo_exception_get_text (&ev));
		result = FALSE;
	}

	CORBA_exception_free (&ev);

	return result;
}


void
bonobo_control_frame_set_socket (BonoboControlFrame *frame,
				 BonoboSocket       *socket)
{
	BonoboSocket *old_socket;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));

	if ((BonoboSocket *) frame->priv->socket == socket)
		return;

	old_socket = (BonoboSocket *) frame->priv->socket;

	if (socket)
		frame->priv->socket = g_object_ref (socket);
	else
		frame->priv->socket = NULL;

	if (old_socket) {
		bonobo_socket_set_control_frame (
			BONOBO_SOCKET (old_socket), NULL);
		g_object_unref (old_socket);
	}

	if (socket)
		bonobo_socket_set_control_frame (socket, frame);
}

BonoboSocket *
bonobo_control_frame_get_socket (BonoboControlFrame *frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (frame), NULL);

	return (BonoboSocket *) frame->priv->socket;
}

BonoboUIComponent *
bonobo_control_frame_get_popup_component (BonoboControlFrame *control_frame,
					  CORBA_Environment  *opt_ev)
{
	CORBA_Environment *ev, tmp_ev;
	BonoboUIComponent *ui_component;
	Bonobo_UIContainer popup_container;

	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), NULL);

	if (control_frame->priv->control == CORBA_OBJECT_NIL)
		return NULL;

	ui_component = bonobo_ui_component_new_default ();

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	popup_container = Bonobo_Control_getPopupContainer (
		control_frame->priv->control, ev);

	if (BONOBO_EX (ev))
		return NULL;

	bonobo_ui_component_set_container (ui_component, popup_container, ev);

	Bonobo_Unknown_unref (popup_container, ev);

	if (ev->_major != CORBA_NO_EXCEPTION) {
		bonobo_object_unref (BONOBO_OBJECT (ui_component));
		ui_component = NULL;
	}

	if (!opt_ev) {
		CORBA_exception_free (ev);
	}

	return ui_component;
}
