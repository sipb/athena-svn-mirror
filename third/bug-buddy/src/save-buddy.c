/*
 * Save a file.
 *
 * Taken from gedit:
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 *
 * (although i think federico wrote this part, taking it from emacs)
 *
 * Hacked up by jacob berkman  <jacob@bug-buddy.org>
 *
 * Copyright 2002 Ximian, Inc.
 *
 */

#include <config.h>

#include "save-buddy.h"
#include "signal-buddy.h"

#include <libgnome/gnome-i18n.h>
#include <gtk/gtk.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define MAX_LINK_LEVEL 256

GQuark
bb_save_error_quark (void)
{
	static GQuark quark = 0;

	if (!quark)
		quark = g_quark_from_static_string ("bb_save_error");

	return quark;
}

/* Does readlink() recursively until we find a real filename. */
static char *
follow_symlinks (const char *filename, GError **error)
{
	char *followed_filename;
	int link_count;

	g_return_val_if_fail (filename != NULL, NULL);
	
	g_return_val_if_fail (strlen (filename) + 1 <= MAXPATHLEN, NULL);

	followed_filename = g_strdup (filename);
	link_count = 0;

	while (link_count < MAX_LINK_LEVEL) {
		struct stat st;

		if (lstat (followed_filename, &st) != 0)
			/* We could not access the file, so perhaps it does not
			 * exist.  Return this as a real name so that we can
			 * attempt to create the file.
			 */
			return followed_filename;

		if (S_ISLNK (st.st_mode)) {
			int len;
			char linkname[MAXPATHLEN];

			link_count++;

			len = readlink (followed_filename, linkname, MAXPATHLEN - 1);

			if (len == -1) {
				g_set_error (error, BB_SAVE_ERROR, errno,
					     _("Could not read symbolic link information "
					       "for %s"), followed_filename);
				g_free (followed_filename);
				return NULL;
			}

			linkname[len] = '\0';

			/* If the linkname is not an absolute path name, append
			 * it to the directory name of the followed filename.  E.g.
			 * we may have /foo/bar/baz.lnk -> eek.txt, which really
			 * is /foo/bar/eek.txt.
			 */

			if (linkname[0] != G_DIR_SEPARATOR) {
				char *slashpos;
				char *tmp;

				slashpos = strrchr (followed_filename, G_DIR_SEPARATOR);

				if (slashpos)
					*slashpos = '\0';
				else {
					tmp = g_strconcat ("./", followed_filename, NULL);
					g_free (followed_filename);
					followed_filename = tmp;
				}

				tmp = g_build_filename (followed_filename, linkname, NULL);
				g_free (followed_filename);
				followed_filename = tmp;
			} else {
				g_free (followed_filename);
				followed_filename = g_strdup (linkname);
			}
		} else
			return followed_filename;
	}

	/* Too many symlinks */

	g_set_error (error, BB_SAVE_ERROR, ELOOP,
		     _("The file has too many symbolic links."));

	return NULL;
}

typedef struct {
	GIOChannel *ioc;
	GtkWindow *parent;
	GtkWidget *dialog;
	GError **error;
	GMainLoop *loop;

	const char *buf;

	gssize bytes_to_write;
	gssize bytes_written;

	gboolean cancelled;
	int pid;

	guint watch_id;
	guint timeout_id;

	const char *wait_msg;
} AsyncXferData;

static gboolean
iofunc (GIOChannel *ioc, GIOCondition cond, gpointer data)
{
	AsyncXferData *xfer_data = data;
	gsize newly_written;
	gboolean retval = FALSE;
	GIOStatus iostatus;

	switch (cond)
	{
	case G_IO_OUT:
	iofunc_do_write:
		iostatus = g_io_channel_write_chars (ioc, 
						     xfer_data->buf + xfer_data->bytes_written, 
						     MIN (xfer_data->bytes_to_write - xfer_data->bytes_written, 1024),
						     &newly_written, xfer_data->error);
		switch (iostatus)
		{
		case G_IO_STATUS_AGAIN:
			goto iofunc_do_write;
		case G_IO_STATUS_ERROR:
			break;
		case G_IO_STATUS_EOF:
			break;
		case G_IO_STATUS_NORMAL:
			xfer_data->bytes_written += newly_written;
			if (newly_written)
				retval = TRUE;
			break;
		}
		break;
	case G_IO_ERR:
	case G_IO_HUP:
		break;
	default:
		g_assert_not_reached ();
	}

	if (!retval)
	{
		xfer_data->ioc = NULL;
		xfer_data->watch_id = 0;
		g_io_channel_shutdown (ioc, FALSE, NULL);
		if (xfer_data->pid <= 0)
			g_main_loop_quit (xfer_data->loop);
	}

	return retval;
}

static void
response_cb (GtkWidget *dialog, guint response, gpointer data)
{
	AsyncXferData *xfer_data = data;

	/* killing the child will trigger our signal handler which quits the
	   main loop */
	if (xfer_data->pid > 0)
		kill (xfer_data->pid, SIGKILL);
	else
		g_main_loop_quit (xfer_data->loop);

	xfer_data->cancelled = TRUE;
}

static gboolean
pulse_cb (gpointer data)
{
	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (data));
	return TRUE;
}

static gboolean
timeout_cb (gpointer data)
{
	AsyncXferData *xfer_data = data;
	GtkWidget *w;

	w = gtk_label_new (xfer_data->wait_msg);
	gtk_widget_show (GTK_WIDGET (w));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (xfer_data->dialog)->vbox), w, TRUE, FALSE, 5);

	w = gtk_progress_bar_new ();
	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (w));
	gtk_widget_show (GTK_WIDGET (w));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (xfer_data->dialog)->vbox), w, TRUE, FALSE, 5);
	xfer_data->timeout_id = gtk_timeout_add (333, pulse_cb, w);

	g_signal_connect (xfer_data->dialog, "response",
			  G_CALLBACK (response_cb), xfer_data);


	gtk_widget_show_now (xfer_data->dialog);
	gtk_widget_queue_draw (xfer_data->dialog);
	gdk_window_process_all_updates ();

	return FALSE;
}

static void
signal_notify (int sig)
{
	bb_signal_notify (sig);
	g_main_context_wakeup (NULL);
}

static gboolean
sigchld_cb (gint8 sig, gpointer data)
{
	AsyncXferData *xfer_data = data;  

	g_return_val_if_fail (sig == SIGCHLD, TRUE);
	g_return_val_if_fail (data != NULL, TRUE);

	if (g_main_loop_is_running (xfer_data->loop))
		g_main_loop_quit (xfer_data->loop);
	else
		g_warning (_("Main loop isn't running!"));

	return TRUE;
}

static void
setup_sigchld_handler (AsyncXferData *xfer_data)
{
	struct sigaction sa;

	sa.sa_handler = signal_notify;
	sa.sa_flags = SA_RESTART|SA_NOCLDSTOP;
	sigemptyset (&sa.sa_mask);
	sigaddset (&sa.sa_mask, SIGCHLD);

	if (sigaction (SIGCHLD, &sa, NULL) < 0)
		g_error (_("Error setting up sigchld handler: %s"), g_strerror (errno));

	bb_signal_add (SIGCHLD, sigchld_cb, xfer_data);
}

static gboolean
bb_write_buffer_to_fd (GtkWindow *parent, const char *wait_msg, int fd, int pid, const char *buffer, gssize buflen, GError **error)
{
	AsyncXferData xfer_data = { NULL };

	xfer_data.ioc = g_io_channel_unix_new (fd);

	xfer_data.parent = parent;
	xfer_data.wait_msg = wait_msg;
	xfer_data.buf = buffer;
	xfer_data.bytes_to_write = buflen >= 0 ? buflen : strlen (buffer);
	xfer_data.bytes_written = 0;
	xfer_data.cancelled = FALSE;

	/* this just complicates things */
	g_io_channel_set_encoding (xfer_data.ioc, NULL, NULL);
	g_io_channel_set_buffered (xfer_data.ioc, FALSE);

	/* When we can write */
	xfer_data.watch_id = g_io_add_watch (xfer_data.ioc, G_IO_OUT | G_IO_ERR | G_IO_HUP, iofunc, &xfer_data);

	/* Wait a bit before popping up a dialog */
	xfer_data.timeout_id = gtk_timeout_add (1100, timeout_cb, &xfer_data);

	xfer_data.pid = pid;
	if (pid > 0)
		setup_sigchld_handler (&xfer_data);

	xfer_data.loop = g_main_loop_new (NULL, FALSE);

	xfer_data.dialog = gtk_dialog_new_with_buttons (NULL, xfer_data.parent,
							GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_MODAL,
							GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							NULL);
	/* make it modal */
	gtk_widget_realize (xfer_data.dialog);
	gtk_grab_add (xfer_data.dialog);

	g_main_loop_run (xfer_data.loop);

	g_main_loop_unref (xfer_data.loop);

	if (xfer_data.ioc) {
		g_io_channel_shutdown (xfer_data.ioc, TRUE, NULL);
		g_io_channel_unref (xfer_data.ioc);
		g_assert (xfer_data.watch_id);
		g_source_remove (xfer_data.watch_id);
	}

	if (xfer_data.dialog)
		gtk_widget_destroy (xfer_data.dialog);

	if (xfer_data.timeout_id)
		g_source_remove (xfer_data.timeout_id);

	if (pid > 0)
		waitpid (xfer_data.pid, NULL, NULL);

	/* We wrote all the data if ioc is NULL */
	if (xfer_data.ioc || xfer_data.cancelled)
		return FALSE;

	return TRUE;
}

/* FIXME: define new ERROR_CODE and remove the error 
 * strings from here -- Paolo
 */

gboolean	
bb_write_buffer_to_file (GtkWindow *parent, const char *wait_msg, const char *filename, const char *buffer, gssize buflen, GError **error)
{
	char *real_filename; /* Final filename with no symlinks */
	char *backup_filename; /* Backup filename, like real_filename.bak */
	char *temp_filename; /* Filename for saving */
	char *slashpos;
	char *dirname;
	mode_t saved_umask;
	struct stat st;
	int fd;
	int retval;
	gboolean create_backup_copy = TRUE;
	
	g_return_val_if_fail (buffer != NULL, FALSE);
	g_return_val_if_fail (g_utf8_validate (buffer, -1, NULL), FALSE);
	if (parent)
		g_return_val_if_fail (GTK_IS_WINDOW (parent), FALSE);

	retval = FALSE;

	real_filename = NULL;
	backup_filename = NULL;
	temp_filename = NULL;

	/* We don't support non-file:/// stuff */

	if (!filename || !*filename) {
		g_set_error (error, BB_SAVE_ERROR, 0,
			     _("Invalid filename."));
		goto out;
	}

	/* Get the real filename and file permissions */

	real_filename = follow_symlinks (filename, error);

	if (!real_filename)
		goto out;

	if (stat (real_filename, &st) != 0) {
		/* File does not exist? */
		create_backup_copy = FALSE;

		/* Use default permissions */
		st.st_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		st.st_uid = getuid ();
		st.st_gid = getgid ();
	} else {
		GtkWidget *d;

		d = gtk_message_dialog_new (parent, 0,
					    GTK_MESSAGE_QUESTION,
					    GTK_BUTTONS_NONE,
					    _("There already exists a file name '%s'.\n\n"
					      "Do you wish to overwrite this file?"),
					    filename);
		gtk_dialog_add_buttons (GTK_DIALOG (d),
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					_("_Overwrite"), GTK_RESPONSE_OK,
					NULL);
		gtk_dialog_set_default_response (GTK_DIALOG (d), GTK_RESPONSE_OK);

		if (GTK_RESPONSE_OK != gtk_dialog_run (GTK_DIALOG (d))) {
			gtk_widget_destroy (d);
			goto out;
		}
		gtk_widget_destroy (d);
	}

	/* Save to a temporary file.  We set the umask because some (buggy)
	 * implementations of mkstemp() use permissions 0666 and we want 0600.
	 */

	slashpos = strrchr (real_filename, G_DIR_SEPARATOR);

	if (slashpos) {
		dirname = g_strdup (real_filename);
		dirname[slashpos - real_filename + 1] = '\0';
	} else
		dirname = g_strdup (".");

	temp_filename = g_build_filename (dirname, ".bug-buddy-save-XXXXXX", NULL);
	g_free (dirname);

	saved_umask = umask (0077);
	fd = g_mkstemp (temp_filename); /* this modifies temp_filename to the used name */
	umask (saved_umask);

	if (fd == -1) {
		g_set_error (error, BB_SAVE_ERROR, errno, " ");
		goto out;
	}

#if 0
	encoding_setting = gedit_prefs_manager_get_save_encoding ();

	if (encoding_setting == GEDIT_SAVE_CURRENT_LOCALE_IF_POSSIBLE) {
		GError *conv_error = NULL;
		char* converted_file_contents = NULL;

		gedit_debug (DEBUG_DOCUMENT, "Using current locale's encoding");

		converted_file_contents = g_locale_from_utf8 (chars, -1, NULL, NULL, &conv_error);

		if (conv_error != NULL) {
			/* Conversion error */
			g_error_free (conv_error);
		} else {
			g_free (chars);
			chars = converted_file_contents;
		}
	} else {
		if ((doc->priv->encoding != NULL) &&
		    ((encoding_setting == GEDIT_SAVE_ORIGINAL_FILE_ENCODING_IF_POSSIBLE) ||
		     (encoding_setting == GEDIT_SAVE_ORIGINAL_FILE_ENCODING_IF_POSSIBLE_NCL)))
		{
			GError *conv_error = NULL;
			char* converted_file_contents = NULL;

			gedit_debug (DEBUG_DOCUMENT, "Using encoding %s", doc->priv->encoding);

			/* Try to convert it from UTF-8 to original encoding */
			converted_file_contents = g_convert (chars, -1, 
							     doc->priv->encoding, "UTF-8", 
							     NULL, NULL, &conv_error); 

			if (conv_error != NULL) {
				/* Conversion error */
				g_error_free (conv_error);
			} else {
				g_free (chars);
				chars = converted_file_contents;
			}
		} else
			gedit_debug (DEBUG_DOCUMENT, "Using UTF-8 (Null)");

	}	    
#endif

	if (!bb_write_buffer_to_fd (parent, wait_msg, fd, -1, buffer, buflen, error)) {
		unlink (temp_filename);
		goto out;
	}

	/* Move the original file to a backup */

	if (create_backup_copy) {	
		int result;

		backup_filename = g_strconcat (real_filename, 
				               "~",
					       NULL);

		result = rename (real_filename, backup_filename);

		if (result != 0) {
			g_set_error (error, BB_SAVE_ERROR, errno,
				     _("Could not create a backup file."));
			unlink (temp_filename);
			goto out;
		}
	}

	/* Move the temp file to the original file */

	if (rename (temp_filename, real_filename) != 0) {
		int saved_errno;

		saved_errno = errno;

		if (create_backup_copy && rename (backup_filename, real_filename) != 0)
			g_set_error (error, BB_SAVE_ERROR, saved_errno,
				     " ");
		else
			g_set_error (error, BB_SAVE_ERROR, saved_errno,
				     " ");

		goto out;
	}

	/* Restore permissions.  There is not much error checking we can do
	 * here, I'm afraid.  The final data is saved anyways.
	 */

	chmod (real_filename, st.st_mode);
	chown (real_filename, st.st_uid, st.st_gid);

	retval = TRUE;

	/* Done */

 out:
	g_free (real_filename);
	g_free (backup_filename);
	g_free (temp_filename);

	return retval;
}

gboolean
bb_write_buffer_to_command (GtkWindow *parent, const char *wait_msg, char **argv, const char *buffer, gssize buflen, GError **error)
{
  int fd, pid;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL,
				 G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,
				 NULL, NULL,
				 &pid, &fd,
				 NULL, NULL, error))
    return FALSE;

  return bb_write_buffer_to_fd (parent, wait_msg, fd, pid, buffer, buflen, error);
}
