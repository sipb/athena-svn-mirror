#include <stdlib.h>
#include <string.h>

#include <bonobo.h>

#include "bonobo-conf/bonobo-config-utils.h"

static char string_7bit[] = "This is a 7bit string";
static char string_8bit[] = "This has 8bit chars: Ææø";
static char string_utf8[] = "FIXME: add a UTF-8 string here";

static void
test_xml_strings ()
{
	BonoboUINode *node;
	CORBA_Environment ev;
	BonoboArg *arg;
	char *string;
	
	CORBA_exception_init (&ev);
	
	/* test 7bit string */
	arg = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (arg, string_7bit);
	node = bonobo_config_xml_encode_any (arg, "string_7bit", &ev);
	g_assert (!BONOBO_EX (&ev));
	
	arg = bonobo_config_xml_decode_any (node, NULL, &ev);
	g_assert (!BONOBO_EX (&ev));
	string = BONOBO_ARG_GET_STRING (arg);
	g_assert (!strcmp (string, string_7bit));
	
	/* test 8bit string */
	arg = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (arg, string_8bit);
	node = bonobo_config_xml_encode_any (arg, "string_8bit", &ev);
	g_assert (!BONOBO_EX (&ev));
	
	arg = bonobo_config_xml_decode_any (node, NULL, &ev);
	g_assert (!BONOBO_EX (&ev));
	string = BONOBO_ARG_GET_STRING (arg);
	g_assert (!strcmp (string, string_8bit));
	
	/* test unicode string */
	arg = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (arg, string_utf8);
	node = bonobo_config_xml_encode_any (arg, "string_utf8", &ev);
	g_assert (!BONOBO_EX (&ev));
	
	arg = bonobo_config_xml_decode_any (node, NULL, &ev);
	g_assert (!BONOBO_EX (&ev));
	string = BONOBO_ARG_GET_STRING (arg);
	g_assert (!strcmp (string, string_utf8));
}

static gint
run_tests ()
{
	test_xml_strings ();

	gtk_main_quit ();

	return 0;
}

int
main (int argc, char **argv)
{
	CORBA_ORB orb;
	
	gnome_init ("test-xml-strings", "0.0", argc, argv);
	
	if ((orb = oaf_init (argc, argv)) == NULL)
		g_error ("Cannot init oaf");
	
	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Cannot init bonobo");
	
	gtk_idle_add ((GtkFunction) run_tests, NULL);
	
	bonobo_main ();
	
	exit (0);
}
