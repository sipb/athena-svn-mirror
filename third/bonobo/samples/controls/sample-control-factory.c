/**
 * sample-control-factory.c
 *
 * Author:
 *   Nat Friedman  (nat@nat.org)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */

#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>

#include "bonobo-clock-control.h"
#include "bonobo-calculator-control.h"

BonoboGenericFactory *factory = NULL;

static BonoboObject *
control_factory (BonoboGenericFactory *this,
		 const char           *object_id,
		 void                 *data)
{
	BonoboObject *object  = NULL;
	
	g_return_val_if_fail (object_id != NULL, NULL);

	if (!strcmp (
		object_id,
		"OAFIID:Bonobo_Sample_Clock"))
		object = bonobo_clock_control_new ();
	
	else if (!strcmp (
		object_id,
		"OAFIID:Bonobo_Sample_Calculator"))
		object = bonobo_calculator_control_new ();
	
	else if (!strcmp (
		object_id,
		"OAFIID:Bonobo_Sample_Entry"))
		object = bonobo_entry_control_new ();

	return object;
}
			
static void
init_bonobo (int argc, char **argv)
{
	CORBA_ORB orb;

        gnome_init_with_popt_table ("bonobo-sample-controls", "0.0",
				    argc, argv,
				    oaf_popt_options, 0, NULL); 
	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error (_("Could not initialize Bonobo"));
}

static void
last_unref_cb (BonoboObject *bonobo_object,
	       gpointer      dummy)
{
	bonobo_object_unref (BONOBO_OBJECT (factory));
	gtk_main_quit ();
}

int
main (int argc, char **argv)
{
	{ char *tmp = malloc (4); free (tmp); } /* -lefence */

	init_bonobo (argc, argv);

	factory = bonobo_generic_factory_new_multi (
		"OAFIID:Bonobo_Sample_ControlFactory",
		control_factory, NULL);	

	gtk_signal_connect (GTK_OBJECT (bonobo_context_running_get ()),
			    "last_unref",
			    GTK_SIGNAL_FUNC (last_unref_cb),
			    NULL);

	bonobo_main ();

	return 0;
}
