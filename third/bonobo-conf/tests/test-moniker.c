#include <stdlib.h>

#include <bonobo.h>

#include "bonobo-conf/bonobo-config-database.h"

#define KEY_CALENDARS_TO_LOAD "/Calendar/AlarmNotify/CalendarsToLoad"

/* Tries to get the config database object; returns CORBA_OBJECT_NIL on failure. */
Bonobo_ConfigDatabase
get_config_db (void)
{
	CORBA_Environment ev;
	Bonobo_ConfigDatabase db;

	CORBA_exception_init (&ev);

	db = bonobo_get_object ("wombat:", "Bonobo/ConfigDatabase", &ev);
	if (BONOBO_EX (&ev) || db == CORBA_OBJECT_NIL) {
		g_message ("get_config_db(): Could not get the config database object '%s'",
			   bonobo_exception_get_text (&ev));
		db = CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return db;
}

void
discard_config_db (Bonobo_ConfigDatabase db)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	Bonobo_ConfigDatabase_sync (db, &ev);
	if (BONOBO_EX (&ev))
		g_message ("discard_config_db(): Got an exception during the sync command '%s'",
			   bonobo_exception_get_text (&ev));
	
	CORBA_exception_free (&ev);

	CORBA_exception_init (&ev);

	bonobo_object_release_unref (db, &ev);
	if (BONOBO_EX (&ev))
		g_message ("discard_config_db(): Could not release/unref the database '%s'",
			   bonobo_exception_get_text (&ev));

	CORBA_exception_free (&ev);
}

void
save_calendars_to_load (GPtrArray *uris)
{
	Bonobo_ConfigDatabase db;
	int len, i;
	Bonobo_Config_StringSeq *seq;
	CORBA_Environment ev;
	CORBA_any *any; 

	g_return_if_fail (uris != NULL);

	db = get_config_db ();
	if (db == CORBA_OBJECT_NIL)
		return;

	/* Build the sequence of URIs */

	len = uris->len;

	seq = Bonobo_Config_StringSeq__alloc ();
	seq->_length = len;
	seq->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (seq, TRUE);

	for (i = 0; i < len; i++) {
		char *uri;

		uri = uris->pdata[i];
		seq->_buffer[i] = CORBA_string_dup (uri);
	}

	/* Save it */

	any = bonobo_arg_new (TC_Bonobo_Config_StringSeq);
	any->_value = seq;

	CORBA_exception_init (&ev);

	bonobo_config_set_value (db, KEY_CALENDARS_TO_LOAD, any, &ev);

	if (ev._major != CORBA_NO_EXCEPTION)
		g_message ("save_calendars_to_load(): Could not save the list of calendars");

	CORBA_exception_free (&ev);

	discard_config_db (db);
	bonobo_arg_release (any); /* this releases the sequence */
}

GPtrArray *
get_calendars_to_load (void)
{
	Bonobo_ConfigDatabase db;
	Bonobo_Config_StringSeq *seq;
	CORBA_Environment ev;
	CORBA_any *any; 
	int len, i;
	GPtrArray *uris;

	db = get_config_db ();
	if (db == CORBA_OBJECT_NIL)
		return NULL;

	/* Get the value */

	CORBA_exception_init (&ev);

	any = bonobo_config_get_value (db, KEY_CALENDARS_TO_LOAD,
				       TC_Bonobo_Config_StringSeq,
				       &ev);
	discard_config_db (db);

	if (ev._major == CORBA_USER_EXCEPTION) {
		char *ex_id;

		ex_id = CORBA_exception_id (&ev);

		if (strcmp (ex_id, ex_Bonobo_ConfigDatabase_NotFound) == 0) {
			CORBA_exception_free (&ev);
			uris = g_ptr_array_new ();
			g_ptr_array_set_size (uris, 0);
			return uris;
		}
	}

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_message ("get_calendars_to_load(): Could not get the list of calendars");
		CORBA_exception_free (&ev);
		return NULL;
	}

	CORBA_exception_free (&ev);

	/* Decode the value */

	seq = any->_value;
	len = seq->_length;

	uris = g_ptr_array_new ();
	g_ptr_array_set_size (uris, len);

	for (i = 0; i < len; i++)
		uris->pdata[i] = g_strdup (seq->_buffer[i]);

	/*
	 * FIXME: The any and sequence are leaked.  If we release them this way,
	 * we crash inside the ORB freeing routines :(
	 */
	bonobo_arg_release (any);

	return uris;
}

static void
read_data (Bonobo_ConfigDatabase db)
{
        CORBA_Environment ev;
	Bonobo_KeyList *klist;
	
        CORBA_exception_init (&ev);

	g_assert (bonobo_config_get_long (db, "/t/test", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "/t/test", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "/t/test////", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "/t/System/Test/t1", NULL) == 5);
	g_assert (bonobo_config_get_long (db, "/t/System/Test/t2", NULL) == 5);
	g_assert (bonobo_config_get_long (db, "/t/System/Test/t3", NULL) == 5);

	g_assert (bonobo_config_get_long (db, "/t/System///Test/t1", NULL)==5);

	g_assert (bonobo_config_get_long (db, "/t/System/Test//t1/", NULL)==5);

	g_assert (bonobo_config_get_long (db, "/t/System/t4", NULL) == 5);

	klist = Bonobo_ConfigDatabase_listDirs (db, "t/Internet", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (klist->_length == 5);

	klist = Bonobo_ConfigDatabase_listKeys (db, "t/Internet/d1", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (klist->_length == 1);

	klist = Bonobo_ConfigDatabase_listKeys (db, "t/Internet/d2", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (klist->_length == 0);

	g_assert (Bonobo_ConfigDatabase_dirExists (db,"t/Internet/d1", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"t/Internet/d2", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"t/Internet/d3", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"t/Internet/d4", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"t/Internet/d5", &ev));
	g_assert (Bonobo_ConfigDatabase_dirExists (db,"t/Internet/d5/d", &ev));
	g_assert (!Bonobo_ConfigDatabase_dirExists (db,"t/Internet/d6", &ev));
	g_assert (!Bonobo_ConfigDatabase_dirExists (db,"t/Internet/d7", &ev));

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
	
	Bonobo_ConfigDatabase_setValue (db, "/t/test", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/System/Test/t1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/System/Test/t2", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/System/Test/t3", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/System/t4", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/Application/a1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/Internet/d1/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/Internet/d2/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_removeValue (db, "/t/Internet/d2/i1", &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/Internet/d3/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/Internet/d4/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/Internet/d5/d/d/i1",value,&ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_setValue (db, "/t/Internet/d6/i1", value, &ev);
	g_assert (!BONOBO_EX (&ev));

	Bonobo_ConfigDatabase_removeDir (db, "/t/Internet/d6", &ev);
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

	//db = bonobo_get_object ("config:", "Bonobo/ConfigDatabase", &ev);
	db = bonobo_get_object ("xmldirdb:/tmp/t.xmldir", 
				"Bonobo/ConfigDatabase", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (db != NULL);

	write_data (db);

	read_data (db);

	bonobo_object_release_unref (db, &ev);
	g_assert (!BONOBO_EX (&ev));
}

static void
test_subdb ()
{
	Bonobo_ConfigDatabase db = NULL;
        CORBA_Environment  ev;
	
        CORBA_exception_init (&ev);

	db = bonobo_get_object ("xmldirdb:/tmp/t.xmldir#config:subdir1", 
				"Bonobo/ConfigDatabase", &ev);
	g_assert (!BONOBO_EX (&ev));
	g_assert (db != NULL);

	write_data (db);

	read_data (db);

	bonobo_object_release_unref (db, &ev);
	g_assert (!BONOBO_EX (&ev));
}

static gint
run_tests ()
{
	GPtrArray *strs;
	int        i;

	test_xmldirdb ();
	test_subdb ();

	strs = g_ptr_array_new ();
	for (i = 0; i < 10; i++)
		g_ptr_array_add (strs, g_strdup ("Foo"));

	g_warning ("Save calendar");
	save_calendars_to_load (strs);
	g_warning ("Load calendar");
	get_calendars_to_load ();

	gtk_main_quit ();

	return 0;
}

int
main (int argc, char **argv)
{
	CORBA_ORB orb;

	gnome_init ("test-moniker", "0.0", argc, argv);

	if ((orb = oaf_init (argc, argv)) == NULL)
		g_error ("Cannot init oaf");

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Cannot init bonobo");

	gtk_idle_add ((GtkFunction) run_tests, NULL);

	bonobo_main ();

	exit (0);
}
