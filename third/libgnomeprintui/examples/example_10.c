/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  example_10.c: sample gnome-print code
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
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2002 Ximian Inc. and authors
 *
 */

/*
 * See README
 */

#define WE_ARE_LIBGNOMEPRINT_INTERNALS /* Needed for gpa-tree-viewer which is not public */
#define GNOME_PRINT_UNSTABLE_API

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprint/private/gpa-root.h>
#include <libgnomeprintui/gnome-print-job-preview.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-paper-selector.h>
#include <libgnomeprintui/gnome-font-dialog.h>
#include <libgnomeprintui/gpaui/gpa-tree-viewer.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

typedef struct {
	GtkWidget *view;
	GnomePrintConfig *config;
	gchar *name;
} MyDoc;

typedef struct {
	GtkWidget *status_bar;

	MyDoc *active_doc; /* The active document */
	
	MyDoc *doc1;
	MyDoc *doc2;
	MyDoc *doc3;
} MyApp;

static MyApp *app;

static gboolean
my_status_bar_pop (gpointer data)
{
	guint id = GPOINTER_TO_UINT (data);
	gtk_statusbar_remove (GTK_STATUSBAR (app->status_bar), 0, id);
	return FALSE;
}

static void
my_status_bar_print (const gchar *message)
{
	guint num;
	num = gtk_statusbar_push (GTK_STATUSBAR (app->status_bar), 0, message);
	g_timeout_add (3000, my_status_bar_pop, GUINT_TO_POINTER (num));
}

static void
my_print_image_from_pixbuf (GnomePrintContext *gpc, GdkPixbuf *pixbuf)
{
	guchar *raw_image;
	gboolean has_alpha;
	gint rowstride, height, width;
	
	raw_image = gdk_pixbuf_get_pixels (pixbuf);
	has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	height    = gdk_pixbuf_get_height (pixbuf);
	width     = gdk_pixbuf_get_width (pixbuf);
	
	if (has_alpha)
		gnome_print_rgbaimage (gpc, (char *)raw_image, width, height, rowstride);
	else
		gnome_print_rgbimage (gpc, (char *)raw_image, width, height, rowstride);
}

static void
my_print_image_from_disk (GnomePrintContext *gpc)
{
	GdkPixbuf *pixbuf;
	GError *error;

	error = NULL;
	pixbuf = gdk_pixbuf_new_from_file ("sample-image.png", &error);
	if (!pixbuf) {
		g_print ("Could not load sample_image.png.\n");
		return;
	}

	gnome_print_gsave (gpc);
	gnome_print_scale (gpc, 144, 144);
	
	my_print_image_from_pixbuf (gpc, pixbuf);
	
	gnome_print_grestore (gpc);
	g_object_unref (G_OBJECT (pixbuf));
}

static void
my_draw (GnomePrintContext *gpc, gint page)
{
	GnomeFont *font;
	gchar *t;
	gchar *page_name;

	font = gnome_font_find_closest ("Sans Regular", 18);
	g_assert (font);

	page_name = g_strdup_printf ("%d", page);
	gnome_print_beginpage (gpc, page_name);
	g_free (page_name);

	gnome_print_moveto (gpc, 1, 1);
	gnome_print_lineto (gpc, 200, 200);
	gnome_print_stroke (gpc);

	gnome_print_setfont (gpc, font);
	gnome_print_moveto  (gpc, 200, 72);
	t = g_strdup_printf ("Page: %d\n", page + 1);
	gnome_print_show    (gpc, t);
	g_free (t);

	my_print_image_from_disk (gpc);
	gnome_print_showpage (gpc);
}


static void
my_print (GnomePrintJob *job, gboolean preview)
{
	GnomePrintContext *gpc;
	gint i;

	gpc = gnome_print_job_get_context (job);
	for (i = 0; i < 4; i++)
		my_draw (gpc, i);
	g_object_unref (G_OBJECT (gpc));
	
	gnome_print_job_close (job);

	if (!preview) {
		my_status_bar_print ("Printing ...");
		gnome_print_job_print (job);
	} else {
		my_status_bar_print ("Print previewing ...");
		gtk_widget_show (gnome_print_job_preview_new (job, "Title goes here"));
	}
}

static void
my_print_cb (void)
{
	GnomePrintJob *job;
	GtkWidget *dialog;
	gint response;

	/* Create the objects */
	g_assert (app->active_doc);
	job    = gnome_print_job_new (app->active_doc->config);
	dialog = gnome_print_dialog_new (job, "Sample print dialog",
					 GNOME_PRINT_DIALOG_RANGE | GNOME_PRINT_DIALOG_COPIES);
	gnome_print_dialog_construct_range_page (GNOME_PRINT_DIALOG (dialog),
						 GNOME_PRINT_RANGE_ALL | GNOME_PRINT_RANGE_SELECTION,
						 1, 2, "A", "Lines");

	response = gnome_print_dialog_run (GNOME_PRINT_DIALOG (dialog));
	switch (response) {
	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
		my_print (job, FALSE);
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		my_print (job, TRUE);
		break;
	default:
		return;
	}

	g_object_unref (G_OBJECT (job));
}

static void
my_print_preview_cb (void)
{
	GnomePrintJob *job;

	job = gnome_print_job_new (app->active_doc->config);
	my_print (job, TRUE);
	g_object_unref (G_OBJECT (job));
}

static void
my_print_setup_cb (void)
{
	GtkWidget *dialog;
	GtkWidget *paper_selector;
	GtkWidget *notebook;
	GtkWidget *label;
	gint response;

	notebook = gtk_notebook_new ();

	/* GnomePrintPaperSelector */
	paper_selector = gnome_paper_selector_new_with_flags (app->active_doc->config, 0);
	gtk_widget_set_usize (paper_selector, 200, 400);
	label = gtk_label_new_with_mnemonic ("P_aper");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), paper_selector, label);

	/* GnomePrintPaperSelector */
	paper_selector = gnome_paper_selector_new_with_flags (app->active_doc->config, 0);
	gtk_widget_set_usize (paper_selector, 200, 400);
	label = gtk_label_new_with_mnemonic ("_foo");
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), paper_selector, label);

	/* Dialog */
	dialog = gtk_dialog_new_with_buttons ("Printer Setup", NULL, 0,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OK,     GTK_RESPONSE_OK,
					      NULL);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    notebook, TRUE, TRUE, 0);
	gtk_widget_show_all (dialog);

	
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
my_font_dialog_cb (void)
{
	static GnomeFont *font = NULL;
	GnomeFontSelection *fontsel;
	GtkWidget *dialog;

	dialog = gnome_font_dialog_new ("Sample Font dialog");
	fontsel = (GnomeFontSelection *) gnome_font_dialog_get_fontsel (GNOME_FONT_DIALOG (dialog));
	if (font)
		gnome_font_selection_set_font (fontsel, font);

	gtk_widget_show_all (dialog);
	gtk_dialog_run (GTK_DIALOG (dialog));

	if (font)
		g_object_unref (G_OBJECT (font));
	font = gnome_font_selection_get_font (fontsel);

	gtk_widget_destroy (dialog);
}


static void
my_tree_cb (void)
{
	GtkWidget *dialog;

	dialog = gpa_tree_viewer_new (GPA_NODE (gpa_root));
}


/**
 * my_document_activate_cb:
 * @document: 
 * @data: 
 * 
 * This function is called when a document is activated. We set the active document here
 * and update the GnomePrintWidgets with the active document config
 **/
static void
my_document_activate_cb (GtkWidget *view, MyDoc *doc)
{
	gchar *message;

	if (app->active_doc == doc)
		return;
	app->active_doc = doc;
	message = g_strdup_printf ("The active document is now \"%s\"", doc->name);
	my_status_bar_print (message);
	g_free (message);
}

/**
 * my_new_doc:
 * @doc_name: 
 * @gui: 
 * 
 * Creates a new document
 * 
 * Return Value: a pointer to the new document, aborts on error
 **/
static MyDoc *
my_new_doc (const gchar *doc_name, GladeXML *gui)
{
	MyDoc *doc;

	doc = g_new (MyDoc, 1);
	doc->view = glade_xml_get_widget (gui, doc_name);
	doc->name = g_strdup (doc_name);
	doc->config = gnome_print_config_default ();
	g_signal_connect (G_OBJECT (doc->view), "grab_focus",
			  G_CALLBACK (my_document_activate_cb), doc);

	g_assert (doc->view);
	g_assert (doc->config);
	
	return doc;
}

static gboolean
my_app_load (void)
{
	GladeXML *gui = glade_xml_new ("example_10.glade", NULL, NULL);
	GtkWidget *item;

	if (!gui) {
		g_warning ("Could not load glade file\n");
		return FALSE;
	}

	/* Connect menu items to callbacks */
	item = glade_xml_get_widget (gui, "menu_quit_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect (G_OBJECT (item), "activate",
			  G_CALLBACK (gtk_main_quit), NULL);
	item = glade_xml_get_widget (gui, "window1");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect (G_OBJECT (item), "delete_event",
			  G_CALLBACK (gtk_main_quit), NULL);
	item = glade_xml_get_widget (gui, "menu_print_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect (G_OBJECT (item), "activate",
			  G_CALLBACK (my_print_cb), NULL);
	item = glade_xml_get_widget (gui, "menu_print_preview_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect (G_OBJECT (item), "activate",
			  G_CALLBACK (my_print_preview_cb), NULL);
	item = glade_xml_get_widget (gui, "menu_print_setup_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect (G_OBJECT (item), "activate",
			  G_CALLBACK (my_print_setup_cb), NULL);
	item = glade_xml_get_widget (gui, "menu_tree_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect (G_OBJECT (item), "activate",
			  G_CALLBACK (my_tree_cb), NULL);
	item = glade_xml_get_widget (gui, "menu_font_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect (G_OBJECT (item), "activate",
			  G_CALLBACK (my_font_dialog_cb), NULL);

	app->status_bar = glade_xml_get_widget (gui, "statusbar");
	app->doc1 = my_new_doc ("doc1", gui);
#if 0	
	app->doc2 = my_new_doc ("doc2", gui);
	app->doc3 = my_new_doc ("doc3", gui);
#endif
	app->active_doc = NULL;
	gtk_widget_grab_focus (app->doc1->view);
	
	return TRUE;
}

int
main (int argc, char * argv[])
{
	gtk_init (&argc, &argv);

	app = g_new (MyApp, 1);
	
	if (!my_app_load ())
		return -1;

	gtk_main ();

	g_free (app);
	
	return 0;
}
