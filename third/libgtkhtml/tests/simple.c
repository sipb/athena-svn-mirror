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

#include <gtk/gtk.h>
#include <libxml/debugXML.h>
#include <string.h>

#include "gtkhtmlcontext.h"
#include "graphics/htmlpainter.h"
#include "layout/htmlbox.h"
#include "view/htmlview.h"

#include "dom/dom-types.h"

HtmlDocument *document;
HtmlParser *parser;
GtkWidget *view;

typedef struct {
	gchar *title;
	gchar *filename;
} Doc;

Doc documents[] = {
	{ "alex test", "alex.html" },
	{ "andersca test", "andersca.html" },
	{ "jborg test", "jborg.html" },
	{ "floats test", "floats.html" },
	{ "table test", "tables.html" },
	{ "table stresstest", "table.html" },
	{ "position test", "position.html" },
	{ "Large file", "gtkwidget.html" },
	{ "Status", NULL },
	{ "Testcases", NULL }
};

Doc status_documents [] = {
	{ "HTML", "status/html.html" },
	{ "XML", "status/xml.html" },
	{ "DOM", "status/dom.html" },
	{ "CSS", "css-support.html" },
	{ "DBaron", "dbaron-status.html" },
};

typedef struct _FetchContext FetchContext;

struct _FetchContext {
	HtmlStream *stream;
	gchar *url;
};

enum {
	TITLE_COLUMN,
	FILENAME_COLUMN,
	NUM_COLUMNS
};

static void
add_status_docs (GtkTreeStore *model, GtkTreeIter *parent)
{
	GtkTreeIter iter;
	gint i;
	
	for (i = 0; i < G_N_ELEMENTS (status_documents); i++) {
		gtk_tree_store_append (GTK_TREE_STORE (model), &iter, parent);
		
		gtk_tree_store_set (GTK_TREE_STORE (model),
				    &iter,
				    TITLE_COLUMN, status_documents[i].title,
				    FILENAME_COLUMN, status_documents[i].filename,
				    -1);
	}
}

static gboolean
url_requested_timeout (FetchContext *context)
{
	gchar *file = NULL;
	gchar *path;
	
	if (g_path_is_absolute (context->url))
		path = g_strdup (context->url);
	else
		path = g_strdup_printf (GTKHTML_SAMPLES_DIRECTORY"/%s", context->url);

	
	if (g_file_test (path, G_FILE_TEST_EXISTS)) {
		gint i;
		guchar buf[4096];
		FILE *f = fopen (path, "r");
		
		while ((i = fread (buf, 1, 4096, f)) != 0) {
			g_print ("i: %d\n", i);
			html_stream_write (context->stream, buf, i);
			
			if (gtk_events_pending ())
				gtk_main_iteration ();
			
		}

		g_warning ("time to close!\n");
		html_stream_close (context->stream);
		fclose (f);
			
	}
	else {
		g_print ("eeek, wrong!\n");
		html_stream_close (context->stream);
	}
	
	g_free (file);
	g_free (context->url);
	g_free (context);
	
	return FALSE;
}

static gboolean
dom_mouse_down (HtmlDocument *doc, DomMouseEvent *event, gpointer data)
{
  /*	g_print ("mouse down!\n"); */

	return FALSE;
}

static gboolean
dom_mouse_up (HtmlDocument *doc, DomMouseEvent *event, gpointer data)
{
  /*	g_print ("mouse up!\n"); */

	return FALSE;
}

static gboolean
dom_mouse_click (HtmlDocument *doc, DomMouseEvent *event, gpointer data)
{
	g_print ("mouse click.!\n");

	return FALSE;
}

static gboolean
dom_mouse_over (HtmlDocument *doc, DomMouseEvent *event, gpointer data)
{
	g_print ("mouse over!\n");

	return FALSE;
}

static gboolean
dom_mouse_out (HtmlDocument *doc, DomMouseEvent *event, gpointer data)
{
	g_print ("mouse out!\n");

	return FALSE;
}

static void
link_clicked (HtmlDocument *doc, const gchar *url)
{
	g_print ("link clicked: %s!\n", url);
}

static gboolean
url_requested (HtmlDocument *doc, const gchar *url, HtmlStream *stream)
{
	FetchContext *context = g_new (FetchContext, 1);
	
	context->stream = stream;
	context->url = g_strdup (url);

	g_print ("URL IS REQUESTED!!!!!!!\n");
	g_print ("context is: %s\n", url);
	
	g_timeout_add (200, (GtkFunction)url_requested_timeout, context);
	
	return TRUE;
}

static void
load_file (const gchar *filename)
{
	FILE *file;
	/* FIXME: loading of bigger files */
	gchar buffer[300000];
	gint i;
	gdouble elapsed_time;
	gchar *path;
  	GTimer *timer;

	if (!filename)
		return;

	path = g_strdup_printf (GTKHTML_SAMPLES_DIRECTORY"/%s", filename);
	timer = g_timer_new ();
		
	memset (buffer, 0, sizeof (buffer));
	
	html_view_set_document (HTML_VIEW (view), NULL);
	html_document_clear (document);
	html_view_set_document (HTML_VIEW (view), document);
	
	file = fopen (path, "r");
	g_free (path);
	
	if (!file)
		return;


	if (html_document_open_stream (document, "text/html")) {
	  while ((i = fread (&buffer, 1, 10, file))) {
	    html_document_write_stream (document, buffer, i);
	  }
	  
	  html_document_close_stream (document);
	}

	elapsed_time = g_timer_elapsed (timer, NULL);

	g_print ("Parsing time is %f secs\n", elapsed_time);
}

static void
selection_cb (GtkTreeSelection *selection,
	      GtkTreeModel     *model)
{
	GtkTreeIter iter;
	GValue value = {0, };
	
	if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
		return;
	
	gtk_tree_model_get_value (model, &iter,
				  FILENAME_COLUMN,
				  &value);

	load_file (g_value_get_string (&value));

	g_value_unset (&value);
}


static GtkWidget *
create_tree (void)
{
	GtkTreeSelection *selection;
	GtkTreeStore *model;
	GtkTreeIter iter;
	GtkWidget *tree_view;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	gint i;
	
	model = gtk_tree_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
	tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
				     GTK_SELECTION_SINGLE);

	g_signal_connect (G_OBJECT (selection), "changed", (GtkSignalFunc)selection_cb, model);
	
	for (i = 0; i < G_N_ELEMENTS (documents); i++) {
		gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);

		gtk_tree_store_set (GTK_TREE_STORE (model),
				    &iter,
				    TITLE_COLUMN, documents[i].title,
				    FILENAME_COLUMN, documents[i].filename,
				    -1);

		if (strcmp (documents[i].title, "Status") == 0)
			add_status_docs (model, &iter);
	}

	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Tests",
							   cell,
							   "text", TITLE_COLUMN,
							   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
				     GTK_TREE_VIEW_COLUMN (column));

	return tree_view;
}

static gboolean
cb_delete_event (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();

	return FALSE;
}

static gboolean
cb_clear_doc (GtkWidget *widget, gpointer data)
{
	html_view_set_document (HTML_VIEW (view), NULL);
  	html_document_clear (document);
	html_view_set_document (HTML_VIEW (view), document);

	return FALSE;
}

static void
debug_dump_boxes (HtmlBox *root, gint indent, gboolean has_node, xmlNode *n)
{
	HtmlBox *box;
	gint i;

	if (!root)
		return;
	
	if (has_node) {
		if (root->dom_node != NULL && root->dom_node->xmlnode != n)
			return;
	}
	
	box = root->children;
	
	
	for (i = 0; i < indent; i++)
		g_print (" ");

	g_print ("Type: %s (%p, %p, %p) (%d %d %d %d)\n",
		 G_OBJECT_TYPE_NAME (root), root, root->dom_node, HTML_BOX_GET_STYLE (root), root->x, root->y, root->width, root->height);

	while (box) {
	  debug_dump_boxes (box, indent + 1, has_node, n);
	  box = box->next;
	}
}

static void
cb_dump_boxes (GtkWidget *widget, HtmlView *view)
{
	debug_dump_boxes (HTML_VIEW (view)->root, 0, FALSE, NULL);  
}

static void
cb_relayouts (GtkWidget *button, HtmlView *view)
{
  	GTimer *timer;
	gdouble tid;

	timer = g_timer_new ();
	
	tid = g_timer_elapsed (timer, NULL);
	g_timer_destroy (timer);

	g_print ("=============================================\n");
	g_print ("Time for 1,000 relayouts is: %f secs\n", tid);
	g_print ("=============================================\n");
}

static void
cb_repaints (GtkWidget *button, HtmlView *view)
{
  	GTimer *timer;
	gdouble tid;

	timer = g_timer_new ();
	
	tid = g_timer_elapsed (timer, NULL);
	g_timer_destroy (timer);

	g_print ("FIXME\n");
	g_print ("=============================================\n");
	g_print ("Time for 1,000 repaints is: %f secs\n", tid);
	g_print ("=============================================\n");
}

static gboolean
request_object (HtmlView *view, GtkWidget *widget, gpointer user_data)
{
	GtkWidget *sel;

	sel = gtk_color_selection_new ();
	gtk_widget_show (sel);

	gtk_container_add (GTK_CONTAINER (widget), sel);

	return TRUE;
}

gint
main (gint argc, gchar **argv)
{
	GtkWidget *window;
	GtkWidget *frame;
	GtkWidget *tree_view;
	GtkWidget *vbox, *hbox;
	GtkWidget *hpaned, *button, *sw;

	gtk_init (&argc, &argv);

	/* Set properties */
	g_object_set (G_OBJECT (gtk_html_context_get ()),
		      "debug_painting", FALSE,
		      NULL);
	
	/* Create the document */
	document = html_document_new ();
	g_signal_connect (G_OBJECT (document), "dom_mouse_down",
			  G_CALLBACK (dom_mouse_down), NULL);
	g_signal_connect (G_OBJECT (document), "dom_mouse_up",
			  G_CALLBACK (dom_mouse_up), NULL);
	g_signal_connect (G_OBJECT (document), "dom_mouse_click",
			  G_CALLBACK (dom_mouse_click), NULL);
	g_signal_connect (G_OBJECT (document), "dom_mouse_over",
			  G_CALLBACK (dom_mouse_over), NULL);
	g_signal_connect (G_OBJECT (document), "dom_mouse_out",
			  G_CALLBACK (dom_mouse_out), NULL);

	g_signal_connect (G_OBJECT (document), "request_url",
			  G_CALLBACK (url_requested), NULL);
	g_signal_connect (G_OBJECT (document), "link_clicked",
			  G_CALLBACK (link_clicked), NULL);
	
	/* And the view */
	view = html_view_new ();

	g_signal_connect (G_OBJECT (view), "request_object",
			  G_CALLBACK (request_object), NULL);

	/*	gtk_widget_set_double_buffered (GTK_WIDGET (view), FALSE); */
	
	sw = gtk_scrolled_window_new (gtk_layout_get_hadjustment (GTK_LAYOUT (view)),
				       gtk_layout_get_vadjustment (GTK_LAYOUT (view)));
	gtk_container_add (GTK_CONTAINER (sw), view);
	
	/* Create the window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 600, 400);
	
	g_signal_connect (window, "delete_event",
			  G_CALLBACK (cb_delete_event), NULL);

	hpaned = gtk_hpaned_new ();

	tree_view = create_tree ();
	
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), tree_view);
	gtk_paned_add1 (GTK_PANED (hpaned), frame);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), sw);
	gtk_paned_add2 (GTK_PANED (hpaned), frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Time 1,000 relayouts");
	g_signal_connect (button, "clicked",
			  G_CALLBACK (cb_relayouts), view);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Time 1,000 repaints");
	g_signal_connect (button, "clicked",
			  G_CALLBACK (cb_repaints), view);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	
	button = gtk_button_new_with_label ("Dump tree");
	g_signal_connect (button, "clicked",
			  G_CALLBACK (cb_dump_boxes), view);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Clear document");
	g_signal_connect (button, "clicked",
			    G_CALLBACK (cb_clear_doc), view);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	/* FIXME: ugly ugly! */
	html_view_set_document (HTML_VIEW (view), document);
	gtk_widget_show_all (window);

	/*	xmlDebugDumpDocument (stdout, _dom_Node__get_xmlNode (document->doc)); */
	/*debug_dump_boxes (HTML_VIEW (view)->root, 0, FALSE, NULL);*/
	
	gtk_main ();

	return 0;
}
