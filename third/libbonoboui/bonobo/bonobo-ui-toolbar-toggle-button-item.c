/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-toggle-button-item.h
 *
 * Author:
 *     Ettore Perazzoli (ettore@ximian.com)
 *
 * Copyright (C) 2000 Ximian, Inc.
 */

#include <config.h>
#include <stdlib.h>
#include <libgnome/gnome-macros.h>
#include <bonobo/bonobo-ui-toolbar-toggle-button-item.h>

GNOME_CLASS_BOILERPLATE (BonoboUIToolbarToggleButtonItem,
			 bonobo_ui_toolbar_toggle_button_item,
			 GObject, 
			 bonobo_ui_toolbar_button_item_get_type ());

enum {
	TOGGLED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

/* GtkToggleButton callback.  */

static void
button_widget_toggled_cb (GtkToggleButton *toggle_button,
			  gpointer         user_data)
{
	g_signal_emit (user_data, signals[TOGGLED], 0);
}

static void
impl_set_state (BonoboUIToolbarItem *item,
		const char          *state)
{
	GtkButton *button;
	gboolean   active = atoi (state);

	button = bonobo_ui_toolbar_button_item_get_button_widget (
		BONOBO_UI_TOOLBAR_BUTTON_ITEM (item));

	if (GTK_WIDGET_STATE (GTK_WIDGET (button)) != active)
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (button), active);
}		

/* GObject initialization.  */

static void
bonobo_ui_toolbar_toggle_button_item_class_init (
	BonoboUIToolbarToggleButtonItemClass *klass)
{
	BonoboUIToolbarItemClass *item_class = (BonoboUIToolbarItemClass *) klass;

	item_class->set_state = impl_set_state;

	signals[TOGGLED] = g_signal_new (
		"toggled", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST,
		G_STRUCT_OFFSET (BonoboUIToolbarToggleButtonItemClass, toggled),
		NULL, NULL, g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);
}


static void
bonobo_ui_toolbar_toggle_button_item_instance_init (
	BonoboUIToolbarToggleButtonItem *toolbar_toggle_button_item)
{
	/* Nothing to do here.  */
}

static void
proxy_toggle_click_cb (GtkWidget *button, GtkObject *item)
{
	gboolean active;
	char    *new_state;

	active = gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (button));

	new_state = g_strdup_printf ("%d", active);

	g_signal_emit_by_name (item, "state_altered", new_state);

	g_free (new_state);
}

void
bonobo_ui_toolbar_toggle_button_item_construct (BonoboUIToolbarToggleButtonItem *toggle_button_item,
					     GdkPixbuf *icon,
					     const char *label)
{
	GtkWidget *button_widget;

	button_widget = gtk_toggle_button_new ();

	g_signal_connect_object (
		button_widget, "toggled",
		G_CALLBACK (button_widget_toggled_cb),
		toggle_button_item, 0);

	g_signal_connect_object (
		button_widget, "clicked",
		G_CALLBACK (proxy_toggle_click_cb),
		toggle_button_item, 0);

	bonobo_ui_toolbar_button_item_construct (
		BONOBO_UI_TOOLBAR_BUTTON_ITEM (toggle_button_item),
		GTK_BUTTON (button_widget), icon, label);
}

GtkWidget *
bonobo_ui_toolbar_toggle_button_item_new (GdkPixbuf *icon,
				       const char *label)
{
	BonoboUIToolbarToggleButtonItem *toggle_button_item;

	toggle_button_item = g_object_new (
		bonobo_ui_toolbar_toggle_button_item_get_type (), NULL);

	bonobo_ui_toolbar_toggle_button_item_construct (toggle_button_item, icon, label);

	return GTK_WIDGET (toggle_button_item);
}


void
bonobo_ui_toolbar_toggle_button_item_set_active (BonoboUIToolbarToggleButtonItem *item,
					      gboolean active)
{
	GtkButton *button_widget;

	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_TOGGLE_BUTTON_ITEM (item));

	button_widget = bonobo_ui_toolbar_button_item_get_button_widget (BONOBO_UI_TOOLBAR_BUTTON_ITEM (item));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_widget), active);
}

gboolean
bonobo_ui_toolbar_toggle_button_item_get_active (BonoboUIToolbarToggleButtonItem *item)
{
	GtkButton *button_widget;

	g_return_val_if_fail (item != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR_TOGGLE_BUTTON_ITEM (item), FALSE);

	button_widget = bonobo_ui_toolbar_button_item_get_button_widget (BONOBO_UI_TOOLBAR_BUTTON_ITEM (item));

	return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_widget));
}
