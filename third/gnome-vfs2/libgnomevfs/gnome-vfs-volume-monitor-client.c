/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-volume-monitor-client.c - client implementation of volume handling

   Copyright (C) 2003 Red Hat, Inc

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

   Author: Alexander Larsson <alexl@redhat.com>
*/

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libbonobo.h>

#include "gnome-vfs-volume-monitor-client.h"
#include "gnome-vfs-volume-monitor-private.h"
#include "gnome-vfs-cdrom.h"
#include "gnome-vfs-filesystem-type.h"
#include "gnome-vfs-client.h"

static void gnome_vfs_volume_monitor_client_class_init (GnomeVFSVolumeMonitorClientClass *klass);
static void gnome_vfs_volume_monitor_client_init       (GnomeVFSVolumeMonitorClient      *volume_monitor_client);
static void gnome_vfs_volume_monitor_client_finalize   (GObject                          *object);


static GnomeVFSVolumeMonitorClass *parent_class = NULL;

GType
gnome_vfs_volume_monitor_client_get_type (void)
{
	static GType volume_monitor_client_type = 0;

	if (!volume_monitor_client_type) {
		static const GTypeInfo volume_monitor_client_info = {
			sizeof (GnomeVFSVolumeMonitorClientClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) gnome_vfs_volume_monitor_client_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (GnomeVFSVolumeMonitorClient),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gnome_vfs_volume_monitor_client_init
		};
		
		volume_monitor_client_type =
			g_type_register_static (GNOME_VFS_TYPE_VOLUME_MONITOR, "GnomeVFSVolumeMonitorClient",
						&volume_monitor_client_info, 0);
	}
	
	return volume_monitor_client_type;
}

static void
gnome_vfs_volume_monitor_client_class_init (GnomeVFSVolumeMonitorClientClass *class)
{
	GObjectClass *o_class;
        CORBA_Environment ev;
	GNOME_VFS_Daemon daemon;
	GnomeVFSClient *client;
	
	parent_class = g_type_class_peek_parent (class);
	
	o_class = (GObjectClass *) class;

	client = _gnome_vfs_get_client ();
	daemon = _gnome_vfs_client_get_daemon (client);

	if (daemon != CORBA_OBJECT_NIL) {
		CORBA_exception_init (&ev);
		GNOME_VFS_Daemon_registerVolumeMonitor (daemon, BONOBO_OBJREF (client), &ev);
		
		if (BONOBO_EX (&ev)) {
			CORBA_exception_free (&ev);
		}
		
		CORBA_Object_release (daemon, NULL);
	}
	
	/* GObject signals */
	o_class->finalize = gnome_vfs_volume_monitor_client_finalize;
}

static void
read_drives_from_daemon (GnomeVFSVolumeMonitorClient *volume_monitor_client)
{
	GnomeVFSClient *client;
	GNOME_VFS_Daemon daemon;
        CORBA_Environment ev;
	GNOME_VFS_DriveList *list;
	GnomeVFSDrive *drive;
	GnomeVFSVolumeMonitor *volume_monitor;
	int i;

	if (volume_monitor_client->is_shutdown)
		return;
	
	volume_monitor = GNOME_VFS_VOLUME_MONITOR (volume_monitor_client);

	client = _gnome_vfs_get_client ();
	daemon = _gnome_vfs_client_get_daemon (client);

	if (daemon != CORBA_OBJECT_NIL) {
		CORBA_exception_init (&ev);

		list = GNOME_VFS_Daemon_getDrives (daemon,
						   BONOBO_OBJREF (client),
						   &ev);
		if (BONOBO_EX (&ev)) {
			CORBA_Object_release (daemon, NULL);
			CORBA_exception_free (&ev);
			return;
		}
		
		for (i = 0; i < list->_length; i++) {
			drive = _gnome_vfs_drive_from_corba (&list->_buffer[i],
							     volume_monitor);
			_gnome_vfs_volume_monitor_connected (volume_monitor, drive);
			gnome_vfs_drive_unref (drive);
		}

		CORBA_free (list);
		CORBA_Object_release (daemon, NULL);
	}
}

static void
read_volumes_from_daemon (GnomeVFSVolumeMonitorClient *volume_monitor_client)
{
	GnomeVFSClient *client;
	GNOME_VFS_Daemon daemon;
        CORBA_Environment ev;
	GNOME_VFS_VolumeList *list;
	GnomeVFSVolume *volume;
	GnomeVFSVolumeMonitor *volume_monitor;
	int i;

	if (volume_monitor_client->is_shutdown)
		return;
	
	volume_monitor = GNOME_VFS_VOLUME_MONITOR (volume_monitor_client);
	
	client = _gnome_vfs_get_client ();
	daemon = _gnome_vfs_client_get_daemon (client);

	if (daemon != CORBA_OBJECT_NIL) {
		CORBA_exception_init (&ev);

		list = GNOME_VFS_Daemon_getVolumes (daemon,
						    BONOBO_OBJREF (client),
						    &ev);
		if (BONOBO_EX (&ev)) {
			CORBA_Object_release (daemon, NULL);
			CORBA_exception_free (&ev);
			return;
		}
		
		for (i = 0; i < list->_length; i++) {
			volume = _gnome_vfs_volume_from_corba (&list->_buffer[i],
							       volume_monitor);
			_gnome_vfs_volume_monitor_mounted (volume_monitor, volume);
			gnome_vfs_volume_unref (volume);
		}
		
		CORBA_free (list);
		CORBA_Object_release (daemon, NULL);
	}
}

static void
gnome_vfs_volume_monitor_client_init (GnomeVFSVolumeMonitorClient *volume_monitor_client)
{
	read_drives_from_daemon (volume_monitor_client);
	read_volumes_from_daemon (volume_monitor_client);
}

/* Remeber that this could be running on a thread other
 * than the main thread */
static void
gnome_vfs_volume_monitor_client_finalize (GObject *object)
{
	GnomeVFSVolumeMonitorClient *volume_monitor_client;

	volume_monitor_client = GNOME_VFS_VOLUME_MONITOR_CLIENT (object);

	g_assert (volume_monitor_client->is_shutdown);
	
	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

void
_gnome_vfs_volume_monitor_client_daemon_died (GnomeVFSVolumeMonitorClient *volume_monitor_client)
{
	GnomeVFSVolumeMonitor *volume_monitor;

	volume_monitor = GNOME_VFS_VOLUME_MONITOR (volume_monitor_client);
	
	_gnome_vfs_volume_monitor_unmount_all (volume_monitor);
	_gnome_vfs_volume_monitor_disconnect_all (volume_monitor);

	read_drives_from_daemon (volume_monitor_client);
	read_volumes_from_daemon (volume_monitor_client);
}

void
_gnome_vfs_volume_monitor_client_shutdown (GnomeVFSVolumeMonitorClient *volume_monitor_client)
{
        CORBA_Environment ev;
	GNOME_VFS_Daemon daemon;
	GnomeVFSClient *client;

	if (volume_monitor_client->is_shutdown)
		return;
	
	volume_monitor_client->is_shutdown = TRUE;
	
	client = _gnome_vfs_get_client ();
	daemon = _gnome_vfs_client_get_daemon (client);
	
	if (daemon != CORBA_OBJECT_NIL) {
		GNOME_VFS_Daemon_deRegisterVolumeMonitor (daemon, BONOBO_OBJREF (client), &ev);
		CORBA_exception_init (&ev);
		
		if (BONOBO_EX (&ev)) {
			CORBA_exception_free (&ev);
		}
		
		CORBA_Object_release (daemon, NULL);
	}
}
