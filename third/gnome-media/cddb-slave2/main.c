/*
 * CDDBSlave 2
 *
 * Copyright (C) 2001-2002 Iain Holmes
 *
 * Authors: Iain Holmes  <iain@ximian.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <bonobo-activation/bonobo-activation.h>
#include <libbonobo.h>

#include <gconf/gconf-client.h>

#include "cddb-slave.h"

#define CDDBSLAVE_IID "OAFIID:GNOME_Media_CDDBSlave2_Factory"
#define CDDB_SERVER "freedb.freedb.org"
#define CDDB_PORT 888;

static int running_objects = 0;
static GConfClient *client = NULL;

gboolean cddb_debugging = FALSE;

enum {
	CDDB_SEND_FAKE_INFO,
	CDDB_SEND_REAL_INFO,
	CDDB_SEND_OTHER_INFO
};

static void
cddb_destroy_cb (GObject *cddb,
		 gpointer data)
{
	running_objects--;

	if (running_objects <= 0) {
		bonobo_main_quit ();
	}
}

static char *
get_hostname (void)
{
	char name[4096];

	if (gethostname (name, 4095) == -1) {
		return g_strdup ("localhost");
	} else {
		return g_strdup (name);
	}
}
	
static BonoboObject *
factory_fn (BonoboGenericFactory *factory,
	    const char *component_id,
	    void *closure)
{
	CDDBSlave *cddb;
	gboolean auth;
	char *server;
	int port;
	int info, server_type;
	char *name;
	char *hostname;
	
	/* Get GConf db */
	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	info = gconf_client_get_int (client, "/apps/CDDB-Slave2/info", NULL);
	switch (info) {
	case CDDB_SEND_FAKE_INFO:
		name = g_strdup ("unknown");
		hostname = g_strdup ("localhost");
		break;

	case CDDB_SEND_REAL_INFO:
		name = g_strdup (g_get_user_name ());
		hostname = get_hostname ();
		break;

	case CDDB_SEND_OTHER_INFO:
		name = g_strdup (gconf_client_get_string (client,
							  "/apps/CDDB-Slave2/name", NULL));
		hostname = g_strdup (gconf_client_get_string (client,
							      "/apps/CDDB-Slave2/hostname", NULL));
		break;

	default:
		break;
	}

	server = gconf_client_get_string (client, 
					  "/apps/CDDB-Slave2/server", NULL);
	port = gconf_client_get_int (client, "/apps/CDDB-Slave2/port", NULL);

	/* Create the new slave */
	cddb = cddb_slave_new (server, port, name, hostname);
	g_free (name);
	g_free (hostname);

	if (cddb == NULL) {
		g_error ("Could not create CDDB slave");
		return NULL;
	}

	/* Keep track of our objects */
	running_objects++;
	g_signal_connect (G_OBJECT (cddb), "destroy",
			  G_CALLBACK (cddb_destroy_cb), NULL);

	return BONOBO_OBJECT (cddb);
}

static gboolean
cddbslave_init (gpointer data)
{
	BonoboGenericFactory *factory;

	factory = bonobo_generic_factory_new (CDDBSLAVE_IID, factory_fn, NULL);
	if (factory == NULL) {
		g_error ("Cannot create factory");
		exit (1);
	}

	bonobo_running_context_auto_exit_unref (BONOBO_OBJECT (factory));

	return FALSE;
}

int 
main (int argc,
      char **argv)
{
	CORBA_ORB orb;
	char *cddbdir;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gnome_program_init ("CDDBSlave2", VERSION, LIBGNOMEUI_MODULE,
			    argc, argv, NULL);

	if (g_getenv ("CDDB_SLAVE_DEBUG")) {
		cddb_debugging = TRUE;
		g_print ("CDDBSlave2 debugging turned on.\n");
	}
	cddbdir = g_build_filename (g_get_home_dir (),
				    ".cddbslave", NULL);
	if (g_file_test (cddbdir, G_FILE_TEST_EXISTS) == FALSE) {
		mkdir (cddbdir, 0755);
	}

	if (g_file_test (cddbdir, G_FILE_TEST_IS_DIR) == FALSE) {
		g_error ("~/.cddbslave needs to be a directory");
	}
	g_free (cddbdir);
	
	g_idle_add (cddbslave_init, NULL);
	bonobo_main ();

	exit (0);
}
