/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  example_09.c: sample gnome-print code. This dialog saves the GnomePrintConfig
 *                to disk after printing and loads it before creating the dialog
 *                This shows how to implement persistent print configuration.
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

#include <stdio.h>
#include <string.h>
#include <libgnomeprint/gnome-print.h>
#include <gtk/gtk.h>
#include <libgnomeprintui/gnome-print-dialog.h>

#define CONFIG_FILE "print_config"

static GnomePrintConfig *
my_config_load_from_file (void)
{
	GnomePrintConfig *config;
	FILE *file;
	gchar *str;
	gint read, allocated;

	file = fopen (CONFIG_FILE, "r");
	if (!file) {
		g_print ("Config not found\n");
		return gnome_print_config_default ();
	}

#define BLOCK_SIZE 10
	read = 0;
	allocated = BLOCK_SIZE;
	str = g_malloc (allocated);
	while (TRUE) {
		gint this;
		this = fread (str + read, sizeof (gchar), allocated - read, file);
		read += this;
		if (this < 1)
			break;
		if ((read + BLOCK_SIZE + 1) > allocated) {
			allocated += BLOCK_SIZE;
			str = g_realloc (str, allocated);
		}
	}
	str[read]=0;
	config = gnome_print_config_from_string (str, 0);

	return config;
}

static void
my_config_save_to_file (GnomePrintConfig *config)
{
	FILE *file;
	gchar *str;
	gint bytes;

	g_return_if_fail (config);

	str = gnome_print_config_to_string (config, 0);
	g_assert (str);
	
	file = fopen (CONFIG_FILE, "w");
	g_assert (file);
	
	bytes = strlen (str);
	str += bytes;
	while (bytes > 0)
		bytes -=  fwrite (str - bytes, sizeof (gchar), bytes < 1024 ? bytes : 1024, file);
	fclose (file);
}
	
static void
my_print (void)
{
	GnomePrintConfig *config;
	GnomePrintJob *job;
	GtkWidget *dialog;
	gint response;

	config = my_config_load_from_file ();
	job = gnome_print_job_new (config);
	dialog = gnome_print_dialog_new (job, "Example 09 print dialog", 0);
	gtk_widget_show (dialog);
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GNOME_PRINT_DIALOG_RESPONSE_CANCEL) {
		g_print ("Printing was canceled, config not saved\n");
		return;
	}

	g_assert (config);
	g_print ("Config saved to \"%s\"\n", CONFIG_FILE);
	my_config_save_to_file (config);

	g_object_unref (job);
	g_object_unref (config);
}

int
main (int argc, char * argv[])
{
	gtk_init (&argc, &argv);
	
	my_print ();

	g_print ("Done...\n");

	return 0;
}
