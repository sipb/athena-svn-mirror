/* gnomevfs-copy.c - Test for open(), read() and write() for gnome-vfs

   Copyright (C) 2003, Red Hat

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

   Author: Bastien Nocera <hadess@hadess.net>
*/

#include <libgnomevfs/gnome-vfs.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Copied directly from eel */
static GnomeVFSResult
copy_uri_simple ( const char *source_uri, const char *dest_uri)
{
	GnomeVFSResult result;
	GnomeVFSURI *real_source_uri, *real_dest_uri;
	real_source_uri = gnome_vfs_uri_new (source_uri);
	real_dest_uri = gnome_vfs_uri_new (dest_uri);

	result = gnome_vfs_xfer_uri (real_source_uri, real_dest_uri,
			GNOME_VFS_XFER_RECURSIVE,
			GNOME_VFS_XFER_ERROR_MODE_ABORT,
			GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
			NULL, NULL);

	gnome_vfs_uri_unref (real_source_uri);
	gnome_vfs_uri_unref (real_dest_uri);

	return  result;
}

int
main (int argc, char **argv)
{
	GnomeVFSResult res;

	if (argc != 3) {
		printf ("Usage: %s <src> <dest>\n", argv[0]);
		return 1;
	}

	if (!gnome_vfs_init ()) {
		fprintf (stderr, "Cannot initialize gnome-vfs.\n");
		return 1;
	}

	res = copy_uri_simple (argv[1], argv[2]);

	if (res != GNOME_VFS_OK) {
		fprintf (stderr, "Failed to copy %s to %s\nReason: %s\n",
				argv[1], argv[2], gnome_vfs_result_to_string (res));
		return 1;
	}

	return 0;
}
