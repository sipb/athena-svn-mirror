/*
 * wombat-example.c: a small demo for the womabt moniker
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include <bonobo.h>
#include "bonobo-conf/bonobo-config-database.h"

static void 
property_change_cb (BonoboListener    *listener,
		    char              *event_name, 
		    CORBA_any         *any,
		    CORBA_Environment *ev,
		    gpointer           user_data)
{

	printf ("got property change event: %s\n", event_name);
}

/*
 * access the storage interface (~/evolution/config/)
 */
static void
run_storage_tests ()
{
	CORBA_Environment ev;
	Bonobo_Storage storage;
	Bonobo_Storage_DirectoryList *dlist;
	int i;

	CORBA_exception_init (&ev);

	/* get a reference to the database object */
	storage = bonobo_get_object ("wombat:", "Bonobo/Storage", &ev);
	g_assert (!BONOBO_EX (&ev));

	dlist = Bonobo_Storage_listContents (storage, "", 0, &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (dlist != NULL);

	for (i = 0; i < dlist->_length; i++) {

		printf ("DIRECTORY content: %s\n", dlist->_buffer [i].name);

	}

	CORBA_free (dlist);

	bonobo_object_release_unref (storage, NULL);
}

/*
 * We can use the Bonobo_ConfigDatabase interface directly.
 */
static void
run_database_tests ()
{
	CORBA_Environment ev;
	Bonobo_ConfigDatabase db;
	long vlong;

	CORBA_exception_init (&ev);

	/* get a reference to the database object */
	db = bonobo_get_object ("wombat:", "Bonobo/ConfigDatabase", &ev);
	g_assert (!BONOBO_EX (&ev));

	/* add a listener */
	bonobo_event_source_client_add_listener (db, property_change_cb,
						 NULL, &ev, NULL);	
	g_assert (!BONOBO_EX (&ev));
	
	/* set a value, which should trigger some events */
	bonobo_config_set_long (db, "/test/example/test-long", 56, &ev);
	g_assert (!BONOBO_EX (&ev));

	/* read it back */
	vlong = bonobo_config_get_long (db, "/test/example/test-long", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (vlong == 56);
}


static void
run_property_bag_tests ()
{
	CORBA_Environment ev;
	Bonobo_PropertyBag bag;
	Bonobo_Property prop;
	BonoboArg *arg;

	CORBA_exception_init (&ev);

	/* get a reference to the property bag */
	bag = bonobo_get_object ("wombat:#config:/test/example",
				 "Bonobo/PropertyBag", &ev);
	printf("TS %s\n", bonobo_exception_get_text (&ev));
	g_assert (!BONOBO_EX (&ev));

	/* add a listener */
	bonobo_event_source_client_add_listener (bag, property_change_cb,
						 NULL, &ev, NULL);	
	g_assert (!BONOBO_EX (&ev));
	
	/* get/create a reference to a property */
	prop = Bonobo_PropertyBag_getPropertyByName (bag, "value1", &ev);
	g_assert (!BONOBO_EX (&ev));

	/* create a new value */
	arg = bonobo_arg_new (BONOBO_ARG_LONG);
	BONOBO_ARG_SET_LONG (arg, 5);

	/* set the value, this should trigger the change callback */
	Bonobo_Property_setValue (prop, arg, &ev);
	g_assert (!BONOBO_EX (&ev));

	bonobo_arg_release (arg);

	/* read the value */
	arg = Bonobo_Property_getValue (prop, &ev);
	g_assert (!BONOBO_EX (&ev));

        printf ("value is %d\n", BONOBO_ARG_GET_LONG (arg)); 
}

static gint
run_tests ()
{
	run_database_tests ();
	run_property_bag_tests ();
	run_storage_tests ();

	printf ("ready\n");
	return 0;
}

int
main (int argc, char **argv)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	gnome_init ("wombat-example", "0.0", argc, argv);

	if ((oaf_init (argc, argv)) == NULL)
		g_error ("Cannot init oaf");

	if (bonobo_init (NULL, NULL, NULL) == FALSE)
		g_error ("Cannot init bonobo");
	
	gtk_idle_add ((GtkFunction) run_tests, NULL);

	/* we depend on the event loop - for notifications events */

	bonobo_main ();

	return 0;
}


