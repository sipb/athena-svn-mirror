/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * Copyright (C) 2000 Eazel, Inc
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
 * Author: Maciej Stachowiak <mjs@eazel.com>
 */

/* test.c - Main function and object activation function for sample
 * view component.
 */

/* WHAT YOU NEED TO CHANGE: You need to change include
 * component-specific header below the include of config.h. Then look
 * for the CHANGE comments below and change the #defines.
 */

#include <config.h>

#include <libgnomeui/libgnomeui.h>
#include <bonobo-activation/bonobo-activation.h>
#include "nautilus-test-view.h"

struct NautilusTestViewDetails {
	char *location;
	GtkWidget *label;
};


int
main (int argc, char *argv[])
{
	GtkWidget *top;

        Bonobo_Unknown obj;
        Nautilus_View view;
        Bonobo_Control control;
        GtkWidget *widget;

	gnome_init_with_popt_table (PACKAGE, VERSION, argc, argv,
			            NULL, 0, NULL);
	/* FIXME: for some reason I can't get gnome_init to work */
	/* gnome_init (PACKAGE, VERSION, argc, argv); */

	if (argc < 2)
	{
		g_print ("Please provide a URI to view !\n");
		return -1;
	}

	top = gtk_window_new (GTK_WINDOW_TOPLEVEL);

        obj = bonobo_activation_activate_from_id
                        ("OAFIID:Nautilus_Test_View", 0, NULL, NULL);
	if (obj == NULL)
	{
		g_print ("Could not activate test bonobo component.\n");
		return -1;
	}

        view = Bonobo_Unknown_queryInterface (obj, "IDL:Nautilus/View:1.0",
                                      NULL);
	g_assert (view);
        Nautilus_View_load_location (view, argv[1], NULL);

        control = Bonobo_Unknown_queryInterface (obj, "IDL:Bonobo/Control:1.0",
                                         NULL);
	g_assert (control);
        widget = GTK_WIDGET (bonobo_widget_new_control_from_objref (control,
                                                        CORBA_OBJECT_NIL));

	/* show the widget */
	gtk_container_add (GTK_CONTAINER (top), widget);
	gtk_widget_show_all (top);
	g_signal_connect (top, "delete-event",
			  G_CALLBACK (gtk_main_quit), NULL);

	gtk_main ();
}
