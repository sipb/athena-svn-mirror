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
#include "save-buddy.h"

#include <gnome.h>
#include <string.h>

#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include <sys/types.h>
#include <sysexits.h>
#include <sys/wait.h>

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
	N_("Select a Product or Application"),
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
		"Kevin Conder  <kevin@kevindumpscore.com>",
		"Eric Baudais  <baudais@kkpsi.org>",
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
	{
		GtkWidget *href;
		href = gnome_href_new ("http://bug-buddy.org/",
				       _("The lame Bug Buddy web page"));
		gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (about)->vbox),
				    href, FALSE, FALSE, 0);
		gtk_widget_show (href);
	}
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
		if (druid_data.download_in_progress)
			druid_set_sensitive (TRUE, FALSE, TRUE);
		break;
	case STATE_COMPONENT:
		break;
	case STATE_MOSTFREQ:
		/* nothing to do */
		break;		
	case STATE_DESC:
		/* nothing to do */
		break;
	case STATE_EMAIL_CONFIG:
		/* FIXME: change next icon */
		on_email_group_toggled (NULL, NULL);
		break;
	case STATE_EMAIL:
		/* fill in the content text */
		s = generate_email_text (druid_data.product != NULL);
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
		if (!druid_data.mostfreq_skipped) {
			newstate = STATE_MOSTFREQ;
			break;
		}
		if (!druid_data.component_skipped && 
		    !GTK_TOGGLE_BUTTON (GET_WIDGET ("no-product-toggle"))->active) {
			newstate = STATE_COMPONENT;
			break;
		}
		if (!druid_data.product_skipped) {
			newstate = STATE_PRODUCT;
			break;
		}
		newstate = STATE_GDB;
		break;
	case STATE_MOSTFREQ:
		if (!druid_data.component_skipped) {
			newstate = STATE_COMPONENT;
			break;
		}
		if (!druid_data.product_skipped) {
			newstate = STATE_PRODUCT;
			break;
		}
		newstate = STATE_GDB;
		break;
	case STATE_COMPONENT:
		if (!druid_data.product_skipped) {
			newstate = STATE_PRODUCT;
			break;
		}
		newstate = STATE_GDB;
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
	char *to=NULL, *s, *file=NULL;
	char *name, *from;
	GtkWidget *w = NULL;
	GString *buf=NULL;
	gboolean retval = FALSE;
	GError *error = NULL;

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
						 RESPONSE_SUBMIT);
		if (RESPONSE_SUBMIT != gtk_dialog_run (GTK_DIALOG (w)))
			goto submit_ok_out;

		gtk_widget_destroy (w);
		w = NULL;
	}

	buf = g_string_new (NULL);

	name = buddy_get_text ("email-name-entry");
	from = buddy_get_text ("email-email-entry");

	g_string_append_printf (buf, "From: %s <%s>\n", name, from);

	g_free (from);
	g_free (name);

	to = buddy_get_text ("email-to-entry");
	g_string_append_printf (buf, "To: %s\n", to);
	
	s = buddy_get_text ("email-cc-entry");
	if (*s) g_string_append_printf (buf, "Cc: %s\n", s);
	g_free (s);

	g_string_append_printf (buf, "X-Mailer: %s %s\n", PACKAGE, VERSION);

	s = buddy_get_text ("email-text");
	g_string_append (buf, s);
	g_free (s);

	if (druid_data.submit_type == SUBMIT_FILE) {
		file = buddy_get_text ("email-file-entry");
		if (!bb_write_buffer_to_file (GTK_WINDOW (GET_WIDGET ("druid-window")),
					      _("Please wait while Bug Buddy saves your bug report..."),
					      file, buf->str, buf->len, &error)) {
			if (error) {
				w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
							    0,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_OK,
							    _("The bug report was not saved in %s:\n\n"
							      "%s\n\n"
							      "Please try again, maybe with a different file name."),
							    file, error->message);
				gtk_dialog_run (GTK_DIALOG (w));
			}
			goto submit_ok_out;
		}

		s = g_strdup_printf (_("Your bug report was saved in %s"), file);
		g_free (file);
	} else {
		char *argv[] = { "", "-i", "-t", NULL };

		argv[0] = buddy_get_text ("email-sendmail-entry");

		if (!bb_write_buffer_to_command (GTK_WINDOW (GET_WIDGET ("druid-window")), 
						 _("Please wait while Bug Buddy submits your bug report..."),
						 argv, buf->str, buf->len, &error)) {
			if (error) {
				w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
							    0,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_OK,
							    _("There was an error submitting the bug report:\n\n"
							      "%s"),
							    error->message);
				gtk_dialog_run (GTK_DIALOG (w));
			}
			goto submit_ok_out;
		}

		s = g_strdup_printf (_("Your bug report has been submitted to:\n\n        <%s>\n\nThanks!"), to);
	}

	buddy_set_text ("finished-label", s);
	g_free (s);
	retval = TRUE;

 submit_ok_out:
	g_free (to);
	if (w)
		gtk_widget_destroy (w);
	if (buf)
		g_string_free (buf, TRUE);
	if (error) 
		g_error_free (error);

	return retval;
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
select_component_row (char *component_name)
{
	GtkTreeView *view;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;

	g_return_if_fail (component_name!=NULL);
	
	view = GTK_TREE_VIEW (GET_WIDGET ("component-list"));
	selection = gtk_tree_view_get_selection (view);
	model = gtk_tree_view_get_model (view);

	if (gtk_tree_model_get_iter_first (model, &iter)) {
		do {
			gchar *tmp = NULL;

			gtk_tree_model_get (model, &iter,
					    COMPONENT_NAME, &tmp,
					    -1);

			if (! strcmp (component_name, tmp)) {
				GtkTreePath *path;

				path = gtk_tree_model_get_path (model, &iter);
				gtk_tree_view_set_cursor (view, path,
							  NULL, FALSE);
				gtk_tree_path_free (path);

				d(g_print("Autoselected %s\n", tmp));
				g_free (tmp);
				break;
			}

			g_free (tmp);
		} while (gtk_tree_model_iter_next (model, &iter));
	}
	
        gtk_tree_view_columns_autosize (view);
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
	{
		BugzillaApplication *application;
		BugzillaProduct *product;
		BugzillaBTS *bts;

		products_list_load ();
		if (!druid_data.current_appname)
			break;
		
		application = g_hash_table_lookup(druid_data.program_to_application, druid_data.current_appname);
		if (!application || !application->bugzilla || !application->product)
			break;

		bts = g_hash_table_lookup (druid_data.bugzillas, application->bugzilla);
		if (!bts)
	       		break;
		
		product = g_hash_table_lookup (bts->products, application->product);
		if (!product)
			break;
		
		druid_data.product = product;
		bugzilla_product_add_components_to_clist (druid_data.product);
		buddy_set_text ("email-to-entry", druid_data.product->bts->email);
		newstate++;
		druid_data.product_skipped = TRUE;
		if (application->component) {
			bugzilla_add_mostfreq (druid_data.product->bts);
			druid_data.component = g_hash_table_lookup(product->components, application->component);
			if (!druid_data.component)
				break;
				
			newstate++;
			druid_data.component_skipped = TRUE;
			if (g_slist_find (druid_data.product->bts->severities, "critical")!=NULL); {
				druid_data.severity = "critical";
			}
			if (!druid_data.product->bts->bugs) {
				newstate++;
				druid_data.mostfreq_skipped = TRUE;
			}
		}
		break;
	}
	case STATE_PRODUCT:
	{
		BugzillaProduct *product = NULL;
		BugzillaApplication *application;
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
			druid_data.component_skipped = TRUE;
			druid_data.mostfreq_skipped = TRUE;
			break;
		}
 
		if (druid_data.show_products) {
			product = get_selected_row ("product-list", PRODUCT_DATA);
 			if (!product) {
				d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
							    0,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_OK,
							    _("Please choose a product for your bug report."));
				gtk_dialog_set_default_response (GTK_DIALOG (d),
								 GTK_RESPONSE_OK);
				gtk_dialog_run (GTK_DIALOG (d));
				gtk_widget_destroy (d);
				return;
			}
		} else {
			application = get_selected_row ("product-list", PRODUCT_DATA);
 			if (!application) {
				d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
							    0,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_OK,
							    _("Please choose an application for your bug report."));
				gtk_dialog_set_default_response (GTK_DIALOG (d),
								 GTK_RESPONSE_OK);
				gtk_dialog_run (GTK_DIALOG (d));
				gtk_widget_destroy (d);
				return;
			}
			if (application->bugzilla && application->product) {
				BugzillaBTS *bts = g_hash_table_lookup (druid_data.bugzillas, application->bugzilla);
				if (bts) {
					product = g_hash_table_lookup (bts->products, application->product);
				}
			}
			if (!product) {
				if (!application->email) {
					d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
								    0,
								    GTK_MESSAGE_WARNING,
								    GTK_BUTTONS_OK,
								    _("This application has not included information about "
								      "how to submit bugs.\n\n"
								      "If you know an email address where bug reports should be sent, "
								      "you will be able to specify that later.\n\n"
								      "You may be able to find one in an \"About\" box in the "
								      "application, or in the application's documentation."));
					gtk_dialog_set_default_response (GTK_DIALOG (d), GTK_RESPONSE_OK);
					gtk_dialog_run (GTK_DIALOG (d));
					gtk_widget_destroy (d);
				}
				buddy_set_text ("email-to-entry", application->email);
				druid_data.product = NULL;
				druid_data.component = NULL;
				newstate = STATE_DESC;
				druid_data.component_skipped = TRUE;
				druid_data.mostfreq_skipped = TRUE;
				break;
			}
				
		}

		if (product != druid_data.product) {
			druid_data.product = product;
			bugzilla_product_add_components_to_clist (druid_data.product);
			buddy_set_text ("email-to-entry", druid_data.product->bts->email);
		}
		
		if (application->component) {
			select_component_row (application->component);
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
		if (!druid_data.product->bts->bugs) {
			newstate++;
			druid_data.mostfreq_skipped = TRUE;
		}
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

	stop_gdb ();
	save_config ();
	gtk_main_quit ();
}
