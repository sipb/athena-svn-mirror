/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-slave-launch.h - Launch the helper asynchronous
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

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#ifndef _GNOME_VFS_SERVER_LAUNCHER_H
#define _GNOME_VFS_SERVER_LAUNCHER_H

GnomeVFSProcess *gnome_vfs_slave_launch (GNOME_VFS_Slave_Notify notify,
					 GNOME_VFS_Slave_Request *request_objref_return);

#endif /* _GNOME_VFS_SERVER_LAUNCHER_H */
