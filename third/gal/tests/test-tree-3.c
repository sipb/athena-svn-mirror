/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * test-tree-3.c
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
#include <gnome.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gal/e-table/e-table-header.h>
#include <gal/e-table/e-table-header-item.h>
#include <gal/e-table/e-table-item.h>
#include <gal/e-table/e-cell-text.h>
#include <gal/e-table/e-cell-tree.h>
#include <gal/e-table/e-cell-checkbox.h>
#include <gal/e-table/e-table.h>
#include <gal/e-table/e-tree-memory.h>
#include <gal/e-table/e-tree-memory-callbacks.h>
#include <gal/e-table/e-tree-table-adapter.h>

#define COLS 4

#define IMPORTANCE_COLUMN 4
#define COLOR_COLUMN 5

/*
 * Here we define the initial layout of the table.  This is an xml
 * format that allows you to change the initial ordering of the
 * columns or to do sorting or grouping initially.  This specification
 * shows all 5 columns, but moves the importance column nearer to the
 * front.  It also sorts by the "Full Name" column (ascending.)
 * Sorting and grouping take the model column as their arguments
 * (sorting is specified by the "column" argument to the leaf elemnt.
 */

/*
 * Virtual Column list:
 * 0   Subject
 * 1   Full Name
 * 2   Email
 * 3   Date
 */

#define INITIAL_SPEC "<ETableSpecification>                    	       \
  <ETableColumn model_col=\"0\" _title=\"Subject\"   expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"tree-string\" compare=\"string\"/> \
  <ETableColumn model_col=\"1\" _title=\"Full Name\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" 	    compare=\"string\"/> \
  <ETableColumn model_col=\"2\" _title=\"Email\"     expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" 	    compare=\"string\"/> \
  <ETableColumn model_col=\"3\" _title=\"Date\"      expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" 	    compare=\"string\"/> \
        <ETableState> \
	        <column source=\"0\"/> \
	        <column source=\"1\"/> \
	        <column source=\"2\"/> \
	        <column source=\"3\"/> \
	        <grouping></grouping> \
        </ETableState> \
</ETableSpecification>"

GtkWidget *e_table;
ETreeTableAdapter *adapter;

/*
 * ETreeSimple callbacks
 * These are the callbacks that define the behavior of our custom model.
 */

static GdkPixbuf *
my_icon_at (ETreeModel *etm, ETreePath path, void *model_data)
{
	/* No icon, since the cell tree renderer takes care of the +/- icons itself. */
	return NULL;
}

/* This function returns the number of columns in our ETableModel. */
static int
my_column_count (ETreeModel *etc, void *data)
{
	return COLS;
}

/* This function duplicates the value passed to it. */
static void *
my_duplicate_value (ETreeModel *etc, int col, const void *value, void *data)
{
	return g_strdup (value);
}

/* This function frees the value passed to it. */
static void
my_free_value (ETreeModel *etc, int col, void *value, void *data)
{
	g_free (value);
}

/* This function creates an empty value. */
static void *
my_initialize_value (ETreeModel *etc, int col, void *data)
{
	return g_strdup ("");
}

/* This function reports if a value is empty. */
static gboolean
my_value_is_empty (ETreeModel *etc, int col, const void *value, void *data)
{
	return !(value && *(char *)value);
}

/* This function reports if a value is empty. */
static char *
my_value_to_string (ETreeModel *etc, int col, const void *value, void *data)
{
	return g_strdup(value);
}

/* This function returns the value at a particular point in our ETreeModel. */
static void *
my_value_at (ETreeModel *etm, ETreePath path, int col, void *model_data)
{
	if (e_tree_model_node_is_root (etm, path)) {
		if (col == 0)
			return "<Root>";
		else
			return "";
				      
	}
	else {
		switch (col) {
		case 0: return e_tree_memory_node_get_data (E_TREE_MEMORY(etm), path);
		case 1: return "Chris Toshok";
		case 2: return "toshok@helixcode.com";
		case 3: return "Jun 07 2000";
		default: return NULL;
		}
	}
}

/* This function sets the value at a particular point in our ETreeModel. */
static void
my_set_value_at (ETreeModel *etm, ETreePath path, int col, const void *val, void *model_data)
{
	if (e_tree_model_node_is_root (etm, path))
		return;

	if (col == 0) {
		char *str = e_tree_memory_node_get_data (E_TREE_MEMORY(etm), path);
		g_free (str);
		e_tree_memory_node_set_data (E_TREE_MEMORY(etm), path, g_strdup(val));
	}
}

/* This function returns whether a particular cell is editable. */
static gboolean
my_is_editable (ETreeModel *etm, ETreePath path, int col, void *model_data)
{
	if (col == 0)
		return TRUE;
	else
		return FALSE;
}

#if 0
static gint
ascending_compare (ETreeModel *model,
		   ETreePath  node1,
		   ETreePath  node2)
{
	char *path1, *path2;
	ETreeMemory *etmm = E_TREE_MEMORY(model);

	path1 = e_tree_memory_node_get_data (etmm, node1);
	path2 = e_tree_memory_node_get_data (etmm, node2);

	return g_strcasecmp (path1, path2);
}

static gint
descending_compare (ETreeModel *model,
		    ETreePath  node1,
		    ETreePath  node2)
{
	return - ascending_compare (model, node1, node2);
}
#endif

static void
sort_descending (GtkButton *button, gpointer data)
{
#if 0
	ETreeModel *model = E_TREE_MODEL (data);
	e_tree_model_node_set_compare_function (model, e_tree_model_get_root (model), ascending_compare);
#endif
}

static void
sort_ascending (GtkButton *button, gpointer data)
{
#if 0
	ETreeModel *model = E_TREE_MODEL (data);
	e_tree_model_node_set_compare_function (model, e_tree_model_get_root (model), descending_compare);
#endif
}

/* We create a window containing our new tree. */
static void
create_tree (void)
{
	GtkWidget *window, *frame, *button, *vbox;
	int i, j;
	ETreeModel *e_tree_model = NULL;
	ETreePath root_node;
	ETreeMemory *etmm;

	/* here we create our model.  This uses the functions we defined
	   earlier. */
	e_tree_model = e_tree_memory_callbacks_new (my_icon_at,

						    my_column_count,

						    NULL,
						    NULL,

						    NULL,
						    NULL,

						    my_value_at,
						    my_set_value_at,
						    my_is_editable,

						    my_duplicate_value,
						    my_free_value,
						    my_initialize_value,
						    my_value_is_empty,
						    my_value_to_string,
						    NULL);

	etmm = E_TREE_MEMORY(e_tree_model);

	adapter = E_TREE_TABLE_ADAPTER(e_tree_table_adapter_new(e_tree_model));

	e_tree_table_adapter_load_expanded_state (adapter, "expanded_state");

	/* create a root node with 5 children */
	root_node = e_tree_memory_node_insert (etmm, NULL,
					      0,
					      NULL);

	e_tree_table_adapter_root_node_set_visible (adapter, TRUE);

	for (i = 0; i < 5; i++){
		char *id = g_strdup_printf ("First level child %d", i);
		ETreePath n = e_tree_memory_node_insert (etmm,
							 root_node, 0,
							 id);
		for (j = 0; j < 5; j ++) {
			char *id = g_strdup_printf ("Second level child %d.%d", i, j);
			e_tree_memory_node_insert (etmm,
						   n, 0,
						   id);
		}
	}

	/*
	 * Here we create a window for our new table.  This window
	 * will get shown and the person will be able to test their
	 * item.
	 */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	/* This frame is simply to get a bevel around our table. */
	vbox = gtk_vbox_new (FALSE, 0);
	frame = gtk_frame_new (NULL);

	/*
	 * Here we create the table.  We give it the three pieces of
	 * the table we've created, the header, the model, and the
	 * initial layout.  It does the rest.
	 */
	e_table = e_table_new (E_TABLE_MODEL(adapter), NULL, INITIAL_SPEC, NULL);

	if (!e_table) printf ("BAH!");

	gtk_object_set (GTK_OBJECT (e_table),
			"cursor_mode", E_CURSOR_LINE,
			NULL);

	/* Build the gtk widget hierarchy. */
	gtk_container_add (GTK_CONTAINER (frame), e_table);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

	button = gtk_button_new_with_label ("Sort Children Ascending");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", sort_ascending, e_tree_model);
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Sort Children Descending");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", sort_descending, e_tree_model);
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	/* Size the initial window. */
	gtk_widget_set_usize (window, 200, 200);

	gtk_signal_connect (GTK_OBJECT (window), "delete-event", gtk_main_quit, NULL);

	/* Show it all. */
	gtk_widget_show_all (window);
}

/* This is the main function which just initializes gnome and call our create_tree function */

int
main (int argc, char *argv [])
{
	gnome_init ("TableExample", "TableExample", argc, argv);

	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	create_tree ();
	
	gtk_main ();

	return 0;
}

