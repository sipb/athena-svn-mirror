/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-async-ops.c - Asynchronous operations supported by the
   GNOME Virtual File System (version for POSIX threads).

   Copyright (C) 1999 Free Software Foundation

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

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#include "gnome-vfs-job.h"

void           pthread_gnome_vfs_async_cancel                 (GnomeVFSAsyncHandle                 *handle);
void           pthread_gnome_vfs_async_open_uri               (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSOpenMode                     open_mode,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_open                   (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSOpenMode                     open_mode,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_open_uri_as_channel    (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSOpenMode                     open_mode,
							       guint                                advised_block_size,
							       GnomeVFSAsyncOpenAsChannelCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_open_as_channel        (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSOpenMode                     open_mode,
							       guint                                advised_block_size,
							       GnomeVFSAsyncOpenAsChannelCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_create_uri             (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSOpenMode                     open_mode,
							       gboolean                             exclusive,
							       guint                                perm,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_create_symbolic_link   (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       const gchar                         *uri_reference,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_create                 (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSOpenMode                     open_mode,
							       gboolean                             exclusive,
							       guint                                perm,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_create_as_channel      (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSOpenMode                     open_mode,
							       gboolean                             exclusive,
							       guint                                perm,
							       GnomeVFSAsyncOpenAsChannelCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_close                  (GnomeVFSAsyncHandle                 *handle,
							       GnomeVFSAsyncCloseCallback           callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_read                   (GnomeVFSAsyncHandle                 *handle,
							       gpointer                             buffer,
							       guint                                bytes,
							       GnomeVFSAsyncReadCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_write                  (GnomeVFSAsyncHandle                 *handle,
							       gconstpointer                        buffer,
							       guint                                bytes,
							       GnomeVFSAsyncWriteCallback           callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_get_file_info          (GnomeVFSAsyncHandle                **handle_return,
							       GList                               *uris,
							       GnomeVFSFileInfoOptions              options,
							       GnomeVFSAsyncGetFileInfoCallback     callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_set_file_info          (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSFileInfo                    *info,
							       GnomeVFSSetFileInfoMask              mask,
							       GnomeVFSFileInfoOptions              options,
							       GnomeVFSAsyncSetFileInfoCallback     callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_load_directory         (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSFileInfoOptions              options,
							       GnomeVFSDirectorySortRule            sort_rules[],
							       gboolean                             reverse_order,
							       GnomeVFSDirectoryFilterType          filter_type,
							       GnomeVFSDirectoryFilterOptions       filter_options,
							       const gchar                         *filter_pattern,
							       guint                                items_per_notification,
							       GnomeVFSAsyncDirectoryLoadCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_load_directory_uri     (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSFileInfoOptions              options,
							       GnomeVFSDirectorySortRule            sort_rules[],
							       gboolean                             reverse_order,
							       GnomeVFSDirectoryFilterType          filter_type,
							       GnomeVFSDirectoryFilterOptions       filter_options,
							       const gchar                         *filter_pattern,
							       guint                                items_per_notification,
							       GnomeVFSAsyncDirectoryLoadCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_find_directory         (GnomeVFSAsyncHandle                **handle_return,
							       GList                               *uris,
							       GnomeVFSFindDirectoryKind            kind,
							       gboolean                             create_if_needed,
							       gboolean                             find_if_needed,
							       guint                                permissions,
							       GnomeVFSAsyncFindDirectoryCallback   callback,
							       gpointer                             user_data);
GnomeVFSResult pthread_gnome_vfs_async_xfer                   (GnomeVFSAsyncHandle                **handle_return,
							       GList                               *source_uri_list,
							       GList                               *target_uri_list,
							       GnomeVFSXferOptions                  xfer_options,
							       GnomeVFSXferErrorMode                error_mode,
							       GnomeVFSXferOverwriteMode            overwrite_mode,
							       GnomeVFSAsyncXferProgressCallback    progress_update_callback,
							       gpointer                             update_callback_data,
							       GnomeVFSXferProgressCallback         progress_sync_callback,
							       gpointer                             sync_callback_data);
guint          pthread_gnome_vfs_async_add_status_callback    (GnomeVFSAsyncHandle                 *handle,
							       GnomeVFSStatusCallback               callback,
							       gpointer                             user_data);
void           pthread_gnome_vfs_async_remove_status_callback (GnomeVFSAsyncHandle                 *handle,
							       guint                                callback_id);

void
pthread_gnome_vfs_async_cancel (GnomeVFSAsyncHandle *handle)
{
	GnomeVFSJob *job;

	g_return_if_fail (handle != NULL);

	job = (GnomeVFSJob *) handle;

	/* FIXME bugzilla.eazel.com 1129: This does not free the
         * handle! Storage leak!
	 */
	gnome_vfs_job_cancel (job);
}



static GnomeVFSAsyncHandle *
async_open (GnomeVFSURI *uri,
	    GnomeVFSOpenMode open_mode,
	    GnomeVFSAsyncOpenCallback callback,
	    gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSOpenOp *open_op;

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_OPEN,
			       (GFunc) callback, callback_data);
	
	open_op = &job->current_op->specifics.open;
	
	open_op->request.uri = uri == NULL ? NULL : gnome_vfs_uri_ref (uri);
	open_op->request.open_mode = open_mode;

	gnome_vfs_job_go (job);

	return (GnomeVFSAsyncHandle *) job;
}

void
pthread_gnome_vfs_async_open_uri (GnomeVFSAsyncHandle **handle_return,
				  GnomeVFSURI *uri,
				  GnomeVFSOpenMode open_mode,
				  GnomeVFSAsyncOpenCallback callback,
				  gpointer callback_data)
{
	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);
	
	*handle_return = async_open (uri, open_mode,
				     callback, callback_data);
}

void
pthread_gnome_vfs_async_open (GnomeVFSAsyncHandle **handle_return,
			      const gchar *text_uri,
			      GnomeVFSOpenMode open_mode,
			      GnomeVFSAsyncOpenCallback callback,
			      gpointer callback_data)
{
	GnomeVFSURI *uri;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	uri = gnome_vfs_uri_new (text_uri);
	*handle_return = async_open (uri, open_mode,
				     callback, callback_data);
	if (uri != NULL) {
		gnome_vfs_uri_unref (uri);
	}
}

static GnomeVFSAsyncHandle *
async_open_as_channel (GnomeVFSURI *uri,
		       GnomeVFSOpenMode open_mode,
		       guint advised_block_size,
		       GnomeVFSAsyncOpenAsChannelCallback callback,
		       gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSOpenAsChannelOp *open_as_channel_op;

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_OPEN_AS_CHANNEL,
			       (GFunc) callback, callback_data);

	open_as_channel_op = &job->current_op->specifics.open_as_channel;
	open_as_channel_op->request.uri = uri == NULL ? NULL : gnome_vfs_uri_ref (uri);
	open_as_channel_op->request.open_mode = open_mode;
	open_as_channel_op->request.advised_block_size = advised_block_size;

	gnome_vfs_job_go (job);

	return (GnomeVFSAsyncHandle *) job;
}

void
pthread_gnome_vfs_async_open_uri_as_channel (GnomeVFSAsyncHandle **handle_return,
					     GnomeVFSURI *uri,
					     GnomeVFSOpenMode open_mode,
					     guint advised_block_size,
					     GnomeVFSAsyncOpenAsChannelCallback callback,
					     gpointer callback_data)
{
	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);

	*handle_return = async_open_as_channel (uri, open_mode, advised_block_size,
						callback, callback_data);
}

void
pthread_gnome_vfs_async_open_as_channel (GnomeVFSAsyncHandle **handle_return,
					 const gchar *text_uri,
					 GnomeVFSOpenMode open_mode,
					 guint advised_block_size,
					 GnomeVFSAsyncOpenAsChannelCallback callback,
					 gpointer callback_data)
{
	GnomeVFSURI *uri;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	uri = gnome_vfs_uri_new (text_uri);
	*handle_return = async_open_as_channel (uri, open_mode, advised_block_size,
						callback, callback_data);
	if (uri != NULL) {
		gnome_vfs_uri_unref (uri);
	}
}

static GnomeVFSAsyncHandle *
async_create (GnomeVFSURI *uri,
	      GnomeVFSOpenMode open_mode,
	      gboolean exclusive,
	      guint perm,
	      GnomeVFSAsyncOpenCallback callback,
	      gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSCreateOp *create_op;

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_CREATE,
			       (GFunc) callback, callback_data);

	create_op = &job->current_op->specifics.create;
	create_op->request.uri = uri == NULL ? NULL : gnome_vfs_uri_ref (uri);
	create_op->request.open_mode = open_mode;
	create_op->request.exclusive = exclusive;
	create_op->request.perm = perm;

	gnome_vfs_job_go (job);

	return (GnomeVFSAsyncHandle *) job;
}

void
pthread_gnome_vfs_async_create_uri (GnomeVFSAsyncHandle **handle_return,
				    GnomeVFSURI *uri,
				    GnomeVFSOpenMode open_mode,
				    gboolean exclusive,
				    guint perm,
				    GnomeVFSAsyncOpenCallback callback,
				    gpointer callback_data)
{
	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);

	*handle_return = async_create (uri, open_mode, exclusive, perm,
				       callback, callback_data);
}

void
pthread_gnome_vfs_async_create (GnomeVFSAsyncHandle **handle_return,
				const gchar *text_uri,
				GnomeVFSOpenMode open_mode,
				gboolean exclusive,
				guint perm,
				GnomeVFSAsyncOpenCallback callback,
				gpointer callback_data)
{
	GnomeVFSURI *uri;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	uri = gnome_vfs_uri_new (text_uri);
	*handle_return = async_create (uri, open_mode, exclusive, perm,
				       callback, callback_data);
	if (uri != NULL) {
		gnome_vfs_uri_unref (uri);
	}
}

void
pthread_gnome_vfs_async_create_as_channel (GnomeVFSAsyncHandle **handle_return,
					   const gchar *text_uri,
					   GnomeVFSOpenMode open_mode,
					   gboolean exclusive,
					   guint perm,
					   GnomeVFSAsyncOpenAsChannelCallback callback,
					   gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSCreateAsChannelOp *create_as_channel_op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_CREATE_AS_CHANNEL,
			       (GFunc) callback, callback_data);

	create_as_channel_op = &job->current_op->specifics.create_as_channel;
	create_as_channel_op->request.uri = gnome_vfs_uri_new (text_uri);
	create_as_channel_op->request.open_mode = open_mode;
	create_as_channel_op->request.exclusive = exclusive;
	create_as_channel_op->request.perm = perm;

	gnome_vfs_job_go (job);

	*handle_return = (GnomeVFSAsyncHandle *) job;
}

void
pthread_gnome_vfs_async_close (GnomeVFSAsyncHandle *handle,
			       GnomeVFSAsyncCloseCallback callback,
			       gpointer callback_data)
{
	GnomeVFSJob *job;

	g_return_if_fail (handle != NULL);
	g_return_if_fail (callback != NULL);

	job = (GnomeVFSJob *) handle;

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_CLOSE,
			       (GFunc) callback, callback_data);
	gnome_vfs_job_go (job);
}

void
pthread_gnome_vfs_async_read (GnomeVFSAsyncHandle *handle,
			      gpointer buffer,
			      guint bytes,
			      GnomeVFSAsyncReadCallback callback,
			      gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSReadOp *read_op;

	g_return_if_fail (handle != NULL);
	g_return_if_fail (buffer != NULL);
	g_return_if_fail (callback != NULL);

	job = (GnomeVFSJob *) handle;

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_READ,
			       (GFunc) callback, callback_data);

	read_op = &job->current_op->specifics.read;
	read_op->request.buffer = buffer;
	read_op->request.num_bytes = bytes;

	gnome_vfs_job_go (job);
}

void
pthread_gnome_vfs_async_write (GnomeVFSAsyncHandle *handle,
			       gconstpointer buffer,
			       guint bytes,
			       GnomeVFSAsyncWriteCallback callback,
			       gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSWriteOp *write_op;

	g_return_if_fail (handle != NULL);
	g_return_if_fail (buffer != NULL);
	g_return_if_fail (callback != NULL);

	job = (GnomeVFSJob *) handle;

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_WRITE,
			       (GFunc) callback, callback_data);

	write_op = &job->current_op->specifics.write;
	write_op->request.buffer = buffer;
	write_op->request.num_bytes = bytes;

	gnome_vfs_job_go (job);
}

void
pthread_gnome_vfs_async_create_symbolic_link (GnomeVFSAsyncHandle **handle_return,
					      GnomeVFSURI *uri,
					      const gchar *uri_reference,
					      GnomeVFSAsyncOpenCallback callback,
					      gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSCreateLinkOp *create_op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_CREATE_SYMBOLIC_LINK,
			       (GFunc) callback, callback_data);

	create_op = &job->current_op->specifics.create_symbolic_link;
	create_op->request.uri = gnome_vfs_uri_ref (uri);
	create_op->request.uri_reference = g_strdup (uri_reference);

	gnome_vfs_job_go (job);

	*handle_return = (GnomeVFSAsyncHandle *) job;
}

void
pthread_gnome_vfs_async_get_file_info (GnomeVFSAsyncHandle **handle_return,
				       GList *uris,
				       GnomeVFSFileInfoOptions options,
				       GnomeVFSAsyncGetFileInfoCallback callback,
				       gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSGetFileInfoOp *get_info_op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_GET_FILE_INFO,
			       (GFunc) callback, callback_data);

	get_info_op = &job->current_op->specifics.get_file_info;

	get_info_op->request.uris = gnome_vfs_uri_list_copy (uris);
	get_info_op->request.options = options;
	get_info_op->notify.result_list = NULL;

	gnome_vfs_job_go (job);

	*handle_return = (GnomeVFSAsyncHandle *) job;
}

void
pthread_gnome_vfs_async_set_file_info (GnomeVFSAsyncHandle **handle_return,
				       GnomeVFSURI *uri,
				       GnomeVFSFileInfo *info,
				       GnomeVFSSetFileInfoMask mask,
				       GnomeVFSFileInfoOptions options,
				       GnomeVFSAsyncSetFileInfoCallback callback,
				       gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSSetFileInfoOp *op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (info != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_SET_FILE_INFO,
			       (GFunc) callback, callback_data);

	op = &job->current_op->specifics.set_file_info;

	op->request.uri = gnome_vfs_uri_ref (uri);
	gnome_vfs_file_info_copy (&op->request.info, info);
	op->request.mask = mask;
	op->request.options = options;

	gnome_vfs_job_go (job);

	*handle_return = (GnomeVFSAsyncHandle *) job;
}

void
pthread_gnome_vfs_async_find_directory (GnomeVFSAsyncHandle **handle_return,
					GList *uris,
					GnomeVFSFindDirectoryKind kind,
					gboolean create_if_needed,
					gboolean find_if_needed,
					guint permissions,
					GnomeVFSAsyncFindDirectoryCallback callback,
					gpointer user_data)
{
	GnomeVFSJob *job;
	GnomeVFSFindDirectoryOp *get_info_op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_FIND_DIRECTORY,
			       (GFunc) callback, user_data);

	get_info_op = &job->current_op->specifics.find_directory;

	get_info_op->request.uris = gnome_vfs_uri_list_copy (uris);
	get_info_op->request.kind = kind;
	get_info_op->request.create_if_needed = create_if_needed;
	get_info_op->request.find_if_needed = find_if_needed;
	get_info_op->request.permissions = permissions;
	get_info_op->notify.result_list = NULL;

	gnome_vfs_job_go (job);

	*handle_return = (GnomeVFSAsyncHandle *) job;
}

static GnomeVFSDirectorySortRule *
copy_sort_rules (GnomeVFSDirectorySortRule *rules)
{
	GnomeVFSDirectorySortRule *new;
	guint count, i;

	if (rules == NULL)
		return NULL;

	for (count = 0; rules[count] != GNOME_VFS_DIRECTORY_SORT_NONE; count++)
		;

	new = g_new (GnomeVFSDirectorySortRule, count + 1);

	for (i = 0; i < count; i++)
		new[i] = rules[i];
	new[i] = GNOME_VFS_DIRECTORY_SORT_NONE;

	return new;
}

static GnomeVFSAsyncHandle *
async_load_directory (GnomeVFSURI *uri,
		      GnomeVFSFileInfoOptions options,
		      GnomeVFSDirectorySortRule sort_rules[],
		      gboolean reverse_order,
		      GnomeVFSDirectoryFilterType filter_type,
		      GnomeVFSDirectoryFilterOptions filter_options,
		      const gchar *filter_pattern,
		      guint items_per_notification,
		      GnomeVFSAsyncDirectoryLoadCallback callback,
		      gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSLoadDirectoryOp *load_directory_op;

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_LOAD_DIRECTORY,
			       (GFunc) callback, callback_data);

	load_directory_op = &job->current_op->specifics.load_directory;
	load_directory_op->request.uri = uri == NULL ? NULL : gnome_vfs_uri_ref (uri);
	load_directory_op->request.options = options;
	load_directory_op->request.sort_rules = copy_sort_rules (sort_rules);
	load_directory_op->request.reverse_order = reverse_order;
	load_directory_op->request.filter_type = filter_type;
	load_directory_op->request.filter_options = filter_options;
	load_directory_op->request.filter_pattern = g_strdup (filter_pattern);
	load_directory_op->request.items_per_notification = items_per_notification;

	gnome_vfs_job_go (job);

	return (GnomeVFSAsyncHandle *) job;
}

void
pthread_gnome_vfs_async_load_directory (GnomeVFSAsyncHandle **handle_return,
					const gchar *text_uri,
					GnomeVFSFileInfoOptions options,
					GnomeVFSDirectorySortRule sort_rules[],
					gboolean reverse_order,
					GnomeVFSDirectoryFilterType filter_type,
					GnomeVFSDirectoryFilterOptions filter_options,
					const gchar *filter_pattern,
					guint items_per_notification,
					GnomeVFSAsyncDirectoryLoadCallback callback,
					gpointer callback_data)
{
	GnomeVFSURI *uri;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	uri = gnome_vfs_uri_new (text_uri);
	*handle_return = async_load_directory (uri, options,
					       sort_rules, reverse_order,
					       filter_type, filter_options, filter_pattern,
					       items_per_notification,
					       callback, callback_data);
	if (uri != NULL) {
		gnome_vfs_uri_unref (uri);
	}
}

void
pthread_gnome_vfs_async_load_directory_uri (GnomeVFSAsyncHandle **handle_return,
					    GnomeVFSURI *uri,
					    GnomeVFSFileInfoOptions options,
					    GnomeVFSDirectorySortRule sort_rules[],
					    gboolean reverse_order,
					    GnomeVFSDirectoryFilterType filter_type,
					    GnomeVFSDirectoryFilterOptions filter_options,
					    const gchar *filter_pattern,
					    guint items_per_notification,
					    GnomeVFSAsyncDirectoryLoadCallback callback,
					    gpointer callback_data)
{
	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);

	*handle_return = async_load_directory (uri, options,
					       sort_rules, reverse_order,
					       filter_type, filter_options, filter_pattern,
					       items_per_notification,
					       callback, callback_data);
}

GnomeVFSResult
pthread_gnome_vfs_async_xfer (GnomeVFSAsyncHandle **handle_return,
			      GList *source_uri_list,
			      GList *target_uri_list,
			      GnomeVFSXferOptions xfer_options,
			      GnomeVFSXferErrorMode error_mode,
			      GnomeVFSXferOverwriteMode overwrite_mode,
			      GnomeVFSAsyncXferProgressCallback progress_update_callback,
			      gpointer update_callback_data,
			      GnomeVFSXferProgressCallback progress_sync_callback,
			      gpointer sync_callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSXferOp *xfer_op;

	g_return_val_if_fail (handle_return != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (progress_update_callback != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	job = gnome_vfs_job_new ();

	gnome_vfs_job_prepare (job, GNOME_VFS_OP_XFER,
			       (GFunc) progress_update_callback,
			       update_callback_data);

	xfer_op = &job->current_op->specifics.xfer;
	xfer_op->request.source_uri_list = gnome_vfs_uri_list_copy (source_uri_list);
	xfer_op->request.target_uri_list = gnome_vfs_uri_list_copy (target_uri_list);
	xfer_op->request.xfer_options = xfer_options;
	xfer_op->request.error_mode = error_mode;
	xfer_op->request.overwrite_mode = overwrite_mode;
	xfer_op->request.progress_sync_callback = progress_sync_callback;
	xfer_op->request.sync_callback_data = sync_callback_data;

	gnome_vfs_job_go (job);

	*handle_return = (GnomeVFSAsyncHandle *) job;

	return GNOME_VFS_OK;
}

guint
pthread_gnome_vfs_async_add_status_callback (GnomeVFSAsyncHandle *handle,
					     GnomeVFSStatusCallback callback,
					     gpointer user_data)
{
	GnomeVFSJob *job;

	g_return_val_if_fail (handle != NULL, 0);
	g_return_val_if_fail (callback != NULL, 0);

	job = (GnomeVFSJob *) handle;

	g_return_val_if_fail (job->current_op != NULL, 0);
	g_return_val_if_fail (job->current_op->context != NULL, 0);

	return gnome_vfs_message_callbacks_add
		(gnome_vfs_context_get_message_callbacks (job->current_op->context),
		 callback, user_data);
}

void
pthread_gnome_vfs_async_remove_status_callback (GnomeVFSAsyncHandle *handle,
						guint callback_id)
{
	GnomeVFSJob *job;

	g_return_if_fail (handle != NULL);
	g_return_if_fail (callback_id > 0);

	job = (GnomeVFSJob *) handle;

	g_return_if_fail (job->current_op != NULL);
	g_return_if_fail (job->current_op->context != NULL);

	gnome_vfs_message_callbacks_remove
		(gnome_vfs_context_get_message_callbacks (job->current_op->context),
		 callback_id);
}
