/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * Copyright (C) 2000 Eazel, Inc.
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GNOME_VFS_MIME_H__
#define __GNOME_VFS_MIME_H__

#include <glib.h>

#include "gnome-vfs-types.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

void 	     gnome_vfs_mime_shutdown 				(void);

/* do not free any of the returned gchar* values */
const char  *gnome_vfs_mime_type_from_name			(const gchar* filename);
const char  *gnome_vfs_mime_type_from_name_or_default	(const gchar *filename,
								 const gchar *defaultv);

const char  *gnome_vfs_get_mime_type				(GnomeVFSURI *uri);
const char  *gnome_vfs_get_mime_type_from_uri			(GnomeVFSURI *uri);
const char  *gnome_vfs_get_mime_type_from_file_data		(GnomeVFSURI *uri);
const char  *gnome_vfs_get_mime_type_for_data			(gconstpointer data,
                                                                 int data_size);
const char  *gnome_vfs_get_file_mime_type 			(const char *path, 
								 const struct stat *optional_stat_info,
								 gboolean suffix_only);
/* functions for working with uri lists */
GList       *gnome_uri_list_extract_filenames			(const gchar* uri_list);
GList       *gnome_uri_list_extract_uris			(const gchar* uri_list);
void         gnome_uri_list_free_strings			(GList *list);

/* utility function for getting a filename only from a single uri */
gchar       *gnome_uri_extract_filename                         (const gchar* uri);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif

