/* -*- mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Bonobo control object
 *
 * Authors:
 *   Michael Meeks     (michael@ximian.com)
 *   Nat Friedman      (nat@ximian.com)
 *   Miguel de Icaza   (miguel@ximian.com)
 *   Maciej Stachowiak (mjs@eazel.com)
 *
 * Copyright 1999, 2001 Ximian, Inc.
 *                 2000 Eazel, Inc.
 */
#include <config.h>
#include <stdlib.h>
#include <string.h>

#include <gdk/gdkx.h>
#include <gtk/gtksignal.h>

#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-plug.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-ui-sync-menu.h>
#include <bonobo/bonobo-control-internal.h>
#include <bonobo/bonobo-property-bag-client.h>

enum {
	SET_FRAME,
	ACTIVATE,
	LAST_SIGNAL
};

static guint control_signals [LAST_SIGNAL];

static GObjectClass *bonobo_control_parent_class = NULL;

struct _BonoboControlPrivate {
	Bonobo_ControlFrame  frame;
	BonoboControlFrame  *inproc_frame;
	BonoboUIComponent   *ui_component;
	Bonobo_PropertyBag   propbag;
	BonoboUIContainer   *popup_ui_container;
	BonoboUIComponent   *popup_ui_component;
	BonoboUIEngine      *popup_ui_engine;
	BonoboUISync        *popup_ui_sync;

	GtkWidget           *plug;
	GtkWidget           *widget;

	guint                active : 1;
	guint                automerge : 1;
};

static void
control_frame_connection_died_cb (gpointer connection,
				  gpointer user_data)
{
	BonoboControl *control = BONOBO_CONTROL (user_data);

	g_return_if_fail (control != NULL);

	dprintf ("The remote control frame died unexpectedly");
	bonobo_object_unref (BONOBO_OBJECT (control));
}

void
bonobo_control_add_listener (CORBA_Object        object,
			     GCallback           fn,
			     gpointer            user_data,
			     CORBA_Environment  *ev)
{
	ORBitConnectionStatus status;

	if (object == CORBA_OBJECT_NIL)
		return;
	
	status = ORBit_small_listen_for_broken (
		object, fn, user_data);
	
	switch (status) {
	case ORBIT_CONNECTION_CONNECTED:
		break;
	default:
		dprintf ("premature CORBA_Object death");
		bonobo_exception_general_error_set (
			ev, NULL, "Control died prematurely");
		break;
	}
}

/**
 * bonobo_control_window_id_from_x11:
 * @x11_id: the x11 window id.
 * 
 * This mangles the X11 name into the ':' delimited
 * string format "X-id: ..."
 * 
 * Return value: the string; free after use.
 **/
Bonobo_Gdk_WindowId
bonobo_control_window_id_from_x11 (guint32 x11_id)
{
	guchar str[32];

	snprintf (str, 31, "%d", x11_id);
	str[31] = '\0';

/*	printf ("Mangled %d to '%s'\n", x11_id, str);*/

	return CORBA_string_dup (str);
}

/**
 * bonobo_control_x11_from_window_id:
 * @id: CORBA_char *
 * 
 * De-mangle a window id string,
 * fields are separated by ':' character,
 * currently only the first field is used.
 * 
 * Return value: the X11 window id.
 **/
guint32
bonobo_control_x11_from_window_id (const CORBA_char *id)
{
	guint32 x11_id;
	char **elements;
	
/*	printf ("ID string '%s'\n", id);*/

	elements = g_strsplit (id, ":", -1);
	if (elements && elements [0])
		x11_id = strtol (elements [0], NULL, 10);
	else {
		g_warning ("Serious X id mangling error");
		x11_id = 0;
	}
	g_strfreev (elements);

/*	printf ("x11 : %d\n", x11_id);*/

	return x11_id;
}

static void
bonobo_control_auto_merge (BonoboControl *control)
{
	Bonobo_UIContainer remote_container;

	if (control->priv->ui_component == NULL)
		return;

	/* 
	 * this makes a CORBA call, so re-entrancy can occur here
	 */
	remote_container = bonobo_control_get_remote_ui_container (control, NULL);
	if (remote_container == CORBA_OBJECT_NIL)
		return;

	/*
	 * we could have been re-entereted in the previous call, so
	 * make sure we are still active
	 */
	if (control->priv->active)
		bonobo_ui_component_set_container (
			control->priv->ui_component, remote_container, NULL);

	bonobo_object_release_unref (remote_container, NULL);
}


static void
bonobo_control_auto_unmerge (BonoboControl *control)
{
	if (control->priv->ui_component == NULL)
		return;
	
	bonobo_ui_component_unset_container (control->priv->ui_component, NULL);
}

static void
impl_Bonobo_Control_activate (PortableServer_Servant servant,
			      CORBA_boolean activated,
			      CORBA_Environment *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));
	gboolean old_activated;

	if (activated == control->priv->active)
		return;
	
	/* 
	 * store the old activated value as we can be re-entered
	 * during (un)merge
	 */
	old_activated = control->priv->active;
	control->priv->active = activated;

	if (control->priv->automerge) {
		if (activated)
			bonobo_control_auto_merge (control);
		else
			bonobo_control_auto_unmerge (control);
	}

	/* 
	 * if our active state is not what we are changing it to, then
	 * don't emit the signal
	 */
	if (control->priv->active != activated)
		return;

	g_signal_emit (control, control_signals [ACTIVATE], 0, (gboolean) activated);
}

static void
bonobo_control_unset_control_frame (BonoboControl     *control,
				    CORBA_Environment *opt_ev)
{
	CORBA_Environment *ev, tmp_ev;

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	if (control->priv->frame != CORBA_OBJECT_NIL) {
		Bonobo_ControlFrame frame = control->priv->frame;

		control->priv->frame = CORBA_OBJECT_NIL;

		ORBit_small_unlisten_for_broken (
			frame, G_CALLBACK (control_frame_connection_died_cb));

		if (control->priv->active)
			Bonobo_ControlFrame_notifyActivated (
				frame, FALSE, ev);

		CORBA_Object_release (frame, ev);
	}

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);
}

static void
create_plug (BonoboControl *control)
{
	GtkWidget *plug;
	
	plug = bonobo_plug_new (0);

	g_object_ref (G_OBJECT (plug));
	gtk_object_sink (GTK_OBJECT (plug));
	
	bonobo_control_set_plug (control, BONOBO_PLUG (plug));

	if (control->priv->widget)
		gtk_container_add (GTK_CONTAINER (plug),
				   control->priv->widget);

	g_object_unref (G_OBJECT (plug));
}

#ifdef HAVE_GTK_MULTIHEAD
static int
parse_cookie (const CORBA_char *cookie)
{
	GString    *ident = NULL;
	GString    *value = NULL;
	const char *p;
	char       *screen = NULL;
	int         retval = -1;

	for (p = cookie; *p && !screen; p++) {
		switch (*p) {
		case ',':
			if (!ident || !value)
				goto parse_failed;

			if (strcmp (ident->str, "screen")) {
				g_string_free (ident, TRUE); ident = NULL;
				g_string_free (value, TRUE); value = NULL;
				break;
			}

			screen = value->str;
			break;
		case '=':
			if (!ident || value)
				goto parse_failed;
			value = g_string_new ("");
			break;
		default:
			if (!ident)
				ident = g_string_new ("");
			
			if (value)
				g_string_append_c (value, *p);
			else
				g_string_append_c (ident, *p);
			break;
		}
	}

	if (ident && value && !strcmp (ident->str, "screen"))
		screen = value->str;

	if (screen)
		retval = atoi (screen);

parse_failed:
	if (ident) g_string_free (ident, TRUE);
	if (value) g_string_free (value, TRUE);

	return retval;
}
#endif /* HAVE_GTK_MULTIHEAD */

static CORBA_char *
impl_Bonobo_Control_getWindowId (PortableServer_Servant servant,
				 const CORBA_char      *cookie,
				 CORBA_Environment     *ev)
{
	guint32        x11_id;
	BonoboControl *control = BONOBO_CONTROL (
		bonobo_object_from_servant (servant));
#ifdef HAVE_GTK_MULTIHEAD
	GdkScreen *gdkscreen;
	int        screen_num;
#endif

	if (!control->priv->plug)
		create_plug (control);

	g_assert (control->priv->plug != NULL);

#ifdef HAVE_GTK_MULTIHEAD
	screen_num = parse_cookie (cookie);
	if (screen_num != -1)
		gdkscreen = gdk_display_get_screen (
				gdk_display_get_default (), screen_num);
	else
		gdkscreen = gdk_screen_get_default ();

	gtk_window_set_screen (GTK_WINDOW (control->priv->plug), gdkscreen);
#endif

	gtk_widget_show (control->priv->plug);

	x11_id = gtk_plug_get_id (GTK_PLUG (control->priv->plug));
		
	dprintf ("plug id %d\n", x11_id);

	return bonobo_control_window_id_from_x11 (x11_id);
}

static Bonobo_UIContainer
impl_Bonobo_Control_getPopupContainer (PortableServer_Servant servant,
				       CORBA_Environment     *ev)
{
	BonoboUIContainer *container;
	
	container = bonobo_control_get_popup_ui_container (
		BONOBO_CONTROL (bonobo_object_from_servant (servant)));

	return bonobo_object_dup_ref (BONOBO_OBJREF (container), ev);
}
	
static void
impl_Bonobo_Control_setFrame (PortableServer_Servant servant,
			      Bonobo_ControlFrame    frame,
			      CORBA_Environment     *ev)
{
	BonoboControl *control = BONOBO_CONTROL (
		bonobo_object_from_servant (servant));

	g_object_ref (control);

	if (control->priv->frame != frame) {
		bonobo_control_unset_control_frame (control, ev);
	
		if (frame == CORBA_OBJECT_NIL)
			control->priv->frame = CORBA_OBJECT_NIL;
		else {
			control->priv->frame = CORBA_Object_duplicate (
				frame, NULL);
		}
	
		control->priv->inproc_frame = (BonoboControlFrame *)
			bonobo_object (ORBit_small_get_servant (frame));

		if (!control->priv->inproc_frame)
			bonobo_control_add_listener (
				frame,
				G_CALLBACK (control_frame_connection_died_cb),
				control, ev);
	
		g_signal_emit (control, control_signals [SET_FRAME], 0);
	}

	g_object_unref (control);
}

static void
impl_Bonobo_Control_setSize (PortableServer_Servant  servant,
			     const CORBA_short       width,
			     const CORBA_short       height,
			     CORBA_Environment      *ev)
{
	GtkAllocation  size;
	BonoboControl *control = BONOBO_CONTROL (
		bonobo_object_from_servant (servant));
	
	size.x = size.y = 0;
	size.width = width;
	size.height = height;

	/*
	 * In the Gnome implementation of Bonobo, all size assignment
	 * is handled by GtkPlug/GtkSocket for us.
	 */

	g_warning ("setSize untested");

	gtk_widget_size_allocate (GTK_WIDGET (control->priv->plug), &size);
}

static Bonobo_Gtk_Requisition
impl_Bonobo_Control_getDesiredSize (PortableServer_Servant servant,
				    CORBA_Environment     *ev)
{
	BonoboControl         *control;
	GtkRequisition         requisition;
	Bonobo_Gtk_Requisition req;

	control = BONOBO_CONTROL (bonobo_object (servant));

	gtk_widget_size_request (control->priv->widget, &requisition);

	req.width  = requisition.width;
	req.height = requisition.height;

	return req;
}

static GtkStateType
bonobo_control_gtk_state_from_corba (const Bonobo_Gtk_State state)
{
	switch (state) {
	case Bonobo_Gtk_StateNormal:
		return GTK_STATE_NORMAL;

	case Bonobo_Gtk_StateActive:
		return GTK_STATE_ACTIVE;

	case Bonobo_Gtk_StatePrelight:
		return GTK_STATE_PRELIGHT;

	case Bonobo_Gtk_StateSelected:
		return GTK_STATE_SELECTED;

	case Bonobo_Gtk_StateInsensitive:
		return GTK_STATE_INSENSITIVE;

	default:
		g_warning ("bonobo_control_gtk_state_from_corba: Unknown state: %d", (gint) state);
		return GTK_STATE_NORMAL;
	}
}

static void
impl_Bonobo_Control_setState (PortableServer_Servant  servant,
			       const Bonobo_Gtk_State state,
			       CORBA_Environment     *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));
	GtkStateType   gtk_state = bonobo_control_gtk_state_from_corba (state);

	g_return_if_fail (control->priv->widget != NULL);

	if (gtk_state == GTK_STATE_INSENSITIVE)
		gtk_widget_set_sensitive (control->priv->widget, FALSE);
	else {
		if (! GTK_WIDGET_SENSITIVE (control->priv->widget))
			gtk_widget_set_sensitive (control->priv->widget, TRUE);

		gtk_widget_set_state (control->priv->widget,
				      gtk_state);
	}
}

static Bonobo_PropertyBag
impl_Bonobo_Control_getProperties (PortableServer_Servant  servant,
				   CORBA_Environment      *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));
	if (control->priv->propbag == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;

	return bonobo_object_dup_ref (control->priv->propbag, ev);
}

static CORBA_boolean
impl_Bonobo_Control_focus (PortableServer_Servant servant,
			   Bonobo_Gtk_Direction   corba_direction,
			   CORBA_Environment     *ev)
{
	BonoboControl        *control;
	GtkDirectionType      direction;
	BonoboControlPrivate *priv;

	control = BONOBO_CONTROL (bonobo_object (servant));
	priv = control->priv;

	/* FIXME: this will not work for local controls. */

	if (!priv->plug)
		return FALSE;

	switch (corba_direction) {
	case Bonobo_Gtk_DirectionTabForward:
		direction = GTK_DIR_TAB_FORWARD;
		break;

	case Bonobo_Gtk_DirectionTabBackward:
		direction = GTK_DIR_TAB_BACKWARD;
		break;

	case Bonobo_Gtk_DirectionUp:
		direction = GTK_DIR_UP;
		break;

	case Bonobo_Gtk_DirectionDown:
		direction = GTK_DIR_DOWN;
		break;

	case Bonobo_Gtk_DirectionLeft:
		direction = GTK_DIR_LEFT;
		break;

	case Bonobo_Gtk_DirectionRight:
		direction = GTK_DIR_RIGHT;
		break;

	default:
		/* Hmmm, we should throw an exception. */
		return FALSE;
	}

	return gtk_widget_child_focus (GTK_WIDGET (priv->plug), direction);
}

BonoboControl *
bonobo_control_construct (BonoboControl  *control,
			  GtkWidget      *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	/*
	 * This sets up the X handler for Bonobo objects.  We basically will
	 * ignore X errors if our container dies (because X will kill the
	 * windows of the container and our container without telling us).
	 */
	bonobo_setup_x_error_handler ();

	control->priv->widget = g_object_ref (widget);
	gtk_object_sink (GTK_OBJECT (widget));

	gtk_container_add (GTK_CONTAINER (control->priv->plug),
			   control->priv->widget);

	control->priv->ui_component = NULL;
	control->priv->propbag = CORBA_OBJECT_NIL;

	return control;
}

/**
 * bonobo_control_new:
 * @widget: a GTK widget that contains the control and will be passed to the
 * container process.
 *
 * This function creates a new BonoboControl object for @widget.
 *
 * Returns: a BonoboControl object that implements the Bonobo::Control CORBA
 * service that will transfer the @widget to the container process.
 */
BonoboControl *
bonobo_control_new (GtkWidget *widget)
{
	BonoboControl *control;
	
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	control = g_object_new (bonobo_control_get_type (), NULL);
	
	return bonobo_control_construct (control, widget);
}

/**
 * bonobo_control_get_widget:
 * @control: a BonoboControl
 *
 * Returns the GtkWidget associated with a BonoboControl.
 *
 * Return value: the BonoboControl's widget
 **/
GtkWidget *
bonobo_control_get_widget (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	return control->priv->widget;
}

/**
 * bonobo_control_set_automerge:
 * @control: A #BonoboControl.
 * @automerge: Whether or not menus and toolbars should be
 * automatically merged when the control is activated.
 *
 * Sets whether or not the control handles menu/toolbar merging
 * automatically.  If automerge is on, the control will automatically
 * register its BonoboUIComponent with the remote BonoboUIContainer
 * when it is activated.
 */
void
bonobo_control_set_automerge (BonoboControl *control,
			      gboolean       automerge)
{
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	control->priv->automerge = automerge;

	if (automerge && !control->priv->ui_component)
		control->priv->ui_component = bonobo_ui_component_new_default ();
}

/**
 * bonobo_control_get_automerge:
 * @control: A #BonoboControl.
 *
 * Returns: Whether or not the control is set to automerge its
 * menus/toolbars.  See bonobo_control_set_automerge().
 */
gboolean
bonobo_control_get_automerge (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), FALSE);

	return control->priv->automerge;
}

static void
bonobo_control_destroy (BonoboObject *object)
{
	BonoboControl *control = (BonoboControl *) object;

	dprintf ("bonobo_control_destroy %p\n", object);

	if (control->priv->plug)
		bonobo_control_set_plug (control, NULL);

	bonobo_control_unset_control_frame (control, NULL);
	bonobo_control_set_properties      (control, CORBA_OBJECT_NIL, NULL);
	bonobo_control_set_ui_component    (control, NULL);

	if (control->priv->widget) {
		gtk_widget_destroy (GTK_WIDGET (control->priv->widget));
		g_object_unref (control->priv->widget);
	}
	control->priv->widget = NULL;

	control->priv->popup_ui_container = bonobo_object_unref (
		(BonoboObject *) control->priv->popup_ui_container);

	if (control->priv->popup_ui_engine)
		g_object_unref (control->priv->popup_ui_engine);
	control->priv->popup_ui_engine = NULL;

	control->priv->popup_ui_component = bonobo_object_unref (
		(BonoboObject *) control->priv->popup_ui_component);

	control->priv->popup_ui_sync = NULL;
	control->priv->inproc_frame  = NULL;

	BONOBO_OBJECT_CLASS (bonobo_control_parent_class)->destroy (object);
}

static void
bonobo_control_finalize (GObject *object)
{
	BonoboControl *control = BONOBO_CONTROL (object);

	dprintf ("bonobo_control_finalize %p\n", object);

	g_free (control->priv);

	bonobo_control_parent_class->finalize (object);
}

/**
 * bonobo_control_get_control_frame:
 * @control: A BonoboControl object whose Bonobo_ControlFrame CORBA interface is
 * being retrieved.
 * @opt_ev: an optional exception environment
 *
 * Returns: The Bonobo_ControlFrame CORBA object associated with @control, this is
 * a CORBA_Object_duplicated object.  You need to CORBA_Object_release it when you are
 * done with it.
 */
Bonobo_ControlFrame
bonobo_control_get_control_frame (BonoboControl     *control,
				  CORBA_Environment *opt_ev)
{
	Bonobo_ControlFrame frame;
	CORBA_Environment *ev, tmp_ev;
	
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), CORBA_OBJECT_NIL);

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;
	
	frame = CORBA_Object_duplicate (control->priv->frame, ev);
	
	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);

	return frame;
}

/**
 * bonobo_control_get_ui_component:
 * @control: The control
 * 
 * Return value: the associated UI component
 **/
BonoboUIComponent *
bonobo_control_get_ui_component (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	if (!control->priv->ui_component)
		control->priv->ui_component = bonobo_ui_component_new_default ();

	return control->priv->ui_component;
}

void
bonobo_control_set_ui_component (BonoboControl     *control,
				 BonoboUIComponent *component)
{
	g_return_if_fail (BONOBO_IS_CONTROL (control));
	g_return_if_fail (component == NULL ||
			  BONOBO_IS_UI_COMPONENT (component));

	if (component == control->priv->ui_component)
		return;

	if (control->priv->ui_component) {
		bonobo_ui_component_unset_container (control->priv->ui_component, NULL);
		bonobo_object_unref (BONOBO_OBJECT (control->priv->ui_component));
	}

	control->priv->ui_component = bonobo_object_ref ((BonoboObject *) component);
}

/**
 * bonobo_control_set_properties:
 * @control: A #BonoboControl object.
 * @pb: A #Bonobo_PropertyBag.
 * @opt_ev: An optional exception environment
 *
 * Binds @pb to @control.  When a remote object queries @control
 * for its property bag, @pb will be used in the responses.
 */
void
bonobo_control_set_properties (BonoboControl      *control,
			       Bonobo_PropertyBag  pb,
			       CORBA_Environment  *opt_ev)
{
	Bonobo_PropertyBag old_bag;

	g_return_if_fail (BONOBO_IS_CONTROL (control));

	if (pb == control->priv->propbag)
		return;

	old_bag = control->priv->propbag;

	control->priv->propbag = bonobo_object_dup_ref (pb, opt_ev);
	bonobo_object_release_unref (old_bag, opt_ev);
}

/**
 * bonobo_control_get_properties:
 * @control: A #BonoboControl whose PropertyBag has already been set.
 *
 * Returns: The #Bonobo_PropertyBag bound to @control.
 */
Bonobo_PropertyBag 
bonobo_control_get_properties (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	return control->priv->propbag;
}

/**
 * bonobo_control_get_ambient_properties:
 * @control: A #BonoboControl which is bound to a remote
 * #BonoboControlFrame.
 * @opt_ev: an optional exception environment
 *
 * Returns: A #Bonobo_PropertyBag bound to the bag of ambient
 * properties associated with this #Control's #ControlFrame.
 */
Bonobo_PropertyBag
bonobo_control_get_ambient_properties (BonoboControl     *control,
				       CORBA_Environment *opt_ev)
{
	Bonobo_ControlFrame frame;
	Bonobo_PropertyBag pbag;
	CORBA_Environment *ev = 0, tmp_ev;

	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	frame = control->priv->frame;

	if (frame == CORBA_OBJECT_NIL)
		return NULL;

	if (opt_ev)
		ev = opt_ev;
	else {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	}

	pbag = Bonobo_ControlFrame_getAmbientProperties (
		frame, ev);

	if (BONOBO_EX (ev)) {
		if (!opt_ev)
			CORBA_exception_free (&tmp_ev);
		pbag = CORBA_OBJECT_NIL;
	}

	return pbag;
}

/**
 * bonobo_control_get_remote_ui_container:
 * @control: A BonoboControl object which is associated with a remote
 * ControlFrame.
 * @opt_ev: an optional exception environment
 *
 * Returns: The Bonobo_UIContainer CORBA server for the remote BonoboControlFrame.
 */
Bonobo_UIContainer
bonobo_control_get_remote_ui_container (BonoboControl     *control,
					CORBA_Environment *opt_ev)
{
	CORBA_Environment  tmp_ev, *ev;
	Bonobo_UIContainer ui_container;

	g_return_val_if_fail (BONOBO_IS_CONTROL (control), CORBA_OBJECT_NIL);

	g_return_val_if_fail (control->priv->frame != CORBA_OBJECT_NIL,
			      CORBA_OBJECT_NIL);

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	ui_container = Bonobo_ControlFrame_getUIContainer (control->priv->frame, ev);

	bonobo_object_check_env (BONOBO_OBJECT (control), control->priv->frame, ev);

	if (BONOBO_EX (ev))
		ui_container = CORBA_OBJECT_NIL;

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);

	return ui_container;
}

/**
 * bonobo_control_activate_notify:
 * @control: A #BonoboControl object which is bound
 * to a remote ControlFrame.
 * @activated: Whether or not @control has been activated.
 * @opt_ev: An optional exception environment
 *
 * Notifies the remote ControlFrame which is associated with
 * @control that @control has been activated/deactivated.
 */
void
bonobo_control_activate_notify (BonoboControl     *control,
				gboolean           activated,
				CORBA_Environment *opt_ev)
{
	CORBA_Environment *ev, tmp_ev;

	g_return_if_fail (BONOBO_IS_CONTROL (control));
	g_return_if_fail (control->priv->frame != CORBA_OBJECT_NIL);

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	Bonobo_ControlFrame_notifyActivated (control->priv->frame, activated, ev);

	bonobo_object_check_env (BONOBO_OBJECT (control), control->priv->frame, ev);

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);
}

static void
bonobo_control_class_init (BonoboControlClass *klass)
{
	GObjectClass *object_class = (GObjectClass *)klass;
	BonoboObjectClass *bonobo_object_class = (BonoboObjectClass *)klass;
	POA_Bonobo_Control__epv *epv;

	bonobo_control_parent_class = g_type_class_peek_parent (klass);

	control_signals [SET_FRAME] =
                g_signal_new ("set_frame",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboControlClass, set_frame),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	control_signals [ACTIVATE] =
                g_signal_new ("activate",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboControlClass, activate),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE, 1,
			      G_TYPE_BOOLEAN);

	object_class->finalize = bonobo_control_finalize;
	bonobo_object_class->destroy = bonobo_control_destroy;

	epv = &klass->epv;

	epv->getProperties     = impl_Bonobo_Control_getProperties;
	epv->getDesiredSize    = impl_Bonobo_Control_getDesiredSize;
	epv->getAccessible     = NULL;
	epv->getWindowId       = impl_Bonobo_Control_getWindowId;
	epv->getPopupContainer = impl_Bonobo_Control_getPopupContainer;
	epv->setFrame          = impl_Bonobo_Control_setFrame;
	epv->setSize           = impl_Bonobo_Control_setSize;
	epv->setState          = impl_Bonobo_Control_setState;
	epv->activate          = impl_Bonobo_Control_activate;
	epv->focus             = impl_Bonobo_Control_focus;
}

static void
bonobo_control_init (BonoboControl *control)
{
	control->priv = g_new0 (BonoboControlPrivate, 1);

	control->priv->frame = CORBA_OBJECT_NIL;

	create_plug (control);
}

BONOBO_TYPE_FUNC_FULL (BonoboControl, 
		       Bonobo_Control,
		       BONOBO_OBJECT_TYPE,
		       bonobo_control);

/*
 * Varrarg Property get/set simplification wrappers.
 */

/**
 * bonobo_control_set_property:
 * @control: the control with associated property bag
 * @opt_ev: optional corba exception environment
 * @first_prop: the first property's name
 * 
 * This method takes a NULL terminated list of name, type, value
 * triplicates, and sets the corresponding values on the control's
 * associated property bag.
 **/
void
bonobo_control_set_property (BonoboControl     *control,
			     CORBA_Environment *opt_ev,
			     const char        *first_prop,
			     ...)
{
	Bonobo_PropertyBag  bag;
	char               *err;
	CORBA_Environment  *ev, tmp_ev;
	va_list             args;

	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	va_start (args, first_prop);

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;
		
	bag = control->priv->propbag;

	if ((err = bonobo_property_bag_client_setv (bag, ev, first_prop, args)))
		g_warning ("Error '%s'", err);

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);

	va_end (args);
}

/**
 * bonobo_control_get_property:
 * @control: the control with associated property bag
 * @opt_ev: optional corba exception environment
 * @first_prop: the first property's name
 * 
 * This method takes a NULL terminated list of name, type, value
 * triplicates, and fetches the corresponding values on the control's
 * associated property bag.
 **/
void
bonobo_control_get_property (BonoboControl     *control,
			     CORBA_Environment *opt_ev,
			     const char        *first_prop,
			     ...)
{
	Bonobo_PropertyBag  bag;
	char               *err;
	CORBA_Environment  *ev, tmp_ev;
	va_list             args;

	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	va_start (args, first_prop);

	if (!opt_ev) {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	} else
		ev = opt_ev;

	bag = control->priv->propbag;

	if ((err = bonobo_property_bag_client_getv (bag, ev, first_prop, args)))
		g_warning ("Error '%s'", err);

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);

	va_end (args);
}

/*
 * Transient / Modality handling logic
 */

#undef TRANSIENT_DEBUG

static void
window_transient_realize_gdk_cb (GtkWidget *widget)
{
	GdkWindow *win;

	win = g_object_get_data (G_OBJECT (widget), "transient");
	g_return_if_fail (win != NULL);

#ifdef TRANSIENT_DEBUG
	g_warning ("Set transient");
#endif
	gdk_window_set_transient_for (
		GTK_WIDGET (widget)->window, win);
}

static void
window_transient_unrealize_gdk_cb (GtkWidget *widget)
{
	GdkWindow *win;

	win = g_object_get_data (G_OBJECT (widget), "transient");
	g_return_if_fail (win != NULL);

	gdk_property_delete (
		win, gdk_atom_intern ("WM_TRANSIENT_FOR", FALSE));
}


static void
window_transient_destroy_gdk_cb (GtkWidget *widget)
{
	GdkWindow *win;
	
	if ((win = g_object_get_data (G_OBJECT (widget), "transient")))
		g_object_unref (win);
}

static void       
window_set_transient_for_gdk (GtkWindow *window, 
			      GdkWindow *parent)
{
	g_return_if_fail (window != NULL);
	g_return_if_fail (g_object_get_data (
		G_OBJECT (window), "transient") == NULL);

	g_object_ref (parent);

	g_object_set_data (G_OBJECT (window), "transient", parent);

	if (GTK_WIDGET_REALIZED (window)) {
#ifdef TRANSIENT_DEBUG
		g_warning ("Set transient");
#endif
		gdk_window_set_transient_for (
			GTK_WIDGET (window)->window, parent);
	}

	/*
	 * FIXME: we need to be able to get the screen from
	 * the remote toplevel window for multi-head to work
	 * could use an ambient property on the ControlFrame
	 */

	g_signal_connect (
		window, "realize",
		G_CALLBACK (window_transient_realize_gdk_cb), NULL);

	g_signal_connect (
		window, "unrealize",
		G_CALLBACK (window_transient_unrealize_gdk_cb), NULL);
	
	g_signal_connect (
		window, "destroy",
		G_CALLBACK (window_transient_destroy_gdk_cb), NULL);
}

/**
 * bonobo_control_set_transient_for:
 * @control: a control with associated control frame
 * @window: a window upon which to set the transient window.
 * 
 *   Attempts to make the @window transient for the toplevel
 * of any associated controlframe the BonoboControl may have.
 **/
void
bonobo_control_set_transient_for (BonoboControl     *control,
				  GtkWindow         *window,
				  CORBA_Environment *opt_ev)
{
	CORBA_char         *id;
	GdkWindow          *win;
	guint32             x11_id;
	CORBA_Environment  *ev = 0, tmp_ev;
	Bonobo_ControlFrame frame;

	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	/* FIXME: special case the local case !
	 * we can only do this if set_transient is virtualized
	 * and thus we can catch it in BonoboSocket and chain up
	 * again if we are embedded inside an embedded thing. */

	frame = control->priv->frame;

	if (frame == CORBA_OBJECT_NIL)
		return;

	if (opt_ev)
		ev = opt_ev;
	else {
		CORBA_exception_init (&tmp_ev);
		ev = &tmp_ev;
	}

	id = Bonobo_ControlFrame_getToplevelId (frame, ev);
	g_return_if_fail (!BONOBO_EX (ev) && id != NULL);

	x11_id = bonobo_control_x11_from_window_id (id);

#ifdef TRANSIENT_DEBUG
	g_warning ("Got id '%s' -> %d", id, x11_id);
#endif
	CORBA_free (id);

	/* FIXME: Special case the local case ? */
	win = gdk_window_foreign_new (x11_id);
	g_return_if_fail (win != NULL);

	window_set_transient_for_gdk (window, win);

	if (!opt_ev)
		CORBA_exception_free (&tmp_ev);
}

/**
 * bonobo_control_unset_transient_for:
 * @control: a control with associated control frame
 * @window: a window upon which to unset the transient window.
 * 
 **/
void
bonobo_control_unset_transient_for (BonoboControl     *control,
				    GtkWindow         *window,
				    CORBA_Environment *opt_ev)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

	g_signal_handlers_disconnect_by_func (
		window, G_CALLBACK (window_transient_realize_gdk_cb), NULL);

	g_signal_handlers_disconnect_by_func (
		window, G_CALLBACK (window_transient_unrealize_gdk_cb), NULL);
	
	g_signal_handlers_disconnect_by_func (
		window, G_CALLBACK (window_transient_destroy_gdk_cb), NULL);

	window_transient_unrealize_gdk_cb (GTK_WIDGET (window));
}

void
bonobo_control_set_plug (BonoboControl *control,
			 BonoboPlug    *plug)
{
	BonoboPlug *old_plug;

	g_return_if_fail (BONOBO_IS_CONTROL (control));

	if ((BonoboPlug *) control->priv->plug == plug)
		return;

	dprintf ("bonobo_control_set_plug (%p, %p) [%p]\n",
		 control, plug, control->priv->plug);

	old_plug = (BonoboPlug *) control->priv->plug;

	if (plug)
		control->priv->plug = g_object_ref (plug);
	else
		control->priv->plug = NULL;

	if (old_plug) {
		bonobo_plug_set_control (old_plug, NULL);
		gtk_widget_destroy (GTK_WIDGET (old_plug));
		g_object_unref (old_plug);
	}

	if (plug)
		bonobo_plug_set_control (plug, control);
}

BonoboPlug *
bonobo_control_get_plug (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	return (BonoboPlug *) control->priv->plug;
}

void
bonobo_control_set_popup_ui_container (BonoboControl     *control,
				       BonoboUIContainer *ui_container)
{
	g_return_if_fail (BONOBO_IS_CONTROL (control));
	g_return_if_fail (BONOBO_IS_UI_CONTAINER (ui_container));

	g_assert (control->priv->popup_ui_container == NULL);

	control->priv->popup_ui_container = bonobo_object_ref (
		BONOBO_OBJECT (ui_container));
}

BonoboUIContainer *
bonobo_control_get_popup_ui_container (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	if (!control->priv->popup_ui_container) {
		BonoboUIEngine *ui_engine;
		BonoboUISync   *ui_sync;

		ui_engine = bonobo_ui_engine_new (G_OBJECT (control));

		ui_sync = bonobo_ui_sync_menu_new (ui_engine, NULL, NULL, NULL);

		bonobo_ui_engine_add_sync (ui_engine, ui_sync);

		/* re-entrancy guard */
		if (control->priv->popup_ui_container) {
			g_object_unref (ui_engine);

			return control->priv->popup_ui_container;
		}

		control->priv->popup_ui_engine = ui_engine;
		control->priv->popup_ui_sync   = ui_sync;

		control->priv->popup_ui_container = bonobo_ui_container_new ();
		bonobo_ui_container_set_engine (
			control->priv->popup_ui_container,
			control->priv->popup_ui_engine);
	}

	return control->priv->popup_ui_container;
}

BonoboUIComponent *
bonobo_control_get_popup_ui_component (BonoboControl *control)
{
	BonoboUIContainer *ui_container;

	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	if (!control->priv->popup_ui_component) {
		ui_container = bonobo_control_get_popup_ui_container (control);

		control->priv->popup_ui_component = 
			bonobo_ui_component_new_default ();

		bonobo_ui_component_set_container (
			control->priv->popup_ui_component,
			BONOBO_OBJREF (ui_container), NULL);
	}

	return control->priv->popup_ui_component;
}

gboolean
bonobo_control_do_popup_full (BonoboControl       *control,
			      GtkWidget           *parent_menu_shell,
			      GtkWidget           *parent_menu_item,
			      GtkMenuPositionFunc  func,
			      gpointer             data,
			      guint                button,
			      guint32              activate_time)
{
	char      *path;
	GtkWidget *menu;

	g_return_val_if_fail (BONOBO_IS_CONTROL (control), FALSE);

	if (!control->priv->popup_ui_container)
		return FALSE;

	path = g_strdup_printf ("/popups/button%d", button);

	menu = gtk_menu_new ();

	bonobo_ui_sync_menu_add_popup (
		BONOBO_UI_SYNC_MENU (control->priv->popup_ui_sync),
		GTK_MENU (menu), path);

	g_free (path);

#ifdef HAVE_GTK_MULTIHEAD
	gtk_menu_set_screen (
		GTK_MENU (menu),
		gtk_window_get_screen (GTK_WINDOW (control->priv->plug)));
#endif /* HAVE_GTK_MULTIHEAD */

	gtk_widget_show (menu);

	gtk_menu_popup (GTK_MENU (menu),
			parent_menu_shell, parent_menu_item,
			func, data,
			button, activate_time);

	return TRUE;
}

gboolean
bonobo_control_do_popup (BonoboControl       *control,
			 guint                button,
			 guint32              activate_time)
{
	return bonobo_control_do_popup_full (
		control, NULL, NULL, NULL, NULL,
		button, activate_time);
}
