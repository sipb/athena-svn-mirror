/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-server-launch.c - Launch the helper asynchronous
   process for the GNOME Virtual File System.

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#include "gnome-vfs-corba.h"
#include "gnome-vfs-slave.h"
#include "gnome-vfs-slave-launch.h"


struct _IorInfo {
	GMainLoop *loop;
	gchar *buffer;
	FILE *fd;
	guint buffer_size;
};
typedef struct _IorInfo IorInfo;

static gboolean
ior_watcher (GIOChannel *source,
	     GIOCondition condition,
	     gpointer data)
{
	IorInfo *ior_info;

	ior_info = (IorInfo *) data;

	if (condition & G_IO_IN) {
		if (fgets (ior_info->buffer, ior_info->buffer_size,
			   ior_info->fd) == NULL)
			ior_info->buffer[0] = 0;
	}

	g_main_quit (ior_info->loop);
	return FALSE;
}

/* Notice that this will close `fd' on exit.  */
static gchar *
get_ior (gint fd)
{
	GIOChannel *io_channel;
	IorInfo *ior_info;
	gchar *buffer;

	ior_info = g_new (IorInfo, 1);

	ior_info->loop = g_main_new (FALSE);
	ior_info->buffer_size = 4096;
	ior_info->buffer = g_malloc (ior_info->buffer_size);
	ior_info->buffer[0] = 0;
	ior_info->fd = fdopen (fd, "r");

	io_channel = g_io_channel_unix_new (fd);

	g_io_add_watch (io_channel, G_IO_IN | G_IO_ERR | G_IO_NVAL | G_IO_HUP,
			ior_watcher, ior_info);

	g_main_run (ior_info->loop);

	g_io_channel_unref (io_channel);
	g_main_destroy (ior_info->loop);

	buffer = ior_info->buffer;
	fclose (ior_info->fd);
	g_free (ior_info);
	
	g_strstrip (buffer);
	return buffer;
}


struct _ProcessData {
	gint ior_fd;
	gint pipe_fd;
};
typedef struct _ProcessData ProcessData;

static void
initialize_process (gpointer data)
{
	ProcessData *pdata;

	pdata = (ProcessData *) data;

	if (pdata->ior_fd != pdata->pipe_fd) {
		dup2 (pdata->pipe_fd, pdata->ior_fd);
		close (pdata->pipe_fd);
	}
}

GnomeVFSProcess *
gnome_vfs_slave_launch (GNOME_VFS_Slave_Notify notify_object,
			GNOME_VFS_Slave_Request *request_objref_return)
{
	GNOME_VFS_Slave_Request request_objref;
	GnomeVFSProcess *process;
	CORBA_Environment ev;
	CORBA_char *notify_ior;
	ProcessData *pdata;
	const gchar *args[4];
	gchar *ior, *ior_fd_string;
	gint ior_fd;
	gint pipe_fd[2];

	CORBA_exception_init (&ev);

	notify_ior = CORBA_ORB_object_to_string (gnome_vfs_orb, notify_object,
						 &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	if (pipe (pipe_fd) < 0) {
		g_warning ("Cannot create pipe for slave communication: %s.",
			   g_strerror (errno));
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	ior_fd = 123;		/* bogus */
	ior_fd_string = g_strdup_printf ("%d", ior_fd);

	pdata = g_new (ProcessData, 1);
	pdata->ior_fd = ior_fd;
	pdata->pipe_fd = pipe_fd[1];

	args[0] = "gnome-vfs-slave";
	args[1] = notify_ior;
	args[2] = ior_fd_string;
	args[3] = NULL;

	process = gnome_vfs_process_new (args[0], args,
					 GNOME_VFS_PROCESS_USEPATH,
					 initialize_process, pdata,
					 NULL, NULL);
	g_free (pdata);
	g_free (ior_fd_string);
	
	close (pipe_fd[1]);
	CORBA_free (notify_ior);

	ior = get_ior (pipe_fd[0]);
	if (strncmp (ior, "IOR:", 4) != 0) {
		g_warning (_("Got weird string from the slave process: `%s'"),
			   ior);
		goto error;
	}

	request_objref = CORBA_ORB_string_to_object (gnome_vfs_orb, ior, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning (_("Cannot get object for `%s'"), ior);
		goto error;
	}

	g_free (ior);

	*request_objref_return = request_objref;
	return process;

error:
	g_free (ior);
	CORBA_exception_free (&ev);
	*request_objref_return = CORBA_OBJECT_NIL;
	gnome_vfs_process_signal (process, SIGTERM);
	gnome_vfs_process_free (process);
	return NULL;
}
