/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-slave.c - Slave process for asynchronous operation of the
   GNOME Virtual File System, with CORBA-based IPC.

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

#include <glib.h>
#include <orb/orbit.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#include "gnome-vfs-slave.h"

#include <liboaf/liboaf.h>


/*  #define SLAVE_DEBUG */

#ifdef SLAVE_DEBUG
#define DPRINTF(x)					\
G_STMT_START{						\
	printf ("gnome-vfs-slave %d: %s, %d: ",		\
		getpid (), __FUNCTION__, __LINE__);	\
	printf x;					\
	putchar ('\n');					\
	fflush (stdout);				\
}G_STMT_END
#else
#define DPRINTF(x)
#endif


static gchar *program_name = NULL;

static GNOME_VFS_Slave_Request request_objref = CORBA_OBJECT_NIL;
static GNOME_VFS_Slave_Notify notify_objref = CORBA_OBJECT_NIL;

static PortableServer_ServantBase__epv Request_base_epv;
static POA_GNOME_VFS_Slave_Request__epv Request_epv;
static POA_GNOME_VFS_Slave_Request__vepv Request_vepv;

struct _RequestServant {
	POA_GNOME_VFS_Slave_Request servant;
	PortableServer_POA poa;
};
typedef struct _RequestServant RequestServant;

static PortableServer_ServantBase__epv FileHandle_base_epv;
static POA_GNOME_VFS_Slave_FileHandle__epv FileHandle_epv;
static POA_GNOME_VFS_Slave_FileHandle__vepv FileHandle_vepv;

struct _FileHandleServant {
	POA_GNOME_VFS_Slave_FileHandle servant;
	GnomeVFSHandle *handle;
};
typedef struct _FileHandleServant FileHandleServant;

static GList *file_handle_servants = NULL;


static void
error (const gchar *s, ...)
{
	va_list ap;

	va_start (ap, s);
	fputs (program_name, stderr);
	fputs (": ", stderr);
	vfprintf (stderr, s, ap);
	fputc ('\n', stderr);
}

static void
set_corba_string (CORBA_char **dest, const gchar *src)
{
	guint len;

	CORBA_free (*dest);

	if (src == NULL)
		len = 0;
	else
		len = strlen (src);

	*dest = CORBA_string_alloc (len);

	if (src == NULL)
		**dest = 0;
	else
		memcpy (*dest, src, len + 1);
}

/* This is somewhat dangerous.  Basically, the only calls that we are allowed
   to get in here are `stop' or `die', otherwise everything will be messed up.
   But we trust the master-side of the library to check for this.  */
static void
dispatch_pending_invocations (void)
{
	while (g_main_pending ())
		g_main_iteration (TRUE);
}


/* Operation control.  When we do long operations, we check if the master has
   requested the operation to be stopped and, if so, we stop.  */

/* This is `TRUE' if an operation is in progress.  */
static gboolean operation_in_progress = FALSE;

/* This variable is set to `TRUE' if the master requests the current operation
   to be stopped.  */
static gboolean stop_operation = FALSE;

static void
long_operation_started (void)
{
	operation_in_progress = TRUE;
}

static void
long_operation_finished (void)
{
	operation_in_progress = FALSE;
	stop_operation = FALSE;
}

static gboolean
check_stop (void)
{
	gboolean retval;

	dispatch_pending_invocations ();

	retval = stop_operation;
	stop_operation = FALSE;

	return retval;
}

static gboolean
stop_long_operation (void)
{
	if (operation_in_progress)
		stop_operation = TRUE;

	return stop_operation;
}


/* FileHandle object handling.  */

static GNOME_VFS_Slave_FileHandle
create_FileHandle (PortableServer_POA poa,
		   GnomeVFSHandle *handle,
		   CORBA_Environment *ev)
{
	FileHandleServant *servant;

	servant = g_new0 (FileHandleServant, 1);
	servant->servant.vepv = &FileHandle_vepv;
	servant->handle = handle;

	POA_GNOME_VFS_Slave_FileHandle__init ((PortableServer_Servant) servant,
					      ev);

	if (ev->_major != CORBA_NO_EXCEPTION){
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}

	CORBA_free (PortableServer_POA_activate_object (poa, servant, ev));

	file_handle_servants = g_list_prepend (file_handle_servants, servant);

	return PortableServer_POA_servant_to_reference (poa, servant, ev);
}

static void
destroy_FileHandle (FileHandleServant *servant,
		    CORBA_Environment *ev)
{
	POA_GNOME_VFS_Slave_FileHandle__fini
		((POA_GNOME_VFS_Slave_FileHandle *) servant, ev);
	g_free (servant);
}

static void
destroy_all_FileHandles (CORBA_Environment *ev)
{
	GList *p;

	for (p = file_handle_servants; p != NULL; p = p->next) {
		FileHandleServant *servant;

		servant = p->data;
		gnome_vfs_close (servant->handle);
		destroy_FileHandle (servant, ev);
	}

	g_list_free (file_handle_servants);
}


/* FileHandle interface methods.  */

static void
impl_FileHandle_read (PortableServer_Servant servant,
		      const CORBA_unsigned_long count,
		      CORBA_Environment *ev)
{
	GNOME_VFS_Buffer buffer;
	GnomeVFSHandle *handle;
	GnomeVFSResult result;
	GnomeVFSFileOffset bytes_read;

	handle = ((FileHandleServant *) servant)->handle;

	buffer._buffer = alloca(count);
	buffer._length = count;
	CORBA_sequence_set_release (&buffer, FALSE);

	result = gnome_vfs_read (handle, buffer._buffer, count, &bytes_read);

	buffer._length = bytes_read;

	GNOME_VFS_Slave_Notify_read (notify_objref,
				     (GNOME_VFS_Result) result,
				     &buffer,
				     ev);
}

static void
impl_FileHandle_write (PortableServer_Servant servant,
		       const GNOME_VFS_Buffer *buffer,
		       CORBA_Environment *ev)
{
	GnomeVFSHandle *handle;
	GnomeVFSResult result;
	GnomeVFSFileOffset bytes_written;

	handle = ((FileHandleServant *) servant)->handle;
	result = gnome_vfs_write (handle, buffer->_buffer, buffer->_length,
				  &bytes_written);

	GNOME_VFS_Slave_Notify_write (notify_objref,
				      (GNOME_VFS_Result) result,
				      (CORBA_unsigned_long) bytes_written,
				      ev);
}

static void
impl_FileHandle_close (PortableServer_Servant servant,
		       CORBA_Environment *ev)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;

	handle = ((FileHandleServant *) servant)->handle;
	result = gnome_vfs_close (handle);

	DPRINTF (("Notify close"));

	GNOME_VFS_Slave_Notify_close (notify_objref, (GNOME_VFS_Result) result,
				      ev);

	if (ev->_major == CORBA_NO_EXCEPTION)
		DPRINTF (("Done"));
	else
		DPRINTF (("Broken"));

	destroy_FileHandle (servant, ev);
}

static void
init_FileHandle (void)
{
	FileHandle_base_epv._private = NULL;
	FileHandle_base_epv.finalize = NULL;
	FileHandle_base_epv.default_POA = NULL;

	FileHandle_epv.read = impl_FileHandle_read;
	FileHandle_epv.write = impl_FileHandle_write;
	FileHandle_epv.close = impl_FileHandle_close;

	FileHandle_vepv._base_epv = &FileHandle_base_epv;
	FileHandle_vepv.GNOME_VFS_Slave_FileHandle_epv = &FileHandle_epv;
}


/* Basic services of the Request interface.  */

static gboolean
idle_die (gpointer data)
{
	CORBA_Environment ev;

	DPRINTF ((_("Dying."), getpid ()));

	CORBA_exception_init (&ev);
	GNOME_VFS_Slave_Notify_dying (notify_objref, &ev);
	CORBA_exception_free (&ev);

	exit (0);

	return FALSE;		/* Shut up stupid compilers.  */
}

static void
impl_Request_die (PortableServer_Servant servant,
		  CORBA_Environment *ev)
{
	DPRINTF (("Requested to die."));

	POA_GNOME_VFS_Slave_Request__fini
		((POA_GNOME_VFS_Slave_Request *) servant, ev);

	g_idle_add (idle_die, NULL);
}

/* This requests the current operation to be stopped.  If no operation is in
   progress, we just dispatch the `stop' notification so that the master knows
   we got it.  */
static void
impl_Request_stop (PortableServer_Servant servant,
		   CORBA_Environment *ev)
{
	DPRINTF (("Requested to stop."));

	if (! stop_long_operation ())
		GNOME_VFS_Slave_Notify_stop (notify_objref, ev);
}

static gboolean
idle_reset (gpointer data)
{
	CORBA_Environment ev;

	DPRINTF (("Resetting."));

	CORBA_exception_init (&ev);

	destroy_all_FileHandles (&ev);
	GNOME_VFS_Slave_Notify_reset (notify_objref, &ev);

	CORBA_exception_free (&ev);

	return FALSE;
}

static void
impl_Request_reset (PortableServer_Servant servant,
		    CORBA_Environment *ev)
{
	DPRINTF (("Requested to reset."));

	impl_Request_stop (servant, ev);
	g_idle_add (idle_reset, NULL);
}

static void
impl_Request_open (PortableServer_Servant servant,
		   const CORBA_char * uri,
		   GNOME_VFS_OpenMode open_mode,
		   CORBA_Environment *ev)
{
	GnomeVFSHandle *handle;
	GnomeVFSResult result;
	GNOME_VFS_Slave_FileHandle file_handle_objref;

	DPRINTF (("Opening file"));

	result = gnome_vfs_open (&handle, uri, open_mode);

	if (result == GNOME_VFS_OK) {
		PortableServer_POA poa;

		DPRINTF (("Creating FileHandle object"));
		poa = ((RequestServant *) servant)->poa;
		file_handle_objref = create_FileHandle (poa, handle, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			return;
		DPRINTF (("All ok."));
	} else {
		DPRINTF (("Couldn't open file: %s",
			  gnome_vfs_result_to_string (result)));
		file_handle_objref = CORBA_OBJECT_NIL;
	}

	DPRINTF (("Sending notification"));

	GNOME_VFS_Slave_Notify_open (notify_objref,
				     (GNOME_VFS_Result) result,
				     file_handle_objref,
				     ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		DPRINTF (("Error %d", ev->_major));
	else
		DPRINTF (("All ok."));
}

static void
impl_Request_create (PortableServer_Servant servant,
		     const CORBA_char *uri,
		     GNOME_VFS_OpenMode open_mode,
		     CORBA_boolean exclusive,
		     GNOME_VFS_Permission perm,
		     CORBA_Environment *ev)
{
	GNOME_VFS_Slave_FileHandle file_handle_objref;
	GnomeVFSHandle *handle;
	GnomeVFSResult result;

	result = gnome_vfs_create (&handle, uri, open_mode, exclusive, perm);

	if (result == GNOME_VFS_OK) {
		PortableServer_POA poa;

		poa = ((RequestServant *) servant)->poa;
		file_handle_objref = create_FileHandle (poa, handle, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			return;
	} else {
		file_handle_objref = CORBA_OBJECT_NIL;
	}

	GNOME_VFS_Slave_Notify_open (notify_objref,
				     (GNOME_VFS_Result) result,
				     file_handle_objref,
				     ev);
}


#define DEFAULT_BUFFER_SIZE 16384

static void
serve_channel_read (GnomeVFSHandle *handle,
		    gint fd,
		    GnomeVFSOpenMode open_mode,
		    gulong advised_block_size)
{
	gpointer buffer;

	if (advised_block_size == 0)
		advised_block_size = DEFAULT_BUFFER_SIZE;

	buffer = alloca (advised_block_size);

	while (1) {
		GnomeVFSResult result;
		GnomeVFSFileSize bytes_read;
		gulong bytes_to_write;
		gchar *p;

		result = gnome_vfs_read (handle, buffer, advised_block_size,
					 &bytes_read);
		if (result == GNOME_VFS_ERROR_INTERRUPTED)
			continue;

		if (result != GNOME_VFS_OK) {
			/* FIXME bugzilla.eazel.com 1226: */
			g_warning (_("Error reading: %s"),
				   gnome_vfs_result_to_string (gnome_vfs_result_from_errno ()));
			return;
		}

		if (bytes_read == 0) {
			DPRINTF (("Done reading file."));
			break;
		}

		bytes_to_write = bytes_read;
		p = (gchar *) buffer;

		while (bytes_to_write > 0) {
			gint bytes_written;

			bytes_written = write (fd, p, bytes_to_write);
			if (bytes_written < 0) {
				if (errno == EINTR)
					continue;

				/* FIXME bugzilla.eazel.com 1226: */
				g_warning (_("Error writing: %s"),
					   g_strerror (errno));
				return;
			}

			p += bytes_written;
			bytes_to_write -= bytes_written;
		}
	}
}

static void
serve_channel_write (GnomeVFSHandle *handle,
		     gint fd,
		     GnomeVFSOpenMode open_mode)
{
	gpointer buffer;
	guint buffer_size;

	buffer_size = DEFAULT_BUFFER_SIZE;
	buffer = alloca (buffer_size);

	while (1) {
		gint bytes_read;
		gulong bytes_to_write;
		gchar *p;

		bytes_read = read (fd, buffer, buffer_size);

		if (bytes_read == 0) {
			DPRINTF (("Done writing file."));
			return;
		}

		bytes_to_write = bytes_read;
		p = (gchar *) buffer;

		while (bytes_to_write > 0) {
			GnomeVFSFileSize bytes_written;
			GnomeVFSResult result;

			result = gnome_vfs_write (handle, p, bytes_to_write,
						  &bytes_written);

			if (result == GNOME_VFS_ERROR_INTERRUPTED)
				continue;

			if (result != GNOME_VFS_OK) {
				/* FIXME bugzilla.eazel.com 1226: */
				g_warning (_("Cannot write: %s"),
					   g_strerror (errno));
				return;
			}

			bytes_to_write -= bytes_written;
			p += bytes_written;
		}
	}
}

static void
setup_and_serve_channel (GnomeVFSHandle *handle,
			 GnomeVFSOpenMode open_mode,
			 glong advised_block_size,
			 CORBA_Environment *ev)
{
	GnomeVFSResult result;
	struct sockaddr_un saddr;
	struct sockaddr caller_addr;
	int caller_addr_size;
	guint size;
	gint socket_fd, fd;
	gchar socket_name[] = "/tmp/gnome-vfs-XXXXXX";

	socket_fd = -1;

	if (mktemp (socket_name) == NULL) {
		g_warning (_("Cannot create temporary file name `%s'"),
			   gnome_vfs_result_to_string(gnome_vfs_result_from_errno()));
		result = GNOME_VFS_ERROR_INTERNAL;
		goto error;
	}

#if defined (PF_LOCAL)
	socket_fd = socket (PF_LOCAL, SOCK_STREAM, 0);
#elif defined (PF_UNIX)
	socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
#else
#error
#endif
	if (socket_fd < 0) {
		g_warning (_("Cannot create socket: %s"), g_strerror (errno));
		result = GNOME_VFS_ERROR_INTERNAL;
		goto error;
	}

#if defined (AF_LOCAL)
	saddr.sun_family = AF_LOCAL;
#elif defined (AF_UNIX)
	saddr.sun_family = AF_UNIX;
#else
#error
#endif

	strncpy (saddr.sun_path, socket_name, sizeof (saddr.sun_path));
	size = (offsetof (struct sockaddr_un, sun_path)
		+ strlen (saddr.sun_path) + 1);

	if (bind (socket_fd, (struct sockaddr *) &saddr, size) < 0) {
		g_warning (_("Cannot bind `%s': %s"),
			   socket_name, g_strerror (errno));
		result = GNOME_VFS_ERROR_INTERNAL;
		goto error;
	}

	if (listen (socket_fd, 1) < 0) {
		g_warning (_("Cannot listen on `%s': %s"),
			   socket_name, g_strerror (errno));
		result = GNOME_VFS_ERROR_INTERNAL;
		goto error;
	}

	DPRINTF (("Socket `%s' ready to accept connections...\n", socket_name));

	GNOME_VFS_Slave_Notify_open_as_channel (notify_objref,
						(GNOME_VFS_Result) GNOME_VFS_OK,
						socket_name, ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		unlink (socket_name);
		close (socket_fd);
		return;
	}

	DPRINTF (("Waiting for connection on `%s'.\n", socket_name));

	fd = accept (socket_fd, (struct sockaddr *) &caller_addr,
		     &caller_addr_size);
	if (fd < 0) {
		g_warning (_("Cannot accept connections on `%s': %s"),
			   socket_name, g_strerror (errno));
		result = GNOME_VFS_ERROR_INTERNAL;
		goto error;
	}

	DPRINTF (("Connection established, now performing operation.\n"));

	if (open_mode & GNOME_VFS_OPEN_READ)
		serve_channel_read (handle, fd, open_mode, advised_block_size);
	else
		serve_channel_write (handle, fd, open_mode);

	DPRINTF (("Closing file descriptors.\n"));

	gnome_vfs_close (handle);
	close (fd);
	close (socket_fd);

	unlink (socket_name);

	return;

error:
	if (socket_fd != -1)
		close (socket_fd);
	unlink (socket_name);
	GNOME_VFS_Slave_Notify_open_as_channel (notify_objref,
						(GNOME_VFS_Result) result,
						"", ev);
	return;
}

static void
impl_Request_open_as_channel (PortableServer_Servant servant,
			      const CORBA_char *uri,
			      const GNOME_VFS_OpenMode open_mode,
			      const CORBA_unsigned_long advised_block_size,
			      CORBA_Environment *ev)
{
	GnomeVFSHandle *handle;
	GnomeVFSResult result;

	DPRINTF (("Opening file for channel"));

	result = gnome_vfs_open (&handle, uri, open_mode);

	if (result != GNOME_VFS_OK) {
		DPRINTF (("Couldn't open file: %s",
			  gnome_vfs_result_to_string (result)));
		GNOME_VFS_Slave_Notify_open_as_channel
			                             (notify_objref,
					              (GNOME_VFS_Result) result,
						      "", ev);
		return;
	}

	setup_and_serve_channel (handle, open_mode, advised_block_size, ev);
}

static void
impl_Request_create_as_channel (PortableServer_Servant servant,
				const CORBA_char *uri,
				GNOME_VFS_OpenMode open_mode,
				CORBA_boolean exclusive,
				GNOME_VFS_Permission perm,
				CORBA_Environment *ev)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;

	DPRINTF (("Opening file for channel"));

	result = gnome_vfs_create (&handle, uri, open_mode, exclusive, perm);

	if (result != GNOME_VFS_OK) {
		DPRINTF (("Couldn't open file: %s",
			  gnome_vfs_result_to_string (result)));
		GNOME_VFS_Slave_Notify_open_as_channel
			                             (notify_objref,
						      (GNOME_VFS_Result) result,
						      "", ev);
		return;
	}

	setup_and_serve_channel (handle, open_mode, 0, ev);
}

/* Directory reading.  */

static GNOME_VFS_Slave_FileInfoList *
allocate_info_list (gulong size)
{
	GNOME_VFS_Slave_FileInfoList *list;
	guint i;

	list = GNOME_VFS_Slave_FileInfoList__alloc ();

	list->_length = 0;
	list->_maximum = size;
	list->_buffer = CORBA_sequence_GNOME_VFS_Slave_FileInfo_allocbuf (size);
	CORBA_sequence_set_release (list, TRUE);

	for (i = 0; i < size; i++) {
		GNOME_VFS_Slave_FileInfo *p;
		guint info_size;

		p = list->_buffer + i;

		info_size = sizeof (GnomeVFSFileInfo);
		p->data._length = info_size;
		p->data._maximum = info_size;
        p->data._buffer
		= CORBA_sequence_CORBA_octet_allocbuf (info_size);
        CORBA_sequence_set_release (&p->data, TRUE);

        p->name = NULL;
        p->symlink_name = NULL;
        p->mime_type = NULL;

	}

	return list;
}


static void
copy_file_info (GNOME_VFS_Slave_FileInfo *dest,
		GnomeVFSFileInfo *src)
{
	memcpy (dest->data._buffer, src, dest->data._length);

	set_corba_string (&dest->name, src->name);
	set_corba_string (&dest->symlink_name, src->symlink_name);
	set_corba_string (&dest->mime_type, src->mime_type);

}

static void
load_directory_not_sorted (const gchar *uri,
			   GnomeVFSFileInfoOptions options,
			   GnomeVFSDirectoryFilter *filter,
			   GNOME_VFS_Slave_FileInfoList *list_buffer,
			   CORBA_Environment *ev)
{
	GnomeVFSDirectoryHandle *handle;
	GnomeVFSResult result;
	GnomeVFSFileInfo *info;
	gboolean stopped;

	result = gnome_vfs_directory_open (&handle, 
					   uri, 
					   options,
					   filter);

	if (result != GNOME_VFS_OK) {
		GNOME_VFS_Slave_Notify_load_directory
			(notify_objref, result, list_buffer, ev);
		return;
	}

	stopped = FALSE;
	long_operation_started ();

	info = gnome_vfs_file_info_new ();
	while (1) {
		if (check_stop ()) {
			stopped = TRUE;
			break;
		}

		result = gnome_vfs_directory_read_next (handle, info);

		if (result == GNOME_VFS_OK) {
			GNOME_VFS_Slave_FileInfo *i;

			i = list_buffer->_buffer + list_buffer->_length;
			copy_file_info (i, info);
			list_buffer->_length++;
		}

		if (result != GNOME_VFS_OK
		    || list_buffer->_length == list_buffer->_maximum) {
			DPRINTF (("Notifying load_directory"));
			GNOME_VFS_Slave_Notify_load_directory
				(notify_objref, result, list_buffer, ev);
			list_buffer->_length = 0;
			DPRINTF (("Notifying done"));
			if (result != GNOME_VFS_OK)
				break;
		}
	}

	gnome_vfs_directory_close (handle);

	long_operation_finished ();

	if (stopped)
		GNOME_VFS_Slave_Notify_stop (notify_objref, ev);

	gnome_vfs_file_info_unref (info);
}

static void
load_directory_sorted (const gchar *uri,
		       GnomeVFSFileInfoOptions options,
		       GnomeVFSDirectoryFilter *filter,
		       const GnomeVFSDirectorySortRule *rules,
		       gboolean reverse_order,
		       GNOME_VFS_Slave_FileInfoList *list_buffer,
		       CORBA_Environment *ev)
{
	GnomeVFSDirectoryList *list;
	GnomeVFSResult result;
	GnomeVFSFileInfo *info;
	gboolean stopped;

	list_buffer->_length = 0;

	result = gnome_vfs_directory_list_load (&list,
						uri,
						options,
						filter);

	if (result != GNOME_VFS_OK) {
		GNOME_VFS_Slave_Notify_load_directory (notify_objref, result,
						       list_buffer, ev);
		return;
	}

	stopped = FALSE;
	long_operation_started ();

	gnome_vfs_directory_list_sort (list, reverse_order, rules);

	info = gnome_vfs_directory_list_first (list);
	while (info != NULL) {
		GNOME_VFS_Slave_FileInfo *i;

		if (check_stop ()) {
			stopped = TRUE;
			break;
		}

		i = list_buffer->_buffer + list_buffer->_length;
		copy_file_info (i, info);
		list_buffer->_length++;

		info = gnome_vfs_directory_list_next (list);
		DPRINTF (("***SLAVE*** Notifying `%s'\n",
			  i->name));
		if (info == NULL) {
			GNOME_VFS_Slave_Notify_load_directory
				(notify_objref, GNOME_VFS_ERROR_EOF,
				 list_buffer, ev);
		} else if (list_buffer->_length == list_buffer->_maximum) {
			GNOME_VFS_Slave_Notify_load_directory
				(notify_objref, GNOME_VFS_OK, list_buffer, ev);
			list_buffer->_length = 0;
		}

#if 0
		if (ev->_major != CORBA_NO_EXCEPTION) {
			/*  printf ("***SLAVE*** error sending notification.\n"); */
			fflush (stdout);
		}
#endif
	}

	long_operation_finished ();

	if (stopped)
		GNOME_VFS_Slave_Notify_stop (notify_objref, ev);

	gnome_vfs_directory_list_destroy (list);
}

static void
impl_Request_get_file_info (PortableServer_Servant servant,
			    const GNOME_VFS_Slave_URIList *uris,
			    const GNOME_VFS_Slave_FileInfoOptions info_options,
			    CORBA_Environment *ev)
{
	GnomeVFSResult result;
	int i;
	GnomeVFSFileInfo *info;
	GNOME_VFS_Slave_FileInfo *p;
	guint info_size;
	GNOME_VFS_Slave_GetFileInfoResultList *result_list;


	result_list = GNOME_VFS_Slave_GetFileInfoResultList__alloc ();
	result_list->_length = uris->_length;
	result_list->_maximum = uris->_length;
	result_list->_buffer = CORBA_sequence_GNOME_VFS_Slave_GetFileInfoResult_allocbuf (uris->_length);
	CORBA_sequence_set_release (result_list, TRUE);

	for (i = 0; i < uris->_length; i++) {
		info = gnome_vfs_file_info_new ();
		result = gnome_vfs_get_file_info (uris->_buffer[i], info, info_options);
		
		result_list->_buffer[i].uri = CORBA_string_dup (uris->_buffer[i]);
		result_list->_buffer[i].result = result;
		p = &result_list->_buffer[i].file_info;

		info_size = sizeof (GnomeVFSFileInfo);
		p->data._length = info_size;
		p->data._maximum = info_size;
		p->data._buffer = alloca (info_size);
		CORBA_sequence_set_release (&p->data, FALSE);
		
		p->name = CORBA_string_dup (info->name?info->name:"");
		p->symlink_name = CORBA_string_dup (info->symlink_name?info->symlink_name:"");
		p->mime_type = CORBA_string_dup (info->mime_type?info->mime_type:"");
		
	}

	GNOME_VFS_Slave_Notify_get_file_info (notify_objref, result_list, ev);

	CORBA_free (result_list);
}

static void
impl_Request_load_directory (PortableServer_Servant servant,
			     const CORBA_char *uri,
			     const GNOME_VFS_Slave_FileInfoOptions info_options,
			     const GNOME_VFS_Slave_DirectoryFilter *filter,
			     const GNOME_VFS_Slave_DirectorySortRuleList *sort_rules,
			     const CORBA_boolean reverse_order,
			     const CORBA_unsigned_long items_per_notification,
			     CORBA_Environment *ev)
{
	GnomeVFSDirectoryFilter *my_filter;
	GnomeVFSDirectorySortRule *my_sort_rules;
	GNOME_VFS_Slave_FileInfoList *list_buffer;
	guint i;

	my_filter = gnome_vfs_directory_filter_new (filter->type,
						    filter->options,
						    filter->pattern);

	list_buffer = allocate_info_list (items_per_notification);

	if (sort_rules->_length == 0) {
		load_directory_not_sorted (uri, info_options,
					   my_filter, list_buffer, ev);
	} else {
		my_sort_rules = alloca (sizeof (gchar *) * sort_rules->_length);
		for (i = 0; i < sort_rules->_length; i++)
			my_sort_rules[i] = sort_rules->_buffer[i];

		load_directory_sorted (uri, info_options,
				       my_filter, my_sort_rules, reverse_order,
				       list_buffer, ev);
	}

	CORBA_free (list_buffer);

	gnome_vfs_directory_filter_destroy (my_filter);
}


static GList *
file_list_to_uri_g_list (const GNOME_VFS_Slave_FileNameList *file_list)
{
	GList *result;
	guint i;

	result = NULL;

	i = file_list->_length;
	while (i > 0) {
		i--;
		result = g_list_prepend (result, gnome_vfs_uri_new (file_list->_buffer[i]));
	}

	return result;
}

static gint
xfer_progress_ok (const GnomeVFSXferProgressInfo *info,
		  CORBA_Environment *ev)
{
	CORBA_boolean retval;

	switch (info->phase) {
	case GNOME_VFS_XFER_PHASE_READYTOGO:
		retval = GNOME_VFS_Slave_Notify_xfer_start (notify_objref,
							    info->files_total,
							    info->bytes_total,
							    ev);
		break;
	case GNOME_VFS_XFER_PHASE_COPYING:
		if (info->bytes_copied == 0)
			retval = GNOME_VFS_Slave_Notify_xfer_file_start
				(notify_objref,
				 info->source_name, info->target_name,
				 info->file_size,
				 ev);
		else
			retval = GNOME_VFS_Slave_Notify_xfer_file_progress
				(notify_objref,
				 info->bytes_copied, info->total_bytes_copied,
				 ev);
		break;
	case GNOME_VFS_XFER_PHASE_FILECOMPLETED:
		retval = GNOME_VFS_Slave_Notify_xfer_file_done (notify_objref, ev);
		break;
	case GNOME_VFS_XFER_PHASE_COMPLETED:
		GNOME_VFS_Slave_Notify_xfer_done (notify_objref, ev);
		retval = TRUE;
		break;
	case GNOME_VFS_XFER_PHASE_MOVING:
	case GNOME_VFS_XFER_PHASE_OPENSOURCE:
	case GNOME_VFS_XFER_PHASE_OPENTARGET:
	case GNOME_VFS_XFER_PHASE_READSOURCE:
	case GNOME_VFS_XFER_PHASE_WRITETARGET:
	case GNOME_VFS_XFER_PHASE_CLOSESOURCE:
	case GNOME_VFS_XFER_PHASE_CLOSETARGET:
	case GNOME_VFS_XFER_PHASE_SETATTRIBUTES:
	case GNOME_VFS_XFER_PHASE_INITIAL:
	case GNOME_VFS_XFER_PHASE_COLLECTING:
	default:
		retval = TRUE;
		break;
	}

	if (ev->_major == CORBA_NO_EXCEPTION)
		return TRUE;
	else
		return retval;
}

static gint
xfer_progress_vfs_error (const GnomeVFSXferProgressInfo *info,
			 CORBA_Environment *ev)
{
	GnomeVFSXferErrorAction action;

	action = GNOME_VFS_Slave_Notify_xfer_query_for_error
		(notify_objref, info->vfs_status, info->phase, ev);

	return action;
}

static gint
xfer_progress_ovewrite (const GnomeVFSXferProgressInfo *info,
			CORBA_Environment *ev)
{
	GnomeVFSXferOverwriteAction action;

	action = GNOME_VFS_Slave_Notify_xfer_query_for_overwrite
		(notify_objref, info->source_name, info->target_name, ev);

	return action;
}

static gint
xfer_progress_callback (GnomeVFSXferProgressInfo *info,
			gpointer data)
{
	CORBA_Environment *ev;

	ev = (CORBA_Environment *) data;

	if (check_stop ()) {
		GNOME_VFS_Slave_Notify_stop (notify_objref, ev);
		return 0;
	}

	switch (info->status) {
	case GNOME_VFS_XFER_PROGRESS_STATUS_OK:
		return xfer_progress_ok (info, ev);
	case GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR:
		return xfer_progress_vfs_error (info, ev);
	case GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE:
		return xfer_progress_ovewrite (info, ev);
	default:
		g_warning ("Unknown GnomeVFSXferProgressStatus %d",
			   info->status);
		return 0;
	}
}

static void
impl_Request_xfer (PortableServer_Servant servant,
		   const GNOME_VFS_Slave_URIList *source_names,
		   const GNOME_VFS_Slave_URIList *target_names,
		   const GNOME_VFS_Slave_XferOptions options,
		   const GNOME_VFS_Slave_XferOverwriteMode overwrite_mode,
		   CORBA_Environment *ev)
{
	GnomeVFSResult result;
	GList *source_uri_list, *target_uri_list;

	source_uri_list = file_list_to_uri_g_list (source_names);
	target_uri_list = file_list_to_uri_g_list (target_names);

	operation_in_progress = TRUE;

	result = gnome_vfs_xfer_uri_list (source_uri_list,
					  target_uri_list,
					  options,
					  GNOME_VFS_XFER_ERROR_MODE_QUERY,
					  overwrite_mode,
					  xfer_progress_callback,
					  ev);

	operation_in_progress = FALSE;
	stop_operation = FALSE;

	gnome_vfs_uri_list_free (source_uri_list);
	gnome_vfs_uri_list_free (target_uri_list);

	if (result != GNOME_VFS_OK && result != GNOME_VFS_ERROR_INTERRUPTED)
		GNOME_VFS_Slave_Notify_xfer_error (notify_objref,
						   result,
						   ev);
}


static void
init_Request (void)
{
	Request_base_epv._private = NULL;
	Request_base_epv.finalize = NULL;
	Request_base_epv.default_POA = NULL;

	Request_epv.die = impl_Request_die;
	Request_epv.reset = impl_Request_reset;
	Request_epv.stop = impl_Request_stop;
	Request_epv.open = impl_Request_open;
	Request_epv.open_as_channel = impl_Request_open_as_channel;
	Request_epv.create = impl_Request_create;
	Request_epv.create_as_channel = impl_Request_create_as_channel;
	Request_epv.get_file_info = impl_Request_get_file_info;
	Request_epv.load_directory = impl_Request_load_directory;
	Request_epv.xfer = impl_Request_xfer;

	Request_vepv._base_epv = &Request_base_epv;
	Request_vepv.GNOME_VFS_Slave_Request_epv = &Request_epv;
}

static GNOME_VFS_Slave_Request
create_Request (PortableServer_POA poa,
		CORBA_Environment *ev)
{
	RequestServant *servant;

	servant = g_new0 (RequestServant, 1);
	servant->servant.vepv = &Request_vepv;
	servant->poa = poa;

	POA_GNOME_VFS_Slave_Request__init ((PortableServer_Servant) servant,
					   ev);
	if (ev->_major != CORBA_NO_EXCEPTION){
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}

	CORBA_free (PortableServer_POA_activate_object (poa, servant, ev));

	return PortableServer_POA_servant_to_reference (poa, servant, ev);
}


static glong
get_max_fds (void)
{
#if defined _SC_OPEN_MAX
	return sysconf (_SC_OPEN_MAX);
#elif defined RLIMIT_NOFILE
	{
		struct rlimit rlimit;

		if (getrlimit (RLIMIT_NOFILE, &rlimit) == 0)
			return rlimit.rlim_max;
		else
			return -1;
	}
#elif defined HAVE_GETDTABLESIZE
	return getdtablesize();
#else
#warning Cannot determine the number of available file descriptors
	return 1024;		/* bogus */
#endif
}

/* Close all the descriptors, avoiding `except_fd', stdin, stdout and
   stderr.  */
static void
shut_down_file_descriptors (guint except_fd)
{
	glong i, max_fds;

	max_fds = get_max_fds ();

	for (i = 3; i < max_fds; i++) {
		if (i != except_fd)
			close (i);
	}
}


static gboolean
init_corba (int *argc,
	    char **argv,
	    CORBA_ORB *orb_return,
	    PortableServer_POA *poa_return,
	    CORBA_Environment *ev)
{
	CORBA_ORB orb;
	PortableServer_POA poa;

	orb = oaf_init (*argc, argv);

	if (ev->_major != CORBA_NO_EXCEPTION) {
		error (_("Cannot initialize CORBA."));
		return FALSE;
	}

	poa = (PortableServer_POA)
		CORBA_ORB_resolve_initial_references (orb, "RootPOA", ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		error (_("Cannot resolve initial reference to RootPOA."));
		return FALSE;
	}

	PortableServer_POAManager_activate
		(PortableServer_POA__get_the_POAManager (poa, ev), ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		error (_("Cannot activate POA manager."));
		return FALSE;
	}

	*orb_return = orb;
	*poa_return = poa;

	return TRUE;
}

int
main (int argc, char **argv)
{
	CORBA_Environment ev;
	CORBA_ORB orb;
	PortableServer_POA poa;
	GMainLoop *main_loop;
	gchar *notify_ior;
	CORBA_char *request_ior;
	FILE *ior_fd;

	putenv("GNOME_DISABLE_CRASH_DIALOG=1");
	program_name = g_strdup (argv[0]);

	if (argc < 2 || argc > 3) {
		fprintf (stderr, _("Usage: %s <ior> [<ior_fd>]\n"), argv[0]);
		return 1;
	}

	notify_ior = argv[1];
	if (argc > 2) {
		gint fd;

		fd = atoi (argv[2]);
		ior_fd = fdopen (fd, "a");
		if (ior_fd == NULL) {
			error (_("Cannot open file descriptor %d."), fd);
			return 1;
		}
	} else {
		ior_fd = stdout;
	}

	shut_down_file_descriptors (fileno (ior_fd));

	CORBA_exception_init (&ev);

	main_loop = g_main_new (FALSE);

	if (! init_corba (&argc, argv, &orb, &poa, &ev)) {
		CORBA_exception_free (&ev);
		return 1;
	}

	gnome_vfs_init ();

	notify_objref = CORBA_ORB_string_to_object (orb, notify_ior, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		CORBA_exception_free (&ev);
		error (_("Notify interface for `%s' not found."), notify_ior);
		return 1;
	}

	init_FileHandle ();

	init_Request ();
	request_objref = create_Request (poa, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		CORBA_exception_free (&ev);
		error (_("Cannot setup Request object."));
		return 1;
	}

	request_ior = CORBA_ORB_object_to_string (orb, request_objref, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		CORBA_exception_free (&ev);
		error (_("Cannot extract IOR."));
		return 1;
	}

	fprintf (ior_fd, "%s\n", request_ior);
	fclose (ior_fd);

	CORBA_free (request_ior);

	g_main_run (main_loop);

	return 0;
}
