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
#include <string.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-ui-toolbar-control-item.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag-client.h>
#include <libgnome/gnome-macros.h>

GNOME_CLASS_BOILERPLATE (BonoboUIToolbarControlItem,
			 bonobo_ui_toolbar_control_item,
			 GObject,
			 GTK_TYPE_TOOL_BUTTON);

static void
set_control_property_bag_value (BonoboUIToolbarControlItem *item,
				const char                 *name,
				BonoboArg                  *value)
{
	BonoboControlFrame *frame;
	Bonobo_PropertyBag bag;

	if (!item->control)
		return;

	frame = bonobo_widget_get_control_frame (item->control);
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

MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (gint,     gint,         INT)
     /* MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (gboolean, gboolean,     BOOLEAN)
	MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (string,   const char *, STRING) */

static GtkToolbar *
get_parent_toolbar (BonoboUIToolbarControlItem *control_item)
{
	GtkWidget *toolbar;

	toolbar = GTK_WIDGET (control_item)->parent;
	if (!GTK_IS_TOOLBAR (toolbar)) {
		g_warning ("Non-toolbar parent '%s'", g_type_name_from_instance (toolbar));
		return NULL;
	}

	return GTK_TOOLBAR (toolbar);
}

static BonoboUIToolbarControlDisplay
get_display_mode (BonoboUIToolbarControlItem *control_item)
{
	GtkToolbar *toolbar = get_parent_toolbar (control_item);
	g_return_val_if_fail (toolbar != NULL, BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL);
	
	if (gtk_toolbar_get_orientation (toolbar) == GTK_ORIENTATION_HORIZONTAL)
		return control_item->hdisplay;
	else
		return control_item->vdisplay;
}

/*
 * We are assuming that there's room in horizontal orientation, but not
 * vertical. This can be made more sophisticated by looking at the control's
 * requested geometry.
 */
static void
impl_toolbar_reconfigured (GtkToolItem *item)
{
	GtkToolbar *toolbar;
	GtkOrientation orientation;
	BonoboUIToolbarControlDisplay display;
	BonoboUIToolbarControlItem *control_item = (BonoboUIToolbarControlItem *) item;

	if (GTK_WIDGET (item)->parent == NULL)
		return;

	toolbar = get_parent_toolbar (control_item);
	g_return_if_fail (toolbar != NULL);

	orientation = gtk_toolbar_get_orientation (toolbar);

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		display = control_item->hdisplay;
	else
		display = control_item->vdisplay;
	
	switch (display) {

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL:
		gtk_widget_hide (control_item->button);
		gtk_widget_show (control_item->widget);
		break;

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_BUTTON:
		gtk_widget_hide (control_item->widget);
		gtk_widget_show (control_item->button);
		break;

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_NONE:
		gtk_widget_hide (control_item->widget);
		gtk_widget_hide (control_item->button);
		break;

	default:
		g_assert_not_reached ();
	}

	set_control_property_bag_gint (control_item, "orientation", orientation);
	set_control_property_bag_gint (control_item, "style",
				       gtk_toolbar_get_style (toolbar));

	GNOME_CALL_PARENT (GTK_TOOL_ITEM_CLASS, toolbar_reconfigured, (item));
}

static void
impl_dispose (GObject *object)
{
	BonoboUIToolbarControlItem *control_item;

	control_item = (BonoboUIToolbarControlItem *) object;
	
	if (control_item->widget) {
		gtk_widget_destroy (control_item->widget);
		control_item->control = NULL;
		control_item->widget = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
menu_item_map (GtkWidget *menu_item, BonoboUIToolbarControlItem *control_item)
{
	if (GTK_BIN (menu_item)->child)
		return;

	g_object_ref (control_item->widget);
	gtk_container_remove (GTK_CONTAINER (control_item->box), 
			      control_item->widget);
	gtk_container_add (GTK_CONTAINER (menu_item), control_item->widget);
	g_object_unref (control_item->widget);
}

static void
menu_item_return_control (GtkWidget *menu_item, BonoboUIToolbarControlItem *control_item)
{
	if (!GTK_BIN (menu_item)->child)
		return;

	if (GTK_BIN (menu_item)->child == control_item->widget) {
		g_object_ref (control_item->widget);
		gtk_container_remove (GTK_CONTAINER (menu_item), 
				      control_item->widget);
		gtk_container_add (GTK_CONTAINER (control_item->box), control_item->widget);
		g_object_unref (control_item->widget);
	}
}

static gboolean
impl_create_menu_proxy (GtkToolItem *tool_item)
{
	GtkWidget *menu_item;
	BonoboUIToolbarControlItem *control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (tool_item);

	if (get_display_mode (control_item) == BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_NONE)
		return FALSE;

	if (control_item->hdisplay != BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL ||
	    control_item->vdisplay != BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL)
		/* Can cope with just a button */
		return GTK_TOOL_ITEM_CLASS (parent_class)->create_menu_proxy (tool_item);

	menu_item = gtk_menu_item_new ();

	/* This sucks, but the best we can do */
	g_signal_connect (menu_item, "map", G_CALLBACK (menu_item_map), tool_item);
	g_signal_connect (menu_item, "destroy", G_CALLBACK (menu_item_return_control), tool_item);
	gtk_tool_item_set_proxy_menu_item (tool_item,
					   "bonobo-control-button-menu-id",
					   menu_item);

	return TRUE;
}

static void
impl_notify (GObject    *object,
	     GParamSpec *pspec)
{
	BonoboUIToolbarControlItem *control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (object);

	if (control_item->control && !strcmp (pspec->name, "sensitive"))
		bonobo_control_frame_control_set_state
			(bonobo_widget_get_control_frame (control_item->control),
			 GTK_WIDGET_SENSITIVE (control_item) ? Bonobo_Gtk_StateNormal : Bonobo_Gtk_StateInsensitive);

	GNOME_CALL_PARENT (G_OBJECT_CLASS, notify, (object, pspec));
}

static gboolean
impl_map_event (GtkWidget   *widget,
		GdkEventAny *event)
{
	BonoboUIToolbarControlItem *control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (widget);

	if (control_item->widget && control_item->widget->parent != control_item->box)
		menu_item_return_control (control_item->widget->parent, control_item);

	return GTK_WIDGET_CLASS (parent_class)->map_event (widget, event);
}

static void
bonobo_ui_toolbar_control_item_class_init (BonoboUIToolbarControlItemClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;
	GtkToolItemClass *tool_item_class = (GtkToolItemClass *) klass;
	
	object_class->dispose  = impl_dispose;
	object_class->notify = impl_notify;

	widget_class->map_event = impl_map_event;

	tool_item_class->create_menu_proxy = impl_create_menu_proxy;
	tool_item_class->toolbar_reconfigured = impl_toolbar_reconfigured;
}

static void
bonobo_ui_toolbar_control_item_instance_init (BonoboUIToolbarControlItem *control_item)
{
	gtk_tool_item_set_homogeneous (GTK_TOOL_ITEM (control_item), FALSE);

	control_item->box = gtk_vbox_new (FALSE, 0);

	control_item->button = GTK_BIN (control_item)->child;
	g_object_ref (control_item->button);
	gtk_object_sink (GTK_OBJECT (control_item->button));
	gtk_container_remove (GTK_CONTAINER (control_item), control_item->button);
	gtk_container_add (GTK_CONTAINER (control_item->box), control_item->button);
	g_object_ref (control_item->button);

	gtk_container_add (GTK_CONTAINER (control_item), control_item->box);
	gtk_widget_show (control_item->box);
}

GtkWidget *
bonobo_ui_toolbar_control_item_construct (
        BonoboUIToolbarControlItem *control_item,
	GtkWidget                  *widget,
        Bonobo_Control              control_ref)
{
	if (!widget)
		widget = bonobo_widget_new_control_from_objref (
			control_ref, CORBA_OBJECT_NIL);

        if (!widget)
                return NULL;

	control_item->widget  = widget;
	control_item->control = BONOBO_IS_WIDGET (widget) ? BONOBO_WIDGET (widget) : NULL;
	
	gtk_container_add (GTK_CONTAINER (control_item->box), control_item->widget);

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

	item->hdisplay = hdisplay;
	item->vdisplay = vdisplay;
}
