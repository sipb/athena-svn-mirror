/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gconf-editor
 *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gconf-cell-renderer.h"
#include "gconf-key-editor.h"

#include <gtk/gtkalignment.h>
#include <gtk/gtkcellrenderer.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkvbbox.h>
#include <libintl.h>
#include <string.h>

#define _(x) gettext (x)

enum
{
  EDIT_INTEGER,
  EDIT_BOOLEAN,
  EDIT_STRING,
  EDIT_FLOAT,
  EDIT_LIST
};


static void
option_menu_changed (GtkWidget *option_menu,
		     GConfKeyEditor *editor)
{
	int index;

	index = gtk_option_menu_get_history (GTK_OPTION_MENU (option_menu));
	
	gtk_notebook_set_current_page (GTK_NOTEBOOK (editor->notebook),
				       index);

	editor->active_type = index;
}

static void
bool_button_toggled (GtkWidget *bool_button,
		     gpointer   data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bool_button)))
		gtk_label_set_label (GTK_LABEL (GTK_BIN (bool_button)->child),
				     _("T_rue"));
	else
		gtk_label_set_label (GTK_LABEL (GTK_BIN (bool_button)->child),
				     _("_False"));
}

static void
gconf_key_editor_list_entry_changed (GConfCellRenderer *cell, const gchar *path_str, GConfValue *new_value, GConfKeyEditor *editor)
{
	GtkTreeIter iter;
	GtkTreePath *path;

	path = gtk_tree_path_new_from_string (path_str);
        gtk_tree_model_get_iter (GTK_TREE_MODEL(editor->list_model), &iter, path);

	gtk_list_store_set (editor->list_model, &iter,
			    0, new_value,
			    -1);
}

static void
list_type_menu_changed (GtkWidget *option_menu,
			GConfKeyEditor *editor)
{
	gtk_list_store_clear (editor->list_model);
}

static GtkWidget *
gconf_key_editor_create_option_menu (GConfKeyEditor *editor)
{
	GtkWidget *option_menu, *menu;

	option_menu = gtk_option_menu_new ();

	/* These have to be ordered so the EDIT_ enum matches the
	 * menu indices
	 */
	menu = gtk_menu_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), 
			       GTK_WIDGET (gtk_menu_item_new_with_label (_("Integer"))));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), 
			       GTK_WIDGET (gtk_menu_item_new_with_label (_("Boolean"))));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), 
			       GTK_WIDGET (gtk_menu_item_new_with_label (_("String"))));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), 
			       GTK_WIDGET (gtk_menu_item_new_with_label (_("Float"))));
 	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
			       GTK_WIDGET (gtk_menu_item_new_with_label (_("List"))));
	gtk_widget_show_all (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), (menu));

	g_signal_connect (option_menu, "changed",
			  G_CALLBACK (option_menu_changed),
			  editor);
	
	gtk_widget_show_all (option_menu);
	return option_menu;
}

static GtkWidget *
gconf_key_editor_create_list_type_menu (GConfKeyEditor *editor)
{
	GtkWidget *option_menu, *menu;

	option_menu = gtk_option_menu_new ();

	/* These have to be ordered so the EDIT_ enum matches the
	 * menu indices
	 */
	menu = gtk_menu_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
			       GTK_WIDGET (gtk_menu_item_new_with_label (_("Integer"))));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
			       GTK_WIDGET (gtk_menu_item_new_with_label (_("Boolean"))));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu),
			       GTK_WIDGET (gtk_menu_item_new_with_label (_("String"))));
	gtk_widget_show_all (menu);
        gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), (menu));

	g_signal_connect (option_menu, "changed",
			  G_CALLBACK (list_type_menu_changed),
			  editor);

        gtk_widget_show_all (option_menu);
        return option_menu;
}

static void
update_list_buttons (GConfKeyEditor *editor)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;
	gint selected;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (editor->list_widget));

	gtk_widget_set_sensitive (editor->remove_button, FALSE);
	gtk_widget_set_sensitive (editor->go_up_button, FALSE);
	gtk_widget_set_sensitive (editor->go_down_button, FALSE);

	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
		path = gtk_tree_model_get_path (model, &iter);

		selected = gtk_tree_path_get_indices (path)[0];

		gtk_widget_set_sensitive (editor->remove_button, TRUE);
		gtk_widget_set_sensitive (editor->go_up_button, selected > 0);
		gtk_widget_set_sensitive (editor->go_down_button, selected < gtk_tree_model_iter_n_children (model, NULL) - 1);

		gtk_tree_path_free (path);
	}
}

static void
list_selection_changed (GtkTreeSelection *selection,
		        GConfKeyEditor *editor)
{
	update_list_buttons (editor);
}

static void
list_add_clicked (GtkButton *button,
		  GConfKeyEditor *editor)
{
	GtkWidget *dialog;
	GtkWidget *hbox1, *hbox2;
	GtkWidget *stock;
	GtkWidget *value_widget;
	GtkWidget *label;
	gint response;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	dialog = gtk_dialog_new_with_buttons (_("Add new list entry"),
                                              GTK_WINDOW (editor),
					      GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OK, GTK_RESPONSE_OK,
					      NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	
	hbox1 = gtk_hbox_new (FALSE, 8);
	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 8);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox1, FALSE, FALSE, 0);

	stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start (GTK_BOX (hbox1), stock, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new (FALSE, 3);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox2, TRUE, TRUE, 0);

	label = gtk_label_new_with_mnemonic (_("_New list element:"));
	gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);

	switch (gtk_option_menu_get_history (GTK_OPTION_MENU (editor->list_type_menu))) {
		case EDIT_INTEGER:
			value_widget = gtk_spin_button_new_with_range (G_MININT, G_MAXINT, 1);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (value_widget), 0);
			break;
		case EDIT_BOOLEAN:
			value_widget = gtk_toggle_button_new_with_mnemonic (_("_False"));
			g_signal_connect (value_widget, "toggled",
		                          G_CALLBACK (bool_button_toggled),
					  editor);
			break;
		case EDIT_STRING:
			value_widget = gtk_entry_new ();
			gtk_entry_set_activates_default (GTK_ENTRY (value_widget), TRUE);
			break;
		default:
			value_widget = NULL;
			g_assert_not_reached ();
	}

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), value_widget);
	gtk_box_pack_start (GTK_BOX (hbox2), value_widget, FALSE, FALSE, 0);

	gtk_widget_show_all (hbox1);

	response = gtk_dialog_run (GTK_DIALOG (dialog));

	if (response == GTK_RESPONSE_OK) {
		GConfValue *value;
		
	        value = NULL;

		switch (gtk_option_menu_get_history (GTK_OPTION_MENU (editor->list_type_menu))) {
			case EDIT_INTEGER:
				value = gconf_value_new (GCONF_VALUE_INT);
				gconf_value_set_int (value,
						     gtk_spin_button_get_value (GTK_SPIN_BUTTON (value_widget)));
				break;

			case EDIT_BOOLEAN:
				value = gconf_value_new (GCONF_VALUE_BOOL);
				gconf_value_set_bool (value,
						      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (value_widget)));
				break;

			case EDIT_STRING:
		                {
		                        char *text;
				
		                        text = gtk_editable_get_chars (GTK_EDITABLE (value_widget), 0, -1);
		                        value = gconf_value_new (GCONF_VALUE_STRING);
		                        gconf_value_set_string (value, text);
		                        g_free (text);
		                }
	                	break;
			default:
				g_assert_not_reached ();
				
		}

		gtk_list_store_append (editor->list_model, &iter);
                gtk_list_store_set (editor->list_model, &iter, 0, value, -1);

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (editor->list_widget));
		gtk_tree_selection_select_iter(selection, &iter);
	}
	
	gtk_widget_destroy (dialog);
}

static void
list_remove_clicked (GtkButton *button,
		     GConfKeyEditor *editor)
{
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (editor->list_widget));

	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		gtk_list_store_remove (editor->list_model, &iter);
	}
}

static void
list_go_up_clicked (GtkButton *button,
		    GConfKeyEditor *editor)
{
	GtkTreeIter iter_first;
	GtkTreeIter iter_second;
	GtkTreeSelection *selection;
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (editor->list_widget));

	if (gtk_tree_selection_get_selected (selection, NULL, &iter_second)) {
		GConfValue *first;
		GConfValue *second;
		GtkTreePath *path;
		
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (editor->list_model), &iter_second);
		gtk_tree_path_prev (path);
		gtk_tree_model_get_iter (GTK_TREE_MODEL (editor->list_model), &iter_first, path);
		
		gtk_tree_model_get (GTK_TREE_MODEL (editor->list_model), &iter_first, 0, &first, -1);
		gtk_tree_model_get (GTK_TREE_MODEL (editor->list_model), &iter_second, 0, &second, -1);
				
		gtk_list_store_set (editor->list_model, &iter_first, 0, second, -1);
		gtk_list_store_set (editor->list_model, &iter_second, 0, first, -1);

		gtk_tree_path_free (path);

		gtk_tree_selection_select_iter(selection, &iter_first);
	}
}

static void
list_go_down_clicked (GtkButton *button,
		      GConfKeyEditor *editor)
{
	GtkTreeIter iter_first;
	GtkTreeIter iter_second;
	GtkTreeSelection *selection;
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (editor->list_widget));

	if (gtk_tree_selection_get_selected (selection, NULL, &iter_first)) {
		GConfValue *first;
		GConfValue *second;

		iter_second = iter_first;

		gtk_tree_model_iter_next (GTK_TREE_MODEL (editor->list_model), &iter_second);

		gtk_tree_model_get (GTK_TREE_MODEL (editor->list_model), &iter_first, 0, &first, -1);
		gtk_tree_model_get (GTK_TREE_MODEL (editor->list_model), &iter_second, 0, &second, -1);
				
		gtk_list_store_set (editor->list_model, &iter_first, 0, second, -1);
		gtk_list_store_set (editor->list_model, &iter_second, 0, first, -1);
		gtk_tree_selection_select_iter(selection, &iter_second);
	}
}

static void
fix_button_align (GtkWidget *button)
{
        GtkWidget *child = gtk_bin_get_child (GTK_BIN (button));

	if (GTK_IS_ALIGNMENT (child))
		g_object_set (G_OBJECT (child), "xalign", 0.0, NULL);
        else if (GTK_IS_LABEL (child))
                g_object_set (G_OBJECT (child), "xalign", 0.0, NULL);
}


static void
gconf_key_editor_class_init (GConfKeyEditorClass *klass)
{
}

static void
gconf_key_editor_init (GConfKeyEditor *editor)
{
	GtkWidget *hbox, *frame;
	GtkWidget *label;
	GtkWidget *value_box;
	GtkWidget *button_box, *button;
	GtkSizeGroup *size_group;
	GtkCellRenderer *cell_renderer;
	GtkTreeSelection *list_selection;
	GtkWidget *sw;

	gtk_dialog_add_buttons (GTK_DIALOG (editor),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
				GTK_STOCK_OK, GTK_RESPONSE_OK,
				NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (editor), GTK_RESPONSE_OK);
	
	size_group = gtk_size_group_new (GTK_SIZE_GROUP_BOTH);
	
	editor->path_box = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (editor->path_box), 8);
	label = gtk_label_new (_("Key path:"));
	gtk_size_group_add_widget (size_group, label);
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
	gtk_box_pack_start (GTK_BOX (editor->path_box), label, FALSE, FALSE, 4);
	editor->path_label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (editor->path_label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (editor->path_box), editor->path_label, TRUE, TRUE, 4);
	gtk_widget_show_all (editor->path_box);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editor)->vbox), editor->path_box, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
	label = gtk_label_new_with_mnemonic (_("Key _name:"));
	gtk_size_group_add_widget (size_group, label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 4);
	editor->name_entry = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (editor->name_entry), TRUE);
	gtk_box_pack_start (GTK_BOX (hbox), editor->name_entry, TRUE, TRUE, 4);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), editor->name_entry);
	gtk_widget_show_all (hbox);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editor)->vbox), hbox, FALSE, FALSE, 0);

	frame = gtk_frame_new (NULL);

	hbox = gtk_hbox_new (FALSE, 2);
	label = gtk_label_new_with_mnemonic (_("_Type:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
	editor->option_menu = gconf_key_editor_create_option_menu (editor);
	gtk_box_pack_start (GTK_BOX (hbox),
			    editor->option_menu,
			    FALSE, FALSE, 2);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), editor->option_menu);
	gtk_widget_show_all (hbox);
	gtk_frame_set_label_widget (GTK_FRAME (frame), hbox);

	editor->notebook = gtk_notebook_new ();
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (editor->notebook),
				    FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (editor->notebook),
				      FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (editor->notebook), 3);
	gtk_container_add (GTK_CONTAINER (frame), editor->notebook);

	editor->active_type = EDIT_INTEGER;

	/* These have to be ordered such that the EDIT_ enum matches
	 * the page indices
	 */

	/* EDIT_INTEGER */
	value_box = gtk_hbox_new (FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("Key _value:"));
	editor->int_widget = gtk_spin_button_new_with_range (G_MININT, G_MAXINT, 1);

	/* Set a nicer default value */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->int_widget), 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), editor->int_widget);
	gtk_box_pack_start (GTK_BOX (value_box), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (value_box), editor->int_widget, TRUE, TRUE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (editor->notebook),
				  value_box, NULL);

	/* EDIT_BOOLEAN */
	value_box = gtk_hbox_new (FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("Key _value:"));
	editor->bool_widget = gtk_toggle_button_new_with_mnemonic (_("_False"));
	g_signal_connect (editor->bool_widget, "toggled",
			  G_CALLBACK (bool_button_toggled),
			  editor);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), editor->bool_widget);
	gtk_box_pack_start (GTK_BOX (value_box), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (value_box), editor->bool_widget, TRUE, TRUE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (editor->notebook),
				  value_box, NULL);

	/* EDIT_STRING */
	value_box = gtk_hbox_new (FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("Key _value:"));
	editor->string_widget = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (editor->string_widget), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), editor->string_widget);	
	gtk_box_pack_start (GTK_BOX (value_box), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (value_box), editor->string_widget, TRUE, TRUE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (editor->notebook),
				  value_box, NULL);

	/* EDIT_INTEGER */
	value_box = gtk_hbox_new (FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("Key _value:"));
	editor->float_widget = gtk_spin_button_new_with_range (G_MINFLOAT, G_MAXFLOAT, 0.1);

	/* Set a nicer default value */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->float_widget), 0.0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), editor->float_widget);
	gtk_box_pack_start (GTK_BOX (value_box), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (value_box), editor->float_widget, TRUE, TRUE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (editor->notebook),
				  value_box, NULL);

        /* EDIT_LIST */
        value_box = gtk_vbox_new (FALSE, 3);

	hbox = gtk_hbox_new (FALSE, 3);
	label = gtk_label_new_with_mnemonic (_("List _type:"));
	editor->list_type_menu = gconf_key_editor_create_list_type_menu (editor);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), editor->list_type_menu);
	gtk_box_pack_start (GTK_BOX (value_box), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox),
                            editor->list_type_menu, TRUE, TRUE, 0);

	label = gtk_label_new_with_mnemonic (_("Key _value:"));
	gtk_misc_set_alignment(GTK_MISC (label), 0.0, 0.5);
  
	hbox = gtk_hbox_new (FALSE, 3);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
					     GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,			      					GTK_POLICY_AUTOMATIC);

        editor->list_model = gtk_list_store_new (1, g_type_from_name("GConfValue"));
	editor->list_widget = gtk_tree_view_new_with_model (GTK_TREE_MODEL (editor->list_model));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (editor->list_widget), FALSE);
	list_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (editor->list_widget));
	g_signal_connect (G_OBJECT (list_selection), "changed",
			  G_CALLBACK (list_selection_changed), editor);
 	
	
	cell_renderer = gconf_cell_renderer_new ();
	g_signal_connect (G_OBJECT (cell_renderer), "changed", G_CALLBACK (gconf_key_editor_list_entry_changed), editor);

	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (editor->list_widget), -1, NULL, cell_renderer, "value", 0, NULL);

	button_box = gtk_vbutton_box_new ();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (button_box), 3);
	button = gtk_button_new_from_stock (GTK_STOCK_ADD);
	fix_button_align (button);
	editor->remove_button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
	fix_button_align (editor->remove_button);
	editor->go_up_button = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
	fix_button_align (editor->go_up_button);
	editor->go_down_button = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
	fix_button_align (editor->go_down_button);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (list_add_clicked), editor);
	g_signal_connect (G_OBJECT (editor->remove_button), "clicked",
			  G_CALLBACK (list_remove_clicked), editor);
	g_signal_connect (G_OBJECT (editor->go_up_button), "clicked",
			  G_CALLBACK (list_go_up_clicked), editor);
	g_signal_connect (G_OBJECT (editor->go_down_button), "clicked",
			  G_CALLBACK (list_go_down_clicked), editor);
	gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (button_box), editor->remove_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (button_box), editor->go_up_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (button_box), editor->go_down_button, FALSE, FALSE, 0);

	update_list_buttons (editor);

        gtk_label_set_mnemonic_widget (GTK_LABEL (label), editor->list_widget); 
        gtk_box_pack_start (GTK_BOX (value_box), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (value_box), hbox, TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), sw, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), button_box, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (sw), editor->list_widget);
  	gtk_notebook_append_page (GTK_NOTEBOOK (editor->notebook),
  				  value_box, NULL);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_container_set_border_width (GTK_CONTAINER (editor->notebook), 8);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editor)->vbox), frame, TRUE, TRUE, 8);
	gtk_widget_show_all (frame);
}

GType
gconf_key_editor_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (GConfKeyEditorClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) gconf_key_editor_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (GConfKeyEditor),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gconf_key_editor_init
		};

		object_type = g_type_register_static (GTK_TYPE_DIALOG, "GConfKeyEditor", &object_info, 0);
	}

	return object_type;
}

GtkWidget *
gconf_key_editor_new (GConfKeyEditorAction action)
{
	GConfKeyEditor *dialog;

	dialog = g_object_new (GCONF_TYPE_KEY_EDITOR, NULL);

	switch (action) {
	case GCONF_KEY_EDITOR_NEW_KEY:
		gtk_window_set_title (GTK_WINDOW (dialog), _("New key"));
		gtk_widget_grab_focus (dialog->name_entry);
		break;
	case GCONF_KEY_EDITOR_EDIT_KEY:
		gtk_window_set_title (GTK_WINDOW (dialog), _("Edit key"));

		gtk_widget_set_sensitive (dialog->name_entry, FALSE);
		gtk_widget_hide (dialog->path_box);
		break;
	default:
		break;
	}
	
	return GTK_WIDGET (dialog);
}

void
gconf_key_editor_set_value (GConfKeyEditor *editor, GConfValue *value)
{
	if (value == NULL) {
		g_print ("dealing with an unset value\n");
		return;
	}

	gtk_widget_set_sensitive (editor->option_menu, FALSE);
	gtk_widget_set_sensitive (editor->list_type_menu, FALSE);

	switch (value->type) {
	case GCONF_VALUE_INT:
		gtk_option_menu_set_history (GTK_OPTION_MENU (editor->option_menu), EDIT_INTEGER);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->int_widget), gconf_value_get_int (value));
		break;
	case GCONF_VALUE_STRING:
		gtk_option_menu_set_history (GTK_OPTION_MENU (editor->option_menu), EDIT_STRING);
		gtk_entry_set_text (GTK_ENTRY (editor->string_widget), gconf_value_get_string (value));
		break;
	case GCONF_VALUE_BOOL:
		gtk_option_menu_set_history (GTK_OPTION_MENU (editor->option_menu), EDIT_BOOLEAN);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (editor->bool_widget), gconf_value_get_bool (value));
		break;
	case GCONF_VALUE_FLOAT:
		gtk_option_menu_set_history (GTK_OPTION_MENU (editor->option_menu), EDIT_FLOAT);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->float_widget), gconf_value_get_float (value));
		break;
	case GCONF_VALUE_LIST:
		{
			GSList* iter = gconf_value_get_list(value);
			
			switch (gconf_value_get_list_type(value)) {
				case GCONF_VALUE_INT:
					gtk_option_menu_set_history (GTK_OPTION_MENU (editor->list_type_menu), EDIT_INTEGER);
					break;
				case GCONF_VALUE_STRING:
					gtk_option_menu_set_history (GTK_OPTION_MENU (editor->list_type_menu), EDIT_STRING);
					break;
				case GCONF_VALUE_BOOL:
					gtk_option_menu_set_history (GTK_OPTION_MENU (editor->list_type_menu), EDIT_BOOLEAN);
					break;
				default:
					g_assert_not_reached ();
			}

			gtk_option_menu_set_history (GTK_OPTION_MENU (editor->option_menu), EDIT_LIST);
				
		        while (iter != NULL) {
				GConfValue* element = (GConfValue*) iter->data;
				GtkTreeIter tree_iter;

			        gtk_list_store_append (editor->list_model, &tree_iter);
			        gtk_list_store_set (editor->list_model, &tree_iter, 0, element, -1);
				iter = g_slist_next(iter);
			}
		}
		break;
	default:
		g_assert_not_reached ();
		break;
	}
	
}

GConfValue*
gconf_key_editor_get_value (GConfKeyEditor *editor)
{
	GConfValue *value;

	value = NULL;
	
	switch (editor->active_type) {

	case EDIT_INTEGER:
		value = gconf_value_new (GCONF_VALUE_INT);
		gconf_value_set_int (value,
				     gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->int_widget)));
		break;

	case EDIT_BOOLEAN:
		value = gconf_value_new (GCONF_VALUE_BOOL);
		gconf_value_set_bool (value,
				      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (editor->bool_widget)));
		break;

	case EDIT_STRING:
		{
			char *text;
			
			text = gtk_editable_get_chars (GTK_EDITABLE (editor->string_widget), 0, -1);
			value = gconf_value_new (GCONF_VALUE_STRING);
			gconf_value_set_string (value, text);
			g_free (text);
		}
		break;
	case EDIT_FLOAT:
		value = gconf_value_new (GCONF_VALUE_FLOAT);
		gconf_value_set_float (value,
				       gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->float_widget)));
		break;

	case EDIT_LIST:
		{
			GSList* list = NULL;
			GtkTreeIter iter;
			GtkTreeModel* model = GTK_TREE_MODEL (editor->list_model);

			if (gtk_tree_model_get_iter_first (model, &iter)) {
				do {
					GConfValue *element;

					gtk_tree_model_get (model, &iter, 0, &element, -1);
					list = g_slist_append (list, element);
				} while (gtk_tree_model_iter_next (model, &iter));
			}

			value = gconf_value_new (GCONF_VALUE_LIST);

			switch (gtk_option_menu_get_history (GTK_OPTION_MENU (editor->list_type_menu))) {
			        case EDIT_INTEGER:
					gconf_value_set_list_type (value, GCONF_VALUE_INT);
					break;
				case EDIT_BOOLEAN:
					gconf_value_set_list_type (value, GCONF_VALUE_BOOL);
					break;
				case EDIT_STRING:
					gconf_value_set_list_type (value, GCONF_VALUE_STRING);
					break;
				default:
					g_assert_not_reached ();
			}
			gconf_value_set_list_nocopy (value, list);
		}
		break;
	}

	return value;
}

void
gconf_key_editor_set_key_path (GConfKeyEditor *editor, const char *path)
{
	gtk_label_set_text (GTK_LABEL (editor->path_label), path);
}

void
gconf_key_editor_set_key_name (GConfKeyEditor *editor, const char *path)
{
	gtk_entry_set_text (GTK_ENTRY (editor->name_entry), path);
}

G_CONST_RETURN char *
gconf_key_editor_get_key_name (GConfKeyEditor *editor)
{
	return gtk_entry_get_text (GTK_ENTRY (editor->name_entry));
}

char *
gconf_key_editor_get_full_key_path (GConfKeyEditor *editor)
{
	char *full_key_path;
	const char *key_path;

	key_path = gtk_label_get_text (GTK_LABEL (editor->path_label));

	if (key_path[strlen(key_path) - 1] != '/') {
		full_key_path = g_strdup_printf ("%s/%s",
						 key_path,
						 gtk_entry_get_text (GTK_ENTRY (editor->name_entry)));
	}
	else {
		full_key_path = g_strdup_printf ("%s%s",
						 key_path,
						 gtk_entry_get_text (GTK_ENTRY (editor->name_entry)));

	}

	return full_key_path;
}

