/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   gnome-vfs-mime-monitor.c: Class for noticing changes in MIME data.
 
   Copyright (C) 2000 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Authors: John Sullivan <sullivan@eazel.com>,
*/

#include <config.h>
#include "gnome-vfs-mime-monitor.h"
#include "gnome-vfs-mime-private.h"
#include "gnome-vfs-ops.h"

enum {
	DATA_CHANGED,
	LAST_SIGNAL
};

enum {
	LOCAL_MIME_DIR,
	GNOME_MIME_DIR,
};

static guint signals[LAST_SIGNAL];

static GnomeVFSMIMEMonitor *global_mime_monitor = NULL;

typedef struct _MonitorCallbackData
{
	GnomeVFSMIMEMonitor *monitor;
	gint type;
} MonitorCallbackData;

struct _GnomeVFSMIMEMonitorPrivate
{
	GnomeVFSMonitorHandle *global_handle;
	GnomeVFSMonitorHandle *local_handle;

	/* The hoops I jump through */
	MonitorCallbackData *gnome_callback_data;
	MonitorCallbackData *local_callback_data;
};


static void                   gnome_vfs_mime_monitor_class_init              (GnomeVFSMIMEMonitorClass *klass);
static void                   gnome_vfs_mime_monitor_init                    (GnomeVFSMIMEMonitor      *monitor);
static void                   mime_dir_changed_callback                      (GnomeVFSMonitorHandle    *handle,
									      const gchar              *monitor_uri,
									      const gchar              *info_uri,
									      GnomeVFSMonitorEventType  event_type,
									      gpointer                  user_data);
static void                   gnome_vfs_mime_monitor_finalize                (GObject                  *object);



static void
gnome_vfs_mime_monitor_class_init (GnomeVFSMIMEMonitorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gnome_vfs_mime_monitor_finalize;

	signals [DATA_CHANGED] = 
		g_signal_new ("data_changed",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GnomeVFSMIMEMonitorClass, data_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
}

static void
gnome_vfs_mime_monitor_init (GnomeVFSMIMEMonitor *monitor)
{
	gchar *mime_dir;

	monitor->priv = g_new (GnomeVFSMIMEMonitorPrivate, 1);

	monitor->priv->gnome_callback_data = g_new (MonitorCallbackData, 1);
	monitor->priv->local_callback_data = g_new (MonitorCallbackData, 1);

	/* FIXME: Bug #80268.  These wouldn't be private members if we had a
	 * _full variant.  However, if I want to clean them up, I need to keep
	 * them around. */
	monitor->priv->gnome_callback_data->type = GNOME_MIME_DIR;
	monitor->priv->gnome_callback_data->monitor = monitor;
	monitor->priv->local_callback_data->type = LOCAL_MIME_DIR;
	monitor->priv->local_callback_data->monitor = monitor;

	mime_dir = g_strdup (DATADIR "/mime-info");
	gnome_vfs_monitor_add (&monitor->priv->global_handle,
			       mime_dir,
			       GNOME_VFS_MONITOR_DIRECTORY,
			       mime_dir_changed_callback,
			       monitor->priv->gnome_callback_data);
	g_free (mime_dir);

	mime_dir = g_strconcat (g_get_home_dir (), "/.gnome/mime-info", NULL);
	gnome_vfs_monitor_add (&monitor->priv->local_handle,
			       mime_dir,
			       GNOME_VFS_MONITOR_DIRECTORY,
			       mime_dir_changed_callback,
			       monitor->priv->local_callback_data);
	g_free (mime_dir);
}


static void
mime_dir_changed_callback (GnomeVFSMonitorHandle    *handle,
			   const gchar              *monitor_uri,
			   const gchar              *info_uri,
			   GnomeVFSMonitorEventType  event_type,
			   gpointer                  user_data)
{
	MonitorCallbackData *monitor_callback_data = (MonitorCallbackData *)user_data;

	if (monitor_callback_data->type == GNOME_MIME_DIR)
		_gnome_vfs_mime_info_mark_gnome_mime_dir_dirty ();
	else if (monitor_callback_data->type == LOCAL_MIME_DIR)
		_gnome_vfs_mime_info_mark_user_mime_dir_dirty ();
		
	gnome_vfs_mime_monitor_emit_data_changed (monitor_callback_data->monitor);
}

static void
gnome_vfs_mime_monitor_finalize (GObject *object)
{
	gnome_vfs_monitor_cancel (GNOME_VFS_MIME_MONITOR (object)->priv->global_handle);
	gnome_vfs_monitor_cancel (GNOME_VFS_MIME_MONITOR (object)->priv->local_handle);
	g_free (GNOME_VFS_MIME_MONITOR (object)->priv->gnome_callback_data);
	g_free (GNOME_VFS_MIME_MONITOR (object)->priv->local_callback_data);
	g_free (GNOME_VFS_MIME_MONITOR (object)->priv);
}

/* Return a pointer to the single global monitor. */
GnomeVFSMIMEMonitor *
gnome_vfs_mime_monitor_get (void)
{
        if (global_mime_monitor == NULL) {
		global_mime_monitor = GNOME_VFS_MIME_MONITOR
			(g_object_new (gnome_vfs_mime_monitor_get_type (), NULL));
        }
        return global_mime_monitor;
}


void
gnome_vfs_mime_monitor_emit_data_changed (GnomeVFSMIMEMonitor *monitor)
{
	g_return_if_fail (GNOME_VFS_IS_MIME_MONITOR (monitor));

	g_signal_emit (G_OBJECT (monitor),
		       signals [DATA_CHANGED], 0);
}

GType
gnome_vfs_mime_monitor_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (GnomeVFSMIMEMonitorClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_vfs_mime_monitor_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (GnomeVFSMIMEMonitor),
			0, /* n_preallocs */
			(GInstanceInitFunc) gnome_vfs_mime_monitor_init
		};
		
		type = g_type_register_static (
			G_TYPE_OBJECT, "GnomeVFSMIMEMonitor", &info, 0);
	}

	return type;
}
