/*
 * LIBGAIL-GNOME -  Accessibility Toolkit Implementation for Bonobo
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <cspi/spi.h>
#include <libspi/libspi.h>
#include <libbonoboui.h>
#include <libgnomeui/gnome-ui-init.h>

static GtkWidget *inproc_widget = NULL;
static GtkWidget *outproc_widget = NULL;
static GtkWidget *container = NULL;

void
setup_inproc_control (void)
{
	AtkObject          *atko;
	GtkWidget          *label;
	BonoboControl      *control;

	label = gtk_label_new ("Yes! Yes! Yes!");
	gtk_widget_show (label);

	control = bonobo_control_new (label);
	g_assert (control != NULL);
	
	inproc_widget = bonobo_widget_new_control_from_objref (
				BONOBO_OBJREF (control), CORBA_OBJECT_NIL);
	gtk_widget_show (inproc_widget);
	atko = gtk_widget_get_accessible (inproc_widget);
	atk_object_set_name (atko, "inproc control");
	g_assert (inproc_widget != NULL);
	gtk_container_add (GTK_CONTAINER (container), inproc_widget);

	bonobo_object_unref (BONOBO_OBJECT (control));
}

void
setup_outproc_control (void)
{
	AtkObject *atko;
	outproc_widget = bonobo_widget_new_control ("OAFIID:Bonobo_Sample_Entry", NULL);
	atko = gtk_widget_get_accessible (outproc_widget);
	atk_object_set_name (atko, "outproc control");
	gtk_widget_show (GTK_WIDGET (outproc_widget));
	gtk_container_add (GTK_CONTAINER (container), outproc_widget);
}

static void
test_spi_stuff (BonoboWidget *widget)
{
	AtkObject                *socket_obj;
	Accessibility_Accessible  accessible;
	Accessibility_Accessible  parent;
	Accessibility_Accessible  tmp;
	CORBA_long                num_children;
	CORBA_Environment         env;
	BonoboControlFrame       *frame;
	GtkWidget                *socket;
	
	frame = bonobo_widget_get_control_frame (widget);

	socket = bonobo_control_frame_get_widget (frame);
	g_assert (BONOBO_IS_SOCKET (socket));

	socket_obj = gtk_widget_get_accessible (socket);
	g_assert (SPI_IS_REMOTE_OBJECT (socket_obj));

	accessible = spi_remote_object_get_accessible (SPI_REMOTE_OBJECT (socket_obj));
	g_assert (accessible != CORBA_OBJECT_NIL);
	
	CORBA_exception_init (&env);

	parent = Accessibility_Accessible__get_parent (accessible, &env);
	g_assert (env._major == CORBA_NO_EXCEPTION);
	g_assert (accessible != CORBA_OBJECT_NIL);

	num_children = Accessibility_Accessible__get_childCount (accessible, &env);
	g_assert (env._major == CORBA_NO_EXCEPTION);
	g_assert (num_children == 1);

	num_children = Accessibility_Accessible__get_childCount (parent, &env);
	g_assert (env._major == CORBA_NO_EXCEPTION);
	g_assert (num_children == 1);

	tmp = Accessibility_Accessible_getChildAtIndex (parent, 0, &env);
	g_assert (env._major == CORBA_NO_EXCEPTION);
	g_assert (CORBA_Object_is_equivalent (accessible, tmp, &env));
	g_assert (env._major == CORBA_NO_EXCEPTION);

	CORBA_exception_free (&env);
}

void
test_gail_gnome (void)
{
	GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	container = gtk_vbox_new (FALSE, 0);
	setup_inproc_control ();
	setup_outproc_control ();
	gtk_container_add (GTK_CONTAINER (window), container);
	gtk_widget_show (GTK_WIDGET (container));
	gtk_widget_show (GTK_WIDGET (window));

	test_spi_stuff (BONOBO_WIDGET (inproc_widget));
	test_spi_stuff (BONOBO_WIDGET (outproc_widget));
	
	gtk_main ();

	gtk_widget_destroy (inproc_widget);
	gtk_widget_destroy (outproc_widget);
}

int
main (int argc, char **argv)
{
	int leaked;

/*	putenv ("GTK_MODULES=gail:gail-gnome:atk-bridge"); */
/*	putenv ("GNOME_ACCESSIBILITY=1");                  */
	
	gnome_program_init ("test-gail-gnome",
			    "0.0.1",
			    LIBGNOMEUI_MODULE,
			    argc, argv, NULL);

	SPI_init ();

	test_gail_gnome ();

	if ((leaked = SPI_exit ()))
		g_error ("Leaked %d SPI handles", leaked);

	g_assert (!SPI_exit ());

	fprintf (stderr, "All tests passed\n");

	if (g_getenv ("_MEMPROF_SOCKET")) {
		fprintf (stderr, "Waiting for memprof\n");
		gtk_main ();
	}

	putenv ("AT_BRIDGE_SHUTDOWN=1");

	return 0;
}
