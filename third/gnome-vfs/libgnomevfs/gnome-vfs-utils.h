/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-utils.h - Public utility functions for the GNOME Virtual
   File System.

   Copyright (C) 1999 Free Software Foundation
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

   Authors: Ettore Perazzoli <ettore@comm2000.it>
   	    John Sullivan <sullivan@eazel.com> 
*/

#ifndef GNOME_VFS_UTILS_H
#define GNOME_VFS_UTILS_H

#include <glib.h>
#include "gnome-vfs-types.h"

/* Makes a human-readable string. */
char *gnome_vfs_format_file_size_for_display (GnomeVFSFileSize  size);

/* Converts unsafe characters to % sequences so the string can be
 * used as a piece of a URI. Escapes all reserved URI characters.
 */
char *gnome_vfs_escape_string                (const char      *string);

/* Converts unsafe characters to % sequences so the path can be
 * used as a piece of a URI. Escapes all reserved URI characters
 * except for "/".
 */
char *gnome_vfs_escape_path_string           (const char      *path);

/* Converts unsafe characters to % sequences so the host/path segment
 * can be used as a piece of a URI.  Allows ":" and "@" in the host
 * section (everything up to the first "/"), and after that, it behaves
 * like gnome_vfs_escape_path_string.
 */
char *gnome_vfs_escape_host_and_path_string  (const char      *path);

/* Converts only slashes and % characters to % sequences. This is useful
 * for code that wants to use an arbitrary string as a file name. To use
 * it in a URI, you have to escape again, of course.
 */
char *gnome_vfs_escape_slashes               (const char      *string);


/* Escapes all the characters that match any of the @match_set */	              			 
char *gnome_vfs_escape_set 		     (const char      *string,
	              			      const char      *match_set);

/* Returns NULL if any of the illegal character appear in escaped
 * form. If the illegal characters are in there unescaped, that's OK.
 * Typically you pass "/" for illegal characters when converting to a
 * Unix path, since pieces of Unix paths can't contain "/". ASCII 0
 * is always illegal due to the limitations of NULL-terminated strings.
 */
char *gnome_vfs_unescape_string              (const char      *string,
					      const char      *illegal_characters);

/* returns a copy of uri, converted to a canonical form */
char *gnome_vfs_make_uri_canonical	     (const char 	*uri);

/* returns a copy of path, converted to a canonical form */
char *gnome_vfs_make_path_name_canonical     (const char      *path);

/* returns a copy of path, with initial ~ expanded, or just copy of path
 * if there's no initial ~ 
 */
char *gnome_vfs_expand_initial_tilde	     (const char      *path);

/* Prepare an escaped string for display. Unlike gnome_vfs_unescape_string,
 * this doesn't return NULL if an illegal sequences appears in the string,
 * instead doing its best to provide a useful result.
 */
char *gnome_vfs_unescape_string_for_display  (const char      *escaped);

/* Turn a "file://" URI in string form into a local path. Returns NULL
 * if it's not a URI that can be converted.
 */
char *gnome_vfs_get_local_path_from_uri      (const char      *uri);

/* Turn a path into a "file://" URI. */
char *gnome_vfs_get_uri_from_local_path      (const char      *local_full_path);

/* Free the list, freeing each item data with a g_free */
void   gnome_vfs_list_deep_free               (GList            *list);


/* Return amount of free space on target */
GnomeVFSResult	gnome_vfs_get_volume_free_space	(const GnomeVFSURI 	*vfs_uri, 
						 GnomeVFSFileSize 	*size);

char *gnome_vfs_icon_path_from_filename       (const char *filename);

#endif /* GNOME_VFS_UTILS_H */
