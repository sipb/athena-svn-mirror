/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * test-e-font.c
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
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

#define _TEST_E_FONT_C_

#include <gnome.h>
#include "e-font.h"
#include "test-e-font.h"

#define PX_WIDTH 480
#define PX_HEIGHT 24

static void
window_delete (GtkWidget * w, GdkEventAny * event)
{
	gtk_main_quit ();
}

static void
font_set (GnomeFontPicker * fp, gchar * name, gpointer data)
{
	GtkWidget * w;
	GdkFont * font;
	gchar * n, * p;
	EFont * efont;
	GdkPixmap * px;

	g_print ("Selected: %s\n", name);

	w = gtk_object_get_data (GTK_OBJECT (data), "label");
	gtk_label_set_text (GTK_LABEL (w), name);

	font = gnome_font_picker_get_font (fp);
	efont = e_font_from_gdk_font (font);
	w = gtk_object_get_data (GTK_OBJECT (data), "px1");
	px = GTK_PIXMAP (w)->pixmap;
	gdk_draw_rectangle (px, w->style->bg_gc[GTK_STATE_NORMAL], TRUE, 0, 0, PX_WIDTH, PX_HEIGHT);
	e_font_draw_utf8_text (px, efont, E_FONT_PLAIN, w->style->text_gc[GTK_STATE_NORMAL], 4, 20, "Test text", 9);
	gtk_widget_queue_draw (w);
	e_font_unref (efont);

	efont = e_font_from_gdk_name (name);
	w = gtk_object_get_data (GTK_OBJECT (data), "px2");
	px = GTK_PIXMAP (w)->pixmap;
	gdk_draw_rectangle (px, w->style->bg_gc[GTK_STATE_NORMAL], TRUE, 0, 0, PX_WIDTH, PX_HEIGHT);
	e_font_draw_utf8_text (px, efont, E_FONT_PLAIN, w->style->text_gc[GTK_STATE_NORMAL], 4, 20, "Test text", 9);
	gtk_widget_queue_draw (w);
	e_font_unref (efont);

	name = g_strdup (name);
	n = name + 1;
	while (*n != '-') n++;
	n++;
	p = n;
	while (*p != '-') p++;
	*p = '\0';

	g_print ("Short name: %s\n", n);
	
	efont = e_font_from_gdk_name (n);
	w = gtk_object_get_data (GTK_OBJECT (data), "px3");
	px = GTK_PIXMAP (w)->pixmap;
	gdk_draw_rectangle (px, w->style->bg_gc[GTK_STATE_NORMAL], TRUE, 0, 0, PX_WIDTH, PX_HEIGHT);
	e_font_draw_utf8_text (px, efont, E_FONT_PLAIN, w->style->text_gc[GTK_STATE_NORMAL], 4, 20, "Test text", 9);
	gtk_widget_queue_draw (w);
	e_font_unref (efont);

	g_free (name);
}

int main (int argc, char ** argv)
{
	GtkWidget * window, * t, * w;
	GdkPixmap * px;

	gnome_init ("TestEFont", "TestEFont", argc, argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Test EFont");
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (window_delete), NULL);

	t = gtk_table_new (2, 5, FALSE);
	gtk_container_add (GTK_CONTAINER (window), t);
	gtk_widget_show (t);

	w = gtk_label_new ("Choose font");
	gtk_table_attach_defaults (GTK_TABLE (t), w, 0, 1, 0, 1);
	gtk_widget_show (w);

	w = gnome_font_picker_new ();
	gnome_font_picker_set_mode (GNOME_FONT_PICKER (w), GNOME_FONT_PICKER_MODE_FONT_INFO);
	gtk_signal_connect (GTK_OBJECT (w), "font_set",
			    GTK_SIGNAL_FUNC (font_set), window);
	gtk_table_attach_defaults (GTK_TABLE (t), w, 1, 2, 0, 1);
	gtk_widget_show (w);

	w = gtk_label_new ("Font name:");
	gtk_table_attach_defaults (GTK_TABLE (t), w, 0, 1, 1, 2);
	gtk_widget_show (w);

	w = gtk_label_new ("---unset---");
	gtk_table_attach_defaults (GTK_TABLE (t), w, 1, 2, 1, 2);
	gtk_widget_show (w);
	gtk_object_set_data (GTK_OBJECT (window), "label", w);

	w = gtk_label_new ("EFont from GdkFont:");
	gtk_table_attach_defaults (GTK_TABLE (t), w, 0, 1, 2, 3);
	gtk_widget_show (w);

	px = gdk_pixmap_new (NULL, PX_WIDTH, PX_HEIGHT, gdk_visual_get_best_depth ());
	w = gtk_pixmap_new (px, NULL);
	gtk_table_attach_defaults (GTK_TABLE (t), w, 1, 2, 2, 3);
	gtk_widget_show (w);
	gtk_object_set_data (GTK_OBJECT (window), "px1", w);

	w = gtk_label_new ("EFont from full name:");
	gtk_table_attach_defaults (GTK_TABLE (t), w, 0, 1, 3, 4);
	gtk_widget_show (w);

	px = gdk_pixmap_new (NULL, PX_WIDTH, PX_HEIGHT, gdk_visual_get_best_depth ());
	w = gtk_pixmap_new (px, NULL);
	gtk_table_attach_defaults (GTK_TABLE (t), w, 1, 2, 3, 4);
	gtk_widget_show (w);
	gtk_object_set_data (GTK_OBJECT (window), "px2", w);

	w = gtk_label_new ("EFont from short name:");
	gtk_table_attach_defaults (GTK_TABLE (t), w, 0, 1, 4, 5);
	gtk_widget_show (w);

	px = gdk_pixmap_new (NULL, PX_WIDTH, PX_HEIGHT, gdk_visual_get_best_depth ());
	w = gtk_pixmap_new (px, NULL);
	gtk_table_attach_defaults (GTK_TABLE (t), w, 1, 2, 4, 5);
	gtk_widget_show (w);
	gtk_object_set_data (GTK_OBJECT (window), "px3", w);

	gtk_widget_show (window);

	gtk_main ();

	return 0;
}
