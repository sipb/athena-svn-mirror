/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-control-item.c: A special toolbar item for controls.
 *
 * Author:
 *	Jon K Hellan (hellan@acm.org)
 *
 * Copyright 2000 Jon K Hellan.
 * Copyright (C) 2001 Eazel, Inc.
 */

#include <config.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-ui-toolbar-control-item.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag-client.h>
#include <libgnome/gnome-macros.h>

GNOME_CLASS_BOILERPLATE (BonoboUIToolbarControlItem,
			 bonobo_ui_toolbar_control_item,
			 GObject,
			 bonobo_ui_toolbar_button_item_get_type ());

struct _BonoboUIToolbarControlItemPrivate {
	GtkWidget    *widget;   /* The widget (of a control, or custom */
        BonoboWidget *control;	/* The wrapped control - if a control, or NULL */
	GtkWidget *button;	/* Button to display instead of control in
				   vertical mode */
	GtkWidget *box;		/* Container for control and button. Which of
				   its children is visible depends on
				   orientation */
	
	GdkWindow *event_window; /* Event window for tool tips */

	BonoboUIToolbarControlDisplay hdisplay;
	BonoboUIToolbarControlDisplay vdisplay;
};

static void
set_control_property_bag_value (BonoboUIToolbarControlItem *item,
				const char                 *name,
				BonoboArg                  *value)
{
	BonoboControlFrame *frame;
	Bonobo_PropertyBag bag;

	if (!item->priv->control)
		return;

	frame = bonobo_widget_get_control_frame (item->priv->control);
	if (!frame)
		return;

	bag = bonobo_control_frame_get_control_property_bag (frame, NULL);
	if (bag == CORBA_OBJECT_NIL)
		return;

	bonobo_pbclient_set_value (bag, name, value, NULL);

	bonobo_object_release_unref (bag, NULL);
}

#define MAKE_SET_CONTROL_PROPERTY_BAG_VALUE(gtype, paramtype, capstype)	\
static void								\
set_control_property_bag_##gtype (BonoboUIToolbarControlItem *item,	\
				  const char *name,			\
				  paramtype value)			\
{									\
	BonoboArg *arg;							\
									\
	arg = bonobo_arg_new (BONOBO_ARG_##capstype);			\
	BONOBO_ARG_SET_##capstype (arg, value);				\
	set_control_property_bag_value (item, name, arg);		\
	bonobo_arg_release (arg);					\
}

MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (gboolean, gboolean,     BOOLEAN)
MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (gint,     gint,         INT)
MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (string,   const char *, STRING)

/* BonoboUIToolbarButtonItem virtual methods.  */
static void
impl_set_icon (BonoboUIToolbarButtonItem *button_item,
	       gpointer                   image)
{
	BonoboUIToolbarControlItemPrivate *priv;
	BonoboUIToolbarControlItem *control_item;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (button_item);
	priv = control_item->priv;

	bonobo_ui_toolbar_button_item_set_image (
		BONOBO_UI_TOOLBAR_BUTTON_ITEM (priv->button), image);
}

static void
impl_set_label (BonoboUIToolbarButtonItem *button_item,
		const char                *label)
{
	BonoboUIToolbarControlItemPrivate *priv;
	BonoboUIToolbarControlItem *control_item;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (button_item);
	priv = control_item->priv;

	bonobo_ui_toolbar_button_item_set_label (
		BONOBO_UI_TOOLBAR_BUTTON_ITEM (priv->button), label);
	set_control_property_bag_string (control_item, "label", label);
}

/* BonoboUIToolbarItem methods.  */

/*
 * We are assuming that there's room in horizontal orientation, but not
 * vertical. This can be made more sophisticated by looking at the control's
 * requested geometry.
 */
static void
impl_set_orientation (BonoboUIToolbarItem *item,
		      GtkOrientation orientation)
{
	BonoboUIToolbarControlItem        *control_item;
	BonoboUIToolbarControlDisplay      display;
	BonoboUIToolbarControlItemPrivate *priv;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (item);
	priv = control_item->priv;

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		display = priv->hdisplay;
	else
		display = priv->vdisplay;
	
	switch (display) {

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL:
		gtk_widget_hide (priv->button);
		gtk_widget_show (priv->widget);
		break;

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_BUTTON:
		gtk_widget_hide (priv->widget);
		gtk_widget_show (priv->button);
		break;

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_NONE:
		gtk_widget_hide (priv->widget);
		gtk_widget_hide (priv->button);
		break;

	default:
		g_assert_not_reached ();
	}

	set_control_property_bag_gint (control_item, "orientation", orientation);

	GNOME_CALL_PARENT (BONOBO_UI_TOOLBAR_ITEM_CLASS, set_orientation,
			   (item, orientation));
}

static void
impl_set_style (BonoboUIToolbarItem     *item,
		BonoboUIToolbarItemStyle style)
{
	BonoboUIToolbarControlItem *control_item;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (item);
	bonobo_ui_toolbar_item_set_style (
		BONOBO_UI_TOOLBAR_ITEM (control_item->priv->button), style);
	set_control_property_bag_gint (control_item, "style", style);
}

static void
impl_set_want_label (BonoboUIToolbarItem     *item,
		     gboolean                 want_label)
{
	BonoboUIToolbarControlItem *control_item;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (item);
	bonobo_ui_toolbar_item_set_want_label (
		BONOBO_UI_TOOLBAR_ITEM (control_item->priv->button),
		want_label);
	set_control_property_bag_gboolean (control_item, "want_label", want_label);
}

static void
impl_set_tooltip (BonoboUIToolbarItem     *item,
                  GtkTooltips             *tooltips,
                  const char              *tooltip)
{
	if (tooltip) 
		gtk_tooltips_set_tip (tooltips, GTK_WIDGET (item), 
				      tooltip, NULL);
}

/* GtkWidget methods. */
static void
impl_realize (GtkWidget *widget)
{
	BonoboUIToolbarControlItem *item;
	GdkWindowAttr attributes;
	int attributes_mask;
	int border_width;
	
	item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget);
	
	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	
	border_width = GTK_CONTAINER (widget)->border_width;
	 
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x + border_width;
	attributes.y = widget->allocation.y + border_width;
	attributes.width = widget->allocation.width - border_width * 2;
	attributes.height = widget->allocation.height - border_width * 2;
	attributes.wclass = GDK_INPUT_ONLY;
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= (GDK_EXPOSURE_MASK |
				  GDK_BUTTON_MOTION_MASK |
				  GDK_BUTTON_PRESS_MASK |
				  GDK_BUTTON_RELEASE_MASK |
				  GDK_ENTER_NOTIFY_MASK |
				  GDK_LEAVE_NOTIFY_MASK);

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gtk_widget_get_parent_window (widget);
	g_object_ref (widget->window);
	
	item->priv->event_window = gdk_window_new
		(gtk_widget_get_parent_window (widget),
		 &attributes, attributes_mask);

	gdk_window_set_user_data (item->priv->event_window, widget);
	
	widget->style = gtk_style_attach (widget->style, widget->window);
}

static void
impl_unrealize (GtkWidget *widget)
{
	BonoboUIToolbarControlItem *item;
	
	item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget);
	
	if (item->priv->event_window) {
		gdk_window_set_user_data (item->priv->event_window, NULL);
		gdk_window_destroy (item->priv->event_window);
		item->priv->event_window = NULL;
	}
	
	GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
impl_size_allocate (GtkWidget *widget,
		    GtkAllocation *allocation)
{
	BonoboUIToolbarControlItem *item;
	int border_width;

	item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget);
	border_width = GTK_CONTAINER (widget)->border_width;

	GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

	if (GTK_WIDGET_REALIZED (widget))
		gdk_window_move_resize
			(item->priv->event_window,
			 widget->allocation.x + border_width,
			 widget->allocation.y + border_width,
			 widget->allocation.width - border_width * 2,
			 widget->allocation.height - border_width * 2);
}

static void
impl_map (GtkWidget *widget)
{
	BonoboUIToolbarControlItem *item;

	item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget);
	
	if (item->priv->event_window) 
		gdk_window_show (item->priv->event_window);

	GTK_WIDGET_CLASS (parent_class)->map (widget);
}

static void
impl_unmap (GtkWidget *widget)
{
	BonoboUIToolbarControlItem *item;

	item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget);
	
	if (item->priv->event_window) 
		gdk_window_hide (item->priv->event_window);

	GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

/* GObject methods.  */

static void
impl_dispose (GObject *object)
{
	BonoboUIToolbarControlItem *control_item;

	control_item = (BonoboUIToolbarControlItem *) object;
	
	if (control_item->priv->widget) {
		gtk_widget_destroy (control_item->priv->widget);
		control_item->priv->control = NULL;
		control_item->priv->widget = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
impl_finalize (GObject *object)
{
	BonoboUIToolbarControlItem *control_item;

	control_item = (BonoboUIToolbarControlItem *) object;

	g_free (control_item->priv);

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

/* Gtk+ object initialization.  */

static void
bonobo_ui_toolbar_control_item_class_init (BonoboUIToolbarControlItemClass *klass)
{
        BonoboUIToolbarButtonItemClass *button_item_class;
        BonoboUIToolbarItemClass *item_class;
	GtkWidgetClass *widget_class;
	GObjectClass *object_class;
	
	button_item_class = BONOBO_UI_TOOLBAR_BUTTON_ITEM_CLASS (klass);
	item_class = BONOBO_UI_TOOLBAR_ITEM_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);
	object_class = G_OBJECT_CLASS (klass);

        button_item_class->set_icon  = impl_set_icon;
        button_item_class->set_label = impl_set_label;
        item_class->set_tooltip      = impl_set_tooltip;
        item_class->set_orientation  = impl_set_orientation;
	item_class->set_style        = impl_set_style;
	item_class->set_want_label   = impl_set_want_label;

	widget_class->realize = impl_realize;
	widget_class->unrealize = impl_unrealize;
	widget_class->size_allocate = impl_size_allocate;
	widget_class->map = impl_map;
	widget_class->unmap = impl_unmap;

	object_class->dispose  = impl_dispose;
	object_class->finalize = impl_finalize;
}

static void
bonobo_ui_toolbar_control_item_instance_init (BonoboUIToolbarControlItem *control_item)
{
        control_item->priv = g_new0 (BonoboUIToolbarControlItemPrivate, 1);
}

static void
proxy_activate_cb (GtkWidget *button, GtkObject *item)
{
	g_signal_emit_by_name (item, "activate");
}

GtkWidget *
bonobo_ui_toolbar_control_item_construct (
        BonoboUIToolbarControlItem *control_item,
	GtkWidget                  *widget,
        Bonobo_Control              control_ref)
{
        BonoboUIToolbarControlItemPrivate *priv = control_item->priv;

	if (!widget)
		widget = bonobo_widget_new_control_from_objref (
			control_ref, CORBA_OBJECT_NIL);

        if (!widget)
                return NULL;

	priv->widget   = widget;
	priv->control  = BONOBO_IS_WIDGET (widget) ? BONOBO_WIDGET (widget) : NULL;
	priv->button   = bonobo_ui_toolbar_button_item_new (NULL, NULL);
        priv->box      = gtk_vbox_new (FALSE, 0);
	
	g_signal_connect (priv->button, "activate",
			  G_CALLBACK (proxy_activate_cb), control_item);
	
	gtk_container_add (GTK_CONTAINER (priv->box), widget);
        gtk_container_add (GTK_CONTAINER (priv->box), priv->button);

	gtk_widget_show (priv->widget);
	gtk_widget_show (priv->box);

        gtk_container_add (GTK_CONTAINER (control_item), priv->box);

        return GTK_WIDGET (control_item);
}

GtkWidget *
bonobo_ui_toolbar_control_item_new (Bonobo_Control control_ref)
{
        BonoboUIToolbarControlItem *control_item;
	GtkWidget *ret;

        control_item = g_object_new (
                bonobo_ui_toolbar_control_item_get_type (), NULL);

        ret = bonobo_ui_toolbar_control_item_construct (
		control_item, NULL, control_ref);

	if (!ret)
		g_object_unref (control_item);

	return ret;
}

GtkWidget *
bonobo_ui_toolbar_control_item_new_widget (GtkWidget *custom_in_proc_widget)
{
	GtkWidget *ret;
        BonoboUIToolbarControlItem *control_item;

        control_item = g_object_new (
                bonobo_ui_toolbar_control_item_get_type (), NULL);

        ret = bonobo_ui_toolbar_control_item_construct (
		control_item, custom_in_proc_widget, CORBA_OBJECT_NIL);

	if (!ret)
		g_object_unref (custom_in_proc_widget);

	return ret;
}

void
bonobo_ui_toolbar_control_item_set_display (BonoboUIToolbarControlItem    *item,
					    BonoboUIToolbarControlDisplay  hdisplay,
					    BonoboUIToolbarControlDisplay  vdisplay)
{
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_CONTROL_ITEM (item));

	item->priv->hdisplay = hdisplay;
	item->priv->vdisplay = vdisplay;
}

void
bonobo_ui_toolbar_control_item_set_sensitive (BonoboUIToolbarControlItem *item,
					      gboolean                    sensitive)
{
	gboolean changed;

	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_CONTROL_ITEM (item));

	changed = (( GTK_WIDGET_IS_SENSITIVE (item) && !sensitive) ||
		   (!GTK_WIDGET_IS_SENSITIVE (item) &&  sensitive));

	if (!changed || !item->priv->control)
		return;

	bonobo_control_frame_control_set_state (
		bonobo_widget_get_control_frame (item->priv->control),
		sensitive ? Bonobo_Gtk_StateNormal : Bonobo_Gtk_StateInsensitive);
}
