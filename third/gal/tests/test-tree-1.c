/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * test-tree-1.c
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

#include <gal/e-table/e-tree-memory-callbacks.h>
#include <gal/e-table/e-tree-memory.h>
#include <gal/e-table/e-tree.h>
#include <gal/e-table/e-tree-scrolled.h>

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

#define INITIAL_SPEC "<ETableSpecification cursor-mode=\"line\" selection-mode=\"browse\" draw-focus=\"true\">                    	       \
  <ETableColumn model_col=\"0\" _title=\"Subject\"   expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"tree-string\" compare=\"string\"/> \
  <ETableColumn model_col=\"1\" _title=\"Full Name\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\"      compare=\"string\"/> \
  <ETableColumn model_col=\"2\" _title=\"Email\"     expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\"      compare=\"string\"/> \
  <ETableColumn model_col=\"3\" _title=\"Date\"      expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\"      compare=\"string\"/> \
        <ETableState> \
	        <column source=\"0\"/> \
	        <column source=\"1\"/> \
	        <column source=\"2\"/> \
	        <column source=\"3\"/> \
	        <grouping></grouping>                                         \
        </ETableState> \
</ETableSpecification>"

GtkWidget *e_tree;

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

/* This function returns the number of columns in our ETreeModel. */
static int
my_col_count (ETreeModel *etm, void *data)
{
	return COLS;
}


/* This function returns the value at a particular point in our ETreeModel. */
static void *
my_value_at (ETreeModel *etm, ETreePath path, int col, void *model_data)
{
	switch (col) {
	case 0: return e_tree_memory_node_get_data (E_TREE_MEMORY(etm), path);
	case 1: return "Chris Toshok";
	case 2: return "toshok@helixcode.com";
	case 3: return "Jun 07 2000";
	default: return NULL;
	}
}

/* This function sets the value at a particular point in our ETreeModel. */
static void
my_set_value_at (ETreeModel *etm, ETreePath path, int col, const void *val, void *model_data)
{
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


/* This function duplicates the value passed to it. */
static void *
my_duplicate_value (ETreeModel *etm, int col, const void *value, void *data)
{
	return g_strdup (value);
}

/* This function frees the value passed to it. */
static void
my_free_value (ETreeModel *etm, int col, void *value, void *data)
{
	g_free (value);
}

/* This function creates an empty value. */
static void *
my_initialize_value (ETreeModel *etm, int col, void *data)
{
	return g_strdup ("");
}

/* This function reports if a value is empty. */
static gboolean
my_value_is_empty (ETreeModel *etm, int col, const void *value, void *data)
{
	return !(value && *(char *)value);
}

/* This function reports if a value is empty. */
static char *
my_value_to_string (ETreeModel *etm, int col, const void *value, void *data)
{
	return g_strdup(value);
}

static void
toggle_root (GtkButton *button, gpointer data)
{
	e_tree_root_node_set_visible (E_TREE(e_tree), !e_tree_root_node_is_visible (E_TREE(e_tree)));
}

static void
add_sibling (GtkButton *button, gpointer data)
{
	ETreeModel *e_tree_model = E_TREE_MODEL (data);
	ETreeMemory *etmm = data;
	ETreePath *selected_node;
	ETreePath *parent_node;

	selected_node = e_tree_get_cursor (E_TREE (e_tree));
	if (selected_node == NULL)
		return;

	parent_node = e_tree_model_node_get_parent (e_tree_model, selected_node);

	e_tree_memory_node_insert_before (etmm, parent_node,
					  selected_node,
					  g_strdup("User added sibling"));
}

static void
add_child (GtkButton *button, gpointer data)
{
	ETreeMemory *etmm = data;
	ETreePath *selected_node;

	selected_node = e_tree_get_cursor (E_TREE (e_tree));
	if (selected_node == NULL)
		return;

	e_tree_memory_node_insert (etmm, selected_node,
				   0,
				   g_strdup("User added child"));
}

static void
remove_node (GtkButton *button, gpointer data)
{
	ETreeMemory *etmm = data;
	char *str;
	ETreePath *selected_node;

	selected_node = e_tree_get_cursor (E_TREE (e_tree));

	if (selected_node == NULL)
		return;

	str = (char*)e_tree_memory_node_remove (etmm, selected_node);
	printf ("removed node %s\n", str);
	g_free (str);
}

static void
expand_all (GtkButton *button, gpointer data)
{
	ETreePath *selected_node;

	selected_node = e_tree_get_cursor (E_TREE (e_tree));
	if (selected_node == NULL)
		return;

	e_tree_node_set_expanded_recurse (E_TREE(e_tree), selected_node, TRUE);
}

static void
collapse_all (GtkButton *button, gpointer data)
{
	ETreePath *selected_node;

	selected_node = e_tree_get_cursor (E_TREE (e_tree));
	if (selected_node == NULL)
		return;

	e_tree_node_set_expanded_recurse (E_TREE(e_tree), selected_node, FALSE);
}

static void
print_tree (GtkButton *button, gpointer data)
{
	EPrintable *printable = e_tree_get_printable (E_TREE (e_tree));
	GnomePrintContext *gpc;

	gpc = gnome_print_context_new (gnome_printer_new_generic_ps ("tree-out.ps"));

	e_printable_print_page (printable, gpc, 8*72, 10*72, FALSE);

	gnome_print_context_close (gpc);
}

static void
save_state (GtkButton *button, gpointer data)
{
	e_tree_save_expanded_state (E_TREE(e_tree), "expanded_state");
	e_tree_save_state (E_TREE(e_tree), "header_state");
}

/* We create a window containing our new tree. */
static void
create_tree (void)
{
	GtkWidget *window, *frame, *button, *vbox, *scrolled;
	int i, j;
	ETreeModel *e_tree_model = NULL;
	ETreeMemory *etmm;
	ETreePath *root_node;

	/* here we create our model.  This uses the functions we defined
	   earlier. */
	e_tree_model = e_tree_memory_callbacks_new (my_icon_at,
						    my_col_count,

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

	/* create a root node with 5 children */
	root_node = e_tree_memory_node_insert (etmm, NULL,
					       0,
					       g_strdup("Root Node"));

	for (i = 0; i < 5; i++){
		char *id = g_strdup_printf ("First level child %d", i);
		ETreePath *n = e_tree_memory_node_insert_id (etmm,
							     root_node, 0,
							     g_strdup("First level of children"),
							     id);
		g_free (id);
		for (j = 0; j < 5; j ++) {
			char *id = g_strdup_printf ("Second level child %d", j);
			e_tree_memory_node_insert_id (etmm,
						      n, 0,
						      g_strdup("Second level of children"), id);
			g_free (id);
		}
	}

	/*
	 * Here we create a window for our new tree.  This window
	 * will get shown and the person will be able to test their
	 * item.
	 */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	/* This frame is simply to get a bevel around our tree. */
	vbox = gtk_vbox_new (FALSE, 0);
	frame = gtk_frame_new (NULL);

	/*
	 * Here we create the tree.  We give it the three pieces of
	 * the tree we've created, the header, the model, and the
	 * initial layout.  It does the rest.
	 */
	scrolled = e_tree_scrolled_new (e_tree_model, NULL, INITIAL_SPEC, NULL);
	e_tree = GTK_WIDGET(e_tree_scrolled_get_tree (E_TREE_SCROLLED(scrolled)));
	e_tree_load_state (E_TREE(e_tree), "header_state");

	e_tree_load_expanded_state (E_TREE(e_tree), "expanded_state");
	e_tree_root_node_set_visible (E_TREE(e_tree), FALSE);

	gtk_widget_set_usize (scrolled, 480, 150);

	if (!e_tree) printf ("BAH!");

	/* Build the gtk widget hierarchy. */
	gtk_container_add (GTK_CONTAINER (frame), scrolled);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

	button = gtk_button_new_with_label ("Toggle Root Node");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", toggle_root, e_tree_model);
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Add Sibling");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", add_sibling, e_tree_model);
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Add Child");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", add_child, e_tree_model);
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Remove Node");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", remove_node, e_tree_model);
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Expand All Below");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", expand_all, e_tree_model);
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Collapse All Below");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", collapse_all, e_tree_model);
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Print Tree");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", print_tree, e_tree_model);
	gtk_box_pack_end (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Save State");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", save_state, e_tree_model);
	gtk_box_pack_end (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	/* Size the initial window. */

	gtk_signal_connect (GTK_OBJECT (window), "delete-event", gtk_main_quit, NULL);

	/* Show it all. */
	gtk_widget_show_all (window);
}

/* This is the main function which just initializes gnome and call our create_tree function */

int
main (int argc, char *argv [])
{
	gnome_init ("TreeExample", "TreeExample", argc, argv);

	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	create_tree ();
	
	gtk_main ();

	return 0;
}

