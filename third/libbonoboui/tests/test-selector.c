/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <config.h>
#include <libbonoboui.h>

int
main (int argc, char *argv[])
{
	gchar *text;

	gnome_program_init ("bonobo-selector", VERSION,
			    LIBBONOBOUI_MODULE,
			    argc, argv, NULL);

	text = bonobo_selector_select_id (_("Select an object"), NULL);
	g_print ("OAFIID: '%s'\n", text ? text : "<Null>");

	g_free (text);
	
	return bonobo_ui_debug_shutdown ();
}
