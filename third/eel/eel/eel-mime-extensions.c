/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
   eel-mime-extensions.c: MIME database manipulation
 
   Copyright (C) 2004 Novell, Inc.
 
   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Dave Camp <dave@novell.com>
*/

#include <config.h>
#include "eel-mime-extensions.h"

#include <glib.h>
#include <libgnomevfs/gnome-vfs-mime.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <libgnomevfs/gnome-vfs-mime-info-cache.h>
#include <libxml/parser.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static gboolean
recursive_mkdir (const char *path)
{
	/* I cannot believe this isn't in some library somewhere */

	char **split_path;
	char *so_far;
	int i;
	
	split_path = g_strsplit (path, G_DIR_SEPARATOR_S, 0);

	so_far = g_strdup ("/");
	for (i = 0; split_path[i] != NULL; i++) {
		char *to_check;
		
		to_check = g_build_filename (so_far, split_path[i], NULL);
		g_free (so_far);

		if (!g_file_test (to_check, G_FILE_TEST_EXISTS)) {
			if (mkdir (to_check, S_IRWXU) != 0) {
				g_warning ("Unable to create %s", to_check);
				g_free (to_check);
				return FALSE;
			}
		}
		
		so_far = to_check;
	}
	g_free (so_far);
	
	g_strfreev (split_path);

	return TRUE;
}

static char *
get_user_dir (const char *suffix)
{
	const char *xdg_data_home;
	char *user_dir;
	
	xdg_data_home = g_getenv ("XDG_DATA_HOME");
	
	if (xdg_data_home) {
		user_dir = g_build_filename (xdg_data_home, suffix, NULL);
	} else {
		const char *home;
		
		home = g_getenv ("HOME");
		
		if (home) {
			user_dir = g_build_filename (home, "/.local/share/", suffix, NULL);
		} else {
			return NULL;
		}
	}
	return user_dir;
}

static gboolean 
ensure_mime_dir (void)
{
	char *user_dir = get_user_dir ("mime");
	char *package_dir;
	
	package_dir = g_build_filename (user_dir, "/packages", NULL);
	g_free (user_dir);
	
	if (!recursive_mkdir (package_dir)) {
		g_free (package_dir);
		return FALSE;
	}
	
	g_free (package_dir);
	return TRUE;
}

static gboolean 
ensure_application_dir (void)
{
	char *user_dir = get_user_dir ("applications");
	
	if (!recursive_mkdir (user_dir)) {
		g_free (user_dir);
		return FALSE;
	}
	
	g_free (user_dir);
	return TRUE;
}

static gboolean
write_desktop_file (const char *filename, const char *text)
{
	int fd;
	GIOChannel *out;
	gboolean ret;

	ret = FALSE;
	fd = open (filename, O_CREAT | O_WRONLY | O_EXCL, 0644);
	if (fd > -1) {
		out = g_io_channel_unix_new (fd);
		g_io_channel_set_close_on_unref (out, TRUE);
		
		ret = (g_io_channel_write_chars (out, text, strlen (text), NULL, NULL) == G_IO_STATUS_NORMAL);
		g_io_channel_unref (out);
	}
	
	return ret;
}

static void
mime_update_program_done (GPid     pid,
			   gint     status,
			   gpointer data)
{
	/* Did the application exit correctly */
	if (WIFEXITED (status) &&
	    WEXITSTATUS (status) == 0) {
		/* FIXME: what is the right thing to do here??  We're going to
		 * have to rethink the gnome-vfs APIs.
		 */
		gnome_vfs_mime_reload ();
	}
}

static void
run_update_command (char *command,
		    char *subdir)
{
	char *argv[3] = {
		NULL,
		NULL
	};
	GPid pid = 0;
	GError *error = NULL;

	argv[0] = command;
	argv[1] = get_user_dir (subdir);

	if (g_spawn_async ("/", argv,
			   NULL,       /* envp */
			   G_SPAWN_SEARCH_PATH |
			   G_SPAWN_STDOUT_TO_DEV_NULL |
			   G_SPAWN_STDERR_TO_DEV_NULL |
			   G_SPAWN_DO_NOT_REAP_CHILD,
			   NULL, NULL, /* No setup function */
			   &pid,
			   &error)) {
		g_child_watch_add (pid, mime_update_program_done, NULL);
	}
	/* If we get an error at this point, it's quite likely the user doesn't
	 * have an installed copy of either 'update-mime-database' or
	 * 'update-desktop-database'.  I don't think we want to popup an error
	 * dialog at this point, so we just do a g_warning to give the user a
	 * chance of debugging it.
	 */
	if (error) {
		g_warning ("%s", error->message);
		g_error_free (error);
	}
	g_free (argv[1]);
}

GnomeVFSMimeApplication *
eel_mime_add_application (const char *mime_type,
			  const char *command_line,
			  const char *name,
			  gboolean needs_terminal)
{
	GnomeVFSMimeApplication *ret;
	char *application_dir;
	char *filename;
	char *basename;
	char *desktop_text;
	int i;
	
	if (!ensure_application_dir ()) {
		return NULL;
	}

	application_dir = get_user_dir ("applications");

	/* Get a filename */
	basename = g_strdup_printf ("%s.desktop", name);
	filename = g_build_filename (application_dir, basename, NULL);

	i = 1;
	while (g_file_test (filename, G_FILE_TEST_EXISTS)) {
		g_free (basename);
		g_free (filename);
		
		basename = g_strdup_printf ("%s%d.desktop", name, i);
		filename = g_build_filename (application_dir, basename, NULL);
		
		i++;
	}

	desktop_text = g_strdup_printf (
		"[Desktop Entry]\n"
		"Encoding=UTF-8\n"
		"Name=%s\n"
		"MimeType=%s;\n"
		"Exec=%s\n"
		"Type=Application\n"
		"Terminal=%s\n"
		"NoDisplay=true\n",
		name, mime_type, 
		command_line, needs_terminal ? "true" : "false");

	if (write_desktop_file (filename, desktop_text)) {
		ret = gnome_vfs_mime_application_new_from_id (basename);
		run_update_command ("update-desktop-database", "applications");
	} else {
		ret = NULL;
	}

        g_free (desktop_text);
	g_free (basename);
	g_free (filename);
	g_free (application_dir);
	
	return ret;
}

static char *
get_override_filename (void)
{
	char *user_dir;
	char *filename;
	
	user_dir = get_user_dir ("mime");
	
	filename = g_build_filename (user_dir, "packages/Override.xml", NULL);
	
	g_free (user_dir);

	return filename;
}

static xmlDocPtr 
get_override (void)
{
	char *filename;
	xmlDocPtr doc;
	
	filename = get_override_filename ();
	
	doc = NULL;
	if (g_file_test (filename, G_FILE_TEST_EXISTS)) {
		doc = xmlParseFile (filename);
	}
	
	if (!doc) {
		xmlNodePtr root_node;
		xmlNsPtr ns;
		doc = xmlNewDoc ("1.0");

		root_node = xmlNewNode (NULL, "mime-info");
		ns = xmlNewNs (root_node, "http://www.freedesktop.org/standards/shared-mime-info", NULL);
		
		xmlDocSetRootElement (doc, root_node);
	}
	
	return doc;
}

static gboolean
write_override (xmlDocPtr doc)
{
	char *override = get_override_filename ();
	int ret;
	
	ret = xmlSaveFormatFileEnc (override, doc, "UTF-8", 1);
	
	g_free (override);

	return (ret != -1);
}

static xmlNodePtr
create_type_node (xmlDocPtr doc, const char *mime_type)
{
	xmlNodePtr root;
	xmlNodePtr type_node;

	root = xmlDocGetRootElement (doc);
	
	type_node = xmlNewChild (root, NULL, "mime-type", "");
	xmlSetNsProp (type_node, NULL, "type", mime_type);

	return type_node;
}

static xmlNodePtr
get_type_node (xmlDocPtr doc, const char *mime_type)
{
	xmlNodePtr root;
	xmlNodePtr node;
	
	root = xmlDocGetRootElement (doc);
	
	node = NULL;
	for (node = root->children; node != NULL; node = node->next) {
		if (!strcmp (node->name, "mime-type")) {
			char *name;
			name = xmlGetProp (node, "type");
			if (!strcmp (name, mime_type)) {
				xmlFree (name);
				break;
			}
			
			xmlFree (name);
		}
	}

	if (!node) {
		node = create_type_node (doc, mime_type);
	}

	return node;
}

static xmlNodePtr
get_comment_node (xmlDocPtr doc, xmlNodePtr type_node)
{
	xmlNodePtr node;
	
	for (node = type_node->children; node != NULL; node = node->next) {
		if (!strcmp (node->name, "comment")) {
			break;
		}
	}

	if (!node) {
		node = xmlNewChild (type_node, NULL, "comment", "");
	}	
	return node;
}

static void
add_glob_node (xmlDocPtr doc, xmlNodePtr type_node, const char *glob)
{
	xmlNodePtr node;
	
	for (node = type_node->children; node != NULL; node = node->next) {
		if (!strcmp (node->name, "glob")) {
			char *name;
			name = xmlGetProp (node, "pattern");
			if (!strcmp (name, glob)) {
				xmlFree (name);
				return;
			}
			
			xmlFree (name);
		}
	}

	node = xmlNewChild (type_node, NULL, "glob", NULL);
	xmlSetNsProp (node, NULL, "pattern", glob);	
}

gboolean
eel_mime_add_glob_type (const char *mime_type,
			const char *description,
			const char *glob)
{
	xmlDocPtr override;
	xmlNodePtr type_node;
	xmlNodePtr comment_node;
	
	if (!ensure_mime_dir ()) {
		return FALSE;
	}

	override = get_override ();
	
	type_node = get_type_node (override, mime_type);

	comment_node = get_comment_node (override, type_node);
	xmlNodeSetContent (comment_node, description);

	add_glob_node (override, type_node, glob);

	if (!write_override (override)) {
		return FALSE;
	}

	run_update_command ("update-mime-database", "mime");

	return TRUE;
}

/* stole this from update-desktop-database */
#define TEMP_CACHE_FILENAME_PREFIX ".defaults.list"
#define TEMP_CACHE_FILENAME_MAX_LENGTH 64

static int
open_temp_cache_file (const char *dir, char **filename, GError **error)
{
  GRand *rand;
  GString *candidate_filename;
  int fd;

  enum 
    { 
      CHOICE_UPPER_CASE = 0, 
      CHOICE_LOWER_CASE, 
      CHOICE_DIGIT, 
      NUM_CHOICES
    } choice;

  candidate_filename = g_string_new (TEMP_CACHE_FILENAME_PREFIX);

  /* Generate a unique candidate_filename and open it for writing
   */
  rand = g_rand_new ();
  fd = -1;
  while (fd < 0)
    {
      char *full_path;

      if (candidate_filename->len > TEMP_CACHE_FILENAME_MAX_LENGTH) 
        g_string_assign (candidate_filename, TEMP_CACHE_FILENAME_PREFIX);

      choice = g_rand_int_range (rand, 0, NUM_CHOICES);

      switch (choice)
        {
        case CHOICE_UPPER_CASE:
          g_string_append_c (candidate_filename,
                             g_rand_int_range (rand, 'A' , 'Z' + 1));
          break;
        case CHOICE_LOWER_CASE:
          g_string_append_c (candidate_filename,
                             g_rand_int_range (rand, 'a' , 'z' + 1));
          break;
        case CHOICE_DIGIT:
          g_string_append_c (candidate_filename,
                             g_rand_int_range (rand, '0' , '9' + 1));
          break;
        default:
          g_assert_not_reached ();
          break;
        }

      full_path = g_build_filename (dir, candidate_filename->str, NULL);
      fd = open (full_path, O_CREAT | O_WRONLY | O_EXCL, 0644); 

      if (fd < 0) 
        {
          if (errno != EEXIST)
            {
              g_set_error (error, G_FILE_ERROR,
                           g_file_error_from_errno (errno),
                           "%s", g_strerror (errno));
              break;
            }
        }
      else if (fd >= 0 && filename != NULL)
        {
          *filename = full_path;
          break;
        }

      g_free (full_path);
    }

  g_rand_free (rand);

  g_string_free (candidate_filename, TRUE);

  return fd;
}


static gboolean
line_is_for_mime_type (const char *line,
		       const char *mime_type)
{
	if (!strncmp (line, mime_type, strlen (mime_type))) {
		const char *p;
		
		p = line + strlen (mime_type);
		while (g_ascii_isspace (*p)) {
			p++;
		}
		if (*p == '=') {
			return TRUE;
		}
	}

	return FALSE;
}

gboolean
eel_mime_set_default_application (const char *mime_type,
				  const char *id)
{
	char *applications_dir;
	char *list_filename;
	char *temp_filename;
	char *new_line;
	int list_file_fd;
	int temp_file_fd;
	GIOChannel *temp;
	GError *error = NULL;
	gboolean ret;

	if (!ensure_application_dir ()) {
		return FALSE;
	}
	
	applications_dir = get_user_dir ("applications");
	list_filename = g_build_filename (applications_dir, "defaults.list", NULL);

	temp_file_fd = open_temp_cache_file (applications_dir, 
					     &temp_filename, 
					     &error);

	g_free (applications_dir);
	if (error) {
		g_free (list_filename);
		return FALSE;
	}

	temp = g_io_channel_unix_new (temp_file_fd);
	g_io_channel_set_close_on_unref (temp, TRUE);

	ret = TRUE;
	
	list_file_fd = open (list_filename, O_RDONLY);
	if (list_file_fd > -1) {
		GIOChannel *list;
		char *line;
		gsize terminator_pos;
		gboolean have_newline;
		have_newline = FALSE;

		list = g_io_channel_unix_new (list_file_fd);
		g_io_channel_set_close_on_unref (list, TRUE);
		
		while (ret && 
		       g_io_channel_read_line (list, 
					       &line, 
					       NULL, 
					       &terminator_pos, 
					       NULL) == G_IO_STATUS_NORMAL) {
			if (line_is_for_mime_type (line, mime_type)) {
				g_free (line);
				continue;
			}
			
			ret = (g_io_channel_write_chars (temp, line, 
							 strlen (line), 
							 NULL, NULL) == G_IO_STATUS_NORMAL);
			have_newline = (line[terminator_pos] == '\n');

			g_free (line);
		}

		if (ret && !have_newline) {
			ret = (g_io_channel_write_chars (temp, "\n", 1, NULL, NULL) == G_IO_STATUS_NORMAL);
		}
		
		g_io_channel_unref (list);
	} else {
		const char *content;
		
		content = "[Default Applications]\n";

		ret = (g_io_channel_write_chars (temp, content, strlen (content), NULL, NULL) == G_IO_STATUS_NORMAL);
	}

	new_line = g_strdup_printf ("%s=%s\n", mime_type, id);
	ret = (g_io_channel_write_chars (temp, new_line, strlen (new_line), NULL, NULL) == G_IO_STATUS_NORMAL);
	g_free (new_line);

	g_io_channel_unref (temp);
	
	if (ret != FALSE) {
		if (rename (temp_filename, list_filename) < 0) {
			ret = FALSE;
			unlink (temp_filename);
		}
	} else {
		unlink (temp_filename);
	}
	
	g_free (temp_filename);
	g_free (list_filename);

	gnome_vfs_mime_reload ();

	return ret;
}

gboolean                 
eel_mime_application_is_user_owned (const char *id)
{
	gboolean ret;
	char *user_dir;
	char *path;
	
	user_dir = get_user_dir ("applications");
	path = g_build_filename (user_dir, id, NULL);

	ret = g_file_test (path, G_FILE_TEST_EXISTS);

	g_free (path);
	g_free (user_dir);

	return ret;
}

void
eel_mime_application_remove (const char *id)
{
	char *user_dir;
	char *path;
	
	user_dir = get_user_dir ("applications");
	path = g_build_filename (user_dir, id, NULL);

	unlink (path);
	
	g_free (path);
	g_free (user_dir);
	
	run_update_command ("update-desktop-database", "applications");
}

static gboolean
arg_is_exec_param (const char *param)
{
	/* Checks if the param is an exec param (eg. %f)
	 */
	if (param == NULL || param[0] != '%' ||
	    param[1] == '\0' || param [2] != '\0') {
		return FALSE;
	}

	if (param[1] == 'f') return TRUE;
	if (param[1] == 'F') return TRUE;
	if (param[1] == 'n') return TRUE;
	if (param[1] == 'N') return TRUE;
	if (param[1] == 'u') return TRUE;
	if (param[1] == 'U') return TRUE;

	return FALSE;
}

GnomeVFSMimeApplication *
eel_mime_check_for_duplicates (const char *mime_type,
			       const char *command_line)
{
	GList *applications;
	GList *list;
	gchar **argv;
	gint argc;

	applications = gnome_vfs_mime_get_all_applications (mime_type);
	if (applications == NULL) {
		return NULL;
	}

	g_shell_parse_argv (command_line, &argc, &argv, NULL);
	/* We expect the caller to have tested command_line already */
	if (argv == NULL) {
		return NULL;
	}

	/* We want to do our comparison ignoring the last argument iff it's an
	 * exec parameter.  This is because "gedit %f", "gedit" and "gedit %u"
	 * are basically the same.
	 * 
	 * It's possible that this merges apps with different behavior depending
	 * on what they're passed uri's and filenames; or if they take different
	 * command lines (eg. 'foo %f' vs 'foo --uri %U').  It's not an
	 * interesting corner case, though.
	 */
	g_assert (argc > 0);
	if (arg_is_exec_param (argv [argc - 1])) {
		argc--;
	}
	
	for (list = applications; list; list = list->next) {
		GnomeVFSMimeApplication *application;
		gchar **app_argv;
		gint app_argc;

		application = list->data;
		/* We do a parsing of the application command and see if it's
		 * identical to the command being passed in. */
		g_shell_parse_argv (application->command,
				    &app_argc, &app_argv, NULL);
		if (app_argv == NULL) {
			continue;
		}

		/* Remove the last arg if its a param.  See long comment
		 * above */
		g_assert (app_argc > 0);
		if (arg_is_exec_param (app_argv [app_argc - 1])) {
			app_argc--;
		}

		if (app_argc == argc) {
			gint i;
			gboolean different_commands = FALSE;
			for (i = 0; i < argc; i++) {
				g_assert (argv[i] != NULL && app_argv[i] != NULL);

				if (strcmp (argv[i], app_argv[i]) != 0) {
					different_commands = TRUE;
					break;
				}
			}

			if (! different_commands) {
				g_strfreev (argv);
				g_strfreev (app_argv);
				g_list_free (applications);

				return gnome_vfs_mime_application_copy (application);
			}

		}
		g_strfreev (app_argv);
	}
	g_strfreev (argv);
	g_list_free (applications);

	return NULL;
}
