/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <libbonobo.h>

static Bonobo_Unknown obj1, obj2;

static gboolean
timeout4 (gpointer data)
{
	printf ("timeout4\n");
	bonobo_main_quit ();
	return FALSE;
}

static gboolean
timeout3 (gpointer data)
{
	printf ("timeout3\n");
	bonobo_object_release_unref (obj2, NULL);
	g_timeout_add (2500, timeout4, NULL);
	return FALSE;
}

static gboolean
timeout2 (gpointer data)
{
	printf ("timeout2\n");
	obj2 = bonobo_activation_activate ("iid == 'OAFIID:Test_Generic_Factory'",
					   NULL, 0, NULL, NULL);
	g_assert (obj2 != CORBA_OBJECT_NIL);
	printf ("activate 2\n");

	g_timeout_add (1500, timeout3, NULL);
	return FALSE;
}

static gboolean
timeout1 (gpointer data)
{
	printf ("timeout1\n");
	bonobo_object_release_unref (obj1, NULL);
	g_timeout_add (1000, timeout2, NULL);
	return FALSE;
}


static gboolean
run_tests (gpointer data)
{
	obj1 = bonobo_activation_activate ("iid == 'OAFIID:Test_Generic_Factory'",
					   NULL, 0, NULL, NULL);
	g_assert (obj1 != CORBA_OBJECT_NIL);
	printf ("activate 1\n");
	g_timeout_add (2000, timeout1, NULL);
	return FALSE;
}

int
main (int argc, char **argv)
{
	if (!bonobo_init (&argc, argv))
		g_error ("Cannot init bonobo");
	g_idle_add (run_tests, NULL);
	bonobo_main ();
	return bonobo_debug_shutdown ();
}
