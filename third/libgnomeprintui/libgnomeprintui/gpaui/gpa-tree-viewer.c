/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-tree-viewer.c: A graphical viewer for a GPANode tree
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
 *  Authors:
 *    Chema Celorio <chema@celorio.com>
 *
 *  Copyright (C) 2002 Ximian Inc.
 *
 */

#include <config.h>

#include <string.h>
#include <gtk/gtk.h>
#include <libgnomeprint/private/gpa-node.h>
#include <libgnomeprint/private/gpa-node-private.h>

#include "gpa-tree-viewer.h"

enum
{
	NODE_COLUMN = 0,
	NUM_COLUMNS
};

typedef struct _GpaTreeViewer GpaTreeViewer;

struct _GpaTreeViewer {
	GtkWidget *dialog;
	GtkWidget *id;
	GtkWidget *type;
	GtkWidget *value;
	GtkWidget *location;
	GtkWidget *ref_count;

	GPANode *node;
	guint handler;
};

static void
gpa_tree_viewer_populate_real (GtkTreeStore *model,
			       GPANode *node,
			       GtkTreeIter *parent_iter,
			       guint level)
{
	GtkTreeIter iter;
	GtkTreeIter * copy;
	GPANode *previous_child;
	GPANode *child;

	gtk_tree_store_append (model, &iter, parent_iter);
	gtk_tree_store_set (model, &iter, NODE_COLUMN, node, -1);

	if (level > 2) {
		if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAReference") == 0) {
			return;
		}
	}

	previous_child = NULL;
	while (TRUE) {
		child = gpa_node_get_child (node, previous_child);
		g_assert (child != node);
		if (!child)
			break;
		previous_child = child;
		copy = gtk_tree_iter_copy (&iter);
		gpa_tree_viewer_populate_real (model, child, copy, level+1);
		gtk_tree_iter_free (copy);
	}
	
}

static void
gpa_tree_viewer_populate (GtkTreeStore *model, GPANode *node)
{
	gpa_tree_viewer_populate_real (model, node, NULL, 0);
}

static void
gpa_tree_viewer_cell (GtkTreeViewColumn *tree_column,
				  GtkCellRenderer   *cell,
				  GtkTreeModel      *tree_model,
				  GtkTreeIter       *iter,
				  gpointer           data)
{
	GPANode *node;

	gtk_tree_model_get (tree_model, iter, NODE_COLUMN, &node, -1);
	g_object_set (G_OBJECT (cell), "text", gpa_node_id (node), NULL);

	if (strcmp (G_OBJECT_TYPE_NAME (node), "GPAReference") == 0) {
		g_object_set (G_OBJECT (cell), "foreground", "blue", NULL);
	} else {
		g_object_set (G_OBJECT (cell), "foreground", "black", NULL);
	}
}

static void
gpa_tree_viewer_info_refresh (GPANode *node, guint flags, GpaTreeViewer *gtv)
{
	gchar *location;
	gchar *value = NULL;
	gchar *ref_count;
	
	gtk_entry_set_text (GTK_ENTRY (gtv->id),   gpa_node_id (node));
	gtk_entry_set_text (GTK_ENTRY (gtv->type), G_OBJECT_TYPE_NAME (node));

	location = g_strdup_printf ("0x%x", GPOINTER_TO_UINT (node));
	gtk_entry_set_text (GTK_ENTRY (gtv->location), location);
	g_free (location);

	if (GPA_NODE_GET_CLASS (node)->get_value)
		value    = gpa_node_get_value (node);

	if (value) {
		gtk_entry_set_text (GTK_ENTRY (gtv->value),    value);
		g_free (value);
	} else {
		gtk_entry_set_text (GTK_ENTRY (gtv->value),    "N/A");
	}

	ref_count = g_strdup_printf ("%d", G_OBJECT (node)->ref_count);
	gtk_entry_set_text (GTK_ENTRY (gtv->ref_count), ref_count);
	g_free (ref_count);
}

static gboolean
gpa_tree_viewer_selection_changed_cb (GtkTreeSelection *selection, GpaTreeViewer *gtv)
{
	GtkTreeModel *model;
	GtkTreeView *view;
	GtkTreeIter iter;
	GPANode *node;

	view = gtk_tree_selection_get_tree_view (selection);
	model = gtk_tree_view_get_model (view);

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return TRUE;

	gtk_tree_model_get (model, &iter, NODE_COLUMN, &node, -1);
	
	gpa_tree_viewer_info_refresh (node, 0, gtv);

	if (gtv->handler) {
		g_signal_handler_disconnect (gtv->node, gtv->handler);
	}
	gtv->node    = node;
	gtv->handler = g_signal_connect (G_OBJECT (node), "modified",
							   (GCallback) gpa_tree_viewer_info_refresh, gtv);
	return TRUE;
}

static GtkWidget *
gpa_tree_viewer_table_append (GtkTable *table, const guchar *text, guint pos)
{
	GtkWidget *label;
	GtkWidget *entry;
	
	label = gtk_label_new (text);
	gtk_table_attach_defaults (table, label, 0, 1, pos, pos + 1);
	entry = gtk_entry_new ();
	gtk_editable_set_editable (GTK_EDITABLE (entry), FALSE);	
	gtk_table_attach_defaults (table, entry, 1, 2, pos, pos + 1);
	
	return entry;
}

static GtkWidget *
gpa_tree_viewer_info (GpaTreeViewer *gtv)
{
	GtkWidget *frame;
	GtkTable *table;

	table = (GtkTable *) gtk_table_new (0, 0, FALSE);

	gtv->id        = gpa_tree_viewer_table_append (table, "Id:",       0);
	gtv->type      = gpa_tree_viewer_table_append (table, "Type:",     1);
	gtv->value     = gpa_tree_viewer_table_append (table, "Value:",    2);
	gtv->location  = gpa_tree_viewer_table_append (table, "Location:", 3);
	gtv->ref_count = gpa_tree_viewer_table_append (table, "RefCount:", 4);

	frame = gtk_frame_new ("Node Info");
	gtk_container_set_border_width (GTK_CONTAINER (frame), 12);
	gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (table));
	
	return frame;
}

GtkWidget *
gpa_tree_viewer_new (GPANode *node)
{
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell_render;
	GtkTreeStore *model;
	GtkWidget *scrolled_window;
	GtkWidget *view;
	GtkWidget *dialog;
	GtkWidget *info;
	GpaTreeViewer *gtv;

	gtv = g_new0 (GpaTreeViewer, 1);
	
	model = gtk_tree_store_new (1, G_TYPE_POINTER);

	gpa_tree_viewer_populate (model, node);

	view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));

	/* Set the cell renderer */
	cell_render = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Node", cell_render, NULL);
	gtk_tree_view_column_set_cell_data_func (column, cell_render, gpa_tree_viewer_cell, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

	/* Selection */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
	g_signal_connect (G_OBJECT (selection), "changed",
				   (GCallback) gpa_tree_viewer_selection_changed_cb, gtv);

	/* Info Table */
	info = gpa_tree_viewer_info (gtv);
	
	/* Scrolled windows and dialog */
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request (scrolled_window, 450, 650);
	gtk_container_add (GTK_CONTAINER (scrolled_window), view);

	dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (dialog), "GPANode tree");
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dialog)->vbox), scrolled_window);
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dialog)->vbox), info);
	gtv->dialog = dialog;
	gtk_widget_show_all (dialog);
	
	return dialog;
}
