/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-slave-process.c - Object for dealing with VFS slave processes.

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

#ifndef _GNOME_VFS_SLAVE_PROCESS_H
#define _GNOME_VFS_SLAVE_PROCESS_H

typedef struct _GnomeVFSSlaveProcess GnomeVFSSlaveProcess;

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#include "gnome-vfs-slave-notify.h"

struct _GnomeVFSAsyncFileOpInfo {
	gpointer buffer;
	GnomeVFSFileSize buffer_size;
};
typedef struct _GnomeVFSAsyncFileOpInfo GnomeVFSAsyncFileOpInfo;

struct _GnomeVFSAsyncDirectoryOpInfo {
	gchar **meta_keys;
	guint num_meta_keys;
	GList *list;
};
typedef struct _GnomeVFSAsyncDirectoryOpInfo GnomeVFSAsyncDirectoryOpInfo;

struct _GnomeVFSAsyncXferOpInfo {
	GnomeVFSXferProgressInfo progress_info;
	GnomeVFSXferOptions xfer_options;
	GnomeVFSXferErrorMode error_mode;
	GnomeVFSXferOverwriteMode overwrite_mode;
};
typedef struct _GnomeVFSAsyncXferOpInfo GnomeVFSAsyncXferOpInfo;

enum _GnomeVFSAsyncOperation {
	GNOME_VFS_ASYNC_OP_NONE,
	GNOME_VFS_ASYNC_OP_CHANNEL,
	GNOME_VFS_ASYNC_OP_CLOSE,
	GNOME_VFS_ASYNC_OP_CREATE,
	GNOME_VFS_ASYNC_OP_CREATE_AS_CHANNEL,
	GNOME_VFS_ASYNC_OP_LOAD_DIRECTORY,
	GNOME_VFS_ASYNC_OP_OPEN,
	GNOME_VFS_ASYNC_OP_OPEN_AS_CHANNEL,
	GNOME_VFS_ASYNC_OP_READ,
	GNOME_VFS_ASYNC_OP_RESET,
	GNOME_VFS_ASYNC_OP_WRITE,
	GNOME_VFS_ASYNC_OP_XFER,
	GNOME_VFS_ASYNC_OP_GET_FILE_INFO
};
typedef enum _GnomeVFSAsyncOperation GnomeVFSAsyncOperation;

struct _GnomeVFSSlaveProcess {
	GNOME_VFS_Slave_Notify notify_objref;
	GnomeVFSSlaveNotifyServant *notify_servant;

	GNOME_VFS_Slave_Request request_objref;
	GNOME_VFS_Slave_FileHandle file_handle_objref;

	GnomeVFSAsyncOperation operation_in_progress;

	union {
		GnomeVFSAsyncFileOpInfo file;
		GnomeVFSAsyncDirectoryOpInfo directory;
		GnomeVFSAsyncXferOpInfo xfer;
	} op_info;

	gpointer callback;
	gpointer callback_data;

	CORBA_Environment ev;

	GnomeVFSContext *context;
};

GnomeVFSSlaveProcess *gnome_vfs_slave_process_new (void);
void gnome_vfs_slave_process_destroy (GnomeVFSSlaveProcess *slave);
gboolean gnome_vfs_slave_process_busy (GnomeVFSSlaveProcess *slave);

#if 0
/* FIXME bugzilla.eazel.com 1124: */
void gnome_vfs_slave_process_reset (GnomeVFSSlaveProcess *slave,
				    GnomeVFSSlaveProcessResetCallback callback,
				    gpointer callback_data);
#endif

#endif /* _GNOME_VFS_SLAVE_PROCESS_H */
