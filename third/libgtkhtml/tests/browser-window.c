/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000 CodeFactory AB
   Copyright (C) 2000 Jonas Borgström <jonas@codefactory.se>
   Copyright (C) 2000 Anders Carlsson <andersca@codefactory.se>
   
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

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/uri.h>
#include <libxml/debugXML.h>
#include <sys/time.h>
#include <unistd.h>
#include <libgnomevfs/gnome-vfs.h>

#include <graphics/htmlpainter.h>
#include <util/htmlstream.h>
#include "gtkhtmlcontext.h"

#include "debug.h"
#include "browser-window.h"
#include "prop-editor.h"

#define BUFFER_SIZE 8192

static void browser_window_init		(BrowserWindow		 *window);
static void browser_window_class_init	(BrowserWindowClass	 *klass);
static void browser_window_load_file (BrowserWindow *window, const gchar *name, HtmlParserType type);
static void link_clicked (HtmlDocument *doc, const gchar *url, gpointer data);

static GtkWindowClass *parent_class = NULL;

static GnomeVFSURI *baseURI = NULL;

typedef struct {
	HtmlDocument *doc;
	HtmlStream *stream;
	GnomeVFSAsyncHandle *handle;
} StreamData;

static gboolean
set_base (BrowserWindow *window, const gchar *url)
{
	gchar *str, *Old, *New;
	GnomeVFSURI *new_uri;
	gboolean equal = FALSE;

	if (baseURI) {

		new_uri = gnome_vfs_uri_resolve_relative (baseURI, url);

		Old = gnome_vfs_uri_to_string (baseURI, GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER);
		New = gnome_vfs_uri_to_string (new_uri, GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER);
		equal = (strcmp (Old, New) == 0);
		g_free (Old);
		g_free (New);
		
		gnome_vfs_uri_unref (baseURI);
		baseURI = new_uri;
	}
	else 
		baseURI = gnome_vfs_uri_new (url);

	str = gnome_vfs_uri_to_string (baseURI, GNOME_VFS_URI_HIDE_NONE);
	gtk_entry_set_text (GTK_ENTRY (window->entry), str);
	g_free (str);

	return equal;
}

static void
free_stream_data (StreamData *sdata, gboolean remove)
{
	GSList *connection_list;

	if (remove) {
		connection_list = g_object_get_data (G_OBJECT (sdata->doc), "connection_list");
		connection_list = g_slist_remove (connection_list, sdata);
		g_object_set_data (G_OBJECT (sdata->doc), "connection_list", connection_list);
	}
	html_stream_close(sdata->stream);
	
	g_free (sdata);
}

static void
stream_cancel (HtmlStream *stream, gpointer user_data, gpointer cancel_data)
{
	StreamData *sdata = (StreamData *)cancel_data;
	gnome_vfs_async_cancel (sdata->handle);
	free_stream_data (sdata, TRUE);
}

static void
vfs_close_callback (GnomeVFSAsyncHandle *handle,
		GnomeVFSResult result,
		gpointer callback_data)
{
}

static void
vfs_read_callback (GnomeVFSAsyncHandle *handle, GnomeVFSResult result,
               gpointer buffer, GnomeVFSFileSize bytes_requested,
	       GnomeVFSFileSize bytes_read, gpointer callback_data)
{
	StreamData *sdata = (StreamData *)callback_data;

	if (result != GNOME_VFS_OK) {
		gnome_vfs_async_close (handle, vfs_close_callback, sdata);
		free_stream_data (sdata, TRUE);
		g_free (buffer);
	} else {
		html_stream_write (sdata->stream, buffer, bytes_read);
		
		gnome_vfs_async_read (handle, buffer, bytes_requested, 
				      vfs_read_callback, sdata);
	}
}

static void
vfs_open_callback  (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer callback_data)
{
	StreamData *sdata = (StreamData *)callback_data;

	if (result != GNOME_VFS_OK) {

		g_warning ("Open failed: %s.\n", gnome_vfs_result_to_string (result));
		free_stream_data (sdata, TRUE);
	} else {
		gchar *buffer;

		buffer = g_malloc (BUFFER_SIZE);
		gnome_vfs_async_read (handle, buffer, BUFFER_SIZE, vfs_read_callback, sdata);
	}
}

typedef struct {
	BrowserWindow *window;
	gchar *action;
	gchar *method;
	gchar *encoding;
} SubmitContext;

static int
on_submit_idle (gpointer data)
{
	SubmitContext *ctx = (SubmitContext *)data;

	g_print ("action = '%s', method = '%s', encoding = '%s'\n", 
		 ctx->action, ctx->method, ctx->encoding);

	if (ctx->method == NULL || strcasecmp (ctx->method, "get") == 0) {
		gchar *url;

		url = g_strdup_printf ("%s?%s", ctx->action, ctx->encoding);
		link_clicked (NULL, url, ctx->window);
		g_free (url);
	}
	g_free (ctx);
	return 0;
}

static void
on_submit (HtmlDocument *document, const gchar *action, const gchar *method, 
	   const gchar *encoding, gpointer data)
{
	SubmitContext *ctx = g_new0 (SubmitContext, 1);

	if (action)
		ctx->action = g_strdup (action);
	if (method)
		ctx->method = g_strdup (method);
	if (action)
		ctx->encoding = g_strdup (encoding);
	ctx->window = data;

	/* Becase the link_clicked method will clear the document and
	 * start loading a new one, we can't call it directly, because
	 * gtkhtml2 will crash if the document becomes deleted before
	 * this signal handler finish */
	gtk_idle_add (on_submit_idle, ctx);
}

static void
url_requested (HtmlDocument *doc, const gchar *uri, HtmlStream *stream, gpointer data)
{
	GnomeVFSURI *vfs_uri;
	StreamData *sdata;
	GSList *connection_list;

	if (baseURI)
		vfs_uri = gnome_vfs_uri_resolve_relative (baseURI, uri);
	else
		vfs_uri = gnome_vfs_uri_new(uri);

	g_assert (HTML_IS_DOCUMENT(doc));
	g_assert (stream != NULL);

	sdata = g_new0 (StreamData, 1);
	sdata->doc = doc;
	sdata->stream = stream;

	connection_list = g_object_get_data (G_OBJECT (doc), "connection_list");
	connection_list = g_slist_prepend (connection_list, sdata);
	g_object_set_data (G_OBJECT (doc), "connection_list", connection_list);

	gnome_vfs_async_open_uri (&sdata->handle, vfs_uri, GNOME_VFS_OPEN_READ,
				  GNOME_VFS_PRIORITY_DEFAULT, vfs_open_callback, sdata);

	gnome_vfs_uri_unref (vfs_uri);

	html_stream_set_cancel_func (stream, stream_cancel, sdata);
}

static void
kill_old_connections (HtmlDocument *doc)
{
	GSList *connection_list, *tmp;

	tmp = connection_list = g_object_get_data (G_OBJECT (doc), "connection_list");
	while(tmp) {

		StreamData *sdata = (StreamData *)tmp->data;
		gnome_vfs_async_cancel (sdata->handle);
		free_stream_data (sdata, FALSE);

		tmp = tmp->next;
	}
	g_object_set_data (G_OBJECT (doc), "connection_list", NULL);
	g_slist_free (connection_list);
}

static void
title_changed (HtmlDocument *doc, const gchar *new_title, gpointer data)
{
	gchar *str = g_strdup_printf("testgtkhtml: %s", new_title);
	gtk_window_set_title (GTK_WINDOW (data), str);
	g_free (str);
}

static void
link_clicked (HtmlDocument *doc, const gchar *url, gpointer data)
{
	BrowserWindow *window = (BrowserWindow *)data;
	const gchar *anchor;
	gchar *str_url;

	g_print ("signal \"link_clicked\" url = \"%s\"\n", url);

	if (set_base (window, url) == TRUE) {
		/* Same document, just jump to the anchor */
		anchor = gnome_vfs_uri_get_fragment_identifier (baseURI);
		if (anchor != NULL)
			html_view_jump_to_anchor (window->view, anchor);
	}
	else {
		kill_old_connections (window->doc);
		
		/* Now load the specified filename */
		html_document_clear (window->doc);
		html_document_open_stream (window->doc, "text/html");
		gtk_adjustment_set_value (gtk_layout_get_vadjustment (GTK_LAYOUT (window->view)), 0);
		
		str_url = gnome_vfs_uri_to_string (baseURI, GNOME_VFS_URI_HIDE_NONE);
		url_requested (window->doc, str_url, window->doc->current_stream, NULL);
		g_free (str_url);
		
		if (baseURI) {
			anchor = gnome_vfs_uri_get_fragment_identifier (baseURI);
			if (anchor != NULL)
				html_view_jump_to_anchor (window->view, anchor);
		}
	}
	gtk_widget_queue_resize (GTK_WIDGET (data));
	gtk_label_set_text (GTK_LABEL (GTK_STATUSBAR (window->status_bar)->label), "");
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

static void
on_url (HtmlView *view, const char *url, gpointer user_data)
{
	BrowserWindow *window = BROWSER_WINDOW (user_data);

	gtk_label_set_text (GTK_LABEL (GTK_STATUSBAR (window->status_bar)->label), 
			    url);
}

static void
browser_window_set_view (BrowserWindow *window, HtmlView *view)
{
	GtkWidget *scr;

	window->view = view;

	g_signal_connect (G_OBJECT (view), "request_object",
			  G_CALLBACK (request_object), NULL);

	g_signal_connect (G_OBJECT (view), "on_url",
			  G_CALLBACK (on_url), window);

	scr = gtk_scrolled_window_new (gtk_layout_get_hadjustment (GTK_LAYOUT (view)),
				       gtk_layout_get_vadjustment (GTK_LAYOUT (view)));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scr), GTK_WIDGET (view));
	gtk_box_pack_start (GTK_BOX (window->vbox), scr, TRUE, TRUE, 0);
	gtk_widget_show_all (scr);
}

static void
browser_window_load_file (BrowserWindow *window, const gchar *name, HtmlParserType type)
{
	FILE *file;
	size_t bytes_read;
	gchar chars[10];

	html_document_clear (window->doc);
				       
	file = fopen (name, "r");

	if (!file) {
		g_print ("Could not open file %s\n", name);
		return;
	}

	if (html_document_open_stream (window->doc, "text/html")) {
		while ((bytes_read = fread (chars, 1, 3, file)) > 0) {
			html_document_write_stream (window->doc, chars, bytes_read);
		}
	}

	html_document_close_stream (window->doc);
}

static void
browser_window_load_test (gpointer data, guint action, GtkWidget *widget)
{
	gchar *dir = g_get_current_dir ();
	gchar *name = NULL;
	
	switch (action) {
	case 1:
		name = g_strdup_printf ("%s/samples/test1.html", dir);
		break;
	case 2:
		name = g_strdup_printf ("%s/samples/test2.html", dir);
		break;
	case 3:
		name = g_strdup_printf ("%s/samples/test3.html", dir);
		break;
	case 4:
		name = g_strdup_printf ("%s/samples/test4.html", dir);
		break;
	case 5:
		name = g_strdup_printf ("%s/samples/test5.html", dir);
		break;
	case 6:
		name = g_strdup_printf ("%s/samples/test6.html", dir);
		break;
	case 7:
		name = g_strdup_printf ("%s/samples/test7.html", dir);
		break;
	case 8:
		name = g_strdup_printf ("%s/samples/test8.html", dir);
		break;
	case 9:
		name = g_strdup_printf ("%s/samples/test9.html", dir);
		break;
	case 100:
		name = g_strdup_printf ("%s/samples/acid.html", dir);
		break;
	case 101:
		name = g_strdup_printf ("%s/samples/mixmagic.html", dir);
		break;
	case 102:
		name = g_strdup_printf ("%s/samples/slashdot.html", dir);
		break;
	case 103:
		name = g_strdup_printf ("%s/samples/freshmeat.html", dir);
		break;
	case 104:
		name = g_strdup_printf ("%s/samples/themes.html", dir);
		break;
	case 105:
		name = g_strdup_printf ("%s/samples/linux.com.html", dir);
		break;
	case 106:
		name = g_strdup_printf ("%s/samples/table.html", dir);
		break;
	case 107:
		name = g_strdup_printf ("%s/samples/mozilla.html", dir);
		break;
	case 108:
		name = g_strdup_printf ("%s/samples/gtk.themes.html", dir);
		break;
	case 109:
		name = g_strdup_printf ("%s/samples/andersca.html", dir);
		break;
	case 110:
		name = g_strdup_printf ("%s/samples/css-support.html", dir);
		break;
	case 111:
		name = g_strdup_printf ("%s/samples/dbaron-status.html", dir);
		break;
	case 112:
		name = g_strdup_printf ("%s/samples/jborg.html", dir);
		break;
	default:
		break;
	}
	g_free (dir);

	/* Now load the specified filename */
	browser_window_load_file (BROWSER_WINDOW (data), name, HTML_PARSER_TYPE_HTML);

	g_free (name);
}

static void
browser_window_clear_doc (gpointer data, guint action, GtkWidget *widget)
{
	gdouble tid;
	GTimer *timer;

	timer = g_timer_new ();
	
	html_document_clear (BROWSER_WINDOW (data)->doc);

	tid = g_timer_elapsed (timer, NULL);
	g_timer_destroy (timer);

	g_print ("=============================================\n");
	g_print ("Time to clear doc is: %f secs\n", tid);
	g_print ("=============================================\n");

}

static void
browser_window_exit (gpointer data, guint action, GtkWidget *widget)
{
	gtk_main_quit ();
}

static void
browser_window_properties (gpointer data, guint action, GtkWidget *widget)
{
	GtkWidget *prop_edit = create_prop_editor (G_OBJECT (gtk_html_context_get ()), 0);
	gtk_widget_show_all (prop_edit);
}

static void
browser_window_dump (gpointer data, guint action, GtkWidget *widget)
{
	BrowserWindow *window = BROWSER_WINDOW (data);

	if (action == 0) {
		debug_dump_boxes (window->view->root, 0, FALSE, NULL);
	}
}

static void
browser_window_new_view (gpointer data, guint action, GtkWidget *widget)
{
	BrowserWindow *window  = BROWSER_WINDOW (data);
	GtkWidget *new_window;

	/* Create a new view */
	new_window = browser_window_new (window->doc);

	gtk_widget_show (new_window);
	
}

static GtkItemFactoryEntry menu_items[] = {
	{ "/_File",          NULL,         0, 0, "<Branch>" },
	{ "/File/_New",      "<Control>N", 0, 0, NULL },
	{ "/File/New _View", NULL,         browser_window_new_view, 0, NULL },
	{ "/File/sep1",      NULL,         0, 0, "<Separator>" },
	{ "/File/_Close",    "<Control>W", 0, 0, NULL },
	{ "/File/E_xit",     "<Control>Q", browser_window_exit, 0, NULL },
	{ "/Tests",          NULL,         0, 0, "<Branch>" },
	{ "/Tests/Test 1",   NULL,         browser_window_load_test, 1, NULL },
	{ "/Tests/Test 2",   NULL,         browser_window_load_test, 2, NULL },
	{ "/Tests/Test 3",   NULL,         browser_window_load_test, 3, NULL },
	{ "/Tests/Test 4",   NULL,         browser_window_load_test, 4, NULL },
	{ "/Tests/Test 5",   NULL,         browser_window_load_test, 5, NULL },
	{ "/Tests/Test 6",   NULL,         browser_window_load_test, 6, NULL },
	{ "/Tests/Test 7",   NULL,         browser_window_load_test, 7, NULL },
	{ "/Tests/Test 8",   NULL,         browser_window_load_test, 8, NULL },
	{ "/Tests/Test 9",   NULL,         browser_window_load_test, 9, NULL },
	{ "/Tests/Acid test",   NULL,         browser_window_load_test, 100, NULL },
	{ "/Tests/MixMagic",   NULL,         browser_window_load_test, 101, NULL },
	{ "/Tests/Slashdot.org",   NULL,         browser_window_load_test, 102, NULL },
	{ "/Tests/Freshmeat.net",   NULL,         browser_window_load_test, 103, NULL },
	{ "/Tests/Themes.org",   NULL,         browser_window_load_test, 104, NULL },
	{ "/Tests/Linux.com",   NULL,         browser_window_load_test, 105, NULL },
	{ "/Tests/Table stress",   NULL,         browser_window_load_test, 106, NULL },
	{ "/Tests/Mozilla.org",   NULL,         browser_window_load_test, 107, NULL },
	{ "/Tests/gtk.themes.org",   NULL,         browser_window_load_test, 108, NULL },
	{ "/Tests/Anders test",   NULL,         browser_window_load_test, 109, NULL },
	{ "/Tests/CSS support",   NULL,         browser_window_load_test, 110, NULL },
	{ "/Tests/DBaron status",   NULL,         browser_window_load_test, 111, NULL },
	{ "/Tests/Jonas test",   NULL,         browser_window_load_test, 112, NULL },
	{ "/Debug",          NULL,         0, 0, "<Branch>" },
	{ "/Debug/Clear doc", NULL,        browser_window_clear_doc, 0, NULL },
	{ "/Debug/Dump boxes", NULL,       browser_window_dump, 0, NULL },
	{ "/Debug/Properties", NULL,       browser_window_properties, 0, NULL },
};

GtkType
browser_window_get_type (void)
{
  static GtkType window_type = 0;

  if (!window_type)
    {
	    static const GTypeInfo window_info =
	    {
		    sizeof (BrowserWindowClass),
		    NULL,
		    NULL,
		    (GClassInitFunc) browser_window_class_init,
		    NULL,
		    NULL,
		    sizeof (BrowserWindow),
		    16,
		    (GtkObjectInitFunc) browser_window_init,
	    };
	    
	    g_print ("gtk_window: %d browser_window: %d\n", sizeof (GtkWindowClass), sizeof (BrowserWindowClass));
	    

	    window_type = g_type_register_static (GTK_TYPE_WINDOW, "BrowserWindow", &window_info, 0);
    }

  return window_type;
}

static void
browser_window_class_init (BrowserWindowClass *klass)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass*) klass;

  parent_class = gtk_type_class (GTK_TYPE_WINDOW);

}


/**
 * browser_window_entry_activate:
 * @widget: a GtkEntry object
 * @data: the HtmlView object.
 * 
 * This function gets called when the user has entered an url into
 * the location bar and pressed enter.
 **/
static void
browser_window_entry_activate (GtkWidget *widget, gpointer data)
{
	const gchar *str = gtk_entry_get_text (GTK_ENTRY (widget));
	gchar *url;

	/*
	 * If no protocol is specified, then use http
	 */
	if (strchr (str, ':'))
		url = g_strdup (str);
	else
		url = g_strdup_printf ("http://%s", str);

	link_clicked (NULL, url, data);

	g_free (url);
}

static void
zoom_changed (GtkSpinButton *spin_button, gpointer user_data)
{
	html_view_set_magnification (BROWSER_WINDOW (user_data)->view,
				     gtk_spin_button_get_value (spin_button) / 100.0);
}

static void
browser_window_init (BrowserWindow *window)
{
	GtkWidget *hbox, *frame, *spinner;

	window->doc = NULL;
	window->view = NULL;

	window->entry = gtk_entry_new ();
	gtk_signal_connect (GTK_OBJECT (window->entry), "activate",
			    GTK_SIGNAL_FUNC (browser_window_entry_activate), window);
	
	window->item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", NULL);
	gtk_item_factory_create_items (window->item_factory, G_N_ELEMENTS (menu_items), menu_items, window);

	window->vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), window->vbox);
	
	gtk_box_pack_start (GTK_BOX (window->vbox),
			    gtk_item_factory_get_widget (window->item_factory, "<main>"),
			    FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);

	spinner = gtk_spin_button_new_with_range (1, 999, 1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinner), 100.0);
	g_signal_connect (G_OBJECT (spinner), "value_changed",
				  G_CALLBACK (zoom_changed), window);
	gtk_box_pack_start (GTK_BOX (hbox),
			    spinner,
			    FALSE, FALSE, 0);
	

	gtk_box_pack_start (GTK_BOX (hbox),
			    gtk_label_new ("URL: "),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox),
			    window->entry, TRUE, TRUE, 0);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_container_border_width (GTK_CONTAINER (hbox), 3);
	gtk_container_add (GTK_CONTAINER (frame), hbox);

	gtk_box_pack_start (GTK_BOX (window->vbox),
		frame, FALSE, FALSE, 0);

	gtk_widget_show_all (window->vbox);

	gtk_window_set_title (GTK_WINDOW (window), "GtkHtml II testbed application");
	gtk_window_set_default_size (GTK_WINDOW (window), 500, 400);

	window->status_bar = gtk_statusbar_new ();
	gtk_widget_show (window->status_bar);

	gtk_box_pack_end (GTK_BOX (window->vbox),
			    window->status_bar, FALSE, FALSE, 0);

}


GtkWidget *
browser_window_new (HtmlDocument *doc)
{
	BrowserWindow *window;

	window = g_object_new (BROWSER_TYPE_WINDOW, NULL);

	if (!doc) {
		window->doc = html_document_new ();

		g_signal_connect (G_OBJECT (window->doc), "link_clicked",
				  G_CALLBACK (link_clicked), window);

		g_signal_connect (G_OBJECT (window->doc), "title_changed",
				  G_CALLBACK (title_changed), window);

		g_signal_connect (G_OBJECT (window->doc), "request_url",
				  GTK_SIGNAL_FUNC (url_requested), window);

		g_signal_connect (G_OBJECT (window->doc), "submit",
				  GTK_SIGNAL_FUNC (on_submit), window);
	}
	else 
		window->doc = doc;

	browser_window_set_view (window, HTML_VIEW (html_view_new ()));
	html_view_set_document (window->view, window->doc);

	return GTK_WIDGET (window);
}

