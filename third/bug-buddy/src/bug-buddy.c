/* Bug-buddy bug submitting program
 *
 * Copyright (C) 1999 - 2001 Jacob Berkman
 * Copyright 2000, 2001 Ximian, Inc.
 *
 * Author:  jacob berkman  <jacob@bug-buddy.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "bug-buddy.h"

#include "libglade-buddy.h"
#include "save-buddy.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <glade/glade.h>
#include <glade/glade-build.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnomecanvas/gnome-canvas-pixbuf.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnome/libgnometypebuiltins.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <sys/types.h>
#include <signal.h>

#define d(x)

#define DRUID_PAGE_HEIGHT 440
#define DRUID_PAGE_WIDTH  600

const gchar *submit_type[] = {
	N_("Submit bug report"),
	N_("Only send report to yourself"),
	N_("Save report to file"),
	NULL
};

const gchar *crash_type[] = {
	N_("crashed application"),
	N_("core file"),
	N_("nothing"),
	NULL
};

const gchar *bug_type[] = {
	N_("Create a new bug report"),
	N_("Add more information to existing report"),
	NULL
};

PoptData popt_data;

DruidData druid_data;

static const struct poptOption options[] = {
	{ "name",        0, POPT_ARG_STRING, &popt_data.name,         0, N_("Name of contact"),                    N_("NAME") },
	{ "email",       0, POPT_ARG_STRING, &popt_data.email,        0, N_("Email address of contact"),           N_("EMAIL") },
	{ "package",     0, POPT_ARG_STRING, &popt_data.package,      0, N_("Package containing the program"),     N_("PACKAGE") },
	{ "package-ver", 0, POPT_ARG_STRING, &popt_data.package_ver,  0, N_("Version of the package"),             N_("VERSION") },
	{ "appname",     0, POPT_ARG_STRING, &popt_data.app_file,     0, N_("File name of crashed program"),       N_("FILE") },
	{ "pid",         0, POPT_ARG_STRING, &popt_data.pid,          0, N_("PID of crashed program"),             N_("PID") },
	{ "core",        0, POPT_ARG_STRING, &popt_data.core_file,    0, N_("Core file from program"),             N_("FILE") },
	{ "include",     0, POPT_ARG_STRING, &popt_data.include_file, 0, N_("Text file to include in the report"), N_("FILE") },
	{ NULL } 
};

GtkWidget *
get_widget (const char *name, const char *loc)
{
	GtkWidget *w;
	w = glade_xml_get_widget (druid_data.xml, name);
	if (!w) {
		g_warning (_("Could not find widget named %s at %s"), name, loc);
	}
	return w;
}

void
buddy_set_text_widget (GtkWidget *w, const char *s)
{
	g_return_if_fail (GTK_IS_ENTRY (w) || 
			  GTK_IS_TEXT_VIEW (w) ||
			  GTK_IS_LABEL (w) ||
			  GTK_IS_BUTTON (w));

	if (!s) s = "";

	if (GTK_IS_ENTRY (w)) {
		gtk_entry_set_text (GTK_ENTRY (w), s);
	} else if (GTK_IS_TEXT_VIEW (w)) {
		gtk_text_buffer_set_text (
			gtk_text_view_get_buffer (GTK_TEXT_VIEW (w)),
			s, strlen (s));
	} else if (GTK_IS_LABEL (w)) {
		gtk_label_set_text (GTK_LABEL (w), s);
	} else if (GTK_IS_BUTTON (w)) {
		gtk_button_set_label (GTK_BUTTON (w), s);
	}
}

char *
buddy_get_text_widget (GtkWidget *w)
{
	char *s = NULL;
	g_return_val_if_fail (GTK_IS_ENTRY (w) || 
			      GTK_IS_TEXT_VIEW (w) ||
			      GTK_IS_LABEL (w) ||
			      GTK_IS_BUTTON (w), 
			      NULL);

	if (GTK_IS_ENTRY (w)) {
		s = g_strdup (gtk_entry_get_text (GTK_ENTRY (w)));
	} else if (GTK_IS_TEXT_VIEW (w)) {
		GtkTextIter start, end;

		gtk_text_buffer_get_bounds (
			gtk_text_view_get_buffer (GTK_TEXT_VIEW (w)),
			&start, &end);
		s = gtk_text_iter_get_text (&start, &end);
	} else if (GTK_IS_LABEL (w)) {
		s = g_strdup (gtk_label_get_text (GTK_LABEL (w)));
	} else if (GTK_IS_BUTTON (w)) {
		s = g_strdup (gtk_button_get_label (GTK_BUTTON (w)));
	}
	return s;
}

static gboolean
update_crash_type (GtkWidget *w, gpointer data)
{
	CrashType new_type = GPOINTER_TO_INT (data);
	GtkWidget *table, *box, *scrolled, *go;

	stop_gdb ();

	druid_data.crash_type = new_type;
	
	table = GET_WIDGET ("gdb-binary-table");
	box = GET_WIDGET ("gdb-core-box");
	scrolled = GET_WIDGET ("gdb-text-scrolled");
	go = GET_WIDGET ("gdb-go");

	switch (new_type) {
	case CRASH_DIALOG:
		gtk_widget_hide (box);
		gtk_widget_show (table);
		gtk_widget_set_sensitive (scrolled, TRUE);
		gtk_widget_set_sensitive (go, TRUE);
		break;
	case CRASH_CORE:
		gtk_widget_hide (table);
		gtk_widget_show (box);
		gtk_widget_set_sensitive (scrolled, TRUE);
		gtk_widget_set_sensitive (go, TRUE);
		break;
	case CRASH_NONE:
		gtk_widget_hide (table);
		gtk_widget_hide (box);
		gtk_widget_set_sensitive (scrolled, FALSE);
		gtk_widget_set_sensitive (go, FALSE);
		break;
	}

	return FALSE;
}

void
on_gdb_go_clicked (GtkWidget *w, gpointer data)
{
	druid_data.explicit_dirty = TRUE;
	start_gdb ();
}

void
on_gdb_stop_clicked (GtkWidget *button, gpointer data)
{
	GtkWidget *w;
	if (!druid_data.fd)
		return;

	w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
				    0,
				    GTK_MESSAGE_QUESTION,
				    GTK_BUTTONS_YES_NO,
				    _("gdb has not finished getting the "
				      "debugging information.\n"
				      "Kill the gdb process (the stack trace "
				      "will be incomplete)?"));

	gtk_dialog_set_default_response (GTK_DIALOG (w),
					 GTK_RESPONSE_YES);

	if (GTK_RESPONSE_YES == gtk_dialog_run (GTK_DIALOG (w))) {
		gtk_widget_destroy (w);
		if (druid_data.gdb_pid == 0) {
			d(g_message (_("gdb has already exited")));
			return;
		}
		stop_gdb ();
		druid_data.explicit_dirty = TRUE;
	}
	gtk_widget_destroy (w);
}

void
on_gdb_copy_clicked (GtkWidget *w, gpointer data)
{
	GtkWidget *text;
	GtkTextBuffer *buffy;
	GtkTextIter start_iter, end_iter;

	text = GET_WIDGET ("gdb-text");
	buffy = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffy, &start_iter, &end_iter);
	gtk_text_buffer_move_mark_by_name (buffy, "insert", &start_iter);
	gtk_text_buffer_move_mark_by_name (buffy, "selection_bound", &end_iter);
	gtk_text_buffer_copy_clipboard (buffy, gtk_clipboard_get (GDK_NONE));
}

void
on_gdb_save_clicked (GtkWidget *w, gpointer data)
{
	GtkWidget *filesel;
	int response;

	filesel = gtk_file_selection_new (_("Save Backtrace"));
 run_save_dialog:
	response = gtk_dialog_run (GTK_DIALOG (filesel));
	if (response == GTK_RESPONSE_OK) {
		const char *file;
		GError *gerr = NULL;
		char *text;

		file = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));
		text = buddy_get_text ("gdb-text");
		if (!bb_write_buffer_to_file (GTK_WINDOW (filesel), 
					      _("Please wait while Bug Buddy saves the stack trace..."),
					      file, text, -1, &gerr)) {
			if (gerr) {
				GtkWidget *d;
				
				d = gtk_message_dialog_new (GTK_WINDOW (filesel),
							    0,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_OK,
							    _("The stack trace was not saved in %s:\n\n"
							      "%s\n\n"
							      "Please try again, maybe with a different file name."),
							    file, gerr->message);
				gtk_dialog_run (GTK_DIALOG (d));
				gtk_widget_destroy (d);
				g_error_free (gerr);
			}
			g_free (text);
			goto run_save_dialog;
		}

		g_free (text);
	}
	
	gtk_widget_destroy (filesel);
}

void
on_product_toggle_clicked (GtkWidget *w, gpointer data)
{
	char *button;

	g_return_if_fail (druid_data.state == STATE_PRODUCT);

	druid_data.show_products = !druid_data.show_products;

	if (druid_data.show_products) {
		button = _("Show _Applications");
	} else {
		button = _("Show _Products");
	}

	buddy_set_text ("product-toggle",    button);

	products_list_load ();
}

static void
on_list_button_press_event (GtkWidget *w, GdkEventButton *button, gpointer data)
{
	GtkTreeSelection *selection;

	if (button->type != GDK_2BUTTON_PRESS)
		return;

	if (druid_data.download_in_progress)
		return;
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (w));
	if (!gtk_tree_selection_get_selected (selection, NULL, NULL))
		return;

	on_druid_next_clicked (NULL, NULL);
}


void
on_file_radio_toggled (GtkWidget *radio, gpointer data)
{
	static GtkWidget *entry2 = NULL;
	int on = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio));
	if (!entry2)
		entry2 = glade_xml_get_widget (druid_data.xml,
					       "file_entry2");
	
	gtk_widget_set_sensitive (GTK_WIDGET (entry2), on);
}

GtkWidget *
stock_pixmap_buddy (gchar *w, char *n, char *a, int b, int c)
{
	return gtk_image_new_from_stock (n, GTK_ICON_SIZE_SMALL_TOOLBAR);
}

/* this is pretty fragile */
void
on_email_group_toggled (GtkWidget *w, gpointer data)
{
	const char *label = GTK_STOCK_GO_FORWARD;
	gboolean using_sendmail;

	using_sendmail = GTK_TOGGLE_BUTTON (GET_WIDGET ("email-sendmail-radio"))->active;

	gtk_widget_set_sensitive (GET_WIDGET ("email-sendmail-frame"), using_sendmail);
	druid_data.submit_type = using_sendmail ? SUBMIT_SENDMAIL : SUBMIT_FILE;

	if (druid_data.submit_type == SUBMIT_FILE) {
		gtk_widget_hide (GET_WIDGET ("email-to-table"));
		gtk_widget_show (GET_WIDGET ("email-save-in-box"));
	} else {
		gtk_widget_show (GET_WIDGET ("email-to-table"));
		gtk_widget_hide (GET_WIDGET ("email-save-in-box"));
	}

	switch (druid_data.state) {
	case STATE_EMAIL_CONFIG:
		if (druid_data.submit_type == SUBMIT_GNOME_MAILER)
			label = _("_Start Mailer");
		break;
	case STATE_EMAIL:
		if (druid_data.submit_type == SUBMIT_SENDMAIL)
			label = _("_Send Report");
		else
			label = GTK_STOCK_SAVE;
		break;
	default:
		break;
	}

	g_object_set (GET_WIDGET ("druid-next"),
		      "label", label,
		      "use-underline", TRUE,

		      NULL);
}

void
on_debugging_options_button_clicked (GtkWidget *w, gpointer null)
{
	GtkWidget *nb;
	int page;

	nb = GET_WIDGET ("gdb-notebook");

	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (nb)) ? 0 : 1;
	
	gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), page);
	gtk_button_set_label (GTK_BUTTON (w),
			      page
			      ? _("Hide Debugging Options")
			      : _("Show Debugging Options"));
}

void
on_email_default_radio_toggled (GtkWidget *w, gpointer data)
{
	static const char *widgets[] = {
		"email-command-label",
		"email-command-entry",
		"email-terminal-toggle",
		"email-remote-toggle",
		NULL
	};
	const char **s;
	gboolean custom = GTK_TOGGLE_BUTTON (GET_WIDGET ("email-custom-radio"))->active;

	gtk_widget_set_sensitive (GET_WIDGET ("email-default-combo"), !custom);

	for (s = widgets; *s; s++) 
		gtk_widget_set_sensitive (GET_WIDGET (*s), custom);
}

static void
fixup_notebook (const char *name)
{
	GtkNotebook *nb = GTK_NOTEBOOK (GET_WIDGET (name));

	gtk_notebook_set_show_border (GTK_NOTEBOOK (nb), FALSE);
	gtk_notebook_set_show_tabs   (GTK_NOTEBOOK (nb), FALSE);
}

static void
init_gnome_version_stuff (void)
{
	xmlDoc *doc;
	char *xml_file;
	xmlNode *node;
	char *platform, *minor, *micro, *vendor;


	druid_data.gnome_version = g_getenv ("BUG_BUDDY_GNOME_VERSION");

	xml_file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_DATADIR,
					      "gnome-about/gnome-version.xml",
					      TRUE, NULL);
	if (!xml_file)
		return;
	doc = xmlParseFile (xml_file);
	if (!doc)
		return;

	platform = minor = micro = vendor = NULL;
	
	for (node = xmlDocGetRootElement (doc)->children; node; node = node->next) {
		if (!strcmp (node->name, "platform"))
			platform = xmlNodeGetContent (node);
		else if (!strcmp (node->name, "minor"))
			minor = xmlNodeGetContent (node);
		else if (!strcmp (node->name, "micro"))
			micro = xmlNodeGetContent (node);
		else if (!strcmp (node->name, "vendor"))
			vendor = xmlNodeGetContent (node);
	}
	
	if (platform && minor && micro)
		druid_data.gnome_platform_version = g_strdup_printf ("GNOME%s.%s.%s", platform, minor, micro);
  
	if (vendor && *vendor)
		druid_data.gnome_vendor = g_strdup (vendor);
	
	xmlFree (platform);
	xmlFree (minor);
	xmlFree (micro);
	xmlFree (vendor);
	
	xmlFreeDoc (doc);
}

/* there should be no setting of default values here, I think */
static void
init_ui (void)
{
	GtkWidget *w, *m;
	char *s;
	const char **sp;
	int i;
	const char *nbs[] = { "druid-notebook", "gdb-notebook", NULL };
	
	glade_xml_signal_autoconnect (druid_data.xml);

	load_config ();

	/* gtk_widget_set_default_direction (GTK_TEXT_DIR_RTL); */

	for (sp = nbs; *sp; sp++)
		fixup_notebook (*sp);

	/* dialog crash page */
	s = popt_data.app_file;
	if (!s) {
		s = getenv ("GNOME_CRASHED_APPNAME");
		if (s)
			g_warning (_("$GNOME_CRASHED_APPNAME is deprecated.\n"
				     "Please use the --appname command line"
				     "argument instead."));
	}
	buddy_set_text ("gdb-binary-entry", s);

	s = popt_data.pid;
	if (!s) {
		s = getenv ("GNOME_CRASHED_PID");
		if (s)
			g_warning (_("$GNOME_CRASHED_PID is deprecated.\n"
				     "Please use the --pid command line"
				     "argument instead."));
	}
	if (s) {
		buddy_set_text ("gdb-pid-entry", s);
		druid_data.crash_type = CRASH_DIALOG;
	}

	/* core crash page */
	if (popt_data.core_file) {
		buddy_set_text ("gdb-core-entry", popt_data.core_file);
		druid_data.crash_type = CRASH_CORE;
	}

	{
		char *message = g_strconcat (_("Welcome to Bug Buddy, a bug reporting tool for GNOME.\n"
					       "It will step you through the process of submitting a bug report."),
					     druid_data.crash_type == CRASH_DIALOG
					     ? _("\n\nYou are seeing this because another application has crashed.")
					     : "",
					     druid_data.crash_type != CRASH_NONE
					     ? _("\n\nPlease wait a few moments while Bug Buddy obtains debugging information "
						 "from the applications.  This allows Bug Buddy to provide a more useful "
						 "bug report.")
					     : "",
					     NULL);
		g_object_set (GET_WIDGET ("gdb-label"),
			      "label", message,
			      "selectable", TRUE,
			      NULL);
		g_free (message);
	}

	if (druid_data.crash_type != CRASH_NONE) 

	/* package version */
	buddy_set_text ("the-version-entry", popt_data.package_ver);

        /* init some ex-radio buttons */
	m = gtk_menu_new ();
	for (i = 0; crash_type[i]; i++) {
		w = gtk_menu_item_new_with_label (_(crash_type[i]));
		g_signal_connect (G_OBJECT (w), "activate",
				  G_CALLBACK (update_crash_type),
				  GINT_TO_POINTER (i));
		gtk_widget_show (w);
		gtk_menu_shell_append (GTK_MENU_SHELL (m), w);
	}
	w = GET_WIDGET ("gdb-option");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (w), m);
	gtk_option_menu_set_history (GTK_OPTION_MENU (w), druid_data.crash_type);
	update_crash_type (NULL, GINT_TO_POINTER (druid_data.crash_type));

	gnome_window_icon_set_from_default (GTK_WINDOW (GET_WIDGET ("druid-window")));

	{
		GtkButtonBox *bbox;
		
		bbox = GTK_BUTTON_BOX (GET_WIDGET ("druid-bbox"));
		gtk_button_box_set_child_secondary (
			bbox, GET_WIDGET ("druid-help"), TRUE);
		gtk_button_box_set_child_secondary (
			bbox, GET_WIDGET ("druid-about"), TRUE);
	}
		
	gtk_misc_set_alignment (GTK_MISC (GET_WIDGET ("druid-logo")),
				1.0, 0.5);

	w = GET_WIDGET ("druid-next");
	gtk_widget_set_direction (GTK_BIN (GTK_BIN (w)->child)->child,
				  gtk_widget_get_direction (w) == GTK_TEXT_DIR_RTL
				  ? GTK_TEXT_DIR_LTR
				  : GTK_TEXT_DIR_RTL);

	g_signal_connect (GET_WIDGET ("product-list"), "event-after",
			  G_CALLBACK (on_list_button_press_event),
			  NULL);

	g_signal_connect (GET_WIDGET ("component-list"), "event-after",
			  G_CALLBACK (on_list_button_press_event),
			  NULL);

	{
		const char **w, *windows[] = { "druid-window", "update-dialog", "progress-dialog", NULL };
		for (w = windows; *w; w++)
			g_signal_connect (GET_WIDGET (*w), "delete-event", 
					  G_CALLBACK (gtk_widget_hide_on_delete),
					  NULL);
	}

	buddy_set_text ("desc-text",
			"Description of Problem:\n\n\n"
			"Steps to reproduce the problem:\n"
			"1. \n"
			"2. \n"
			"3. \n\n"
			"Actual Results:\n\n\n"
			"Expected Results:\n\n\n"
			"How often does this happen?\n\n\n"
			"Additional Information:\n");

	/* set the cursor at the beginning of the second line */
	{
		GtkTextBuffer *buffy;
		GtkTextIter iter;

		buffy = gtk_text_view_get_buffer (GTK_TEXT_VIEW (GET_WIDGET ("desc-text")));
		gtk_text_buffer_get_iter_at_line (buffy, &iter, 1);
		gtk_text_buffer_place_cursor (buffy, &iter);
	}
}

GtkWidget *
make_image (char *widget_name, char *icon, char *s2, int stock, int i2)
{
	GtkWidget *w = NULL;

	if (stock) {
		GtkIconSize size = GTK_ICON_SIZE_BUTTON;

		if (s2)
			size = glade_enum_from_string (GTK_TYPE_ICON_SIZE, s2);

		w = gtk_image_new_from_stock (icon, size);
	} else {
		GnomeFileDomain domain = GNOME_FILE_DOMAIN_APP_PIXMAP;
		char *filename;
		
		if (s2)
			domain = glade_enum_from_string (GNOME_TYPE_FILE_DOMAIN, s2);

		filename = gnome_program_locate_file (NULL, domain, icon, TRUE, NULL);

		if (filename)
			w = gtk_image_new_from_file (filename);
		else
			g_warning (_("Could not find pixmap: %s (%d)\n"), icon, domain);
		g_free (filename);
	}

	if (w)
		gtk_widget_show (w);

	return w;
}

gint
delete_me (GtkWidget *w, GdkEventAny *evt, gpointer data)
{
	save_config ();
	gtk_main_quit ();
	return FALSE;
}

void
on_druid_window_style_set (GtkWidget *widget, GtkStyle *old_style, gpointer data)
{
	gtk_widget_modify_fg (GET_WIDGET ("druid-label"), GTK_STATE_NORMAL, &widget->style->fg[GTK_STATE_SELECTED]);
	gtk_widget_modify_bg (GET_WIDGET ("druid-background"), GTK_STATE_NORMAL, &widget->style->bg[GTK_STATE_SELECTED]);
}

int
main (int argc, char *argv[])
{
	GtkWidget *w;
	char *s;
		
	memset (&druid_data, 0, sizeof (druid_data));
	memset (&popt_data,  0, sizeof (popt_data));

	druid_data.crash_type = CRASH_NONE;
	druid_data.state = -1;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);

	gnome_program_init (PACKAGE, VERSION,
			    LIBGNOMEUI_MODULE,
			    argc, argv,
			    GNOME_PARAM_POPT_TABLE, options,
			    GNOME_PARAM_APP_DATADIR, REAL_DATADIR,
			    NULL);

	s = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP, 
				       "bug-buddy.png", TRUE, NULL);
	
	if (s)
		gnome_window_icon_set_default_from_file (s);
	g_free (s);

	s = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_DATADIR,
				       "bug-buddy/bug-buddy.glade", TRUE, NULL);
	if (s)
		druid_data.xml = glade_xml_new (s, NULL, GETTEXT_PACKAGE);
	else
		druid_data.xml = NULL;

	if (!druid_data.xml) {
		w = gtk_message_dialog_new (NULL,
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Bug Buddy could not load its user interface file (%s).\n"
					      "Please make sure Bug Buddy was installed correctly."), 
					    s);
					      
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		return 0;
	}
	g_free (s);	

	druid_set_state (STATE_GDB);

	init_gnome_version_stuff ();
	init_ui ();

	load_bugzillas ();
	if (druid_data.need_to_download) {
		int response;
		if (gtk_dialog_run (GTK_DIALOG (GET_WIDGET ("update-dialog"))) == GTK_RESPONSE_OK) {
			start_bugzilla_download ();
			response = gtk_dialog_run (GTK_DIALOG (GET_WIDGET ("progress-dialog")));
			if (response == GTK_RESPONSE_CANCEL) {
				gnome_vfs_async_cancel (druid_data.vfshandle);
				gtk_widget_destroy(GET_WIDGET ("progress-dialog"));
				druid_data.download_in_progress = FALSE;
			} else if (response == DOWNLOAD_FINISHED_ZERO_RESPONSE) {
				w = gtk_message_dialog_new (NULL,
							    0,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_OK,
							    _("Bug Buddy could not update its bug information.\n"
							      "The old one will be used."));
					      
				gtk_dialog_set_default_response (GTK_DIALOG (w),
						 		GTK_RESPONSE_OK);
				gtk_dialog_run (GTK_DIALOG (w));
				gtk_widget_destroy (w);

				gtk_widget_destroy(GET_WIDGET ("progress-dialog"));
			} else if (response == DOWNLOAD_FINISHED_OK_RESPONSE) {
				w = gtk_message_dialog_new (NULL,
							    0,
							    GTK_MESSAGE_INFO,
							    GTK_BUTTONS_OK,
							    _("Bug Buddy updated its bug information correctly."));
					      
				gtk_dialog_set_default_response (GTK_DIALOG (w),
						 		GTK_RESPONSE_OK);
				gtk_dialog_run (GTK_DIALOG (w));
				gtk_widget_destroy (w);

				gtk_widget_destroy(GET_WIDGET ("progress-dialog"));
			}
		}
		
		gtk_widget_destroy (GET_WIDGET ("update-dialog"));
	}
	
	load_bugzilla_xml ();

	gtk_widget_show (GET_WIDGET ("druid-window"));

	load_applications ();
	start_gdb ();

	gtk_main ();

	return 0;
}
