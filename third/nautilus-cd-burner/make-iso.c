/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   make-iso.c: code to generate iso files
 
   Copyright (C) 2002-2004 Red Hat, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Authors: Alexander Larsson <alexl@redhat.com>
*/

#include "config.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif /* __FreeBSD__ || __NetBSD__ || __OpenBSD__ */
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_STATVFS
#include <sys/statvfs.h>
#endif
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gtk/gtkmessagedialog.h>

#include "nautilus-cd-burner.h"
#include "make-iso.h"

#ifndef HAVE_MKDTEMP
#include "mkdtemp.h"
#endif

struct mkisofs_output {
	GMainLoop *loop;
	int result;
	int pid;
	const char *filename;
	gboolean debug;
};

static struct mkisofs_output *mkisofs_output_ptr;

void
make_iso_cancel (void)
{
	if (mkisofs_output_ptr)
	{
		kill (mkisofs_output_ptr->pid, SIGINT);
		unlink (mkisofs_output_ptr->filename);

		g_main_loop_quit (mkisofs_output_ptr->loop);
	}
}

static void
write_all (int fd, char *buf, int len)
{
	int bytes;
	int res;
	
	bytes = 0;
	while (bytes < len) {
		res = write (fd, buf + bytes, len - bytes);
		if (res <= 0) {
			return;
		}
		bytes += res;
	}
	return;
}

static void
copy_file (const char *source, const char *dest)
{
	int sfd, dfd;
	struct stat stat_buf;
	char buffer[1024*8];
	ssize_t len;

	if (link (source, dest) == 0) {
		return;
	}
	
	if (stat (source, &stat_buf) != 0) {
		g_warning ("Trying to copy nonexisting file\n");
		return;
	}

	sfd = open (source, O_RDONLY);
	if (sfd == -1) {
		g_warning ("Can't copy file (open source failed)\n");
		return;
	}
	
	dfd = open (dest, O_WRONLY | O_CREAT, stat_buf.st_mode);
	if (dfd == -1) {
		close (sfd);
		g_warning ("Can't copy file (open dest '%s' failed)\n", dest);
		perror ("error:");
		return;
	}

	while ( (len = read (sfd, &buffer, sizeof (buffer))) > 0) {
		write_all (dfd, buffer, len);
	}
	close (dfd);
	close (sfd);
}

static char *
escape_path (const char *str)
{
	char *escaped, *d;
	const char *s;
	int len;
	
	s = str;
	len = 1;
	while (*s != 0) {
		if (*s == '\\' ||
		    *s == '=') {
			len++;
		}
		
		len++;
		s++;
	}
	
	escaped = g_malloc (len);
	
	s = str;
	d = escaped;
	while (*s != 0) {
		if (*s == '\\' ||
		    *s == '=') {
			*d++ = '\\';
		}
		
		*d++ = *s++;
	}
	*d = 0;
	
	return escaped;
}

static gboolean
dir_is_empty (const char *virtual_path)
{
	GnomeVFSFileInfo *info;
	GnomeVFSDirectoryHandle *handle;
	GnomeVFSResult result;
	char *escaped_path, *uri;
	gboolean found_file;
	
	escaped_path = gnome_vfs_escape_path_string (virtual_path);
	uri = g_strconcat ("burn:///", escaped_path, NULL);
	g_free (escaped_path);
	
	result = gnome_vfs_directory_open (&handle, uri, GNOME_VFS_FILE_INFO_DEFAULT);
	if (result != GNOME_VFS_OK) {
		g_free (uri);
		return TRUE;
	}
	
	info = gnome_vfs_file_info_new ();

	found_file = FALSE;
	
	while (TRUE) {
		result = gnome_vfs_directory_read_next (handle, info);
		if (result != GNOME_VFS_OK)
			break;

		/* Skip "." and "..".  */
		if (info->name[0] == '.'
		    && (info->name[1] == 0
			|| (info->name[1] == '.' && info->name[2] == 0))) {
			gnome_vfs_file_info_clear (info);
			continue;
		}
		
		found_file = TRUE;
		break;
	}

	gnome_vfs_directory_close (handle);
	gnome_vfs_file_info_unref (info);

	return !found_file;
}

static char *
get_backing_file (const char *virtual_path)
{
	GnomeVFSHandle *handle;
	GnomeVFSResult res;
	char *escaped_path, *uri;
	char *mapping;

	escaped_path = gnome_vfs_escape_path_string (virtual_path);
	uri = g_strconcat ("burn:///", escaped_path, NULL);
	g_free (escaped_path);
	res = gnome_vfs_open (&handle,
			      uri,
			      GNOME_VFS_OPEN_READ);
	g_free (uri);
	if (res == GNOME_VFS_OK) {
		res =  gnome_vfs_file_control (handle,
					       "mapping:get_mapping",
					       &mapping);
		gnome_vfs_close	(handle);
		if (res == GNOME_VFS_OK) {
			return mapping;
		}
	}
	return NULL;
}

static gboolean
ask_disable_joliet (GtkWindow *parent)
{
	GtkWidget *dialog;
	int res;

	dialog = gtk_message_dialog_new (parent,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_QUESTION,
					 GTK_BUTTONS_OK_CANCEL,
					 _("Some files don't have a suitable name for a Windows-compatible CD.\nDo you want to continue with Windows compatibility disabled?"));
	gtk_window_set_title (GTK_WINDOW (dialog), _("Windows compatibility"));
	res = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	return (res == GTK_RESPONSE_OK);
}

struct mkisofs_state {
	FILE *graft_file;
	char *emptydir;
	int depth;
	char *tmpdir;
	char *copy_to_dir;
	int copy_depth;
	GList *remove_files;
	gboolean found_file;
};

static void
graft_file_end_dir_visitor (struct mkisofs_state *state)
{
	char *last_slash;
	
	if (state->copy_depth > 0) {
		state->copy_depth--;
		if (state->copy_depth == 0) {
			g_free (state->copy_to_dir);
			state->copy_to_dir = NULL;
		} else {
			last_slash = strrchr (state->copy_to_dir, '/');
			if (last_slash != NULL) {
				*last_slash = 0;
			}
		}
	}
}

static gboolean
graft_file_visitor (const gchar *rel_path,
		    GnomeVFSFileInfo *info,
		    struct mkisofs_state *state,
		    gboolean *recurse)
{
	char *mapping, *path1, *path2;
	char *new_copy_dir;
	char *copy_path;

	*recurse = TRUE;
	
	if (state->copy_to_dir != NULL) {
		if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			new_copy_dir = g_build_filename (state->copy_to_dir,
							 info->name,
							 NULL);
			g_free (state->copy_to_dir);
			state->copy_to_dir = new_copy_dir;
			mkdir (state->copy_to_dir, 0777);
			state->remove_files = g_list_prepend (state->remove_files, g_strdup (state->copy_to_dir));
			state->copy_depth++;
		} else {
			copy_path = g_build_filename (state->copy_to_dir, info->name, NULL);
			mapping = get_backing_file (rel_path);
			if (mapping != NULL) {
				copy_file (mapping, copy_path);
				state->remove_files = g_list_prepend (state->remove_files, g_strdup (copy_path));
			}
		}
		return TRUE;
	}
	
	if (info->type != GNOME_VFS_FILE_TYPE_DIRECTORY) {
		mapping = get_backing_file (rel_path);
		if (mapping != NULL) {
			path1 = escape_path (rel_path);
			path2 = escape_path (mapping);
			state->found_file = TRUE;
			fprintf (state->graft_file, "%s=%s\n", path1, path2);
			g_free (path1);
			g_free (path2);
			g_free (mapping);
		}
	} else {
		if (dir_is_empty (rel_path)) {
			path1 = escape_path (rel_path);
			path2 = escape_path (state->emptydir);
			state->found_file = TRUE;
			fprintf (state->graft_file, "%s/=%s\n", path1, path2);
			g_free (path1);
			g_free (path2);
		} else if (state->depth >= 6) {
			new_copy_dir = g_build_filename (state->tmpdir, "subdir.XXXXXX", NULL);
			copy_path = mkdtemp (new_copy_dir);
			if (copy_path != NULL) {
				state->remove_files = g_list_prepend (state->remove_files, g_strdup (copy_path));
				state->copy_depth = 1;
				state->copy_to_dir = copy_path;
				path1 = escape_path (rel_path);
				path2 = escape_path (copy_path);
				state->found_file = TRUE;
				fprintf (state->graft_file, "%s/=%s\n", path1, path2);
				g_free (path1);
				g_free (path2);
			} else {
				g_free (new_copy_dir);
				g_warning ("Couldn't create temp subdir\n");
				*recurse = FALSE;
			}
		} 
	}

	return TRUE;
}
	
static void
create_graft_file (GnomeVFSURI *uri,
		   const gchar *prefix,
		   struct mkisofs_state *state)
{
	GnomeVFSFileInfo *info;
	GnomeVFSDirectoryHandle *handle;
	GnomeVFSResult result;
	gboolean stop;

	result = gnome_vfs_directory_open_from_uri (&handle, uri, GNOME_VFS_FILE_INFO_DEFAULT);
	if (result != GNOME_VFS_OK)
		return;

	info = gnome_vfs_file_info_new ();

	stop = FALSE;
	while (! stop) {
		gchar *rel_path;
		gboolean recurse;

		result = gnome_vfs_directory_read_next (handle, info);
		if (result != GNOME_VFS_OK)
			break;

		/* Skip "." and "..".  */
		if (info->name[0] == '.'
		    && (info->name[1] == 0
			|| (info->name[1] == '.' && info->name[2] == 0))) {
			gnome_vfs_file_info_clear (info);
			continue;
		}

		if (prefix == NULL)
			rel_path = g_strdup (info->name);
		else
			rel_path = g_strconcat (prefix, info->name, NULL);

		recurse = FALSE;
		stop = ! graft_file_visitor (rel_path, info, state, &recurse);
		
		if (! stop
		    && recurse
		    && info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			GnomeVFSURI *new_uri;
			gchar *new_prefix;

			if (prefix == NULL)
				new_prefix = g_strconcat (info->name, "/",
							  NULL);
			else
				new_prefix = g_strconcat (prefix, info->name,
							  "/", NULL);

			new_uri = gnome_vfs_uri_append_file_name (uri, info->name);

			state->depth++;
			create_graft_file (new_uri,
					   new_prefix,
					   state);
			state->depth--;
			graft_file_end_dir_visitor (state);

			gnome_vfs_uri_unref (new_uri);
			g_free (new_prefix);
		}

		g_free (rel_path);

		gnome_vfs_file_info_clear (info);

		if (stop)
			break;
	}

	gnome_vfs_directory_close (handle);
	gnome_vfs_file_info_unref (info);
}

static gboolean  
mkisofs_stdout_read (GIOChannel   *source,
		     GIOCondition  condition,
		     gpointer      data)
{
	struct mkisofs_output *mkisofs_output = data;
	char *line;
	GIOStatus status;
	
	status = g_io_channel_read_line (source,
					 &line, NULL, NULL, NULL);

	if (line && mkisofs_output->debug) {
		g_print ("make_iso stdout: %s", line);
	}

	if (status == G_IO_STATUS_NORMAL) {
		g_free (line);
	}
	return TRUE;
}

static gboolean  
mkisofs_stderr_read (GIOChannel   *source,
		     GIOCondition  condition,
		     gpointer      data)
{
	struct mkisofs_output *mkisofs_output = data;
	char *line;
	char fraction_str[7];
	double fraction;
	GIOStatus status;

	status = g_io_channel_read_line (source,
					 &line, NULL, NULL, NULL);

	if (line && mkisofs_output->debug) {
		g_print ("make_iso stderr: %s", line);
	}

	if (status == G_IO_STATUS_NORMAL) {
		if (strncmp (line, "Total translation table size", 28) == 0) {
			cd_progress_set_fraction (1.0);
			g_main_loop_quit (mkisofs_output->loop);
			mkisofs_output->result = RESULT_FINISHED;
		}

		if (strstr (line, "estimate finish")) {
			if (sscanf (line, "%6c%% done, estimate finish",
						fraction_str) == 1) {
				fraction_str[6] = 0;
				fraction = g_strtod (fraction_str, NULL);
				cd_progress_set_fraction (fraction/100.0);
			}
		}
		if (strstr (line, "Incorrectly encoded string")) {
			cd_progress_set_fraction (1.0);
			g_main_loop_quit (mkisofs_output->loop);
			mkisofs_output->result = RESULT_ERROR;
		}

		g_free (line);
	}
	return TRUE;
}

/**
 * Create an ISO image in filename from the data files in burn:///
 */
int
make_iso (const char *filename, const char *label,
		gboolean warn_low_space, gboolean use_joliet, gboolean debug)
{
	GnomeVFSURI *uri;
	char *filelist = NULL;
	struct mkisofs_state state = {NULL};
	char *tempdir;
	GList *l;
	int stdout_pipe, stderr_pipe;
	int i;
	GError *error;
	const char *argv[20]; /* Shouldn't need more than 20 arguments */
	struct mkisofs_output mkisofs_output;
	GIOChannel *channel;
	guint stdout_tag, stderr_tag;
	char *stdout_data, *stderr_data;
	char *dirname;
	int exit_status, res;
	unsigned long iso_size;
#ifdef HAVE_STATVFS
	struct statvfs statfs_buf;
#else
	struct statfs statfs_buf;
#endif
	GtkWidget *dialog;

	if (label) {
		g_return_val_if_fail (strlen (label) < 32, RESULT_ERROR);
	}

	mkisofs_output.debug = debug;

	dirname = g_strdup_printf ("iso-%s.XXXXXX", g_get_user_name ());
	tempdir = g_build_filename (g_get_tmp_dir (), dirname, NULL);
	g_free (dirname);
	state.tmpdir = mkdtemp (tempdir);

	mkisofs_output.result = RESULT_ERROR;

	if (state.tmpdir == 0) {
		g_warning ("Unable to create temp dir\n");
		goto cleanup;
	}
	
	state.emptydir = g_build_filename (state.tmpdir, "emptydir", NULL);
	mkdir (state.emptydir, 0777);
	state.remove_files = g_list_prepend (state.remove_files, g_strdup (state.emptydir));

	filelist = g_build_filename (state.tmpdir, "filelist", NULL);

	state.graft_file = fopen (filelist, "w");
	if (state.graft_file == NULL) {
		goto cleanup;
	}
	
	uri = gnome_vfs_uri_new ("burn:///");
	state.found_file = FALSE;
	create_graft_file (uri, NULL, &state);
	gnome_vfs_uri_unref (uri);

	fclose (state.graft_file);
	state.remove_files = g_list_prepend (state.remove_files, g_strdup (filelist));

	if (!state.found_file) {
		dialog = gtk_message_dialog_new (cd_progress_get_window (),
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 _("No files selected to write to CD."));
		gtk_window_set_title (GTK_WINDOW (dialog),
				      _("No files selected"));
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		mkisofs_output.result = RESULT_ERROR;
		goto cleanup;
	}

retry:
	i = 0;
	argv[i++] = "mkisofs";
	argv[i++] = "-r";
	if (use_joliet) {
		argv[i++] = "-J";
	}
	/* Undocumented -input-charset option */
	argv[i++] = "-input-charset";
	argv[i++] = "utf8";
	argv[i++] = "-q";
	argv[i++] = "-graft-points";
	argv[i++] = "-path-list";
	argv[i++] = filelist;
	argv[i++] = "-print-size";
	argv[i++] = NULL;

	error = NULL;

	if (debug) {
		g_print ("launching command: ");
		for (i = 0; argv[i] != NULL; i++) {
			g_print ("%s ", argv[i]);
		}
		g_print ("\n");
	}

	if (!g_spawn_sync (NULL,
			   (char **)argv,
			   NULL,
			   G_SPAWN_SEARCH_PATH,
			   NULL, NULL,
			   &stdout_data,
			   &stderr_data,
			   &exit_status,
			   &error)) {
		g_warning ("mkisofs command failed: %s\n", error->message);
		g_error_free (error);
		/* TODO: Better error handling */
		mkisofs_output.result = RESULT_ERROR;
		goto cleanup;
	}

	if (exit_status != 0 && use_joliet) {
		if (strstr (stderr_data, "Joliet tree sort failed.") != NULL) {
			g_free (stdout_data);
			g_free (stderr_data);
			if (ask_disable_joliet (cd_progress_get_window ())) {
				use_joliet = FALSE;
				goto retry;
			} else {
				mkisofs_output.result = RESULT_ERROR;
				goto cleanup;
			}
		}
	}

	g_free (stderr_data);
	iso_size = atol (stdout_data); /* blocks of 2048 bytes */
	g_free (stdout_data);

	dirname = g_path_get_dirname (filename);
#ifdef HAVE_STATVFS
	res = statvfs (dirname, &statfs_buf);
#else
	res = statfs (dirname, &statfs_buf);
#endif
	if (res == -1) {
		g_warning ("Cannot get free space at %s\n", dirname);
		g_free (dirname);
	} else if (iso_size / statfs_buf.f_bsize >= statfs_buf.f_bavail / 2048) {
		g_free (dirname);
		if (warn_low_space) {
			dialog = gtk_message_dialog_new (cd_progress_get_window (),
							 GTK_DIALOG_DESTROY_WITH_PARENT,
							 GTK_MESSAGE_ERROR,
							 GTK_BUTTONS_CLOSE,
							 ngettext("Not enough space to store CD image (%ld Megabyte needed)",
								  "Not enough space to store CD image (%ld Megabytes needed)",
								  iso_size / 512),
							 iso_size / 512);
			gtk_window_set_title (GTK_WINDOW (dialog),
					      _("Not enough space"));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			mkisofs_output.result = RESULT_ERROR;
		} else {
			mkisofs_output.result = RESULT_RETRY;
		}

		goto cleanup;
	}

	i = 0;
	argv[i++] = "mkisofs";
	argv[i++] = "-r";
	if (use_joliet) {
		argv[i++] = "-J";
	}
	argv[i++] = "-input-charset";
	argv[i++] = "utf8";
	argv[i++] = "-graft-points";
	argv[i++] = "-path-list";
	argv[i++] = filelist;
	if (label) {
		argv[i++] = "-V";
		argv[i++] = label;
	}
	argv[i++] = "-o";
	argv[i++] = filename;
	argv[i++] = NULL;

	if (debug) {
		g_print ("launching command: ");
		for (i = 0; argv[i] != NULL; i++) {
			g_print ("%s ", argv[i]);
		}
		g_print ("\n");
	}

	cd_progress_set_text (_("Creating CD image"));
	cd_progress_set_fraction (0.0);
	cd_progress_set_image_spinning (TRUE);
	error = NULL;
	if (!g_spawn_async_with_pipes  (NULL,
					(char **)argv,
					NULL,
					G_SPAWN_SEARCH_PATH,
					NULL, NULL,
					&mkisofs_output.pid,
					/*stdin*/NULL,
					&stdout_pipe,
					&stderr_pipe,
					&error)) {
		g_warning ("mkisofs command failed: %s\n", error->message);
		g_error_free (error);
		mkisofs_output.result = RESULT_ERROR;
		goto cleanup;
	} else {
		mkisofs_output.loop = g_main_loop_new (NULL, FALSE);
	
		channel = g_io_channel_unix_new (stdout_pipe);
		g_io_channel_set_encoding (channel, NULL, NULL);
		stdout_tag = g_io_add_watch (channel, 
					     (G_IO_IN | G_IO_HUP | G_IO_ERR), 
					     mkisofs_stdout_read,
					     &mkisofs_output);
		g_io_channel_unref (channel);
		channel = g_io_channel_unix_new (stderr_pipe);
		g_io_channel_set_encoding (channel, NULL, NULL);
		stderr_tag = g_io_add_watch (channel, 
					     (G_IO_IN | G_IO_HUP | G_IO_ERR), 
					     mkisofs_stderr_read,
					     &mkisofs_output);
		g_io_channel_unref (channel);

		mkisofs_output_ptr = &mkisofs_output;
		mkisofs_output.filename = filename;
		
		g_main_loop_run (mkisofs_output.loop);
		g_main_loop_unref (mkisofs_output.loop);
		
		g_source_remove (stdout_tag);
		g_source_remove (stderr_tag);
	}
	mkisofs_output_ptr = NULL;

 cleanup:
	for (l = state.remove_files; l != NULL; l = l->next) {
		remove ((char *)l->data);
		g_free (l->data);
	}
	g_list_free (state.remove_files);
	
	g_free (filelist);
	g_free (state.emptydir);
	rmdir (tempdir);
	g_free (tempdir);
	cd_progress_set_image_spinning (FALSE);

	return mkisofs_output.result;
}

