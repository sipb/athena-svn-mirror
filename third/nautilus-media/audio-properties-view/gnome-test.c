/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * Copyright (C) 2002 Thomas Vander Stichele
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Thomas Vander Stichele <thomas at apestaart dot org>
 */

/* gnome-test.c - Main function and widget creation using gnome */

#include <config.h>

#include <libgnomeui/libgnomeui.h>

#include "../media-info/media-info.h"
#include "audio-properties-view.h"

static void
quit (GtkWidget *widget, GdkEvent *event, AudioPropertiesView *apv)
{
	g_print ("DEBUG: gnome-test: quit: audio_properties_view %p\n", apv);
	audio_properties_view_dispose (apv);
	g_print ("DEBUG: finalized view\n");
	gtk_main_quit ();
	g_print ("DEBUG: gnome-test: quit: done\n");
}

int
main (int argc, char *argv[])
{
	GtkWidget *top;
	AudioPropertiesView *apv;
        GtkWidget *widget;
	GnomeProgram *program;

	struct poptOption options[] =
	{
		{ NULL, '\0', POPT_ARG_INCLUDE_TABLE, NULL, 0,
		  "GStreamer", NULL },
		POPT_TABLEEND
	};

	poptContext context;
	const gchar **argvn;

	options[0].arg = (void *) gst_init_get_popt_table ();
	if (! (program = gnome_program_init (PACKAGE, VERSION,
					     LIBGNOMEUI_MODULE,
				             argc, argv,
					     GNOME_PARAM_POPT_TABLE,
			                     options, NULL)))
		g_error ("gnome_program_init_failed");;

	g_object_get (program, "popt-context", &context, NULL);
	argvn = poptGetArgs (context);

	if (!argvn)
	{
		g_print ("Please provide a file to view !\n");
		return -1;
	}

	g_print ("creating top-level window\n");
	top = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_resize (GTK_WINDOW (top), 640, 480);

	apv = audio_properties_view_new ();
	g_print ("DEBUG: gnome-test: audio_properties_view %p\n", apv);
	audio_properties_view_load_location (apv, *argvn);

	widget = audio_properties_view_get_widget (apv);
	g_assert (GTK_IS_WIDGET (widget));

	gtk_container_add (GTK_CONTAINER (top), widget);
	gtk_widget_show (top);
	g_signal_connect (top, "delete-event",
			  G_CALLBACK (quit), apv);

	gtk_main ();
	g_print ("Stick a fork in me.\n");
}
