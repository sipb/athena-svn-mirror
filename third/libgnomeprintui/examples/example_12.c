/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  example_12.c: sample gnome-print code to show gnome-print-widget usage
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
#include <libgnomeprint/gnome-print-config.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkmain.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-widget.h>
#include <libgnomeprintui/gnome-print-i18n.h>

GnomePrintConfigOption frames [] = {
	{"AsDisplayed",   N_("As _displayed"),   0},
	{"SelectedFrame", N_("_Selected Frame"), 1},
	{"Separately",    N_("Print each frame _separately"), 2},
	{NULL}
};

GnomePrintConfigOption colors [] = {
	{"Color",     N_("_Color"),           0},
	{"Grayscale", N_("_Black and white"), 1},
	{NULL}
};

static void
add_custom_options (GnomePrintConfig *config)
{
	gnome_print_config_insert_boolean (
		config, "Settings.Application.PageTitle",   TRUE);
	gnome_print_config_insert_boolean (
		config, "Settings.Application.PageURL",     TRUE);
	gnome_print_config_insert_boolean (
		config, "Settings.Application.PageNumbers", TRUE);
	gnome_print_config_insert_boolean (
		config, "Settings.Application.Date",        TRUE);

	gnome_print_config_insert_options (
		config,  "Settings.Application.Color",
		colors, "Color");
	gnome_print_config_insert_options (
		config,  "Settings.Application.FrameType",
		frames, "AsDisplayed");
}

static GtkDialog *
add_custom_widgets (GnomePrintConfig *config)
{
	GtkWidget *w1, *w2, *w3, *w4, *w5, *w6;
	GtkWidget *dialog;
	GtkBox *box;

	dialog = gtk_dialog_new ();
	box = GTK_BOX (GTK_DIALOG (dialog)->vbox);
	
	w1 = gnome_print_checkbutton_new (
		config, "Settings.Application.PageTitle",   _("Add page title"));
	w2 = gnome_print_checkbutton_new (
		config, "Settings.Application.PageURL",     _("Add page URL"));
	w3 = gnome_print_checkbutton_new (
		config, "Settings.Application.PageNumbers", _("Show page numbers"));
	w4 = gnome_print_checkbutton_new (
		config, "Settings.Application.Date",        _("Show date"));

	w5 = gnome_print_radiobutton_new (
		config, "Settings.Application.Color", colors);
	w6 = gnome_print_radiobutton_new (
		config, "Settings.Application.FrameType",  frames);
	
	gtk_box_pack_start (box, w1, TRUE, TRUE, 0);
	gtk_box_pack_start (box, w2, TRUE, TRUE, 0);
	gtk_box_pack_start (box, w3, TRUE, TRUE, 0);
	gtk_box_pack_start (box, w4, TRUE, TRUE, 0);
	gtk_box_pack_start (box, w5, TRUE, TRUE, 0);
	gtk_box_pack_start (box, w6, TRUE, TRUE, 0);
	
	gtk_widget_show_all (dialog);

	return GTK_DIALOG (dialog);
}

static void
dump_custom_options (GnomePrintConfig *config)
{
	gboolean state = FALSE;
	gint index;
	gchar *id;

	state = TRUE;
	gnome_print_config_get_boolean (config, "Settings.Application.PageTitle", &state);
	g_print ("Page title: %s\n", state ? "yes" : "no");

	state = TRUE;
	gnome_print_config_get_boolean (config, "Settings.Application.PageURL", &state);
	g_print ("Page url:   %s\n", state ? "yes" : "no");

	state = TRUE;
	gnome_print_config_get_boolean (config, "Settings.Application.PageNumbers", &state);
	g_print ("Page num.:  %s\n", state ? "yes" : "no");

	state = TRUE;
	gnome_print_config_get_boolean (config, "Settings.Application.Date", &state);
	g_print ("Print date: %s\n", state ? "yes" : "no");

	index = 0;
	gnome_print_config_get_option (config, "Settings.Application.Color", colors, &index);
	g_print ("Color by index: %d\n", index);

	id = gnome_print_config_get (config, "Settings.Application.Color");
	g_print ("Color by id:    %s\n", id);

	index = 0;
	gnome_print_config_get_option (config, "Settings.Application.FrameType", frames, &index);
	g_print ("Frame type by index: %d\n", index);

	id = gnome_print_config_get (config, "Settings.Application.FrameType");
	g_print ("Frame type by id   : %s\n", id);
}

static void
my_print (void)
{
	GnomePrintJob *job;
	GnomePrintConfig *config;
	GtkDialog *d;

	job    = gnome_print_job_new (NULL);
	config = gnome_print_job_get_config (job);

	add_custom_options (config);
	d = add_custom_widgets (config);
	gtk_window_move (GTK_WINDOW (d), 10, 10);
	d = add_custom_widgets (config);

	gtk_dialog_run (d);
	
	gnome_print_config_dump (config);
	dump_custom_options (config);

	g_object_unref (config);
	g_object_unref (job);
}

int
main (int argc, char * argv[])
{
	gtk_init (&argc, &argv);
	
	my_print ();

	g_print ("Done...\n");

	return 0;
}
