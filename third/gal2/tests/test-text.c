/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * test-text-1.c
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Chris Toshok <toshok@ximian.com>
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

#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libgnomeui/gnome-ui-init.h>
#include "gal/widgets/e-cursors.h"
#include "gal/e-text/e-text.h"
#include "gal/e-text/e-text-model.h"
#include "gal/e-text/e-text-model-uri.h"
#include "gal/e-text/e-entry.h"
#include "gal/widgets/e-canvas-utils.h"
#include "gal/util/e-i18n.h"

static void
weak_ref_func (gpointer data, GObject *where_object_was)
{
	gtk_main_quit();
}

/* We create a window containing our new entry. */
static void
create_entry (void)
{
	GtkWidget *entry, *window, *frame, *button, *vbox;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "EEntry Test");

	frame = gtk_frame_new (NULL);

	entry = e_entry_new ();

	g_object_set (entry,
		      "use_ellipsis", TRUE,
		      NULL);

	button = gtk_button_new_with_label ("EEntry test");

	/* Build the gtk widget hierarchy. */
	vbox = gtk_vbox_new (FALSE, 2);

	gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_container_add (GTK_CONTAINER (window), frame);

	/* Show it all. */
	gtk_widget_show_all (window);

	g_object_weak_ref (G_OBJECT (window), weak_ref_func, NULL);
}

static void
canvas_size_allocate (GtkWidget *widget, GtkAllocation *alloc,
		      EText *etext)
{
	gnome_canvas_set_scroll_region (GNOME_CANVAS (widget),
					0, 0, alloc->width, alloc->height);
	g_object_set (etext,
		      "width", (double) (alloc->width),
		      "clip_width", (double) (alloc->width),
		      "clip_height", (double) (alloc->height),
		      NULL);

	e_canvas_item_move_absolute(GNOME_CANVAS_ITEM(etext),
				    0, 0);
}

static void
create_text (void)
{
	EText *etext;
	GtkWidget *canvas, *window, *frame;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "EText Test");

	frame = gtk_frame_new (NULL);

	canvas = e_canvas_new ();

	etext = E_TEXT(gnome_canvas_item_new(
		gnome_canvas_root (GNOME_CANVAS (canvas)),
		e_text_get_type(),
		"model", e_text_model_uri_new(),
		"clip", TRUE,
		"fill_clip_rectangle", TRUE,
		"anchor", GTK_ANCHOR_NW,
		"draw_borders", TRUE,
		"draw_background", TRUE,
		"draw_button", FALSE,
		"editable", TRUE,
		"allow_newlines", TRUE,
		"line_wrap", TRUE,
		NULL));

	g_signal_connect (canvas,
			  "size_allocate",
			  G_CALLBACK (canvas_size_allocate),
			  etext);

	/* Build the gtk widget hierarchy. */
	gtk_container_add (GTK_CONTAINER (window), canvas);

	/* Show it all. */
	gtk_widget_show_all (window);

	g_object_weak_ref (G_OBJECT (window), weak_ref_func, NULL);
}

static void
create_ro_text (void)
{
	EText *etext;
	GtkWidget *canvas, *window, *frame;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "EText Readonly Test");

	frame = gtk_frame_new (NULL);

	canvas = e_canvas_new ();

	etext = E_TEXT(gnome_canvas_item_new(
		gnome_canvas_root (GNOME_CANVAS (canvas)),
		e_text_get_type(),
		"clip", TRUE,
		"fill_clip_rectangle", TRUE,
		"anchor", GTK_ANCHOR_NW,
		"draw_borders", TRUE,
		"draw_background", TRUE,
		"draw_button", FALSE,
		"editable", FALSE,
		"allow_newlines", TRUE,
		"line_wrap", TRUE,
		NULL));

	g_signal_connect (canvas,
			  "size_allocate",
			  G_CALLBACK (canvas_size_allocate),
			  etext);

	/* Build the gtk widget hierarchy. */
	gtk_container_add (GTK_CONTAINER (window), canvas);

	/* Show it all. */
	gtk_widget_show_all (window);

	g_object_weak_ref (G_OBJECT (window), weak_ref_func, NULL);
}

int
main (int argc, char *argv [])
{
	gnome_program_init ("TextExample", "1.0",
			    LIBGNOMEUI_MODULE,
			    argc, argv, NULL);

	e_cursors_init ();

	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	create_entry ();

	create_text ();

	create_ro_text ();
	
	gtk_main ();

	e_cursors_shutdown ();
	return 0;
}
