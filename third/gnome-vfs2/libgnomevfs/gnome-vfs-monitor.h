/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-monitor.h - File Monitoring for the GNOME Virtual File System.

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

#ifndef GNOME_VFS_MONITOR_H
#define GNOME_VFS_MONITOR_H

#include <glib.h>

typedef enum {
  GNOME_VFS_MONITOR_FILE,
  GNOME_VFS_MONITOR_DIRECTORY
} GnomeVFSMonitorType;

typedef enum {
  GNOME_VFS_MONITOR_EVENT_CHANGED,
  GNOME_VFS_MONITOR_EVENT_DELETED,
  GNOME_VFS_MONITOR_EVENT_STARTEXECUTING,
  GNOME_VFS_MONITOR_EVENT_STOPEXECUTING,
  GNOME_VFS_MONITOR_EVENT_CREATED,
  GNOME_VFS_MONITOR_EVENT_METADATA_CHANGED
} GnomeVFSMonitorEventType;

typedef struct GnomeVFSMonitorHandle GnomeVFSMonitorHandle;

typedef void (* GnomeVFSMonitorCallback) (GnomeVFSMonitorHandle *handle,
                                          const gchar *monitor_uri,
                                          const gchar *info_uri,
                                          GnomeVFSMonitorEventType event_type,
                                          gpointer user_data);
#endif /* GNOME_VFS_MONITOR_H */
