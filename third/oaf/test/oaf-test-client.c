/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include <liboaf/liboaf.h>
#include <liboaf/oaf-async.h>
#include <liboaf/liboaf-private.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "empty.h"

#define TOTAL_TEST_SCORE 15

CORBA_Object name_service = CORBA_OBJECT_NIL;

static char *
oaf_exception_id (CORBA_Environment *ev)
{
        if (ev->_major == CORBA_USER_EXCEPTION) {
                if (!strcmp (ev->_repo_id, "IDL:OAF/GeneralError:1.0")) {
                        OAF_GeneralError *err = ev->_params;
                        
                        if (!err || !err->description) {
                                return "No general exception error message";
                        } else {
                                return err->description;
                        }
                } else {
                        return ev->_repo_id;
                }
        } else {
                return CORBA_exception_id (ev);
        }
}

static gboolean
test_oafd (CORBA_Environment *ev, const char *type)
{
        CORBA_Object ns;

        ns = oaf_name_service_get (ev);
        if (ev->_major != CORBA_NO_EXCEPTION) {
                g_warning ("Exception '%s' (%s) finding oafd %s",
                           oaf_exception_id (ev), ev->_repo_id, type);
                return FALSE;
        }

        if (name_service != CORBA_OBJECT_NIL &&
            name_service != ns) {
                g_warning ("oafd crashed %s", type);
                return FALSE;
        }

        name_service = ns;

        return TRUE;
}

static gboolean
test_object (CORBA_Object obj, CORBA_Environment *ev, const char *type)
{
	if (CORBA_Object_is_nil (obj, ev)) {
		g_warning ("Activation %s failed!", type);

	} else if (ev->_major != CORBA_NO_EXCEPTION) {
		g_warning ("Activation %s failed: %s\n", type,
			   oaf_exception_id (ev));
	} else {
                return TRUE;
        }

        if (!test_oafd (ev, type)) {
                return FALSE;
        }

        return FALSE;
}

static int
test_empty (CORBA_Object obj, CORBA_Environment *ev, const char *type)
{
        Empty_doNothing (obj, ev);

        if (ev->_major != CORBA_NO_EXCEPTION) {
                g_warning ("Call failed: %s\n",
                           oaf_exception_id (ev));
                return 0;
        } else {
                fprintf (stderr, "Test %s succeeded\n", type);
                return 1;
        }
}


static int
idle_base_activation (gpointer user_data)
{
        /* This is a facile test, we always activate the
         * ActivationContext first and then get the OD from it */
        oaf_activation_context_get ();

        return FALSE;
}

static void
race_base_init (void)
{
        g_idle_add (idle_base_activation, NULL);
        /* to race with the activation context get in the same process */
        oaf_object_directory_get (NULL, NULL, NULL);
}

int passed = 0;
int async_done = 0;

static void
empty_activation_cb (CORBA_Object   obj,
                     const char    *error_reason, 
                     gpointer       user_data)
{
        CORBA_Environment ev;

        CORBA_exception_init (&ev);

        if (error_reason)
                g_warning ("Async activation error '%s'", error_reason);

        else if (test_object (obj, &ev, "by async query"))
                passed += test_empty (obj, &ev, "by async query");

        CORBA_exception_free (&ev);

        async_done++;
}

static void
race_empty (CORBA_Environment *ev)
{
	oaf_activate_async (
                "repo_ids.has('IDL:Empty2:1.0')", NULL,
                0, empty_activation_cb, NULL, ev);
        g_assert (ev->_major == CORBA_NO_EXCEPTION);

	oaf_activate_async (
                "repo_ids.has('IDL:Empty:1.0')", NULL,
                0, empty_activation_cb, NULL, ev);
        g_assert (ev->_major == CORBA_NO_EXCEPTION);

        while (async_done < 2)
                g_main_iteration (TRUE);
}

int
main (int argc, char *argv[])
{
	CORBA_Object obj;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	oaf_init (argc, argv);

/*      putenv("OAF_BARRIER_INIT=1"); */

        race_base_init ();
        race_empty (&ev);

	obj = oaf_activate ("repo_ids.has('IDL:Empty:1.0')", NULL, 0, NULL,
                            &ev);
        if (test_object (obj, &ev, "by query")) {
                passed += test_empty (obj, &ev, "by query");
        }


	obj = oaf_activate_from_id ("OAFIID:Empty:19991025", 0, NULL, &ev);
        if (test_object (obj, &ev, "from id")) {
                passed += test_empty (obj, &ev, "from id");
        }


	obj = oaf_activate_from_id ("OAFAID:[OAFIID:Empty:19991025]", 0, NULL, &ev);
        if (test_object (obj, &ev, "from aid")) {
                passed += test_empty (obj, &ev, "from aid");
        }


        fprintf (stderr, "Broken link test ");
        obj = oaf_activate_from_id ("OAFIID:Bogus:20000526", 0, NULL, &ev);
        if (ev._major == CORBA_NO_EXCEPTION) {
                fprintf (stderr, "failed 1");
        } else {
                fprintf (stderr, "passed 1 ('%s')", oaf_exception_id (&ev));
                CORBA_exception_free (&ev);
                passed++;
        }
        if (test_oafd (&ev, "with broken factory link")) {
                fprintf (stderr, ", passed 2");
                passed++;
        } else {
                fprintf (stderr, ", failed 2");
        }
        fprintf (stderr, "\n");


        fprintf (stderr, "Broken exe test ");
        obj = oaf_activate_from_id ("OAFIID:Broken:20000530", 0, NULL, &ev);
        if (ev._major == CORBA_NO_EXCEPTION) {
                fprintf (stderr, "failed 1");
        } else {
                fprintf (stderr, "passed 1 ('%s')", oaf_exception_id (&ev));
                CORBA_exception_free (&ev);
                passed++;
        }
        if (test_oafd (&ev, "with broken factory link")) {
                fprintf (stderr, ", passed 2");
                passed++;
        } else {
                fprintf (stderr, ", failed 2");
        }
        fprintf (stderr, "\n");


        fprintf (stderr, "Circular link test ");
        obj = oaf_activate_from_id ("OAFIID:Circular:20000530", 0, NULL, &ev);
        if (ev._major == CORBA_NO_EXCEPTION)
                fprintf (stderr, "failed 1");
        else {
                fprintf (stderr, "passed 1 ('%s')", oaf_exception_id (&ev));
                CORBA_exception_free (&ev);
                passed++;
        }
        if (test_oafd (&ev, "with broken factory link")) {
                fprintf (stderr, ", passed 2");
                passed++;
        } else {
                fprintf (stderr, ", failed 2");
        }
        fprintf (stderr, "\n");


        fprintf (stderr, "Server that doesn't register IID test ");
        obj = oaf_activate_from_id ("OAFIID:NotInServer:20000717", 0, NULL, &ev);
        if (ev._major == CORBA_NO_EXCEPTION) {
                fprintf (stderr, "failed 1");
        } else {
                fprintf (stderr, "passed 1 ('%s')", oaf_exception_id (&ev));
                CORBA_exception_free (&ev);
                passed++;
        }
        if (test_oafd (&ev, "with non-registering server")) {
                fprintf (stderr, ", passed 2");
                passed++;
        } else {
                fprintf (stderr, ", failed 2");
        }
        fprintf (stderr, "\n");

        fprintf (stderr, "Server with IID but no type or location ");
        obj = oaf_activate_from_id ("OAFIID:BrokenNoType:20000808", 0, NULL, &ev);
        if (ev._major == CORBA_NO_EXCEPTION) {
                fprintf (stderr, "failed (except) 1");
                CORBA_exception_free (&ev);
        } else if (obj) {
                fprintf (stderr, "failed (obj) 1");
        } else {
                fprintf (stderr, "passed 1 ('%s')", oaf_exception_id (&ev));
                passed++;
        }
        if (test_oafd (&ev, "with no-type/loc server")) {
                fprintf (stderr, ", passed 2");
                passed++;
        } else {
                fprintf (stderr, ", failed 2");
        }
        fprintf (stderr, "\n");

        fprintf (stderr, "\n%d of %d tests passed (%s)\n", passed,
                 TOTAL_TEST_SCORE,
                 passed == TOTAL_TEST_SCORE? "All": "some failures");

        if (passed < (TOTAL_TEST_SCORE * 2 / 3)) {
                fprintf (stderr, "It looks like you havn't installed broken.oafinfo "
                         "into ${prefix}/oaf, this must be done by hand to avoid "
                         "redundant warnings.\n");
		fprintf (stderr, "Another possibility is that you failed to kill "
			 "oafd before running make check; try running oaf-slay.\n");
        }

	CORBA_exception_free (&ev);

        if (passed == TOTAL_TEST_SCORE) {
                return 0;
        } else {
                return 1;
        }
}
