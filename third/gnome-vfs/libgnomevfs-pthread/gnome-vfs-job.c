/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */ /*
gnome-vfs-job.c - Jobs for asynchronous operation of the GNOME Virtual File
System (version for POSIX threads).

   Copyright (C) 1999 Free Software Foundation
   Copyright (C) 2000 Eazel

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

   Authors: 
   	Ettore Perazzoli <ettore@gnu.org> 
  	Pavel Cisler <pavel@eazel.com> 
  	Darin Adler <darin@eazel.com> 

   */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnome-vfs-job.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "gnome-vfs-job-slave.h"

#if GNOME_VFS_JOB_DEBUG

/* FIXME bugzilla.eazel.com 1130
 * - this is should use the correct static mutex initialization macro.
 * However glibconfig.h is broken and the supplied macro gives a warning.
 * Since this id debug only, just use what the macro should be here.
 * even though it is not portable.
 */
GStaticMutex debug_mutex = { NULL, { { } } };
#endif

static void gnome_vfs_job_release_current_op (GnomeVFSJob *job);
static void gnome_vfs_job_release_notify_op  (GnomeVFSJob *job);
static void gnome_vfs_job_finish_destroy     (GnomeVFSJob *job);

static int job_count = 0;

static void
set_fl (int fd, int flags)
{
	int val;

	val = fcntl (fd, F_GETFL, 0);
	if (val < 0) {
		g_warning ("fcntl() F_GETFL failed: %s", strerror (errno));
		return;
	}

	val |= flags;
	
	val = fcntl (fd, F_SETFL, val);
	if (val < 0) {
		g_warning ("fcntl() F_SETFL failed: %s", strerror (errno));
		return;
	}
}

static void
clr_fl (int fd, int flags)
{
	int val;

	val = fcntl (fd, F_GETFL, 0);
	if (val < 0) {
		g_warning ("fcntl() F_GETFL failed: %s", strerror (errno));
		return;
	}

	val &= ~flags;
	
	val = fcntl (fd, F_SETFL, val);
	if (val < 0) {
		g_warning ("fcntl() F_SETFL failed: %s", strerror (errno));
		return;
	}
}



static void
job_signal_ack_condition (GnomeVFSJob *job)
{
	g_mutex_lock (job->notify_ack_lock);
	JOB_DEBUG (("Ack needed: signaling condition. %p", job));
	g_cond_signal (job->notify_ack_condition);
	JOB_DEBUG (("Ack needed: unlocking notify ack. %p", job));
	g_mutex_unlock (job->notify_ack_lock);
}

/* This is used by the master thread to notify the slave thread that it got the
   notification.  */
static void
job_ack_notify (GnomeVFSJob *job)
{
	JOB_DEBUG (("Checking if ack is needed. %p", job));
	if (job->want_notify_ack) {
		JOB_DEBUG (("Ack needed: lock notify ack. %p", job));
		job_signal_ack_condition (job);
	}

	JOB_DEBUG (("unlocking wakeup channel. %p", job));

	g_assert (job->notify_op == NULL);
	g_mutex_unlock (job->wakeup_channel_lock);
}

#if GNOME_VFS_JOB_DEBUG
static char debug_wake_channel_out = 'a';
#endif

static gboolean
wakeup (GnomeVFSJob *job)
{
	gboolean retval;
	guint bytes_written;

	JOB_DEBUG (("Wake up! %p", job));

	/* Wake up the main thread.  */

#if GNOME_VFS_JOB_DEBUG
	debug_wake_channel_out++;
	if (debug_wake_channel_out > 'z')
		debug_wake_channel_out = 'a';

	g_io_channel_write (job->wakeup_channel_out, &debug_wake_channel_out, 
		1, &bytes_written);
#else
	g_io_channel_write (job->wakeup_channel_out, "a", 
		1, &bytes_written);
#endif
	JOB_DEBUG (("sent wakeup %c %p", debug_wake_channel_out, job));
	if (bytes_written != 1) {
		JOB_DEBUG (("problems sending a wakeup! %p", job));
		g_warning (_("Error writing to the wakeup GnomeVFSJob channel."));
		retval = FALSE;
	} else {
		retval = TRUE;
	}

	return retval;
}


/* This notifies the master thread asynchronously, without waiting for an
   acknowledgment.  */
static gboolean
job_oneway_notify (GnomeVFSJob *job)
{

	JOB_DEBUG (("lock channel %p", job));
	g_mutex_lock (job->wakeup_channel_lock);

	/* Record which op we want notified. */
	g_assert (job->notify_op == NULL || job->current_op == NULL);
	if (job->notify_op == NULL)
		job->notify_op = job->current_op;

	job->want_notify_ack = FALSE;

	return wakeup (job);
}


/* This notifies the master threads, waiting until it acknowledges the
   notification.  */
static gboolean
job_notify (GnomeVFSJob *job)
{
	gboolean retval;

	if (gnome_vfs_context_check_cancellation (job->current_op->context)) {
		JOB_DEBUG (("job cancelled, bailing %p", job));
		return FALSE;
	}

	JOB_DEBUG (("Locking wakeup channel - %p", job));
	g_mutex_lock (job->wakeup_channel_lock);

	/* Record which op we want notified. */
	g_assert (job->notify_op == NULL);
	job->notify_op = job->current_op;

	JOB_DEBUG (("Locking notification lock %p", job));
	/* Lock notification, so that the master cannot send the signal until
           we are ready to receive it.  */
	g_mutex_lock (job->notify_ack_lock);

	job->want_notify_ack = TRUE;

	/* Send the notification.  This will wake up the master thread, which
           will in turn signal the notify condition.  */
	retval = wakeup (job);

	JOB_DEBUG (("Wait notify condition %p", job));
	/* Wait for the notify condition.  */
	g_cond_wait (job->notify_ack_condition, job->notify_ack_lock);

	JOB_DEBUG (("Unlock notify ack lock %p", job));
	/* Acknowledgment got: unlock the mutex.  */
	g_mutex_unlock (job->notify_ack_lock);

	JOB_DEBUG (("Done %p", job));
	return retval;
}

/* This closes the job.  */
static void
job_close (GnomeVFSJob *job)
{
	job->is_empty = TRUE;
	JOB_DEBUG (("Unlocking access lock %p", job));
	g_mutex_unlock (job->access_lock);
}

static gboolean
job_oneway_notify_and_close (GnomeVFSJob *job)
{
	gboolean retval;

	retval = job_oneway_notify (job);
	job_close (job);

	return retval;
}

static gboolean
job_notify_and_close (GnomeVFSJob *job)
{
	gboolean retval;

	retval = job_notify (job);
	job_close (job);

	return retval;
}

static void
dispatch_open_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncOpenCallback callback;
	GnomeVFSOpenOp *open_op;

	open_op = &op->specifics.open;

	callback = (GnomeVFSAsyncOpenCallback) op->callback;
	(* callback) ((GnomeVFSAsyncHandle *) job,
		      open_op->notify.result,
		      op->callback_data);
}

static void
dispatch_create_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncCreateCallback callback;
	GnomeVFSCreateOp *create_op;

	create_op = &op->specifics.create;

	callback = (GnomeVFSAsyncCreateCallback) op->callback;
	(* callback) ((GnomeVFSAsyncHandle *) job,
		      create_op->notify.result,
		      op->callback_data);
}

static void
dispatch_open_as_channel_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncOpenAsChannelCallback callback;
	GnomeVFSOpenAsChannelOp *open_as_channel_op;

	open_as_channel_op = &op->specifics.open_as_channel;

	callback = (GnomeVFSAsyncOpenAsChannelCallback) op->callback;
	(* callback) ((GnomeVFSAsyncHandle *) job,
		      open_as_channel_op->notify.channel,
		      open_as_channel_op->notify.result,
		      op->callback_data);
}

static void
dispatch_create_as_channel_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncCreateAsChannelCallback callback;
	GnomeVFSCreateAsChannelOp *create_as_channel_op;

	create_as_channel_op = &op->specifics.create_as_channel;

	callback = (GnomeVFSAsyncCreateAsChannelCallback) op->callback;
	(* callback) ((GnomeVFSAsyncHandle *) job,
		      create_as_channel_op->notify.channel,
		      create_as_channel_op->notify.result,
		      op->callback_data);
}

static void
dispatch_close_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncCloseCallback callback;
	GnomeVFSCloseOp *close_op;

	close_op = &op->specifics.close;

	callback = (GnomeVFSAsyncCloseCallback) op->callback;
	(* callback) ((GnomeVFSAsyncHandle *) job,
		      close_op->notify.result,
		      op->callback_data);
}

static void
dispatch_read_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncReadCallback callback;
	GnomeVFSReadOp *read_op;

	callback = (GnomeVFSAsyncReadCallback) op->callback;

	read_op = &op->specifics.read;

	(* callback) ((GnomeVFSAsyncHandle *) job,
		      read_op->notify.result,
		      read_op->request.buffer,
		      read_op->request.num_bytes,
		      read_op->notify.bytes_read,
		      op->callback_data);
}

static void
dispatch_write_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncWriteCallback callback;
	GnomeVFSWriteOp *write_op;

	callback = (GnomeVFSAsyncWriteCallback) op->callback;

	write_op = &op->specifics.write;

	(* callback) ((GnomeVFSAsyncHandle *) job,
		      write_op->notify.result,
		      write_op->request.buffer,
		      write_op->request.num_bytes,
		      write_op->notify.bytes_written,
		      op->callback_data);
}

static void
dispatch_load_directory_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncDirectoryLoadCallback callback;
	GnomeVFSLoadDirectoryOp *load_directory_op;

	load_directory_op = &op->specifics.load_directory;

	callback = (GnomeVFSAsyncDirectoryLoadCallback) op->callback;
	(* callback) ((GnomeVFSAsyncHandle *) job,
		      load_directory_op->notify.result,
		      load_directory_op->notify.list,
		      load_directory_op->notify.entries_read,
		      op->callback_data);
}

static void
dispatch_get_file_info_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncGetFileInfoCallback callback;
	GList *result_list;

	callback = (GnomeVFSAsyncGetFileInfoCallback) op->callback;
	result_list = op->specifics.get_file_info.notify.result_list;

	(* callback) ((GnomeVFSAsyncHandle *) job,
		      result_list,
		      op->callback_data);
}

static void
free_get_file_info_data (GnomeVFSOp *op)
{
	GList *result_list, *p;
	GnomeVFSGetFileInfoResult *result_item;

	gnome_vfs_uri_list_free (op->specifics.get_file_info.request.uris);

	result_list = op->specifics.get_file_info.notify.result_list;

	for (p = result_list; p != NULL; p = p->next) {
		result_item = p->data;

		gnome_vfs_uri_unref (result_item->uri);
		gnome_vfs_file_info_unref (result_item->file_info);
		g_free (result_item);
	}
	g_list_free (result_list);
}

static void
dispatch_find_directory_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncFindDirectoryCallback callback;
	GList *result_list;

	callback = (GnomeVFSAsyncFindDirectoryCallback) op->callback;
	result_list = op->specifics.find_directory.notify.result_list;

	(* callback) ((GnomeVFSAsyncHandle *) job,
		      result_list,
		      op->callback_data);
}

static void
free_find_directory_data (GnomeVFSOp *op)
{
	GList *result_list, *p;
	GnomeVFSFindDirectoryResult *result_item;

	gnome_vfs_uri_list_free (op->specifics.find_directory.request.uris);

	result_list = op->specifics.find_directory.notify.result_list;

	for (p = result_list; p != NULL; p = p->next) {
		result_item = p->data;

		if (result_item->uri != NULL) {
			gnome_vfs_uri_unref (result_item->uri);
		}
		g_free (result_item);
	}
	g_list_free (result_list);
}

static void
dispatch_set_file_info_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncSetFileInfoCallback callback;
	gboolean new_info_is_valid;
	
	new_info_is_valid = 
		op->specifics.set_file_info.notify.set_file_info_result == GNOME_VFS_OK
		&& op->specifics.set_file_info.notify.get_file_info_result == GNOME_VFS_OK;
	
	callback = (GnomeVFSAsyncSetFileInfoCallback) op->callback;
	
	(* callback) ((GnomeVFSAsyncHandle *) job,
		      op->specifics.set_file_info.notify.set_file_info_result,
		      new_info_is_valid 
		      ? &op->specifics.set_file_info.notify.info 
		      : NULL,
		      op->callback_data);
}

static void
dispatch_xfer_callback (GnomeVFSJob *job, GnomeVFSOp *op)
{
	GnomeVFSAsyncXferProgressCallback callback;
	GnomeVFSXferOp *xfer_op;
	gint callback_retval;

	callback = (GnomeVFSAsyncXferProgressCallback) op->callback;

	xfer_op = &op->specifics.xfer;

	callback_retval = (* callback) ((GnomeVFSAsyncHandle *) job,
					xfer_op->notify.progress_info,
					op->callback_data);

	xfer_op->notify_answer.value = callback_retval;
}

static void
close_callback (GnomeVFSAsyncHandle *handle,
		GnomeVFSResult result,
		gpointer callback_data)
{
}

static void
handle_cancelled_open (GnomeVFSJob *job, GnomeVFSOp *op)
{
	gnome_vfs_job_prepare (job, GNOME_VFS_OP_CLOSE,
			       (GFunc) close_callback, NULL);
	gnome_vfs_job_go (job);
}

static gboolean
dispatch_job_callback (GIOChannel *source,
                       GIOCondition condition,
                       gpointer data)
{
	GnomeVFSJob *job;
	GnomeVFSOp *op;
	gchar c;
	guint bytes_read;

	job = (GnomeVFSJob *) data;

	JOB_DEBUG (("waiting for channel wakeup %p", job));
	for (;;) {
		g_io_channel_read (job->wakeup_channel_in, &c, 1, &bytes_read);
		if (bytes_read > 0)
			break;
	}
	JOB_DEBUG (("got channel wakeup %p %c %d", job, c, bytes_read));

	op = job->notify_op;

	/* The last notify is the one that tells us to go away. */
	if (op == NULL) {
		JOB_DEBUG (("no op left %p", job));
		g_assert (job->current_op == NULL);
		g_assert (!job->want_notify_ack);
		job_ack_notify (job);
		gnome_vfs_job_finish_destroy (job);
		return FALSE;
	}
	
	JOB_DEBUG (("dispatching %p", job));
	/* Do the callback, but not if this operation has been cancelled. */

	if (gnome_vfs_context_check_cancellation (op->context)) {
		switch (op->type) {
		case GNOME_VFS_OP_CREATE:
			if (op->specifics.create.notify.result == GNOME_VFS_OK)
				handle_cancelled_open (job, op);
			break;
		case GNOME_VFS_OP_CREATE_AS_CHANNEL:
			if (op->specifics.create_as_channel.notify.result == GNOME_VFS_OK)
				handle_cancelled_open (job, op);
			break;
		case GNOME_VFS_OP_OPEN:
			if (op->specifics.open.notify.result == GNOME_VFS_OK)
				handle_cancelled_open (job, op);
			break;
		case GNOME_VFS_OP_OPEN_AS_CHANNEL:
			if (op->specifics.open_as_channel.notify.result == GNOME_VFS_OK)
				handle_cancelled_open (job, op);
			break;
		case GNOME_VFS_OP_CLOSE:
		case GNOME_VFS_OP_CREATE_SYMBOLIC_LINK:
		case GNOME_VFS_OP_FIND_DIRECTORY:
		case GNOME_VFS_OP_GET_FILE_INFO:
		case GNOME_VFS_OP_LOAD_DIRECTORY:
		case GNOME_VFS_OP_READ:
		case GNOME_VFS_OP_SET_FILE_INFO:
		case GNOME_VFS_OP_WRITE:
		case GNOME_VFS_OP_XFER:
			break;
		}
	} else {
		switch (op->type) {
		case GNOME_VFS_OP_CLOSE:
			dispatch_close_callback (job, op);
			break;
		case GNOME_VFS_OP_CREATE:
			dispatch_create_callback (job, op);
			break;
		case GNOME_VFS_OP_CREATE_AS_CHANNEL:
			dispatch_create_as_channel_callback (job, op);
			break;
		case GNOME_VFS_OP_CREATE_SYMBOLIC_LINK:
			dispatch_create_callback (job, op);
			break;
		case GNOME_VFS_OP_FIND_DIRECTORY:
			dispatch_find_directory_callback (job, op);
			break;
		case GNOME_VFS_OP_GET_FILE_INFO:
			dispatch_get_file_info_callback (job, op);
			break;
		case GNOME_VFS_OP_LOAD_DIRECTORY:
			dispatch_load_directory_callback (job, op);
			break;
		case GNOME_VFS_OP_OPEN:
			dispatch_open_callback (job, op);
			break;
		case GNOME_VFS_OP_OPEN_AS_CHANNEL:
			dispatch_open_as_channel_callback (job, op);
			break;
		case GNOME_VFS_OP_READ:
			dispatch_read_callback (job, op);
			break;
		case GNOME_VFS_OP_SET_FILE_INFO:
			dispatch_set_file_info_callback (job, op);
			break;
		case GNOME_VFS_OP_WRITE:
			dispatch_write_callback (job, op);
			break;
		case GNOME_VFS_OP_XFER:
			dispatch_xfer_callback (job, op);
			break;
		}
	}

	JOB_DEBUG (("dispatch callback - done %p", job));
	gnome_vfs_job_release_notify_op (job);
	job_ack_notify (job);
	return TRUE;
}

GnomeVFSJob *
gnome_vfs_job_new (void)
{
	GnomeVFSJob *new_job;
	gint pipefd[2];
	
	if (pipe (pipefd) != 0) {
		g_warning ("Cannot create pipe for the new GnomeVFSJob: %s",
			   g_strerror (errno));
		return NULL;
	}
	
	new_job = g_new0 (GnomeVFSJob, 1);
	
	new_job->access_lock = g_mutex_new ();
	new_job->execution_condition = g_cond_new ();
	new_job->notify_ack_condition = g_cond_new ();
	new_job->notify_ack_lock = g_mutex_new ();
	
	new_job->is_empty = TRUE;
	
	new_job->wakeup_channel_in = g_io_channel_unix_new (pipefd[0]);
	new_job->wakeup_channel_out = g_io_channel_unix_new (pipefd[1]);
	new_job->wakeup_channel_lock = g_mutex_new ();
	
	g_io_add_watch_full (new_job->wakeup_channel_in, G_PRIORITY_HIGH, G_IO_IN,
			     dispatch_job_callback, new_job, NULL);
	
	if (!gnome_vfs_job_create_slave (new_job)) {
		g_warning ("Cannot create job slave.");
		/* FIXME bugzilla.eazel.com 3833: A lot of leaked objects here. */
		g_free (new_job);
		return NULL;
	}
	
	JOB_DEBUG (("new job %p", new_job));

	job_count++;

	return new_job;
}

void
gnome_vfs_job_destroy (GnomeVFSJob *job)
{
	JOB_DEBUG (("job %p", job));

	gnome_vfs_job_release_current_op (job);

	job_oneway_notify (job);

	JOB_DEBUG (("done %p", job));
	/* We'll finish destroying on the main thread. */
}

static void
gnome_vfs_job_finish_destroy (GnomeVFSJob *job)
{
	g_assert (job->is_empty);

	g_mutex_free (job->access_lock);

	g_cond_free (job->execution_condition);

	g_cond_free (job->notify_ack_condition);
	g_mutex_free (job->notify_ack_lock);

	g_io_channel_close (job->wakeup_channel_in);
	g_io_channel_unref (job->wakeup_channel_in);
	g_io_channel_close (job->wakeup_channel_out);
	g_io_channel_unref (job->wakeup_channel_out);

	g_mutex_free (job->wakeup_channel_lock);

	JOB_DEBUG (("job %p terminated cleanly", job));

	g_free (job);

	job_count--;
}

int
gnome_vfs_job_get_count (void)
{
	return job_count;
}

static void
gnome_vfs_op_destroy (GnomeVFSOp *op)
{
	switch (op->type) {
	case GNOME_VFS_OP_CREATE:
		if (op->specifics.create.request.uri != NULL) {
			gnome_vfs_uri_unref (op->specifics.create.request.uri);
		}
		break;
	case GNOME_VFS_OP_CREATE_AS_CHANNEL:
		if (op->specifics.create_as_channel.request.uri != NULL) {
			gnome_vfs_uri_unref (op->specifics.create_as_channel.request.uri);
		}
		break;
	case GNOME_VFS_OP_CREATE_SYMBOLIC_LINK:
		gnome_vfs_uri_unref (op->specifics.create_symbolic_link.request.uri);
		g_free (op->specifics.create_symbolic_link.request.uri_reference);
		break;
	case GNOME_VFS_OP_FIND_DIRECTORY:
		free_find_directory_data (op);
		break;
	case GNOME_VFS_OP_GET_FILE_INFO:
		free_get_file_info_data (op);
		break;
	case GNOME_VFS_OP_LOAD_DIRECTORY:
		if (op->specifics.load_directory.request.uri != NULL) {
			gnome_vfs_uri_unref (op->specifics.load_directory.request.uri);
		}
		g_free (op->specifics.load_directory.request.sort_rules);
		g_free (op->specifics.load_directory.request.filter_pattern);
		break;
	case GNOME_VFS_OP_OPEN:
		if (op->specifics.open.request.uri != NULL) {
			gnome_vfs_uri_unref (op->specifics.open.request.uri);
		}
		break;
	case GNOME_VFS_OP_OPEN_AS_CHANNEL:
		if (op->specifics.open_as_channel.request.uri != NULL) {
			gnome_vfs_uri_unref (op->specifics.open_as_channel.request.uri);
		}
		break;
	case GNOME_VFS_OP_SET_FILE_INFO:
		gnome_vfs_uri_unref (op->specifics.set_file_info.request.uri);
		gnome_vfs_file_info_clear (&op->specifics.set_file_info.request.info);
		gnome_vfs_file_info_clear (&op->specifics.set_file_info.notify.info);
		break;
	case GNOME_VFS_OP_XFER:
		gnome_vfs_uri_list_free (op->specifics.xfer.request.source_uri_list);
		gnome_vfs_uri_list_free (op->specifics.xfer.request.target_uri_list);
		break;
	case GNOME_VFS_OP_CLOSE:
	case GNOME_VFS_OP_READ:
	case GNOME_VFS_OP_WRITE:
		break;
	default:
		g_warning (_("Unknown job ID %d"), op->type);
	}
	
	gnome_vfs_context_unref (op->context);
	g_free (op);
}

static void
gnome_vfs_job_release_current_op (GnomeVFSJob *job)
{
	if (job->current_op == NULL) {
		return;
	}
	if (job->current_op != job->notify_op) {
		gnome_vfs_op_destroy (job->current_op);
	}
	job->current_op = NULL;
}

static void
gnome_vfs_job_release_notify_op (GnomeVFSJob *job)
{
	if (job->current_op != job->notify_op) {
		gnome_vfs_op_destroy (job->notify_op);
	}
	job->notify_op = NULL;
}

void
gnome_vfs_job_prepare (GnomeVFSJob *job,
		       GnomeVFSOpType type,
		       GFunc callback,
		       gpointer callback_data)
{
	GnomeVFSOp *op;

	g_mutex_lock (job->access_lock);

	op = g_new (GnomeVFSOp, 1);
	op->type = type;
	op->callback = callback;
	op->callback_data = callback_data;
	op->context = gnome_vfs_context_new ();

	gnome_vfs_job_release_current_op (job);
	job->current_op = op;
	JOB_DEBUG (("%p %d", job, job->current_op->type));
}

void
gnome_vfs_job_go (GnomeVFSJob *job)
{
	job->is_empty = FALSE;
	g_cond_signal (job->execution_condition);
	g_mutex_unlock (job->access_lock);
}


#define DEFAULT_BUFFER_SIZE 16384

static void
serve_channel_read (GnomeVFSHandle *handle,
		    GIOChannel *channel_in,
		    GIOChannel *channel_out,
		    gulong advised_block_size,
		    GnomeVFSContext *context)
{
	gpointer buffer;
	guint filled_bytes_in_buffer;
	guint written_bytes_in_buffer;
	guint current_buffer_size;
	
	if (advised_block_size == 0)
		advised_block_size = DEFAULT_BUFFER_SIZE;

	current_buffer_size = advised_block_size;
	buffer = g_malloc(current_buffer_size);
	filled_bytes_in_buffer = 0;
	written_bytes_in_buffer = 0;

	while (1) {
		GnomeVFSResult result;
		GIOError io_result;
		GnomeVFSFileSize bytes_read;
		
	restart_toplevel_loop:
		
		g_assert(filled_bytes_in_buffer <= current_buffer_size);
		g_assert(written_bytes_in_buffer == 0);
		
		result = gnome_vfs_read_cancellable (handle,
						     (char *) buffer + filled_bytes_in_buffer,
						     MIN(advised_block_size, (current_buffer_size - filled_bytes_in_buffer)),
						     &bytes_read, context);

		if (result == GNOME_VFS_ERROR_CANCELLED)
			goto end;
		else if (result == GNOME_VFS_ERROR_INTERRUPTED)
			continue;
		else if (result != GNOME_VFS_OK)
			goto end;
		
		filled_bytes_in_buffer += bytes_read;
		
		if (filled_bytes_in_buffer == 0)
			goto end;

		g_assert(written_bytes_in_buffer <= filled_bytes_in_buffer);

		if (gnome_vfs_context_check_cancellation(context))
			goto end;
		
		while (written_bytes_in_buffer < filled_bytes_in_buffer) {
			guint bytes_written;
			
			/* channel_out is nonblocking; if we get
			   EAGAIN (G_IO_ERROR_AGAIN) then we tried to
			   write but the pipe was full. In this case, we
			   want to enlarge our buffer and go back to
			   reading for one iteration, so we can keep
			   collecting data while the main thread is
			   busy. */
			
			io_result = g_io_channel_write (channel_out,
							(char *) buffer + written_bytes_in_buffer,
							filled_bytes_in_buffer - written_bytes_in_buffer,
							&bytes_written);
			
			if (gnome_vfs_context_check_cancellation(context))
				goto end;
			
			if (io_result == G_IO_ERROR_AGAIN) {
				/* if bytes_read == 0 then we reached
				   EOF so there's no point reading
				   again. So turn off nonblocking and
				   do a blocking write next time through. */
				if (bytes_read == 0) {
					int fd;

					fd = g_io_channel_unix_get_fd (channel_out);
					
					clr_fl (fd, O_NONBLOCK);
				} else {
					if (written_bytes_in_buffer > 0) {
						/* Need to shift the unwritten bytes
						   to the start of the buffer */
						g_memmove(buffer,
							  (char *) buffer + written_bytes_in_buffer,
							  filled_bytes_in_buffer - written_bytes_in_buffer);
						filled_bytes_in_buffer =
							filled_bytes_in_buffer - written_bytes_in_buffer;
						
						written_bytes_in_buffer = 0;
					}
					
 				        /* If the buffer is more than half
					   full, double its size */
					if (filled_bytes_in_buffer*2 > current_buffer_size) {
						current_buffer_size *= 2;
						buffer = g_realloc(buffer, current_buffer_size);
					}

					/* Leave this loop, start reading again */
					goto restart_toplevel_loop;

				} /* end of else (bytes_read != 0) */
				
			} else if (io_result != G_IO_ERROR_NONE || bytes_written == 0) {
				goto end;
			}

			written_bytes_in_buffer += bytes_written;
		}

		g_assert(written_bytes_in_buffer == filled_bytes_in_buffer);
		
		/* Reset, we wrote everything */
		written_bytes_in_buffer = 0;
		filled_bytes_in_buffer = 0;
	}

 end:
	g_free (buffer);
	g_io_channel_close (channel_out);
	g_io_channel_unref (channel_out);
	g_io_channel_unref (channel_in);
}

static void
serve_channel_write (GnomeVFSHandle *handle,
		     GIOChannel *channel_in,
		     GIOChannel *channel_out,
		     GnomeVFSContext *context)
{
	gchar buffer[DEFAULT_BUFFER_SIZE];
	guint buffer_size;

	buffer_size = DEFAULT_BUFFER_SIZE;

	while (1) {
		GnomeVFSResult result;
		GIOError io_result;
		guint bytes_read;
		guint bytes_to_write;
		GnomeVFSFileSize bytes_written;
		gchar *p;

		io_result = g_io_channel_read (channel_in, buffer, buffer_size,
					       &bytes_read);
		if (io_result == G_IO_ERROR_AGAIN || io_result == G_IO_ERROR_UNKNOWN)
			/* we will get G_IO_ERROR_UNKNOWN if a signal occurrs */
			continue;
		if (io_result != G_IO_ERROR_NONE || bytes_read == 0)
			goto end;

		p = buffer;
		bytes_to_write = bytes_read;
		while (bytes_to_write > 0) {
			result = gnome_vfs_write_cancellable (handle,
							      p,
							      bytes_to_write,
							      &bytes_written,
							      context);
			if (result == GNOME_VFS_ERROR_INTERRUPTED)
				continue;
			if (result != GNOME_VFS_OK || bytes_written == 0)
				goto end;

			p += bytes_written;
			bytes_to_write -= bytes_written;
		}
	}

 end:
	g_io_channel_close (channel_in);
	g_io_channel_unref (channel_in);
	g_io_channel_unref (channel_out);
}

/* Job execution.  This is performed by the slave thread.  */

static gboolean
execute_open (GnomeVFSJob *job)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	GnomeVFSOpenOp *open_op;
	gboolean notify_retval;

	open_op = &job->current_op->specifics.open;

	if (open_op->request.uri == NULL) {
		result = GNOME_VFS_ERROR_INVALID_URI;
	} else {
		result = gnome_vfs_open_uri_cancellable (&handle, open_op->request.uri,
							 open_op->request.open_mode,
							 job->current_op->context);
		job->handle = handle;
	}
	open_op->notify.result = result;

	notify_retval = job_oneway_notify_and_close (job);

	if (result == GNOME_VFS_OK)
		return notify_retval;
	else
		return FALSE;
}

static gboolean
execute_open_as_channel (GnomeVFSJob *job)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	GnomeVFSOpenAsChannelOp *open_as_channel_op;
	GnomeVFSOpenMode open_mode;
	GIOChannel *channel_in, *channel_out;
	gint pipefd[2];

	open_as_channel_op = &job->current_op->specifics.open_as_channel;

	if (open_as_channel_op->request.uri == NULL) {
		result = GNOME_VFS_ERROR_INVALID_URI;
	} else {
		result = gnome_vfs_open_uri_cancellable
			(&handle,
			 open_as_channel_op->request.uri,
			 open_as_channel_op->request.open_mode,
			 job->current_op->context);
	}

	if (result != GNOME_VFS_OK) {
		open_as_channel_op->notify.channel = NULL;
		open_as_channel_op->notify.result = result;
		job_oneway_notify_and_close (job);
		return FALSE;
	}

	if (pipe (pipefd) < 0) {
		g_warning (_("Cannot create pipe for open GIOChannel: %s"),
			   g_strerror (errno));
		open_as_channel_op->notify.channel = NULL;
		open_as_channel_op->notify.result = GNOME_VFS_ERROR_INTERNAL;
		job_oneway_notify_and_close (job);
		return FALSE;
	}

	/* Set up the pipe for nonblocking writes, so if the main
	 * thread is blocking for some reason the slave can keep
	 * reading data.
	 */
	set_fl (pipefd[1], O_NONBLOCK);
	
	channel_in = g_io_channel_unix_new (pipefd[0]);
	channel_out = g_io_channel_unix_new (pipefd[1]);

	open_mode = open_as_channel_op->request.open_mode;
	
	if (open_mode & GNOME_VFS_OPEN_READ)
		open_as_channel_op->notify.channel = channel_in;
	else
		open_as_channel_op->notify.channel = channel_out;
	
	open_as_channel_op->notify.result = GNOME_VFS_OK;

	if (! job_notify (job))
		return FALSE;

	if (open_mode & GNOME_VFS_OPEN_READ)
		serve_channel_read (handle, channel_in, channel_out,
				    open_as_channel_op->request.advised_block_size,
				    job->current_op->context);
	else
		serve_channel_write (handle, channel_in, channel_out,
				     job->current_op->context);

	job_close (job);

	return FALSE;
}

static gboolean
execute_create (GnomeVFSJob *job)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	GnomeVFSCreateOp *create_op;
	gboolean notify_retval;

	create_op = &job->current_op->specifics.create;

	if (create_op->request.uri == NULL) {
		result = GNOME_VFS_ERROR_INVALID_URI;
	} else {
		result = gnome_vfs_create_uri_cancellable
			(&handle,
			 create_op->request.uri,
			 create_op->request.open_mode,
			 create_op->request.exclusive,
			 create_op->request.perm,
			 job->current_op->context);
		
		job->handle = handle;
	}
	create_op->notify.result = result;

	notify_retval = job_oneway_notify_and_close (job);

	if (result != GNOME_VFS_OK)
		return FALSE;

	return notify_retval;
}

static gboolean
execute_create_symbolic_link (GnomeVFSJob *job)
{
	GnomeVFSResult result;
	GnomeVFSCreateLinkOp *create_op;
	gboolean notify_retval;

	create_op = &job->current_op->specifics.create_symbolic_link;

	result = gnome_vfs_create_symbolic_link_cancellable
		(create_op->request.uri,
		 create_op->request.uri_reference,
		 job->current_op->context);

	create_op->notify.result = result;

	notify_retval = job_oneway_notify_and_close (job);

	if (result != GNOME_VFS_OK)
		return FALSE;

	return notify_retval;
}
	
static gboolean
execute_create_as_channel (GnomeVFSJob *job)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	GnomeVFSCreateAsChannelOp *create_as_channel_op;
	GIOChannel *channel_in, *channel_out;
	gint pipefd[2];

	create_as_channel_op = &job->current_op->specifics.create_as_channel;

	if (create_as_channel_op->request.uri == NULL) {
		result = GNOME_VFS_ERROR_INVALID_URI;
	} else {
		result = gnome_vfs_open_uri_cancellable
			(&handle,
			 create_as_channel_op->request.uri,
			 create_as_channel_op->request.open_mode,
			 job->current_op->context);
	}
	
	if (result != GNOME_VFS_OK) {
		create_as_channel_op->notify.channel = NULL;
		create_as_channel_op->notify.result = result;
		job_oneway_notify_and_close (job);
		return FALSE;
	}

	if (pipe (pipefd) < 0) {
		g_warning (_("Cannot create pipe for open GIOChannel: %s"),
			   g_strerror (errno));
		create_as_channel_op->notify.channel = NULL;
		create_as_channel_op->notify.result = GNOME_VFS_ERROR_INTERNAL;
		job_oneway_notify_and_close (job);
		return FALSE;
	}

	
	
	channel_in = g_io_channel_unix_new (pipefd[0]);
	channel_out = g_io_channel_unix_new (pipefd[1]);

	create_as_channel_op->notify.channel = channel_out;
	create_as_channel_op->notify.result = GNOME_VFS_OK;

	if (! job_notify (job))
		return FALSE;

	serve_channel_write (handle, channel_in, channel_out, job->current_op->context);

	job_close (job);

	return FALSE;
}

static gboolean
execute_close (GnomeVFSJob *job)
{
	GnomeVFSCloseOp *close_op;

	close_op = &job->current_op->specifics.close;

	close_op->notify.result
		= gnome_vfs_close_cancellable (job->handle, job->current_op->context);

	job_notify_and_close (job);

	return FALSE;
}

static gboolean
execute_read (GnomeVFSJob *job)
{
	GnomeVFSReadOp *read_op;

	read_op = &job->current_op->specifics.read;

	read_op->notify.result
		= gnome_vfs_read_cancellable (job->handle,
					      read_op->request.buffer,
					      read_op->request.num_bytes,
					      &read_op->notify.bytes_read,
					      job->current_op->context);

	return job_oneway_notify_and_close (job);
}

static gboolean
execute_write (GnomeVFSJob *job)
{
	GnomeVFSWriteOp *write_op;

	write_op = &job->current_op->specifics.write;

	write_op->notify.result
		= gnome_vfs_write_cancellable (job->handle,
					       write_op->request.buffer,
					       write_op->request.num_bytes,
					       &write_op->notify.bytes_written,
					       job->current_op->context);

	return job_oneway_notify_and_close (job);
}

static gboolean
execute_load_directory_not_sorted (GnomeVFSJob *job,
				   GnomeVFSDirectoryFilter *filter)
{
	GnomeVFSLoadDirectoryOp *load_directory_op;
	GnomeVFSDirectoryHandle *handle;
	GnomeVFSDirectoryList *directory_list;
	GnomeVFSFileInfo *info;
	GnomeVFSResult result;
	guint count;

	JOB_DEBUG (("%p", job));
	load_directory_op = &job->current_op->specifics.load_directory;

	if (load_directory_op->request.uri == NULL) {
		result = GNOME_VFS_ERROR_INVALID_URI;
	} else {
		result = gnome_vfs_directory_open_from_uri
			(&handle,
			 load_directory_op->request.uri,
			 load_directory_op->request.options,
			 filter);
	}

	if (result != GNOME_VFS_OK) {
		load_directory_op->notify.result = result;
		load_directory_op->notify.list = NULL;
		load_directory_op->notify.entries_read = 0;
		job_notify_and_close (job);
		return FALSE;
	}

	directory_list = gnome_vfs_directory_list_new ();
	load_directory_op->notify.list = directory_list;

	count = 0;
	while (1) {
		if (gnome_vfs_context_check_cancellation (job->current_op->context)) {
			JOB_DEBUG (("cancelled, bailing %p", job));
			result = GNOME_VFS_ERROR_CANCELLED;
			break;
		}
		
		info = gnome_vfs_file_info_new ();

		result = gnome_vfs_directory_read_next (handle, info);

		if (result == GNOME_VFS_OK) {
			gnome_vfs_directory_list_append (directory_list, info);
			count++;
		} else {
			gnome_vfs_file_info_unref (info);
		}

		if (count == load_directory_op->request.items_per_notification
		    || result != GNOME_VFS_OK) {
			load_directory_op->notify.result = result;
			load_directory_op->notify.entries_read = count;

			/* If we have not set a position yet, it means this is
                           the first iteration, so we must position on the
                           first element.  Otherwise, the last time we got here
                           we positioned on the last element with
                           `gnome_vfs_directory_list_last()', so we have to go
                           to the next one.  */
			if (gnome_vfs_directory_list_get_position
			    (directory_list) == NULL)
				gnome_vfs_directory_list_first (directory_list);
			else
				gnome_vfs_directory_list_next (directory_list);

			if (!job_notify (job)) {
				break;
			}
			

			if (result != GNOME_VFS_OK)
				break;

			count = 0;
			gnome_vfs_directory_list_last (directory_list);
		}
	}

	gnome_vfs_directory_list_destroy (directory_list);
	gnome_vfs_directory_close (handle);

	job_close (job);

	return FALSE;
}

static gboolean
execute_load_directory_sorted (GnomeVFSJob *job,
			       GnomeVFSDirectoryFilter *filter)
{
	GnomeVFSLoadDirectoryOp *load_directory_op;
	GnomeVFSDirectoryList *directory_list;
	GnomeVFSDirectoryListPosition previous_p, p;
	GnomeVFSResult result;
	guint count;

	JOB_DEBUG (("%p", job));
	load_directory_op = &job->current_op->specifics.load_directory;

	if (load_directory_op->request.uri == NULL) {
		result = GNOME_VFS_ERROR_INVALID_URI;
	} else {
		result = gnome_vfs_directory_list_load_from_uri
			(&directory_list,
			 load_directory_op->request.uri,
			 load_directory_op->request.options,
			 filter);
	}

	if (result != GNOME_VFS_OK) {
		load_directory_op->notify.result = result;
		load_directory_op->notify.list = NULL;
		load_directory_op->notify.entries_read = 0;
		job_notify (job);
		return FALSE;
	}

	gnome_vfs_directory_list_sort
		(directory_list,
		 load_directory_op->request.reverse_order,
		 load_directory_op->request.sort_rules);

	load_directory_op->notify.result = GNOME_VFS_OK;
	load_directory_op->notify.list = directory_list;

	count = 0;
	p = gnome_vfs_directory_list_get_first_position (directory_list);

	if (p == NULL) {
		load_directory_op->notify.result = GNOME_VFS_ERROR_EOF;
		load_directory_op->notify.entries_read = 0;
		job_notify (job);
		return FALSE;
	}

	previous_p = p;
	while (p != NULL) {
		count++;
		p = gnome_vfs_directory_list_position_next (p);
		if (p == NULL
		    || count == load_directory_op->request.items_per_notification) {
			gnome_vfs_directory_list_set_position (directory_list,
							       previous_p);
			if (p == NULL)
				load_directory_op->notify.result = GNOME_VFS_ERROR_EOF;
			else
				load_directory_op->notify.result = GNOME_VFS_OK;
			load_directory_op->notify.entries_read = count;
			if (!job_notify (job)) {
				break;
			}
			count = 0;
			previous_p = p;
		}
	}

	return FALSE;
}

static gboolean
execute_get_file_info (GnomeVFSJob *job)
{
	GnomeVFSGetFileInfoOp *gijob;
	GList *p;
	GnomeVFSGetFileInfoResult *result_item;

	gijob = &job->current_op->specifics.get_file_info;

	for (p = gijob->request.uris; p != NULL; p = p->next) {
		result_item = g_new (GnomeVFSGetFileInfoResult, 1);

		result_item->uri = gnome_vfs_uri_ref (p->data);
		result_item->file_info = gnome_vfs_file_info_new ();

		result_item->result = gnome_vfs_get_file_info_uri_cancellable
			(result_item->uri,
			 result_item->file_info,
			 gijob->request.options,
			 job->current_op->context);

		gijob->notify.result_list = g_list_prepend
			(gijob->notify.result_list, result_item);
	}
	gijob->notify.result_list = g_list_reverse (gijob->notify.result_list);

	job_oneway_notify_and_close (job);
	return FALSE;
}

static gboolean
execute_set_file_info (GnomeVFSJob *job)
{
	GnomeVFSSetFileInfoOp *op;
	GnomeVFSURI *parent_uri, *uri_after;

	op = &job->current_op->specifics.set_file_info;

	op->notify.set_file_info_result = gnome_vfs_set_file_info_cancellable
		(op->request.uri, &op->request.info, op->request.mask,
		 job->current_op->context);

	/* Get the new URI after the set_file_info. The name may have
	 * changed.
	 */
	uri_after = NULL;
	if (op->notify.set_file_info_result == GNOME_VFS_OK
	    && (op->request.mask & GNOME_VFS_SET_FILE_INFO_NAME) != 0) {
		parent_uri = gnome_vfs_uri_get_parent (op->request.uri);
		if (parent_uri != NULL) {
			uri_after = gnome_vfs_uri_append_file_name
				(parent_uri, op->request.info.name);
			gnome_vfs_uri_unref (parent_uri);
		}
	}
	if (uri_after == NULL) {
		uri_after = op->request.uri;
		gnome_vfs_uri_ref (uri_after);
	}

	/* Always get new file info, even if setter failed. Init here
	 * and clear in dispatch_set_file_info.
	 */
	gnome_vfs_file_info_init (&op->notify.info);
	if (uri_after == NULL) {
		op->notify.get_file_info_result = GNOME_VFS_ERROR_INVALID_URI;
	} else {
		op->notify.get_file_info_result = gnome_vfs_get_file_info_uri_cancellable
			(uri_after,
			 &op->notify.info,
			 op->request.options,
			 job->current_op->context);
		gnome_vfs_uri_unref (uri_after);
	}

	job_oneway_notify_and_close (job);
	return FALSE;
}

static gboolean
execute_find_directory (GnomeVFSJob *job)
{
	GnomeVFSFindDirectoryOp *op;
	GList *p;
	GnomeVFSGetFileInfoResult *result_item;

	op = &job->current_op->specifics.find_directory;
	for (p = op->request.uris; p != NULL; p = p->next) {
		result_item = g_new (GnomeVFSGetFileInfoResult, 1);

		result_item->result = gnome_vfs_find_directory_cancellable
			((GnomeVFSURI *) p->data,
			 op->request.kind,
			 &result_item->uri,
			 op->request.create_if_needed,
			 op->request.find_if_needed,
			 op->request.permissions,
			 job->current_op->context);
		op->notify.result_list = g_list_prepend (op->notify.result_list, result_item);
	}

	op->notify.result_list = g_list_reverse (op->notify.result_list);
	
	job_oneway_notify_and_close (job);
	return FALSE;
}

static gboolean
execute_load_directory (GnomeVFSJob *job)
{
	GnomeVFSLoadDirectoryOp *load_directory_op;
	GnomeVFSDirectorySortRule *sort_rules;
	GnomeVFSDirectoryFilter *filter;
	gboolean retval;

	load_directory_op = &job->current_op->specifics.load_directory;

	filter = gnome_vfs_directory_filter_new
		(load_directory_op->request.filter_type,
		 load_directory_op->request.filter_options,
		 load_directory_op->request.filter_pattern);

	sort_rules = load_directory_op->request.sort_rules;
	if (sort_rules == NULL
	    || sort_rules[0] == GNOME_VFS_DIRECTORY_SORT_NONE)
		retval = execute_load_directory_not_sorted (job, filter);
	else
		retval = execute_load_directory_sorted (job, filter);

	gnome_vfs_directory_filter_destroy (filter);

	job_close (job);

	return FALSE;
}

static gint
xfer_callback (GnomeVFSXferProgressInfo *info,
	       gpointer data)
{
	GnomeVFSJob *job;
	GnomeVFSXferOp *xfer_op;

	job = (GnomeVFSJob *) data;
	xfer_op = &job->current_op->specifics.xfer;
	xfer_op->notify.progress_info = info;

	/* Forward the callback to the master thread, which will fill in the
           `notify_answer' member appropriately.  */
	job_notify (job);

	/* Pass the value returned from the callback in the master thread.  */
	return xfer_op->notify_answer.value;
}

static gboolean
execute_xfer (GnomeVFSJob *job)
{
	GnomeVFSXferOp *xfer_op;
	GnomeVFSResult result;

	xfer_op = &job->current_op->specifics.xfer;

	result = gnome_vfs_xfer_private (xfer_op->request.source_uri_list,
					 xfer_op->request.target_uri_list,
					 xfer_op->request.xfer_options,
					 xfer_op->request.error_mode,
					 xfer_op->request.overwrite_mode,
					 xfer_callback,
					 job,
					 xfer_op->request.progress_sync_callback,
					 xfer_op->request.sync_callback_data);

	/* If the xfer functions returns an error now, something really bad
           must have happened.  */
	if (result != GNOME_VFS_OK && result != GNOME_VFS_ERROR_INTERRUPTED) {
		GnomeVFSXferProgressInfo info;


		info.status = GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR;
		info.vfs_status = result;
		info.phase = GNOME_VFS_XFER_PHASE_INITIAL;
		info.source_name = NULL;
		info.target_name = NULL;
		info.file_index = 0;
		info.files_total = 0;
		info.bytes_total = 0;
		info.file_size = 0;
		info.bytes_copied = 0;
		info.total_bytes_copied = 0;

		xfer_op->notify.progress_info = &info;

		job_notify (job);
	}

	job_close (job);

	return FALSE;
}

/* This function is called by the slave thread to execute a
   GnomeVFSJob.  */
gboolean
gnome_vfs_job_execute (GnomeVFSJob *job)
{
	JOB_DEBUG (("locking access_lock %p", job));
	g_mutex_lock (job->access_lock);
	if (job->is_empty) {
		JOB_DEBUG (("waiting for execution condition %p", job));
		g_cond_wait (job->execution_condition, job->access_lock);
	}

	JOB_DEBUG (("executing %p %d", job, job->current_op->type));

	switch (job->current_op->type) {
	case GNOME_VFS_OP_OPEN:
		return execute_open (job);
	case GNOME_VFS_OP_OPEN_AS_CHANNEL:
		return execute_open_as_channel (job);
	case GNOME_VFS_OP_CREATE:
		return execute_create (job);
	case GNOME_VFS_OP_CREATE_AS_CHANNEL:
		return execute_create_as_channel (job);
	case GNOME_VFS_OP_CREATE_SYMBOLIC_LINK:
		return execute_create_symbolic_link (job);
	case GNOME_VFS_OP_CLOSE:
		return execute_close (job);
	case GNOME_VFS_OP_READ:
		return execute_read (job);
	case GNOME_VFS_OP_WRITE:
		return execute_write (job);
	case GNOME_VFS_OP_LOAD_DIRECTORY:
		return execute_load_directory (job);
	case GNOME_VFS_OP_FIND_DIRECTORY:
		return execute_find_directory (job);
	case GNOME_VFS_OP_XFER:
		return execute_xfer (job);
	case GNOME_VFS_OP_GET_FILE_INFO:
		return execute_get_file_info (job);
	case GNOME_VFS_OP_SET_FILE_INFO:
		return execute_set_file_info (job);
	default:
		g_warning (_("Unknown job ID %d"), job->current_op->type);
		return FALSE;
	}
}


void
gnome_vfs_job_cancel (GnomeVFSJob *job)
{
	GnomeVFSOp *op;
	GnomeVFSCancellation *cancellation;

	JOB_DEBUG (("async cancel %p", job));

	g_return_if_fail (job != NULL);

	op = job->current_op;
	if (op == NULL) {
		op = job->notify_op;
	}

	g_return_if_fail (op != NULL);

	cancellation = gnome_vfs_context_get_cancellation (op->context);
	if (cancellation != NULL) {
		JOB_DEBUG (("cancelling %p", job));
		gnome_vfs_cancellation_cancel (cancellation);
	}

	/* handle the case when the job is stuck waiting in job_notify */
	JOB_DEBUG (("unlock job_notify %p", job));
	job_signal_ack_condition (job);
	
	gnome_vfs_context_emit_message (op->context, _("Operation stopped"));

	/* Since we are cancelling, we won't have anyone respond to notifications;
	 * set the expectations right.
	 */
	JOB_DEBUG (("done cancelling %p", job));
}
