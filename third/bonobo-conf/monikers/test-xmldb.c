#include <stdlib.h>

#include <bonobo.h>

#include "bonobo-conf/bonobo-config-bag.h"
#include "bonobo-conf/bonobo-config-property.h"
#include "bonobo-config-xmldb.h"
#include "bonobo-config-dirdb.h"

static void
test_xmldb ()
{
	Bonobo_ConfigDatabase db = NULL;
        CORBA_Environment  ev;
	CORBA_any *value;

	db = bonobo_config_xmldb_new ("/tmp/t.xml");

	g_assert (db != NULL);

	value = bonobo_config_get_value (db, "/test", TC_long, &ev);

	if (value) {
		printf ("got value %d\n", BONOBO_ARG_GET_LONG (value));
	}
        CORBA_exception_init (&ev);

	value = bonobo_config_get_value (db, "/storagetype",
					 TC_Bonobo_StorageType, &ev);
	if (value) {
		printf ("got value\n");
	}
        CORBA_exception_init (&ev);

	value = bonobo_arg_new (TC_long);
	BONOBO_ARG_SET_LONG (value, 5);
	
	/* we cant set values on directory names */
	Bonobo_ConfigDatabase_setValue (db, "/adaf/safadsd/", value, &ev);
	g_assert (BONOBO_EX (&ev));
        CORBA_exception_init (&ev);

	Bonobo_ConfigDatabase_setValue (db, "/test", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/test/level2", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/test/level21", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/test/level3/level3", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/bonobo/test1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	value = bonobo_arg_new (TC_Bonobo_StorageType);
	
	Bonobo_ConfigDatabase_setValue (db, "/storagetype", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	value = bonobo_arg_new (TC_string);
	BONOBO_ARG_SET_STRING (value, "a simple test");
	Bonobo_ConfigDatabase_setValue (db, "a/b/c/d", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_removeDir (db, "/", &ev);
	g_assert (!BONOBO_EX (&ev));

	value = bonobo_arg_new (TC_long);
	BONOBO_ARG_SET_LONG (value, 5);
	
	Bonobo_ConfigDatabase_setValue (db, "/test", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/test/level2", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/test/level21", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/test/level3/level3", value, &ev);
	g_assert (!BONOBO_EX (&ev));


	value = bonobo_arg_new (TC_Bonobo_StorageType);
	Bonobo_ConfigDatabase_setValue (db, "/storagetype", value, &ev);
	g_assert (!BONOBO_EX (&ev));


	{
		gboolean r, def = FALSE, b = TRUE;

		bonobo_config_set_boolean (db, "/test/boolean", b, &ev);
		g_assert (!BONOBO_EX (&ev));

		r = bonobo_config_get_boolean (db, "/test/boolean", &ev);
		g_assert (!BONOBO_EX (&ev));
		g_assert (r == 1);

		r = bonobo_config_get_boolean_with_default (db, 
                        "/test/boolean", FALSE, &def);
		g_assert (!def);
		g_assert (r == 1);	
	}

	{
		char *str;

		str = "TEST";
		bonobo_config_set_string (db, "/test/string", str, &ev);
		g_assert (!BONOBO_EX (&ev));

		str = bonobo_config_get_string (db, "/test/string", &ev);
		g_assert (!BONOBO_EX (&ev));
		g_assert (!strcmp ("TEST", str));
	}

	{
		gint16 val = -5;

		bonobo_config_set_short (db, "/test/short", val, &ev);
		g_assert (!BONOBO_EX (&ev));

		val = bonobo_config_get_short (db, "/test/short", &ev);
		g_assert (!BONOBO_EX (&ev));
		g_assert (val == -5);
	}

	{
		guint16 val = 5;

		bonobo_config_set_ushort (db, "/test/ushort", val, &ev);
		g_assert (!BONOBO_EX (&ev));

		val = bonobo_config_get_ushort (db, "/test/ushort", &ev);
		g_assert (!BONOBO_EX (&ev));
		g_assert (val == 5);
	}

	{
		glong val = -15;

		bonobo_config_set_long (db, "/test/long", val, &ev);
		g_assert (!BONOBO_EX (&ev));

		val = bonobo_config_get_long (db, "/test/long", &ev);
		g_assert (!BONOBO_EX (&ev));
		g_assert (val == -15);
	}

	{
		gfloat val = 1.25;

		bonobo_config_set_float (db, "/test/float", val, &ev);
		g_assert (!BONOBO_EX (&ev));

		val = bonobo_config_get_float (db, "/test/float", &ev);
		g_assert (!BONOBO_EX (&ev));

		g_assert (((int)(val*1000)) == 1250);
	}

	{
		gdouble val = 1.125;

		bonobo_config_set_double (db, "/test/double", val, &ev);
		g_assert (!BONOBO_EX (&ev));

		val = bonobo_config_get_double (db, "/test/double", &ev);
		g_assert (!BONOBO_EX (&ev));

		g_assert (((int)(val*1000)) == 1125);
	}

	

	Bonobo_ConfigDatabase_sync (db, &ev);
	g_assert (!BONOBO_EX (&ev));
}

static void
read_data (Bonobo_ConfigDatabase db)
{
        CORBA_Environment ev;
	Bonobo_KeyList *klist;
	
        CORBA_exception_init (&ev);

	g_assert (bonobo_config_get_long (db, "/test", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "test", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "test////", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "/System/Test/t1", NULL) == 5);
	g_assert (bonobo_config_get_long (db, "/System/Test/t2", NULL) == 5);
	g_assert (bonobo_config_get_long (db, "/System/Test/t3", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "System///Test/t1", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "/System/Test//t1/", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "/System/t4", NULL) == 5);

	klist = Bonobo_ConfigDatabase_listDirs (db, "Internet", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (klist->_length == 5);

	klist = Bonobo_ConfigDatabase_listKeys (db, "Internet/d1", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (klist->_length == 1);

	klist = Bonobo_ConfigDatabase_listKeys (db, "Internet/d2", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (klist->_length == 0);

	g_assert (Bonobo_ConfigDatabase_dirExists (db,"/Internet/d1", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"/Internet/d2", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"/Internet/d3", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"/Internet/d4", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"/Internet/d5", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"/Internet/d5/d", &ev));
	g_assert (!Bonobo_ConfigDatabase_dirExists (db,"/Internet/d6", &ev));
	g_assert (!Bonobo_ConfigDatabase_dirExists (db,"/Internet/d7", &ev));

	printf ("read test finished\n"); 
}

static void
write_data (Bonobo_ConfigDatabase db)
{
        CORBA_Environment ev;
	CORBA_any *value;

        CORBA_exception_init (&ev);

	value = bonobo_arg_new (TC_long);
	BONOBO_ARG_SET_LONG (value, 5);
	
	Bonobo_ConfigDatabase_setValue (db, "/test", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/System/Test/t1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/System/Test/t2", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/System/Test/t3", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/System/t4", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/Application/a1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/Internet/d1/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/Internet/d2/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_removeValue (db, "/Internet/d2/i1", &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/Internet/d3/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/Internet/d4/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/Internet/d5/d/d/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/Internet/d6/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_removeDir (db, "/Internet/d6", &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_sync (db, &ev);
	g_assert (!BONOBO_EX (&ev));

	printf ("write test finished\n"); 
}

static void
test_xmldirdb ()
{
	Bonobo_ConfigDatabase db = NULL;
        CORBA_Environment  ev;
	
        CORBA_exception_init (&ev);

	system ("rm -fr /tmp/testxmldir");

	db = bonobo_config_dirdb_new ("/tmp/testxmldir");
	g_assert (db != NULL);

	write_data (db);

	bonobo_object_release_unref (db, &ev);
	g_assert (!BONOBO_EX (&ev));

	db = bonobo_config_dirdb_new ("/tmp/testxmldir");
	g_assert (db != NULL);

	read_data (db);
}

static gint
run_tests ()
{
	test_xmldirdb ();
	test_xmldb ();

	gtk_main_quit ();

	return 0;
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

	exit (0);
}
