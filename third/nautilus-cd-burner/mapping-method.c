/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* mapping-method.c - VFS modules for handling remapped files
 *
 *  Copyright (C) 2002 Red Hat Inc,
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 */

#include "mapping-protocol.h"
#include "mapping-method.h"
#include <glib/ghash.h>
#include <glib/glist.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/poll.h>

#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-cancellable-ops.h>

#include <libgnomevfs/gnome-vfs-method.h>
#include <libgnomevfs/gnome-vfs-module.h>
#include <libgnomevfs/gnome-vfs-module-shared.h>

int daemon_fd;
static GMutex *mapping_lock = NULL;

#define LAUNCH_DAEMON_TIMEOUT 2*1000

#undef DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define DEBUG_PRINT(x) g_print x
#else
#define DEBUG_PRINT(x) 
#endif


typedef struct {
	char *root;
	int pos;
	char **listing;
	int n_items;
	char *dirname;
	GnomeVFSFileInfoOptions options;
} VirtualDirHandle;

typedef struct {
	GnomeVFSHandle *handle;
	char *backing_file;
} VirtualFileHandle;

static gchar *
get_path_from_uri (GnomeVFSURI const *uri)
{
	gchar *path;

	path = gnome_vfs_unescape_string (uri->text, 
		G_DIR_SEPARATOR_S);
		
	if (path == NULL) {
		return NULL;
	}

	if (path[0] != G_DIR_SEPARATOR) {
		g_free (path);
		return NULL;
	}

	return path;
}

static GnomeVFSURI *
get_uri (char *path)
{
	char *text_uri;
	GnomeVFSURI *uri;
	
	g_assert (path != NULL);

	text_uri = gnome_vfs_get_uri_from_local_path (path);
	uri = gnome_vfs_uri_new (text_uri);
	g_free (text_uri);
	return uri;
}

static GnomeVFSResult
request_op (gint32 operation,
	    char *root,
	    char *path1,
	    char *path2,
	    gboolean option,
	    MappingReply *reply)
{
	int res;

	g_mutex_lock (mapping_lock);
	res = encode_request (daemon_fd,
			      operation,
			      root,
			      path1,
			      path2,
			      option);
	res = decode_reply (daemon_fd, reply);
	g_mutex_unlock (mapping_lock);

	return reply->result;
}

static GnomeVFSResult
remove_file_helper (char *method, char *path)
{
	MappingReply reply;
	GnomeVFSResult res;

	res = request_op (MAPPING_REMOVE_FILE, method,
		    path, NULL, FALSE, &reply);
	destroy_reply (&reply);
	return res;
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	GnomeVFSResult res;
	GnomeVFSURI *file_uri = NULL;
	GnomeVFSHandle *file_handle;
	VirtualFileHandle *handle;
	char *path;
	MappingReply reply;

	DEBUG_PRINT (("do_open: %s\n", gnome_vfs_uri_to_string (uri, 0)));
	
	*method_handle = NULL;
	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	res = request_op (MAPPING_GET_BACKING_FILE,
			  uri->method_string,
			  path,
			  NULL, 
			  mode & GNOME_VFS_OPEN_WRITE,
			  &reply);
	g_free (path);

		
	if (res == GNOME_VFS_OK) {
		file_uri = get_uri (reply.path);
		res = gnome_vfs_open_uri_cancellable (&file_handle,
						      file_uri, mode, context);

		if (res == GNOME_VFS_OK) {
			handle = g_new (VirtualFileHandle, 1);
			handle->handle = file_handle;
			handle->backing_file = g_strdup (reply.path);
			*method_handle = (GnomeVFSMethodHandle *)handle;
		}
		
		gnome_vfs_uri_unref (file_uri);
	}

	destroy_reply (&reply);
	
	return res;
}

static GnomeVFSResult
do_create (GnomeVFSMethod *method,
	   GnomeVFSMethodHandle **method_handle,
	   GnomeVFSURI *uri,
	   GnomeVFSOpenMode mode,
	   gboolean exclusive,
	   guint perm,
	   GnomeVFSContext *context)
{
	GnomeVFSResult res;
	GnomeVFSURI *file_uri;
	char *path;
	MappingReply reply;
	gboolean newly_created;
	GnomeVFSHandle *file_handle;
	VirtualFileHandle *handle;
	
	DEBUG_PRINT (("do_create: %s\n", gnome_vfs_uri_to_string (uri, 0)))
	
	*method_handle = NULL;
	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	res = request_op (MAPPING_CREATE_FILE,
			  uri->method_string,
			  path, NULL,
			  exclusive,
			  &reply);
	newly_created = reply.option;
	
	if (res == GNOME_VFS_OK) {
		file_uri = get_uri (reply.path);
		res = gnome_vfs_create_uri_cancellable
			(&file_handle,
			 file_uri, mode, exclusive,
			 perm, context);
		gnome_vfs_uri_unref (file_uri);
		if (res == GNOME_VFS_OK) {
			handle = g_new (VirtualFileHandle, 1);
			handle->handle = file_handle;
			handle->backing_file = g_strdup (reply.path);
			*method_handle = (GnomeVFSMethodHandle *)handle;
		}

	}
	
	destroy_reply (&reply);
	

	if (res != GNOME_VFS_OK && newly_created) {
			/* TODO: Remove the file that was created, since we failed */
			/* virtual_unlink (dir, file);
			 * virtual_node_free (file, FALSE);
			 */
	}

	return res;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)

{
	DEBUG_PRINT (("do_close: %p\n", method_handle));
	return gnome_vfs_close_cancellable (((VirtualFileHandle *)method_handle)->handle, context);
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	DEBUG_PRINT (("do_read: %p\n", method_handle));
	return gnome_vfs_read_cancellable (((VirtualFileHandle *)method_handle)->handle,
					   buffer,
					   num_bytes,
					   bytes_read,
					   context);
}

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)
{
	DEBUG_PRINT (("do_write: %p\n", method_handle));
	return gnome_vfs_write_cancellable
		(((VirtualFileHandle *)method_handle)->handle,
		 buffer,
		 num_bytes,
		 bytes_written,
		 context);

}

static GnomeVFSResult
do_seek (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSSeekPosition whence,
	 GnomeVFSFileOffset offset,
	 GnomeVFSContext *context)
{
	DEBUG_PRINT (("do_seek: %p\n", method_handle));
	return gnome_vfs_seek_cancellable
		(((VirtualFileHandle *)method_handle)->handle,
		 whence,
		 offset,
		 context);
}

static GnomeVFSResult
do_tell (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSFileOffset *offset_return)
{
	DEBUG_PRINT (("do_tell: %p\n", method_handle));
	return gnome_vfs_tell (((VirtualFileHandle *)method_handle)->handle,
			       offset_return);
}

static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize where,
		    GnomeVFSContext *context)
{
	DEBUG_PRINT (("do_truncate_handle: %p\n", method_handle));
	return gnome_vfs_truncate_handle_cancellable (((VirtualFileHandle *)method_handle)->handle,
						      where,
						      context);
}

static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   GnomeVFSContext *context)
{
	char *path;
	VirtualDirHandle *handle;
	MappingReply reply;
	GnomeVFSResult res;
	

	DEBUG_PRINT (("do_open_directory: %s\n", gnome_vfs_uri_to_string (uri, 0)));
	
	g_return_val_if_fail (uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}


	res = request_op (MAPPING_LIST_DIR,
			  uri->method_string,
			  path, NULL,
			  FALSE,
			  &reply);
	
	if (res == GNOME_VFS_OK) {
		handle = g_new (VirtualDirHandle, 1);

		handle->pos = 0;
		handle->dirname = path;
		handle->listing = reply.strings;
		g_assert ((reply.n_strings % 2) == 0);
		handle->n_items = reply.n_strings/2;
		handle->root = g_strdup (uri->method_string);
		handle->options = options;
		
		*method_handle = (GnomeVFSMethodHandle *)handle;
	} else {
		g_free (path);
	}
	
	destroy_reply (&reply);
	return res;
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
	VirtualDirHandle *handle = (VirtualDirHandle *)method_handle;
	int i;
	
	DEBUG_PRINT (("do_close_directory: %p\n", method_handle));
	for (i = 0; i < handle->n_items*2; i++) {
		g_free (handle->listing[i]);
	}
	g_free (handle->listing);
	g_free (handle->root);
	g_free (handle->dirname);
	g_free (handle);
	return GNOME_VFS_OK;
}

static void
fill_in_directory_info (GnomeVFSFileInfo *file_info)
{
	file_info->valid_fields |=
		GNOME_VFS_FILE_INFO_FIELDS_TYPE |
		GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS |
		GNOME_VFS_FILE_INFO_FIELDS_FLAGS |
		GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

	file_info->uid = getuid ();
	file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
	file_info->permissions = GNOME_VFS_PERM_USER_ALL | GNOME_VFS_PERM_OTHER_ALL;
	file_info->flags = GNOME_VFS_FILE_FLAGS_LOCAL;
	file_info->mime_type = g_strdup ("x-directory/normal");
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	VirtualDirHandle *handle = (VirtualDirHandle *)method_handle;
	GnomeVFSResult res;
	char *name;
	char *backingfile;
	char *path;
	GnomeVFSURI *file_uri;

	DEBUG_PRINT (("do_read_directory: %p\n", method_handle));

	while (TRUE) {
		if (handle->pos >= handle->n_items) {
			return GNOME_VFS_ERROR_EOF;
		}

		name = handle->listing[handle->pos*2];
		backingfile = handle->listing[handle->pos*2+1];
		++handle->pos;

		if (backingfile == NULL) {
			file_info->name = g_strdup (name);
			fill_in_directory_info (file_info);
			break;
		}

		file_uri = get_uri (backingfile);
		res = gnome_vfs_get_file_info_uri_cancellable
			(file_uri, file_info, handle->options, context);
		gnome_vfs_uri_unref (file_uri);

		if (res == GNOME_VFS_ERROR_NOT_FOUND) {
			path = g_build_filename (handle->dirname, name, NULL);
			remove_file_helper (handle->root, path);
			g_free (path);
			continue;
		}

		g_free (file_info->name);
		file_info->name = g_strdup (name);
		break;
	}
	
	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
	GnomeVFSResult res;
	GnomeVFSURI *file_uri = NULL;
	char *path;
	MappingReply reply;

	DEBUG_PRINT (("do_get_file_info: %s\n", gnome_vfs_uri_to_string (uri, 0)));
	
	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	res = request_op (MAPPING_GET_BACKING_FILE,
			  uri->method_string,
			  path,
			  NULL, 
			  FALSE,
			  &reply);
		
	if (res == GNOME_VFS_ERROR_IS_DIRECTORY) {
		file_info->name = g_path_get_basename (path);
		fill_in_directory_info (file_info);
		res = GNOME_VFS_OK;
	} else if (res == GNOME_VFS_OK) {
		file_uri = get_uri (reply.path);
		res = gnome_vfs_get_file_info_uri_cancellable
			(file_uri, file_info, options, context);
		gnome_vfs_uri_unref (file_uri);

		g_free (file_info->name);
		file_info->name = g_path_get_basename (path);
	}
	
	destroy_reply (&reply);
	g_free (path);
	
	return res;
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)
{
	GnomeVFSResult res;
	DEBUG_PRINT (("do_get_file_info_from_handle: %p\n", method_handle));
	res = gnome_vfs_get_file_info_from_handle_cancellable
		(((VirtualFileHandle *)method_handle)->handle,
		 file_info, options, context);
	/* TODO: Need to fill out the real name here. Need to wrap method_handle */
	return res;
}

static GnomeVFSResult
do_make_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *uri,
		   guint perm,
		   GnomeVFSContext *context)
{
	GnomeVFSResult res;
	char *path;
	MappingReply reply;

	DEBUG_PRINT (("do_make_directory: %s\n", gnome_vfs_uri_to_string (uri, 0)));
	
	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	
	res = request_op (MAPPING_CREATE_DIR,
			  uri->method_string,
			  path,
			  NULL, FALSE,
			  &reply);
	g_free (path);
	destroy_reply (&reply);

	return res;
}

static GnomeVFSResult
do_remove_directory (GnomeVFSMethod *method,
		     GnomeVFSURI *uri,
		     GnomeVFSContext *context)
{
	GnomeVFSResult res;
	char *path;
	MappingReply reply;

	DEBUG_PRINT (("do_remove_directory: %s\n", gnome_vfs_uri_to_string (uri, 0)));
	
	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	
	res = request_op (MAPPING_REMOVE_DIR,
			  uri->method_string,
			  path,
			  NULL, FALSE,
			  &reply);
	g_free (path);
	destroy_reply (&reply);

	return res;
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod *method,
	   GnomeVFSURI *uri,
	   GnomeVFSContext *context)
{
	GnomeVFSResult res;
	char *path;

	DEBUG_PRINT (("do_remove_directory: %s\n", gnome_vfs_uri_to_string (uri, 0)));
	
	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	res = remove_file_helper (uri->method_string, path);
	g_free (path);

	return res;
}

static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{
	GnomeVFSResult res;
	MappingReply reply;
	char *old_path;
	char *new_path;

	DEBUG_PRINT(("do_move (%s, %s)\n", gnome_vfs_uri_to_string (old_uri, 0), gnome_vfs_uri_to_string (new_uri, 0)));
	
	if (strcmp (new_uri->method_string,
		    old_uri->method_string) != 0) {
		return GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM;
	}
		
	old_path = get_path_from_uri (old_uri);
	if (old_path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	new_path = get_path_from_uri (new_uri);
	if (new_path == NULL) {
		g_free (old_path);
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	res = request_op (MAPPING_MOVE_FILE,
			  old_uri->method_string,
			  old_path,
			  new_path,
			  FALSE,
			  &reply);
	destroy_reply (&reply);
	g_free (old_path);
	g_free (new_path);
	
	return res;
}

static gboolean
do_is_local (GnomeVFSMethod *method,
	     const GnomeVFSURI *uri)
{
	return TRUE;
}

/* When checking whether two locations are on the same file system, we are
   doing this to determine whether we can recursively move or do other
   sorts of transfers.  When a symbolic link is the "source", its
   location is the location of the link file, because we want to
   know about transferring the link, whereas for symbolic links that
   are "targets", we use the location of the object being pointed to,
   because that is where we will be moving/copying to. */
static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod *method,
		  GnomeVFSURI *source_uri,
		  GnomeVFSURI *target_uri,
		  gboolean *same_fs_return,
		  GnomeVFSContext *context)
{
	DEBUG_PRINT (("check_same_fs (%s, %s)\n", gnome_vfs_uri_to_string (source_uri, 0), gnome_vfs_uri_to_string (target_uri, 0)));
	*same_fs_return = strcmp (source_uri->method_string,  target_uri->method_string) == 0;
	return GNOME_VFS_OK;
}


static GnomeVFSResult
do_set_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  const GnomeVFSFileInfo *info,
		  GnomeVFSSetFileInfoMask mask,
		  GnomeVFSContext *context)
{
	gchar *full_name;
	GnomeVFSResult res;
	MappingReply reply;
	GnomeVFSURI *file_uri = NULL;
	
	DEBUG_PRINT (("do_set_file_info: %s\n", gnome_vfs_uri_to_string (uri, 0)));

	full_name = get_path_from_uri (uri);
	if (full_name == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	if (mask & GNOME_VFS_SET_FILE_INFO_NAME) {
		gchar *dir, *encoded_dir;
		gchar *new_name;

		encoded_dir = gnome_vfs_uri_extract_dirname (uri);
		dir = gnome_vfs_unescape_string (encoded_dir, G_DIR_SEPARATOR_S);
		g_free (encoded_dir);
		g_assert (dir != NULL);

		/* FIXME bugzilla.eazel.com 645: This needs to return
		 * an error for incoming names with "/" characters in
		 * them, instead of moving the file.
		 */

		if (dir[strlen(dir) - 1] != '/') {
			new_name = g_strconcat (dir, "/", info->name, NULL);
		} else {
			new_name = g_strconcat (dir, info->name, NULL);
		}

		res = request_op (MAPPING_MOVE_FILE,
				  uri->method_string,
				  full_name,
				  new_name,
				  FALSE,
				  &reply);
		destroy_reply (&reply);

		g_free (dir);
		
		g_free (full_name);
		full_name = new_name;

		if (res != GNOME_VFS_OK) {
			g_free (full_name);
			return res;
		}
		mask = mask & ~GNOME_VFS_SET_FILE_INFO_NAME;
	}

	if (mask != 0) {
		res = request_op (MAPPING_GET_BACKING_FILE,
				  uri->method_string,
				  full_name,
				  NULL, 
				  TRUE,
				  &reply);
		g_free (full_name);
		if (res != GNOME_VFS_OK) {
			destroy_reply (&reply);
			return res;
		}

		file_uri = get_uri (reply.path);
		destroy_reply (&reply);
		
		res = gnome_vfs_set_file_info_cancellable (file_uri,
							   info, mask, context);
		gnome_vfs_uri_unref (file_uri);
	}

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_truncate (GnomeVFSMethod *method,
	     GnomeVFSURI *uri,
	     GnomeVFSFileSize where,
	     GnomeVFSContext *context)
{
	GnomeVFSResult res;
	GnomeVFSURI *file_uri = NULL;
	char *path;
	MappingReply reply;

	DEBUG_PRINT (("do_truncate: %s\n", gnome_vfs_uri_to_string (uri, 0)));
	
	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	res = request_op (MAPPING_GET_BACKING_FILE,
			  uri->method_string,
			  path,
			  NULL, 
			  TRUE,
			  &reply);

		
	if (res == GNOME_VFS_OK) {
		file_uri = get_uri (reply.path);
		res = gnome_vfs_truncate_uri_cancellable (file_uri, where, context);
		gnome_vfs_uri_unref (file_uri);
	}
	destroy_reply (&reply);
	
	return res;
}

static GnomeVFSResult
do_create_symbolic_link (GnomeVFSMethod *method,
			 GnomeVFSURI *uri,
			 const char *target_reference,
			 GnomeVFSContext *context)
{
	GnomeVFSResult res;
	MappingReply reply;
	char *path;
	char *target_path;
	
	DEBUG_PRINT (("do_create_symbolic_link: %s -> %s\n", gnome_vfs_uri_to_string (uri, 0), target_reference));
	
	path = get_path_from_uri (uri);
	if (path == NULL) {
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	target_path = gnome_vfs_get_local_path_from_uri (target_reference);

	if (target_path == NULL) {
		g_free (path);
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}
	
	res = request_op (MAPPING_CREATE_LINK,
			  uri->method_string,
			  path,
			  target_path,
			  FALSE,
			  &reply);
	destroy_reply (&reply);
	g_free (target_path);
	g_free (path);

	return res;
}

static GnomeVFSResult
do_file_control (GnomeVFSMethod *method,
		 GnomeVFSMethodHandle *method_handle,
		 const char *operation,
		 gpointer operation_data,
		 GnomeVFSContext *context)
{
	VirtualFileHandle *handle;

	handle = (VirtualFileHandle *)method_handle;

	if (strcmp (operation, "mapping:get_mapping") == 0) {
		*(char **)operation_data = g_strdup (handle->backing_file);
		return GNOME_VFS_OK;
	}
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}


static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	do_seek,
	do_tell,
	do_truncate_handle,
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	do_get_file_info_from_handle,
	do_is_local,
	do_make_directory,
	do_remove_directory,
	do_move,
	do_unlink,
	do_check_same_fs,
	do_set_file_info,
	do_truncate,
	NULL, /* do_find_directory */
	do_create_symbolic_link,
	NULL, /* do_monitor_add */
	NULL, /* do_monitor_cancel */
	do_file_control
};

static void
daemon_child_setup (gpointer user_data)
{ 
	gint open_max;
	gint i;
	int *pipes = user_data;
      
	close (pipes[0]);
	dup2 (pipes[1], 3);
	
	open_max = sysconf (_SC_OPEN_MAX);
	for (i = 4; i < open_max; i++) {
		fcntl (i, F_SETFD, FD_CLOEXEC);
	}
}

static gboolean
launch_daemon (void)
{
	GError *error;
	gint pipes[2];
	gchar *argv[] = {
		LIBEXECDIR "/mapping-daemon",
		NULL
	};
	struct pollfd pollfd;
	char c;

	if (pipe(pipes) != 0) {
		g_warning ("pipe failure");
		return FALSE;
	}

	error = NULL;
	if (!g_spawn_async (NULL,
			    argv, NULL,
			    G_SPAWN_LEAVE_DESCRIPTORS_OPEN,
			    &daemon_child_setup, pipes,
			    NULL,
			    &error)) {
		g_warning ("Couldn't launch mapping-daemon: %s\n",
			   error->message);
		g_error_free (error);
		return FALSE;
	}
	
	close (pipes[1]);

	pollfd.fd = pipes[0];
	pollfd.events = POLLIN; 
	pollfd.revents = 0;

	if ( poll (&pollfd, 1, LAUNCH_DAEMON_TIMEOUT) != 1) {
		g_warning ("Didn't get any signs from mapping-daemon\n");
		return FALSE;
	}
	read (pipes[0], &c, 1);
	close (pipes[0]);
	
	return TRUE;
}


GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
        struct sockaddr_un sin;

	sin.sun_family = AF_UNIX;
	g_snprintf (sin.sun_path, sizeof(sin.sun_path), "%s/mapping-%s", g_get_tmp_dir (), g_get_user_name ());

        if ((daemon_fd = socket (AF_UNIX, SOCK_STREAM, 0)) == -1) {
                perror("mapping method init - socket");
                return NULL;
        }
	
        if (connect (daemon_fd, (const struct sockaddr *) &sin, sizeof(sin)) == -1) {
		if (errno == ECONNREFUSED || errno == ENOENT) {
			if (launch_daemon ()) {
				if (connect (daemon_fd,  (const struct sockaddr *) &sin, sizeof(sin)) == -1) {
					perror("mapping method init - connect2");
					return NULL;
				}
			} else {
				return NULL;
			}
		} else {
			perror("mapping method init - connect");
			return NULL;
		}
        }

	mapping_lock = g_mutex_new();
	
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	close (daemon_fd);
	g_mutex_free (mapping_lock);
}
