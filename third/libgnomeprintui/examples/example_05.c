/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  example_05.c: sample gnome-print code
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

#include <string.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprint/gnome-print-config.h>

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
my_set_names (GnomePrintJob *job)
{
	GnomePrintConfig *config;
	gchar *printer;
	
	config = gnome_print_job_get_config (job);

	/* Set the printer */
	gnome_print_config_set (config, "Printer", "GENERIC");
	printer = gnome_print_config_get (config, "Printer");
	if (strcmp (printer, "GENERIC") != 0) {
		g_warning ("Could not set printer to GENERIC Postscript");
		return;
	}
	g_free (printer);
	
	gnome_print_job_print_to_file (job, "output_05.ps");
	gnome_print_config_set (config, GNOME_PRINT_KEY_DOCUMENT_NAME, "Sample gnome-print document");
}

static void
my_dump_orientation (GnomePrintJob *job)
{
	GnomePrintConfig *config;
	gchar *orientation;

	config = gnome_print_job_get_config (job);

	orientation = gnome_print_config_get (config, GNOME_PRINT_KEY_ORIENTATION);
	g_print ("Orientation is: %s\n", orientation);
	if (orientation)
		g_free (orientation);
}

static void
my_print (void)
{
	GnomePrintJob *job;
	GnomePrintContext *gpc;

	job = gnome_print_job_new (NULL);
	gpc = gnome_print_job_get_context (job);

	my_set_names (job);
	my_dump_orientation (job);
	
	my_draw (gpc);

	gnome_print_job_close (job);
	gnome_print_job_print (job);

	g_object_unref (G_OBJECT (gpc));
	g_object_unref (G_OBJECT (job));
}

int
main (int argc, char * argv[])
{
	g_type_init ();
	
	my_print ();

	g_print ("Done...\n");

	return 0;
}
