/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-httpd.c: Soup HTTPD server.
 *
 * Authors:
 *      Alex Graveley (alex@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <ctype.h>
#include <errno.h>
#include <glib.h>
#include <gmodule.h>
#include <popt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <libsoup/soup-private.h>
#include <libsoup/soup-server.h>

static int port = SOUP_SERVER_ANY_PORT;
static int ssl_port = SOUP_SERVER_ANY_PORT;

static SoupServer *server;
static SoupServer *ssl_server;

typedef void (*ModuleInitFn) (SoupServer *server);

static void
soup_httpd_load_module (const gchar *file)
{
	GModule *gmod;
	ModuleInitFn init;
	struct stat mod_stat;
	
	if (stat (file, &mod_stat) != 0) { 
		g_print ("ERROR.\n\tFile Not Found.\n");
		exit (1);
	}

	gmod = g_module_open (file, 0);
	if (!gmod) {
		g_print ("ERROR.\n\tUnable to load module \"%s\":\n\t%s\n", 
			 file,
			 g_module_error ());
		exit (1);
	}

	if (!g_module_symbol (gmod, "soup_server_init", (gpointer *) &init)) {
		g_print ("ERROR.\n\tModule \"%s\"\n"
			 "\thas no soup_server_init function.\n", 
			 file);
		exit (1);
	}

	(*init) (server);
	(*init) (ssl_server);

	g_print ("OK.\n");		
}

static struct poptOption options[] = {
        { "port", 
	  'p', 
	  POPT_ARG_INT, 
	  &port, 
	  0, 
	  "Server Port", 
	  "PORT" },
        { "ssl-port", 
	  '\0', 
	  POPT_ARG_INT, 
	  &ssl_port, 
	  0, 
	  "Server secure SSL Port", 
	  "PORT" },
	POPT_AUTOHELP
        { NULL, 0, 0, NULL, 0 }
};

int
main (int argc, const char **argv)
{
	poptContext ctx;
	int nextopt;
	const gchar **mod_names;
	GMainLoop *loop;

	if (!g_module_supported ()) {
		g_print ("Dynamically loadable modules not supported by the "
			 "version of GLIB installed on this system.\n");
		exit (1);
	}

	ctx = poptGetContext(argv [0], argc, argv, options, 0);
	poptSetOtherOptionHelp(ctx, "MODULE...");

        while ((nextopt = poptGetNextOpt (ctx)) > 0)
                /* do nothing */ ;

	if (nextopt != -1) {
		g_print("Error on option %s: %s.\nRun '%s --help' to see a "
			"full list of command line options.\n",
			poptBadOption (ctx, 0),
			poptStrerror (nextopt),
			argv [0]);
		exit (1);
	}

	mod_names = poptGetArgs (ctx);

	if (!mod_names) {
		g_print ("No soup server modules specified.\n");
		poptPrintUsage (ctx, stderr, 0);
		exit (1);
	}

	server = soup_server_new (SOUP_PROTOCOL_HTTP, port);
	if (!server) {
		g_print ("Unable to bind to server port %d\n", port);
		exit (1);
	}

	ssl_server = soup_server_new (SOUP_PROTOCOL_HTTPS, ssl_port);
	if (!ssl_server) {
		g_print ("Unable to bind to SSL server port %d\n", ssl_port);
		exit (1);
	}

	while (*mod_names) {
		g_print ("Loading module \"%s\"... ", *mod_names);
		soup_httpd_load_module (*mod_names);
		mod_names++;
	}

	poptFreeContext (ctx);

	g_print ("\nStarting Server on port %d\n", 
		 soup_server_get_port (server));
	soup_server_run_async (server);

	g_print ("Starting SSL Server on port %d\n", 
		 soup_server_get_port (ssl_server));
	soup_server_run_async (ssl_server);

	g_print ("\nWaiting for requests...\n");

	loop = g_main_new (TRUE);
	g_main_run (loop);

	return 0;
}

/*
 * Dummy Apache method implementations, to allow for Soup apache modules to be
 * loaded by soup-httpd. These are never called.
 */

void ap_construct_url           (void);
void ap_table_get               (void);
void ap_table_do                (void);
void ap_setup_client_block      (void);
void ap_should_client_block     (void);
void ap_palloc                  (void);
void ap_hard_timeout            (void);
void ap_get_client_block        (void);
void ap_reset_timeout           (void);
void ap_kill_timeout            (void);
void ap_table_set               (void);
void ap_log_error               (void);
void ap_auth_type               (void);
void ap_get_basic_auth_pw       (void);
void ap_note_basic_auth_failure (void);
void ap_psprintf                (void);
void ap_send_http_header        (void);
void ap_rwrite                  (void);
void ap_ctx_get                 (void);
void ap_get_server_port         (void);

void
ap_construct_url (void)
{
}

void
ap_table_get (void)
{
}

void
ap_table_do (void)
{
}

void
ap_setup_client_block (void)
{
}

void
ap_should_client_block (void)
{
}

void
ap_palloc (void)
{
}

void
ap_hard_timeout (void)
{
}

void
ap_get_client_block (void)
{
}

void
ap_reset_timeout (void)
{
}

void
ap_kill_timeout (void)
{
}

void
ap_table_set (void)
{
}

void
ap_log_error (void)
{
}

void
ap_auth_type (void)
{
}

void
ap_get_basic_auth_pw (void)
{
}

void
ap_note_basic_auth_failure (void)
{
}

void
ap_psprintf (void)
{
}

void
ap_send_http_header (void)
{
}

void
ap_rwrite (void)
{
}

void
ap_ctx_get (void)
{
}

void
ap_get_server_port (void)
{
}

