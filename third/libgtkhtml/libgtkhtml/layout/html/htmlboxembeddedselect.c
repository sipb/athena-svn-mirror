/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000-2001 CodeFactory AB
   Copyright (C) 2000-2001 Jonas Borgström <jonas@codefactory.se>
   Copyright (C) 2000-2001 Anders Carlsson <andersca@codefactory.se>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <string.h>
#include <stdlib.h>
#include "layout/html/htmlboxembeddedselect.h"
#include "dom/html/dom-htmlselectelement.h"
#include "dom/html/dom-htmloptionelement.h"

static HtmlBoxClass *parent_class = NULL;

static gint combo_selected = 0;

static gboolean 
create_list_foreach (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	GList **list = (GList **)data;
	DomHTMLOptionElement *element;
	GValue value = { 0, };
	gchar *str;

	gtk_tree_model_get_value (model, iter, 0, &value);
	gtk_tree_model_get (model, iter, 2, &element, -1);

	g_assert (G_VALUE_HOLDS_STRING(&value));
	str = g_strdup (g_value_get_string (&value));
	g_strchug (str);

	*list = g_list_append (*list, str);

	if (dom_HTMLOptionElement__get_defaultSelected (element)) {
		combo_selected = g_list_length (*list) - 1;
	}
	g_value_unset (&value);

	return FALSE;
}

static void
update_combo_list (GtkTreeModel *model, GtkWidget *widget)
{
	GList *list = NULL;

	gtk_tree_model_foreach (model, create_list_foreach, &list);
	if (list)
		gtk_combo_set_popdown_strings (GTK_COMBO (widget), list);
#ifdef GTK_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED
	gtk_list_select_item (GTK_LIST (GTK_COMBO (widget)->list),
			      combo_selected);
#define GTK_DISABLE_DEPRECATED
#else
	gtk_list_select_item (GTK_LIST (GTK_COMBO (widget)->list),
			      combo_selected);
#endif
	g_list_foreach (list, (GFunc)g_free, NULL);
	g_list_free (list);

}

static void
html_box_embedded_select_relayout (HtmlBox *box, HtmlRelayout *relayout) 
{
	DomHTMLSelectElement *select_node = DOM_HTML_SELECT_ELEMENT (box->dom_node);
	GtkTreeModel *model = dom_html_select_element_get_tree_model (select_node);
	HtmlStyle *style = HTML_BOX_GET_STYLE (box);
	GtkWidget *widget = HTML_BOX_EMBEDDED (box)->widget;
	HtmlBoxEmbeddedSelect *select = HTML_BOX_EMBEDDED_SELECT (box);

	if (dom_HTMLSelectElement__get_multiple (select_node) == FALSE &&
	    dom_HTMLSelectElement__get_size (select_node) == 1) {

		if (select->combo_up_to_date == FALSE) {

			update_combo_list (model, widget);
			select->combo_up_to_date = TRUE;
		}

		if (style->box->width.type == HTML_LENGTH_AUTO) {
			GtkRequisition requisition;
			GtkRequisition requisition2;
			
			gtk_widget_size_request (GTK_COMBO(widget)->list, &requisition);
			gtk_widget_size_request (GTK_COMBO(widget)->button, &requisition2);
			gtk_widget_set_usize (widget, requisition.width + requisition2.width + 5, -1);
		}
	}
	else {
		/* FIXME: This is not correct... */
		gtk_widget_set_usize (widget, -1, (1 + style->inherited->font_spec->size) * 
				      dom_HTMLSelectElement__get_size (select_node) + 5);
	}
}

static gboolean 
treeview_select_default (GtkTreeModel *model, GtkTreePath *path,
			 GtkTreeIter *iter, gpointer data)
{
	DomHTMLOptionElement *element;
	GtkTreeView *treeview = GTK_TREE_VIEW (data);

	gtk_tree_model_get (model, iter, 2, &element, -1);

	if (dom_HTMLOptionElement__get_defaultSelected (element)) {
		gtk_tree_selection_select_iter (gtk_tree_view_get_selection (treeview), iter);
		gtk_tree_view_scroll_to_cell (treeview, path, NULL, TRUE,
					      0.5, 0.0);
	}

	return FALSE;
}

static void
update_treeview_selection (GtkWidget *widget, GtkRequisition *req,
			   gpointer data)
{
	DomHTMLSelectElement *select_node;
	GtkTreeModel *model;
	GtkTreeView *treeview;

	select_node  = DOM_HTML_SELECT_ELEMENT (data);
	treeview = GTK_TREE_VIEW (widget);
	model = dom_html_select_element_get_tree_model (select_node);

	gtk_tree_model_foreach (model, treeview_select_default, treeview);
}

static void
create_treeview_widget (HtmlBoxEmbedded *embedded, DomHTMLSelectElement *select_node)
{
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	GtkWidget *treeview;

	html_box_embedded_set_widget (embedded, gtk_scrolled_window_new (NULL, NULL));

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (embedded->widget),
					GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (embedded->widget),
					     GTK_SHADOW_IN);

	treeview = gtk_tree_view_new_with_model (dom_html_select_element_get_tree_model (select_node));

	if (dom_HTMLSelectElement__get_multiple (select_node))
		gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)), GTK_SELECTION_MULTIPLE);

	gtk_container_add (GTK_CONTAINER (embedded->widget), treeview);
	gtk_widget_show (treeview);

	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Select", cell, "text", 0, NULL);

	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);

	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (column));

	g_signal_connect (G_OBJECT (treeview), "size_request",
			  G_CALLBACK (update_treeview_selection), select_node);
}

static void
row_changed_callback (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
	       HtmlBoxEmbeddedSelect *select)
{
	select->combo_up_to_date = FALSE;
}

static void
create_combo_widget (HtmlBoxEmbedded *embedded, DomHTMLSelectElement *select_node)
{
	GtkTreeModel *model = dom_html_select_element_get_tree_model (select_node);

	html_box_embedded_set_widget (embedded, gtk_combo_new ());
	update_combo_list (model, embedded->widget);

	g_signal_connect (G_OBJECT (model), "row_changed", 
			  (GCallback) row_changed_callback, embedded);
}

static void
html_box_embedded_select_finalize (GObject *object)
{
	HtmlBox *box = HTML_BOX (object);
	DomHTMLSelectElement *select_node;

	if (box->dom_node) {
		GtkTreeModel *model;
		select_node = DOM_HTML_SELECT_ELEMENT (box->dom_node);
		model = dom_html_select_element_get_tree_model (select_node);

		if (dom_HTMLSelectElement__get_multiple (select_node) == FALSE &&
	            dom_HTMLSelectElement__get_size (select_node) == 1) {

			g_signal_handlers_disconnect_matched (G_OBJECT (model),
							      G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
							      0, 0, NULL, row_changed_callback, box);
		}
	}
	G_OBJECT_CLASS(parent_class)->finalize (object);
}

static void
html_box_embedded_select_class_init (HtmlBoxClass *klass)
{
	GObjectClass *object_class = (GObjectClass *)klass;

	klass->relayout = html_box_embedded_select_relayout;
	object_class->finalize = html_box_embedded_select_finalize;

	parent_class = g_type_class_peek_parent (klass);
}

static void
html_box_embedded_select_init (HtmlBoxEmbeddedSelect *select)
{
}

GType
html_box_embedded_select_get_type (void)
{
	static GType html_type = 0;
	
	if (!html_type) {
		static GTypeInfo type_info = {
			sizeof (HtmlBoxEmbeddedSelectClass),
			NULL,
			NULL,
			(GClassInitFunc) html_box_embedded_select_class_init,		       
			NULL,
			NULL,
			sizeof (HtmlBoxEmbeddedSelect),
			16, 
			(GInstanceInitFunc) html_box_embedded_select_init
		};
		html_type = g_type_register_static (HTML_TYPE_BOX_EMBEDDED, "HtmlBoxEmbeddedSelect", &type_info, 0);
	}
	return html_type;
}

HtmlBox *
html_box_embedded_select_new (HtmlView *view, DomNode *node)
{
	HtmlBoxEmbeddedSelect *result;
	HtmlBoxEmbedded *embedded;
	DomHTMLSelectElement *select_node = DOM_HTML_SELECT_ELEMENT (node);

	result = g_object_new (HTML_TYPE_BOX_EMBEDDED_SELECT, NULL);
	embedded = HTML_BOX_EMBEDDED (result);
	
	html_box_embedded_set_view (embedded, view);

	if (dom_HTMLSelectElement__get_multiple (select_node) ||
	    dom_HTMLSelectElement__get_size (select_node) > 1)
		create_treeview_widget (embedded, select_node);
	else
		create_combo_widget (embedded, select_node);

	html_box_embedded_set_descent (HTML_BOX_EMBEDDED (result), 4);

	return HTML_BOX (result);
}


