/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-monitor.c - File Monitoring for the GNOME Virtual File System.

   Copyright (C) 2001 Ian McKellar

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

   Author: Ian McKellar <yakk@yakk.net>
*/

#include <libgnomevfs/gnome-vfs-monitor.h>
#include <libgnomevfs/gnome-vfs-monitor-private.h>
#include <libgnomevfs/gnome-vfs-method.h>
#include <glib.h>

struct GnomeVFSMonitorHandle {
	GnomeVFSURI *uri; /* the URI being monitored */
	GnomeVFSMethodHandle *method_handle;
	GnomeVFSMonitorType type;
	GnomeVFSMonitorCallback callback;
	gpointer user_data; /* FIXME - how does this get freed */

	gboolean cancelled;
	GList *pending_callbacks;
};

struct GnomeVFSMonitorCallbackData {
	GnomeVFSMonitorHandle *monitor_handle;
	char *info_uri;
	GnomeVFSMonitorEventType event_type;
};

typedef struct GnomeVFSMonitorCallbackData GnomeVFSMonitorCallbackData;

/* This hash maps the module-supplied handle pointer to our own MonitrHandle */
static GHashTable *handle_hash = NULL;
G_LOCK_DEFINE_STATIC (handle_hash);

static void 
init_hash_table (void)
{
	G_LOCK (handle_hash);

	if (handle_hash == NULL) {
		handle_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
	}

	G_UNLOCK (handle_hash);
}

GnomeVFSResult
_gnome_vfs_monitor_do_add (GnomeVFSMethod *method,
			  GnomeVFSMonitorHandle **handle,
			  GnomeVFSURI *uri,
                          GnomeVFSMonitorType monitor_type,
			  GnomeVFSMonitorCallback callback,
			  gpointer user_data)
{
	GnomeVFSResult result;
	GnomeVFSMonitorHandle *monitor_handle = 
		g_new0(GnomeVFSMonitorHandle, 1);

	init_hash_table ();
	gnome_vfs_uri_ref (uri);
	monitor_handle->uri = uri;

	monitor_handle->type = monitor_type;
	monitor_handle->callback = callback;
	monitor_handle->user_data = user_data;

	result = uri->method->monitor_add (uri->method, 
			&monitor_handle->method_handle, uri, monitor_type);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_uri_unref (uri);
		g_free (monitor_handle);
		monitor_handle = NULL;
	} else {
		G_LOCK (handle_hash);
		g_hash_table_insert (handle_hash, 
				monitor_handle->method_handle,
				monitor_handle);
		G_UNLOCK (handle_hash);
	}

	*handle = monitor_handle;

	return result;
}

static void
destroy_monitor_handle (GnomeVFSMonitorHandle *handle) {
	G_LOCK (handle_hash);
	g_hash_table_remove (handle_hash, handle->method_handle);
	G_UNLOCK (handle_hash);

	gnome_vfs_uri_unref (handle->uri);
	g_free (handle);
}

GnomeVFSResult 
_gnome_vfs_monitor_do_cancel (GnomeVFSMonitorHandle *handle)
{
	GnomeVFSResult result;

	init_hash_table ();

	if (!VFS_METHOD_HAS_FUNC(handle->uri->method, monitor_cancel)) {
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

	result = handle->uri->method->monitor_cancel (handle->uri->method,
			handle->method_handle);

	if (result != GNOME_VFS_OK) {
		/* mark this monitor as cancelled */
		handle->cancelled = TRUE;

		/* destroy the handle if there are no outstanding callbacks */
		if (handle->pending_callbacks == NULL) {
			destroy_monitor_handle (handle);
		}
	}

	return result;
}

static gint
actually_dispatch_callback (gpointer data)
{
	GnomeVFSMonitorCallbackData *callback_data = data;
	gchar *uri;

	if (!callback_data->monitor_handle->cancelled) {
		uri = gnome_vfs_uri_to_string 
			(callback_data->monitor_handle->uri, 
			 GNOME_VFS_URI_HIDE_NONE);

		/* actually run app code */
		callback_data->monitor_handle->callback(
				callback_data->monitor_handle, uri,
				callback_data->info_uri, 
				callback_data->event_type,
				callback_data->monitor_handle->user_data);

		g_free (uri);
	}

	G_LOCK (handle_hash);

	/* remove the callback from the pending queue */
	callback_data->monitor_handle->pending_callbacks = g_list_remove
		(callback_data->monitor_handle->pending_callbacks, 
		 callback_data);

	/* if we were waiting for this callback to be dispatched to free
	 * this monitor, then do it now.
	 */
	if (callback_data->monitor_handle->cancelled &&
			callback_data->monitor_handle->pending_callbacks ==
								NULL) {
		destroy_monitor_handle (callback_data->monitor_handle);
	}

	/* free the callback_data */
	g_free (callback_data->info_uri);
	g_free (callback_data);

	G_UNLOCK (handle_hash);

	return FALSE;
}

/* for modules to send callbacks to the app */
void
gnome_vfs_monitor_callback (GnomeVFSMethodHandle *method_handle,
                            GnomeVFSURI *info_uri, /* GList of uris */
                            GnomeVFSMonitorEventType event_type)
{
	GnomeVFSMonitorCallbackData *callback_data =
		g_new0(GnomeVFSMonitorCallbackData, 1);
	GnomeVFSMonitorHandle *monitor_handle;

	init_hash_table ();

	G_LOCK (handle_hash);
	monitor_handle = g_hash_table_lookup (handle_hash, method_handle);
	g_assert (monitor_handle != NULL);
	callback_data->monitor_handle = monitor_handle;
	if (info_uri != NULL) {
		callback_data->info_uri = gnome_vfs_uri_to_string
			(info_uri, GNOME_VFS_URI_HIDE_NONE);
	} else {
		callback_data->info_uri = NULL;
	}
	callback_data->event_type = event_type;
	monitor_handle->pending_callbacks = 
		g_list_append(monitor_handle->pending_callbacks, callback_data);
	G_UNLOCK (handle_hash);

	g_idle_add (actually_dispatch_callback, callback_data);

}
