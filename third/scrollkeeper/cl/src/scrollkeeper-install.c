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
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <libintl.h>
#include <dirent.h>
#include <scrollkeeper.h>

static xmlExternalEntityLoader defaultEntityLoader = NULL;

static void usage()
{
    printf(_("Usage: scrollkeeper-install [-n] [-v] [-q] [-p <SCROLLKEEPER_DB_DIR>] <OMF FILE>\n"));
    exit(EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{
    char *omf_name;
    char scrollkeeper_dir[PATHLEN];
    char scrollkeeper_data_dir[PATHLEN];
    FILE *fid;
    char outputprefs=0;
    int i;
    
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
    textdomain (PACKAGE);

    if (argc == 1) {
	usage();
    }

    defaultEntityLoader = xmlGetExternalEntityLoader();
    xmlSetExternalEntityLoader(xmlNoNetExternalEntityLoader);

    scrollkeeper_dir[0] = '\0';
    while ((i = getopt (argc, argv, "p:vqn")) != -1)
    {
        switch (i)
        {
        case 'p':
            strncpy (scrollkeeper_dir, optarg, PATHLEN);
            break;

        case 'v':
            outputprefs = outputprefs | SKOUT_STD_VERBOSE | SKOUT_LOG_VERBOSE;
            break;

        case 'n':
	    xmlSetExternalEntityLoader(defaultEntityLoader);
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
    
    fid = popen("scrollkeeper-config --pkgdatadir", "r");
    fscanf(fid, "%s", scrollkeeper_data_dir);
    pclose(fid);
    
    if (create_database_directory(scrollkeeper_dir, scrollkeeper_data_dir, outputprefs) != 0) {
        sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "scrollkeeper-install", _("Could not create database.  Aborting install.\n"));
        return 1;
    }
    
    sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "scrollkeeper-install", _("Registering %s\n"), omf_name);
    if (! install(omf_name, scrollkeeper_dir, scrollkeeper_data_dir, outputprefs)) {
    	sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "scrollkeeper-install", _("Unable to register %s\n"), omf_name);
    	exit(EXIT_FAILURE);
    }
     
    return 0;
}
