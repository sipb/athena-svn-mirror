/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000 CodeFactory AB
   Copyright (C) 2000 Jonas Borgstr\366m <jonas@codefactory.se>
   Copyright (C) 2000 Anders Carlsson <andersca@codefactory.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <libxml/debugXML.h>
#include <libgtkhtml/dom/core/dom-characterdata.h>
#include <libgtkhtml/dom/core/dom-core-utils.h>
#include "dom-test-tree-model.h"
#include "dom-test-node-menu.h"

typedef struct _DomTestMenuContext DomTestMenuContext;

struct _DomTestMenuContext {
	DomTestWindow *window;
	DomNode *node;
};



static void
dom_test_dialog_response (GtkWidget *widget, gint response_id, gint *result)
{
	if (result)
		*result = response_id;

	gtk_main_quit ();
}

static void
dom_test_exception_dialog (GtkWindow *parent, DomException exc)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (parent,
					 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,   GTK_BUTTONS_OK,
					 "An exception occured!: %s", dom_exception_get_name (exc));
	g_signal_connect (dialog, "response",
			  G_CALLBACK (dom_test_dialog_response), NULL);

	gtk_widget_show_all (dialog);
	
	gtk_main ();
	gtk_widget_destroy (dialog);
}

static DomNode *
dom_test_node_dialog_run (DomTestTreeModelType type, GtkWindow *parent, const gchar *message, DomNode *root)
{
	DomTestTreeModel *tree_model;
	GtkWidget *dialog, *frame, *scrolled_window, *tree_view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	gint result;
	DomNode *result_node = NULL;
	
	dialog = gtk_dialog_new_with_buttons ("DOM Test",
					      parent,
					      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_OK,
					      GTK_RESPONSE_OK,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_CANCEL,
					      NULL);
	
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 250, 300);
	
	g_signal_connect (dialog, "response",
			    G_CALLBACK (dom_test_dialog_response), &result);
	
	frame = gtk_frame_new (message);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	tree_model = dom_test_tree_model_new (root, type);
	
	tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (tree_model));

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("DOM Tree", renderer,
  							   "text", 0,
  							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
				  
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
	gtk_container_add (GTK_CONTAINER (frame), scrolled_window);

	gtk_widget_realize (tree_view);
	gtk_tree_view_expand_all (GTK_TREE_VIEW (tree_view));

	gtk_widget_show_all (dialog);
	gtk_main ();

	if (result == GTK_RESPONSE_OK) {
		GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
		GtkTreeIter iter;
		
		if (gtk_tree_selection_get_selected (sel, NULL, &iter)) {
			result_node = DOM_NODE (iter.user_data);
		}
	}

	gtk_widget_destroy (dialog);
	return result_node;
}

static DomString *
dom_test_string_dialog_run (GtkWindow *parent, const gchar *message, const gchar *initial_value)
{
	GtkWidget *dialog;
	GtkWidget *hbox, *stock, *vbox;
	GtkWidget *entry;
	gint result;
	
	dialog = gtk_dialog_new_with_buttons ("DOM Test",
					      parent,
					      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_OK,
					      GTK_RESPONSE_OK,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_CANCEL,
					      NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	g_signal_connect (dialog, "response",
			  G_CALLBACK (dom_test_dialog_response), &result);

	hbox = gtk_hbox_new (FALSE, 8);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE, 0);
	stock = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG);
	gtk_box_pack_start (GTK_BOX (hbox), stock, FALSE, FALSE, 0);

	vbox = gtk_vbox_new (FALSE, 8);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

	entry = gtk_entry_new ();
	if (initial_value)
		gtk_entry_set_text (GTK_ENTRY (entry), initial_value);
	
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_box_pack_start (GTK_BOX (vbox), gtk_label_new (message), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), entry, TRUE, TRUE, 0);

	gtk_widget_grab_focus (entry);
	
	gtk_widget_show_all (dialog);
	gtk_main ();

	if (result == GTK_RESPONSE_OK) {
		gchar *text = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

		gtk_widget_destroy (dialog);
		return text;
	}
	else {
		gtk_widget_destroy (dialog);
		return NULL;
	}
}

static void
dom_test_document_create_element (GtkWidget *widget, DomTestMenuContext *context)
{
	DomString *str;
	DomNode *node;
	
	if ((str = dom_test_string_dialog_run (GTK_WINDOW (context->window), "Element name:", NULL))) {
		DomException exc = DOM_NO_EXCEPTION;
		
		node = DOM_NODE (dom_Document_createElement (DOM_DOCUMENT (context->window->orphan_root_node), str));

		/* Add the node to the list of orphan nodes */
		dom_Node_appendChild (context->window->orphan_root_node, node, &exc);

		if (exc != DOM_NO_EXCEPTION)
			dom_test_exception_dialog (GTK_WINDOW (context->window), exc);
		else {
			GtkTreeIter iter;
			GtkTreePath *path;
			iter.user_data = node;

			/* Let the model know that things have changed */
			path = gtk_tree_model_get_path (GTK_TREE_MODEL (context->window->orphan_nodes), &iter);
			gtk_tree_model_row_inserted (GTK_TREE_MODEL (context->window->orphan_nodes),
						 path, &iter);
			gtk_tree_path_free (path);
		}

		g_free (str);
	}
}

static void
dom_test_document_create_text_node (GtkWidget *widget, DomTestMenuContext *context)
{
	DomString *str;
	DomNode *node;
	
	if ((str = dom_test_string_dialog_run (GTK_WINDOW (context->window), "Text node contents:", NULL))) {
		DomException exc = DOM_NO_EXCEPTION;
		
		node = DOM_NODE (dom_Document_createTextNode (DOM_DOCUMENT (context->window->orphan_root_node), str));

		/* Add the node to the list of orphan nodes */
		dom_Node_appendChild (context->window->orphan_root_node, node, &exc);

#ifdef LIBXML_DEBUG_ENABLED
		xmlDebugDumpDocument (stdout, (xmlDoc *)context->window->orphan_root_node->xmlnode);
#endif
		
		if (exc != DOM_NO_EXCEPTION)
			dom_test_exception_dialog (GTK_WINDOW (context->window), exc);
		else {
			GtkTreeIter iter;
			GtkTreePath *path;
			iter.user_data = node;
			
			/* Let the model know that things have changed */
			path = gtk_tree_model_get_path (GTK_TREE_MODEL (context->window->orphan_nodes), &iter);
			g_print ("The path is: %s\n", gtk_tree_path_to_string (path));
			gtk_tree_model_row_inserted (GTK_TREE_MODEL (context->window->orphan_nodes),
						 path, &iter);
			gtk_tree_path_free (path);
		}

		g_free (str);
	}
}

static void
dom_test_character_data_set_data (GtkWidget *item, DomTestMenuContext *context)
{
	DomString *str;
	DomString *prevVal = dom_CharacterData__get_data (DOM_CHARACTER_DATA (context->node));
	
	if ((str = dom_test_string_dialog_run (GTK_WINDOW (context->window), "Change data to:", prevVal))) {
		dom_CharacterData__set_data (DOM_CHARACTER_DATA (context->node), str, NULL);

		g_free (str);
	}

	g_free (prevVal);
}

static void
dom_test_node_append_child (GtkWidget *item, DomTestMenuContext *context)
{
	DomNode *node;
	DomException exc = DOM_NO_EXCEPTION;

	if ((node = dom_test_node_dialog_run (DOM_TEST_TREE_MODEL_LIST, GTK_WINDOW (context->window), "Select child node to append:",
					      context->window->orphan_root_node))) {
		GtkTreeIter iter;
		GtkTreePath *path;

		DomNode *imported_node = dom_Document_importNode (dom_Node__get_ownerDocument (context->node), node, TRUE, &exc);
		DomNode *removed_node;

		dom_Node_appendChild (context->node, imported_node, &exc);
		
		if (exc != DOM_NO_EXCEPTION) {
			dom_test_exception_dialog (GTK_WINDOW (context->window), exc);
			g_object_unref (G_OBJECT (imported_node));

			return;
		}
		
		iter.user_data = imported_node;
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (context->window->tree_model), &iter);
		gtk_tree_model_row_inserted (GTK_TREE_MODEL (context->window->tree_model), path, &iter);
		gtk_tree_path_free (path);
		
		if (exc != DOM_NO_EXCEPTION)
			dom_test_exception_dialog (GTK_WINDOW (context->window), exc);


		iter.user_data = node;
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (context->window->orphan_nodes), &iter);
		removed_node = dom_Node_removeChild (DOM_NODE (context->window->orphan_root_node), node, &exc);
		gtk_tree_model_row_deleted (GTK_TREE_MODEL (context->window->orphan_nodes), path);
		gtk_tree_path_free (path);


		g_object_unref (removed_node);

	}
}

static void
dom_test_node_remove_child (GtkWidget *item, DomTestMenuContext *context)
{
	DomNode *node;
	DomException exc = DOM_NO_EXCEPTION;
	
	if ((node = dom_test_node_dialog_run (DOM_TEST_TREE_MODEL_LIST, GTK_WINDOW (context->window), "Select child node to remove:", context->node))) {
		GtkTreeIter iter;
		GtkTreePath *path;
		DomNode *removed_node;
		DomNode *imported_node;
		
		iter.user_data = node;
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (context->window->tree_model), &iter);
		g_print ("The path is: %s\n", gtk_tree_path_to_string (path));

		g_print ("Going to remove: %p %p\n", context->node->xmlnode, node->xmlnode);
		
		removed_node = dom_Node_removeChild (context->node, node, &exc);

		if (exc != DOM_NO_EXCEPTION) {
			dom_test_exception_dialog (GTK_WINDOW (context->window), exc);
			
			gtk_tree_path_free (path);
			
			return;
		}

		gtk_tree_model_row_deleted (GTK_TREE_MODEL (context->window->tree_model), path);
		gtk_tree_path_free (path);

		/* Append the node to the list of orphan nodes */
		imported_node = dom_Document_importNode (DOM_DOCUMENT (context->window->orphan_root_node), removed_node, TRUE, &exc);
		dom_Node_appendChild (context->window->orphan_root_node, imported_node, &exc);

		if (exc != DOM_NO_EXCEPTION) {
			dom_test_exception_dialog (GTK_WINDOW (context->window), exc);

			/* Unref the imported node */
			g_object_unref (G_OBJECT (imported_node));
					
			return;
		}

		/* Unref the old node */
		g_object_unref (G_OBJECT (node));
		
		iter.user_data = imported_node;
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (context->window->orphan_nodes), &iter);
		gtk_tree_model_row_inserted (GTK_TREE_MODEL (context->window->orphan_nodes), path, &iter);
		gtk_tree_path_free (path);
	}

}

GtkWidget *
dom_test_node_menu_new (DomTestWindow *window, DomNode *node)
{
	GtkWidget *menu = gtk_menu_new ();
	GtkWidget *item;
	static DomTestMenuContext context;
	
	context.window = window;
	context.node = node;
	
	if (DOM_IS_NODE (node)) {
 		item = gtk_menu_item_new_with_label ("DomNode");
		gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_menu_item_new ());

		if (dom_Node_hasChildNodes (node)) {
			item = gtk_menu_item_new_with_label ("removeChild");
			g_signal_connect (item, "activate", 
					  G_CALLBACK (dom_test_node_remove_child), &context);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		}

		item = gtk_menu_item_new_with_label ("appendChild");
		g_signal_connect (item, "activate", 
				    G_CALLBACK (dom_test_node_append_child), &context);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		
	}

	if (DOM_IS_CHARACTER_DATA (node)) {
 		item = gtk_menu_item_new_with_label ("DomCharacterData");
		gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_menu_item_new ());

		item = gtk_menu_item_new_with_label ("set data");
		g_signal_connect (item, "activate", 
				    G_CALLBACK (dom_test_character_data_set_data), &context);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	}

	if (DOM_IS_DOCUMENT (node)) {
		item = gtk_menu_item_new_with_label ("DomDocument");
		gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_menu_item_new ());

		item = gtk_menu_item_new_with_label ("createElement");
		g_signal_connect (item, "activate",
				  G_CALLBACK (dom_test_document_create_element), &context);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

		item = gtk_menu_item_new_with_label ("createTextNode");
		g_signal_connect (item, "activate",
				  G_CALLBACK (dom_test_document_create_text_node), &context);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	}
	
	gtk_widget_show_all (GTK_WIDGET (menu));
	
	return menu;
}


