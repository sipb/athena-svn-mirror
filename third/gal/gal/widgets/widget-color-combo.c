/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * widget-color-combo.c - A color selector combo box
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Dom Lachowicz (dominicl@seas.upenn.edu)
 *
 * Reworked and split up into a separate ColorPalette object:
 *   Michael Levy (mlevy@genoscope.cns.fr)
 *
 * And later revised and polished by:
 *   Almer S. Tigelaar (almer@gnome.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>

#include <gtk/gtkentry.h>
#include <gtk/gtksignal.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeui/gnome-canvas-image.h>
#include <libgnomeui/gnome-canvas-rect-ellipse.h>
#include <libgnomeui/gnome-preferences.h>
#include "e-colors.h"
#include "widget-color-combo.h"
#include "gal/util/e-util.h"

enum {
	CHANGED,
	LAST_SIGNAL
};

static gint color_combo_signals [LAST_SIGNAL] = { 0, };

static GtkObjectClass *color_combo_parent_class;

#define make_color(CC,COL) (((COL) != NULL) ? (COL) : ((CC) ? ((CC)->default_color) : NULL))

static void
color_combo_set_color_internal (ColorCombo *cc, GdkColor *color)
{
	GdkColor *new_color;
	GdkColor *outline_color;

	new_color = make_color (cc,color);
	/* If the new and the default are NULL draw an outline */
	outline_color = (new_color) ? new_color : &e_dark_gray;

	gnome_canvas_item_set (cc->preview_color_item,
			       "fill_color_gdk", new_color,
			       "outline_color_gdk", outline_color,
			       NULL);
}

static void
color_combo_finalize (GtkObject *object)
{
	(*color_combo_parent_class->finalize) (object);
}

typedef void (*GtkSignal_NONE__POINTER_BOOL) (GtkObject * object,
					      gpointer arg1,
					      gboolean arg2,
					      gpointer user_data);
static void
marshal_NONE__POINTER_BOOL (GtkObject * object,
			    GtkSignalFunc func,
			    gpointer func_data,
			    GtkArg * args)
{
	GtkSignal_NONE__POINTER_BOOL rfunc;
	rfunc = (GtkSignal_NONE__POINTER_BOOL) func;
	(*rfunc) (object,
		  GTK_VALUE_POINTER (args[0]),
		  GTK_VALUE_BOOL    (args[1]),
		  func_data);
}

static void
color_combo_class_init (GtkObjectClass *object_class)
{
	object_class->finalize = color_combo_finalize;

	color_combo_parent_class = gtk_type_class (gtk_combo_box_get_type ());

	color_combo_signals [CHANGED] =
		gtk_signal_new (
			"changed",
			GTK_RUN_LAST,
			E_OBJECT_CLASS_TYPE (object_class),
			GTK_SIGNAL_OFFSET (ColorComboClass, changed),
			marshal_NONE__POINTER_BOOL,
			GTK_TYPE_NONE, 2, GTK_TYPE_POINTER, GTK_TYPE_BOOL);

	E_OBJECT_CLASS_ADD_SIGNALS (object_class, color_combo_signals, LAST_SIGNAL);
}

GtkType
color_combo_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"ColorCombo",
			sizeof (ColorCombo),
			sizeof (ColorComboClass),
			(GtkClassInitFunc) color_combo_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_combo_box_get_type (), &info);
	}

	return type;
}

/*
 * Fires signal "changed" with the current color as its param
 */
static void
emit_change (ColorCombo *cc, GdkColor *color, gboolean by_user)
{
  	gtk_signal_emit (
		GTK_OBJECT (cc),
		color_combo_signals [CHANGED],
		color, by_user);
	gtk_combo_box_popup_hide (GTK_COMBO_BOX (cc));
}

static void
cb_color_change (ColorPalette *P, GdkColor *color,
		 gboolean custom, gboolean by_user,
		 ColorCombo *cc)
{
	color_combo_set_color_internal (cc, color);
		
	emit_change (cc, color, by_user);
}

static void
preview_clicked (GtkWidget *button, ColorCombo *cc)
{
	GdkColor *color = color_palette_get_current_color (cc->palette);
	emit_change (cc, color, TRUE);
	
	if (color)
		gdk_color_free (color);
}

static void
cb_cust_color_clicked (GtkWidget *widget, ColorCombo *cc)
{
	gtk_combo_box_popup_hide (GTK_COMBO_BOX (cc));
}

/*
 * Creates the color table
 */
static void
color_table_setup (ColorCombo *cc,
		   char const *no_color_label, ColorGroup *color_group)
{
	g_return_if_fail (cc != NULL);

	/* Tell the palette that we will be changing it's custom colors */
	cc->palette = 
		COLOR_PALETTE (color_palette_new (no_color_label, 
						  cc->default_color,
						  color_group));

	{
		GtkWidget *picker = color_palette_get_color_picker (cc->palette);
		gtk_signal_connect (GTK_OBJECT (picker), "clicked",
				    GTK_SIGNAL_FUNC (cb_cust_color_clicked), cc);
	}
	
	gtk_signal_connect (GTK_OBJECT (cc->palette), "changed",
			    GTK_SIGNAL_FUNC (cb_color_change), cc);

	gtk_widget_show_all (GTK_WIDGET (cc->palette));

	return;
}

/*
 * Where the actual construction goes on
 */
static void
color_combo_construct (ColorCombo *cc, char **icon,
		       char const * const no_color_label,
		       ColorGroup *color_group)
{
	GdkImlibImage *image;

	g_return_if_fail (cc != NULL);
	g_return_if_fail (IS_COLOR_COMBO (cc));

	/*
	 * Our button with the canvas preview
	 */
	cc->preview_button = gtk_button_new ();
	if (!gnome_preferences_get_toolbar_relief_btn ())
		gtk_button_set_relief (GTK_BUTTON (cc->preview_button), GTK_RELIEF_NONE);

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
	cc->preview_canvas = GNOME_CANVAS (gnome_canvas_new ());
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();

	gnome_canvas_set_scroll_region (cc->preview_canvas, 0, 0, 24, 24);
	if (icon) {
		image = gdk_imlib_create_image_from_xpm_data (icon);

		gnome_canvas_item_new (
			GNOME_CANVAS_GROUP (gnome_canvas_root (cc->preview_canvas)),
			gnome_canvas_image_get_type (),
			"image",  image,
			"x",      0.0,
			"y",      0.0,
			"width",  (double) image->rgb_width,
			"height", (double) image->rgb_height,
			"anchor", GTK_ANCHOR_NW,
			NULL);
		
		cc->preview_color_item = gnome_canvas_item_new (
			GNOME_CANVAS_GROUP (gnome_canvas_root (cc->preview_canvas)),
			gnome_canvas_rect_get_type (),
			"x1",         3.0,
			"y1",         19.0,
			"x2",         20.0,
			"y2",         22.0,
			"fill_color", "black",
			"width_pixels", 1,
			NULL);
	} else
		cc->preview_color_item = gnome_canvas_item_new (
			GNOME_CANVAS_GROUP (gnome_canvas_root (cc->preview_canvas)),
			gnome_canvas_rect_get_type (),
			"x1",         2.0,
			"y1",         1.0,
			"x2",         21.0,
			"y2",         22.0,
			"fill_color", "black",
			"width_pixels", 1,
			NULL);
	
	gtk_container_add (GTK_CONTAINER (cc->preview_button), GTK_WIDGET (cc->preview_canvas));
	gtk_widget_set_usize (GTK_WIDGET (cc->preview_canvas), 24, 24);
	gtk_signal_connect (GTK_OBJECT (cc->preview_button), "clicked",
			    GTK_SIGNAL_FUNC (preview_clicked), cc);
	
	/*
	 * Our table selector
	 */
	color_table_setup (cc, no_color_label, color_group);
	
	gtk_widget_show_all (cc->preview_button);
	
	gtk_combo_box_construct (GTK_COMBO_BOX (cc),
				 cc->preview_button,
				 GTK_WIDGET (cc->palette));

	if (!gnome_preferences_get_toolbar_relief_btn ())
		gtk_combo_box_set_arrow_relief (GTK_COMBO_BOX (cc), GTK_RELIEF_NONE);

	/* Set our color to the current color on the palette */
	color_combo_set_color_internal (cc, color_palette_get_current_color (cc->palette));
}

/* color_combo_get_color:
 *
 * Return current color, result must be freed with gdk_color_free !
 */
GdkColor *
color_combo_get_color (ColorCombo *cc)
{
	return color_palette_get_current_color (cc->palette);
}

void
color_combo_set_color (ColorCombo *cc, GdkColor *color)
{
	/* This will change the color on the palette than
	 * it will invoke cb_color_change which will call emit_change
	 * and set_color_internal which will change the color on
	 * our preview and will let the users of the combo know
	 * that the current color has changed
	 */
	color_palette_set_current_color (cc->palette, color);
}

/*
 * Default constructor. Pass an XPM icon and an optional label for
 * the no/auto color button.
 */
GtkWidget *
color_combo_new (char **icon, const char *no_color_label,
		 GdkColor *default_color,
		 ColorGroup  *color_group)
{
	ColorCombo *cc;

	cc = gtk_type_new (color_combo_get_type ());

        cc->default_color = default_color;

	color_combo_construct (cc, icon, no_color_label, color_group);

	return GTK_WIDGET (cc);
}
