/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  example_11.c: sample gnome-print code
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
 *    Chris Lahey <clahey@ximian.com>
 *
 *  Copyright (C) 2002, 2003 Ximian Inc. and authors
 *
 */

/*
 * See README
 */

#define GNOME_PRINT_UNSTABLE_API

#define TEMP_FILE "temp.ps"

#include <stdio.h>
#include <string.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-job-preview.h>
#include <libgnomeprintui/gnome-print-dialog.h>

char *format = "%! PS-Adobe-3.0 \n"
"1 setlinewidth\n"
"newpath\n"
"100 100 moveto\n"
"600 600 lineto\n"
"stroke\n"
"/Helvetica findfont\n"
"24 scalefont setfont\n"
"100 230 moveto\n"
"(My page size is %fx%f) show\n"
"showpage";

static void
my_print (GnomePrintJob *job, gboolean preview)
{
	FILE *file;
	char *output, *str;
	int bytes;
	double width, height;
	GnomePrintConfig *config;

	file = fopen (TEMP_FILE, "w");
	g_assert (file);

	config = gnome_print_job_get_config(job);
	gnome_print_config_get_page_size (config, &width, &height);

	output = g_strdup_printf (format, width, height);
	str = output;

	bytes = strlen (str);
	str += bytes;
	while (bytes > 0)
		bytes -=  fwrite (str - bytes, sizeof (gchar), bytes < 1024 ? bytes : 1024, file);
	fclose (file);
	g_free (output);

	gnome_print_job_set_file (job, TEMP_FILE);

	if (!preview) {
		gnome_print_job_print (job);
	} else {
		gtk_widget_show (gnome_print_job_preview_new (job, "Title goes here"));
	}

	g_object_unref (G_OBJECT (job));
}

int
main (int argc, char * argv[])
{
	GnomePrintJob *job;
	GtkWidget *dialog;
	gint response;

	gtk_init (&argc, &argv);

	job    = gnome_print_job_new (NULL);
	dialog = gnome_print_dialog_new (job, "Sample print dialog", 0);

	/* Run the dialog */
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	switch (response) {
	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
		my_print (job, FALSE);
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		my_print (job, TRUE);
		break;
	}
	return 0;
}
