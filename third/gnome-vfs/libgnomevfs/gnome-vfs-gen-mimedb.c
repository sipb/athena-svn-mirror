/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* The Gnome Library is free software; you can redistribute it and/or
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
*/

#ifndef _BSD_SOURCE
#  define _BSD_SOURCE 1
#endif
#include <sys/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include "gnome-vfs-mime-magic.h"
#include "gnome-vfs.h"

extern GnomeMagicEntry *gnome_vfs_mime_magic_parse (const char *filename, int *nents);

int
main (int argc, char *argv[])
{
	GnomeMagicEntry *ents;
	char *filename, *out_filename;
	int nents;
	FILE *f;

	gnome_vfs_init();

	filename = NULL;
	if (argc > 1) {
		if (argv[1][0] == '-') {
			fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
			return 1;
		} else if (access (argv[1], F_OK) == 0)
			filename = argv[1];
	} else {
	        filename = g_strconcat (GNOME_VFS_CONFDIR, "/gnome-vfs-mime-magic", NULL);
	}

	if (filename == NULL) {
		fprintf (stderr, "Input file does not exist (or unspecified)...\n");
		fprintf (stderr, "Usage: %s [filename]\n", argv [0]);
		return 1;
	}

	ents = gnome_vfs_mime_magic_parse (filename, &nents);
	if (nents == 0) {
		fprintf (stderr, "%s: Error parsing the %s file\n", argv [0], filename);
		return 1;
	}

	out_filename = g_strconcat (filename, ".dat", NULL);

	f = fopen (out_filename, "w");
	if (f == NULL){
		fprintf (stderr, "%s: Can not create the output file %s\n", argv [0], out_filename);
		return 1;
	}

	if (fwrite (ents, sizeof(GnomeMagicEntry), nents, f) != nents){
		fprintf (stderr, "%s: Error while writing the contents of %s\n", argv [0], out_filename);
		fclose (f);
		return 1;
	}

	fclose (f);

	return 0;
}
