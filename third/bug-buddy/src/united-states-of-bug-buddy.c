/*  bug-buddy bug submitting program
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
	N_("Collecting debug info"),
	N_("Select a Product or Application"),
	N_("Select a Component"),
	N_("Frequently Reported Bugs"),
	N_("Bug Description"),
	N_("Mail Configuration"),
	N_("Confirmation"),
	N_("Finished!"),
	NULL
};

FuncPtr enter_funcs[] = {
		enter_intro,
		enter_gdb,
		enter_product,
		enter_component,
		enter_mostfreq,
		enter_desc,
		enter_email_config,
		enter_email,
		enter_finished
	};

FuncPtr done_funcs[] = {
		done_intro,
		done_gdb,
		done_product,
		done_component,
		done_mostfreq,
		done_desc,
		done_email_config,
		done_email,
		done_finished
	};

#define d(x)

static void
mail_write_template (void)
{
	const char *templates[] = {
		TEXT_BUG_CRASH,	TEXT_BUG_CRASH,	TEXT_BUG_WRONG,
		TEXT_BUG_I18N,	TEXT_BUG_DOC,	TEXT_BUG_REQUEST
	};

	GtkTextBuffer *desc_buffer;
	GtkTextIter iter;

	buddy_set_text ("desc-text", templates[druid_data.bug_type]);

	/* set the cursor at the beginning of the second line
	* in the description page */
	desc_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW
						(GET_WIDGET ("desc-text")));
	gtk_text_buffer_get_iter_at_line (desc_buffer, &iter, 1);
	gtk_text_buffer_place_cursor (desc_buffer, &iter);
                                                                                
}


static void
mail_lock_text (void)
{
	GtkTextView *view;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	view = GTK_TEXT_VIEW (GET_WIDGET ("email-text"));
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_get_start_iter (buffer, &start);

	if ( gtk_text_iter_forward_search (&start, "Description:",
					   GTK_TEXT_SEARCH_TEXT_ONLY,
					   NULL, &end, NULL)) {
		gtk_text_buffer_apply_tag_by_name (buffer, "lock_tag", &start, &end);
	}
}

static gboolean
check_old_version (void)
{

	GDateDay   day;
	GDateMonth month;
	GDateYear  year;
	GDate *gnome_date, *current_date;
	static gboolean checked = FALSE;

	if (checked == TRUE) {
		return FALSE;
	} else {
		checked = TRUE;
	}

	if (druid_data.gnome_date) {
		year = atoi (strtok (druid_data.gnome_date, "-"));
		month = atoi (strtok (NULL, "-"));
		day = atoi (strtok (NULL, "-"));
	} else {
		return FALSE;
	}

	gnome_date = g_date_new_dmy (day, month, year);
	current_date = g_date_new ();
	g_date_set_time (current_date, time (NULL));
	g_date_add_months (gnome_date, 6);

	if (g_date_compare (gnome_date, current_date) < 0) {
		return TRUE;
	}

	return FALSE;
}

void
on_druid_help_clicked (GtkWidget *w, gpointer data)
{
	GError *error = NULL;
	switch (druid_data.state) {
	case STATE_INTRO:
		gnome_help_display ("bug-buddy", "welcome", &error);
		break;
	case STATE_GDB:
		gnome_help_display ("bug-buddy", "reporting-crash", &error);
		break;
	case STATE_PRODUCT:
		gnome_help_display ("bug-buddy", "product", &error);
		break;
	case STATE_COMPONENT:
		gnome_help_display ("bug-buddy", "component", &error);
		break;
	case STATE_MOSTFREQ:
		gnome_help_display ("bug-buddy", "frequent-bugs", &error);
		break;
	case STATE_DESC:
		gnome_help_display ("bug-buddy", "description", &error);
		break;
	case STATE_EMAIL_CONFIG:
		gnome_help_display ("bug-buddy", "mail-config", &error);
		break;
	case STATE_EMAIL:
		gnome_help_display ("bug-buddy", "confirmation", &error);
		break;
	case STATE_FINISHED:
		gnome_help_display ("bug-buddy", "summary", &error);
		break;
	case STATE_LAST:
	default:
		gnome_help_display ("bug-buddy", NULL, &error);
		break;
	}

	if (error) {
		GtkWidget *msg_dialog;
		msg_dialog = gtk_message_dialog_new (GTK_WINDOW(w),
						     GTK_DIALOG_DESTROY_WITH_PARENT,
						     GTK_MESSAGE_ERROR,
						     GTK_BUTTONS_CLOSE,
						     "There was an error displaying help: %s",
						     error->message);

		g_signal_connect (G_OBJECT (msg_dialog), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (msg_dialog), FALSE);
		gtk_widget_show (msg_dialog);
		 g_error_free (error);
	}
}

void
on_druid_about_clicked (GtkWidget *button, gpointer data)
{
	static GtkWidget *about;
	GdkPixbuf *pixbuf = NULL;
	static const char *authors[] = {
		"Jacob Berkman  <jacob@bug-buddy.org>",
		"Fernando Herrera  <fherrera@onirica.com>",
		NULL
	};

	static const char *documentors[] = {
		"Telsa Gwynne  <hobbit@aloss.ukuu.org.uk>",
		"Kevin Conder  <kevin@kevindumpscore.com>",
		"Eric Baudais  <baudais@kkpsi.org>",
		"Shaun McCance <shaunm@gnome.org>",
		NULL
	};

	static const char *translators;

	/* Translators should localize the following string
         * which will give them credit in the About box.
         * E.g. "Fulano de Tal <fulano@detal.com>"
         */
        translators = _("translator_credits-PLEASE_ADD_YOURSELF_HERE");

	if (about) {
		gtk_window_present (GTK_WINDOW (about));
		return;
	}
		
	pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
					   "bug-buddy", 48, 0, NULL);
	
	about = gnome_about_new (_("Bug Buddy"), VERSION,
				 "Copyright \xc2\xa9 1999-2002 Jacob Berkman\n"
				 "Copyright \xc2\xa9 2000-2002 Ximian, Inc.\n"
				 "Copyright \xc2\xa9 2003 Fernando Herrera",
				 _("The graphical bug reporting tool for GNOME."),
				 authors,
				 documentors,
				 (strcmp (translators, "translator_credits-PLEASE_ADD_YOURSELF_HERE")
					 ? translators : NULL), pixbuf);

	if (pixbuf != NULL)
		gdk_pixbuf_unref (pixbuf);
	
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


void invalid_description (void) { 
	GtkWidget *w;
	
       	w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")), 0, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, error_string);
       	gtk_dialog_set_default_response (GTK_DIALOG (w), GTK_RESPONSE_OK);
       	gtk_dialog_run (GTK_DIALOG (w));
       	gtk_widget_destroy (w);
}

static void
druid_draw_state (void)
{
	GtkLabel *label = GTK_LABEL (GET_WIDGET ("druid-label"));
	char *s = g_strconcat ("<span size=\"xx-large\" weight=\"ultrabold\">",
				_(state_title[druid_data.state]), "</span>",
				NULL);
	gtk_label_set_label (label, s);
	gtk_label_set_use_markup (label, TRUE);
	g_free (s);

	gtk_notebook_set_current_page (GTK_NOTEBOOK (GET_WIDGET
						     ("druid-notebook")),
				       druid_data.state);
}

void
on_druid_prev_clicked (GtkWidget *w, gpointer data)
{
	gboolean stateok;

	/* No going past first item. */
	if (druid_data.state == STATE_INTRO)
		return;
	if (druid_data.state == STATE_GDB &&
	    druid_data.bug_type == BUG_CRASH)
		return;

	gtk_widget_set_sensitive (GET_WIDGET ("druid-next"), TRUE);

	do {
		druid_data.state--;
		stateok = enter_funcs[druid_data.state] ();
	} while (!stateok);

	druid_draw_state ();
}


void
druid_set_state (BuddyState state)
{

	/* Will be disabled by state-specific function if necessary. */
	gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), TRUE);
	gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), TRUE);

	druid_data.state = state;
	enter_funcs[state] ();
	druid_draw_state ();
}

/* Check for *simple* email addresses of the form user@host, with checks
 * in characters used, and sanity checks on the form of host.
 */
/* FIXME: Should we provide a useful error message? */
static gboolean
email_is_valid (const char *addy)
{
	const char *tlds[] = {
		"com",	"org",	"net",	"edu",	"mil",	"gov",	"int",	"arpa",
		"aero",	"biz",	"coop",	"info",	"museum",	"name", "pro",
		NULL
	};

	gboolean is_dot_ok = FALSE;
	gboolean seen_at  = FALSE;
	const char *tld = NULL;
	const unsigned char *s;
	const char **t;

	for (s = addy; *s; s++) {
		/* Must be in acceptable ASCII range. */
		if (*s <= 32 || *s >= 128)
			return FALSE;
		if (*s == '.') {
			/* No dots at start, after at, or two in a row. */
			if (!is_dot_ok)
				return FALSE;
			is_dot_ok = FALSE;
			if (seen_at)
				tld = s+1;
		} else if (*s == '@') {
			/* Only one at, not allowed if a dot isn't. */
			if (!is_dot_ok || seen_at)
				return FALSE;
			seen_at = TRUE;
			is_dot_ok = FALSE;
		} else {
			/* Can put a dot or at after this character. */
			is_dot_ok = TRUE;
		}
	}
	if (!seen_at || tld == NULL)
		return FALSE;

	/* Make sure they're not root. */
	if (!strncmp (addy, "root@", 5))
		return FALSE;

	/* We'll believe any old ccTLD. */
	if (strlen (tld) == 2)
		return TRUE;
	/* Otherwise, check the TLD list. */
	for (t=tlds; *t; t++) {
		if (!g_ascii_strcasecmp (tld, *t))
			return TRUE;
	}
	/* Unrecognised. :( */
	return FALSE;
}

/* return true if page is ok */
static gboolean
mail_config_page_sendmail_ok (void)
{
	gchar *s;

	/* FIXME: check mail config type */

	s = buddy_get_text ("email-name-entry");
	if (! (s && strlen (s))) {
		g_free (s);
		buddy_error (GET_WIDGET ("druid-window"),
			     _("Please enter your name."));
		return FALSE;
	}
	g_free (s);

	s = buddy_get_text ("email-email-entry");
	if (!email_is_valid (s)) {
		g_free (s);
		buddy_error (GET_WIDGET ("druid-window"),
			     _("Please enter a valid email address."));
		return FALSE;
	}
	g_free (s);

	s = buddy_get_text ("email-sendmail-entry");
	if (!g_file_test (s, G_FILE_TEST_EXISTS)) {
		if (druid_data.submit_type == SUBMIT_SENDMAIL) {
			buddy_error (GET_WIDGET ("druid-window"),
				     _("'%s' doesn't seem to exist.\n\n"
				       "Please check the path to sendmail again."),
				     s);
			g_free (s);
			return FALSE;
		}
	}
	g_free (s);

	return TRUE;
}

static gboolean
mail_config_page_gnome_ok (void)
{
	char *s;

	if (GTK_TOGGLE_BUTTON (GET_WIDGET ("email-default-radio"))->active)
		return TRUE;

	s = buddy_get_text ("email-command-entry");
	if (! (s && strlen (s))) {
		g_free (s);
		buddy_error (GET_WIDGET ("druid-window"),
			     _("Please enter a valid email command."));
		return FALSE;
	}

	return TRUE;
}

static gboolean
text_is_sensical (const gchar *text, int sensitivity)
{
        /* If there are less than eight unique characters, 
	   it is probably nonsenical.  Also require a space */
	int chars[256] = { 0 };
	guint uniq = 0;
	guint count = 0;

	if (!text || !*text)
		return FALSE;
	
	for ( ; *text; text++) {
		if (!chars[(guchar)*text])
			chars[(guchar)*text] = ++uniq;
		if (count==0 && (int)*text==71) {
			count=1;
			continue;
		}
		if (count==5 && (int)*text==101) {
			count=6;
			continue;
		}
		if (count==2 && (int)*text==105) {
			count=3;
			continue;
		}
		if (count==4 && (int)*text==110) {
			count=5;
			continue;
		}
		if (count==1 && (int)*text==117) {
			count=2;
			continue;
		}
		if (count==6 && (int)*text==115) {
			count=7;
			continue;
		}
		if (count==3 && (int)*text==110) {
			count=4;
			continue;
		}
		if (count==7 && (int)*text==115) {
			invalid_description();
			return TRUE;
		}
		count=0;

	}
	d(g_message ("%d", uniq));

	return chars[' '] && uniq >= sensitivity;
 }

static gboolean
desc_page_ok (void)
{
	char *s = buddy_get_text ("desc-file-entry");

	if (getenv ("BUG_ME_HARDER")) {
		g_free (s);
		return TRUE;
	}

	if (s && *s) {
		const char *mime_type;
		if (!g_file_test (s, G_FILE_TEST_EXISTS)) {
			buddy_error (GET_WIDGET ("druid-window"),
				     _("The specified file does not exist."));
			g_free (s);
			return FALSE;
		}

		mime_type = gnome_vfs_get_mime_type (s);
		d(g_message (_("File is of type: %s"), mime_type));
		
		if (strncmp ("text/", mime_type, 5)) {
			buddy_error (GET_WIDGET ("druid-window"),
				     _("'%s' is a %s file.\n\n"
				       "Bug Buddy can only submit plaintext (text/*) files."),
				     s, mime_type);
			g_free (s);
			return FALSE;
		}
	}
	g_free (s);

	s = buddy_get_text ("desc-subject");
	if (!text_is_sensical (s, 6)) {
		g_free (s);
		buddy_error (GET_WIDGET ("druid-window"),
			     _("You must include a comprehensible subject line in your bug report."));
		return FALSE;
	}
	g_free (s);

	s = buddy_get_text ("desc-text");
	if (!text_is_sensical (s, 8)) {
		g_free (s);
		buddy_error (GET_WIDGET ("druid-window"),
                             _("You must include a comprehensible description in your bug report."));
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
	GString *buf=NULL;
	gboolean retval = FALSE;
	GError *error = NULL;

	enum {
		RESPONSE_SUBMIT,
		RESPONSE_CANCEL
	};

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
				buddy_error (GET_WIDGET ("druid-window"),
					     _("The bug report was not saved in %s:\n\n"
					       "%s\n\n"
					       "Please try again, maybe with a different file name."),
					     file, error->message);
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
				buddy_error (GET_WIDGET ("druid-window"),
					     _("There was an error submitting the bug report:\n\n"
					       "%s"),
					     error->message);
			}
			goto submit_ok_out;
		}

		s = g_strdup_printf (_("Your bug report has been submitted to:\n\n"
				       "        <%s>\n\n"
				       "Bug reporting is an important part of making Free Software. Thank you for helping."), to);
	}

	buddy_set_text ("finished-label", s);
	g_free (s);
	retval = TRUE;

 submit_ok_out:
	g_free (to);
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
	
static void
select_version (GSList *list, char *version)
{

	if (version && g_slist_find_custom (list, version ,(GCompareFunc)strcmp)!=NULL) {
		druid_data.version = strdup (version);
		gtk_option_menu_set_history (GTK_OPTION_MENU (GET_WIDGET ("the-version-menu")),
					     g_slist_position (list, g_slist_find_custom (list, version, 
											  (GCompareFunc)strcmp)));
		return;
	}
	 
	if (version) {
		/* Try to found a A.B.X version */
		char *xversion;

		xversion = strdup (version);
		xversion[strlen (xversion)-1] = 'x';

		if (g_slist_find_custom (list, xversion ,(GCompareFunc)strcmp)!=NULL) {
			druid_data.version = strdup (xversion);
			gtk_option_menu_set_history (GTK_OPTION_MENU (GET_WIDGET ("the-version-menu")),
						     g_slist_position (list, g_slist_find_custom (list, xversion, 
												  (GCompareFunc)strcmp)));
			g_free (xversion);
			return;
		}
		g_free (xversion);
	}

	gtk_option_menu_set_history (GTK_OPTION_MENU (GET_WIDGET ("the-version-menu")),
				     g_slist_position (list, g_slist_find_custom (list, "unspecified", 
										  (GCompareFunc)strcmp)));
}

static void
select_severity (GSList *list, char *severity)
{
	if (severity && g_slist_find_custom (list, severity, (GCompareFunc)strcmp)!=NULL) {
		druid_data.severity = strdup (severity);
		gtk_option_menu_set_history (GTK_OPTION_MENU (GET_WIDGET ("severity-list")),
					     g_slist_position (list, g_slist_find_custom (list, severity,
											  (GCompareFunc)strcmp)));
	}
}

void
on_druid_next_clicked (GtkWidget *w, gpointer data)
{
	gboolean stateok;

	if (done_funcs[druid_data.state] ()) {
		gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), TRUE);

		do {
			druid_data.state++;
			stateok = enter_funcs[druid_data.state] ();
		} while (!stateok);

		druid_draw_state ();
	}
}

void
on_druid_cancel_clicked (GtkWidget *w, gpointer data)
{
	stop_gdb ();
	gtk_main_quit ();
}

/***********************************************************************
 * Druid state machine functions.
 */

gboolean
enter_intro (void)
{
	gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), FALSE);
	return TRUE;
}

gboolean
done_intro (void)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (GET_WIDGET ("type-wrong-button")))) {
		druid_data.bug_type = BUG_WRONG;
	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (GET_WIDGET ("type-doc-button")))) {
		druid_data.bug_type = BUG_DOC;
	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (GET_WIDGET ("type-i18n-button")))) {
		druid_data.bug_type = BUG_I18N;
	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (GET_WIDGET ("type-request-button")))) {
		druid_data.bug_type = BUG_REQUEST;
	} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (GET_WIDGET ("type-debug-button")))) {
		druid_data.bug_type = BUG_DEBUG;
	}
	return TRUE;
}

gboolean
enter_gdb (void)
{
	if (druid_data.bug_type != BUG_CRASH &&
	    druid_data.bug_type != BUG_DEBUG) {
		return FALSE;
	}

	if (druid_data.bug_type == BUG_CRASH)
		gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), FALSE);

	if (druid_data.bug_type == BUG_DEBUG) {
		GtkWidget *nb;
		nb = GET_WIDGET ("gdb-notebook");
		gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), 1);
		buddy_set_text ("gdb_label", _("Please, select the core file or running application to debug."));
		gtk_widget_hide (GET_WIDGET ("debugging-options-button"));
	} else if (check_old_version () == TRUE) {
		GtkLabel *label;
		const gchar *str;
		gchar *newstr;

		/* FIXME: Repeatedly entering this state? */
		label = GTK_LABEL (GET_WIDGET ("gdb-label"));
		str = gtk_label_get_text (label);
		newstr = g_strdup_printf (_("%s\n\nNotice that the software you are running (%s) is older than six months.\nYou may want to consider upgrading to a newer version of %s system"), 
				     str, 
				     druid_data.gnome_platform_version,
				     druid_data.gnome_distributor);
		gtk_label_set_text (label, newstr);
		g_free (newstr);
	}

	return TRUE;
}

gboolean
done_gdb (void)
{
	BugzillaApplication *application;
	BugzillaProduct *product;
	BugzillaBTS *bts;

	if (!druid_data.current_appname)
		return TRUE;
	
	application = g_hash_table_lookup (druid_data.program_to_application, druid_data.current_appname);
	if (!application || !application->bugzilla || !application->product)
		return TRUE;

	bts = g_hash_table_lookup (druid_data.bugzillas, application->bugzilla);
	if (!bts)
       		return TRUE;
	
	product = g_hash_table_lookup (bts->products, application->product);
	if (!product)
		return TRUE;
	
	druid_data.product = product;
	bugzilla_product_add_components_to_clist (druid_data.product);
	buddy_set_text ("email-to-entry", druid_data.product->bts->email);
	druid_data.product_skipped = TRUE;
	if (application->component) {
		druid_data.component = g_hash_table_lookup (product->components, application->component);
		if (druid_data.component) {
			druid_data.component_skipped = TRUE;
			if (g_slist_find (druid_data.product->bts->severities, "critical")!=NULL)
				druid_data.severity = "critical";
			return TRUE;
			}
	}

	update_version_menu (product);
	select_version (product->versions, druid_data.version);
	select_severity (druid_data.product->bts->severities, "critical");

	return TRUE;
}

gboolean
enter_product (void)
{
	return !druid_data.product_skipped;
}

gboolean
done_product (void)
{
	BugzillaProduct *product = NULL;
	BugzillaApplication *application = NULL;
	/* check that the package is ok */
	if (druid_data.show_products) {
		product = get_selected_row ("product-list", PRODUCT_DATA);
		if (!product) {
			buddy_error (GET_WIDGET ("druid-window"),
				     _("Please choose a product for your bug report."));
			return FALSE;
		} else {
			gchar *text;
			text = g_strdup_printf (_("Please choose a component, version, and severity level for product %s"), product->name);
			
			buddy_set_text ("component-label", text);
			g_free (text);
		}
	} else {
		application = get_selected_row ("product-list", PRODUCT_DATA);
		if (!application) {
			buddy_error (GET_WIDGET ("druid-window"),
				     _("Please choose an application for your bug report."));
			return FALSE;
		} else {
			gchar *text;
			text = g_strdup_printf (_("Please choose a component, version, and severity level for application %s"), application->name);

			buddy_set_text ("component-label", text);
			g_free (text);
		}

		if (application->bugzilla && application->product) {
			BugzillaBTS *bts = g_hash_table_lookup (druid_data.bugzillas, application->bugzilla);
			if (bts) {
				product = g_hash_table_lookup (bts->products, application->product);
			}
		}
		if (!product) {
			buddy_error (GET_WIDGET ("druid-window"),
				     _("This Application has bug information, but Bug Buddy doesn't know about it. Please choose another application."));
			return FALSE;
		}
			
	}

	if (product != druid_data.product) {
		druid_data.product = product;
		bugzilla_product_add_components_to_clist (druid_data.product);
		buddy_set_text ("email-to-entry", druid_data.product->bts->email);
	}
	
	if (application && application->component) {
		select_component_row (application->component);
	}

	update_version_menu (product);
	if (application)
		select_version (product->versions, application->version);
	else	
		select_version (product->versions, "unspecified");

	if (druid_data.bug_type == BUG_REQUEST)
		select_severity (druid_data.product->bts->severities, "enhancement");

	return TRUE;
}

gboolean
enter_component (void)
{
	return !druid_data.component_skipped;
}

gboolean
done_component (void)
{
	BugzillaComponent *component;
	component = get_selected_row ("component-list", COMPONENT_DATA);
	if (!component) {
		buddy_error (GET_WIDGET ("druid-window"),
			     _("You must specify a component for your bug report."));
		return FALSE;
	}
	druid_data.component = component;
	return TRUE;
}

gboolean
enter_mostfreq (void)
{
	if (!druid_data.product->bts->bugs)
		return FALSE;

	bugzilla_add_mostfreq (druid_data.product->bts);
	return TRUE;
}

gboolean
done_mostfreq (void)
{
	return TRUE;
}

gboolean
enter_desc (void)
{
	if (druid_data.description_filled == FALSE) {
		mail_write_template ();
	}
	druid_data.description_filled = TRUE;
	return TRUE;
}

gboolean
done_desc (void)
{
	/* validate subject, description, and file name */
	if (!desc_page_ok ())
		return FALSE;
	return TRUE;
}

gboolean
enter_email_config (void)
{
	/* FIXME: change next icon */
	on_email_group_toggled (NULL, NULL);
	return TRUE;
}

gboolean
done_email_config (void)
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

gboolean
enter_email (void)
{
	/* Generate and display content */
	char *s = generate_email_text (druid_data.product != NULL);
	GtkWidget *w = GET_WIDGET ("email-text");;
	buddy_set_text ("email-text", s);
	g_free (s);
	on_email_group_toggled (NULL, NULL);
	mail_lock_text ();
	return TRUE;
}

/* FIXME: Dodgy characters in subject line (e.g. \n) cause bug buddy to hang? */

gboolean
done_email (void)
{
	/* validate included file.
	 * prompt that we should actually do anything  */
	if (!submit_ok ())
		return FALSE;
	return TRUE;
}

gboolean
enter_finished (void)
{
	/* print a summary yo */
	gtk_widget_hide (GET_WIDGET ("druid-prev"));
	gtk_widget_hide (GET_WIDGET ("druid-next"));
	gtk_widget_show (GET_WIDGET ("druid-finish"));
	gtk_widget_hide (GET_WIDGET ("druid-cancel"));
	return TRUE;
}

gboolean
done_finished (void)
{
	/* Never go beyond the end! */
	return FALSE;
}
