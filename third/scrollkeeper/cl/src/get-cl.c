/* copyright (C) 2000 Sun Microsystems, Inc.*/

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
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>
#include <libxml/tree.h>
#include <scrollkeeper.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <locale.h>

/* cycles through five temporary filenames of the form /tmp/scrollkeeper-templfile.x,
   where x is number from 0 to 4 and returns the first one that does not exist or the
   oldest one
*/
static char *get_next_free_temp_file_path(char outputprefs)
{
	char path[PATHLEN], *filename;
	int i, num;
	struct stat buf;
	time_t prev;
	
	prev = 0;
	num = 0;
	
	for(i = 0; i < 5; i++) {
		snprintf(path, PATHLEN, "/tmp/scrollkeeper-tempfile.%d", i);
		if (stat(path, &buf) == -1) {
			if (errno == ENOENT) {
				/* this is an empty slot so use it */
				
				num = i;
				break;
			}
			else {
                                sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "scrollkeeper-get-cl", _("Cannot open temporary file.\n"));
				exit(EXIT_FAILURE);
			}
		} else {
			if (i == 0) {
				prev = buf.st_ctime;
			} else {
				if (prev > buf.st_ctime) {
					/* this is the oldest so use it */
				
					num = i;
					break;
				}
			}
		}
		
	}
	
	if (i == 5) {
		/* if we got here it means that all slots are taken
		   and the first is the oldest
		*/
		
		num = 0;
	}

	filename = malloc(sizeof(char)*PATHLEN);
	check_ptr(filename, "scrollkeeper-get-cl");
	snprintf(filename, PATHLEN, "/tmp/scrollkeeper-tempfile.%d", num);

	return filename;
}

static void
usage (int argc, char **argv) {
    
    	if (argc != 3 && argc != 4) {
    		printf(
	    	_("Usage: %s [-v] <LOCALE> <CATEGORY TREE FILE NAME>\n"), *argv);
		exit(EXIT_SUCCESS);
    	}
}

int main(int argc, char **argv)
{
    	FILE *config_fid;
    	char scrollkeeper_dir[PATHLEN], *locale;
    	char *base_name, *path;
	xmlDocPtr merged_tree;
        char outputprefs=0;

    	setlocale (LC_ALL, "");
    	bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    	textdomain (PACKAGE);

    	usage(argc, argv);

    	if (argc == 3)
    	{
    		locale = argv[1];
    		base_name = argv[2];
    	}
    	else /* argc == 4 */
    	{
		outputprefs = outputprefs | SKOUT_STD_VERBOSE | SKOUT_LOG_VERBOSE;
		locale = argv[2];
		base_name = argv[3];
    	}
	
	umask(0022);

    	config_fid = popen("scrollkeeper-config --pkglocalstatedir", "r");
    	fscanf(config_fid, "%s", scrollkeeper_dir);  /* XXX buffer overflow */
    	pclose(config_fid);

    	merged_tree = merge_locale_trees(scrollkeeper_dir, locale, base_name);
	
	if (merged_tree == NULL) {
		sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "get_next_free_temp_file_path", _("No Content List for this locale!!!\n"));
		return 1;
	}
	
	path = get_next_free_temp_file_path(outputprefs);
	check_ptr(path, "scrollkeeper-get-cl");
	
	xmlSaveFile(path, merged_tree);
	printf("%s\n", path);
	free(path);
	
	return 0;
}
