/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-font-picker.c - A simple widget to select scalable fonts.

   Copyright (C) 1999, 2000 Eazel, Inc.
   Copyright (C) 2002, Bent Spoon Software

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Ramiro Estrugo <ramiro@eazel.com>, Darin Adler <darin@bentspoon.com>
*/

#include <config.h>
#include "eel-font-picker.h"

#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkmenushell.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtksignal.h>
#include <libgnome/gnome-macros.h>
#include <stdlib.h>
#include <string.h>

#define FONT_PICKER_SPACING 10

#define FAMILY_KEY "eel-family"
#define FACE_KEY "eel-face"

enum {
	CHANGED,
	LAST_SIGNAL
};

struct EelFontPickerDetails
{
	GtkWidget *option_menu;
	PangoFontDescription *selected_font;
};

GNOME_CLASS_BOILERPLATE (EelFontPicker, eel_font_picker,
			 EelCaption, EEL_TYPE_CAPTION)

static guint signals[LAST_SIGNAL];

static GtkMenuShell *
get_family_menu (EelFontPicker *font_picker)
{
	return GTK_MENU_SHELL (gtk_option_menu_get_menu (GTK_OPTION_MENU (font_picker->details->option_menu)));
}

static int
font_picker_get_index_for_font (EelFontPicker *font_picker,
				PangoFontDescription *font)
{
	GList *node;
	int i;
	const char *family_name;
	PangoFontFamily *family;

	g_return_val_if_fail (EEL_IS_FONT_PICKER (font_picker), -1);

	if (font == NULL) {
		return -1;
	}

	family_name = pango_font_description_get_family (font);
	if (family_name == NULL) {
		return -1;
	}

	for (node = get_family_menu (font_picker)->children, i = 0; node != NULL; node = node->next, i++) {

		family = PANGO_FONT_FAMILY (g_object_get_data (G_OBJECT (node->data), FAMILY_KEY));
		if (strcmp (pango_font_family_get_name (family), family_name) == 0) {
			return i;
		}
	}

	return -1;
}

static void
select_option_menu_item_for_current_font (EelFontPicker *font_picker)
{
	gtk_option_menu_set_history (GTK_OPTION_MENU (font_picker->details->option_menu),
				     font_picker_get_index_for_font (font_picker,
								     font_picker->details->selected_font));
}
	
/* In case no valid selection was made, we restore the old selected
 * menu item since the font didn't change. It does no harm to also
 * do that if a valid selection was made.
 */
static void
menu_deactivate_callback (GtkMenuShell *menu_shell,
			  EelFontPicker *font_picker)
{
	g_assert (GTK_IS_MENU_SHELL (menu_shell));
	g_assert (EEL_IS_FONT_PICKER (font_picker));
	
	select_option_menu_item_for_current_font (font_picker);
	
	/* This is needed to work around a painting bug in menus. In
	 * some themes (like the Eazel Crux theme) 'almost selected'
	 * items will appear white for some crazy reason.
	 */
	gtk_widget_queue_draw (GTK_WIDGET (menu_shell));
}

static void
eel_font_picker_instance_init (EelFontPicker *font_picker)
{
	GtkWidget *menu;

	font_picker->details = g_new0 (EelFontPickerDetails, 1);

	gtk_box_set_spacing (GTK_BOX (font_picker), FONT_PICKER_SPACING);

	menu = gtk_menu_new ();
	g_signal_connect (menu,
			  "deactivate",
			  G_CALLBACK (menu_deactivate_callback),
			  font_picker);

	font_picker->details->option_menu = gtk_option_menu_new ();
	gtk_option_menu_set_menu (GTK_OPTION_MENU (font_picker->details->option_menu), menu);

	eel_caption_set_child (EEL_CAPTION (font_picker),
			       font_picker->details->option_menu,
			       FALSE,
			       FALSE);
}

static void
eel_font_picker_finalize (GObject *object)
{
	EelFontPicker *font_picker;

	g_return_if_fail (EEL_IS_FONT_PICKER (object));
	
	font_picker = EEL_FONT_PICKER (object);

	if (font_picker->details->selected_font != NULL) {
		pango_font_description_free (font_picker->details->selected_font);
	}
	g_free (font_picker->details);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}


static void
select_font (EelFontPicker *font_picker,
	     PangoFontDescription *font)
{
	g_return_if_fail (EEL_IS_FONT_PICKER (font_picker));

	if (font == NULL) {
		if (font_picker->details->selected_font == NULL) {
			return;
		}
	} else {
		if (font_picker->details->selected_font != NULL
		    && pango_font_description_equal (font, font_picker->details->selected_font)) {
			return;
		}
	}

	if (font_picker->details->selected_font != NULL) {
		pango_font_description_free (font_picker->details->selected_font);
	}
	font_picker->details->selected_font = font == NULL ? NULL : pango_font_description_copy (font);

	select_option_menu_item_for_current_font (font_picker);
	
	g_signal_emit (font_picker, signals[CHANGED], 0);
}

static void
face_menu_item_activate_callback (GtkWidget *menu_item,
				  EelFontPicker *font_picker)
{
	PangoFontDescription *font;
	
	g_assert (GTK_IS_MENU_ITEM (menu_item));
	g_assert (EEL_IS_FONT_PICKER (font_picker));

	font = pango_font_face_describe (g_object_get_data (G_OBJECT (menu_item), FACE_KEY));
	select_font (font_picker, font);
	pango_font_description_free (font);
}

static int
compare_family_pointers_by_name (gconstpointer a, gconstpointer b)
{
	PangoFontFamily * const *fa;
	PangoFontFamily * const *fb;

	fa = a;
	fb = b;

	return g_utf8_collate (pango_font_family_get_name (*fa),
			       pango_font_family_get_name (*fb));
}

static void
font_picker_populate (EelFontPicker *font_picker, PangoContext *context)
{
	PangoFontFamily **families;
	int n_families, i;
	PangoFontFace **faces;
	int n_faces, j;
	GtkMenuShell *family_menu;
	GtkWidget *family_menu_item, *face_menu, *face_menu_item;

	g_return_if_fail (EEL_IS_FONT_PICKER (font_picker));

	family_menu = get_family_menu (font_picker);
	
	pango_context_list_families (context, &families, &n_families);
	qsort (families, n_families, sizeof (*families),
	       compare_family_pointers_by_name);
	/* sort alphabetically with g_utf8_collate? */

	for (i = 0; i < n_families; i++) {
		pango_font_family_list_faces (families[i], &faces, &n_faces);
		/* sort? how? */

		face_menu = gtk_menu_new ();
		gtk_widget_show (face_menu);

		for (j = 0; j < n_faces; j++) {
			face_menu_item = gtk_menu_item_new_with_label
				(pango_font_face_get_face_name (faces[j]));
			gtk_widget_show (face_menu_item);

			gtk_menu_shell_append (GTK_MENU_SHELL (face_menu), face_menu_item);

			g_object_ref (faces[j]);
			g_object_set_data_full (G_OBJECT (face_menu_item), FACE_KEY,
						faces[j], g_object_unref);
						
			g_signal_connect (face_menu_item,
					  "activate",
					  G_CALLBACK (face_menu_item_activate_callback),
					  font_picker);
		}

		g_free (faces);

		family_menu_item = gtk_menu_item_new_with_label (pango_font_family_get_name (families[i]));
		gtk_widget_show (family_menu_item);
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (family_menu_item), face_menu);
		gtk_menu_shell_append (family_menu, family_menu_item);

		g_object_ref (families[i]);
		g_object_set_data_full (G_OBJECT (family_menu_item), FAMILY_KEY,
					families[i], g_object_unref);
						
	}

	g_free (families);
}

GtkWidget *
eel_font_picker_new (PangoContext *context)
{
	GtkWidget *widget;

	widget = gtk_widget_new (eel_font_picker_get_type (), NULL);
	if (context != NULL) {
		font_picker_populate (EEL_FONT_PICKER (widget), context);
	} else {
		font_picker_populate (EEL_FONT_PICKER (widget),
				      gtk_widget_get_pango_context (widget));
	}
	gtk_widget_ensure_style (widget);
	select_font (EEL_FONT_PICKER (widget), gtk_widget_get_style (widget)->font_desc);
	return widget;
}

char *
eel_font_picker_get_selected_font (EelFontPicker *font_picker)
{
	g_return_val_if_fail (EEL_IS_FONT_PICKER (font_picker), NULL);

	if (font_picker->details->selected_font == NULL) {
		return NULL;
	}
	return pango_font_description_to_string (font_picker->details->selected_font);
}

void
eel_font_picker_set_selected_font (EelFontPicker *font_picker,
				   const char *font_str)
{
	select_font (font_picker, pango_font_description_from_string (font_str));
}

static void
eel_font_picker_class_init (EelFontPickerClass *class)
{
	G_OBJECT_CLASS (class)->finalize = eel_font_picker_finalize;

	signals[CHANGED] = g_signal_new ("changed",
					 G_TYPE_FROM_CLASS (class),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET (EelFontPickerClass, changed),
					 NULL, NULL,
					 g_cclosure_marshal_VOID__VOID,
					 G_TYPE_NONE, 
					 0);
}
