/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
   eel-mime-application-chooser.c: an mime-application chooser
 
   Copyright (C) 2004 Novell, Inc.
 
   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but APPLICATIONOUT ANY WARRANTY; applicationout even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along application the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Dave Camp <dave@novell.com>
*/

#include <config.h>
#include "eel-mime-application-chooser.h"

#include "eel-mime-extensions.h"
#include "eel-open-with-dialog.h"

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtkalignment.h> 
#include <gtk/gtkbox.h> 
#include <gtk/gtkbutton.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrenderertoggle.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkhbbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkvbox.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h> 
#include <libgnomevfs/gnome-vfs-mime-monitor.h> 
#include <libgnomevfs/gnome-vfs-uri.h> 

struct _EelMimeApplicationChooserDetails {
	char *uri;

	char *mime_type;
	char *real_mime_type;
	char *mime_description;
	
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *treeview;
	GtkWidget *remove_button;

	GtkListStore *model;
	GtkCellRenderer *toggle_renderer;
};

enum {
	COLUMN_DEFAULT,
	COLUMN_NAME,
	COLUMN_ID,
	NUM_COLUMNS
};

static void refresh_model (EelMimeApplicationChooser *chooser);

static gpointer parent_class;

static void
eel_mime_application_chooser_finalize (GObject *object)
{
	EelMimeApplicationChooser *chooser;

	chooser = EEL_MIME_APPLICATION_CHOOSER (object);

	g_free (chooser->details->uri);
	g_free (chooser->details->mime_type);
	g_free (chooser->details->mime_description);
	
	g_free (chooser->details);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
eel_mime_application_chooser_destroy (GtkObject *object)
{
	EelMimeApplicationChooser *chooser;

	chooser = EEL_MIME_APPLICATION_CHOOSER (object);
	
	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
eel_mime_application_chooser_class_init (EelMimeApplicationChooserClass *class)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);

	gobject_class = G_OBJECT_CLASS (class);
	gobject_class->finalize = eel_mime_application_chooser_finalize;
	
	object_class = GTK_OBJECT_CLASS (class);
	object_class->destroy = eel_mime_application_chooser_destroy;
}

static void
default_toggled_cb (GtkCellRendererToggle *renderer,
		    const char *path_str,
		    gpointer user_data)
{
	EelMimeApplicationChooser *chooser;
	GtkTreeIter iter;
	GtkTreePath *path;
	
	chooser = EEL_MIME_APPLICATION_CHOOSER (user_data);
	
	path = gtk_tree_path_new_from_string (path_str);
	if (gtk_tree_model_get_iter (GTK_TREE_MODEL (chooser->details->model),
				     &iter, path)) {
		gboolean is_default;
		char *id;
		
		gtk_tree_model_get (GTK_TREE_MODEL (chooser->details->model),
				    &iter,
				    COLUMN_DEFAULT, &is_default,
				    COLUMN_ID, &id,
				    -1);
		
		if (!is_default) {
			eel_mime_set_default_application (chooser->details->mime_type,
							  id);
			refresh_model (chooser);
		}
		g_free (id);
	}
	gtk_tree_path_free (path);
}

static char *
get_selected_application (EelMimeApplicationChooser *chooser)
{
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	char *id;
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chooser->details->treeview));

	id = NULL;
	if (gtk_tree_selection_get_selected (selection, 
					     NULL,
					     &iter)) {
		gtk_tree_model_get (GTK_TREE_MODEL (chooser->details->model),
				    &iter,
				    COLUMN_ID, &id,
				    -1);
	}
	
	return id;
}

static void
selection_changed_cb (GtkTreeSelection *selection, 
		      gpointer user_data)
{
	EelMimeApplicationChooser *chooser;
	char *id;
	
	chooser = EEL_MIME_APPLICATION_CHOOSER (user_data);
	
	id = get_selected_application (chooser);
	if (id) {
		gtk_widget_set_sensitive (chooser->details->remove_button,
					  eel_mime_application_is_user_owned (id));
		
		g_free (id);
	} else {
		gtk_widget_set_sensitive (chooser->details->remove_button,
					  FALSE);
	}
}

static GtkWidget *
create_tree_view (EelMimeApplicationChooser *chooser)
{
	GtkWidget *treeview;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;
	
	treeview = gtk_tree_view_new ();
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
	
	store = gtk_list_store_new (NUM_COLUMNS,
				    G_TYPE_BOOLEAN,
				    G_TYPE_STRING,
				    G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview),
				 GTK_TREE_MODEL (store));
	chooser->details->model = store;
	
	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (renderer, "toggled", 
			  G_CALLBACK (default_toggled_cb), 
			  chooser);
	gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE (renderer),
					    TRUE);
	
	column = gtk_tree_view_column_new_with_attributes (_("Default"),
							   renderer,
							   "active",
							   COLUMN_DEFAULT,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	chooser->details->toggle_renderer = renderer;
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Name"),
							   renderer,
							   "markup",
							   COLUMN_NAME,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (selection, "changed", 
			  G_CALLBACK (selection_changed_cb), 
			  chooser);	

	return treeview;
}

static void
add_clicked_cb (GtkButton *button,
		gpointer user_data)
{
	EelMimeApplicationChooser *chooser;
	GtkWidget *dialog;
	
	chooser = EEL_MIME_APPLICATION_CHOOSER (user_data);
	
	dialog = eel_add_application_dialog_new (chooser->details->uri,
						 chooser->details->real_mime_type);
	gtk_window_set_screen (GTK_WINDOW (dialog),
			       gtk_widget_get_screen (GTK_WIDGET (chooser)));
	gtk_widget_show (dialog);
}

static void
remove_clicked_cb (GtkButton *button, 
		   gpointer user_data)
{
	const char *id;
	EelMimeApplicationChooser *chooser;
	
	chooser = EEL_MIME_APPLICATION_CHOOSER (user_data);
	
	id = get_selected_application (chooser);

	if (id) {
		eel_mime_application_remove (id);
		refresh_model (chooser);
	}
}

static void
mime_monitor_data_changed_cb (GnomeVFSMIMEMonitor *monitor,
			      gpointer user_data)
{
	EelMimeApplicationChooser *chooser;

	chooser = EEL_MIME_APPLICATION_CHOOSER (user_data);

	refresh_model (chooser);
}

static void
eel_mime_application_chooser_instance_init (EelMimeApplicationChooser *chooser)
{
	GtkWidget *box;
	GtkWidget *scrolled;
	GtkWidget *button;
	
	chooser->details = g_new0 (EelMimeApplicationChooserDetails, 1);

	gtk_container_set_border_width (GTK_CONTAINER (chooser), 8);
	gtk_box_set_spacing (GTK_BOX (chooser), 0);
	gtk_box_set_homogeneous (GTK_BOX (chooser), FALSE);

	chooser->details->label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (chooser->details->label), 0.0, 0.5);
	gtk_label_set_line_wrap (GTK_LABEL (chooser->details->label), TRUE);
	gtk_box_pack_start (GTK_BOX (chooser), chooser->details->label, 
			    FALSE, FALSE, 0);

	gtk_widget_show (chooser->details->label);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
					     GTK_SHADOW_IN);
	
	gtk_widget_show (scrolled);
	gtk_box_pack_start (GTK_BOX (chooser), scrolled, TRUE, TRUE, 6);

	chooser->details->treeview = create_tree_view (chooser);
	gtk_widget_show (chooser->details->treeview);
	
	gtk_container_add (GTK_CONTAINER (scrolled), 
			   chooser->details->treeview);

	box = gtk_hbutton_box_new ();
	gtk_box_set_spacing (GTK_BOX (box), 6);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (box), GTK_BUTTONBOX_END);
	gtk_box_pack_start (GTK_BOX (chooser), box, FALSE, FALSE, 6);
	gtk_widget_show (box);

	button = gtk_button_new_from_stock (GTK_STOCK_ADD);
	g_signal_connect (button, "clicked", 
			  G_CALLBACK (add_clicked_cb),
			  chooser);

	gtk_widget_show (button);
	gtk_container_add (GTK_CONTAINER (box), button);

	button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
	g_signal_connect (button, "clicked", 
			  G_CALLBACK (remove_clicked_cb),
			  chooser);
	
	gtk_widget_show (button);
	gtk_container_add (GTK_CONTAINER (box), button);
	
	chooser->details->remove_button = button;

	g_signal_connect_object (gnome_vfs_mime_monitor_get (),
				 "data_changed",
				 G_CALLBACK (mime_monitor_data_changed_cb),
				 chooser,
				 0);
}

static char *
get_extension (const char *basename)
{
	char *p;
	
	p = strrchr (basename, '.');
	
	if (p && *(p + 1) != '\0') {
		return g_strdup (p + 1);
	} else {
		return NULL;
	}
}

static void
refresh_model (EelMimeApplicationChooser *chooser)
{
	GList *applications;
	GnomeVFSMimeApplication *default_app;
	GList *l;
	GtkTreeSelection *selection;
	
	gtk_list_store_clear (chooser->details->model);

	applications = gnome_vfs_mime_get_all_applications (chooser->details->mime_type);
	
	default_app = gnome_vfs_mime_get_default_application (chooser->details->mime_type);

	for (l = applications; l != NULL; l = l->next) {
		GtkTreeIter iter;
		gboolean is_default;
		GnomeVFSMimeApplication *application;
		char *escaped;

		application = l->data;
		
		is_default = default_app && !strcmp (default_app->id, application->id);

		escaped = g_markup_escape_text (application->name, -1);

		gtk_list_store_append (chooser->details->model, &iter);
		gtk_list_store_set (chooser->details->model, &iter,
				    COLUMN_DEFAULT, is_default,
				    COLUMN_NAME, escaped,
				    COLUMN_ID, application->id, 
				    -1);
		g_free (escaped);
	}

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (chooser->details->treeview));
	
	if (applications) {
		g_object_set (chooser->details->toggle_renderer,
			      "visible", TRUE, 
			      NULL);
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	} else {
		GtkTreeIter iter;

		g_object_set (chooser->details->toggle_renderer,
			      "visible", FALSE,
			      NULL);
		gtk_list_store_append (chooser->details->model, &iter);
		gtk_list_store_set (chooser->details->model, &iter,
				    COLUMN_NAME, _("<i>No applications selected</i>"),
				    COLUMN_ID, NULL,
				    -1);

		gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);
	}
	
	if (default_app) {
		gnome_vfs_mime_application_free (default_app);
	}
	
	gnome_vfs_mime_application_list_free (applications);
}

static gboolean
set_uri_and_mime_type (EelMimeApplicationChooser *chooser, 
		       const char *uri,
		       const char *mime_type)
{
	char *label;
	char *name;
	GnomeVFSURI *vfs_uri;
	
	chooser->details->uri = g_strdup (uri);
	
	vfs_uri = gnome_vfs_uri_new (uri);

	name = gnome_vfs_uri_extract_short_name (vfs_uri);

	chooser->details->real_mime_type = g_strdup (mime_type);
	if (!strcmp (mime_type, "application/octet-stream")) {
		char *extension;
		
		extension = get_extension (uri);
		
		if (!extension) {
			g_warning ("No extension, not implemented yet");
			return FALSE;
		}

		chooser->details->mime_type = 
			g_strdup_printf ("application/x-extension-%s", 
					 extension);
		chooser->details->mime_description = 
			g_strdup_printf (_("%s document"), extension);
		
		g_free (extension);
	} else {
		char *description;
		
		chooser->details->mime_type = g_strdup (mime_type);
		description = g_strdup (gnome_vfs_mime_get_description (mime_type));
		
		if (description == NULL) {
			description = g_strdup (_("Unknown"));
		}

		chooser->details->mime_description = description;
	}

	label = g_strdup_printf (_("Select an application to open <i>%s</i> and others of type \"%s\""), name, chooser->details->mime_description);
	
	gtk_label_set_markup (GTK_LABEL (chooser->details->label), label);

	g_free (label);
	g_free (name);
	gnome_vfs_uri_unref (vfs_uri);

	refresh_model (chooser);

	return TRUE;
}

GtkWidget *
eel_mime_application_chooser_new (const char *uri,
			  const char *mime_type)
{
	GtkWidget *chooser;

	chooser = gtk_widget_new (EEL_TYPE_MIME_APPLICATION_CHOOSER, NULL);

	set_uri_and_mime_type (EEL_MIME_APPLICATION_CHOOSER (chooser), uri, mime_type);

	return chooser;
}

GType
eel_mime_application_chooser_get_type (void)
{
	static GType type = 0;
	
	if (!type) {
		static const GTypeInfo info = {
			sizeof (EelMimeApplicationChooserClass),
			NULL, 
			NULL,
			(GClassInitFunc)eel_mime_application_chooser_class_init,
			NULL,
			NULL,
			sizeof (EelMimeApplicationChooser),
			0,
			(GInstanceInitFunc)eel_mime_application_chooser_instance_init,
		};
		
		type = g_type_register_static (GTK_TYPE_VBOX, 
					       "EelMimeApplicationChooser",
					       &info, 0);
	}
	
	return type;		       
}
