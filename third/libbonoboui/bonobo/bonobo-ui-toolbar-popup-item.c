/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-popup-item.h
 *
 * Author:
 *    Ettore Perazzoli (ettore@ximian.com)
 *
 * Copyright (C) 2000 Ximian, Inc.
 */

#include <config.h>
#include <bonobo/bonobo-ui-toolbar-popup-item.h>
#include <libgnome/gnome-macros.h>

GNOME_CLASS_BOILERPLATE (BonoboUIToolbarPopupItem,
			 bonobo_ui_toolbar_popup_item,
			 BonoboUIToolbarToggleButtonItem,
			 bonobo_ui_toolbar_toggle_button_item_get_type ());

static GdkPixbuf *right_arrow_pixbuf = NULL;
static GdkPixbuf *down_arrow_pixbuf = NULL;

static const char *right_arrow_xpm_data[] = {
	"8 10 2 1",
	" 	c none",
	".	c #000000000000",
	"        ",
	" .      ",
	" ..     ",
	" ...    ",
	" ....   ",
	" .....  ",
	" ....   ",
	" ...    ",
	" ..     ",
	" .      ",
	"        "
};

static const char *down_arrow_xpm_data[] = {
	"11 7 2 1",
	" 	c none",
	".	c #000000000000",
	"           ",
	" ......... ",
	"  .......  ",
	"   .....   ",
	"    ...    ",
	"     .     ",
	" 	    ",
};


/* Utility functions.  */

static void
create_arrow_pixbufs (void)
{
	g_assert (right_arrow_pixbuf == NULL);
	right_arrow_pixbuf = gdk_pixbuf_new_from_xpm_data (right_arrow_xpm_data);

	g_assert (down_arrow_pixbuf == NULL);
	down_arrow_pixbuf = gdk_pixbuf_new_from_xpm_data (down_arrow_xpm_data);
}

static GdkPixbuf *
get_icon_for_orientation (BonoboUIToolbarPopupItem *popup_item)
{
	GtkOrientation orientation;

	orientation = bonobo_ui_toolbar_item_get_orientation (BONOBO_UI_TOOLBAR_ITEM (popup_item));

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		return right_arrow_pixbuf;
	else
		return down_arrow_pixbuf;
}


static void
impl_set_orientation (BonoboUIToolbarItem *item,
		      GtkOrientation orientation)
{
	BonoboUIToolbarPopupItem *popup_item;
	GtkWidget *image;
	GdkPixbuf *icon;

	GNOME_CALL_PARENT (
		BONOBO_UI_TOOLBAR_ITEM_CLASS, set_orientation, (item, orientation));

	popup_item = BONOBO_UI_TOOLBAR_POPUP_ITEM (item);

	icon = get_icon_for_orientation (popup_item);
	image = gtk_image_new_from_pixbuf (icon);

	bonobo_ui_toolbar_button_item_set_image (BONOBO_UI_TOOLBAR_BUTTON_ITEM (item), image);
}

static void
bonobo_ui_toolbar_popup_item_class_init (
	BonoboUIToolbarPopupItemClass *popup_item_class)
{
	BonoboUIToolbarItemClass *toolbar_item_class;

	toolbar_item_class = BONOBO_UI_TOOLBAR_ITEM_CLASS (popup_item_class);
	toolbar_item_class->set_orientation = impl_set_orientation;

	create_arrow_pixbufs ();
}

static void
bonobo_ui_toolbar_popup_item_instance_init (
	BonoboUIToolbarPopupItem *toolbar_popup_item)
{
}

void
bonobo_ui_toolbar_popup_item_construct (BonoboUIToolbarPopupItem *popup_item)
{
	GdkPixbuf *icon;

	g_return_if_fail (popup_item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_POPUP_ITEM (popup_item));

	icon = get_icon_for_orientation (popup_item);

	bonobo_ui_toolbar_toggle_button_item_construct (BONOBO_UI_TOOLBAR_TOGGLE_BUTTON_ITEM (popup_item), icon, NULL);
}

GtkWidget *
bonobo_ui_toolbar_popup_item_new (void)
{
	BonoboUIToolbarPopupItem *popup_item;

	popup_item = g_object_new (
		bonobo_ui_toolbar_popup_item_get_type (), NULL);

	bonobo_ui_toolbar_popup_item_construct (popup_item);

	return GTK_WIDGET (popup_item);
}
