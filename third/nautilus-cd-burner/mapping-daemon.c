/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   mapping-daemon.c: central daemon to keep track of file mappings
 
   Copyright (C) 2002 Red Hat, Inc.
  
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
#include <config.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <glib/glist.h>
#include <libgnomevfs/gnome-vfs.h>

#ifndef HAVE_MKDTEMP
#include "mkdtemp.h"
#endif

#include "mapping-daemon.h"
#include "mapping-protocol.h"

/* Global daemon */
GHashTable *roots;
int n_clients = 0;
time_t last_disconnect = 0;

#define KEEP_DATA_TIME 60*60*2

typedef enum {
	VIRTUAL_NODE_FILE,
	VIRTUAL_NODE_DIRECTORY
} VirtualNodeType;

typedef struct {
	char *filename;
	VirtualNodeType type;
	
	/* for files: */
	char *backing_file; /* local filename */
	gboolean owned_file;
	
	/* for directories: */
	GList *children;
} VirtualNode;

typedef struct {
	char *method;
	char *tempdir;
	VirtualNode *root_node;
} VirtualRoot;

static VirtualNode *
virtual_node_new (const char *filename, VirtualNodeType type)
{
	VirtualNode *node;

	node = g_new0 (VirtualNode, 1);
	node->filename = g_strdup (filename);
	node->type = type;
	
	return node;
}


static void
virtual_node_free (VirtualNode *node, gboolean deep)
{
	GList *l;
	
	g_free (node->filename);
	switch (node->type) {
	case VIRTUAL_NODE_FILE:
		if (node->backing_file) {
			if (node->owned_file) {
				unlink (node->backing_file);
			}
			g_free (node->backing_file);
		}
		break;
	case VIRTUAL_NODE_DIRECTORY:
		if (deep) {
			for (l = node->children; l != NULL; l = l->next) {
				virtual_node_free ((VirtualNode *)l->data, TRUE);
			}
		}
		break;
	}
	g_free (node);
}


static void
virtual_root_free (VirtualRoot *root)
{
	virtual_node_free (root->root_node, TRUE);
	g_free (root->method);
	rmdir (root->tempdir);
	g_free (root->tempdir);
}

static VirtualRoot *
virtual_root_new (const char *method)
{
	VirtualRoot *root;
	char *tempdir, *filename;
	char *dir;

	filename = g_strdup_printf ("virtual-%s.XXXXXX", g_get_user_name ());
	tempdir = g_build_filename (g_get_tmp_dir(), filename, NULL);
	g_free (filename);

	root = g_new (VirtualRoot, 1);
	
	dir = mkdtemp (tempdir);
	
	if (dir == NULL) {
		g_free (tempdir);
		g_free (root);
		root = NULL;
		return NULL;
	}
	
	root->method = g_strdup (method);
	root->tempdir = g_strdup (dir);
	root->root_node = virtual_node_new (NULL, VIRTUAL_NODE_DIRECTORY);

	g_free (tempdir);
	
	return root;
}



static VirtualRoot *
lookup_root (const char *method,
	     gboolean create)
{
	VirtualRoot *root;
	
	root = g_hash_table_lookup (roots, method);

	if (root == NULL && create) {
		root = virtual_root_new (method);
		g_hash_table_replace (roots,
				      root->method,
				      root);
	}
	
	return root;
}

static VirtualNode *
virtual_dir_lookup (VirtualNode *dir, const char *filename)
{
	GList *l;
	VirtualNode *node;
	
	g_assert (dir->type == VIRTUAL_NODE_DIRECTORY);

	for (l = dir->children; l != NULL; l = l->next) {
		node = (VirtualNode *)l->data;
		if (strcmp (node->filename, filename) == 0) {
			return node;
		}
	}
	
	return NULL;
}

static VirtualNode *
virtual_node_lookup (VirtualRoot *root, const char *path, VirtualNode **parent)
{
	gchar *copy, *next;
	VirtualNode *node;

	copy = g_strdup (path);

	if (parent != NULL) {
		*parent = NULL;
	}
	node = root->root_node;
	while (copy != NULL) {
		/* Skip initial/multiple slashes */
		while (*copy == '/') {
			++copy;
		}

		if (*copy == 0) {
			break;
		}
		
		next = strchr (copy, '/');
		if (next) {
			*next = 0;
			next++;
		}

		if (node->type != VIRTUAL_NODE_DIRECTORY) {
			/* Found a file in the middle of the path */
			node = NULL;
			break;
		}
			
		if (parent != NULL) {
			*parent = node;
		}
		node = virtual_dir_lookup (node, copy);
		if (node == NULL) {
			break;
		}

		copy = next;
	}
	
	return node;
}

static VirtualNode *
virtual_mkdir (VirtualNode *node, const char *name)
{
	VirtualNode *subdir;

	g_assert (node->type == VIRTUAL_NODE_DIRECTORY);

	if (virtual_dir_lookup (node, name) != NULL) {
		return NULL;
	}
	
	subdir = virtual_node_new (name, VIRTUAL_NODE_DIRECTORY);
	node->children = g_list_append (node->children, subdir);

	return subdir;
}

static void
virtual_unlink (VirtualNode *dir, VirtualNode *node)
{
	g_assert (dir->type == VIRTUAL_NODE_DIRECTORY);
	
	dir->children = g_list_remove (dir->children, node);
}

static VirtualNode *
virtual_create (VirtualRoot *root, VirtualNode *dir,
		const char *name, const char *backing_file)
{
	VirtualNode *file;
	char *template;
	int fd;

	g_assert (dir->type == VIRTUAL_NODE_DIRECTORY);

	if (virtual_dir_lookup (dir, name) != NULL) {
		return NULL;
	}
	
	file = virtual_node_new (name, VIRTUAL_NODE_FILE);

	if (backing_file != NULL) {
		file->backing_file = g_strdup (backing_file);
		file->owned_file = FALSE;
	} else {
		template = g_build_filename (root->tempdir, "file.XXXXXX", NULL);

		fd = g_mkstemp (template);

		if (fd < 0) {
			g_free (template);
			virtual_node_free (file, FALSE);
			return NULL;
		}
		close (fd);
		unlink (template);
		
		file->backing_file = template;
		file->owned_file = TRUE;
	}

	dir->children = g_list_append (dir->children, file);

	return file;
}

static GnomeVFSResult
get_backing_file (const char *rootname,
		  const char *path,
		  const gboolean write,
		  char **backing_file)
{
	VirtualRoot *root;
	VirtualNode *node;

	root = lookup_root (rootname, TRUE);
	
	*backing_file = NULL;
	node = virtual_node_lookup (root, path, NULL);
	if (node == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	if (node->type != VIRTUAL_NODE_FILE) {
		return GNOME_VFS_ERROR_IS_DIRECTORY;
	}
	if (write && !node->owned_file) {
		return GNOME_VFS_ERROR_READ_ONLY;
	}
	
	*backing_file = node->backing_file;
	
	return GNOME_VFS_OK;
}

static GnomeVFSResult
create_dir (const char *rootname,
	    const char *path)
{
	VirtualRoot *root;
	char *dirname;
	char *basename;
	VirtualNode *dir;
	VirtualNode *file;

	root = lookup_root (rootname, TRUE);

	dirname = g_path_get_dirname (path);
	
	dir = virtual_node_lookup (root, dirname, NULL);
	g_free (dirname);
	if (dir == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	
	basename = g_path_get_basename (path);
	file = virtual_dir_lookup (dir, basename);
	if (file != NULL) {
		g_free (basename);
		return GNOME_VFS_ERROR_FILE_EXISTS;
	}

	file = virtual_mkdir (dir, basename);
	g_free (basename);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
remove_dir (const char *rootname,
	    const char *path)
{
	VirtualRoot *root;
	VirtualNode *dir;
	VirtualNode *file;
	
	root = lookup_root (rootname, TRUE);

	file = virtual_node_lookup (root, path, &dir);
	if (file == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	if (dir == NULL) {
		/* Don't allow you to remove root */
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	
	if (file->type != VIRTUAL_NODE_DIRECTORY) {
		return GNOME_VFS_ERROR_NOT_A_DIRECTORY;
	}

	if (file->children != NULL) {
		return GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY;
	}

	virtual_unlink (dir, file);
	virtual_node_free (file, FALSE);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
remove_file (const char *rootname,
	     const char *path)
{
	VirtualRoot *root;
	VirtualNode *dir;
	VirtualNode *file;
	
	root = lookup_root (rootname, TRUE);

	file = virtual_node_lookup (root, path, &dir);
	if (file == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	
	if (file->type != VIRTUAL_NODE_FILE) {
		return GNOME_VFS_ERROR_IS_DIRECTORY;
	}

	virtual_unlink (dir, file);
	virtual_node_free (file, FALSE);

	return GNOME_VFS_OK;
}



static GnomeVFSResult
create_file (const char *rootname,
	     const char *path,
	     const gboolean exclusive,
	     char **backing_file,
	     gboolean *newly_created)
{
	VirtualRoot *root;
	char *dirname;
	char *basename;
	VirtualNode *dir;
	VirtualNode *file;
	
	*backing_file = NULL;
	*newly_created = FALSE;
	
	root = lookup_root (rootname, TRUE);

	dirname = g_path_get_dirname (path);
	
	dir = virtual_node_lookup (root, dirname, NULL);
	g_free (dirname);
	if (dir == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	
	basename = g_path_get_basename (path);
	file = virtual_dir_lookup (dir, basename);
	if (exclusive && file != NULL) {
		g_free (basename);
		return GNOME_VFS_ERROR_FILE_EXISTS;
	}

	*newly_created = FALSE;
	if (file == NULL) {
		file = virtual_create (root, dir, basename, NULL);
		*newly_created = TRUE;
	}
	
	g_free (basename);

	if (file == NULL) {
		return GNOME_VFS_ERROR_NO_SPACE;
	}

	*backing_file = file->backing_file;

	return GNOME_VFS_OK;
}

static void
create_dir_link (VirtualRoot *root,
		 VirtualNode *parent,
		 char *dirname,
		 const char *target_path)
{
	VirtualNode *dir;
	DIR *d;
	struct dirent *dirent;
	char *path;
	struct stat stat_buf;

	dir = virtual_mkdir (parent, dirname);
	if (dir == NULL) {
		return;
	}

	d = opendir (target_path);
	if (d == NULL) {
		return;
	}

	while ((dirent = readdir (d)) != NULL) {
		if (strcmp (dirent->d_name, ".") == 0 ||
		    strcmp (dirent->d_name, "..") == 0) {
			continue;
		}
		
		path = g_build_filename (target_path, dirent->d_name, NULL);

		if (stat (path, &stat_buf) == 0) {
			if (S_ISDIR (stat_buf.st_mode)) {
				create_dir_link (root, dir, dirent->d_name, path);
			} else {
				virtual_create (root, dir, dirent->d_name, path);
			}
		}
		g_free (path);
	}

	closedir (d);
}

static GnomeVFSResult
create_link (const char *rootname,
	     const char *path,
	     const char *target_path)
{
	VirtualRoot *root;
	char *dirname;
	char *basename;
	VirtualNode *dir;
	VirtualNode *file;
	struct stat stat_buf;
	
	root = lookup_root (rootname, TRUE);

	dirname = g_path_get_dirname (path);
	
	dir = virtual_node_lookup (root, dirname, NULL);
	g_free (dirname);
	if (dir == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	
	basename = g_path_get_basename (path);
	
	file = virtual_dir_lookup (dir, basename);
	if (file != NULL) {
		g_free (basename);
		return GNOME_VFS_ERROR_FILE_EXISTS;
	}

	if (stat (target_path, &stat_buf) < 0) {
		g_free (basename);
		return GNOME_VFS_ERROR_NOT_FOUND;
	}

	if (S_ISDIR (stat_buf.st_mode)) {
		create_dir_link (root, dir, basename, target_path);
	} else {
		file = virtual_create (root, dir, basename, target_path);
		g_free (basename);
		
		if (file == NULL) {
			return GNOME_VFS_ERROR_NO_SPACE;
		}
	}

	return GNOME_VFS_OK;
}


static GnomeVFSResult
move_file (const char *rootname,
	   const char *old_path,
	   const char *new_path)
{
	VirtualRoot *root;
	char *dirname;
	VirtualNode *old_node, *new_node;
	VirtualNode *old_dir, *new_dir;
	
	root = lookup_root (rootname, TRUE);

	old_node = virtual_node_lookup (root, old_path, &old_dir);
	if (old_node == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}

	new_node = virtual_node_lookup (root, new_path, &new_dir);
	if (new_node != NULL) {
		if (new_node->type == VIRTUAL_NODE_DIRECTORY &&
		    old_node->type != VIRTUAL_NODE_DIRECTORY) {
			return GNOME_VFS_ERROR_IS_DIRECTORY;
		}
		if (new_node->type == VIRTUAL_NODE_DIRECTORY &&
		    new_node->children != NULL) {
			return GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY;
		}
		g_free (old_node->filename);
		old_node->filename = g_strdup (new_node->filename);
		virtual_unlink (new_dir, new_node);
		virtual_unlink (old_dir, old_node);
		new_dir->children = g_list_append (new_dir->children, old_node);
		virtual_node_free (new_node, FALSE);
	} else {
		dirname = g_path_get_dirname (new_path);
		new_dir = virtual_node_lookup (root, dirname, NULL);
		g_free (dirname);
		if (new_dir == NULL) {
			return GNOME_VFS_ERROR_NOT_FOUND;
		}
		g_free (old_node->filename);
		old_node->filename = g_path_get_basename (new_path);
		virtual_unlink (old_dir, old_node);
		new_dir->children = g_list_append (new_dir->children, old_node);
	}

	return GNOME_VFS_OK;
	
}

static GnomeVFSResult
list_dir (const char *rootname,
	  const char *path,
	  int *n_elements,
	  char ***listing_out)
{
	VirtualRoot *root;
	VirtualNode *node;
	int len, i;
	GList *l;
	char **listing;

	*n_elements = 0;
	
	root = lookup_root (rootname, TRUE);
	
	node = virtual_node_lookup (root, path, NULL);
	if (node == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}
	if (node->type != VIRTUAL_NODE_DIRECTORY) {
		return GNOME_VFS_ERROR_NOT_A_DIRECTORY;
	}

	len = g_list_length (node->children);
	listing = g_new (char *, len * 2);
	*listing_out = listing;
	*n_elements = len * 2;

	for (i=0, l = node->children; l != NULL; l = l->next, i++) {
		node = (VirtualNode *)l->data;

		listing[i*2] = node->filename;
		if (node->type == VIRTUAL_NODE_FILE) {
			listing[i*2 + 1] = node->backing_file;
		} else {
			listing[i*2 + 1] = NULL;
		}
	}
	
	return GNOME_VFS_OK;
}


static void
init_roots (void)
{
	roots = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
client_connected (void)
{
	n_clients++;
}

static void
client_died (void)
{
	n_clients--;
	last_disconnect = time (NULL);
}


static gboolean
handle_request (GIOChannel   *source,
		GIOCondition  condition,
		gpointer      data)
{
	int fd = g_io_channel_unix_get_fd (source);
	int res;
	MappingRequest req;
	MappingReply reply;
	
	if (condition & G_IO_ERR) {
		g_warning ("handle_request_error\n");
		client_died ();
		return FALSE;
	}
	
	res = decode_request (fd, &req);

	if (res != 0) {
		client_died ();
		return FALSE;
	}

	memset (&reply, 0, sizeof (reply));

	switch (req.operation) {
	case MAPPING_GET_BACKING_FILE:
		reply.result = get_backing_file (req.root,
						 req.path1,
						 req.option,
						 &reply.path);
		break;
	case MAPPING_LIST_DIR:
		reply.result = list_dir (req.root,
					 req.path1,
					 &reply.n_strings,
					 &reply.strings);
		break;
	case MAPPING_CREATE_DIR:
		reply.result = create_dir (req.root,
					   req.path1);
		break;
	case MAPPING_REMOVE_DIR:
		reply.result = remove_dir (req.root,
					   req.path1);
		break;
	case MAPPING_REMOVE_FILE:
		reply.result = remove_file (req.root,
					    req.path1);
		break;
	case MAPPING_CREATE_FILE:
		reply.result = create_file (req.root,
					    req.path1,
					    req.option,
					    &reply.path,
					    &reply.option);
		break;
	case MAPPING_CREATE_LINK:
		reply.result = create_link (req.root,
					    req.path1,
					    req.path2);
		break;
	case MAPPING_MOVE_FILE:
		reply.result = move_file (req.root,
					  req.path1,
					  req.path2);
		break;
	default:
		g_warning ("Unimplemented op %d\n", req.operation);
		break;
	}

	destroy_request (&req);
	
	res = encode_reply (fd, &reply);
	destroy_reply (&reply);
	if (res != 0) {
		client_died ();
		return FALSE;
	}
	
	return TRUE;
}

static gboolean
handle_new_client (GIOChannel   *source,
		   GIOCondition  condition,
		   gpointer      data)
{
	GIOChannel *io;
	int master_fd = g_io_channel_unix_get_fd (source);
	int client;

	/* wait for a client to talk to us */
        if ((client = accept (master_fd, 0, 0)) == -1) {
                g_warning ("accept failed");
		return TRUE;
        }
	io = g_io_channel_unix_new (client);
	g_io_add_watch (io, G_IO_IN | G_IO_ERR, handle_request, NULL);
	g_io_channel_unref (io);

	client_connected ();
	
	return TRUE;
}

static void
have_data_helper (gpointer       key,
		  gpointer       value,
		  gpointer       user_data)
{
	VirtualRoot *root = value;
	gboolean *res = user_data;

	if (root->root_node->children != NULL) {
		*res = TRUE;
	}
}

static gboolean
have_data (void)
{
	gboolean res = FALSE;
	
	g_hash_table_foreach (roots,
			      have_data_helper, &res);

	return res;
}

static gboolean
free_roots_helper (gpointer       key,
		   gpointer       value,
		   gpointer       user_data)
{
	VirtualRoot *root = value;

	virtual_root_free (root);
	return TRUE;
}

static void
free_roots (void)
{
	g_hash_table_foreach_remove (roots,
				     free_roots_helper, NULL);
}


static gboolean
cleanup_timeout (gpointer data)
{
	if (n_clients == 0 ) {
		if (have_data ()) {
			time_t now;
			now = time (NULL);
			if (now - last_disconnect > KEEP_DATA_TIME) {
				free_roots ();
				exit (0);
			}
		} else {
			free_roots ();
			exit (0);
		}
	}
	
	return TRUE;
}

int
main (int argc, char *argv[])
{
        struct sockaddr_un sin;
	int master_socket;
	GIOChannel *master_io;
	GMainLoop *loop;
	int pipe_fd;

	/* See if we have a valid pipe to write to */
	pipe_fd = dup (3);
	close (3);

	init_roots();
	
	if ((master_socket = socket (AF_UNIX, SOCK_STREAM, 0)) == -1) {
                perror ("Master socket creation failed");
                exit(1);
        }
	
	sin.sun_family = AF_UNIX;
	g_snprintf (sin.sun_path, sizeof(sin.sun_path), "%s/mapping-%s", g_get_tmp_dir (), g_get_user_name ());
	unlink (sin.sun_path);
	if (bind (master_socket, (const struct sockaddr *)&sin, sizeof(sin)) == -1) {
                perror ("Failed to bind master socket");
                exit(1);
        }
	if (listen (master_socket, 5) == -1) {
		perror ("Failed to listen to master socket");
                exit(1);
        }

	/* Trigger launching app */
	if (pipe_fd >= 0) {
	  write (pipe_fd, "G", 1);
	  close (pipe_fd);
	}
	
	master_io = g_io_channel_unix_new (master_socket);
	g_io_add_watch (master_io, G_IO_IN, handle_new_client, NULL);
	g_io_channel_unref (master_io);

	/* Don't hold up the cwd, allowing unmounts of e.g. /home
	 * since we run a while after logout.
	 */
	chdir ("/");

	loop = g_main_loop_new (NULL, TRUE);

	g_timeout_add (5000, &cleanup_timeout, NULL);

	g_main_loop_run (loop);
	
	free_roots ();
	return 0;
}
