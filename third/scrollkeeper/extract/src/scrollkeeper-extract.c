/* copyright (C) 2001 Sun Microsystems */

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
#include <locale.h>
#include <libintl.h>
#include <scrollkeeper.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

static void usage()
{
	printf(_("Usage: scrollkeeper-extract <xml file> <stylesheet 1> <output file 1> <stylesheet 2> <output file 2> ...\n"));
}

int
main (int argc, char *argv[])
{
	char **stylesheets, **outputs;
	int i, j, num;
	char *extension;
        char outputprefs=0;

	setlocale (LC_ALL, "");
  	bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
  	textdomain (PACKAGE);

	num = (argc-2)/2;

	if (num <= 0) {
		usage();
		exit(EXIT_FAILURE);
	}
	
	umask(0022);

	extension = strrchr(argv[1], '.');

        outputprefs = SKOUT_STD_QUIET;	/* we should add command-line switches */

	if (extension == NULL ||
	    (strcmp(extension, ".sgml") &&
	      strcmp(extension, ".xml"))) {
		sk_message(outputprefs, SKOUT_DEFAULT, SKOUT_QUIET, "scrollkeeper-extract", _("Unrecognized file type: %s\n"), extension);
		exit(EXIT_FAILURE);
	}
	extension++;

	stylesheets = (char **)malloc(num*(sizeof(char *)));
	check_ptr(stylesheets, "scrollkeeper-extract");
	outputs = (char **)malloc(num*(sizeof(char *)));
	check_ptr(outputs, "scrollkeeper-extract");

	for(i = 2, j = 0; i < argc; i+= 2, j++) {
		stylesheets[j] = argv[i];
		outputs[j] = argv[i+1];
	}

	if (!apply_stylesheets(argv[1], extension, num, stylesheets, outputs, outputprefs)) {
		free(stylesheets);
		free(outputs);
		exit(EXIT_FAILURE);
	}
	
	free(stylesheets);
	free(outputs);
	exit(EXIT_SUCCESS);
}
