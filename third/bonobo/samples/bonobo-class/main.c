/*
 * main.c: Startup code for the Echo Bonobo Component.
 *
 * Author:
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * (C) 1999, 2000 Helix Code, Inc.  http://www.helixcode.com
 */
#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>

#include <bonobo.h>
#include "Echo.h"
#include "echo.h"

CORBA_Environment ev;

CORBA_ORB orb;

/*
 * The factory object for Echo servers
 */
static BonoboGenericFactory *factory;

/*
 * The count of active Echo servers running, when this reaches zero, we
 * shutdown the component
 */
static int active_echo_servers;

static void
init_server_factory (int argc, char **argv)
{
        gnome_init_with_popt_table("echo", "1.0",
				   argc, argv,
				   oaf_popt_options, 0, NULL); 
	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error (_("I could not initialize Bonobo"));
}

static void
echo_destroyed (GtkObject *echo_object)
{
	active_echo_servers--;

	if (active_echo_servers != 0)
		return;
		
	bonobo_object_unref (BONOBO_OBJECT (factory));
	gtk_main_quit ();
}

static BonoboObject *
echo_factory (BonoboGenericFactory *this_factory, void *data)
{
	Echo *echo;

	echo = echo_new ();
	
	if (echo == NULL) {
		return NULL;
	}

	active_echo_servers++;

	gtk_signal_connect (
		GTK_OBJECT (echo), "destroy",
		echo_destroyed, NULL);

	return BONOBO_OBJECT (echo);
}

static void
echo_factory_init (void)
{
	/*
	 * Creates and registers our Factory for Echo servers
	 */
	factory = bonobo_generic_factory_new (
		"OAFIID:demo_echo_factory:a7080731-d06c-42d2-852e-179c538f6ee5",
		echo_factory, NULL);

	if (factory == NULL)
		g_error ("It was not possible to register a new echo factory");
}

int
main (int argc, char *argv [])
{
	CORBA_exception_init (&ev);

	init_server_factory (argc, argv);

	/*
	 * Create an Echo factory
	 */
	echo_factory_init ();

	/*
	 * Let Bonobo start accepting requests
	 */
	bonobo_activate ();

	/*
	 * Main loop
	 */
	puts ("Echo component is active");
	gtk_main ();

	CORBA_exception_free (&ev);

	return 0;
}
