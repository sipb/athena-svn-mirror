/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-result.h - Result handling for the GNOME Virtual File System.

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

   Author: Ettore Perazzoli <ettore@comm2000.it>
*/

#ifndef GNOME_VFS_RESULT_H
#define GNOME_VFS_RESULT_H

#include <libgnomevfs/gnome-vfs-types.h>

const gchar	*gnome_vfs_result_to_string	  (GnomeVFSResult result);
GnomeVFSResult   gnome_vfs_result_from_errno_code (int errno_code);
GnomeVFSResult	 gnome_vfs_result_from_errno	  (void);
GnomeVFSResult   gnome_vfs_result_from_h_errno    (void);

#endif /* GNOME_VFS_RESULT_H */
