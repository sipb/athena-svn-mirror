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

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkdialog.h>
#include <glade/glade.h>

static void
my_draw (GnomePrintContext *gpc)
{
	gnome_print_beginpage (gpc, "1");

	gnome_print_moveto (gpc, 100, 200);
	gnome_print_lineto (gpc, 200, 200);

	gnome_print_showpage (gpc);
}


static void
my_print (GnomePrintMaster *gpm, gboolean preview)
{
	GnomePrintContext *gpc;

	g_print ("%s ...\n", preview ? "Print preview" : "Print");

	gpc = gnome_print_master_get_context (gpm);
	my_draw (gpc);
	gnome_print_master_close (gpm);
	gnome_print_master_print (gpm);
}

static void
my_print_cb (void)
{
	GnomePrintMaster *gpm;
	GtkWidget *dialog;
	gint response;

	/* Create the objects */
	gpm    = gnome_print_master_new ();
	dialog = gnome_print_dialog_new_from_master (gpm, "Sample print dialog", 0);

	/* Run the dialog */
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	switch (response) {
	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
		my_print (gpm, FALSE);
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		my_print (gpm, TRUE);
		break;
	default:
		return;
	}
}

static void
my_print_preview_cb (void)
{
	GnomePrintMaster *gpm;

	gpm = gnome_print_master_new ();
	my_print (gpm, TRUE);
}

static void
my_print_setup_cb (void)
{
	g_print ("Implement me\n");
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

	/* Connect callbacks */
	item = glade_xml_get_widget (gui, "menu_quit_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect_swapped (G_OBJECT (item), "activate",
				  G_CALLBACK (gtk_main_quit), NULL);
	item = glade_xml_get_widget (gui, "menu_print_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect_swapped (G_OBJECT (item), "activate",
				  G_CALLBACK (my_print_cb), NULL);
	item = glade_xml_get_widget (gui, "menu_print_preview_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect_swapped (G_OBJECT (item), "activate",
				  G_CALLBACK (my_print_preview_cb), NULL);
	item = glade_xml_get_widget (gui, "menu_print_setup_item");
	g_return_val_if_fail (item, FALSE);
	g_signal_connect_swapped (G_OBJECT (item), "activate",
				  G_CALLBACK (my_print_setup_cb), NULL);

	item = glade_xml_get_widget (gui, "window1");
	
	return TRUE;
}

int
main (int argc, char * argv[])
{
	gtk_init (&argc, &argv);

	if (!my_app_load ())
		return -1;

	gtk_main ();

	return 0;
}
