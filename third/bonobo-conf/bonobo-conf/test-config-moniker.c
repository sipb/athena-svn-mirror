/*
 * test-config-moniker.c: configuration moniker tests
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include <bonobo.h>
#include <bonobo-config-utils.h>

#if 0
static void
create_enum (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Unknown v;
	CORBA_any *value;

	v = bonobo_get_object (moniker, "IDL:Bonobo/Property:1.0", ev);
	if (BONOBO_EX(ev) || !v) {
		printf ("ERR %s\n", bonobo_exception_get_text (ev));
		g_error ("Couldn't get Bonobo/Property interface");
	}

	value = bonobo_arg_new (TC_Bonobo_StorageType);
	BONOBO_ARG_SET_GENERAL (value, Bonobo_STORAGE_TYPE_REGULAR,
				TC_Bonobo_StorageType, long, NULL); 

	Bonobo_Property_setValue (v, value, ev);
}

static void
create_struct (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Unknown v;
	CORBA_any *value;

	v = bonobo_get_object (moniker, "IDL:Bonobo/Property:1.0", ev);
	if (BONOBO_EX(ev) || !v) {
		printf ("ERR %s\n", bonobo_exception_get_text (ev));
		g_error ("Couldn't get Bonobo/Property interface");
	}

	value = bonobo_arg_new (TC_Bonobo_StorageInfo);

	Bonobo_Property_setValue (v, value, ev);
}

static void
create_string_list (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Unknown v;
	CORBA_any *value;
	DynamicAny_DynSequence dyn;

	v = bonobo_get_object (moniker, "IDL:Bonobo/Property:1.0", ev);
	if (BONOBO_EX(ev) || !v) {
		printf ("ERR %s\n", bonobo_exception_get_text (ev));
		g_error ("Couldn't get Bonobo/Property interface");
	}

	dyn = CORBA_ORB_create_dyn_sequence (bonobo_orb (), 
					     TC_CORBA_sequence_CORBA_string,
					     ev);

	DynamicAny_DynSequence_set_length (dyn, 2, ev);
	
	value = DynamicAny_DynAny_to_any (dyn, ev);

	CORBA_Object_release ((CORBA_Object) dyn, ev);

	Bonobo_Property_setValue (v, value, ev);
}

static void
create_complex_list (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Unknown v;
	CORBA_any *value;
	DynamicAny_DynSequence dyn;
	
	v = bonobo_get_object (moniker, "IDL:Bonobo/Property:1.0", ev);
	if (BONOBO_EX(ev) || !v) {
		printf ("ERR %s\n", bonobo_exception_get_text (ev));
		g_error ("Couldn't get Bonobo/Property interface");
	}

	dyn = CORBA_ORB_create_dyn_sequence (bonobo_orb (), 
	        TC_CORBA_sequence_Bonobo_StorageInfo, ev);

	DynamicAny_DynSequence_set_length (dyn, 3, ev);
	
	value = DynamicAny_DynAny_to_any (dyn, ev);

	CORBA_Object_release ((CORBA_Object) dyn, ev);

	Bonobo_Property_setValue (v, value, ev);
}
#endif

static void 
listener_callback (BonoboListener    *listener,
		   char              *event_name, 
		   CORBA_any         *any,
		   CORBA_Environment *ev,
		   gpointer           user_data)
{
	BonoboUINode      *node;
	gchar             *enc;

	node = bonobo_property_bag_xml_encode_any (NULL, any, ev);
	enc = bonobo_ui_node_to_string (node, TRUE);

	printf ("got event %s value %s\n", event_name, enc);

	bonobo_ui_node_free (node);

        bonobo_ui_node_free_string (enc);
}

static void
test_moniker (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Unknown bag, v;
	CORBA_any *value;
	CORBA_TypeCode tc;
	CORBA_char *name, *doc;
	Bonobo_PropertyNames *names;
	Bonobo_PropertyList *plist;
	gint i;

	printf("TEST0\n");

	bag = bonobo_get_object (moniker, "IDL:Bonobo/PropertyBag:1.0", ev);
	if (BONOBO_EX(ev) || !bag) {
		printf ("ERR %s\n", bonobo_exception_get_text (ev));
		g_error ("Couldn't get Bonobo/PropertyBag interface");
	}

	bonobo_event_source_client_add_listener (bag, listener_callback, NULL, 
						 NULL, NULL); 

	printf("TEST01\n");
	names = Bonobo_PropertyBag_getPropertyNames (bag, ev);
	if (BONOBO_EX (ev))
		return;

	for (i = 0; i < names->_length; i++)
		printf ("FOUND name: %s\n", names->_buffer [i]);

	CORBA_free (names);

	printf("TEST1\n");

	plist = Bonobo_PropertyBag_getProperties (bag, ev);
	if (BONOBO_EX (ev))
		return;

	for (i = 0; i < plist->_length; i++) {

		name = Bonobo_Property_getName (plist->_buffer [i], ev);
		if (BONOBO_EX (ev))
			return;

		printf ("property name: %s\n", name);
	}

	CORBA_free (plist);

	printf("TEST2\n");

	v = Bonobo_PropertyBag_getPropertyByName (bag, "value2", ev);
	if (BONOBO_EX (ev))
		return;
	
	printf("TEST3\n");
	name = Bonobo_Property_getName (v, ev);
	if (BONOBO_EX (ev))
		return;

	printf("TEST4\n");
	printf ("property name: %s\n", name);
	if (name)
		CORBA_free (name);

	value = bonobo_arg_new (BONOBO_ARG_INT);
	BONOBO_ARG_SET_LONG (value, 17);

	Bonobo_Property_setValue (v, value, ev);
	if (BONOBO_EX (ev))
		return;

	BONOBO_ARG_SET_LONG (value, 19);

	Bonobo_Property_setValue (v, value, ev);
	if (BONOBO_EX (ev))
		return;

	doc = Bonobo_Property_getDocString (v, ev);	
	if (BONOBO_EX (ev))
		return;

	printf("DocString: %s\n", doc);

	tc = Bonobo_Property_getType (v, ev);
	if (BONOBO_EX (ev))
		return;

	if (!bonobo_arg_type_is_equal(tc, BONOBO_ARG_LONG, NULL)) {
		printf ("Wrong TypeCode %p\n", value->_type);
		return;
	}

	value = Bonobo_Property_getValue (v, ev);
	if (BONOBO_EX (ev))
		return;

	if (bonobo_arg_type_is_equal(value->_type, BONOBO_ARG_LONG, NULL))
		printf ("Value: %d\n", BONOBO_ARG_GET_LONG(value));
	else
		printf ("Property is not long %p\n", value->_type);
	
}

static void
test2_moniker (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Unknown v;
	CORBA_any *value;
	CORBA_TypeCode tc;
	CORBA_char *name, *doc;

	printf("XTEST1\n");
	v = bonobo_get_object (moniker, "IDL:Bonobo/Property:1.0", ev);
	if (BONOBO_EX(ev) || !v) {
		printf ("ERR %s\n", bonobo_exception_get_text (ev));
		g_error ("Couldn't get Bonobo/Property interface");
	}

	bonobo_event_source_client_add_listener (v, listener_callback, NULL, 
						 NULL, NULL); 

	printf("XTEST2\n");
	name = Bonobo_Property_getName (v, ev);
	if (BONOBO_EX (ev))
		return;

	printf("XTEST3\n");
	printf ("property name: %s\n", name);
	CORBA_free (name);

	value = bonobo_arg_new (BONOBO_ARG_INT);
	BONOBO_ARG_SET_LONG (value, 17);

	Bonobo_Property_setValue (v, value, ev);
	if (BONOBO_EX (ev))
		return;

	BONOBO_ARG_SET_LONG (value, 19);

	Bonobo_Property_setValue (v, value, ev);
	if (BONOBO_EX (ev))
		return;

	doc = Bonobo_Property_getDocString (v, ev);	
	if (BONOBO_EX (ev))
		return;

	printf("XDocString: %s\n", doc);

	tc = Bonobo_Property_getType (v, ev);
	if (BONOBO_EX (ev))
		return;

	if (!bonobo_arg_type_is_equal(tc, BONOBO_ARG_LONG, NULL)) {
		printf ("Wrong TypeCode %p\n", value->_type);
		return;
	}
	printf ("TypeCode %p\n", tc->name);

	value = Bonobo_Property_getValue (v, ev);
	if (BONOBO_EX (ev))
		return;

	if (bonobo_arg_type_is_equal(value->_type, BONOBO_ARG_LONG, NULL))
		printf ("Value: %d\n", BONOBO_ARG_GET_LONG(value));
	else
		printf ("Property is not long %p\n", value->_type);
	
}

#if 0
static void
test3_moniker (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Unknown v;
	CORBA_any *value;

	printf("YTEST1\n");
	v = bonobo_get_object (moniker, "IDL:Bonobo/Property:1.0", ev);
	if (BONOBO_EX(ev) || !v) {
		printf ("ERR %s\n", bonobo_exception_get_text (ev));
		g_error ("Couldn't get Bonobo/Property interface");
	}

	bonobo_event_source_client_add_listener (v, listener_callback, NULL, 
						 NULL, NULL); 


	value = bonobo_arg_new (TC_Bonobo_StorageType);
	BONOBO_ARG_SET_GENERAL (value, Bonobo_STORAGE_TYPE_REGULAR,
				TC_Bonobo_StorageType, long, NULL); 

	Bonobo_Property_setValue (v, value, ev);
	if (BONOBO_EX (ev))
		return;

}
#endif

void
run_tests ()
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	{
		CORBA_any *value = CORBA_any_alloc();
		BonoboUINode *node;
		char *text;
		/* Bonobo_PropertySet *set; */

		/*
		set = Bonobo_PropertySet__alloc ();
		set->_length = 1;
		set->_buffer = CORBA_sequence_Bonobo_Pair_allocbuf (1);

		value = bonobo_arg_new (TC_Bonobo_Pair);

		set->_buffer [0].name = "test";
		set->_buffer [0].value._type = value->_type;
		set->_buffer [0].value._value = value->_value;
		
		value->_type  = TC_Bonobo_PropertySet;
		value->_value = set;
		*/

		/* value = bonobo_arg_new (TC_string); */
		value = bonobo_arg_new (TC_Bonobo_PropertySet);
		node = bonobo_config_xml_encode_any (value, "test", &ev);

		text = bonobo_ui_node_to_string (node, TRUE);

		printf ("VALUE:\n%s\n", text);

		exit (0);

	}


	test_moniker ("xmldb:/tmp/y.xmldb#config:/test3", &ev);

	if (!BONOBO_EX (&ev))
		test2_moniker ("xmldb:/tmp/y.xmldb#config:/test3/test2/value5",
			       &ev);

	/*
	if (!BONOBO_EX (&ev))
	    test2_moniker ("config:/test3/test2/value1", &ev);
	if (!BONOBO_EX (&ev))
	    test3_moniker ("config:/test3/test2/value2", &ev);
	*/

	
	
	gtk_main_quit ();
}

int
main (int argc, char **argv)
{
	CORBA_ORB orb;

	gnome_init ("moniker-test", "0.0", argc, argv);

	if ((orb = oaf_init (argc, argv)) == NULL)
		g_error ("Cannot init oaf");

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Cannot init bonobo");

	gtk_idle_add ((GtkFunction) run_tests, NULL);

	bonobo_main ();

	return 0;
}


