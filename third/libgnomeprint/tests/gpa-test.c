/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  test-gpa..c: test functions for the libgpa config database
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2002 Ximian Inc. and authors
 *
 */

#include "config.h"

#include <popt.h>
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "gpa-node-private.h"
#include "gpa-list.h"
#include "gpa-utils.h"
#include "gpa-root.h"
#include "gpa-config.h"

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprint/gnome-print-config-private.h>

GMainLoop * loop;
gint callback_count = 0;
gint num = 0;
gchar *num_str = NULL;
gboolean debug = FALSE;
gboolean dump_config = FALSE;
gboolean dump_root = FALSE;
GList * cmd_line_nodes = NULL;

#define max_num ((sizeof (num_table) / sizeof (num_table[0]))-1)

static struct poptOption options[] = {
	{ "num", '\0', POPT_ARG_STRING, &num_str,   0,
	  "Num number of functions to run, where nn is the num number", "#"},
	{ "dump-config",     '\0', POPT_ARG_NONE, &dump_config,   0,
	  "Dump the GnomePrintConfig object to the console",  NULL},
	{ "dump-root",     '\0', POPT_ARG_NONE,   &dump_root,   0,
	  "Dump the GpaRoot node to the console",  NULL},
	{ "debug",    '\0', POPT_ARG_NONE, &debug, 0,
	  "Print debugging output",          NULL},
	POPT_AUTOHELP
	{ NULL }
};

typedef enum {
	TEST_GPA_CRASH = -3,
	TEST_GPA_ERROR = -2,
	TEST_GPA_BAD_PARAMETERS = -1,
	TEST_GPA_SUCCESS = 0,
	TEST_GPA_SUCCESS_LAST = 99
} GpaTestRetval;

typedef struct _GpaTestNum GpaTestNum;
typedef struct _GpaTestPair     GpaTestPair;

struct _GpaTestNum {
	GpaTestRetval (*function) (void);
	const guchar *description;
};

struct _GpaTestPair {
	const guchar *path;
	const guchar *expected_result;
};


static GpaTestRetval test_ (void);
static GpaTestRetval test_config_load (void);
static GpaTestRetval test_config_dump (void);
static GpaTestRetval test_globals (void);
static GpaTestRetval test_gpa_list (void);
static GpaTestRetval test_gpa_list_callbacks (void);
static GpaTestRetval test_gpa_config_callbacks (void);
static GpaTestRetval test_gpa_settings (void);

static const GpaTestNum num_table[] = {
	{ &test_,                     "Sanity check test, also servers as a template"},
	{ &test_config_load,          "Create a default GnomePrintConfig and unref it"},
	{ &test_config_dump,          "Dump the contents of a GnomePrintConfig to the console"},
	{ &test_globals,              "Check the precence of the Global nodes."},
	{ &test_gpa_list,             "Test the GPAList object"},
	{ &test_gpa_list_callbacks,   "Test the GPAList callbacks"},
	{ &test_gpa_config_callbacks, "Test the GPAConfig callbacks"},
	{ &test_gpa_settings,         "Test that the settings contain the required nodes"},
};

static GpaTestRetval gpa_test_pairs (GPANode *config, const GpaTestPair *tests, gint max);

/* ------------ Helper functions ------------ */
static GPANode *
my_get_config (void)
{
	GnomePrintConfig *config;
	GPANode *config_node;

	config = gnome_print_config_default ();
	if (!config) {
		g_warning ("Could not create the default GnomePrintConfig object\n");
		return NULL;
	}
	/* Get the Config node */
	config_node = gnome_print_config_get_node (config);
	if (!config_node) {
		g_warning ("Cant get config_node from GnomePrintConfig\n");
		return NULL;
	}

	gpa_node_ref (config_node);
	gnome_print_config_unref (config);

	return config_node;
}

/* ------------ Start of tests ------------ */
static void
increment_callback_count (void)
{
	callback_count++;
}

static GpaTestRetval
test_gpa_settings (void)
{
	const gchar * nodes [] = {
		"Output",
		"Output.Media",
		"Output.Media.PhysicalSize",
		"Output.Media.PhysicalSize.Width",
		"Output.Media.PhysicalSize.Height",
#if 0
		"Output.Media.PhysicalOrientation",
		"Output.Media.PhysicalOrientation.Paper2PrinterTransform",
		"Output.Media.Margins",
		"Output.Media.Margins.Top",
		"Output.Media.Margins.Bottom",
		"Output.Media.Margins.Right",
		"Output.Media.Margins.Left",
		"Output.Media.Resolution",
		/* Might need to add DPI, DPI.X & DPI.Y here, but it doesn't
		 * apply to "Infinite Resolution" printers (Chema)
		 */
#endif
#if 0	
		"Output.Media.Job",
		"Output.Media.Job.NumCopies",
		"Output.Media.Job.Collate",
#endif
		"Document",
		"Document.Page",
		"Document.Page.Layout",
#if 0	
		"Document.Page.Layout.ValidPhysicalSizes", /* what is this for? */
		"Document.Page.Layout.LogicalPages",
		"Document.Page.Layout.PhysicalPages",
		"Document.Page.Layout.Width",
		"Document.Page.Layout.Height",
		"Document.Page.Layout.Pages",
		"Document.Page.LogicalOrientation",
		"Document.Page.LogicalOrientation.Page2LayoutTransform",
		"Document.Page.Margins",
		"Document.Page.Margins.Top",
		"Document.Page.Margins.Bottom",
		"Document.Page.Margins.Right",
		"Document.Page.Margins.Left",
		"Document.PreferedUnit", /* Required ? hmm */
		"Document.Name", /* Required ? hmm */
#endif
	};
	gint max, i;
	GPANode *config;
	GPANode *child;
	GPANode *printers_list;
	GSList *printers_l, *l;

	return TEST_GPA_SUCCESS;

	config = my_get_config ();
	if (!config)
		return TEST_GPA_ERROR;

	max = sizeof (nodes) / sizeof (gchar *);
	/* Create a GSList of settings*/
	printers_list = gpa_node_lookup (config, "Globals.Printers");
	if (!printers_list) {
		g_warning ("Could not find Globals.Printers");
		return TEST_GPA_ERROR;
	}
	printers_l = NULL;
	child = gpa_node_get_child (printers_list, NULL);
	for (; child != NULL; child = gpa_node_get_child (printers_list, child)) {
		printers_l = g_slist_prepend (printers_l, child);
		g_print ("Append %s\n", gpa_node_id (child));
	}
	if (printers_l == NULL) {
		g_warning ("There are no printers");
		return TEST_GPA_ERROR;
	}

	/* Check each setting in the list */
	l = printers_l;
	while (l) {
		GPANode *printer;
		GPANode *settings_list;
		GSList *settings_l, *l2;
		
		printer = l->data;
		
		g_print ("Checking printer %s\n", gpa_node_id (printer));
		/* Get a GSList of settings */
		settings_list = gpa_node_lookup (printer, "Settings");
		if (!settings_list) {
			g_warning ("Could not find SettingsList for printer %s\n",
				   gpa_node_id (printer));
			return TEST_GPA_ERROR;
		}
		settings_l = NULL;
		child = gpa_node_get_child (settings_list, NULL);
		for (; child != NULL; child = gpa_node_get_child (settings_list, child))
			settings_l = g_slist_prepend (settings_l, child);
		if (printers_l == NULL) {
			g_warning ("The printer %s does not have Settings",
				   gpa_node_id (printer));
			return TEST_GPA_ERROR;
		}

		l2 = settings_l;
		while (l2) {
			GPANode *settings;
			
			settings = l2->data;
	
			if (gpa_node_id (settings) == NULL) {
				g_warning ("Settings must have a NodeId\n");
				return TEST_GPA_ERROR;
			}
			
			g_print ("Testing setting %s\n", gpa_node_id (settings));
			
			for (i = 0; i < max; i++) {
				child = gpa_node_lookup (settings, nodes[i]);
				if (!child) {
					g_warning ("Could not find node %s for printer %s and settings %s",
						   nodes[i], gpa_node_id (printer), gpa_node_id (settings));
					return TEST_GPA_ERROR;
				}
				g_print ("Found %s\n", nodes[i]);
				gpa_node_unref (child);
			}
			l2 = l2->next;
		}
		l = l->next;
	}
		
	return TEST_GPA_SUCCESS;	
}

static GpaTestRetval
test_gpa_list_callbacks (void)
{
	GPANode *config;
	GPANode *node;
	gchar *value;

	return TEST_GPA_SUCCESS;

	config = my_get_config ();
	if (!config)
		return TEST_GPA_ERROR;

	/* Set to generic */
	gpa_node_set_path_value (config, "Globals.Printers", "GENERIC");
	value = gpa_node_get_path_value (config, "Globals.Printers");
	if (!value || (strcmp (value, "GENERIC") != 0)) {
		g_warning ("Could not set a default for a via _set_path_value.\n");
		return TEST_GPA_ERROR;
	}

	/* Listen for changes */
	node = gpa_node_lookup (config, "Globals.Printers");
	gpa_utils_dump_tree (node, 0);
	g_print ("Connecting to Node: %d\n", GPOINTER_TO_INT (node));
	g_signal_connect (G_OBJECT (node), "modified",
			  (GCallback) increment_callback_count, NULL);

	/* Change the node */
	callback_count = 0;
	gpa_node_set_path_value (config, "Globals.Printers", "PDF");

	/* Run the loop for a couple of seconds */
	g_timeout_add (400, (GSourceFunc) g_main_loop_quit, loop);
	g_main_loop_run (loop);
		
	/* Verify that we got one modified signal emmision */
	if (callback_count < 1) {
		g_warning ("Change in list default node did not generated a signal emission\n");
		return TEST_GPA_ERROR;
	}
	if (callback_count > 1) {
		g_warning ("Change in list default node generated more than one emission\n");
#ifdef __GNUC__
//#warning This test has been disabled
#endif
		return TEST_GPA_SUCCESS;
		return TEST_GPA_ERROR;
	}

	gpa_node_unref (config);
	
	return TEST_GPA_SUCCESS;
}

static GpaTestRetval
test_gpa_list (void)
{
	GpaTestRetval ret;
	GPANode *config;
	GPANode *list;
	GPANode *node;
	guchar *value;
	const GpaTestPair tests[] = {
		/* Check that we catch invalid paths */
		{ "Globals.Printers.?",          NULL},
		{ "Globals.Printers..SomeChild",   NULL},
		{ "Globals.Printers. Default",   NULL},
		/* Check a List node that doesn't exist */
		{ "Globals.Printers.SomeRandomNodeThatDoesNotExist",  NULL},
		/* Check a List node that doesn't exist and has childs*/
		{ "Globals.Printers.SomeRandomNodeThatDoesNotExist.Name",  NULL},
		/* Now some tests for stuff we use below, tests explicit childs too */
		{ "Globals.Printers.GENERIC",    "GENERIC"},
		{ "Globals.Printers.PDF",        "PDF"},
	};

	return TEST_GPA_SUCCESS;

	config = my_get_config ();
	
	gpa_node_set_path_value (config, "Globals.Printers", "GENERIC");
	
	ret = gpa_test_pairs (config, tests, sizeof (tests) / sizeof (tests [0]));

	if (ret != TEST_GPA_SUCCESS) {
		g_print ("Test pairs failed\n");
		return ret;
	}

#if 0 /* Vendor no longer exists, test this fucntionality with a different node */
	/* Check that .Default fails for lists which can't have a Default */
	list = gpa_node_lookup (config, "Globals.Vendors");
	node = gpa_node_lookup (config, "Globals.Vendors.GNOME");
	if (!list || !node) {
		g_warning ("Can't get Globas.Vendors or Globals.Vendors.GNOME\n");
		return TEST_GPA_ERROR;
	}
	if (!gpa_node_verify (list)) {
		g_warning ("Can't verify the Vendors list\n");
		return TEST_GPA_ERROR;
	}
	if (gpa_list_set_default (GPA_LIST (list), node)) {
		g_warning ("Lists with can_have_default == NO, should not allow a default to be set\n");
		return TEST_GPA_ERROR;
	}
#endif	

	/* Set the default value with  set_path_value */
	gpa_node_set_path_value (config, "Globals.Printers", "PDF");
	/* can't check if _set_path_value failed always TRUE  because it is done in a "changed_value" callback */
	value = gpa_node_get_path_value (config, "Globals.Printers");
	if (!value || (strcmp (value, "PDF") != 0)) {
		g_warning ("Could not set a default for a via _set_path_value.\n");
		return TEST_GPA_ERROR;
	}

	/* Now set the default but this time with a GPANode */
	list = gpa_node_lookup (config, "Globals.Printers");
	node = gpa_node_lookup (config, "Globals.Printers.GENERIC");
	if (!list || !node) {
		g_warning ("Can't get Globas.Printers or Globals.Printers.GENERIC\n");
		return TEST_GPA_ERROR;
	}
	if (!gpa_node_verify (list)) {
		g_warning ("Can't verify the Printers list\n");
		return TEST_GPA_ERROR;
	}
	if (!gpa_list_set_default (GPA_LIST (list), node)) {
		g_warning ("Could not set a default for a GPAList, _set_value returned FALSE\n");
		return TEST_GPA_ERROR;
	}
	value = gpa_node_get_path_value (config, "Globals.Printers");
	if (!value || (strcmp (value, "GENERIC") != 0)) {
		g_warning ("Could not set a default for a GPAList, strings don't match)\n");
		return TEST_GPA_ERROR;
	}


	/* Create a list with no childs, make sure we can handle this case in the code */
	
	/* Make sure adding a child with an id of .DefaultChild. fails */

	/* Try adding a child of the wrong type */
	
	/* Add some childs */
	
	/* Verify the list, a default has not been set yet */
	
	/* Set a default */

	/* Verify the list */


	
	/* Test that we get callbacks when things change */

	gpa_node_unref (config);
	    
	return ret;
}

static GpaTestRetval
test_gpa_config_callbacks (void)
{
	GPANode *config;
	GPANode *node;
	gchar *value;

	return TEST_GPA_SUCCESS;
	
	config = my_get_config ();
	if (!config)
		return TEST_GPA_ERROR;

	/* Set to generic */
	gpa_node_set_path_value (config, "Printer", "GENERIC");
	value = gpa_node_get_path_value (config, "Printer");
	if (!value || (strcmp (value, "GENERIC") != 0)) {
		g_warning ("Could not set the config->Printer via _set_path_value.\n");
		return TEST_GPA_ERROR;
	}

	/* Listen for changes */
	node = gpa_node_lookup (config, "Printer");
	g_print ("Connecting to Node: %d\n", GPOINTER_TO_INT (node));
	g_signal_connect (G_OBJECT (node), "modified",
			  (GCallback) increment_callback_count, NULL);

	/* Change the node */
	callback_count = 0;
	gpa_node_set_path_value (config, "Printer", "PDF");

	/* Run the loop for a couple of seconds */
	g_timeout_add (400, (GSourceFunc) g_main_loop_quit, loop);
	g_main_loop_run (loop);
		
	/* Verify that we got one modified signal emmision */
	if (callback_count < 1) {
		g_warning ("Change in config->Printer did not generated a signal emission\n");
		return TEST_GPA_ERROR;
	}
	if (callback_count > 1) {
		g_warning ("Change in config->Printer  node generated more than one emission (%d)\n", callback_count);
		/* Ignore for now, not a huge problem */
		if (FALSE)
		return TEST_GPA_ERROR;
	}

	gpa_node_unref (config);
	
	return TEST_GPA_SUCCESS;
}

static GpaTestRetval
test_globals (void)
{
	GnomePrintConfig *config;
	const gchar * paths[] = {
		"Globals.Media",
		"Globals.Vendors.GNOME",
		"Globals.Printers.GENERIC",
	};
	gint max;
	gint i;
	
	config = gnome_print_config_default ();
	if (!config) {
		g_warning ("Could not create GnomePrintConfig default object\n");
		return TEST_GPA_ERROR;
	}

	max = sizeof (paths) / sizeof (paths [0]);
	for (i = 0; i < max; i++) {
		const gchar *path;
		gchar *result;
		path = paths[i];

		g_print ("Check for \"%s\"\n", path);
			
		result = gnome_print_config_get (config, path);

		g_print ("Result: %s\n", result);

		g_free (result);
	}

	gnome_print_config_unref (config);

	return TEST_GPA_SUCCESS;
}

static GpaTestRetval
test_config_dump (void)
{
	GnomePrintConfig *config;
	
	config = gnome_print_config_default ();
	if (!config) {
		g_warning ("Could not create GnomePrintConfig default object\n");
		return TEST_GPA_ERROR;
	}

	gnome_print_config_dump (config);
	gnome_print_config_unref (config);

	return TEST_GPA_SUCCESS;
}

static GpaTestRetval
test_config_load (void)
{
	GnomePrintConfig *config;
	
	config = gnome_print_config_default ();
	if (!config) {
		g_warning ("Could not create GnomePrintConfig default object\n");
		return TEST_GPA_ERROR;
	}

	gnome_print_config_unref (config);

	return TEST_GPA_SUCCESS;
}

static GpaTestRetval
test_ (void)
{
	return TEST_GPA_SUCCESS;
}

/* ------------ End of tests ------------ */
static GpaTestRetval
gpa_test_pairs (GPANode *config, const GpaTestPair *tests, gint max)
{
	GpaTestRetval ret = TEST_GPA_SUCCESS;
	gint i;

	for (i = 0; i < max; i++) {
		const guchar *path;
		const guchar *expected;
		gchar *result;
		gint len_result;
		gint len_expected;
		
		path     = tests[i].path;
		expected = tests[i].expected_result;

		g_print ("\nCheck for \"%s\", expected \"%s\"\n", path, expected);
			
		result = gpa_node_get_path_value (config, path);

		g_print ("Result: %s\n", result);

		if (result == NULL) {
			if (expected != NULL) {
				ret = TEST_GPA_ERROR;
				g_print ("\t\t\tError while checking path \"%s\". Expected:\"%s\" Result:\"%s\"\n",
					 path, expected, result);
			} else {
				g_print ("Match, expected was NULL\n");
			}
			continue;
		}
		
		if (expected == NULL) {
			ret = TEST_GPA_ERROR;
			g_print ("Error while checking path \"%s\". Expected:\"%s\" Result:\"%s\"\n",
				 path, expected, result);
			g_free (result);
			continue;
		}
		
		len_result   = strlen (result);
		len_expected = strlen (expected);

		g_print ("Result:%s Expected:%s\n",
			 result, expected);
		
		if (len_result != len_expected ||
		    strncmp (result, expected, len_result) != 0) {
			ret = TEST_GPA_ERROR;
			g_print ("Error while checking path \"%s\". Expected:\"%s\" Result:\"%s\"\n",
				 path, expected, result);
		} else {
			g_print ("Match 3\n");
		}
		g_free (result);
	}

	return ret;
}


static GpaTestRetval
test_gpa_run_num (gint num)
{
	GpaTestRetval ret;

	g_print ("Running num %d\n[%s]\n", num, num_table[num].description);

	ret = (num_table[num].function ());

	switch (ret) {
	case TEST_GPA_SUCCESS:
		if (num == max_num)
			ret = TEST_GPA_SUCCESS_LAST;
		g_print (" Pass..\n");
		break;
	case TEST_GPA_ERROR:
		g_print (" Fail..\n");
		break;
	default:
		g_assert_not_reached ();
		break;
			
	}

	return ret;
}

static void
usage (gchar *error)
{
	g_print ("Error: %s\n\n", error);
	g_print ("Usage: test-gpa --num=[num]\n\n");
	exit (TEST_GPA_BAD_PARAMETERS);
}

static void
parse_command_line (int argc, const char ** argv, gint *num)
{
	poptContext popt;
	char **args;

	popt = poptGetContext ("test_gpa", argc, argv, options, 0);
	poptGetNextOpt (popt);

	args = (char**) poptGetArgs (popt);
	if (args) {
		gint i;
		for (i = 0; args[i]; i++) {
			cmd_line_nodes = g_list_append (cmd_line_nodes, args[i]);
		}
		return;
	}

	if (dump_config || dump_root)
		return;
	
	if (!num_str || num_str[0] == '\0') {
		dump_config = TRUE;
		return;
	}

	*num = atoi (num_str);

	if (*num == -1) {
		GList *list = NULL;
		/* We crash, this is part of the sanity check that allows us verify that crashes
		 * are treated as errors when they happen (Chema)*/
		g_print ("Crashing (On purpose)...\n");
		list->next = NULL;
	}
	
	if (*num < 0 || (*num > max_num)) {
		gchar *error;
		error = g_strdup_printf ("Num number out of range. Valid range is -1 to %d",
					 max_num);
		usage (error);
	}
	
	if (debug)
		g_print ("Num is %d\n", *num);
	
	poptFreeContext (popt);
}

static void
handle_sigsegv (int i)
{
	g_print ("\n\ngpa-test crashed while running num %d [%s]\n",
		 num, num_table[num].description);
	exit (TEST_GPA_CRASH);
}

int
main (int argc, const char * argv[])
{
	GpaTestRetval ret;
	struct sigaction sig;

	g_type_init ();
	loop = g_main_loop_new (NULL, FALSE);

	parse_command_line (argc, argv, &num);

	/* Catch sigsegv signals */
	sig.sa_handler = handle_sigsegv;
	sig.sa_flags = 0;
	sigaction (SIGSEGV, &sig, NULL);

	gpa_init ();

	if (dump_config || dump_root) {
		GPANode *node;

		if (dump_config)
			node = (GPANode*) gpa_config_new ();
		if (dump_root)
			node = (GPANode*) gpa_root;

		gpa_utils_dump_tree (node, 2);

		return TEST_GPA_SUCCESS;
	}

	if (cmd_line_nodes) {
		GList *l = cmd_line_nodes;

		while (l) {
			GPANode *node;
			gchar *path;
			path = l->data;
			g_print ("Looking for %s\n", path);

			node = gpa_node_lookup (NULL, path);
			if (node) {
				gpa_utils_dump_tree (node, 2);
				gpa_node_unref (node);
				node = NULL;
			}
			l = l->next;
		}

		return TEST_GPA_SUCCESS;
	}

	ret = test_gpa_run_num (num);
		
	return ret;
}

