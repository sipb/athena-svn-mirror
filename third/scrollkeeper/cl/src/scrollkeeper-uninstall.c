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

static void usage()
{
    printf(_("Usage: scrollkeeper-uninstall [-v] [-q] [-p <SCROLLKEEPER_DB_DIR>] <OMF FILE>\n"));
    exit(EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
    char *omf_name, scrollkeeper_dir[PATHLEN];
    FILE *fid;
    char outputprefs=0;
    int i;
 
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    textdomain (PACKAGE);
    
    if (argc == 1)
	usage();

    scrollkeeper_dir[0] = '\0';

    while ((i = getopt (argc, argv, "p:vq")) != -1)
    {
        switch (i)
        {
        case 'p':
            strncpy (scrollkeeper_dir, optarg, PATHLEN);
	    break;

        case 'v':
            outputprefs = outputprefs | SKOUT_STD_VERBOSE | SKOUT_LOG_VERBOSE;
            break;

        case 'q':
            outputprefs = outputprefs | SKOUT_STD_QUIET;
            break;

        default:
            usage (argv);
            exit (EXIT_FAILURE);
        }
    }
    
    umask(0022);

    omf_name = argv[argc - 1];

    if (scrollkeeper_dir[0] == '\0')
    {
	fid = popen("scrollkeeper-config --pkglocalstatedir", "r");
    	fscanf(fid, "%s", scrollkeeper_dir);
    	pclose(fid);
    }
	        
    sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "scrollkeeper-uninstall", _("Unregistering %s\n"), omf_name);
    uninstall(omf_name, scrollkeeper_dir, outputprefs);
       
    return 0;
}
