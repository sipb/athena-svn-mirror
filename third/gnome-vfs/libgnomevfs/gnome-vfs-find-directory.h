/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-find-directory.h - Public utility functions for the GNOME Virtual
   File System.

   Copyright (C) 2000 Eazel, Inc.

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

   Authors: Pavel Cisler <pavel@eazel.com>
*/

#ifndef _GNOME_VFS_FIND_DIRECTORY_H
#define _GNOME_VFS_FIND_DIRECTORY_H

#include <glib.h>
#include "gnome-vfs-types.h"

GnomeVFSResult	gnome_vfs_find_directory (GnomeVFSURI 			*near_uri,
					  GnomeVFSFindDirectoryKind 	kind,
					  GnomeVFSURI 			**result,
					  gboolean 			create_if_needed,
		   			  gboolean			find_if_needed,
					  guint 			permissions);
#endif
