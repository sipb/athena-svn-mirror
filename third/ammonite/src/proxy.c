/* $Id: proxy.c,v 1.1.1.1 2001-01-16 15:26:27 ghudson Exp $
 *
 * A trivial proxy server for relaying Eazel HTTP connections.  Sets up
 * initial config options, daemonizes if necessary, and then stays in a
 * socket event loop until death.
 *
 * Copyright (C) 1998  Steven Young
 * Copyright (C) 1999  Robert James Kaes (rjkaes@flarenet.com)
 * Copyright (C) 2000  Chris Lightfoot (chris@ex-parrot.com)
 * Copyright (C) 2000  Eazel, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Authors: Steven Young
 *          Robert James Kaes <rjkaes@flarenet.com>
 *          Chris Lightfoot <chris@ex-parrot.com>
 *          Robey Pointer <robey@eazel.com>
 *          Mike Fleming <mfleming@eazel.com>
 */

#undef CHECK_FOR_LEAKS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sysexits.h>
#include <popt.h>
#include <glib.h>

#ifndef SIMPLE_PROXY
#include <liboaf/liboaf.h>
#endif /* SIMPLE_PROXY */

#ifdef CHECK_FOR_LEAKS
#include <dlfcn.h>
#endif

#include "sock.h"
#include "proxy.h"
#include "log.h"
#include "utils.h"
#ifndef SIMPLE_PROXY
#include "impl-eazelproxy.h"
#endif /* SIMPLE_PROXY */
#include "sock-ssl.h"
#include "request.h"
#include "digest.h"

#ifndef SIMPLE_PROXY
#include "util-gconf.h"
#endif /* SIMPLE_PROXY */

#define PORT_HTTP_DEFAULT 80
#define PORT_HTTPS_DEFAULT 443
#define PATH_GCONF_GNOME_VFS "/system/gnome-vfs/"
#define ITEM_GCONF_HTTP_PROXY "http-proxy"
#define KEY_GCONF_HTTP_PROXY (PATH_GCONF_GNOME_VFS ITEM_GCONF_HTTP_PROXY)

#define KEY_GCONF_DEFAULT_SSL_CERT "/apps/eazel-trilobite/ssl-cert"
#define KEY_GCONF_DEFAULT_DISABLE_SSL "/apps/eazel-trilobite/ssl-disable"

#define EAZEL_DEFAULT_CERT "/nautilus/certs/eazel_dsa_cert.pem"

void takesig(int sig);


/* global config */
Config config = {
	NULL,		/* logfile handle */
	NULL,		/* logfile name */
        NULL,		/* target path */
	NULL,		/* upstream proxy host */
	PORT_HTTP_DEFAULT,/* upstream proxy port */
	FALSE, 		/* was -u specified? */
	0,		/* local port -- for debug only -- if 0, search */
	FALSE,		/* append to logfile (instead of truncate)? */
	FALSE,		/* use stderr for logging */
	FALSE,		/* use ssl for outbound connections (only when built against openssl) */
	TRUE,		/* use OAF */
	FALSE		/* quit/terminate */
};

struct stat_s stats;

GMainLoop *l_main_loop = NULL;

char *gl_opt_certfile = NULL;
int gl_opt_no_ssl = 0;
gboolean gl_gconf_ssl_disabled = FALSE;
char *gl_gconf_ssl_cert = NULL;


static const struct poptOption popt_options[] = {
	{"target-path", 't', POPT_ARG_STRING, NULL, 't', "force all web pages to a specific site", "url"},
	{"logfile", 'l', POPT_ARG_STRING, NULL, 'l', "log to a logfile", "filename"},
	{"append", 'L', POPT_ARG_NONE, NULL, 'L', "append to logfile instead of truncating", NULL},
#ifndef SIMPLE_PROXY
	{"no-oaf", 'n', POPT_ARG_NONE, NULL, 'n', "don't use OAF or CORBA hooks", NULL},
#ifdef HAVE_OPENSSL
	{"no-ssl", '\0', POPT_ARG_NONE, &gl_opt_no_ssl, 0, "Do not make outbound connections use SSL", NULL },
	{"ssl-cert", '\0', POPT_ARG_STRING, &gl_opt_certfile, 0, "SSL certificate to use to verify server ", "cert-file" },
#endif /* HAVE_OPENSSL */
#endif /* SIMPLE_PROXY */
	{"upstream-proxy", 'u', POPT_ARG_STRING, NULL, 'u', "use an upstream proxy for connections",
		"host[:port]"},
#ifndef NO_DEBUG_MIRRORING
	{"no-daemon", 'd', POPT_ARG_NONE, NULL, 'd', "do not daemonize", NULL},
	{"stderr", 'e', POPT_ARG_NONE, NULL, 'e', "use stderr for logging (debug)", NULL},
	{"port", 'p', POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, NULL, 'p', "specify listen port number (debug)", "port"},
	{"mirror", 'm', POPT_ARG_STRING | POPT_ARGFLAG_DOC_HIDDEN, NULL, 'm', "log i/o, place files in directory (debug)", "log-directory"},
#endif /* DEBUG */
	{"version", 'v', POPT_ARG_NONE, NULL, 'v', "display version string", NULL},
#ifndef SIMPLE_PROXY
	{NULL, '\0', POPT_ARG_INCLUDE_TABLE, oaf_popt_options, 0, "OAF options", NULL},
#endif /* SIMPLE_PROXY */
	POPT_AUTOHELP
	{NULL, 'h', POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, NULL, 'h', "", NULL},
	{NULL, '\0', 0, NULL, 0, NULL}
};


#ifdef CHECK_FOR_LEAKS
static void
leak_checker_init (const char *path)
{
	void (*real_leak_checker_init) (const char *path);

	real_leak_checker_init = dlsym (RTLD_NEXT, "nautilus_leak_checker_init");
	if (real_leak_checker_init) {
		(*real_leak_checker_init) (path);
	}
}

static void
leak_checker_dump_internal (int stack_grouping_depth,
			    int stack_print_depth,
			    int max_count,
			    int sort_by_count)
{
	void (*real_leak_checker_dump) (int stack_grouping_depth,
					int stack_print_depth,
					int max_count,
					int sort_by_count);

	real_leak_checker_dump = dlsym (RTLD_NEXT, "nautilus_leak_print_leaks");
	if (real_leak_checker_dump) {
		(*real_leak_checker_dump) (stack_grouping_depth, stack_print_depth,
					   max_count, sort_by_count);
	}
}

static void
leak_checker_dump (void)
{
	leak_checker_dump_internal (8, 15, 100, TRUE);
}
#endif

/*
 * Handle a signal
 */
void takesig(int sig)
{
	switch (sig) {
	case SIGUSR1:
		socket_log_debug ();
		log ("::: stats: RX %d, TX %d", stats.num_rx, stats.num_tx);
		break;
	case SIGHUP:
		if (config.logf)
			ftruncate (fileno (config.logf), 0);
		log ("SIGHUP received, cleared log...");
		break;
	case SIGKILL:
	case SIGTERM:
		g_main_quit (l_main_loop);
		break;
	case SIGALRM:
		/* ignore */
		break;
	}
	if (sig != SIGTERM)
		signal (sig, takesig);
	signal (SIGPIPE, SIG_IGN);
}


static void display_version(void)
{
	printf("\n" PACKAGE " version " VERSION "\n");
#ifdef HAVE_OPENSSL
	fprintf (stderr, "\tincluding openSSL support\n");
#endif
	printf("\n");

	printf("Copyright 1998       Steven Young (sdyoung@well.com)\n");
	printf("Copyright 1998-1999  Robert James Kaes (rjkaes@flarenet.com)\n");
	printf("Copyright 2000       Chris Lightfoot (chris@ex-parrot.com)\n");
	printf("Copyright 2000       Eazel, Inc.\n");

	printf("This software is licensed under the GNU General Public License (GPL).\n");
	printf("See the file 'COPYING' included with " PACKAGE " source.\n\n");
}

#ifndef SIMPLE_PROXY
static void /*UtilGConfCb*/
watch_gconf_proxy_cb (
	const UtilGConfWatchVariable *watched, 
	const GConfValue *new_value
) {
	if (NULL == new_value) {
		g_free (config.upstream_host);
		config.upstream_host = NULL;
		log ("HTTP proxy changed to <none>");
	} else if (GCONF_VALUE_STRING == new_value->type) {
		char *p;

		g_free (config.upstream_host);
		config.upstream_host = g_strdup (gconf_value_get_string (new_value));
		if (strlen (config.upstream_host) == 0) {
			/* setting it to the empty string means unsetting it too */
			g_free (config.upstream_host);
			config.upstream_host = NULL;
		}

		if (NULL == config.upstream_host) {
			log ("HTTP proxy changed to <none>");
		} else {
			p = strchr (config.upstream_host, ':');
			if (p) {
				config.upstream_port = atoi (p+1);
				*p = 0;
			} else {
				config.upstream_port = PORT_HTTP_DEFAULT;
			}

			log("HTTP proxy changed to '%s:%d'", 
				config.upstream_host, 
				config.upstream_port
			);
		}
	}
}

static void 
load_gconf_defaults()
{
	static const UtilGConfWatchVariable to_watch[] = {
		{KEY_GCONF_HTTP_PROXY, GCONF_VALUE_STRING, {NULL}, watch_gconf_proxy_cb},
		{KEY_GCONF_DEFAULT_SSL_CERT, GCONF_VALUE_STRING, {(gchar **)&gl_gconf_ssl_cert}, NULL},
		{KEY_GCONF_DEFAULT_DISABLE_SSL, GCONF_VALUE_BOOL, {(gchar **) /*gboolean*/&gl_gconf_ssl_disabled}, NULL}
	};

	if ( ! config.specified_upstream_host ) {
		util_gconf_watch_variable (to_watch+0);
	}

	/* FIXME these don't have to be watched because we don't do anything when they change */
	util_gconf_watch_variable (to_watch+1);
	util_gconf_watch_variable (to_watch+2);
}

#endif /*SIMPLE_PROXY*/

/*
 * If we're running in -t mode, we need to translate DAV
 * Destination: headers
 * (just like in impl-eazel-proxy.c:user_proxy_request_cv
 */
static gpointer /* ProxyRequestCb */
simple_proxy_request_cb (gpointer user_data, unsigned short port, HTTPRequest *request, GList **p_header_list)
{
	GList * destination_node;

	
	if ( ! config.target_path ) {
		return NULL;
	}

	destination_node = g_list_find_custom (*p_header_list, "Destination: ", util_glist_string_starts_with);

	if (destination_node) {
		char *new_header;
		char *dest_url;
		HTTPRequest *target_request;
		HTTPRequest *dest_request;

		dest_url = (char *)(destination_node->data) + strlen ("Destination: ");

		dest_request = request_new();
		target_request = request_new();
		
		if ( request_parse_url (config.target_path, target_request ) 
			&& request_parse_url (dest_url, dest_request)
			&& dest_request->host 
			&& (0 == strcmp ("localhost", dest_request->host) || 0 == strcmp("127.0.0.1", dest_request->host))
			&& dest_request->port == port
		) {
			new_header = g_strdup_printf ("Destination: %s://%s:%d%s",
					dest_request->uri,
					target_request->host,
					target_request->port,
					dest_request->path);
			g_free (destination_node->data);
			destination_node->data = new_header;
		}
		
		request_free (target_request);
		request_free (dest_request);
	}


	return NULL;
}

int main(int argc, char **argv)
{
	gboolean godaemon = TRUE;
	char *p;
	int port;
	int i;
	char *mode;
	int option;
	poptContext  popt_context;

#ifdef CHECK_FOR_LEAKS
	leak_checker_init (argv[0]);
//	g_atexit (leak_checker_dump);
#endif

	memset(&stats, 0, sizeof(struct stat_s));

	popt_context = poptGetContext("eazel-proxy", argc, (const char **)argv, popt_options, 0);
	while ((option = poptGetNextOpt(popt_context)) >= 0) {
		switch (option) {
		case 'h':
			poptPrintUsage (popt_context, stderr, 0);
			exit (EX_OK);
			break;
		case 'v':
			display_version ();
			exit (EX_OK);
			break;
		case 'l':
                        g_free (config.logf_name);
			config.logf_name = g_strdup (poptGetOptArg (popt_context));
			break;
		case 'n':
			config.use_oaf = FALSE;
			break;
                case 't':
                        g_free (config.target_path);
                        config.target_path = g_strdup (poptGetOptArg (popt_context));
			if (strlen (config.target_path) == 0) {
				g_free (config.target_path);
				config.target_path = NULL;
			}

			if ( ! util_validate_url (config.target_path)) {
				fprintf (stderr, "-t argument must be url (was '%s')\n", config.target_path);
				exit (-1);
			}
                        break;
		case 'u':
			config.specified_upstream_host = TRUE;
			g_free (config.upstream_host);
			config.upstream_host = g_strdup (poptGetOptArg (popt_context));
			if (strlen (config.upstream_host) == 0) {
				config.upstream_host = NULL;
				break;
			}

			p = strchr (config.upstream_host, ':');
			if (p) {
				config.upstream_port = atoi (p+1);
				*p = 0;
			} else {
				config.upstream_port = PORT_HTTP_DEFAULT;
			}
			break;
		case 'L':
			config.append_log = TRUE;
			break;
#ifndef NO_DEBUG_MIRRORING
		case 'd':
			godaemon = FALSE;
			break;
		case 'p':
			config.local_port = atoi (poptGetOptArg (popt_context));
			break;
		case 'e':
			config.use_stderr = TRUE;
			break;
		case 'm':
			config.mirror_log_dir = g_strdup (poptGetOptArg (popt_context));
			break;
#endif
		}
	}

	if (config.logf_name && (config.logf_name[0] == '~') && (config.logf_name[1] == '/')) {
		char *oldlog = config.logf_name;

		/* replace leading ~/ with the user's homedir */
		config.logf_name = g_strjoin("/", g_get_home_dir(), oldlog + 2, NULL);
		g_free(oldlog);
	}

	if (config.use_stderr) {
		config.logf = stderr;
		godaemon = FALSE;
	} else if (config.logf_name) {
		mode = (config.append_log ? "a" : "w");

		if (!(config.logf = fopen(config.logf_name, mode))) {
			fprintf(stderr, "Unable to open logfile %s for writing!\n",
				config.logf_name);
			exit(EX_CANTCREAT);
		}
	}

#ifdef HAVE_OPENSSL

	/* strdup it before I free the context */
	if (gl_opt_certfile) {
		gl_opt_certfile = g_strdup (gl_opt_certfile);
	}
#endif

	poptFreeContext (popt_context);
	
	if (godaemon) {
		fprintf (stderr, "eazel-proxy launching into the background...\n\n");
		make_daemon();
	}

#ifndef SIMPLE_PROXY
	if (config.use_oaf) {
		CORBA_ORB orb;

		/* Init CORBA and OAF */
		orb = oaf_init (argc, argv);

		if (NULL == orb) {
			log("Unable to init OAF");
			exit (-1);
		}

		util_gconf_init ();
		load_gconf_defaults();

#ifdef HAVE_OPENSSL
		if (! gl_opt_no_ssl && !gl_gconf_ssl_disabled) {
			config.use_ssl = TRUE;
			/* FIXME failure here appears to be fatal; should be more graceful */

			if (gl_opt_certfile) {
				log ("Enabling SSL; using certfile %s", gl_opt_certfile);
				eazel_init_ssl (NULL, gl_opt_certfile);
			} else if (gl_gconf_ssl_cert) {
				log ("Enabling SSL; using certfile %s", gl_opt_certfile);
				eazel_init_ssl (NULL, gl_gconf_ssl_cert);
			} else {
				log ("Enabling SSL; using certfile %s", DATADIR EAZEL_DEFAULT_CERT);
				eazel_init_ssl (NULL, DATADIR EAZEL_DEFAULT_CERT);
			}
		}
#endif
		/* Exits on failure */
		init_impl_eazelproxy (orb);
	}
#endif /*SIMPLE_PROXY*/

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGPIPE\n");
		exit(EX_OSERR);
	}
	if (signal(SIGUSR1, takesig) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGUSR1\n");
		exit(EX_OSERR);
	}
	if (signal(SIGTERM, takesig) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGTERM\n");
		exit(EX_OSERR);
	}
	if (signal(SIGHUP, takesig) == SIG_ERR) {
		fprintf(stderr, "Could not set SIGHUP\n");
		exit(EX_OSERR);
	}

	log(PACKAGE " " VERSION " starting...");

#ifndef SIMPLE_PROXY
	/* If we're using OAF, we'll wait to be directed to listen to a port */
	if (!config.use_oaf)
#endif /* SIMPLE_PROXY */
	{
		static const ProxyCallbackInfo callbacks = {
			simple_proxy_request_cb,
			NULL,
			NULL
		};

		port = proxy_listen (config.local_port, NULL, &callbacks, NULL);
		if ( 0 == port ) {
			exit (EX_UNAVAILABLE);
		}
		log("listening on port %d", port);
#ifndef SIMPLE_PROXY
	} else {
		log ("Waiting for EazelProxy::UserControl::authenticate_user");
#endif /* SIMPLE_PROXY */
	}

	g_main_set_poll_func(socket_glib_poll_func);

	l_main_loop = g_main_new (TRUE);

	while (g_main_is_running(l_main_loop)) {
		g_main_iteration (TRUE);
		socket_event_pump ();
	}

	log("shutting down...");

	/* it's nice to call the event loop a few more times, to try to let any last
	 * buffered output get shoved out...
	 */
	for (i = 0; i < 10; i++) {
		g_main_iteration (FALSE);
	}

#ifndef SIMPLE_PROXY
	if (config.use_oaf) {
		shutdown_impl_eazelproxy();
	}
#endif /*SIMPLE_PROXY*/

	g_main_destroy (l_main_loop);

	log(PACKAGE " " VERSION " terminated.");
	if (config.logf && (config.logf != stderr)) {
		fclose(config.logf);
	}

#ifdef CHECK_FOR_LEAKS
	leak_checker_dump();
#endif

	exit(EX_OK);
}
