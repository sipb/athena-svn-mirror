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

#include <libart_lgpl/libart.h>

#define d(x)

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
	g_return_if_fail (druid_data.throbber_id == 0);

	druid_data.throbber_id = gtk_timeout_add (150, animate, NULL);
	gtk_widget_show (GET_WIDGET ("gdb-progress"));
	gtk_widget_set_sensitive (GET_WIDGET ("gdb-copy-save-box"), FALSE);
}

static void
stop_animation (void)
{
	g_return_if_fail (druid_data.throbber_id != 0);

	gtk_timeout_remove (druid_data.throbber_id);
	druid_data.throbber_id = 0;
	gtk_widget_hide (GET_WIDGET ("gdb-progress"));
	gtk_widget_set_sensitive (GET_WIDGET ("gdb-copy-save-box"), TRUE);
}

void
start_gdb (void)
{
	static gchar *old_app = NULL;
	static gchar *old_extra = NULL;
	static CrashType old_type = -1;
	gchar *app = NULL, *extra = NULL;

	d(g_message (_("Obtaining stack trace... (%d)"), druid_data.crash_type));

	switch (druid_data.crash_type) {
	case CRASH_NONE:
		return;
	case CRASH_DIALOG:
		app = buddy_get_text ("gdb-binary-entry");
		extra = buddy_get_text ("gdb-pid-entry");
		druid_data.app_pid = atoi (extra);
		kill (druid_data.app_pid, SIGCONT);
		if (druid_data.explicit_dirty ||
		    (old_type != CRASH_DIALOG) ||
		    (!old_app || strcmp (app, old_app)) ||
		    (!old_extra || strcmp (extra, old_extra))) {
			get_trace_from_pair (app, extra);
		}
		break;
	case CRASH_CORE:
		extra = buddy_get_text ("gdb-core-entry");
		if (druid_data.explicit_dirty ||
		    (old_type != CRASH_CORE) ||
		    (!old_extra || strcmp (extra, old_extra))) {
			get_trace_from_core (extra);
		}
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	g_free (old_extra);
	old_extra = extra;

	g_free (old_app);
	old_app = app;

	old_type = druid_data.crash_type;
}

void
stop_gdb (void)
{
	if (!druid_data.ioc) {
		d(g_message (_("gdb has already exited")));
		return;
	}
	
	g_io_channel_shutdown (druid_data.ioc, 1, NULL);
	waitpid (druid_data.gdb_pid, NULL, 0);
	
	druid_data.gdb_pid = 0;

	druid_set_sensitive (FALSE, TRUE, TRUE);
	stop_animation ();

	druid_data.fd = 0;
	druid_data.ioc = NULL;
	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-stop")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-go")), TRUE);

	return;
}

void 
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
		GtkWidget *d;
		d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Unable to process core file with gdb:\n"
					      "'%s'"), core_file);
		gtk_dialog_set_default_response (GTK_DIALOG (d), GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (d));
		gtk_widget_destroy (d);
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
		GtkWidget *d;
		d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("GDB was unable to determine which binary created\n"
					      "'%s'"), core_file);
		gtk_dialog_set_default_response (GTK_DIALOG (d), GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (d));
		gtk_widget_destroy (d);
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
	guint len;
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

		tv = GTK_TEXT_VIEW (GET_WIDGET ("gdb-text"));
		buffy = gtk_text_view_get_buffer (tv);

		gtk_text_buffer_get_end_iter (buffy, &end);
		gtk_text_buffer_insert (buffy, &end, buf, len);
	}

	if (!retval)
		stop_gdb ();

	return retval;
}

void
get_trace_from_pair (const gchar *app, const gchar *extra)
{
	GtkWidget *d;
	char *s;
	char *app2;
	char *args[] = { "gdb",
			 "--batch", 
			 "--quiet",
			 "--command=" BUDDY_DATADIR "/gdb-cmd",
			 NULL, NULL, NULL };
	args[0] = g_find_program_in_path ("gdb");
	args[5] = (char *)extra;

	if (!args[0]) {
		d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("GDB could not be found on your system.\n"
					      "Debugging information will not be obtained."));
		d(g_message ("Path: %s", getenv ("PATH")));
		gtk_dialog_set_default_response (GTK_DIALOG (d),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (d));
		gtk_widget_destroy (d);
		return;
	}

	if (!app || !extra || !*app || !*extra)
		return;
	
	/* FIXME: we should probably be fully expanding the link to
	   see if it is a directory, but unix sucks and i am lazy */
	if (g_file_test (app, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_SYMLINK))
		app2 = g_strdup (app);
	else
		app2 = g_find_program_in_path (app);

	if (!app2) {
		g_free (args[0]);
		return;
	}

	args[4] = app2;

	d(g_message ("About to debug '%s'", app2));
	
	if (!g_file_test (BUDDY_DATADIR "/gdb-cmd", G_FILE_TEST_EXISTS)) {
		d = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Could not find the gdb-cmd file.\n"
					      "Please try reinstalling Bug Buddy."));
		gtk_dialog_run (GTK_DIALOG (d));
		gtk_dialog_set_default_response (GTK_DIALOG (d),
						 GTK_RESPONSE_OK);
		gtk_widget_destroy (d);
		g_free (app2);
		return;
	}

	/* FIXME: use GError */
	if (!g_spawn_async_with_pipes (NULL, args, NULL, 0, NULL, NULL,
				       &druid_data.gdb_pid,
				       NULL, 
				       &druid_data.fd, 
				       NULL, NULL)) {
		d = gtk_message_dialog_new (GTK_WINDOW (GTK_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("There was an error running gdb."));
		gtk_dialog_run (GTK_DIALOG (d));
		gtk_dialog_set_default_response (GTK_DIALOG (d),
						 GTK_RESPONSE_OK);
		gtk_widget_destroy (d);
		g_free (app2);
		return;
	}
	
	druid_data.ioc = g_io_channel_unix_new (druid_data.fd);
	
	s = g_strdup_printf ("Backtrace was generated from '%s'\n\n", app2);
	buddy_set_text ("gdb-text", s);
	g_free (s);
	g_io_add_watch (druid_data.ioc, G_IO_IN | G_IO_HUP,
			handle_gdb_input, NULL);
	g_io_channel_unref (druid_data.ioc);

	druid_set_sensitive (FALSE, FALSE, TRUE);
	start_animation ();

	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-stop")), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-go")), FALSE);

	druid_data.explicit_dirty = FALSE;

	g_free (app2);
}
