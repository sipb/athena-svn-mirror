/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-job.h - Jobs for asynchronous operation of the GNOME
   Virtual File System (version for POSIX threads).

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@gnu.org>
*/

#ifndef GNOME_VFS_JOB_PTHREAD_H
#define GNOME_VFS_JOB_PTHREAD_H

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

typedef struct GnomeVFSJob GnomeVFSJob;

#define GNOME_VFS_JOB_DEBUG 0

#if GNOME_VFS_JOB_DEBUG

#include <stdio.h>

extern GStaticMutex debug_mutex;

#define JOB_DEBUG(x)				\
G_STMT_START{					\
	g_static_mutex_lock (&debug_mutex);	\
	printf ("%d ", __LINE__);		\
	fputs (__FUNCTION__ ": ", stdout);	\
	printf x;				\
	fputc ('\n', stdout);			\
	fflush (stdout);			\
	g_static_mutex_unlock (&debug_mutex);	\
}G_STMT_END

#define JOB_DEBUG_ONLY(x) x

#else
#define JOB_DEBUG(x)
#define JOB_DEBUG_ONLY(x)

#endif

enum GnomeVFSOpType {
	GNOME_VFS_OP_OPEN,
	GNOME_VFS_OP_OPEN_AS_CHANNEL,
	GNOME_VFS_OP_CREATE,
	GNOME_VFS_OP_CREATE_SYMBOLIC_LINK,
	GNOME_VFS_OP_CREATE_AS_CHANNEL,
	GNOME_VFS_OP_CLOSE,
	GNOME_VFS_OP_READ,
	GNOME_VFS_OP_WRITE,
	GNOME_VFS_OP_LOAD_DIRECTORY,
	GNOME_VFS_OP_FIND_DIRECTORY,
	GNOME_VFS_OP_XFER,
	GNOME_VFS_OP_GET_FILE_INFO,
	GNOME_VFS_OP_SET_FILE_INFO
};
typedef enum GnomeVFSOpType GnomeVFSOpType;

typedef struct {
	struct {
		GnomeVFSURI *uri;
		GnomeVFSOpenMode open_mode;
	} request;

	struct {
		GnomeVFSResult result;
	} notify;
} GnomeVFSOpenOp;

typedef struct {
	struct {
		GnomeVFSURI *uri;
		GnomeVFSOpenMode open_mode;
		guint advised_block_size;
	} request;

	struct {
		GnomeVFSResult result;
		GIOChannel *channel;
	} notify;
} GnomeVFSOpenAsChannelOp;


typedef struct {
	struct {
		GnomeVFSURI *uri;
		GnomeVFSOpenMode open_mode;
		gboolean exclusive;
		guint perm;
	} request;

	struct {
		GnomeVFSResult result;
	} notify;
} GnomeVFSCreateOp;


typedef struct {
	struct {
		GnomeVFSURI *uri;
		char *uri_reference;
	} request;

	struct {
		GnomeVFSResult result;
	} notify;
} GnomeVFSCreateLinkOp;


typedef struct {
	struct {
		GnomeVFSURI *uri;
		GnomeVFSOpenMode open_mode;
		gboolean exclusive;
		guint perm;
	} request;

	struct {
		GnomeVFSResult result;
		GIOChannel *channel;
	} notify;
} GnomeVFSCreateAsChannelOp;


typedef struct {
	struct {
	} request;

	struct {
		GnomeVFSResult result;
	} notify;
} GnomeVFSCloseOp;


typedef struct {
	struct {
		GnomeVFSFileSize num_bytes;
		gpointer buffer;
	} request;

	struct {
		GnomeVFSResult result;
		GnomeVFSFileSize bytes_read;
	} notify;
} GnomeVFSReadOp;


typedef struct {
	struct {
		GnomeVFSFileSize num_bytes;
		gconstpointer buffer;
	} request;

	struct {
		GnomeVFSResult result;
		GnomeVFSFileSize bytes_written;
	} notify;
} GnomeVFSWriteOp;


typedef struct {
	struct {
		GList *uris; /* GnomeVFSURI* */
		GnomeVFSFileInfoOptions options;
	} request;

	struct {
		GList *result_list; /* GnomeVFSGetFileInfoResult* */
	} notify;
} GnomeVFSGetFileInfoOp;

typedef struct {
	struct {
		GnomeVFSURI *uri;
		GnomeVFSFileInfo info;
		GnomeVFSSetFileInfoMask mask;
		GnomeVFSFileInfoOptions options;
	} request;

	struct {
		GnomeVFSResult set_file_info_result;
		GnomeVFSResult get_file_info_result;
		GnomeVFSFileInfo info;
	} notify;
} GnomeVFSSetFileInfoOp;


typedef struct {
	struct {
		GList *uris; /* GnomeVFSURI* */
		GnomeVFSFindDirectoryKind kind;
		gboolean create_if_needed;
		gboolean find_if_needed;
		guint permissions;
	} request;

	struct {
		GList *result_list; /* GnomeVFSFindDirectoryResult */
	} notify;
} GnomeVFSFindDirectoryOp;

/* "Complex operations.  */

typedef struct {
	struct {
		GnomeVFSURI *uri;
		GnomeVFSFileInfoOptions options;
		GnomeVFSDirectorySortRule *sort_rules;
		gboolean reverse_order;
		GnomeVFSDirectoryFilterType filter_type;
		GnomeVFSDirectoryFilterOptions filter_options;
		gchar *filter_pattern;
		guint items_per_notification;
	} request;

	struct {
		GnomeVFSResult result;
		GnomeVFSDirectoryList *list;
		guint entries_read;
	} notify;
} GnomeVFSLoadDirectoryOp;

typedef struct {
	struct {
		GList *source_uri_list;
		GList *target_uri_list;
		GnomeVFSXferOptions xfer_options;
		GnomeVFSXferErrorMode error_mode;
		GnomeVFSXferOverwriteMode overwrite_mode;
		GnomeVFSXferProgressCallback progress_sync_callback;
		gpointer sync_callback_data;
	} request;

	struct {
		GnomeVFSXferProgressInfo *progress_info;
	} notify;

	struct {
		gint value;
	} notify_answer;
} GnomeVFSXferOp;

typedef union {
	GnomeVFSOpenOp open;
	GnomeVFSOpenAsChannelOp open_as_channel;
	GnomeVFSCreateOp create;
	GnomeVFSCreateLinkOp create_symbolic_link;
	GnomeVFSCreateAsChannelOp create_as_channel;
	GnomeVFSCloseOp close;
	GnomeVFSReadOp read;
	GnomeVFSWriteOp write;
	GnomeVFSLoadDirectoryOp load_directory;
	GnomeVFSXferOp xfer;
	GnomeVFSGetFileInfoOp get_file_info;
	GnomeVFSSetFileInfoOp set_file_info;
	GnomeVFSFindDirectoryOp find_directory;
} GnomeVFSSpecificOp;

typedef struct {
	/* ID of the job (e.g. open, create, close...). */
	GnomeVFSOpType type;

	/* The callback for when the op is completed. */
	GFunc callback;
	gpointer callback_data;

	/* Details of the op. */
	GnomeVFSSpecificOp specifics;

	/* The context for cancelling the operation. */
	GnomeVFSContext *context;
} GnomeVFSOp;

/* FIXME bugzilla.eazel.com 1135: Move private stuff out of the header.  */
struct GnomeVFSJob {
	/* Handle being used for file access.  */
	GnomeVFSHandle *handle;

	/* Global lock for accessing data.  */
	GMutex *access_lock;

	/* Condition that is raised when a new job has been prepared.  As
           `GnomeVFSJob' can hold one job at a given time, the way to set up a
           new job is as follows: (a) lock `access_lock' (b) write job
           information into the struct (c) signal `execution_condition' (d)
           unlock `access_lock'.  */
	GCond *execution_condition;

	/* This condition is signalled when the master thread gets a
           notification and wants to acknowledge it.  */
	GCond *notify_ack_condition;

	/* Mutex associated with `notify_ack_condition'.  We cannot just use
           `access_lock', because we want to keep the lock in the slave thread
           until the job is really finished.  */
	GMutex *notify_ack_lock;

	/* Whether this struct is ready for containing a new job for
	   execution.  */
	gboolean is_empty;

	/* I/O channels used to wake up the master thread.  When the slave
           thread wants to notify the master thread that an operation has been
           done, it writes a character into `wakeup_channel_in' and the master
           thread detects this in the GLIB main loop by using a watch.  */
	GIOChannel *wakeup_channel_in;
	GIOChannel *wakeup_channel_out;

	/* Channel mutex to prevent more than one notification to be queued
           into the channel.  */
	GMutex *wakeup_channel_lock;

	/* Whether this job wants the notification acknowledged.  */
	gboolean want_notify_ack;

	/* Operations that are being done and those that are completed and
	 * ready for notification to take place.
	 */
	GnomeVFSOp *current_op;
	GnomeVFSOp *notify_op;
};

GnomeVFSJob *gnome_vfs_job_new       (void);
void         gnome_vfs_job_destroy   (GnomeVFSJob    *job);
void         gnome_vfs_job_prepare   (GnomeVFSJob    *job,
				      GnomeVFSOpType  type,
				      GFunc           callback,
				      gpointer        callback_data);
void         gnome_vfs_job_go        (GnomeVFSJob    *job);
gboolean     gnome_vfs_job_execute   (GnomeVFSJob    *job);
void         gnome_vfs_job_cancel    (GnomeVFSJob    *job);
int          gnome_vfs_job_get_count (void);

#endif /* GNOME_VFS_JOB_PTHREAD_H */
