/* bug-buddy bug submitting program
 *
 * Copyright (C) 2000 Jacob Berkman
 *
 * Author:  Jacob Berkman  <jacob@bug-buddy.org>
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

#include <gnome.h>
#include <string.h>

#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#if 0
static char *help_pages[] = {
	"index.html",
	"debug-info.html",
	"description.html",
	"updating.html",
	"product.html",
	"component.html",
	"system-config.html",
	"submit-report.html",
	"summary.html",
	NULL
};
#endif

static char *state_title[] = {
	N_("Welcome to Bug Buddy"),
	N_("Select a Product"),
	N_("Select a Component"),
	N_("Frequently Reported Bugs"),
	N_("Bug Description"),
	N_("Mail Configuration"),
	N_("Confirmation"),
	N_("Finished!"),
	NULL
};

#define d(x)

void
druid_set_sensitive (gboolean prev, gboolean next, gboolean cancel)
{
	gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), prev);
	gtk_widget_set_sensitive (GET_WIDGET ("druid-next"), next);
	gtk_widget_set_sensitive (GET_WIDGET ("druid-cancel"), cancel);
}

void
on_druid_help_clicked (GtkWidget *w, gpointer data)
{
	gnome_help_display ("bug-buddy", NULL, NULL);
}

void
on_druid_about_clicked (GtkWidget *button, gpointer data)
{
	static GtkWidget *about;
	static const char *authors[] = {
		"Jacob Berkman  <jacob@bug-buddy.org>",
		NULL
	};

	static const char *documentors[] = {
		"Telsa Gwynne  <hobbit@aloss.ukuu.org.uk>",
		NULL
	};

	if (about) {
		gtk_window_present (GTK_WINDOW (about));
		return;
	}
		
	about = gnome_about_new (_("Bug Buddy"), VERSION,
				 "Copyright (C) 1999, 2000, 2001, 2002, Jacob Berkman\n"
				 "Copyright 2000, 2001, 2002 Ximian, Inc.",
				 _("The graphical bug reporting tool for GNOME."),
				 authors,
				 documentors,
				 NULL, NULL);

	g_signal_connect (G_OBJECT (about), "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &about);

#if 0
	href = gnome_href_new ("http://bug-buddy.org/",
			       _("The lame Bug Buddy web page"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (about)->vbox),
			    href, FALSE, FALSE, 0);
	gtk_widget_show (href);
#endif
	gtk_window_set_transient_for (GTK_WINDOW (about),
				      GTK_WINDOW (GET_WIDGET ("druid-window")));
	gtk_widget_show (about);
}

void
druid_set_state (BuddyState state)
{
	static gboolean been_here = FALSE;
	BuddyState oldstate;
	GtkWidget *w;
	char *s;

	g_return_if_fail (state >= 0);
	g_return_if_fail (state < STATE_LAST);

	if (druid_data.state == state && been_here)
		return;

	been_here = TRUE;

	oldstate = druid_data.state;
	druid_data.state = state;

	druid_set_sensitive ((state > 0), (state < STATE_FINISHED), TRUE);

	{
		GtkLabel *label = GTK_LABEL (GET_WIDGET ("druid-label"));
		char *s = g_strconcat ("<span size=\"xx-large\" weight=\"ultrabold\">",
				       _(state_title[state]), "</span>", NULL);
		gtk_label_set_label (label, s);
		gtk_label_set_use_markup (label, TRUE);
		g_free (s);
	}

	gtk_notebook_set_current_page (GTK_NOTEBOOK (GET_WIDGET ("druid-notebook")),
				       state);	

	switch (druid_data.state) {
	case STATE_GDB:
		break;
	case STATE_PRODUCT:
#if 0
		if (!druid_data.package_name)
			determine_our_package ();
		buddy_set_text ("bts-package-entry".
				druid_data.package_name);
#endif
#if 0
		load_bugzilla_xml ();
#endif
#if 0
		if (!druid_data.product)
			druid_set_sensitive (TRUE, FALSE,  TRUE);
#endif
		break;
	case STATE_COMPONENT:
#if 0
		if (!druid_data.component)
			druid_set_sensitive (TRUE, FALSE,  TRUE);
#endif
		break;
	case STATE_MOSTFREQ:
		/* nothing to do */
		break;		
	case STATE_DESC:
		/* nothing to do */
		break;
#if 0
	case STATE_SYSTEM:
		/* start the process of version checking if we haven't
		 * run anything or the list of thingies has changed */
		do_dependency_stuff ();
		druid_set_state (state - 1);
		break;
#endif
	case STATE_EMAIL_CONFIG:
		/* FIXME: change next icon */
		on_email_group_toggled (NULL, NULL);
		break;
	case STATE_EMAIL:
		/* fill in the content text */
		s = generate_email_text (TRUE);
		w = GET_WIDGET ("email-text");
		buddy_set_text ("email-text", s);
		g_free (s);
		on_email_group_toggled (NULL, NULL);
		break;
	case STATE_FINISHED:
		/* print a summary yo */
		gtk_widget_hide (GET_WIDGET ("druid-prev"));
		gtk_widget_hide (GET_WIDGET ("druid-next"));
		gtk_widget_show (GET_WIDGET ("druid-finish"));
		gtk_widget_hide (GET_WIDGET ("druid-cancel"));
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

void
on_druid_prev_clicked (GtkWidget *w, gpointer data)
{
	BuddyState newstate = druid_data.state - 1;

	switch (druid_data.state) {
	case STATE_DESC:
		if (GTK_TOGGLE_BUTTON (GET_WIDGET ("no-product-toggle"))->active)
			newstate = STATE_PRODUCT;
		else if (!druid_data.product->bts->bugs)
			newstate = STATE_COMPONENT;
		break;
	default:
		break;
	}

	druid_set_state (newstate);
}


static gboolean
email_is_valid (const char *addy)
{
	char *rev;

	if (!addy || strlen (addy) < 4 || !strchr (addy, '@') || strstr (addy, "@.") || strstr (addy, "root@"))
		return FALSE;

	g_strreverse (rev = g_strdup (addy));
	
	/* assume that the country thingies are ok */
	if (rev[2] == '.') {
		g_free (rev);
		return TRUE ;
	}

	if (g_ascii_strncasecmp (rev, "moc.", 4) &&
	    g_ascii_strncasecmp (rev, "gro.", 4) &&
	    g_ascii_strncasecmp (rev, "ten.", 4) &&
	    g_ascii_strncasecmp (rev, "ude.", 4) &&
	    g_ascii_strncasecmp (rev, "lim.", 4) &&
	    g_ascii_strncasecmp (rev, "vog.", 4) &&
	    g_ascii_strncasecmp (rev, "tni.", 4) &&
	    g_ascii_strncasecmp (rev, "apra.", 5) &&

	    /* In the year 2000, seven new toplevel domains were approved by ICANN. */
	    g_ascii_strncasecmp (rev, "orea.", 5) &&
	    g_ascii_strncasecmp (rev, "zib.", 4) &&
	    g_ascii_strncasecmp (rev, "pooc.", 5) &&
	    g_ascii_strncasecmp (rev, "ofni.", 5) &&
	    g_ascii_strncasecmp (rev, "muesum.", 5) &&
	    g_ascii_strncasecmp (rev, "eman.", 5) &&
	    g_ascii_strncasecmp (rev, "orp.", 5)) {
		g_free (rev);
		return FALSE;
	}

	g_free (rev);

	return TRUE;
}

/* return true if page is ok */
static gboolean
mail_config_page_sendmail_ok (void)
{
	GtkWidget *w;
	gchar *s;

	/* FIXME: check mail config type */

	s = buddy_get_text ("email-name-entry");
	if (! (s && strlen (s))) {
		g_free (s);
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Please enter your name."));
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		return FALSE;
	}
	g_free (s);

	s = buddy_get_text ("email-email-entry");
	if (!email_is_valid (s)) {
		g_free (s);
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Please enter a valid email address."));
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		return FALSE;
	}
	g_free (s);

	s = buddy_get_text ("email-sendmail-entry");
	if (!g_file_test (s, G_FILE_TEST_EXISTS)) {
		if (druid_data.submit_type == SUBMIT_SENDMAIL) {
			w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
						    0,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK,
						    _("'%s' doesn't seem to exist.\n\n"
						      "Please check the path to sendmail again."),
						    s);
			gtk_dialog_set_default_response (GTK_DIALOG (w),
							 GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (w));
			g_free (s);
			gtk_widget_destroy (w);
			return FALSE;
		}
	}
	g_free (s);

	return TRUE;
}

static gboolean
mail_config_page_gnome_ok (void)
{
	GtkWidget *w;
	char *s;

	if (GTK_TOGGLE_BUTTON (GET_WIDGET ("email-default-radio"))->active)
		return TRUE;

	s = buddy_get_text ("email-command-entry");
	if (! (s && strlen (s))) {
		g_free (s);
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Please enter a valid email command."));
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		return FALSE;
	}

	return TRUE;
}

static gboolean
mail_config_page_ok (void)
{
	switch (druid_data.submit_type) {
	case SUBMIT_GNOME_MAILER:
		return mail_config_page_gnome_ok ();
	case SUBMIT_SENDMAIL:
		return mail_config_page_sendmail_ok ();
	case SUBMIT_FILE:
		return TRUE;
	}
	return FALSE;
}

static gboolean
text_is_sensical (const gchar *text, int sensitivity)
{
        /* If there are less than eight unique characters, 
	   it is probably nonsenical.  Also require a space */
	int chars[256] = { 0 };
	guint uniq = 0;

	if (!text || !*text)
		return FALSE;
	
	for ( ; *text; text++)
		if (!chars[(guchar)*text])
			chars[(guchar)*text] = ++uniq;
	
	d(g_message ("%d", uniq));

	return chars[' '] && uniq >= sensitivity;
 }

static gboolean
desc_page_ok (void)
{
	GtkWidget *w;

	char *s = buddy_get_text ("desc-file-entry");

	if (getenv ("BUG_ME_HARDER")) {
		g_free (s);
		return TRUE;
	}

	if (s && *s) {
		const char *mime_type;
		if (!g_file_test (s, G_FILE_TEST_EXISTS)) {
			w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
						    0,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK,
						    _("The specified file does not exist."));
			gtk_dialog_set_default_response (GTK_DIALOG (w),
							 GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (w));
			gtk_widget_destroy (w);
			g_free (s);
			return FALSE;
		}

		mime_type = gnome_vfs_get_mime_type (s);
		d(g_message (_("File is of type: %s"), mime_type));
		
		if (strncmp ("text/", mime_type, 5)) {
			w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
						    0,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK,
						    _("'%s' is a %s file.\n\n"
						      "Bug Buddy can only submit plaintext (text/*) files."),
						    s, mime_type);
			gtk_dialog_set_default_response (GTK_DIALOG (w),
							 GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (w));
			gtk_widget_destroy (w);
			g_free (s);
			return FALSE;
		}
	}
	g_free (s);

	s = buddy_get_text ("desc-subject");
	if (!text_is_sensical (s, 6)) {
		g_free (s);
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("You must include a comprehensible subject line in your bug report."));
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		return FALSE;
	}
	g_free (s);

	s = buddy_get_text ("desc-text");
	if (!text_is_sensical (s, 8)) {
		g_free (s);
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("You must include a comprehensible description in your bug report."));
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		return FALSE;
	}
	g_free (s);
	return TRUE;
}

static gboolean
submit_ok (void)
{
	gchar *to, *s, *file=NULL, *command;
	GtkWidget *w;
	FILE *fp;

	enum {
		RESPONSE_SUBMIT,
		RESPONSE_CANCEL
	};

	if (druid_data.submit_type != SUBMIT_FILE) {
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_QUESTION,
					    GTK_BUTTONS_NONE,
					    _("Submit this bug report now?"));
		gtk_dialog_add_buttons (GTK_DIALOG (w),
					GTK_STOCK_CANCEL, RESPONSE_CANCEL,
					_("_Submit"), RESPONSE_SUBMIT,
					NULL);
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_YES);
		if (RESPONSE_SUBMIT != gtk_dialog_run (GTK_DIALOG (w))) {
			gtk_widget_destroy (w);
			return FALSE;
		}
		gtk_widget_destroy (w);
	}

	to = buddy_get_text ("email-to-entry");

	if (druid_data.submit_type == SUBMIT_FILE) {
		file = buddy_get_text ("email-file-entry");
		fp = fopen (file, "w");
		if (!fp) {
			w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
						    0,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK,
						    _("Unable to open file '%s':\n%s"), 
						    file, g_strerror (errno));
			g_free (file);
			g_free (to);
			gtk_dialog_set_default_response (GTK_DIALOG (w),
							 GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (w));
			gtk_widget_destroy (w);
			return FALSE;
		}
	} else {
		s = buddy_get_text ("email-sendmail-entry");
		command = g_strdup_printf ("%s -i -t", s);

		d(g_message (_("about to run '%s'"), command));
		fp =  popen (command, "w");
		g_free (command);
		if (!fp) {
			w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
						    0,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK,
						    _("Unable to start mail program '%s':\n%s"), 
						    s, g_strerror (errno));
			gtk_dialog_set_default_response (GTK_DIALOG (w),
							 GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (w));
			gtk_widget_destroy (w);
			g_free (s);
			g_free (to);
			return FALSE;
		}
		g_free (s);
	}

	{
		char *name, *from;
		
		name = buddy_get_text ("email-name-entry");
		from = buddy_get_text ("email-email-entry");

		fprintf (fp, "From: %s <%s>\n", name, from);
		g_free (from);
		g_free (name);
	}

	fprintf (fp, "To: %s\n", to);
	
	s = buddy_get_text ("email-cc-entry");
	if (*s) fprintf (fp, "Cc: %s\n", s);
	g_free (s);

	fprintf (fp, "X-Mailer: %s %s\n", PACKAGE, VERSION);

	s = buddy_get_text ("email-text");
	fprintf (fp, "%s", s);
	g_free (s);

	if (druid_data.submit_type == SUBMIT_FILE) {
		fclose (fp);
		s = g_strdup_printf (_("Your bug report was saved in '%s'"), file);
	} else {
		pclose (fp);
		s = g_strdup_printf (_("Your bug report has been submitted to:\n\n        <%s>\n\nThanks!"), to);
	}
	g_free (to);

	buddy_set_text ("finished-label", s);
	g_free (file);
	g_free (s);

	return TRUE;
}

static gpointer
get_selected_row (const char *w, int col)
{
	GtkTreeView *view;
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gpointer retval;

	view = GTK_TREE_VIEW (GET_WIDGET (w));
	selection = gtk_tree_view_get_selection (view);
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return NULL;

	gtk_tree_model_get (model, &iter, col, &retval, -1);			    
	
	return retval;
}

void
on_druid_next_clicked (GtkWidget *w, gpointer data)
{
	GtkWidget *d;
	BuddyState newstate;
	char *s;

	newstate = druid_data.state + 1;

	switch (druid_data.state) {
	case STATE_GDB:
		/* nothing */
		break;
	case STATE_PRODUCT:
	{
		BugzillaProduct *product;
		/* check that the package is ok */
		if (GTK_TOGGLE_BUTTON (GET_WIDGET ("no-product-toggle"))->active) {
			static gboolean dialog_shown = FALSE;
			if (!dialog_shown) {
				d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
							    0,
							    GTK_MESSAGE_WARNING,
							    GTK_BUTTONS_OK,
							    _("Since Bug Buddy doesn't know about the product "
							      "you wish to submit a bug report in, you will have "
							      "to manually address the bug report."));
				gtk_dialog_set_default_response (GTK_DIALOG (d), GTK_RESPONSE_OK);
				gtk_dialog_run (GTK_DIALOG (d));
				gtk_widget_destroy (d);
				dialog_shown = TRUE;
			}
			buddy_set_text ("email-to-entry", "");
			druid_data.product = NULL;
			druid_data.component = NULL;
			newstate = STATE_DESC;
			break;
		}
 
		product = get_selected_row ("product-list", PRODUCT_DATA);
		if (!product) {
			d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
						    0,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK,
						    _("You must specify a product for your bug report."));
			gtk_dialog_set_default_response (GTK_DIALOG (d),
							 GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (d));
			gtk_widget_destroy (d);
			return;
		}
		if (product != druid_data.product) {
			druid_data.product = product;
			bugzilla_product_add_components_to_clist (druid_data.product);
			buddy_set_text ("email-to-entry", druid_data.product->bts->email);
		}
		break;
	}
	case STATE_COMPONENT:
	{
		BugzillaComponent *component;
		component = get_selected_row ("component-list", COMPONENT_DATA);
		if (!component) {
			d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
						    0,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK,
						    _("You must specify a component for your bug report."));
			gtk_dialog_set_default_response (GTK_DIALOG (d),
							 GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (d));
			gtk_widget_destroy (d);
			return;
		}
		s = buddy_get_text ("the-version-entry");
		if (!s[0] && !getenv ("BUG_ME_HARDER")) {
			g_free (s);
			d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
						    0,
						    GTK_MESSAGE_ERROR,
						    GTK_BUTTONS_OK,
						    _("You must specify a version for your bug report."));
			gtk_dialog_set_default_response (GTK_DIALOG (d),
							 GTK_RESPONSE_OK);
			gtk_dialog_run (GTK_DIALOG (d));
			gtk_widget_destroy (d);
			return;
		}
		g_free (s);
		if (component != druid_data.component) {
			druid_data.component = component;
			bugzilla_add_mostfreq (druid_data.product->bts);
		}
		if (!druid_data.product->bts->bugs)
			newstate++;
		break;
	}
	case STATE_MOSTFREQ:
		break;
	case STATE_DESC:
		/* validate subject, description, and file name */
		if (!desc_page_ok ())
			return;
		break;
	case STATE_EMAIL_CONFIG:
		if (!mail_config_page_ok ())
			return;

		if (GTK_TOGGLE_BUTTON (GET_WIDGET ("email-mailer-radio"))->active) {
			MailerItem *mailer;
			char *orig_body, *uri_body, *to, *orig_subject, *uri_subject;
			int argc;
			char **argv;

			if (GTK_TOGGLE_BUTTON (GET_WIDGET ("email-custom-radio"))->active) {
				mailer = &druid_data.custom_mailer;
			} else {
				char *s;
				s = buddy_get_text ("email-default-entry");
				mailer = g_hash_table_lookup (druid_data.mailer_hash, s);
				g_free (s);
			}

			/* FIXME: validate mailer */

			g_shell_parse_argv (mailer->command, &argc, &argv, NULL);
			argv = g_realloc (argv, ++argc);

			/* escape from la */
			orig_body  = generate_email_text (FALSE);
			uri_body   = gnome_vfs_escape_string (orig_body);

			orig_subject = buddy_get_text ("desc-subject");
			uri_subject  = gnome_vfs_escape_string (orig_subject);

			if (druid_data.product)
				to = gnome_vfs_escape_string (druid_data.product->bts->email);
			else
				to = g_strdup ("");

			argv[argc-1] = g_strdup_printf ("mailto:%s?subject=%s&body=%s", to, uri_subject, uri_body);
			argv[argc]   = NULL;

			{
				char **s;
				for (s = argv; *s; s++)
					g_print ("%s\n", *s);
			}

			/* FIXME: check for errors */
			g_spawn_async (NULL, argv, NULL,
				       G_SPAWN_SEARCH_PATH,
				       NULL, NULL, NULL, NULL);

			g_strfreev (argv);
			g_free (orig_body);
			g_free (uri_body);
			g_free (orig_subject);
			g_free (uri_subject);
			g_free (to);

			newstate = STATE_FINISHED;

			buddy_set_text ("finished-label", 
					_("Your email program has been launched.  Please look it over and send it.\n\n"
					  "Thank you for submitting this bug report."));
		}
		break;
	case STATE_EMAIL:
		/* validate included file.
		 * prompt that we should actually do anything  */
		if (!submit_ok ())
			return;
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	druid_set_state (newstate);
}

void
on_druid_cancel_clicked (GtkWidget *w, gpointer data)
{
	GtkWidget *d;

	d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
				    0,
				    GTK_MESSAGE_QUESTION,
				    GTK_BUTTONS_YES_NO,
				    _("Are you sure you want to cancel\n"
				      "this bug report?"));
	gtk_dialog_set_default_response (GTK_DIALOG (d),
					 GTK_RESPONSE_YES);
	if (GTK_RESPONSE_YES != gtk_dialog_run (GTK_DIALOG (d))) {
		gtk_widget_destroy (d);
		return;
	}
	gtk_widget_destroy (d);

	save_config ();
	gtk_main_quit ();
}
