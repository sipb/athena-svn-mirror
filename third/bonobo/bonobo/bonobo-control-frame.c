/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo control frame object.
 *
 * Authors:
 *   Nat Friedman      (nat@helixcode.com)
 *   Miguel de Icaza   (miguel@kernel.org)
 *   Maciej Stachowiak (mjs@eazel.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
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

enum {
	ACTIVATED,
	ACTIVATE_URI,
	LAST_SIGNAL
};

static guint control_frame_signals [LAST_SIGNAL];

/* Parent object class in GTK hierarchy */
static BonoboObjectClass *bonobo_control_frame_parent_class;

/* The entry point vectors for the server we provide */
POA_Bonobo_ControlFrame__vepv bonobo_control_frame_vepv;

struct _BonoboControlFramePrivate {
	Bonobo_Control	   control;
	GtkWidget         *container;
	GtkWidget	  *socket;
	Bonobo_UIContainer ui_container;
	BonoboPropertyBag *propbag;
	gboolean           autoactivate;
	gboolean           autostate;
	gboolean           activated;
};

static void
impl_Bonobo_ControlFrame_activated (PortableServer_Servant  servant,
				    const CORBA_boolean     state,
				    CORBA_Environment      *ev)
{
	BonoboControlFrame *control_frame = BONOBO_CONTROL_FRAME (bonobo_object_from_servant (servant));

	gtk_signal_emit (GTK_OBJECT (control_frame),
			 control_frame_signals [ACTIVATED], state);
}

static Bonobo_UIContainer
impl_Bonobo_ControlFrame_getUIHandler (PortableServer_Servant  servant,
					 CORBA_Environment      *ev)
{
	BonoboControlFrame *control_frame = BONOBO_CONTROL_FRAME (bonobo_object_from_servant (servant));

	if (control_frame->priv->ui_container == NULL)
		return CORBA_OBJECT_NIL;

	return bonobo_object_dup_ref (control_frame->priv->ui_container, ev);
}

static Bonobo_PropertyBag
impl_Bonobo_ControlFrame_getAmbientProperties (PortableServer_Servant  servant,
						 CORBA_Environment      *ev)
{
	BonoboControlFrame *control_frame = BONOBO_CONTROL_FRAME (bonobo_object_from_servant (servant));
	Bonobo_PropertyBag corba_propbag;

	if (control_frame->priv->propbag == NULL)
		return CORBA_OBJECT_NIL;

	corba_propbag = (Bonobo_PropertyBag)
		bonobo_object_corba_objref (BONOBO_OBJECT (control_frame->priv->propbag));

	return bonobo_object_dup_ref (corba_propbag, ev);
}

static void
impl_Bonobo_ControlFrame_queueResize (PortableServer_Servant  servant,
				       CORBA_Environment      *ev)
{
	/*
	 * Nothing.
	 *
	 * In the Gnome implementation of Bonobo, all size negotiation
	 * is handled by GtkPlug/GtkSocket for us.
	 */
}

static void
impl_Bonobo_ControlFrame_activateURI (PortableServer_Servant  servant,
				       const CORBA_char       *uri,
				       CORBA_boolean           relative,
				       CORBA_Environment      *ev)
{
	BonoboControlFrame *control_frame = BONOBO_CONTROL_FRAME (bonobo_object_from_servant (servant));

	gtk_signal_emit (GTK_OBJECT (control_frame),
			 control_frame_signals [ACTIVATE_URI],
			 (const char *) uri, (gboolean) relative);
}

static CORBA_Object
create_bonobo_control_frame (BonoboObject *object)
{
	POA_Bonobo_ControlFrame *servant;
	CORBA_Environment ev;
	
	servant = (POA_Bonobo_ControlFrame *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_control_frame_vepv;

	CORBA_exception_init (&ev);
	POA_Bonobo_ControlFrame__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return bonobo_object_activate_servant (object, servant);
}


static gint
bonobo_control_frame_autoactivate_focus_in (GtkWidget     *widget,
					    GdkEventFocus *focus,
					    gpointer       user_data)
{
	BonoboControlFrame *control_frame = user_data;
	
	if (! control_frame->priv->autoactivate)
		return FALSE;

	bonobo_control_frame_control_activate (control_frame);

	return FALSE;
}

static gint
bonobo_control_frame_autoactivate_focus_out (GtkWidget     *widget,
					     GdkEventFocus *focus,
					     gpointer       user_data)
{
	BonoboControlFrame *control_frame = user_data;

	if (! control_frame->priv->autoactivate)
		return FALSE;
	
	bonobo_control_frame_control_deactivate (control_frame);

	return FALSE;
}


static void
bonobo_control_frame_socket_state_changed (GtkWidget    *socket,
					   GtkStateType  previous_state,
					   gpointer      user_data)
{
	BonoboControlFrame *control_frame = user_data;

	if (! control_frame->priv->autostate)
		return;

	bonobo_control_frame_control_set_state (
		control_frame,
		GTK_WIDGET_STATE (control_frame->priv->socket));
}


static void
bonobo_control_frame_socket_destroy (GtkWidget          *socket,
				     BonoboControlFrame *control_frame)
{
	gtk_widget_unref (control_frame->priv->socket);
	control_frame->priv->socket = NULL;
}


static void
bonobo_control_frame_set_remote_window (GtkWidget          *socket,
					BonoboControlFrame *control_frame)
{
	Bonobo_Control control = bonobo_control_frame_get_control (control_frame);
	CORBA_Environment ev;
	Bonobo_Control_windowId id;

	/*
	 * If we are not yet bound to a remote control, don't do
	 * anything.
	 */
	if (control == CORBA_OBJECT_NIL || !socket)
		return;

	/*
	 * Sync the server, since the XID may have been created on the
	 * client side without communication with the X server.
	 */
	/* FIXME: not very convinced about this flush */
	gdk_flush ();

	/*
	 * Otherwise, pass the window ID of our GtkSocket to the
	 * remote Control.
	 */
	CORBA_exception_init (&ev);
	id = bonobo_control_windowid_from_x11 (
		GDK_WINDOW_XWINDOW (socket->window));

	Bonobo_Control_setWindowId (control, id, &ev);
	g_free (id);
	if (BONOBO_EX (&ev))
		bonobo_object_check_env (BONOBO_OBJECT (control_frame), control, &ev);
	CORBA_exception_free (&ev);
}

static void
bonobo_control_frame_create_socket (BonoboControlFrame *control_frame)
{
	/*
	 * Now create the GtkSocket which will be used to embed
	 * the Control.
	 */
	control_frame->priv->socket = bonobo_socket_new ();
	gtk_widget_show (control_frame->priv->socket);

	bonobo_socket_set_control_frame (
		BONOBO_SOCKET (control_frame->priv->socket),
		control_frame);

	/*
	 * Connect to the focus events on the socket so
	 * that we can provide the autoactivation feature.
	 */
	gtk_signal_connect (GTK_OBJECT (control_frame->priv->socket),
			    "focus_in_event",
			    GTK_SIGNAL_FUNC (bonobo_control_frame_autoactivate_focus_in),
			    control_frame);

	gtk_signal_connect (GTK_OBJECT (control_frame->priv->socket),
			    "focus_out_event",
			    GTK_SIGNAL_FUNC (bonobo_control_frame_autoactivate_focus_out),
			    control_frame);

	/*
	 * Setup a handler to proxy state changes.
	 */
	gtk_signal_connect (GTK_OBJECT (control_frame->priv->socket),
			    "state_changed",
			    bonobo_control_frame_socket_state_changed,
			    control_frame);

	/*
	 * Setup a handler for socket destroy.
	 */
	gtk_signal_connect (GTK_OBJECT (control_frame->priv->socket),
			    "destroy",
			    bonobo_control_frame_socket_destroy,
			    control_frame);

	/*
	 * Ref the socket so we can handle the case of the control destroying it for bypass.
	 */
	gtk_object_ref (GTK_OBJECT (control_frame->priv->socket));


	/*
	 * Pack into the hack box
	 */
	gtk_box_pack_start (GTK_BOX (control_frame->priv->container),
			    control_frame->priv->socket,
			    TRUE, TRUE, 0);
}
				

/**
 * bonobo_control_frame_construct:
 * @control_frame: The #BonoboControlFrame object to be initialized.
 * @corba_control_frame: A CORBA object for the Bonobo_ControlFrame interface.
 * @ui_container: A CORBA object for the UIContainer for the container application.
 *
 * Initializes @control_frame with the parameters.
 *
 * Returns: the initialized BonoboControlFrame object @control_frame that implements the
 * Bonobo::ControlFrame CORBA service.
 */
BonoboControlFrame *
bonobo_control_frame_construct (BonoboControlFrame  *control_frame,
				Bonobo_ControlFrame  corba_control_frame,
				Bonobo_UIContainer   ui_container)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), NULL);

	bonobo_object_construct (BONOBO_OBJECT (control_frame), corba_control_frame);

	/*
	 * See ui-faq.txt if this dies on you.
	 */
	if (ui_container != CORBA_OBJECT_NIL) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		g_assert (CORBA_Object_is_a (ui_container, "IDL:Bonobo/UIContainer:1.0", &ev));
		control_frame->priv->ui_container = bonobo_object_dup_ref (ui_container, &ev);
		CORBA_exception_free (&ev);
	} else
		control_frame->priv->ui_container = ui_container;

	/*
	 * Finally, create a box to hold the socket; this no-window
	 * container is needed solely for the sake of bypassing
	 * plug/socket in the local case.
	 */
	control_frame->priv->container = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (control_frame->priv->container), 0);
	gtk_widget_ref (control_frame->priv->container);
	gtk_object_sink (GTK_OBJECT (control_frame->priv->container));
	gtk_widget_show (control_frame->priv->container);

	bonobo_control_frame_create_socket (control_frame);

	return control_frame;
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
	Bonobo_ControlFrame corba_control_frame;
	BonoboControlFrame *control_frame;

	control_frame = gtk_type_new (BONOBO_CONTROL_FRAME_TYPE);

	corba_control_frame = create_bonobo_control_frame (BONOBO_OBJECT (control_frame));
	if (corba_control_frame == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (control_frame));
		return NULL;
	}

	return bonobo_control_frame_construct (control_frame, corba_control_frame, ui_container);
}

static void
bonobo_control_frame_destroy (GtkObject *object)
{
	BonoboControlFrame *control_frame = BONOBO_CONTROL_FRAME (object);

	gtk_widget_destroy (control_frame->priv->container);

	if (control_frame->priv->control != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (control_frame->priv->control, NULL);
	control_frame->priv->control = CORBA_OBJECT_NIL;

	if (control_frame->priv->socket) {
		bonobo_socket_set_control_frame (
			BONOBO_SOCKET (control_frame->priv->socket), NULL);
		gtk_signal_disconnect_by_data (
			GTK_OBJECT (control_frame->priv->socket),
			control_frame);
		gtk_widget_unref (control_frame->priv->socket);
		control_frame->priv->socket = NULL;
	}

	if (control_frame->priv->ui_container != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (control_frame->priv->ui_container, NULL);
	control_frame->priv->ui_container = CORBA_OBJECT_NIL;

	g_free (control_frame->priv);
	control_frame->priv = NULL;
	
	GTK_OBJECT_CLASS (bonobo_control_frame_parent_class)->destroy (object);
}

static void
bonobo_control_frame_activated (BonoboControlFrame *control_frame, gboolean state)
{
	g_return_if_fail (control_frame != NULL);
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));

	control_frame->priv->activated = state;
}

/**
 * bonobo_control_frame_get_epv:
 *
 * Returns: The EPV for the default BonoboControlFrame implementation. 
 */
POA_Bonobo_ControlFrame__epv *
bonobo_control_frame_get_epv (void)
{
	POA_Bonobo_ControlFrame__epv *epv;

	epv = g_new0 (POA_Bonobo_ControlFrame__epv, 1);

	epv->activated            = impl_Bonobo_ControlFrame_activated;
	epv->getUIHandler         = impl_Bonobo_ControlFrame_getUIHandler;
	epv->queueResize          = impl_Bonobo_ControlFrame_queueResize;
	epv->activateURI          = impl_Bonobo_ControlFrame_activateURI;
	epv->getAmbientProperties = impl_Bonobo_ControlFrame_getAmbientProperties;

	return epv;
}

static void
init_control_frame_corba_class (void)
{
	/* Setup the vector of epvs */
	bonobo_control_frame_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_control_frame_vepv.Bonobo_ControlFrame_epv = bonobo_control_frame_get_epv ();
}

typedef void (*GnomeSignal_NONE__STRING_BOOL) (GtkObject *, const char *, gboolean, gpointer);

static void
gnome_marshal_NONE__STRING_BOOL (GtkObject     *object,
				 GtkSignalFunc  func,
				 gpointer       func_data,
				 GtkArg        *args)
{
	GnomeSignal_NONE__STRING_BOOL rfunc;

	rfunc = (GnomeSignal_NONE__STRING_BOOL) func;
	(*rfunc)(object, GTK_VALUE_STRING (args [0]), GTK_VALUE_BOOL (args [1]), func_data);
}

static void
bonobo_control_frame_class_init (BonoboControlFrameClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *)klass;

	bonobo_control_frame_parent_class = gtk_type_class (BONOBO_OBJECT_TYPE);

	control_frame_signals [ACTIVATED] =
		gtk_signal_new ("activated",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboControlFrameClass, activated),
				gtk_marshal_NONE__BOOL,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_BOOL);

	
	control_frame_signals [ACTIVATE_URI] =
		gtk_signal_new ("activate_uri",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboControlFrameClass, activate_uri),
				gnome_marshal_NONE__STRING_BOOL,
				GTK_TYPE_NONE, 2,
				GTK_TYPE_STRING, GTK_TYPE_BOOL);
	
	gtk_object_class_add_signals (
		object_class,
		control_frame_signals,
		LAST_SIGNAL);

	klass->activated = bonobo_control_frame_activated;

	object_class->destroy = bonobo_control_frame_destroy;

	init_control_frame_corba_class ();
}

static void
bonobo_control_frame_init (BonoboObject *object)
{
	BonoboControlFrame *control_frame = BONOBO_CONTROL_FRAME (object);

	control_frame->priv               = g_new0 (BonoboControlFramePrivate, 1);
	control_frame->priv->autoactivate = FALSE;
	control_frame->priv->autostate    = TRUE;
}

/**
 * bonobo_control_frame_get_type:
 *
 * Returns: The GtkType for the BonoboControlFrame class.  */
GtkType
bonobo_control_frame_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboControlFrame",
			sizeof (BonoboControlFrame),
			sizeof (BonoboControlFrameClass),
			(GtkClassInitFunc) bonobo_control_frame_class_init,
			(GtkObjectInitFunc) bonobo_control_frame_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_control_frame_control_activate:
 * @control_frame: The BonoboControlFrame object whose control should be
 * activated.
 *
 * Activates the BonoboControl embedded in @control_frame by calling the
 * activate() #Bonobo_Control interface method on it.
 */
void
bonobo_control_frame_control_activate (BonoboControlFrame *control_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));

	/*
	 * Check that this ControLFrame actually has a Control associated
	 * with it.
	 */
	g_return_if_fail (control_frame->priv->control != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	Bonobo_Control_activate (control_frame->priv->control, TRUE, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (control_frame),
			(CORBA_Object) control_frame->priv->control, &ev);

	}

	CORBA_exception_free (&ev);
}


/**
 * bonobo_control_frame_control_deactivate:
 * @control_frame: The BonoboControlFrame object whose control should be
 * deactivated.
 *
 * Deactivates the BonoboControl embedded in @control_frame by calling
 * the activate() CORBA method on it with the parameter %FALSE.
 */
void
bonobo_control_frame_control_deactivate (BonoboControlFrame *control_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));

	/*
	 * Check that this ControlFrame actually has a Control associated
	 * with it.
	 */
	g_return_if_fail (control_frame->priv->control != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	Bonobo_Control_activate (control_frame->priv->control, FALSE, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (control_frame),
			(CORBA_Object) control_frame->priv->control, &ev);
	}

	CORBA_exception_free (&ev);
}

/**
 * bonobo_control_frame_set_autoactivate:
 * @control_frame: A BonoboControlFrame object.
 * @autoactivate: A flag which indicates whether or not the
 * ControlFrame should automatically perform activation on the Control
 * to which it is bound.
 *
 * Modifies the autoactivate behavior of @control_frame.  If
 * @control_frame is set to autoactivate, then it will automatically
 * send an "activate" message to the Control to which it is bound when
 * it gets a focus-in event, and a "deactivate" message when it gets a
 * focus-out event.  Autoactivation is off by default.
 */
void
bonobo_control_frame_set_autoactivate (BonoboControlFrame  *control_frame,
				       gboolean             autoactivate)
{
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));

	control_frame->priv->autoactivate = autoactivate;
}


/**
 * bonobo_control_frame_get_autoactivate:
 * @control_frame: A #BonoboControlFrame object.
 *
 * Returns: A boolean which indicates whether or not @control_frame is
 * set to automatically activate its Control.  See
 * bonobo_control_frame_set_autoactivate().
 */
gboolean
bonobo_control_frame_get_autoactivate (BonoboControlFrame *control_frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), FALSE);

	return control_frame->priv->autoactivate;
}

static Bonobo_Control_State
bonobo_control_frame_state_to_corba (const GtkStateType state)
{
	switch (state) {
	case GTK_STATE_NORMAL:
		return Bonobo_Control_StateNormal;

	case GTK_STATE_ACTIVE:
		return Bonobo_Control_StateActive;

	case GTK_STATE_PRELIGHT:
		return Bonobo_Control_StatePrelight;

	case GTK_STATE_SELECTED:
		return Bonobo_Control_StateSelected;

	case GTK_STATE_INSENSITIVE:
		return Bonobo_Control_StateInsensitive;

	default:
		g_warning ("bonobo_control_frame_state_to_corba: Unknown state: %d", (gint) state);
		return Bonobo_Control_StateNormal;
	}
}

/**
 * bonobo_control_frame_control_set_state:
 * @control_frame: A #BonoboControlFrame object which is bound to a
 * remote #BonoboControl.
 * @state: A #GtkStateType value, specifying the widget state to apply
 * to the remote control.
 *
 * Proxies @state to the control bound to @control_frame.
 */
void
bonobo_control_frame_control_set_state (BonoboControlFrame  *control_frame,
					GtkStateType         state)
{
	Bonobo_Control_State  corba_state;
	CORBA_Environment     ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));
	g_return_if_fail (control_frame->priv->control != CORBA_OBJECT_NIL);

	corba_state = bonobo_control_frame_state_to_corba (state);

	CORBA_exception_init (&ev);

	Bonobo_Control_setState (control_frame->priv->control, corba_state, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (control_frame),
			control_frame->priv->control, &ev);
	}

	CORBA_exception_free (&ev);
}

/**
 * bonobo_control_frame_set_autostate:
 * @control_frame: A #BonoboControlFrame object.
 * @autostate: Whether or not GtkWidget state changes should be
 * automatically propagated down to the Control.
 *
 * Changes whether or not @control_frame automatically proxies
 * state changes to its associated control.  The default mode
 * is for the control frame to autopropagate.
 */
void
bonobo_control_frame_set_autostate (BonoboControlFrame  *control_frame,
				    gboolean             autostate)
{
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));

	control_frame->priv->autostate = autostate;
}

/**
 * bonobo_control_frame_get_autostate:
 * @control_frame: A #BonoboControlFrame object.
 *
 * Returns: Whether or not this control frame will automatically
 * proxy GtkState changes to its associated Control.
 */
gboolean
bonobo_control_frame_get_autostate (BonoboControlFrame *control_frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), FALSE);

	return control_frame->priv->autostate;
}


/**
 * bonobo_control_frame_get_ui_container:
 * @control_frame: A BonoboControlFrame object.

 * Returns: The Bonobo_UIContainer object reference associated with this
 * ControlFrame.  This ui_container is specified when the ControlFrame is
 * created.  See bonobo_control_frame_new().
 */
Bonobo_UIContainer
bonobo_control_frame_get_ui_container (BonoboControlFrame *control_frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), CORBA_OBJECT_NIL);

	return control_frame->priv->ui_container;
}

/**
 * bonobo_control_frame_set_ui_container:
 * @control_frame: A BonoboControlFrame object.
 * @uic: A Bonobo_UIContainer object reference.
 *
 * Associates a new %Bonobo_UIContainer object with this ControlFrame. This
 * is only allowed while the Control is deactivated.
 */
void
bonobo_control_frame_set_ui_container (BonoboControlFrame *control_frame, Bonobo_UIContainer ui_container)
{
	Bonobo_UIContainer old_ui_container;

	g_return_if_fail (control_frame != NULL);
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));
	g_return_if_fail (control_frame->priv->activated == FALSE);

	old_ui_container = control_frame->priv->ui_container;

	if (ui_container != CORBA_OBJECT_NIL) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		g_assert (CORBA_Object_is_a (ui_container, "IDL:Bonobo/UIContainer:1.0", &ev));
		control_frame->priv->ui_container = bonobo_object_dup_ref (ui_container, &ev);
		CORBA_exception_free (&ev);
	} else
		control_frame->priv->ui_container = CORBA_OBJECT_NIL;

	if (old_ui_container)
		bonobo_object_release_unref (old_ui_container, NULL);
}


/**
 * bonobo_control_frame_bind_to_control:
 * @control_frame: A BonoboControlFrame object.
 * @control: The CORBA object for the BonoboControl embedded
 * in this BonoboControlFrame.
 *
 * Associates @control with this @control_frame.
 */
void
bonobo_control_frame_bind_to_control (BonoboControlFrame *control_frame, Bonobo_Control control)
{
	CORBA_Environment ev;

	g_return_if_fail (control != CORBA_OBJECT_NIL);
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));

	/*
	 * Keep a local handle to the Control.
	 */
	if (control_frame->priv->control != CORBA_OBJECT_NIL)
		g_warning ("FIXME: leaking control reference");

	CORBA_exception_init (&ev);

	control_frame->priv->control = bonobo_object_dup_ref (control, &ev);

	/*
	 * Introduce ourselves to the Control.
	 */
	Bonobo_Control_setFrame (control,
				 bonobo_object_corba_objref (BONOBO_OBJECT (control_frame)),
				 &ev);
	if (BONOBO_EX (&ev))
		bonobo_object_check_env (BONOBO_OBJECT (control_frame), control, &ev);
	CORBA_exception_free (&ev);

	/* 
	 * Re-create the socket if it got destroyed by the Control before.
	 */
	if (control_frame->priv->socket == NULL) 
		bonobo_control_frame_create_socket (control_frame);

	/*
	 * If the socket is realized, then we transfer the
	 * window ID to the remote control.
	 */
	if (GTK_WIDGET_REALIZED (control_frame->priv->socket))
		bonobo_control_frame_set_remote_window (
			control_frame->priv->socket, control_frame);
}

/**
 * bonobo_control_frame_get_control:
 * @control_frame: A BonoboControlFrame which is bound to a remote
 * BonoboControl.
 *
 * Returns: The Bonobo_Control CORBA interface for the remote Control
 * which is bound to @frame.  See also
 * bonobo_control_frame_bind_to_control().
 */
Bonobo_Control
bonobo_control_frame_get_control (BonoboControlFrame *control_frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), CORBA_OBJECT_NIL);

	return control_frame->priv->control;
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
bonobo_control_frame_get_widget (BonoboControlFrame *control_frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), NULL);

	return control_frame->priv->container;
}

/**
 * bonobo_control_frame_set_propbag:
 * @control_frame: A BonoboControlFrame object.
 * @propbag: A BonoboPropertyBag which will hold @control_frame's
 * ambient properties.
 *
 * Makes @control_frame use @propbag for its ambient properties.  When
 * @control_frame's Control requests the ambient properties, it will
 * get them from @propbag.
 */

void
bonobo_control_frame_set_propbag (BonoboControlFrame  *control_frame,
				 BonoboPropertyBag   *propbag)
{
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG (propbag));

	control_frame->priv->propbag = propbag;
}

/**
 * bonobo_control_frame_get_propbag:
 * @control_frame: A BonoboControlFrame object whose PropertyBag has
 * been set.
 *
 * Returns: The BonoboPropertyBag object which has been associated with
 * @control_frame.
 */
BonoboPropertyBag *
bonobo_control_frame_get_propbag (BonoboControlFrame  *control_frame)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), NULL);

	return control_frame->priv->propbag;
}

/**
 * bonobo_control_frame_get_control_property_bag:
 */
Bonobo_PropertyBag
bonobo_control_frame_get_control_property_bag (BonoboControlFrame *control_frame,
					       CORBA_Environment  *ev)
{
	Bonobo_PropertyBag pbag;
	Bonobo_Control control;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (control_frame != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame), NULL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	control = control_frame->priv->control;

	pbag = Bonobo_Control_getProperties (control, real_ev);

	if (BONOBO_EX (real_ev)) {
		if (!ev)
			CORBA_exception_free (&tmp_ev);
		pbag = CORBA_OBJECT_NIL;
	}

	return pbag;
}

void
bonobo_control_frame_size_request (BonoboControlFrame *control_frame,
				   int *desired_width, int *desired_height)
{
	CORBA_Environment ev;
	CORBA_short width, height;

	g_return_if_fail (control_frame != NULL);
	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (control_frame));
	g_return_if_fail (control_frame->priv->control != CORBA_OBJECT_NIL);
	g_return_if_fail (desired_width != NULL);
	g_return_if_fail (desired_height != NULL);

	CORBA_exception_init (&ev);

	Bonobo_Control_getDesiredSize (control_frame->priv->control,
				       &width, &height, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (control_frame),
			(CORBA_Object) control_frame->priv->control, &ev);

		width = height = 0;
	}

	*desired_width = width;
	*desired_height = height;

	CORBA_exception_free (&ev);
}

/*
 *  These methods are used by the bonobo-socket to
 * sync the X pipe with the CORBA connection.
 */
void
bonobo_control_frame_sync_realize (BonoboControlFrame *frame)
{
	Bonobo_Control control;
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_CONTROL_FRAME (frame));
	
	if (!frame->priv || frame->priv->control == CORBA_OBJECT_NIL)
		return;

	control = frame->priv->control;

	bonobo_control_frame_set_remote_window (
		frame->priv->socket, frame);

	/*
	 * We sync here so that we make sure that if the XID for
	 * our window is passed to another application, SubstructureRedirectMask
	 * will be set by the time the other app creates its window.
	 */
	gdk_flush ();

	if (control == CORBA_OBJECT_NIL)
		return;

	CORBA_exception_init (&ev);

	Bonobo_Control_realize (control, &ev);
	if (BONOBO_EX (&ev))
		g_warning ("Exception on unrealize '%s'",
			   bonobo_exception_get_text (&ev));

	CORBA_exception_free (&ev);

	gdk_flush ();
}

void
bonobo_control_frame_sync_unrealize (BonoboControlFrame *frame)
{
	Bonobo_Control control;
	CORBA_Environment ev;

	if (!frame->priv || frame->priv->control == CORBA_OBJECT_NIL)
		return;

	control = frame->priv->control;

	gdk_flush ();

	if (control == CORBA_OBJECT_NIL)
		return;

	CORBA_exception_init (&ev);

	Bonobo_Control_unrealize (control, &ev);
	if (BONOBO_EX (&ev))
		g_warning ("Exception on unrealize '%s'",
			   bonobo_exception_get_text (&ev));

	CORBA_exception_free (&ev);

	gdk_flush ();
}
