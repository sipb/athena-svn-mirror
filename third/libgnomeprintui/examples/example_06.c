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

#define GNOME_PRINT_UNSTABLE_API

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
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
	GnomePrintJob *job;
	GtkWidget *dialog;
	gint response;

	/* Create the objects */
	job    = gnome_print_job_new (NULL);
	dialog = gnome_print_dialog_new (job, "Sample print dialog", 0);
	gpc    = gnome_print_job_get_context (job);

	/* Run the dialog */
	response = gnome_print_dialog_run (GNOME_PRINT_DIALOG (dialog));
	if (response == GNOME_PRINT_DIALOG_RESPONSE_CANCEL) {
		g_print ("Printing was canceled\n");
		return;
	}

	/* Draw & print */
	g_print ("Printing ...\n");
	my_draw (gpc);
	gnome_print_job_close (job);
	gnome_print_job_print (job);

	g_object_unref (G_OBJECT (gpc));
	g_object_unref (G_OBJECT (job));
}

int
main (int argc, char * argv[])
{
	gtk_init (&argc, &argv);
	
	my_print ();

	g_print ("Done...\n");

	return 0;
}
