/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Copyright (C) 2001, 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "gconf-bookmarks-dialog.h"
#include <gconf/gconf-client.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcellrendererpixbuf.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeviewcolumn.h>
#include <gtk/gtkvbox.h>
#include <libintl.h>

#include "gconf-stock-icons.h"

#define _(x) gettext (x)
#define N_(x) (x)

#define BOOKMARKS_KEY "/apps/gconf-editor/bookmarks"

static GtkDialogClass *parent_class;

static void
gconf_bookmarks_dialog_response (GtkDialog *dialog, gint response_id)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
gconf_bookmarks_dialog_destroy (GtkObject *object)
{
	GConfClient *client;
	GConfBookmarksDialog *dialog;
	
	client = gconf_client_get_default ();
	dialog = GCONF_BOOKMARKS_DIALOG (object);
	
	if (dialog->notify_id != 0) {
		gconf_client_notify_remove (client, dialog->notify_id);
		gconf_client_remove_dir (client, BOOKMARKS_KEY, NULL);
		dialog->notify_id = 0;
	}
	
	if (GTK_OBJECT_CLASS (parent_class)->destroy) {
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
	}
}

static void
gconf_bookmarks_dialog_class_init (GConfBookmarksDialogClass *klass)
{
	GtkDialogClass *dialog_class;
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *)klass;
	dialog_class = (GtkDialogClass *)klass;
	
	parent_class = g_type_class_peek_parent (klass);

	object_class->destroy = gconf_bookmarks_dialog_destroy;
	dialog_class->response = gconf_bookmarks_dialog_response;
}

static void
gconf_bookmarks_dialog_populate_model (GConfBookmarksDialog *dialog)
{
	GConfClient *client;
	GSList *value_list, *p;
	GtkTreeIter iter;
	
	client = gconf_client_get_default ();
	
	/* First clear the list store */
	dialog->changing_model = TRUE;
	gtk_list_store_clear (dialog->list_store);

	value_list = gconf_client_get_list (client, BOOKMARKS_KEY,
					    GCONF_VALUE_STRING, NULL);

	for (p = value_list; p; p = p->next) {
		gtk_list_store_append (dialog->list_store, &iter);
		gtk_list_store_set (dialog->list_store, &iter,
				    0, p->data,
				    -1);
	}
	dialog->changing_model = FALSE;
}

static void
gconf_bookmarks_dialog_selection_changed (GtkTreeSelection *selection, GConfBookmarksDialog *dialog)
{
	gtk_widget_set_sensitive (dialog->delete_button,
				  gtk_tree_selection_get_selected (selection, NULL, NULL));

}

static void
gconf_bookmarks_dialog_update_gconf_key (GConfBookmarksDialog *dialog)
{
	GSList *list;
	GtkTreeIter iter;
	char *bookmark;
	GConfClient *client;
	
	list = NULL;

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (dialog->list_store), &iter)) {
		do {
			gtk_tree_model_get (GTK_TREE_MODEL (dialog->list_store), &iter,
					    0, &bookmark,
					    -1);
			list = g_slist_append (list, bookmark);
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (dialog->list_store), &iter));
	}

	client = gconf_client_get_default ();

	dialog->changing_key = TRUE;
	gconf_client_set_list (client, BOOKMARKS_KEY,
			       GCONF_VALUE_STRING, list, NULL);
}

static void
gconf_bookmarks_dialog_row_deleted (GtkTreeModel *tree_model, GtkTreePath *path,
				    GConfBookmarksDialog *dialog)
{
	if (dialog->changing_model) {
		return;
	}

	gconf_bookmarks_dialog_update_gconf_key (dialog);

}

static void
gconf_bookmarks_dialog_delete_bookmark (GtkWidget *button, GConfBookmarksDialog *dialog)
{
	GtkTreeIter iter;
	
	gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tree_view)),
					 NULL, &iter);

	dialog->changing_model = TRUE;
	gtk_list_store_remove (dialog->list_store, &iter);
	dialog->changing_model = FALSE;

	gconf_bookmarks_dialog_update_gconf_key (dialog);
}

void
gconf_bookmarks_dialog_bookmarks_key_changed (GConfClient* client,
					      guint cnxn_id,
					      GConfEntry *entry,
					      gpointer user_data)
{
	GConfBookmarksDialog *dialog;

	dialog = user_data;

	if (dialog->changing_key) {
		dialog->changing_key = FALSE;
		return;
	}
	
	gconf_bookmarks_dialog_populate_model (dialog);	
}

static void
gconf_bookmarks_dialog_init (GConfBookmarksDialog *dialog)
{
	GtkWidget *scrolled_window, *hbox, *vbox;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	GdkPixbuf *pixbuf;
	GConfClient *client;
       
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 2);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 200);
	
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Edit Bookmarks"));

	dialog->list_store = gtk_list_store_new (1, G_TYPE_STRING);
	g_signal_connect (dialog->list_store, "row_deleted",
			  G_CALLBACK (gconf_bookmarks_dialog_row_deleted), dialog);
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrolled_window, TRUE, TRUE, 0);
	
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	dialog->tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (dialog->list_store));
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW (dialog->tree_view), TRUE);
	
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->tree_view)), "changed",
			  G_CALLBACK (gconf_bookmarks_dialog_selection_changed), dialog);
	gconf_bookmarks_dialog_populate_model (dialog);

	column = gtk_tree_view_column_new ();

	cell = gtk_cell_renderer_pixbuf_new ();
	pixbuf = gtk_widget_render_icon (dialog->tree_view, GCONF_STOCK_BOOKMARK, GTK_ICON_SIZE_MENU, NULL);
	g_object_set (G_OBJECT (cell),
		      "pixbuf", pixbuf,
		      NULL);
	g_object_unref (pixbuf);
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", 0,
					     NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (dialog->tree_view), column);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (dialog->tree_view), FALSE);
	
	g_object_unref (dialog->list_store);
	gtk_container_add (GTK_CONTAINER (scrolled_window), dialog->tree_view);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
	dialog->delete_button = gtk_button_new_from_stock (GTK_STOCK_DELETE);
	gtk_widget_set_sensitive (dialog->delete_button, FALSE);
	g_signal_connect (dialog->delete_button, "clicked",
			  G_CALLBACK (gconf_bookmarks_dialog_delete_bookmark), dialog);
	
	gtk_box_pack_start (GTK_BOX (vbox), dialog->delete_button, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show_all (hbox);

	/* Listen for gconf changes */
	client = gconf_client_get_default ();
	gconf_client_add_dir (client, BOOKMARKS_KEY, GCONF_CLIENT_PRELOAD_NONE, NULL);

	dialog->notify_id = gconf_client_notify_add (client, BOOKMARKS_KEY,
						     gconf_bookmarks_dialog_bookmarks_key_changed,
						     dialog,
						     NULL,
						     NULL);
}

GType
gconf_bookmarks_dialog_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (GConfBookmarksDialogClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) gconf_bookmarks_dialog_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (GConfBookmarksDialog),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gconf_bookmarks_dialog_init
		};

		object_type = g_type_register_static (GTK_TYPE_DIALOG, "GConfBookmarksDialog", &object_info, 0);
	}

	return object_type;
}

GtkWidget *
gconf_bookmarks_dialog_new (GtkWindow *parent)
{
	GtkWidget *dialog;

	dialog = g_object_new (GCONF_TYPE_BOOKMARKS_DIALOG, NULL);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

	return dialog;
}
