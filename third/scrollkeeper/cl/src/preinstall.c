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
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libintl.h>
#include <locale.h>
#include <scrollkeeper.h>

static int validate_args(int argc)
{
    if (argc == 4)
	return 1;
	    
    printf(_("Usage: scrollkeeper-preinstall <DOC FILE> <OMF FILE> <NEW OMF FILE>\n"));
    return 0;
}

int
main (int argc, char *argv[])
{
    char *omf_name, *url, *omf_new_name;
    
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    textdomain (PACKAGE);
    
    if (!validate_args(argc))
        return 1;
        
    omf_name = argv[2];
	
    if (!strncmp("file:", argv[1], 5))
    {
	url = argv[1];
    }
    else
    {
	url = calloc(strlen(argv[1]) + 7, sizeof(char));
	check_ptr(url, argv[0]);
	strcpy(url, "file:");
	strcat(url, argv[1]); 
    }
    
    omf_new_name = argv[3];

    if (!update_doc_url_in_omf_file(omf_name, url, omf_new_name)) {
    	fprintf(stderr, _("Unable to update URL in OMF file %s.  Copying OMF file unchanged.\n"), omf_name);

        /*
         *  Copy the old file to the new name, since the missing file will
	 *  break some packaging systems.  In these cases, it is better to
	 *  at least get a package, even if a doc or two isn't registered
	 *  properly than to not get a package built at all.
         */
	copy_file(omf_name, omf_new_name);

	return 1;
    }
            
    return 0;
}

