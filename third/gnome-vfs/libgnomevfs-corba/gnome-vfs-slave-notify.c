/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-slave-notify.c - Handling of notifications from the VFS slave.

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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
     
#include <orb/orbit.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#include "gnome-vfs-corba.h"
#include "gnome-vfs-slave.h"
#include "gnome-vfs-slave-process.h"
#include "gnome-vfs-slave-launch.h"

#include "gnome-vfs-slave-notify.h"


#if ! defined (SUN_LEN)
/* This system is not POSIX.1g.  */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
                      + strlen ((ptr)->sun_path))
#endif


struct _GnomeVFSSlaveNotifyServant {
	POA_GNOME_VFS_Slave_Notify servant;
	GnomeVFSSlaveProcess *slave;
};


static PortableServer_ServantBase__epv base_epv;
static POA_GNOME_VFS_Slave_Notify__epv Notify_epv;
static POA_GNOME_VFS_Slave_Notify__vepv Notify_vepv;


static GnomeVFSSlaveProcess *
slave_from_servant (PortableServer_Servant servant)
{
	GnomeVFSSlaveNotifyServant *notify_servant;

	notify_servant = (GnomeVFSSlaveNotifyServant *) servant;
	return notify_servant->slave;
}

static void
free_servant (PortableServer_Servant servant)
{
	g_free (servant);
}


/* Basic methods in the Notify interface.  */

static void
impl_Notify_reset (PortableServer_Servant servant,
		   CORBA_Environment *ev)
{
	/* FIXME bugzilla.eazel.com 1124: */
#if 0
	GnomeVFSSlaveProcess *slave;
	GnomeVFSSlaveProcessResetCallback callback;

	slave = slave_from_servant (servant);
	callback = slave->callback;

	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_RESET) {
		g_warning ("slave received reset notify, "
			   "but reset operation is not in progress");
		return;
	}

	free_handle_list (slave);

	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	(* callback) (slave, slave->callback_data);
#endif
}

static void
impl_Notify_dying (PortableServer_Servant servant,
		   CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;

	slave = slave_from_servant (servant);

	if (slave->request_objref != CORBA_OBJECT_NIL)
		CORBA_Object_release (slave->request_objref, ev);

	if (slave->notify_objref != CORBA_OBJECT_NIL) {
		POA_GNOME_VFS_Slave_Notify__fini
			((POA_GNOME_VFS_Slave_Notify *) slave->notify_servant,
			 ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			g_warning (_("Cannot kill GNOME::VFS::Slave::Notify -- exception %s"),
				   CORBA_exception_id (ev));
		free_servant (slave->notify_servant);
		CORBA_Object_release (slave->notify_objref, ev);
	}

	CORBA_exception_free (&slave->ev);

	g_free (slave);
}

static void
impl_Notify_open (PortableServer_Servant servant,
		  const GNOME_VFS_Result result,
		  const GNOME_VFS_Slave_FileHandle handle,
		  CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncOpenCallback callback;

	slave = slave_from_servant (servant);

	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_OPEN
	    && slave->operation_in_progress != GNOME_VFS_ASYNC_OP_CREATE) {
		g_warning ("slave received open notify, "
			   "but open operation is not in progress");
		return;
	}

	slave->file_handle_objref = CORBA_Object_duplicate (handle, ev);
	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	callback = (GnomeVFSAsyncOpenCallback) slave->callback;
	(* callback) ((GnomeVFSAsyncHandle *) slave,
		      (GnomeVFSResult) result,
		      slave->callback_data);
}

static void
impl_Notify_open_as_channel (PortableServer_Servant servant,
			     const GNOME_VFS_Result result,
			     const CORBA_char *sock_path,
			     CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncOpenAsChannelCallback callback;
	GIOChannel *new_channel;
	GnomeVFSResult vfs_result;
	gint fd;

	slave = slave_from_servant (servant);

	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_OPEN_AS_CHANNEL
	    && slave->operation_in_progress != GNOME_VFS_ASYNC_OP_CREATE_AS_CHANNEL) {
		g_warning ("slave received open_as_channel notify, "
			   "but open_as_channel operation is not in progress");
		return;
	}

	new_channel = NULL;
	vfs_result = (GnomeVFSResult) result;

	if (vfs_result != GNOME_VFS_OK) {
		slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;
	} else {
		fd = socket (AF_UNIX, SOCK_STREAM, 0);
		if (fd < 0) {
			g_warning (_("Cannot create socket: %s"),
				   g_strerror (errno));
		} else {
			struct sockaddr_un saddr;
			gint r;

			saddr.sun_family = AF_UNIX;
			strncpy (saddr.sun_path, sock_path,
				 sizeof (saddr.sun_path));
			r = connect (fd, (struct sockaddr *)&saddr,
				     SUN_LEN (&saddr));
			if (r < 0) {
				g_warning (_("Cannot connect socket `%s': %s"),
					   saddr.sun_path, g_strerror (errno));
				close (fd);
				vfs_result = GNOME_VFS_ERROR_INTERNAL;
			} else {
				new_channel = g_io_channel_unix_new (fd);
				slave->operation_in_progress
					= GNOME_VFS_ASYNC_OP_CHANNEL;
			}
		}
	}

	callback = (GnomeVFSAsyncOpenAsChannelCallback) slave->callback;
	(* callback) ((GnomeVFSAsyncHandle *) slave,
		      new_channel, vfs_result, slave->callback_data);
}

static void
impl_Notify_close (PortableServer_Servant servant,
		   const GNOME_VFS_Result result,
		   CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncCloseCallback callback;

	slave = slave_from_servant (servant);

	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_CLOSE) {
		g_warning ("slave received close notify, "
			   "but close operation is not in progress");
		return;
	}

	CORBA_Object_release (slave->file_handle_objref, ev);
	slave->file_handle_objref = CORBA_OBJECT_NIL;
	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	callback = (GnomeVFSAsyncCloseCallback) slave->callback;
	(* callback) ((GnomeVFSAsyncHandle *) slave,
		      (GnomeVFSResult) result,
		      slave->callback_data);

	gnome_vfs_slave_process_destroy (slave);
}

static void
impl_Notify_read (PortableServer_Servant servant,
		  const GNOME_VFS_Result result,
		  const GNOME_VFS_Buffer *data,
		  CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncReadCallback callback;

	slave = slave_from_servant (servant);

	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_READ) {
		g_warning ("slave received read notify, "
			   "but read operation is not in progress");
		return;
	}

	memcpy (slave->op_info.file.buffer, data->_buffer, data->_length);

	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	callback = (GnomeVFSAsyncReadCallback) slave->callback;
	(* callback) ((GnomeVFSAsyncHandle *) slave,
		      (GnomeVFSResult) result,
		      slave->op_info.file.buffer,
		      slave->op_info.file.buffer_size,
		      data->_length,
		      slave->callback_data);
}

static void
impl_Notify_write (PortableServer_Servant servant,
		   const GNOME_VFS_Result result,
		   const CORBA_unsigned_long bytes_written,
		   CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncReadCallback callback;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_WRITE) {
		g_warning ("slave received write notify, "
			   "but write operation is not in progress");
		return;
	}

	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	callback = (GnomeVFSAsyncReadCallback) slave->callback;
	(* callback) ((GnomeVFSAsyncHandle *) slave,
		      (GnomeVFSResult) result,
		      slave->op_info.file.buffer,
		      slave->op_info.file.buffer_size,
		      bytes_written,
		      slave->callback_data);
}


/* Transfer-related Notify methods.  */

static void
free_progress (GnomeVFSXferProgressInfo *info)
{
	g_free (info->source_name);
	g_free (info->target_name);
}

static CORBA_boolean
impl_Notify_xfer_start (PortableServer_Servant servant,
			const CORBA_unsigned_long files_total,
			const CORBA_unsigned_long bytes_total,
			CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncXferOpInfo *op_info;
	GnomeVFSXferProgressInfo *progress_info;
	GnomeVFSAsyncXferProgressCallback callback;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_XFER) {
		g_warning ("slave received xfer_start notify, "
			   "but xfer operation is not in progress");
		return CORBA_FALSE;
	}

	op_info = &slave->op_info.xfer;
	progress_info = &op_info->progress_info;
	callback = (GnomeVFSAsyncXferProgressCallback) slave->callback;

	progress_info->phase = GNOME_VFS_XFER_PHASE_READYTOGO;
	progress_info->files_total = files_total;
	progress_info->bytes_total = bytes_total;

	return (* callback) ((GnomeVFSAsyncHandle *) slave,
			     progress_info, slave->callback_data);
}

static CORBA_boolean
impl_Notify_xfer_file_start (PortableServer_Servant servant,
			     const CORBA_char *source_uri,
			     const CORBA_char *target_uri,
			     const CORBA_unsigned_long bytes_to_copy,
			     CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncXferOpInfo *op_info;
	GnomeVFSXferProgressInfo *progress_info;
	GnomeVFSAsyncXferProgressCallback callback;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_XFER) {
		g_warning ("slave received xfer_file_start notify, "
			   "but xfer operation is not in progress");
		return CORBA_FALSE;
	}

	op_info = &slave->op_info.xfer;
	progress_info = &op_info->progress_info;
	callback = (GnomeVFSAsyncXferProgressCallback) slave->callback;

	g_free (progress_info->source_name);
	g_free (progress_info->target_name);

	progress_info->phase = GNOME_VFS_XFER_PHASE_COPYING;
	progress_info->file_index++;
	progress_info->source_name = g_strdup (source_uri);
	progress_info->target_name = g_strdup (target_uri);
	progress_info->file_size = bytes_to_copy;
	progress_info->bytes_copied = 0;

	return (* callback) ((GnomeVFSAsyncHandle *) slave,
			     progress_info, slave->callback_data);
}

static CORBA_boolean
impl_Notify_xfer_file_progress (PortableServer_Servant servant,
				const CORBA_unsigned_long bytes_copied,
				const CORBA_unsigned_long total_bytes_copied,
				CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncXferOpInfo *op_info;
	GnomeVFSXferProgressInfo *progress_info;
	GnomeVFSAsyncXferProgressCallback callback;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_XFER) {
		g_warning ("slave received xfer_file_progress notify, "
			   "but xfer operation is not in progress");
		return CORBA_FALSE;
	}

	op_info = &slave->op_info.xfer;
	progress_info = &op_info->progress_info;
	callback = (GnomeVFSAsyncXferProgressCallback) slave->callback;

	progress_info->phase = GNOME_VFS_XFER_PHASE_COPYING;
	progress_info->bytes_copied = bytes_copied;
	progress_info->total_bytes_copied = total_bytes_copied;

	return (* callback) ((GnomeVFSAsyncHandle *) slave,
			     progress_info, slave->callback_data);
}

static CORBA_boolean
impl_Notify_xfer_file_done (PortableServer_Servant servant,
			    CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncXferOpInfo *op_info;
	GnomeVFSXferProgressInfo *progress_info;
	GnomeVFSAsyncXferProgressCallback callback;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_XFER) {
		g_warning ("slave received xref_file_done notify, "
			   "but xfer operation is not in progress");
		return CORBA_FALSE;
	}

	op_info = &slave->op_info.xfer;
	progress_info = &op_info->progress_info;
	callback = (GnomeVFSAsyncXferProgressCallback) slave->callback;

	progress_info->phase = GNOME_VFS_XFER_PHASE_FILECOMPLETED;

	return (* callback) ((GnomeVFSAsyncHandle *) slave,
			     progress_info, slave->callback_data);
}

static void
impl_Notify_xfer_done (PortableServer_Servant servant,
		       CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncXferOpInfo *op_info;
	GnomeVFSXferProgressInfo *progress_info;
	GnomeVFSAsyncXferProgressCallback callback;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_XFER) {
		g_warning ("slave received xfer_done notify, "
			   "but xfer operation is not in progress");
		return;
	}

	op_info = &slave->op_info.xfer;
	progress_info = &op_info->progress_info;
	callback = (GnomeVFSAsyncXferProgressCallback) slave->callback;

	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	progress_info->phase = GNOME_VFS_XFER_PHASE_COMPLETED;
	(* callback) ((GnomeVFSAsyncHandle *) slave,
		      progress_info, slave->callback_data);

	free_progress (progress_info);
	gnome_vfs_slave_process_destroy (slave);
}

static void
impl_Notify_xfer_error (PortableServer_Servant servant,
			const GNOME_VFS_Result result,
			CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncXferOpInfo *op_info;
	GnomeVFSXferProgressInfo *progress_info;
	GnomeVFSAsyncXferProgressCallback callback;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_XFER) {
		g_warning ("slave received xfer_error notify, "
			   "but xfer operation is not in progress");
		return;
	}

	op_info = &slave->op_info.xfer;
	progress_info = &op_info->progress_info;
	callback = (GnomeVFSAsyncXferProgressCallback) slave->callback;

	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR;
	progress_info->vfs_status = result;

	(* callback) ((GnomeVFSAsyncHandle *) slave,
		      progress_info, slave->callback_data);

	free_progress (progress_info);
	gnome_vfs_slave_process_destroy (slave);
}

static CORBA_unsigned_short
impl_Notify_xfer_query_for_error (PortableServer_Servant servant,
				  const GNOME_VFS_Result result,
				  const GNOME_VFS_Slave_XferPhase phase,
				  CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncXferOpInfo *op_info;
	GnomeVFSXferProgressInfo *progress_info;
	GnomeVFSAsyncXferProgressCallback callback;
	GnomeVFSXferErrorAction action;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_XFER) {
		g_warning ("slave received xfer_query_for_error notify, "
			   "but xfer operation is not in progress");
		return GNOME_VFS_XFER_ERROR_ACTION_ABORT;
	}

	op_info = &slave->op_info.xfer;
	progress_info = &op_info->progress_info;
	callback = (GnomeVFSAsyncXferProgressCallback) slave->callback;

	action = (* callback) ((GnomeVFSAsyncHandle *) slave,
			       progress_info, slave->callback_data);

	if (action == GNOME_VFS_XFER_ERROR_ACTION_ABORT)
		slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	return action;
}

static CORBA_unsigned_short
impl_Notify_xfer_query_for_overwrite (PortableServer_Servant servant,
				      const CORBA_char *source_uri,
				      const CORBA_char *target_uri,
				      CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncXferOpInfo *op_info;
	GnomeVFSXferProgressInfo *progress_info;
	GnomeVFSAsyncXferProgressCallback callback;
	GnomeVFSXferOverwriteAction action;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_XFER) {
		g_warning ("slave received xfer_query_for_overwrite notify, "
			   "but xfer operation is not in progress");
		return GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT;
	}

	op_info = &slave->op_info.xfer;
	progress_info = &op_info->progress_info;
	callback = (GnomeVFSAsyncXferProgressCallback) slave->callback;

	action = (* callback) ((GnomeVFSAsyncHandle *) slave,
			       progress_info, slave->callback_data);

	if (action == GNOME_VFS_XFER_OVERWRITE_MODE_ABORT)
		slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	return action;
}



static void
impl_Notify_load_directory (PortableServer_Servant servant,
			    const GNOME_VFS_Result result,
			    const GNOME_VFS_Slave_FileInfoList *files,
			    CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GnomeVFSAsyncDirectoryOpInfo *op_info;
	GnomeVFSDirectoryList *list;
	GnomeVFSAsyncDirectoryLoadCallback callback;
	guint i;

	slave = slave_from_servant (servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_LOAD_DIRECTORY) {
		g_warning ("slave received load_directory notify, "
			   "but load_directory operation is not in progress");
		return;
	}

	op_info = &slave->op_info.directory;
	list = op_info->list;

	if (list == NULL) {
		if (files->_length > 0) {
			op_info->list = gnome_vfs_directory_list_new ();
			list = op_info->list;
		}
	} else {
		gnome_vfs_directory_list_last (list);
	}

	for (i = 0; i < files->_length; i++) {
		GNOME_VFS_Slave_FileInfo *slave_info;
		GnomeVFSFileInfo *info;

		slave_info = files->_buffer + i;

		info = gnome_vfs_file_info_new ();
		memcpy (info, (GnomeVFSFileInfo *) slave_info->data._buffer,
			slave_info->data._length);

		if (slave_info->name[0] == 0)
			info->name = NULL;
		else
			info->name = g_strdup (slave_info->name);

		if (slave_info->mime_type[0] == 0)
			info->mime_type = NULL;
		else
			info->mime_type = g_strdup (slave_info->mime_type);

		if (slave_info->symlink_name[0] == 0)
			info->symlink_name = NULL;
		else
			info->symlink_name
				= g_strdup (slave_info->symlink_name);

		gnome_vfs_directory_list_append (list, info);
	}

	if (result != GNOME_VFS_OK) {
		slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;
	}

	/* Make sure we have set a current position on the list.  */
	if (list != NULL
	    && gnome_vfs_directory_list_get_position (list) == NULL)
		gnome_vfs_directory_list_first (list);

	callback = (GnomeVFSAsyncDirectoryLoadCallback) slave->callback;
	(* callback) ((GnomeVFSAsyncHandle *) slave, result,
		      list, files->_length, slave->callback_data);
}

static gchar *
strdup_or_null (const gchar *string)
{
	return string[0] == '\0' ? NULL : g_strdup (string);
}

static void
impl_Notify_get_file_info (PortableServer_Servant servant,
			   const GNOME_VFS_Slave_GetFileInfoResultList *results,
			   CORBA_Environment *ev)
{
	GnomeVFSSlaveProcess *slave;
	GList *result_list, *p;
	GnomeVFSGetFileInfoResult *result_item;
	GnomeVFSAsyncGetFileInfoCallback callback;
	const GNOME_VFS_Slave_FileInfo *file_info;
	GnomeVFSFileInfo *info;
	int i;

	slave = slave_from_servant(servant);
	if (slave->operation_in_progress != GNOME_VFS_ASYNC_OP_GET_FILE_INFO) {
		g_warning ("slave received get_file_info notify, "
			   "but get_file_info operation is not in progress");
		return;
	}


	result_list = NULL;
	for (i = 0; i < results->_length; i++) {
		file_info = &results->_buffer[i].file_info;

		/* Create a file info object from the flat one on the wire. */
		info = gnome_vfs_file_info_new ();
		g_assert (file_info->data._length == sizeof (GnomeVFSFileInfo));
		*info = * (GnomeVFSFileInfo *) file_info->data._buffer;
		info->refcount = 1; /* stupid memcpy, doh doh doh */

		info->name = strdup_or_null (file_info->name);
		info->mime_type = strdup_or_null (file_info->mime_type);
		info->symlink_name = strdup_or_null (file_info->symlink_name);
		
		result_item = g_new (GnomeVFSGetFileInfoResult, 1);

		result_item->uri = gnome_vfs_uri_new (results->_buffer[i].uri);
		result_item->result = results->_buffer[i].result;
		result_item->file_info = info;

		result_list = g_list_prepend (result_list, result_item);
	}
	result_list = g_list_reverse (result_list);

	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	callback = (GnomeVFSAsyncGetFileInfoCallback)slave->callback;
	(* callback) ((GnomeVFSAsyncHandle *) slave,
		      result_list,
		      slave->callback_data);

	for (p = result_list; p != NULL; p = p->next) {
		result_item = p->data;

		gnome_vfs_uri_unref (result_item->uri);
		gnome_vfs_file_info_unref (result_item->file_info);
		g_free (result_item);
	}
	g_list_free (result_list);

}


gboolean
gnome_vfs_slave_notify_create (GnomeVFSSlaveProcess *slave)
{
	GnomeVFSSlaveNotifyServant *servant;

	base_epv._private = NULL;
	base_epv.finalize = NULL;
	base_epv.default_POA = NULL;

	Notify_epv.open = impl_Notify_open;
	Notify_epv.open_as_channel = impl_Notify_open_as_channel;
	Notify_epv.close = impl_Notify_close;
	Notify_epv.read  = impl_Notify_read;
	Notify_epv.write = impl_Notify_write;
	Notify_epv.load_directory = impl_Notify_load_directory;
	Notify_epv.dying = impl_Notify_dying;
	Notify_epv.reset = impl_Notify_reset;
	Notify_epv.get_file_info = impl_Notify_get_file_info;

	Notify_epv.xfer_start = impl_Notify_xfer_start;
	Notify_epv.xfer_file_start = impl_Notify_xfer_file_start;
	Notify_epv.xfer_file_progress = impl_Notify_xfer_file_progress;
	Notify_epv.xfer_file_done = impl_Notify_xfer_file_done;
	Notify_epv.xfer_done = impl_Notify_xfer_done;
	Notify_epv.xfer_error = impl_Notify_xfer_error;
	Notify_epv.xfer_query_for_overwrite = impl_Notify_xfer_query_for_overwrite;
	Notify_epv.xfer_query_for_error = impl_Notify_xfer_query_for_error;

	Notify_vepv._base_epv = &base_epv;
	Notify_vepv.GNOME_VFS_Slave_Notify_epv = &Notify_epv;

	servant = g_new0 (GnomeVFSSlaveNotifyServant, 1);
	servant->servant.vepv = &Notify_vepv;
	servant->slave = slave;

	POA_GNOME_VFS_Slave_Notify__init ((PortableServer_Servant) servant,
					  &slave->ev);
	if (slave->ev._major != CORBA_NO_EXCEPTION){
		g_warning (_("Cannot initialize GNOME::VFS:Slave::Notify"));
		g_free (servant);
		return FALSE;
	}

	CORBA_free (PortableServer_POA_activate_object (gnome_vfs_poa,
							servant,
							&slave->ev));

	slave->notify_objref
		= PortableServer_POA_servant_to_reference (gnome_vfs_poa,
							   servant,
							   &slave->ev);

	slave->notify_servant = servant;

	return TRUE;
}

