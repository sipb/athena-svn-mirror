/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs.h - The GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

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

   Author: Ettore Perazzoli <ettore@comm2000.it>
*/

#ifndef _GNOME_VFS_H
#define _GNOME_VFS_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "gnome-vfs-constants.h"
#include "gnome-vfs-types.h"

#include "gnome-vfs-async-ops.h"
#include "gnome-vfs-directory.h"
#include "gnome-vfs-directory-filter.h"
#include "gnome-vfs-directory-list.h"
#include "gnome-vfs-file-info.h"
#include "gnome-vfs-find-directory.h"
#include "gnome-vfs-init.h"
#include "gnome-vfs-xfer.h"
#include "gnome-vfs-ops.h"
#include "gnome-vfs-process.h"
#include "gnome-vfs-result.h"
#include "gnome-vfs-uri.h"
#include "gnome-vfs-utils.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _GNOME_VFS_H */
