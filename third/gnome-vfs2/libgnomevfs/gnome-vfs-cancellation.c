/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-cancellation.c - Cancellation handling for the GNOME Virtual File
   System access methods.

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

   Author: Ettore Perazzoli <ettore@gnu.org> */

#include <config.h>
#include "gnome-vfs-cancellation-private.h"
#include "GNOME_VFS_Daemon.h"

#include "gnome-vfs-utils.h"
#include "gnome-vfs-client.h"
#include "gnome-vfs-client-call.h"
#include <unistd.h>

/* WARNING: this code is not general-purpose.  It is supposed to make the two
   sides of the VFS (i.e. the main process/thread and its asynchronous slave)
   talk in a simple way.  For this reason, only the main process/thread should
   be allowed to call `gnome_vfs_cancellation_cancel()'.  *All* the code is
   based on this assumption.  */


struct GnomeVFSCancellation {
	gboolean cancelled;
	gint pipe_in;
	gint pipe_out;
	GnomeVFSClientCall *client_call;
};

G_LOCK_DEFINE_STATIC (pipes);
G_LOCK_DEFINE_STATIC (client_call);


/**
 * gnome_vfs_cancellation_new:
 * 
 * Create a new GnomeVFSCancellation object for reporting cancellation to a
 * GNOME VFS module.
 * 
 * Return value: A pointer to the new GnomeVFSCancellation object.
 **/
GnomeVFSCancellation *
gnome_vfs_cancellation_new (void)
{
	GnomeVFSCancellation *new;

	new = g_new (GnomeVFSCancellation, 1);
	new->cancelled = FALSE;
	new->pipe_in = -1;
	new->pipe_out = -1;
	new->client_call = NULL;

	return new;
}

/**
 * gnome_vfs_cancellation_destroy:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Destroy @cancellation.
 **/
void
gnome_vfs_cancellation_destroy (GnomeVFSCancellation *cancellation)
{
	g_return_if_fail (cancellation != NULL);

	if (cancellation->pipe_in >= 0) {
		close (cancellation->pipe_in);
		close (cancellation->pipe_out);
	}
	/* Can't have outstanding calls when destroying the cancellation */
	g_assert (cancellation->client_call == NULL);
	
	g_free (cancellation);
}

void
_gnome_vfs_cancellation_add_client_call (GnomeVFSCancellation *cancellation,
					 GnomeVFSClientCall *client_call)
{
	G_LOCK (client_call);
	/* Each client call uses its own context/cancellation */
	g_assert (cancellation->client_call == NULL);
	
	cancellation->client_call = client_call;
	G_UNLOCK (client_call);
}

void
_gnome_vfs_cancellation_remove_client_call (GnomeVFSCancellation *cancellation,
					    GnomeVFSClientCall *client_call)
{
	G_LOCK (client_call);
	g_assert (cancellation->client_call == client_call);
	
	cancellation->client_call = NULL;
	G_UNLOCK (client_call);
}

/**
 * gnome_vfs_cancellation_cancel:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Send a cancellation request through @cancellation.
 * Must be called on the main thread.
 **/
void
gnome_vfs_cancellation_cancel (GnomeVFSCancellation *cancellation)
{
	GNOME_VFS_AsyncDaemon daemon;
	GnomeVFSClient *client;
	GnomeVFSClientCall *client_call;

	g_return_if_fail (cancellation != NULL);

	GNOME_VFS_ASSERT_PRIMARY_THREAD;

	if (cancellation->cancelled)
		return;

	if (cancellation->pipe_out >= 0)
		write (cancellation->pipe_out, "c", 1);

	client_call = CORBA_OBJECT_NIL;
	G_LOCK (client_call);
	if (cancellation->client_call != NULL) {
		/* We need to delay the finishing of the client call to avoid
		 * the cancel call below to cancel the next call in the job thread
		 * if the job finishes after we drop the lock.
		 */
		_gnome_vfs_client_call_delay_finish (cancellation->client_call);
		client_call = cancellation->client_call;
		bonobo_object_ref (client_call);
	}
	G_UNLOCK (client_call);

	cancellation->cancelled = TRUE;

	if (client_call != NULL) {
		client = _gnome_vfs_get_client ();
		daemon = _gnome_vfs_client_get_async_daemon (client);

		GNOME_VFS_AsyncDaemon_Cancel (daemon, BONOBO_OBJREF (client_call), NULL);
		_gnome_vfs_client_call_delay_finish_done (client_call);
		bonobo_object_unref (client_call);
		CORBA_Object_release (daemon, NULL);
	}
}

/**
 * gnome_vfs_cancellation_check:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Check for pending cancellation.
 * 
 * Return value: %TRUE if the operation should be interrupted.
 **/
gboolean
gnome_vfs_cancellation_check (GnomeVFSCancellation *cancellation)
{
	if (cancellation == NULL)
		return FALSE;

	return cancellation->cancelled;
}

/**
 * gnome_vfs_cancellation_ack:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Acknowledge a cancellation.  This should be called if
 * `gnome_vfs_cancellation_check()' returns %TRUE or if `select()' reports that
 * input is available on the file descriptor returned by
 * `gnome_vfs_cancellation_get_fd()'.
 **/
void
gnome_vfs_cancellation_ack (GnomeVFSCancellation *cancellation)
{
	gchar c;

	/* ALEX: What the heck is this supposed to be used for?
	 * It seems totatlly wrong, and isn't used by anything.
	 * Also, the read() seems to block if it was cancelled before
	 * the pipe was gotten.
	 */
	
	if (cancellation == NULL)
		return;

	if (cancellation->pipe_in >= 0)
		read (cancellation->pipe_in, &c, 1);

	cancellation->cancelled = FALSE;
}

/**
 * gnome_vfs_cancellation_get_fd:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Get a file descriptor -based notificator for @cancellation.  When
 * @cancellation receives a cancellation request, a character will be made
 * available on the returned file descriptor for input.
 *
 * This is very useful for detecting cancellation during I/O operations: you
 * can use the `select()' call to check for available input/output on the file
 * you are reading/writing, and on the notificator's file descriptor at the
 * same time.  If a data is available on the notificator's file descriptor, you
 * know you have to cancel the read/write operation.
 * 
 * Return value: the notificator's file descriptor, or -1 if starved of
 *               file descriptors.
 **/
gint
gnome_vfs_cancellation_get_fd (GnomeVFSCancellation *cancellation)
{
	g_return_val_if_fail (cancellation != NULL, -1);

	G_LOCK (pipes);
	if (cancellation->pipe_in <= 0) {
		gint pipefd [2];

		if (pipe (pipefd) == -1) {
			G_UNLOCK (pipes);
			return -1;
		}

		cancellation->pipe_in = pipefd [0];
		cancellation->pipe_out = pipefd [1];
	}
	G_UNLOCK (pipes);

	return cancellation->pipe_in;
}
