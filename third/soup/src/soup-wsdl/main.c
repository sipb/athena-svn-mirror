/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * main.c: Main program driver
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <popt.h>
#include <string.h>
#include <stdlib.h>

#include <libwsdl/wsdl-typecodes.h>

#include "wsdl.h"
#include "wsdl-parse.h"
#include "wsdl-memory.h"
#include "wsdl-soap-stubs.h"
#include "wsdl-soap-skels.h"
#include "wsdl-soap-common.h"
#include "wsdl-soap-headers.h"
#include "wsdl-trace.h"
#include "wsdl-thread.h"
#include "wsdl-describe.h"

gboolean option_warnings_are_errors = FALSE;
gboolean option_show_doc = FALSE;
gboolean option_show_xml_tree = FALSE;
gboolean option_emit_stubs = FALSE;
gboolean option_emit_skels = FALSE;
gboolean option_emit_common = FALSE;
gboolean option_emit_headers = FALSE;
const guchar *option_outdir = "./";
const guchar *option_debug_level = "";
const guchar *option_debug_modules = "";

struct poptOption options[] = {
	{"debug-level", '\0', POPT_ARG_STRING, &option_debug_level, 0,
	 "Set logging verbosity", NULL},
	{"debug-modules", '\0', POPT_ARG_STRING, &option_debug_modules, 0,
	 "The set of code modules to log", NULL},
	{"show-doc", '\0', POPT_ARG_NONE, &option_show_doc, 0,
	 "Show the document structure after parsing", NULL},
	{"show-xml-tree", '\0', POPT_ARG_NONE, &option_show_xml_tree, 0,
	 "Show the XML document structure built by libxml", NULL},
	{"werror", '\0', POPT_ARG_NONE, &option_warnings_are_errors, 0,
	 "Abort compilation due to warnings", NULL},
	{"outdir", '\0', POPT_ARG_STRING, &option_outdir, 0,
	 "The directory to use for output files", NULL},
	{"stubs", '\0', POPT_ARG_NONE, &option_emit_stubs, 0,
	 "Write client stubs", NULL},
	{"skels", '\0', POPT_ARG_NONE, &option_emit_skels, 0,
	 "Write server skeleton routines", NULL},
	{"common", '\0', POPT_ARG_NONE, &option_emit_common, 0,
	 "Write code common to client and server", NULL},
	{"headers", '\0', POPT_ARG_NONE, &option_emit_headers, 0,
	 "Write headers", NULL},
	POPT_AUTOHELP {NULL, '\0', 0, NULL, 0, NULL, NULL},
};

int
main (int argc, const char **argv)
{
	poptContext popt_con;
	const char **wsdl_files;
	int rc;
	guchar *outdir;

	popt_con = poptGetContext ("soup-wsdl", argc, argv, options, 0);
	if ((rc = poptGetNextOpt (popt_con)) < -1) {
		g_print ("%s: bad argument %s: %s\n", 
			 argv[0],
			 poptBadOption (popt_con, POPT_BADOPTION_NOALIAS),
			 poptStrerror (rc));
		exit (-1);
	}

	wsdl_files = poptGetArgs (popt_con);
	if (wsdl_files == NULL) {
		g_print ("Nothing to do!\n");
		exit (0);
	}

	/* Make sure the outdir has a trailing '/' */
	if (option_outdir[strlen (option_outdir) - 1] != '/') {
		outdir = g_strconcat (option_outdir, "/", NULL);
	} else {
		outdir = g_strdup (option_outdir);
	}

	/* Set up logging */
	wsdl_parse_debug_domain_string (option_debug_modules);
	wsdl_parse_debug_level_string (option_debug_level);

	while (*wsdl_files) {
		wsdl_definitions *definitions;
		guchar *fileroot, *c;
		int ret;

		definitions = wsdl_parse (*wsdl_files);

		if (option_show_doc) {
			g_print ("Document hierarchy\n");
			g_print ("==================\n\n");

			if (definitions == NULL) {
				g_print ("*** Empty\n");
			} else {
				wsdl_describe_definitions (0, definitions);
			}
		}

		if (option_show_xml_tree && definitions->xml) {
			xmlDocDump (stdout, definitions->xml);
		}

		if ((ret = wsdl_thread (definitions)) > 0) {
			wsdl_debug (WSDL_LOG_DOMAIN_MAIN, 
				    G_LOG_LEVEL_INFO,
				    "Skipping to next WSDL file.", 
				    ret);
		} else {
			wsdl_debug (WSDL_LOG_DOMAIN_MAIN, 
				    G_LOG_LEVEL_INFO,
				    "Writing output files.");

			fileroot = g_strdup (g_basename (*wsdl_files));
			c = strrchr (fileroot, '.');
			if (c != NULL) {
				*c = '\0';
			}

			if (option_emit_stubs) {
				wsdl_emit_soap_stubs (outdir, 
						      fileroot,
						      definitions);
			}

			if (option_emit_skels) {
				wsdl_emit_soap_skels (outdir, 
						      fileroot,
						      definitions);
			}

			if (option_emit_common) {
				wsdl_emit_soap_common (outdir, 
						       fileroot,
						       definitions);
			}

			if (option_emit_headers) {
				wsdl_emit_soap_headers (outdir, 
							fileroot,
							definitions);
			}

			g_free (fileroot);
		}

		wsdl_free_definitions (definitions);

		wsdl_files++;
	}

	g_free (outdir);

	poptFreeContext (popt_con);

	return (0);
}
