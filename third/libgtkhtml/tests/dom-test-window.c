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

#include <gtkhtml.h>
#include "dom-test-window.h"
#include "dom-test-tree-model.h"
#include "dom-test-node-menu.h"


static const gchar simple_doc[] = "<html><body bgcolor=\"white\"><div>Foo bar</div><p style=\"background: green\">And here's some more text to see if line-wrapping works correctly. And here's some more text to see if line-wrapping works correctly.";


static void
dom_test_layout_tree_helper (HtmlBox *root, gint indent)
{
	HtmlBox *box;
	gint i;
	
	if (!root)
		return;

	box = root->children;


	for (i = 0; i < indent; i++)
		g_print (" ");

	g_print ("Type: %s (%p, %p, %p) (%d %d %d %d)\n",
		 G_OBJECT_TYPE_NAME (root), root, root->dom_node, HTML_BOX_GET_STYLE (root), root->x, root->y, root->width, root->height);

	while (box) {
		dom_test_layout_tree_helper (box, indent + 1);
		box = box->next;
	}

}

static void
dom_test_dump_layout_tree (gpointer action_callback, guint action, GtkWidget *widget)
{
	GtkItemFactory *item_factory = gtk_item_factory_from_widget (widget);
	HtmlView *view =  g_object_get_data (G_OBJECT (item_factory), "view");      

	dom_test_layout_tree_helper (view->root, 0);
}

static void
dom_test_force_relayout (gpointer action_callback, guint action, GtkWidget *widget)
{
	GtkItemFactory *item_factory = gtk_item_factory_from_widget (widget);
	HtmlView *view =  g_object_get_data (G_OBJECT (item_factory), "view");      

	html_box_set_unrelayouted_down (view->root);
	gtk_widget_queue_resize (GTK_WIDGET (view));
}

static void
dom_test_force_restyle (gpointer action_callbac, guint action, GtkWidget *widget)
{
	GtkItemFactory *item_factory = gtk_item_factory_from_widget (widget);
	HtmlView *view =  g_object_get_data (G_OBJECT (item_factory), "view");      
#if 0
	html_view_restyle (view, view->root);
	html_view_relayout (view);
#endif	
	gtk_widget_draw (GTK_WIDGET (view), NULL);

}


static GtkItemFactoryEntry menu_items[] =
{
	{ "/_File",                 NULL, NULL,                       0, "<Branch>" },
	{ "/File/_Quit",            NULL, (GtkItemFactoryCallback)gtk_main_quit,           0, "<StockItem>", GTK_STOCK_QUIT },
	{ "/_Debug",                NULL, NULL,                       0, "<Branch>" },
	{ "/Debug/Force re_layout", NULL, (GtkItemFactoryCallback)dom_test_force_relayout, 0, NULL },
	{ "/Debug/Force re_style",  NULL, (GtkItemFactoryCallback)dom_test_force_restyle,  0, NULL },
	{ "/Debug/Dump layout _tree", NULL, (GtkItemFactoryCallback)dom_test_dump_layout_tree, 0, NULL }
};

static gboolean
dom_test_tree_view_button_press (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
  GtkTreeIter iter;

  if (event->button == 3) {
	  
	  if (gtk_tree_selection_get_selected (sel, NULL, &iter)) {
		  GtkWidget *menu = dom_test_node_menu_new (DOM_TEST_WINDOW (data), DOM_NODE (iter.user_data));
		  
		  gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
				  NULL, NULL, 3, event->time);

		  return TRUE;
	  }
  }
  
  return FALSE;
}

static void
dom_test_window_class_init (DomTestWindowClass *klass)
{
}

static void
dom_test_window_init (DomTestWindow *doc)
{
}

GType
dom_test_window_get_type (void)
{
	static GType dom_test_window_type = 0;

	if (!dom_test_window_type) {
		static const GTypeInfo dom_test_window_info = {
			sizeof (DomTestWindowClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) dom_test_window_class_init,
			NULL, /* class_finalize */
			NULL, /* class_window */
			sizeof (DomTestWindow),
			16,   /* n_preallocs */
			(GInstanceInitFunc) dom_test_window_init,
		};

		dom_test_window_type = g_type_register_static (GTK_TYPE_WINDOW, "DomTestWindow", &dom_test_window_info, 0);
	}

	return dom_test_window_type;
}

#if 0
static void
dom_test_orphan_node_cell_data_func (GtkTreeViewColumn *column , GtkCellRenderer *cell, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	DomNode *node;
	gchar *str;
	static GdkPixbuf *elem_pixbuf = NULL, *text_pixbuf = NULL;
	GdkPixbuf *pixbuf;
	
	if (!elem_pixbuf) {
		elem_pixbuf = gdk_pixbuf_new_from_xpm_data (elem_xpm);
		text_pixbuf = gdk_pixbuf_new_from_xpm_data (text_xpm);
	}
	gtk_tree_model_get (tree_model, iter, 0, &node, -1);

	if (node) {
		switch (dom_Node__get_nodeType (node)) {
		case DOM_TEXT_NODE:
			pixbuf = text_pixbuf;
			str = dom_Node__get_nodeValue (node, NULL);
			break;
		case DOM_DOCUMENT_NODE:
		case DOM_ELEMENT_NODE:
		default:
			pixbuf = elem_pixbuf;
			str = dom_Node__get_nodeName (node);
			break;
		}
		
		
		g_object_set (G_OBJECT (cell),
			      "text", str,
			      "pixbuf", pixbuf,
			      NULL);

		if (str)
			g_free (str);
	}
}
#endif
  
void
dom_test_window_construct (DomTestWindow *window)
{
	GtkWidget *paned = gtk_hpaned_new ();
	GtkWidget *vpaned = gtk_vpaned_new ();
	GtkWidget *box1 = gtk_vbox_new (FALSE, 0);
	GtkWidget *frame, *sw;
	HtmlDocument *document = html_document_new ();
	GtkWidget *html_view = html_view_new ();
	GtkWidget *tree_view = gtk_tree_view_new ();
	GtkWidget *scrolled_window;
	GtkItemFactory *item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", NULL);
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;

	g_object_set_data (G_OBJECT (item_factory), "view", html_view);
	
	gtk_item_factory_create_items (item_factory, G_N_ELEMENTS (menu_items), menu_items, NULL);
	gtk_box_pack_start (GTK_BOX (box1),
			    gtk_item_factory_get_widget (item_factory, "<main>"),
			    FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (box1),
			    paned,
			    TRUE, TRUE, 0);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);

	gtk_paned_add1 (GTK_PANED (vpaned), scrolled_window);

	gtk_paned_add1 (GTK_PANED (paned), vpaned);
	
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
	gtk_paned_add2 (GTK_PANED (paned), frame);
	sw = gtk_scrolled_window_new (gtk_layout_get_hadjustment (GTK_LAYOUT (html_view)),
				      gtk_layout_get_vadjustment (GTK_LAYOUT (html_view)));
	gtk_container_add (GTK_CONTAINER (sw), html_view);

	gtk_container_add (GTK_CONTAINER (frame), sw);
	
	gtk_container_add (GTK_CONTAINER (window), box1);

	gtk_paned_set_position (GTK_PANED (paned), 200);


	html_view_set_document (HTML_VIEW (html_view), document);

	if (html_document_open_stream (document, "text/html")) {
		html_document_write_stream (document, simple_doc, sizeof (simple_doc));
		html_document_close_stream (document);
	}


	window->tree_model = dom_test_tree_model_new (DOM_NODE (document->dom_document), DOM_TEST_TREE_MODEL_TREE);
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (window->tree_model));
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "DOM Tree");

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", 0);
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer, "pixbuf", 1);

	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
	g_signal_connect (tree_view, "button_press_event",
			  G_CALLBACK (dom_test_tree_view_button_press), window);

	/* Create a new "fake" document for orphan nodes */
	window->orphan_root_node = dom_Node_mkref ((xmlNode *)xmlNewDoc (NULL));
	window->orphan_nodes = dom_test_tree_model_new (window->orphan_root_node, DOM_TEST_TREE_MODEL_TREE);
	
	tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (window->orphan_nodes));
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "DOM Tree");

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "text", 0);
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer, "pixbuf", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

	gtk_paned_set_position (GTK_PANED (vpaned), 250);
	
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
	gtk_paned_add2 (GTK_PANED (vpaned), scrolled_window);
}

GtkWidget *
dom_test_window_new (void)
{
	DomTestWindow *window = g_object_new (DOM_TYPE_TEST_WINDOW, NULL);

	dom_test_window_construct (window);
	
	return GTK_WIDGET (window);
}

