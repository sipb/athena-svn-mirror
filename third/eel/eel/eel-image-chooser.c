/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-image-chooser.c - A widget to choose an image from a list.

   Copyright (C) 2001 Eazel, Inc.
   Copyright (C) 2002 Anders Carlsson
   
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

   Authors: Anders Carlsson <andersca@gnu.org>

   Based on a version by Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>
#include "eel-image-chooser.h"

#include "eel-gtk-extensions.h"
#include <gtk/gtkcellrendererpixbuf.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreeview.h>
#include <libgnome/gnome-macros.h>

#define IMAGE_CHOOSER_INVALID_INDEX -1
#define IMAGE_CHOOSER_ROW_BORDER 4

struct EelImageChooserDetails {
	GtkListStore *list_store;

	GtkTreeRowReference *selected_row;

	int maximum_height;
};

enum {
	TITLE_COLUMN,
	DESCRIPTION_COLUMN,
	PIXBUF_COLUMN,
	ROW_DATA_COLUMN,
	ROW_DATA_FREE_FUNC_COLUMN,
	NUM_COLUMNS
};

/* Signals */
enum
{
	SELECTION_CHANGED,
	LAST_SIGNAL
};

static guint image_chooser_signals[LAST_SIGNAL];

GNOME_CLASS_BOILERPLATE (EelImageChooser, eel_image_chooser,
			 GtkTreeView, GTK_TYPE_TREE_VIEW)

static void
eel_image_chooser_row_activated (GtkTreeSelection *selection,
				 GtkTreePath *path,
				 GtkTreeViewColumn *column,
				 EelImageChooser *image_chooser)
{
	if (image_chooser->details->selected_row) {
		gtk_tree_row_reference_free (image_chooser->details->selected_row);
	}
	
	image_chooser->details->selected_row = 
		gtk_tree_row_reference_new (GTK_TREE_MODEL (image_chooser->details->list_store), path);

	g_signal_emit (image_chooser, 
		       image_chooser_signals[SELECTION_CHANGED], 
		       0);
}

static void
eel_image_chooser_cell_data_func (GtkTreeViewColumn *tree_column,
				  GtkCellRenderer *cell,
				  GtkTreeModel *tree_model,
				  GtkTreeIter *iter,
				  gpointer user_data)
{
	gchar *title, *description;
	gchar *markup;
	
	gtk_tree_model_get (tree_model, iter,
			    TITLE_COLUMN, &title,
			    DESCRIPTION_COLUMN, &description,
			    -1);
	
	markup = g_strdup_printf ("<b>%s</b>\n%s", title, description);
	g_free (title);
	g_free (description);
	
	g_object_set (G_OBJECT (cell), "markup", markup,
		      NULL);

	g_free (markup);	
}

static void
eel_image_chooser_finalize (GObject *object)
{
	EelImageChooser *image_chooser;
	
	image_chooser = EEL_IMAGE_CHOOSER (object);

	if (image_chooser->details->selected_row) {
		gtk_tree_row_reference_free (image_chooser->details->selected_row);
	}

	g_object_unref (image_chooser->details->list_store);

	g_free (image_chooser->details);
	
	G_OBJECT_CLASS (parent_class)-> finalize (object);
}

/* GtkObjectClass methods */
static void
eel_image_chooser_destroy (GtkObject *object)
{
	EelImageChooser *image_chooser;
	
	image_chooser = EEL_IMAGE_CHOOSER (object);

	eel_image_chooser_clear (image_chooser);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/*
 * EelImageChooserClass methods
 */
static void
eel_image_chooser_class_init (EelImageChooserClass *image_chooser_class)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;

	gobject_class = (GObjectClass *)image_chooser_class;
	object_class = (GtkObjectClass *)image_chooser_class;

	gobject_class->finalize = eel_image_chooser_finalize;
	
	object_class->destroy = eel_image_chooser_destroy;
	
	image_chooser_signals[SELECTION_CHANGED] =
		g_signal_new ("selection_changed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EelImageChooserClass, selection_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
}

static void
eel_image_chooser_instance_init (EelImageChooser *image_chooser)
{
	GtkCellRenderer *text_renderer, *pixbuf_renderer;
	GtkTreeViewColumn *column;
	
	image_chooser->details = g_new0 (EelImageChooserDetails, 1);
	
	image_chooser->details->list_store = gtk_list_store_new (NUM_COLUMNS,
								 G_TYPE_STRING,
								 G_TYPE_STRING,
								 GDK_TYPE_PIXBUF,
								 G_TYPE_POINTER,
								 G_TYPE_POINTER);
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (image_chooser), GTK_TREE_MODEL (image_chooser->details->list_store));

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (image_chooser), FALSE);
	
	/* Append the tree column */
	column = gtk_tree_view_column_new ();

	pixbuf_renderer = gtk_cell_renderer_pixbuf_new ();

	gtk_tree_view_column_pack_start (column,
					 pixbuf_renderer,
					 FALSE);
	gtk_tree_view_column_add_attribute (column,
				     pixbuf_renderer,
				     "pixbuf", PIXBUF_COLUMN);
	
	text_renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column,
					 text_renderer,
					 TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
						 text_renderer,
						 eel_image_chooser_cell_data_func,
						 NULL, NULL);
	
	gtk_tree_view_append_column (GTK_TREE_VIEW (image_chooser), column);

	eel_gtk_tree_view_set_activate_on_single_click 
		(GTK_TREE_VIEW (image_chooser), TRUE);

	g_signal_connect (GTK_TREE_VIEW (image_chooser), 
			  "row_activated",
			  G_CALLBACK (eel_image_chooser_row_activated),
			  image_chooser);
}

GtkWidget *
eel_image_chooser_new (void)
{
	return g_object_new (EEL_TYPE_IMAGE_CHOOSER, NULL);
}

void
eel_image_chooser_insert_row (EelImageChooser *image_chooser,
			     GdkPixbuf *pixbuf,
			     const char *title,
			     const char *description,
			     gpointer row_data,
			     GFreeFunc row_data_free_func)
{
	GtkTreeIter iter;
	int pixbuf_height;
	
	gtk_list_store_append (image_chooser->details->list_store,
			       &iter);
	gtk_list_store_set (image_chooser->details->list_store,
			    &iter,
			    TITLE_COLUMN, title,
			    DESCRIPTION_COLUMN, description,
			    PIXBUF_COLUMN, pixbuf,
			    ROW_DATA_COLUMN, row_data,
			    ROW_DATA_FREE_FUNC_COLUMN, row_data_free_func,
			    -1);

	pixbuf_height = gdk_pixbuf_get_height (pixbuf);

	if (pixbuf_height > image_chooser->details->maximum_height)
		image_chooser->details->maximum_height = pixbuf_height;
}

static GtkTreePath *
eel_image_chooser_get_selected_path (const EelImageChooser *image_chooser)
{
	if (image_chooser->details->selected_row) {
		return gtk_tree_row_reference_get_path (image_chooser->details->selected_row);
	} else {
		return NULL;
	}
}

int
eel_image_chooser_get_selected_row (const EelImageChooser *image_chooser)
{
	GtkTreePath *path;
	int row;
	
	path = eel_image_chooser_get_selected_path (image_chooser);
	
	if (path == NULL) {
		return IMAGE_CHOOSER_INVALID_INDEX;
	}
	
	row = gtk_tree_path_get_indices (path)[0];
	gtk_tree_path_free (path);
	
	return row;
}

gpointer
eel_image_chooser_get_row_data (const EelImageChooser *image_chooser,
			       guint row_index)
{
	GtkTreeIter iter;
	gpointer data;

	if (eel_image_chooser_get_num_rows (image_chooser) == 0) {
		return NULL;
	}

	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (image_chooser->details->list_store),
				       &iter, NULL, row_index);
	
	gtk_tree_model_get (GTK_TREE_MODEL (image_chooser->details->list_store), &iter,
			    ROW_DATA_COLUMN, &data,
			    -1);

	return data;
}

void
eel_image_chooser_set_selected_row (EelImageChooser *image_chooser,
				    int icon_position)
{
	GtkTreePath *path;

	if (image_chooser->details->selected_row) {
		gtk_tree_row_reference_free (image_chooser->details->selected_row);
		image_chooser->details->selected_row = NULL;
	}

	if (icon_position < 0) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (image_chooser)));
		return;
	}

	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path, icon_position);
	gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (image_chooser)), path);

	image_chooser->details->selected_row = gtk_tree_row_reference_new
		(gtk_tree_view_get_model (GTK_TREE_VIEW (image_chooser)),
		 path);

	gtk_tree_path_free (path);
}

guint
eel_image_chooser_get_num_rows (const EelImageChooser *image_chooser)
{
	return gtk_tree_model_iter_n_children (GTK_TREE_MODEL (image_chooser->details->list_store),
					       NULL);
}

static gboolean
eel_image_chooser_foreach_destroy_func (GtkTreeModel *tree_model,
					GtkTreePath *path,
					GtkTreeIter *iter,
					gpointer data)
{
	gpointer row_data;
	GFreeFunc free_func;
	
	gtk_tree_model_get (tree_model, iter,
			    ROW_DATA_FREE_FUNC_COLUMN, &free_func,
			    ROW_DATA_COLUMN, &row_data,
			    -1);

	if (free_func) {
		(* free_func) (row_data);
	}

	return FALSE;
}

void
eel_image_chooser_clear (EelImageChooser *image_chooser)
{
	gtk_tree_model_foreach (GTK_TREE_MODEL (image_chooser->details->list_store),
				eel_image_chooser_foreach_destroy_func, NULL);
	
	gtk_list_store_clear (image_chooser->details->list_store);
}

GtkWidget *
eel_scrolled_image_chooser_new (GtkWidget **image_chooser_out)
{
	GtkWidget *scrolled_window;
	GtkWidget *image_chooser;
	
	g_return_val_if_fail (image_chooser_out != NULL, NULL);
	
 	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
 					GTK_POLICY_NEVER,
 					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
					     GTK_SHADOW_IN);
	
	image_chooser = eel_image_chooser_new ();
	gtk_widget_show (image_chooser);
	
 	gtk_container_add (GTK_CONTAINER (scrolled_window), image_chooser);
	
	*image_chooser_out = image_chooser;

	return scrolled_window;
}

void
eel_scrolled_image_chooser_set_num_visible_rows (EelImageChooser *image_chooser,
						 GtkWidget *scrolled_window,
						 guint num_visible_rows)
{
	int maximum_height;
	
	if (eel_image_chooser_get_num_rows (image_chooser) == 0) {
		return;
	}

	maximum_height = image_chooser->details->maximum_height;
	
	gtk_widget_set_size_request (scrolled_window,
				     -1, maximum_height * num_visible_rows);
}

void
eel_scrolled_image_chooser_show_selected_row (EelImageChooser *image_chooser,
					      GtkWidget *scrolled_window)
{
	GtkTreePath *path;

	path = eel_image_chooser_get_selected_path (image_chooser);

	if (path == NULL) {
		return;
	}

	if (!eel_gtk_tree_view_cell_is_completely_visible (GTK_TREE_VIEW (image_chooser), path, NULL)) {
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (image_chooser), path, NULL, TRUE, 0.5, 0.0);
	}

	gtk_tree_path_free (path);
}
