/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-printer-selector.c: A simple Optionmenu for selecting printers
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
 *
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include <config.h>

#include <string.h>
#include <gtk/gtk.h>

#include "gnome-print-i18n.h"
#include "gpa-printer-selector.h"
#include "libgnomeprint/private/gnome-print-config-private.h"
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/private/gpa-list.h>
#include <libgnomeprint/private/gpa-printer.h>
#include <libgnomeprint/private/gpa-root.h>
#include <libgnomeprint/private/gpa-config.h>

static void gpa_printer_selector_class_init (GPAPrinterSelectorClass *klass);
static void gpa_printer_selector_init (GPAPrinterSelector *selector);
static void gpa_printer_selector_finalize (GObject *object);

static gboolean gpa_printer_selector_construct (GPAWidget *gpa);
static void gpa_printer_selector_sync_printer (GtkListStore *model, 
					       GtkTreeIter  *iter, 
					       GPAPrinter   *printer);
static void selection_changed_cb (GtkTreeSelection *selection, 
				  gpointer          data);
static void set_printer_icon (GtkCellLayout   *layout, 
			      GtkCellRenderer *rend,
			      GtkTreeModel    *model, 
			      GtkTreeIter     *iter,
			      gpointer         data);
static void set_printer_name (GtkCellLayout   *layout, 
			      GtkCellRenderer *rend,
			      GtkTreeModel    *model, 
			      GtkTreeIter     *iter,
			      gpointer         data);
static void set_printer_state (GtkCellLayout   *layout, 
			       GtkCellRenderer *rend,
			       GtkTreeModel    *model, 
			       GtkTreeIter     *iter,
			       gpointer         data);
static void set_printer_jobs (GtkCellLayout   *layout, 
			      GtkCellRenderer *rend,
			      GtkTreeModel    *model, 
			      GtkTreeIter     *iter,
			      gpointer         data);
static void set_printer_location (GtkCellLayout   *layout, 
				  GtkCellRenderer *rend,
				  GtkTreeModel    *model, 
				  GtkTreeIter     *iter,
				  gpointer         data);
static int printer_sort_func (GtkTreeModel *model,
			      GtkTreeIter *a,
			      GtkTreeIter *b,
			      gpointer data);



static GPAWidgetClass *parent_class;

GType
gpa_printer_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAPrinterSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gpa_printer_selector_class_init,
			NULL, NULL,
			sizeof (GPAPrinterSelector),
			0,
			(GInstanceInitFunc) gpa_printer_selector_init
		};
		type = g_type_register_static (GPA_TYPE_WIDGET, "GPAPrinterSelector", &info, 0);
	}
	return type;
}

static void
gpa_printer_selector_class_init (GPAPrinterSelectorClass *klass)
{
	GObjectClass *object_class;
	GPAWidgetClass *gpa_class;

	object_class = (GObjectClass *) klass;
	gpa_class = (GPAWidgetClass *) klass;

	parent_class = gtk_type_class (GPA_TYPE_WIDGET);
	gpa_class->construct = gpa_printer_selector_construct;
	object_class->finalize = gpa_printer_selector_finalize;
}

static void
gpa_printer_selector_init (GPAPrinterSelector *ps)
{
	GtkCellRenderer *rend;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *col;
	GtkWidget *scrolledwin;
	
	scrolledwin = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwin);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwin),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwin),
					     GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (ps), scrolledwin);

	ps->model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_POINTER));
	ps->sortmodel = gtk_tree_model_sort_new_with_model (ps->model);
	gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (ps->sortmodel),
						 printer_sort_func,
						 ps, NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (ps->sortmodel),
					      GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
					      GTK_SORT_ASCENDING);
	
	ps->treeview = gtk_tree_view_new_with_model (ps->sortmodel);
	gtk_container_add (GTK_CONTAINER (scrolledwin), ps->treeview);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (ps->treeview));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
	g_signal_connect (selection, "changed", G_CALLBACK (selection_changed_cb), ps);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (ps->treeview), TRUE);

	rend = gtk_cell_renderer_pixbuf_new ();
	col = gtk_tree_view_column_new_with_attributes ("", rend, NULL);	
	gtk_tree_view_column_set_cell_data_func (col, rend, 
						 (GtkTreeCellDataFunc)set_printer_icon, 
						 NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (ps->treeview), col);
	
	rend = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Printer"), rend, NULL);	
	gtk_tree_view_column_set_cell_data_func (col, rend, 
						 (GtkTreeCellDataFunc)set_printer_name, 
						 NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (ps->treeview), col);

	rend = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("State"), rend, NULL);	
	gtk_tree_view_column_set_cell_data_func (col, rend, 
						 (GtkTreeCellDataFunc)set_printer_state, 
						 NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (ps->treeview), col);

	rend = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Jobs"), rend, NULL);	
	gtk_tree_view_column_set_cell_data_func (col, rend, 
						 (GtkTreeCellDataFunc)set_printer_jobs, 
						 NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (ps->treeview), col);

	rend = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Location"), rend, NULL);	
	gtk_tree_view_column_set_cell_data_func (col, rend, 
						 (GtkTreeCellDataFunc)set_printer_location, 
						 NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (ps->treeview), col);
	
	gtk_widget_show (ps->treeview);
}

static void
gpa_printer_selector_finalize (GObject *object)
{
	GPAPrinterSelector *ps;

	ps = (GPAPrinterSelector *) object;

	gpa_node_unref (ps->node);
	ps->node = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
node_to_iter (GtkTreeModel *model, GPANode *node, GtkTreeIter *iter)
{
	if (!gtk_tree_model_get_iter_first (model, iter))
		return FALSE;
	do {
		GPANode *tem;
		gtk_tree_model_get (model, iter, 0, &tem, -1);
		if (tem == node)
			return TRUE;
	} while (gtk_tree_model_iter_next (model, iter));
	return FALSE;
}

static gboolean
printer_has_queue (GPAPrinter *printer)
{
	GPANode *settings;
	char * backend;
	gboolean ret;

	settings = gpa_printer_get_default_settings (GPA_PRINTER (printer));
	backend = gpa_node_get_path_value (settings, "Transport.Backend");
	ret = strcmp (backend, "file") && strcmp (backend, "lpr") && strcmp (backend, "custom");
	g_free (backend);
	return ret;
}

static int
printer_sort_func (GtkTreeModel *model,
		   GtkTreeIter *iter_a,
		   GtkTreeIter *iter_b,
		   gpointer data)
{
	GPANode *a, *b;
	gboolean a_has_queue, b_has_queue;

	gtk_tree_model_get (model, iter_a, 0, &a, -1);
	gtk_tree_model_get (model, iter_b, 0, &b, -1);

	if (a == NULL
	    || b == NULL)
		return 0;
	
	a_has_queue = printer_has_queue (GPA_PRINTER (a));
	b_has_queue = printer_has_queue (GPA_PRINTER (b));
	if (!a_has_queue && b_has_queue)
	    return -1;
	else if (a_has_queue && !b_has_queue)
		return 1;
	else {
		int ret;
		char *a_name, *b_name;
		a_name = gpa_node_get_value (a);
		b_name = gpa_node_get_value (b);
		ret = strcmp (a_name, b_name);
		g_free (a_name);
		g_free (b_name);
		return ret;
	}
}

void
gpa_printer_selector_printer_state_changed (GPAPrinterSelector *selector,
					    GPANode *printer)
{
	GtkTreeIter iter;

	g_return_if_fail (node_to_iter (selector->model, printer, &iter));

	gpa_printer_selector_sync_printer (GTK_LIST_STORE (selector->model),
					   &iter,
					   GPA_PRINTER (printer));
}

static void
selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
	GPAPrinterSelector *selector;
	GtkTreeIter sort_iter;
	GtkTreeIter iter;
	GPANode *node;

	selector = GPA_PRINTER_SELECTOR (data);
	if (!gtk_tree_selection_get_selected (selection, NULL, &sort_iter))
		return;
	gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT (selector->sortmodel),
							&iter, &sort_iter);
	gtk_tree_model_get (selector->model, &iter, 0, &node, -1);

	gpa_node_set_path_value (selector->config, "Printer", gpa_node_id (node));
}

/**
 * get_printer_icon:
 * @printer: 
 * 
 * Should be inside gpa (Chema) as gpa_printer_get_icon
 * 
 * Return Value: 
 **/
static GdkPixbuf *
get_printer_icon (GPANode *printer)
{
	GtkIconTheme *theme = gtk_icon_theme_get_default ();
	GPANode *settings_list;
	GPANode *settings;
	GdkPixbuf *res = NULL;
	gchar *icon_name;

	settings_list = gpa_node_get_child_from_path (printer, "Settings");
	settings = gpa_list_get_default (GPA_LIST (settings_list));
	icon_name = gpa_node_get_path_value (settings, "Icon.Filename");
	if (icon_name != NULL)
		res = gtk_icon_theme_load_icon (theme, icon_name,
						18, 0, NULL);
	if (res == NULL)
		res = gtk_icon_theme_load_icon (theme, "gnome-dev-printer",
						18, 0, NULL);
	/* #warning TODO : allow for transport specific icons to get things like local vs network printing */
	if (res == NULL)
		res = gtk_icon_theme_load_icon (theme, GTK_STOCK_MISSING_IMAGE,
			48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	return res;
}

static void
set_printer_icon (GtkCellLayout   *layout, 
		  GtkCellRenderer *rend,
		  GtkTreeModel    *model, 
		  GtkTreeIter     *iter,
		  gpointer         data)
{
	GPANode *node;
	GdkPixbuf *pixbuf;
	
	gtk_tree_model_get (model, iter, 0, &node, -1);
	pixbuf = get_printer_icon (node);
	g_object_set (rend, "pixbuf", pixbuf, NULL);
	g_object_unref (pixbuf);
}

static void
set_printer_name (GtkCellLayout   *layout, 
		  GtkCellRenderer *rend,
		  GtkTreeModel    *model, 
		  GtkTreeIter     *iter,
		  gpointer         data)
{
	GPANode *node;
	gchar *text;

	gtk_tree_model_get (model, iter, 0, &node, -1);
	text = gpa_node_get_value (node);
	g_object_set (rend, "text", text, NULL);
	g_free (text);
}

static void
set_printer_state (GtkCellLayout   *layout, 
		   GtkCellRenderer *rend,
		   GtkTreeModel    *model, 
		   GtkTreeIter     *iter,
		   gpointer         data)
{
	GPANode *node;
	GPANode *queue_state;
	gchar *text;

	gtk_tree_model_get (model, iter, 0, &node, -1);

	queue_state = gpa_printer_get_state_by_id (GPA_PRINTER (node), "PrinterState");
	if (queue_state && printer_has_queue (GPA_PRINTER (node)))
		text = gpa_node_get_value (queue_state);
	else
		text = g_strdup ("");
	g_object_set (rend, "text", text, NULL);
	g_free (text);
}

static void
set_printer_jobs (GtkCellLayout   *layout, 
		  GtkCellRenderer *rend,
		  GtkTreeModel    *model, 
		  GtkTreeIter     *iter,
		  gpointer         data)
{
	GPANode *node;
	GPANode *queue_state;
	gchar *text;

	gtk_tree_model_get (model, iter, 0, &node, -1);
	queue_state = gpa_printer_get_state_by_id (GPA_PRINTER (node), "QueueLength");
	if (queue_state && printer_has_queue (GPA_PRINTER (node)))
		{
			text = gpa_node_get_value (queue_state);
			if (text[0] == '0')
				{
					g_free (text);
					text = g_strdup ("");
				}
		}
	else
		text = g_strdup ("");
	g_object_set (rend, "text", text, NULL);
	g_free (text);
}

static void
set_printer_location (GtkCellLayout   *layout, 
		      GtkCellRenderer *rend,
		      GtkTreeModel    *model, 
		      GtkTreeIter     *iter,
		      gpointer         data)
{
	GPANode *node;
	GPANode *location;
	gchar *text;

	gtk_tree_model_get (model, iter, 0, &node, -1);
	location = gpa_printer_get_state_by_id (GPA_PRINTER (node), "Location");
	if (location && printer_has_queue (GPA_PRINTER (node)))
		text = gpa_node_get_value (location);
	else
		text = g_strdup ("");
	g_object_set (rend, "text", text, NULL);
	g_free (text);
}

static void
gpa_printer_selector_sync_printer (GtkListStore *model,
				   GtkTreeIter *iter,
				   GPAPrinter *printer)
{
	gtk_list_store_set (GTK_LIST_STORE (model), iter, 0, printer, -1);
}

static void
gpa_printer_selector_printer_added_cb (GPANode *parent, GPANode *child,
				       GPAPrinterSelector *ps)
{
	GtkTreeIter iter;

	GDK_THREADS_ENTER ();

	g_return_if_fail (node_to_iter (ps->model, child, &iter) == FALSE);

	gtk_list_store_append (GTK_LIST_STORE (ps->model), &iter);
	gpa_printer_selector_sync_printer (GTK_LIST_STORE (ps->model), &iter,
					   GPA_PRINTER (child));

	GDK_THREADS_LEAVE ();
}

static void
gpa_printer_selector_printer_removed_cb (GPANode *parent, GPANode *child,
					 GPAPrinterSelector *ps)
{
	GtkTreeIter iter;

	GDK_THREADS_ENTER ();
	
	g_return_if_fail (node_to_iter (ps->model, child, &iter));
	
	gtk_list_store_remove (GTK_LIST_STORE (ps->model), &iter);

	GDK_THREADS_LEAVE ();
}

static gboolean
gpa_printer_selector_construct (GPAWidget *gpa)
{
	GPAPrinterSelector *ps;
	GPANode *child;
	GtkTreeIter iter;
	GtkTreeIter sort_iter;
	GPANode *default_printer = NULL;
	GtkTreeSelection *selection;

	ps = GPA_PRINTER_SELECTOR (gpa);
	ps->config = GNOME_PRINT_CONFIG_NODE (gpa->config);
	ps->node   = GPA_NODE (gpa_get_printers ());
	g_signal_connect_object (ps->node, "child-added",
				 G_CALLBACK (gpa_printer_selector_printer_added_cb),
				 ps, 0);
	g_signal_connect_object (ps->node, "child-removed",
				 G_CALLBACK (gpa_printer_selector_printer_removed_cb),
				 ps, 0);

	child = gpa_node_get_child (ps->node, NULL);
	while (child) {
		GtkTreeIter iter;
		gtk_list_store_append (GTK_LIST_STORE (ps->model), &iter);
		gpa_printer_selector_sync_printer (GTK_LIST_STORE (ps->model),
						   &iter,
						   GPA_PRINTER (child));
		child = gpa_node_get_child (ps->node, child);
	}

	if (ps->config != NULL)
		default_printer = GPA_REFERENCE_REFERENCE (GPA_CONFIG (ps->config)->printer);

	if (default_printer != NULL &&
	    node_to_iter (ps->model, default_printer, &iter), FALSE) {
		gtk_tree_model_sort_convert_child_iter_to_iter (
			GTK_TREE_MODEL_SORT (ps->sortmodel), &sort_iter, &iter);
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (ps->treeview));
		gtk_tree_selection_select_iter (selection, &sort_iter);
	}

	return TRUE;
}

