/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * test-table-1.c
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
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
#include "gal/widgets/e-cursors.h"
#include "gal/e-table/e-table-simple.h"
#include "gal/e-table/e-table.h"
#include "gal/util/e-i18n.h"

#include <libgnomeui/libgnomeui.h>

/*
 * One way in which we make it simpler to build an ETableModel is through
 * the ETableSimple class.  Instead of creating your own ETableModel
 * class, you simply create a new object of the ETableSimple class.  You
 * give it a bunch of functions that act as callbacks.
 * 
 * You also get to pass a void * to ETableSimple and it gets passed to
 * your callbacks.  This would be for having multiple models of the same
 * type.  This is just an example though, so we statically define all the
 * data and ignore the void *data parameter.
 * 
 * In our example we will be creating a table model with 4 columns and 10
 * rows.  This corresponds to having 4 different types of information and
 * 10 different sets of data in our database.
 * 
 * The headers will be hard coded, as will be the example data.
 *
 */

/*
 * There are two different meanings to the word "column".  The first is
 * the model column.  A model column corresponds to a specific type of
 * data.  This is very much like the usage in a database table where a
 * column is a field in the database.
 *
 * The second type of column is a view column.  A view column
 * corresponds to a visually displayed column.  Each view column
 * corresponds to a specific model column, though a model column may
 * have any number of view columns associated with it, from zero to
 * greater than one.
 *
 * Also, a view column doesn't necessarily depend on only one model
 * column.  In some cases, the view column renderer can be given a
 * reference to another column to get extra information about its
 * display.
*/

#define ROWS 10
#define COLS 4

#if 0 /* For translators */
char *headers [COLS] = {
  N_("Email"),
  N_("Full Name"),
  N_("Address"),
  N_("Phone"),
};
#endif

#define SPEC "<ETableSpecification cursor-mode=\"line\" selection-mode=\"browse\" draw-focus=\"true\" allow-grouping=\"false\">				\
  <ETableColumn model_col=\"0\" _title=\"Email\"   expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\"     compare=\"string\"/>	\
  <ETableColumn model_col=\"1\" _title=\"Full Name\" expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\"   compare=\"string\"/>	\
  <ETableColumn model_col=\"2\" _title=\"Address\"     expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\" compare=\"string\"/>	\
  <ETableColumn model_col=\"3\" _title=\"Phone\"      expansion=\"1.0\" minimum_width=\"20\" resizable=\"true\" cell=\"string\"  compare=\"string\"/>	\
        <ETableState>																	\
	        <column source=\"0\"/>															\
	        <column source=\"1\"/>															\
	        <column source=\"2\"/>															\
	        <column source=\"3\"/>															\
	        <grouping> <leaf column=\"1\" ascending=\"true\"/> </grouping>										\
        </ETableState>																	\
</ETableSpecification>"

/*
 * Virtual Column list:
 * 0   Email
 * 1   Full Name
 * 2   Address
 * 3   Phone
 */

char *table_data [ROWS] [COLS];

/*
 * ETableSimple callbacks
 * These are the callbacks that define the behavior of our custom model.
 */

/*
 * Since our model is a constant size, we can just return its size in
 * the column and row count fields.
 */

/* This function returns the number of columns in our ETableModel. */
static int
my_col_count (ETableModel *etc, void *data)
{
	return COLS;
}

/* This function returns the number of rows in our ETableModel. */
static int
my_row_count (ETableModel *etc, void *data)
{
	return ROWS;
}

/* This function returns the value at a particular point in our ETableModel. */
static void *
my_value_at (ETableModel *etc, int col, int row, void *data)
{
	return (void *) table_data [row] [col];
}

/* This function sets the value at a particular point in our ETableModel. */
static void
my_set_value_at (ETableModel *etc, int col, int row, const void *val, void *data)
{
	g_free (table_data [row] [col]);
	table_data [row] [col] = g_strdup (val);
}

/* This function returns whether a particular cell is editable. */
static gboolean
my_is_cell_editable (ETableModel *etc, int col, int row, void *data)
{
	return TRUE;
}

/* This function duplicates the value passed to it. */
static void *
my_duplicate_value (ETableModel *etc, int col, const void *value, void *data)
{
	return g_strdup (value);
}

/* This function frees the value passed to it. */
static void
my_free_value (ETableModel *etc, int col, void *value, void *data)
{
	g_free (value);
}

/* This function creates an empty value. */
static void *
my_initialize_value (ETableModel *etc, int col, void *data)
{
	return g_strdup ("");
}

/* This function reports if a value is empty. */
static gboolean
my_value_is_empty (ETableModel *etc, int col, const void *value, void *data)
{
	return !(value && *(char *)value);
}

/* This function reports if a value is empty. */
static char *
my_value_to_string (ETableModel *etc, int col, const void *value, void *data)
{
	return g_strdup(value);
}

static gchar *
make_rand_str (void)
{
	static int i = 0;

	return g_strdup_printf ("%d%2x", i++, rand ());
}

static void
weak_ref_func (gpointer data, GObject *where_object_was)
{
	gtk_main_quit();
}

/* We create a window containing our new table. */
static void
create_table (void)
{
	GtkWidget *e_table, *window, *frame;
	int i, j;
	ETableModel *e_table_model = NULL;

	/* First we fill in the simple data. */
	for (i = 0; i < ROWS; i++){
		for (j = 0; j < COLS; j++)
			table_data [i] [j] = make_rand_str ();
	}
	/* Next we create our model.  This uses the functions we defined
	   earlier. */
	e_table_model = e_table_simple_new (my_col_count, my_row_count, NULL,

					    my_value_at, my_set_value_at, my_is_cell_editable,

					    NULL, NULL,
					    my_duplicate_value, my_free_value, 
					    my_initialize_value, my_value_is_empty,
					    my_value_to_string,
					    NULL);
		
	/*
	 * Here we create a window for our new table.  This window
	 * will get shown and the person will be able to test their
	 * item.
	 */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	/* This frame is simply to get a bevel around our table. */
	frame = gtk_frame_new (NULL);

	/*
	 * Here we create the table.  We give it the three pieces of
	 * the table we've created, the header, the model, and the
	 * initial layout.  It does the rest.
	 */
	e_table = e_table_new (e_table_model, NULL, SPEC, NULL);

	/* Build the gtk widget hierarchy. */
	gtk_container_add (GTK_CONTAINER (frame), e_table);
	gtk_container_add (GTK_CONTAINER (window), frame);

	/* Size the initial window. */
	gtk_widget_set_usize (window, 200, 200);

	/* Show it all. */
	gtk_widget_show_all (window);

	g_object_weak_ref (window, weak_ref_func, NULL);
}

/* This is the main function which just initializes gnome and call our create_table function */

int
main (int argc, char *argv [])
{
	gnome_program_init ("TableExample", "1.0",
			    LIBGNOMEUI_MODULE,
			    argc, argv, NULL);

	e_cursors_init ();

	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	create_table ();
	
	gtk_main ();

	e_cursors_shutdown ();
	return 0;
}
