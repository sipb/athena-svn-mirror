/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * widget-color-combo.h - A color selector combo box
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

#ifndef GNUMERIC_WIDGET_COLOR_COMBO_H
#define GNUMERIC_WIDGET_COLOR_COMBO_H

#include <gtk/gtkwidget.h>
#include <libgnome/gnome-defs.h>
#include <gal/widgets/gtk-combo-box.h>
#include <gal/widgets/color-palette.h>

BEGIN_GNOME_DECLS

typedef struct _ColorCombo {
	GtkComboBox     combo_box;

	/*
	 * Canvas where we display
	 */
	GtkWidget       *preview_button;
	GnomeCanvas     *preview_canvas;
	GnomeCanvasItem *preview_color_item;
	ColorPalette    *palette;

        GdkColor *default_color;
	gboolean  trigger;
} ColorCombo;

typedef struct {
	GtkComboBoxClass parent_class;

	/* Signals emited by this widget */
	void (* changed) (ColorCombo *color_combo, GdkColor *color, gboolean by_user);
} ColorComboClass;

#define COLOR_COMBO_TYPE     (color_combo_get_type ())
#define COLOR_COMBO(obj)     (GTK_CHECK_CAST((obj), COLOR_COMBO_TYPE, ColorCombo))
#define COLOR_COMBO_CLASS(k) (GTK_CHECK_CLASS_CAST(k), COLOR_COMBO_TYPE)
#define IS_COLOR_COMBO(obj)  (GTK_CHECK_TYPE((obj), COLOR_COMBO_TYPE))


GtkType    color_combo_get_type   (void);
GtkWidget *color_combo_new        (char       **icon,
				   const char  *no_color_label,
				   GdkColor    *default_color,
				   ColorGroup  *color_group);
void       color_combo_set_color  (ColorCombo  *cc,
				   GdkColor    *color);
GdkColor  *color_combo_get_color  (ColorCombo  *cc);

END_GNOME_DECLS

#endif /* GNUMERIC_WIDGET_COLOR_COMBO_H */
