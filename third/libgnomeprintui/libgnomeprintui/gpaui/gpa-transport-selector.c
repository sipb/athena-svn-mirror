/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-transport-selector.c: A print transport selector
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors :
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio <chema@ximian.com>
 *    Andreas J. Guelzow <aguelzow@taliesin.ca>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc. 
 *  Copyright (C) 2004 Andreas J. Guelzow
 *
 */

#include "config.h"

#include <string.h>
#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-transport-selector.h"
#include <libgnomeprint/private/gnome-print-config-private.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/private/gpa-key.h>
#include <libgnomeprint/gnome-print-transport.h>

static void gpa_transport_selector_class_init (GPATransportSelectorClass *klass);
static void gpa_transport_selector_init (GPATransportSelector *selector);
static void gpa_transport_selector_finalize (GObject *object);
static gint gpa_transport_selector_construct (GPAWidget *widget);

static void gpa_transport_selector_file_button_clicked_cb       (GtkButton *button, GPATransportSelector *ts);
static void gpa_transport_selector_custom_entry_changed_cb (GtkEntry *entry, GPATransportSelector *ts);
static void gpa_transport_selector_node_modified_cb (GPANode *node, guint flags, GPATransportSelector *ts);
static gboolean gpa_transport_selector_check_consistency_real (GPATransportSelector *ts);

static GPAWidgetClass *parent_class;

GType
gpa_transport_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPATransportSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gpa_transport_selector_class_init,
			NULL, NULL,
			sizeof (GPATransportSelector),
			0,
			(GInstanceInitFunc) gpa_transport_selector_init,
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPATransportSelector", &info, 0);
	}
	return type;
}

static void
gpa_transport_selector_class_init (GPATransportSelectorClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);

	gpa_class->construct   = gpa_transport_selector_construct;
	object_class->finalize = gpa_transport_selector_finalize;
	klass->check_consistency 
		= gpa_transport_selector_check_consistency_real;
}

static void
gpa_transport_selector_init (GPATransportSelector *ts)
{
	GtkBox *hbox;

	hbox = (GtkBox *) gtk_hbox_new (FALSE, 4);

	ts->combo = gtk_combo_box_new ();
	ts->file_name   = g_strdup ("");
	ts->file_name_force = FALSE;
	ts->file_name_label   = gtk_label_new ("");
	ts->file_button   = gtk_button_new_from_stock (GTK_STOCK_SAVE_AS);
	ts->custom_entry = gtk_entry_new ();

	g_signal_connect (G_OBJECT (ts->file_button), "clicked", (GCallback)
			  gpa_transport_selector_file_button_clicked_cb, ts);
	g_signal_connect (G_OBJECT (ts->custom_entry), "changed", (GCallback)
			  gpa_transport_selector_custom_entry_changed_cb, ts);

	gtk_box_pack_start (hbox, GTK_WIDGET (ts->combo),	FALSE, FALSE, 0);
	gtk_box_pack_start (hbox, ts->file_button,   FALSE, FALSE, 0);
	gtk_box_pack_start (hbox, ts->file_name_label,  FALSE, FALSE, 0);
	gtk_box_pack_start (hbox, ts->custom_entry, FALSE, FALSE, 0);
	
	gtk_container_add (GTK_CONTAINER (ts), GTK_WIDGET (hbox));
	gtk_widget_show_all (GTK_WIDGET (hbox));
}

static void
gpa_transport_selector_disconnect (GPATransportSelector *ts)
{
	if (ts->handler) {
		g_signal_handler_disconnect (ts->node, ts->handler);
		ts->handler = 0;
	}
	
	if (ts->node) {
		gpa_node_unref (ts->node);
		ts->node = NULL;
	}
}

static void
gpa_transport_selector_finalize (GObject *object)
{
	GPATransportSelector *ts;

	ts = (GPATransportSelector *) object;

	if (ts->file_selector)
		gtk_widget_destroy (GTK_WIDGET(ts->file_selector));
	ts->file_selector = NULL;

	gpa_transport_selector_disconnect (ts);

	if (ts->handler_config)
		g_signal_handler_disconnect (ts->config, ts->handler_config);
	ts->handler_config = 0;
	ts->config = NULL;
	g_free (ts->file_name);
	ts->file_name = NULL;
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpa_transport_selector_file_selected_cb (GtkFileChooser *dialog,
					 gint response_id,
					 GPATransportSelector *ts) 
{
	char *selected_filename;
	gchar *filename;
	gsize bytes_read;
	gsize bytes_written;

	if (response_id == GTK_RESPONSE_DELETE_EVENT)
		ts->file_selector = NULL;

	if (response_id != GTK_RESPONSE_OK || 
	    ((selected_filename = gtk_file_chooser_get_filename (dialog)) 
	     == NULL)) {
		gtk_main_quit ();
		return;
	}

	filename = g_filename_to_utf8 (selected_filename, -1,
				       &bytes_read, &bytes_written,NULL);
	
	if (g_file_test (selected_filename, G_FILE_TEST_IS_DIR)) {
		GtkWidget *warning_dialog;
		warning_dialog = gtk_message_dialog_new
			(GTK_WINDOW (ts->file_selector),
			 GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			 GTK_BUTTONS_CLOSE,
			 _("The specified filename \"%s\" is an existing "
			   "directory."), filename);
		
		g_signal_connect_swapped (GTK_OBJECT (warning_dialog), 
					  "response",
					  G_CALLBACK (gtk_widget_destroy),
					  GTK_OBJECT (warning_dialog));
		gtk_widget_show (warning_dialog);
		return;
	}
	
	if (g_file_test (selected_filename, G_FILE_TEST_EXISTS)) {
		GtkWidget *warning_dialog;
		gint response;
		
		warning_dialog = gtk_message_dialog_new
			(GTK_WINDOW (ts->file_selector),
			 GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			 GTK_BUTTONS_YES_NO,
			 _("Should the file %s be overwritten?"), filename);
		
		response = gtk_dialog_run (GTK_DIALOG (warning_dialog));
		
		gtk_widget_destroy (warning_dialog);
		
		if (GTK_RESPONSE_YES != response) {
			ts->file_name_force = FALSE;
			return;
		}
		ts->file_name_force = TRUE;
	} else {
		ts->file_name_force = FALSE;
	}
	
	/* FIXME: One of these should be enough... */
	gpa_node_set_path_value (ts->config,
				 "Settings.Output.Job.FileName",
				 filename);
	gpa_node_set_path_value (ts->config,
				 "Settings.Transport.Backend.FileName",
				 filename);
	gpa_node_set_path_value (ts->node, "FileName", filename);
	g_free (ts->file_name);
	ts->file_name = filename;
	gtk_label_set_text (GTK_LABEL (ts->file_name_label), ts->file_name);
	
	g_free (selected_filename);
	gtk_main_quit ();
}



static void
gpa_transport_selector_file_button_clicked_cb (GtkButton *button, GPATransportSelector *ts)
{
	gchar     *filename;
	gsize bytes_read;
	gsize bytes_written;
	
	/* Create the selector */
   
	ts->file_selector = GTK_FILE_CHOOSER
		(g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG,
			       "action", GTK_FILE_CHOOSER_ACTION_SAVE,
			       "title", _("Please specify the location and filename of the output file:"),
			       NULL));
	gtk_dialog_add_buttons (GTK_DIALOG (ts->file_selector),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_OK,
				NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (ts->file_selector), 
					 GTK_RESPONSE_OK);
	/* Filters */
	{	
		GtkFileFilter *filter;

 
		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, "PDF Files");
		gtk_file_filter_add_pattern (filter, "*.pdf");
		gtk_file_chooser_add_filter (ts->file_selector, filter);

		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, "Postscript Files");
		gtk_file_filter_add_pattern (filter, "*.ps");
		gtk_file_chooser_add_filter (ts->file_selector, filter);

		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, _("All Files"));
		gtk_file_filter_add_pattern (filter, "*");
		gtk_file_chooser_add_filter (ts->file_selector, filter);
		gtk_file_chooser_set_filter (ts->file_selector, filter);
	}

	gtk_file_chooser_unselect_all (ts->file_selector);
	
	filename = g_filename_from_utf8 (ts->file_name, -1,
					 &bytes_read, &bytes_written, NULL);
	if ((filename != NULL) && g_path_is_absolute (filename))
		gtk_file_chooser_set_filename (ts->file_selector,
					       filename);
	else if (ts->file_name != NULL) {
		gtk_file_chooser_set_current_name (ts->file_selector, 
						   ts->file_name);
	}
	
	if (filename != NULL)
		g_free (filename);
   
	g_signal_connect (ts->file_selector, "response",
			  G_CALLBACK (gpa_transport_selector_file_selected_cb),
			  (gpointer) ts);
	
	gtk_window_set_modal(GTK_WINDOW (ts->file_selector), TRUE);
	/* Display that dialog */
	gtk_widget_show_all (GTK_WIDGET(ts->file_selector));
	
	gtk_grab_add (GTK_WIDGET(ts->file_selector));
	gtk_main ();

	if (ts->file_selector != NULL) {
		gtk_widget_destroy (GTK_WIDGET (ts->file_selector));
		ts->file_selector = NULL;
	}
}

static void
gpa_transport_selector_custom_entry_changed_cb (GtkEntry *entry, GPATransportSelector *ts)
{
	const guchar *text;

	if (ts->updating)
		return;

	text = gtk_entry_get_text (entry);
	ts->updating = TRUE;
	gpa_node_set_path_value (ts->node, "Command", text);
	ts->updating = FALSE;
}

static void
gpa_transport_selector_update_widgets (GPATransportSelector *ts)
{
	gchar *backend, *filename, *command;

	backend  = gpa_node_get_path_value (ts->config, "Settings.Transport.Backend");
	filename = gpa_node_get_path_value (ts->config, "Settings.Transport.Backend.FileName");
	command  = gpa_node_get_path_value (ts->config, "Settings.Transport.Backend.Command");
	
	gtk_widget_hide (ts->file_name_label);
	gtk_widget_hide (ts->file_button);
	gtk_widget_hide (ts->custom_entry);

	if (backend && !strcmp (backend, "file")) {
		ts->updating = TRUE;
		g_free (ts->file_name);
		ts->file_name = g_strdup (filename ? filename 
					  : "gnome-print.out");
		gtk_label_set_text (GTK_LABEL (ts->file_name_label), 
				    ts->file_name);
		ts->updating = FALSE;
		gtk_widget_show (ts->file_button);
		gtk_widget_show (ts->file_name_label);
	}
	
	if (backend && !strcmp (backend, "custom")) {
		ts->updating = TRUE;
		gtk_entry_set_text (GTK_ENTRY (ts->custom_entry), command ? command : "lpr %f");
		ts->updating = FALSE;
		gtk_widget_show (ts->custom_entry);
	}

	my_g_free (filename);
	my_g_free (command);
	my_g_free (backend);
}

static void
gpa_transport_selector_item_activate_cb (GPATransportSelector *ts)
{
	GtkTreeIter   iter;
	GPANode *node;

	gtk_combo_box_get_active_iter (GTK_COMBO_BOX (ts->combo), &iter);
	gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (ts->combo)),
		&iter, 1, &node, -1);

	ts->updating = TRUE;
	gpa_node_set_value (ts->node, gpa_node_id (node));
	ts->updating = FALSE;

	gpa_transport_selector_update_widgets (ts);
}

static void
gpa_transport_selector_rebuild_combo (GPATransportSelector *ts)
{
	GtkListStore	*model;
	GtkTreeIter	 iter;
	GPANode *child, *next, *mod, *option = NULL;
	guchar *mod_name, *trans_name;
	gint pos = 0;
	gint sel = -1;

	model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_OBJECT);
	if (ts->node != NULL) {
		guchar *def = gpa_node_get_value (ts->node);
		option = GPA_KEY (ts->node)->option;
		
		for (child = gpa_node_get_child (option, NULL); child != NULL ; child = next) {
			mod = gpa_node_get_child_from_path (child, "Module");
			if (mod != NULL) {
				mod_name = gpa_node_get_value (mod);
				if (gnome_print_transport_exists_by_name (mod_name)) {
					trans_name = gpa_node_get_value (child);
					gtk_list_store_append (model, &iter);
					gtk_list_store_set (model, &iter,
							    0, trans_name,
							    1, child,
							    -1);
					if (GPA_NODE_ID_COMPARE (child, def))
						sel = pos;
					pos++;
					g_free (trans_name);
				}
				g_free (mod_name);
			}

			next = gpa_node_get_child (option, child);
			gpa_node_unref (child);
		}

		if (sel == -1) {
			g_warning ("gpa_transport_selector_rebuild_combo, could not set value of %s to %s",
				   gpa_node_id (option), def);
			sel = 0;
		}

		if (def)
			g_free (def);
	}

	if (pos <= 1)
		gtk_widget_hide (ts->combo);
	else
		gtk_widget_show (ts->combo);

	ts->updating = TRUE;
	gtk_combo_box_set_model (GTK_COMBO_BOX (ts->combo), GTK_TREE_MODEL (model));
	if (pos > 0)
		gtk_combo_box_set_active (GTK_COMBO_BOX (ts->combo), sel);
	ts->updating = FALSE;

	gpa_transport_selector_update_widgets (ts);
}

static void
gpa_transport_selector_connect (GPATransportSelector *ts)
{
	ts->node = gpa_node_lookup (ts->config, "Settings.Transport.Backend");
	if (ts->node)
		ts->handler = g_signal_connect 
			(G_OBJECT (ts->node), 
			 "modified", (GCallback)
			 gpa_transport_selector_node_modified_cb, ts);
}

static void
gpa_transport_selector_config_modified_cb (GPANode *node, guint flags, GPATransportSelector *ts)
{
	gpa_transport_selector_disconnect (ts);
	gpa_transport_selector_connect (ts);
	gpa_transport_selector_rebuild_combo (ts);
}

static void
gpa_transport_selector_node_modified_cb (GPANode *node, guint flags, GPATransportSelector *ts)
{
	if (ts->updating)
		return;
	
	gpa_transport_selector_rebuild_combo (ts);
}

static gint
gpa_transport_selector_construct (GPAWidget *gpaw)
{
	GPATransportSelector *ts;
        GtkCellRenderer *renderer;

	ts = GPA_TRANSPORT_SELECTOR (gpaw);
	ts->config  = GNOME_PRINT_CONFIG_NODE (gpaw->config);
	ts->handler_config = g_signal_connect (G_OBJECT (ts->config), "modified", (GCallback)
					       gpa_transport_selector_config_modified_cb, ts);

	gpa_transport_selector_connect (ts);
	gpa_transport_selector_rebuild_combo (ts);

	g_signal_connect_swapped (G_OBJECT (ts->combo),
		"changed", 
		G_CALLBACK (gpa_transport_selector_item_activate_cb), ts);
        renderer = gtk_cell_renderer_text_new ();
        gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (ts->combo),
		renderer, TRUE);
        gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (ts->combo),
		renderer, "text", 0, NULL);

	return TRUE;
}

static gboolean 
gpa_transport_selector_check_consistency_real (GPATransportSelector *ts)
{
	gchar *backend;
	gchar *selected_filename;
	gsize bytes_read, bytes_written;

	backend  = gpa_node_get_path_value 
		(ts->config, "Settings.Transport.Backend");

	if (backend == NULL)
		return TRUE;

	if (strcmp (backend, "file") != 0)
		return TRUE;
	
	if (ts->file_name_force)
		return TRUE;

	g_return_val_if_fail (ts->file_name != NULL, FALSE);

	selected_filename = g_filename_from_utf8 (ts->file_name, -1,
						  &bytes_read, &bytes_written,
						  NULL);

	g_return_val_if_fail (selected_filename != NULL, FALSE);

	if (g_file_test (selected_filename, G_FILE_TEST_IS_DIR)) {
		GtkWidget *dialog;
		GtkWidget *window = ts->file_name_label;
		
		while ((window != NULL) && !(GTK_IS_WINDOW (window)))
			window = gtk_widget_get_parent (window);
		
		dialog = gtk_message_dialog_new 
			(GTK_WINDOW(window),
			 GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			 GTK_BUTTONS_CLOSE,
			 _("The specified filename \"%s\" is an existing "
			   "directory."), ts->file_name);
		
		gtk_dialog_run (GTK_DIALOG(dialog));
		gtk_widget_destroy (dialog);

		g_free (selected_filename);
		return FALSE;
	}

	if (g_file_test (selected_filename, G_FILE_TEST_EXISTS)) {
		GtkWidget *dialog;
		GtkWidget *window = ts->file_name_label;
		gint response;

		while ((window != NULL) && !(GTK_IS_WINDOW (window)))
			window = gtk_widget_get_parent (window);

		dialog = gtk_message_dialog_new 
			(GTK_WINDOW(window), 
			 GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			 GTK_BUTTONS_YES_NO,
			 _("Should the file %s be overwritten?"), 
			 ts->file_name);
		
		response = gtk_dialog_run (GTK_DIALOG (dialog));

		gtk_widget_destroy (dialog);

		if (GTK_RESPONSE_YES != response) {
			ts->file_name_force = FALSE;
			g_free (selected_filename);
			return FALSE;
		}
		ts->file_name_force = TRUE;
	} else {
		ts->file_name_force = FALSE;
	}

	g_free (selected_filename);
	return TRUE;
}

gboolean 
gpa_transport_selector_check_consistency (GPATransportSelector *ts)
{
	GPATransportSelectorClass *klass 
		= GPA_TRANSPORT_SELECTOR_GET_CLASS (ts);

	if (klass->check_consistency == NULL)
		return TRUE;

	return (klass->check_consistency (ts));
}
