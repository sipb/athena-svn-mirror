/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  example_06.c: sample gnome-print code
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
#include <gtk/gtkdialog.h>
#include <gtk/gtkmain.h>

static void
my_draw (GnomePrintContext *gpc)
{
	gnome_print_beginpage (gpc, "1");

	gnome_print_moveto (gpc, 100, 100);
	gnome_print_lineto (gpc, 200, 200);
	gnome_print_stroke (gpc);
	
	gnome_print_showpage (gpc);
}

static void
my_print (void)
{
	GnomePrintContext *gpc;
	GnomePrintMaster *gpm;
	GtkWidget *dialog;
	gint response;

	/* Create the objects */
	gpm    = gnome_print_master_new ();
	dialog = gnome_print_dialog_new_from_master (gpm, "Sample print dialog", 0);
	gpc    = gnome_print_master_get_context (gpm);

	/* Run the dialog */
	gtk_widget_show (dialog);
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GNOME_PRINT_DIALOG_RESPONSE_CANCEL) {
		g_print ("Printing was canceled\n");
		return;
	}

	/* Draw & print */
	g_print ("Printing ...\n");
	my_draw (gpc);
	gnome_print_master_close (gpm);
	gnome_print_master_print (gpm);
}

int
main (int argc, char * argv[])
{
	gtk_init (&argc, &argv);
	
	my_print ();

	g_print ("Done...\n");

	return 0;
}
