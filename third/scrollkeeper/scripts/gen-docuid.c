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
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <uuid.h>
#include <scrollkeeper.h>

int
main (int argc, char *argv[])
{
	char   str[256];
	uuid_t uu;

	if (argc > 1)
	{
	     setlocale (LC_ALL, "");
	     bindtextdomain (PACKAGE, SCROLLKEEPERLOCALEDIR);
	     textdomain (PACKAGE);

	     if (strcmp (argv[1], "--help") == 0)
	     {
		  printf (_("Usage: %s\n"), *argv);
		  return 0;
	     }
	     fprintf (stderr, _("Usage: %s\n"), *argv);
	     return 1;
	}
	
	uuid_generate_time(uu);
	
	uuid_unparse(uu, str);

	printf("%s\n", str);

	return 0;
}
