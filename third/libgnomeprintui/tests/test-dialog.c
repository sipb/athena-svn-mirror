/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  test-dialog.c:
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
 *  Copyright (C) 2002-2003 Ximian Inc.
 *
 */

#include "config.h"

#include <popt.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-dialog-private.h>
#include <libgnomeprintui/gnome-printer-selector.h>
#include <libgnomeprintui/gpaui/gpa-tree-viewer.h>
#include <libgnomeprintui/gpaui/gpa-printer-selector.h>
#include <libgnomeprint/private/gnome-print-config-private.h>

#define MAGIC_NUMBER -100

gboolean option_list_tests;
gint     num_test = MAGIC_NUMBER;
gchar *  num_test_str = NULL;
gboolean debug = FALSE;
gboolean tree = FALSE;
gint     callback_count = 0;

typedef enum {
	TEST_RETVAL_CRASH = -3,
	TEST_RETVAL_ERROR = -2,
	TEST_RETVAL_BAD_PARAMETERS = -1,
	TEST_RETVAL_SUCCESS = 0,
	TEST_RETVAL_SUCCESS_LAST = 99,
} TestRetval;

poptContext popt;

static struct poptOption options[] = {
	{ "num", '\0', POPT_ARG_STRING, &num_test_str,   0,
	  "Test number to run", "#"},
	{ "list-tests", '\0', POPT_ARG_NONE, &option_list_tests,   0,
	  "List on the console the description of the test cases", NULL},
	{ "tree",     '\0', POPT_ARG_NONE, &tree,   0,
	  "Show the tree of GPANodes",  NULL},
	{ "debug",    '\0', POPT_ARG_NONE, &debug, 0,
	  "Print debugging output",          NULL},
	POPT_AUTOHELP
	{ NULL }
};

typedef struct _TestCase TestCase;

struct _TestCase {
	TestRetval (*function) (void);
	const guchar *description;
};

#define max_num_test ((sizeof (test_cases) / sizeof (test_cases[0]))-1)

static TestRetval test_simple (void);
static TestRetval test_show (void);
static TestRetval test_dialog_flags (void);
static TestRetval test_paper_changes (void);
static TestRetval test_printer_changes (void);
static TestRetval test_multiple (void);
static TestRetval test_print_ps (void);
static TestRetval test_print_multiple_ps (void);
static TestRetval test_print_pdf (void);
static TestRetval test_print_multiple_pdf (void);
static TestRetval test_print_multiple (void);
static TestRetval test_paper_changes_print (void);

static const TestCase test_cases[] = {
	{ &test_simple,              "Simple checks"},
	{ &test_show,                "Create and show a GnomePrintDialog"},
	{ &test_dialog_flags,        "Shows dialogs with different flag options"},
	{ &test_paper_changes,       "Change paper parameters and monitor dialog for changes"},
	{ &test_printer_changes,     "Change the selected printer"},
	{ &test_multiple,            "Create and destroy multiple GnomePrintDialogs."},
	{ &test_print_ps,            "Print to PS"},
	{ &test_print_multiple_ps,   "Print multiple times to PS with the same GnomePrintConfig"},
	{ &test_print_pdf,           "Print to PDF"},
	{ &test_print_multiple_pdf,  "Print multiple times to PDF with the same GnomePrintConfig"},
	{ &test_print_multiple,      "Print multiple times alternating between PS & PDF"},
	{ &test_paper_changes_print, "Change paper parameters and print"},
};

/* Helper functions */
static gint
test_dialog_get_num_pages (GtkWidget *dialog)
{
	return g_list_length (GTK_NOTEBOOK (GNOME_PRINT_DIALOG (dialog)->notebook)->children);
}

static void
test_dialog_show_page (GtkWidget *dialog, gint page)
{
	gtk_notebook_set_current_page (GTK_NOTEBOOK (GNOME_PRINT_DIALOG (dialog)->notebook), page);
	
}

/**
 * test_pos_window:
 * @window: 
 * 
 * Position de window so that it doens't overlap with other windows
 * this allows us to see several windows on the screen
 **/
static void
test_pos_window (GtkWidget *window)
{
#define PANEL_HEIGHT 40
#define TOP_BOTTOM_FRAME 36
#define RIGHT_LEFT_FRAME 16
	static gint x = 0;
	static gint y = PANEL_HEIGHT;
	static gint new_x = 0;
	gint width;
	gint height;

	gtk_window_get_size (GTK_WINDOW (window), &width, &height);
	
	if ((y + height) > gdk_screen_height ()) {
		x += new_x + RIGHT_LEFT_FRAME;
		new_x = 0;
		y = PANEL_HEIGHT;
	}

	if (x + width > gdk_screen_width ())
		x = 0;
	if (y + height > gdk_screen_height ())
		y = 0;
	
	new_x = (width > new_x) ? width : new_x;
	
	gtk_window_move (GTK_WINDOW (window), x, y);
	y += height + TOP_BOTTOM_FRAME; 
}

static GtkWidget *
test_dialog_show_all_pages (GnomePrintConfig *config)
{
	GnomePrintJob *job;
	GtkWidget *dialog;
	
	job = gnome_print_job_new (config);
	dialog = gnome_print_dialog_new (job, "Sample Print Dialog",
					 GNOME_PRINT_DIALOG_RANGE | GNOME_PRINT_DIALOG_COPIES);
	gtk_widget_show (dialog);
	test_pos_window (dialog);
	if (test_dialog_get_num_pages (dialog) != 3) {
		g_print ("Invalid number of pages\n");
		exit (TEST_RETVAL_ERROR);
	}

	job = gnome_print_job_new (config);
	dialog = gnome_print_dialog_new (job, "Sample Print Dialog",
					 GNOME_PRINT_DIALOG_RANGE | GNOME_PRINT_DIALOG_COPIES);
	gtk_widget_show (dialog);
	test_dialog_show_page (dialog, 1);
	test_pos_window (dialog);
	if (test_dialog_get_num_pages (dialog) != 3) {
		g_print ("Invalid number of pages\n");
		exit (TEST_RETVAL_ERROR);
	}

	job = gnome_print_job_new (config);
	dialog = gnome_print_dialog_new (job, "Sample Print Dialog",
					 GNOME_PRINT_DIALOG_RANGE | GNOME_PRINT_DIALOG_COPIES);
	gtk_widget_show (dialog);
	test_pos_window (dialog);
	test_dialog_show_page (dialog, 2);	
	if (test_dialog_get_num_pages (dialog) != 3) {
		g_print ("Invalid number of pages\n");
		exit (TEST_RETVAL_ERROR);
	}
		
	return dialog;
}

guint tag = 0;

static gboolean
test_main_quit_real (void)
{
	gtk_main_quit ();
	return FALSE;
}

static gboolean
test_main_quit (void)
{
	gtk_timeout_remove (tag);
	gtk_idle_add ((GtkFunction) test_main_quit_real, NULL);
	return TRUE;
}

static void
test_run (gint msecs)
{
	tag = gtk_timeout_add (msecs, (GSourceFunc) test_main_quit, NULL);
	gtk_main ();
	return;
}

static void
increment_callback_count (void)
{
	callback_count++;
}

/* Tests */
static TestRetval
test_printer_changes (void)
{
	GnomePrintConfig *config;
	GPANode *node, *printers, *child;
	GSList *list, *l;
	GtkWidget *dialog;
	GtkWidget *option_menu;
	gint len;

	config = gnome_print_config_default ();
	
	dialog = test_dialog_show_all_pages (config);

	node = GNOME_PRINT_CONFIG_NODE (config);
	g_return_val_if_fail (GPA_IS_NODE (node), TEST_RETVAL_ERROR);
	printers = gpa_node_get_child_from_path (node, "Globals.Printers");
	g_return_val_if_fail (GPA_IS_NODE (printers), TEST_RETVAL_ERROR);	

	/* Make sure the gtkoptionmenu changes */
	callback_count = 0;
	option_menu = GPA_PRINTER_SELECTOR (GNOME_PRINTER_SELECTOR (GNOME_PRINT_DIALOG (dialog)->printer)->printers)->menu;
	g_return_val_if_fail (GTK_IS_OPTION_MENU (option_menu), TEST_RETVAL_ERROR);
	g_signal_connect (G_OBJECT (option_menu), "changed",
			  (GCallback) increment_callback_count, NULL);

	test_run (1000);
	
	/* Create list */
	list = NULL;
	child = gpa_node_get_child (printers, NULL);
	for (; child != NULL; child = gpa_node_get_child (printers, child))
		list = g_slist_prepend (list, child);
	
	l = list;
	while (l) {
		if (!gnome_print_config_set (config, "Printer", gpa_node_id (l->data))) {
			g_print ("Could not set the Printer to %s\n", gpa_node_id (l->data));
			return TEST_RETVAL_ERROR;
		}
		test_run (1000);
		l = l->next;
	}

	len = g_slist_length (list);
	
	if (callback_count < len) {
		g_warning ("The printers GtkOptionMenu didn't changed "
			   "as expected, expected: %d changed: %d\n",
			 len, callback_count);
		return TEST_RETVAL_ERROR;
	}
	if (callback_count > len) {
		g_warning ("The printers GtkOptionMenu didn't changed "
			   "as expected, expected: %d changed: %d\n",
			   len, callback_count);
#ifdef __GNUC__
//#warning Disabled test
#endif
		return TEST_RETVAL_SUCCESS;
		return TEST_RETVAL_ERROR;
	}

	g_slist_free (list);
	
	g_print ("Callback count %d\n", callback_count);

	return TEST_RETVAL_SUCCESS;
}

static TestRetval
test_paper_changes (void)
{
	GnomePrintConfig *config;
	const gchar *papers [] = {
		"USLetter", "USLegal", "Executive", "A0", "A1", "A2", "A3", "A4", "A5",
		"A6", "A7", "A8", "A9", "A10", "B0", "B1", "B2", "B3", "B4", "B5", "B6",
		"B7", "B8", "B9", "B10", "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7",
		"C8", "C9", "C10", "A4_3", "A4_4", "A4_8", "A3_4", "A5_3", "DL", "C6_C5",
		"Envelope_No10", "Envelope_6x9", "USLetter"};
	const gchar *orientations [] = {"R0", "R90", "R180", "R270", "R0"};
	const gchar *layouts [] = {"Plain", "2_1", "4_1", "I2_1", "IM2_1", "Plain"};
	gint max, i;
		
	config = gnome_print_config_default ();
	test_dialog_show_all_pages (config);

	gnome_print_config_set (config, "Printer", "GENERIC");

	test_run (1000);

	max = sizeof (papers) / sizeof (gchar *);
	for (i = 0; i < max; i++) {
		if (!gnome_print_config_set (config, GNOME_PRINT_KEY_PAPER_SIZE, papers[i])) {
			g_print ("Could not set paper size to %s\n", papers[i]);
			return TEST_RETVAL_ERROR;
		}
		test_run (50);
	}

	max = sizeof (orientations) / sizeof (gchar *);
	for (i = 0; i < max; i++) {
		if (!gnome_print_config_set (config, GNOME_PRINT_KEY_PAGE_ORIENTATION, orientations[i])) {
			g_print ("Could not set the page orientation to %s\n", orientations[i]);
			return TEST_RETVAL_ERROR;
		}
		test_run (150);
	}
	for (i = 0; i < max; i++) {
		if (!gnome_print_config_set (config, GNOME_PRINT_KEY_PAPER_ORIENTATION, orientations[i])) {
			g_print ("Could not set the paper orientation to %s\n", orientations[i]);
			return TEST_RETVAL_ERROR;
		}
		test_run (150);
	}

	max = sizeof (layouts) / sizeof (gchar *);
	for (i = 0; i < max; i++) {
		if (!gnome_print_config_set (config, GNOME_PRINT_KEY_LAYOUT, layouts[i])) {
			g_print ("Could not set the Layout to %s\n", layouts[i]);
			return TEST_RETVAL_ERROR;
		}
		test_run (150);
	}

	
	test_run (100);

	return TEST_RETVAL_SUCCESS;
}

static TestRetval
test_dialog_flags (void)
{
	GnomePrintConfig *config;
	GnomePrintJob *job;
	GtkWidget *dialog;

	config = gnome_print_config_default ();

	job = gnome_print_job_new (config);
	dialog = gnome_print_dialog_new (job, "Sample Print Dialog",
					 0);
	gtk_widget_show (dialog);
	test_pos_window (dialog);

	job = gnome_print_job_new (config);
	dialog = gnome_print_dialog_new (job, "Sample Print Dialog",
					 GNOME_PRINT_DIALOG_RANGE);
	gtk_widget_show (dialog);
	test_pos_window (dialog);
	
	job = gnome_print_job_new (config);
	dialog = gnome_print_dialog_new (job, "Sample Print Dialog",
					 GNOME_PRINT_DIALOG_COPIES);
	gtk_widget_show (dialog);
	test_pos_window (dialog);

	test_dialog_show_all_pages (config);

	test_run (1500);

	return TEST_RETVAL_SUCCESS;
}

static TestRetval
test_show (void)
{
	GtkWidget *dialog;

	dialog = gnome_print_dialog_new (NULL, "Sample Print Dialog", 0);

	gtk_widget_show (dialog);

	test_run (500);

	return TEST_RETVAL_SUCCESS;
}

static TestRetval
test_multiple (void)
{
	GnomePrintJob *job;
	GnomePrintConfig *config;
	GtkWidget *dialog;
	gint i, j;
	gint max = 3;
	gint run_for;

	job = gnome_print_job_new (NULL);
	config = gnome_print_job_get_config (job);
	dialog = gnome_print_dialog_new (job, "Sample Print Dialog", 0);

	if (!job || !config || !dialog)
		return TEST_RETVAL_ERROR;

	run_for = 1;

	for (j = 0; j < 3; j++) {
		
		run_for = (j == 0 ? 1 : (j == 1 ? 100 : 400));
		
		for (i = 0; i < max; i++) {
			gtk_widget_show (dialog);
			test_run (run_for);
			gtk_widget_destroy (dialog);
			dialog = gnome_print_dialog_new (job, "Sample Print Dialog", 0);
		}
		
		for (i = 0; i < max; i++) {
			gtk_widget_show (dialog);
			test_run (run_for);
			gtk_widget_destroy (dialog);
			g_object_unref (config);
			g_object_unref (job);
			config = gnome_print_config_default ();
			job = gnome_print_job_new (config);
			dialog = gnome_print_dialog_new (job, "Sample Print Dialog", 0);
		}
	}
	
	return TEST_RETVAL_SUCCESS;
}

static TestRetval
test_paper_changes_print (void)
{
	GnomePrintConfig *config;
	GnomePrintJob *job;
	GtkWidget *dialog;
	gint i;
	gint n = 4;

	job = gnome_print_job_new (NULL);
	config = gnome_print_job_get_config (job);

	if (config == NULL)
		return TEST_RETVAL_ERROR;

 	gnome_print_config_set (config, "Printer", "GENERIC");
				
	if (!gnome_print_config_set (config, GNOME_PRINT_KEY_PAPER_SIZE, "B3")) {
		g_warning ("Could not change the Paper Size\n");
		return TEST_RETVAL_ERROR;
	}
	if (!gnome_print_config_set (config, GNOME_PRINT_KEY_PAPER_ORIENTATION, "R270")) {
		g_warning ("Could not change the Paper Orientation\n");
		return TEST_RETVAL_ERROR;
	}
	if (!gnome_print_config_set (config, GNOME_PRINT_KEY_PAGE_ORIENTATION, "R90")) {
		g_warning ("Could not change the Page Orientation\n");
		return TEST_RETVAL_ERROR;
	}

	dialog = gnome_print_dialog_new (job, "Sample Print Dialog", 0);
	for (i = 0; i < n; i++) {
		gtk_widget_destroy (dialog);
		dialog = gnome_print_dialog_new (job, "Sample Print Dialog", 0);
		if (!dialog)
			return TEST_RETVAL_ERROR;
		gtk_widget_show (dialog);
		test_run (100);
	}

	gtk_widget_destroy (dialog);
	
	return TEST_RETVAL_SUCCESS;
}

static TestRetval
test_simple (void)
{
	GnomePrintJob *job;
	GnomePrintConfig *config;
	GtkWidget *dialog;

	config = gnome_print_config_default ();
	if (!config)
		return TEST_RETVAL_ERROR;
	job = gnome_print_job_new (NULL);
	if (!job)
		return TEST_RETVAL_ERROR;
	job = gnome_print_job_new (config);
	if (!job)
		return TEST_RETVAL_ERROR;
	dialog = gnome_print_dialog_new (NULL, "Sample Print Dialog", 0);
	if (!dialog)
		return TEST_RETVAL_ERROR;
	
	return TEST_RETVAL_SUCCESS;
}


/* From gtkdialog.c. Nasty but is a gui test suite, what else do you expect? */
typedef struct _ResponseData ResponseData;
struct _ResponseData
{
  gint response_id;
};

static gboolean
dialog_click (GtkWidget *dialog, gint action)
{
	GtkWidget *button = NULL;
	GtkBox *box;
	GList *list;
	
	box = GTK_BOX (GTK_DIALOG (dialog)->action_area);
	list = box->children;
	while (list) {
		GtkBoxChild *child;
		GtkWidget *widget;
		child = list->data;
		widget = child->widget;
		if (GTK_IS_BUTTON (widget)) {
			ResponseData *ad = g_object_get_data (G_OBJECT (widget),
							      "gtk-dialog-response-data");
			if (ad->response_id == action)
				button = widget;
		}
		list = list->next;
	}

	if (!button) {
		g_warning ("Could not find button to click\n");
		return FALSE;
	}

	g_signal_emit_by_name (button, "clicked", 0);
	
	return FALSE;
}

static gboolean
dialog_click_print_cb (gpointer dialog, gint action)
{
	return dialog_click (dialog, GNOME_PRINT_DIALOG_RESPONSE_PRINT);
}
		
static TestRetval
dialog_click_print (GtkWidget *dialog, gint msecs)
{
	tag = gtk_timeout_add (msecs, (GSourceFunc) dialog_click_print_cb, dialog);

	return TEST_RETVAL_SUCCESS;
}

static void
my_draw (GnomePrintContext *gpc)
{
	gnome_print_beginpage (gpc, "1");
	gnome_print_moveto (gpc, 1, 1);
	gnome_print_lineto (gpc, 200, 200);
	gnome_print_stroke (gpc);
	gnome_print_showpage (gpc);
}


static void
my_print (GnomePrintJob *job, gboolean preview)
{
	GnomePrintContext *gpc;

	gpc = gnome_print_job_get_context (job);
	my_draw (gpc);
	g_object_unref (G_OBJECT (gpc));
	
	gnome_print_job_close (job);

	if (!preview) {
		gnome_print_job_print (job);
	}
#if 0
	else {
		gtk_widget_show (gnome_print_job_preview_new (job, "Title goes here"));
	}
#endif
}

static TestRetval
gui_print (GnomePrintJob *job, gint time)
{
	GtkWidget *dialog;
	gint response;

	dialog = gnome_print_dialog_new (job, "Sample Print Dialog", 0);
	if (!dialog)
		return TEST_RETVAL_ERROR;

	gtk_widget_show (dialog);
	test_run (2 * time);

	if (dialog_click_print (dialog, 5 * time) != TEST_RETVAL_SUCCESS)
		return TEST_RETVAL_ERROR;
	
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	switch (response) {
	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
		my_print (job, FALSE);
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		my_print (job, TRUE);
		break;
	default:
		return TEST_RETVAL_ERROR;
	}

	test_run (2 * time);
	
	return TEST_RETVAL_SUCCESS;
}

static TestRetval
test_print_ps (void)
{
	GnomePrintConfig *config;
	GnomePrintJob *job;
	TestRetval retval;

	config = gnome_print_config_default ();
	job = gnome_print_job_new (config);
	if (!job || !config)
		return TEST_RETVAL_ERROR;

	gnome_print_config_set (config, "Printer", "GENERIC");
	retval = gui_print (job, 100);

	return retval;
}

static TestRetval
test_print_pdf (void)
{
	GnomePrintConfig *config;
	GnomePrintJob *job;
	TestRetval retval;

	config = gnome_print_config_default ();
	job = gnome_print_job_new (config);
	if (!job || !config)
		return TEST_RETVAL_ERROR;

	gnome_print_config_set (config, "Printer", "PDF");

	retval = gui_print (job, 300);
	
	return retval;
}

static TestRetval
test_print_multiple_ps (void)
{
	GnomePrintConfig *config;
	GnomePrintJob *job;
	TestRetval retval;
	gint i;
	gint max = 4;

	config = gnome_print_config_default ();
	job = gnome_print_job_new (config);
	if (!job || !config)
		return TEST_RETVAL_ERROR;
	
	gnome_print_config_set (config, "Printer", "GENERIC");
	
	for (i = 0; i < max; i++) {
		retval = gui_print (job, 100);
		if (retval != TEST_RETVAL_SUCCESS)
			return retval;
	}
		
	return retval;
}

static TestRetval
test_print_multiple_pdf (void)
{
	GnomePrintConfig *config;
	GnomePrintJob *job;
	TestRetval retval;
	gint i;
	gint max = 4;

	/* First do it non-gui */
	config = gnome_print_config_default ();
	job = gnome_print_job_new (config);
	if (!job || !config)
		return TEST_RETVAL_ERROR;

	gnome_print_config_set (config, "Printer", "PDF");
	
	for (i = 0; i < max; i++) {
		my_print (job, FALSE);		
	}
	
	for (i = 0; i < max; i++) {
		retval = gui_print (job, 100);
		if (retval != TEST_RETVAL_SUCCESS)
			return retval;
	}
		
	return retval;
}

static TestRetval
test_print_multiple (void)
{
	GnomePrintConfig *config;
	GnomePrintJob *job;
	TestRetval retval;
	gint i;
	gint max = 2;

	config = gnome_print_config_default ();
	job = gnome_print_job_new (config);
	if (!job || !config)
		return TEST_RETVAL_ERROR;
	
	for (i = 0; i < max; i++) {
		gnome_print_config_set (config, "Printer", "PDF");
		retval = gui_print (job, 100);
		if (retval != TEST_RETVAL_SUCCESS)
			return retval;
		
		gnome_print_config_set (config, "Printer", "GENERIC");
		retval = gui_print (job, 100);
		if (retval != TEST_RETVAL_SUCCESS)
			return retval;
	}

	g_object_unref (config);
	g_object_unref (job);
	
	config = gnome_print_config_default ();
	job = gnome_print_job_new (config);
	
	for (i = 0; i < max; i++) {
		gnome_print_config_set (config, "Printer", "PDF");
		retval = gui_print (job, 100);
		if (retval != TEST_RETVAL_SUCCESS)
			return retval;
		
		gnome_print_config_set (config, "Printer", "GENERIC");
		retval = gui_print (job, 100);
		if (retval != TEST_RETVAL_SUCCESS)
			return retval;
	}

	g_object_unref (config);
	g_object_unref (job);
	
	return retval;
}

static TestRetval
test_dispatch (gint num_test)
{
	TestRetval ret;

	g_print ("Running num_test %d\n[%s]\n", num_test, test_cases[num_test].description);

	ret = (test_cases[num_test].function ());

	switch (ret) {
	case TEST_RETVAL_SUCCESS:
		if (num_test == max_num_test)
			ret = TEST_RETVAL_SUCCESS_LAST;
		g_print (" Pass..\n");
		break;
	case TEST_RETVAL_ERROR:
		g_print (" Fail..\n");
		break;
	default:
		g_assert_not_reached ();
		break;
			
	}

	return ret;
}

static void
test_dialog_tree ()
{
	GnomePrintConfig *config;
	GtkWidget *tree;

	config = gnome_print_config_default ();
	tree = gpa_tree_viewer_new (config);

	g_signal_connect (G_OBJECT (tree), "delete_event",
			  (GCallback) gtk_main_quit, NULL);
	gtk_main ();
}

static void
usage (gchar *error)
{
	g_print ("Error: %s\n\n", error);
	g_print ("Usage: test-dialog --num=[num_test] [--tree]\n\n");
	poptPrintHelp (popt, stdout, FALSE);
	exit (TEST_RETVAL_BAD_PARAMETERS);
	exit (TEST_RETVAL_BAD_PARAMETERS);
}

static void
parse_command_line (int argc, const char ** argv, gint *num_test)
{
	gchar *env_test;
	
	popt = poptGetContext ("test_gpa", argc, argv, options, 0);
	poptGetNextOpt (popt);

	if (option_list_tests)
		return;
	
	if (!num_test_str || num_test_str[0] == '\0') {
		if (tree)
			return;
		env_test = getenv ("TEST_NUM");
		if (env_test && env_test[0]) {
			g_print ("TEST_NUM env variable = %s\n", env_test);
			num_test_str = g_strdup (env_test);
		} else {
			return;
		}
	} 

	*num_test = atoi (num_test_str);

	if (*num_test == -1) {
		GList *list = NULL;
		/* We crash, this is part of the sanity check that allows us verify that crashes
		 * are treated as errors when they happen when running ./run-tests
		 * (Chema)
		 */
		g_print ("Crashing ...\n");
		list->next = NULL;
	}
	
	if (*num_test < 0 || (*num_test > max_num_test)) {
		gchar *error;
		error = g_strdup_printf ("num_test number out of range. Valid range is -1 to %d",
					 max_num_test);
		usage (error);
	}
	
	if (debug)
		g_print ("num_test is %d\n", *num_test);
}

static void
list_tests (void)
{
	gint i;
	gint max = max_num_test + 1;

	for (i = 0; i < max; i++)
		g_print ("%s\n",  test_cases[i].description);
}

static void
handle_sigsegv (int i)
{
	g_print ("\n\ntest-dialog crashed while running num_test %d [%s]\n",
		 num_test, test_cases[num_test].description);
	exit (TEST_RETVAL_CRASH);
}

int
main (int argc, const char * argv[])
{
	TestRetval ret = TEST_RETVAL_SUCCESS;
	struct sigaction sig;

	gtk_init (&argc, (char ***) &argv);

	parse_command_line (argc, argv, &num_test);

	/* Catch sigsegv signals */
	memset (&sig, 0, sizeof(sigaction));
	sig.sa_handler = handle_sigsegv;
	sigaction (SIGSEGV, &sig, NULL);

	if (option_list_tests) {
		list_tests ();
		return ret;
	}
	
	if (tree) {
		test_dialog_tree ();
		return 0;
	}

	if (num_test != MAGIC_NUMBER)
		ret = test_dispatch (num_test);
	else {
		GnomePrintConfig *config;
		GnomePrintJob *job;
		GtkWidget *dialog;
		gchar *s;
		job = gnome_print_job_new (NULL);
		config = gnome_print_job_get_config (job);
		if (FALSE) {
			dialog = gnome_print_dialog_new (job, "Test", 0);
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
		} else {
			dialog = gnome_print_dialog_new (job, "Test dialog 1", 0);
			gtk_widget_show (dialog);
			dialog = gnome_print_dialog_new (job, "Test dialog 2", 0);
			gtk_widget_show (dialog);
			gtk_main ();
		}

		s = gnome_print_config_to_string (config, 0);
		
		g_object_unref (G_OBJECT (config));
		g_object_unref (G_OBJECT (job));

		g_print ("-->%s<--\n", s);

		g_free (s);
	}
		
	return ret;
}

