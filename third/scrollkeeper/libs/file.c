/* copyright (C) 2001 Sun Microsystems, Inc.*/

/*    
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <sys/stat.h>
#include <scrollkeeper.h>
#include <stdio.h>

int is_file(char *filename)
{
    struct stat buf;

    if (!stat(filename, &buf) && S_ISREG(buf.st_mode))
        return 1;

    return 0;
}

int is_dir(char *path)
{
    struct stat buf;

    if (!stat(path, &buf) && S_ISDIR(buf.st_mode))
        return 1;

    return 0;
}

int copy_file(char *old, char *new)
{
	FILE *old_fid, *new_fid;
	unsigned char buf[1024];
	int nitems;
	
	old_fid = fopen(old, "r");
	if (old_fid == NULL) {
		return 0;
	}
	
	new_fid = fopen(new, "w");
	if (new_fid == NULL) {
		fclose (old_fid);
		return 0;
	}
	
	
	while (!feof(old_fid)) {
		nitems = fread((void *)buf, sizeof(unsigned char), 1024, old_fid);
		if (nitems == 0) {
			if (ferror(old_fid)) {
				fclose (old_fid);
				fclose (new_fid);
				return 1;
			}
		}
		
		if (fwrite((void *)buf, sizeof(unsigned char), nitems, new_fid) == 0) {
			fclose (old_fid);
			fclose (new_fid);
			return 1;
		}
	}
	
	fclose(old_fid);
	fclose(new_fid);
		
	return 1;
}

