/* bug-buddy bug submitting program
 *
 * Copyright (C) Jacob Berkman
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

#include <config.h>

#include "bug-buddy.h"

#include <gnome.h>

#include <stdio.h>

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#if 0
#include <libart_lgpl/libart.h>
#endif

#define d(x)

static guint throbber_id = 0;

static void get_trace_from_core (const gchar *core_file);
static void get_trace_from_pair (const gchar *app, const gchar *extra);

static gint
animate (gpointer data)
{
#if 0
	double affine[6];

	art_affine_rotate (affine, -36);
	gnome_canvas_item_affine_relative (druid_data.throbber, affine);
#endif

	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (GET_WIDGET ("gdb-progress")));
	return TRUE;
}

static void
start_animation (void)
{
	g_return_if_fail (throbber_id == 0);

	throbber_id = gtk_timeout_add (150, animate, NULL);
	gtk_widget_show (GET_WIDGET ("gdb-progress"));
}

static void
stop_animation (void)
{
	g_return_if_fail (throbber_id != 0);

	gtk_timeout_remove (throbber_id);
	throbber_id = 0;
	gtk_widget_hide (GET_WIDGET ("gdb-progress"));
}

void
start_gdb (void)
{
	gchar *app, *extra;

	d(g_message (_("Obtaining stack trace... (%d)"), druid_data.crash_type));

	switch (druid_data.crash_type) {
	case CRASH_NONE:
		return;
	case CRASH_DIALOG:
		app = buddy_get_text ("gdb-binary-entry");
		extra = buddy_get_text ("gdb-pid-entry");
		druid_data.app_pid = atoi (extra);
		kill (druid_data.app_pid, SIGCONT);
		get_trace_from_pair (app, extra);
		g_free (app);
		g_free (extra);
		break;
	case CRASH_CORE:
		extra = buddy_get_text ("gdb-core-entry");
		get_trace_from_core (extra);
		g_free (extra);
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

void
stop_gdb (void)
{
	if (!druid_data.ioc) {
		d(g_message (_("gdb has already exited")));
		return;
	}
	
	g_io_channel_shutdown (druid_data.ioc, 1, NULL);
	
	kill (druid_data.gdb_pid, SIGTERM);
	/* i don't think we need to SIGKILL it */
	/*kill (druid_data.gdb_pid, SIGKILL);*/
	waitpid (druid_data.gdb_pid, NULL, 0);
	
	druid_data.gdb_pid = 0;

	/* sometimes gdb doesn't restart the old app... */
	if (druid_data.app_pid) {
		kill (druid_data.app_pid, SIGCONT);
		druid_data.app_pid = 0;
	}

	if (druid_data.bug_type != BUG_CRASH)
		gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), TRUE);
	gtk_widget_set_sensitive (GET_WIDGET ("druid-next"), TRUE);
	gtk_widget_set_sensitive (GET_WIDGET ("gdb-copy-save-box"), TRUE);
	stop_animation ();

	druid_data.fd = 0;
	druid_data.ioc = NULL;
	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-stop")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-go")), TRUE);
}

static void 
get_trace_from_core (const gchar *core_file)
{
	gchar *gdb_cmd;
	gchar buf[1024];
	gchar *binary = NULL;
	int status;
	FILE *f;

	gdb_cmd = g_strdup_printf ("gdb --batch --core=%s", core_file);

	f = popen (gdb_cmd, "r");
	g_free (gdb_cmd);

	if (!f) {
		buddy_error (GET_WIDGET ("druid-window"),
			     _("Unable to process core file with gdb:\n"
			       "'%s'"),
			     core_file);
		return;
	}

	while (fgets(buf, 1024, f) != NULL) {
		if (!binary && !strncmp(buf, "Core was generated", 16)) {
			gchar *s;
			gchar *ptr = buf;
			while (*ptr != '`' && *ptr !='\0') ptr++;
			if (*ptr == '`') {
				ptr++;
				s = ptr;
				while (*ptr != '\'' && *ptr !=' ' && *ptr !='\0') ptr++;
				*ptr = '\0';
				binary = g_strdup(s);
			}
		}
	}

	status = pclose(f);

	if (!binary) {
		buddy_error (GET_WIDGET ("druid-window"),
			     _("GDB was unable to determine which binary created\n"
			       "'%s'"),
			     core_file);
		return;
	}	

	if (!popt_data.app_file) {
		d(g_message ("Setting binary: %s", binary));
		popt_data.app_file = g_strdup (binary);
	}

	get_trace_from_pair (binary, core_file);
	g_free (binary);
}

static gboolean
handle_gdb_input (GIOChannel *ioc, GIOCondition condition, gpointer data)
{	
	gboolean retval = FALSE;
	gchar buf[1024];
	gsize len;
	GIOStatus io_status;

 gdb_try_read:
	io_status = g_io_channel_read_chars (ioc, buf, 1024, &len, NULL);

	switch (io_status) {
	case G_IO_STATUS_AGAIN:
		goto gdb_try_read;
	case G_IO_STATUS_ERROR:
		d(g_warning (_("Error on read... aborting")));
		break;
	case G_IO_STATUS_NORMAL:
		retval = TRUE;
		break;
	default:
		break;
	}

	if (len > 0) {
		GtkTextIter end;
		GtkTextBuffer *buffy;
		GtkTextView *tv;
		char *utftext;
		gsize localelen;
		gsize utflen;

		tv = GTK_TEXT_VIEW (GET_WIDGET ("gdb-text"));
		buffy = gtk_text_view_get_buffer (tv);

		gtk_text_buffer_get_end_iter (buffy, &end);
		/* gdb charset is ISO-8859-1 */
		utftext = g_convert_with_fallback (buf, len, "UTF-8", "ISO-8859-1", NULL, &localelen, &utflen, NULL);
		gtk_text_buffer_insert (buffy, &end, utftext, utflen);
		g_free (utftext);
	}

	if (!retval)
		stop_gdb ();

	return retval;
}

static void
get_trace_from_pair (const gchar *app, const gchar *extra)
{
	char *s;
	const char *short_app;
	char *long_app;
	GError *error = NULL;
	char *args[] = { "gdb",
			 "--batch", 
			 "--quiet",
			 "--command=" BUDDY_DATADIR "/gdb-cmd",
			 NULL, NULL, NULL };

	if (!app || !extra || !*app || !*extra) {
		buddy_error (GET_WIDGET ("druid-window"), 
			     _("Both a binary file and PID are required to debug."));
		return;
	}

	if (app[0] == G_DIR_SEPARATOR) {
		long_app = g_strdup (app);
		short_app = strrchr (app, G_DIR_SEPARATOR) + 1;
	} else {
		long_app = g_find_program_in_path (app);
		if (!long_app && g_getenv("GNOME2_PATH")) {
			/* Applets are not in path... */
			long_app = g_strconcat(g_getenv("GNOME2_PATH"), "/libexec/", app, NULL);
		}
		short_app = app;
	}	

	g_free (druid_data.current_appname);
	druid_data.current_appname = g_strdup (short_app);

	if (!long_app) {
		buddy_error (GET_WIDGET ("druid-window"),
			     _("The binary file could not be found. Try using an absolute path."));
		return;
	}

	args[0] = g_find_program_in_path ("gdb");
	args[4] = long_app;
	args[5] = (char *)extra;

	if (!args[0]) {
		buddy_error (GET_WIDGET ("druid-window"),
			     _("GDB could not be found on your system.\n"
			       "Debugging information will not be obtained."));
		d(g_message ("Path: %s", getenv ("PATH")));
	} else {
		d(g_message ("About to debug '%s'", long_app));
	
		if (!g_file_test (BUDDY_DATADIR "/gdb-cmd", G_FILE_TEST_EXISTS)) {
			buddy_error (GET_WIDGET ("druid-window"),
				     _("Could not find the gdb-cmd file.\n"
				       "Please try reinstalling Bug Buddy."));
		} else if (!g_spawn_async_with_pipes (NULL, args, NULL, 0, NULL, NULL,
					       &druid_data.gdb_pid,
					       NULL, 
					       &druid_data.fd, 
					       NULL, &error)) {
			buddy_error (GTK_WIDGET ("druid-window"),
				     _("There was an error running gdb:\n\n%s"),
				     error->message);
			g_error_free (error);
		} else {
			druid_data.ioc = g_io_channel_unix_new (druid_data.fd);
			g_io_channel_set_encoding (druid_data.ioc, NULL, NULL);
		
			s = g_strdup_printf ("Backtrace was generated from '%s'\n\n", long_app);
			buddy_set_text ("gdb-text", s);
			g_free (s);

			g_io_add_watch (druid_data.ioc, G_IO_IN | G_IO_HUP,
					handle_gdb_input, NULL);
			g_io_channel_unref (druid_data.ioc);

			gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), FALSE);
			gtk_widget_set_sensitive (GET_WIDGET ("druid-next"), FALSE);
			gtk_widget_set_sensitive (GET_WIDGET ("gdb-copy-save-box"), FALSE);

			start_animation ();

			gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-stop")), TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-go")), FALSE);
		}
		
		g_free (args[0]);
	}
	g_free (long_app);
}
