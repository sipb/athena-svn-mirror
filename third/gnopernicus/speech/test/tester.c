/* tester.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>/*malloc and free*/
#include "srs-xml.h"
#include "libbonobo.h"

int
main ()
{
	FILE* fp;
	int len;
	char* buff;
	
	srs_init (NULL);
	
	fp = fopen ("srsml.xml", "rt");
	
	if (fp)
	{
		/* get file len */
		fseek (fp, 0, SEEK_END);
		len = ftell (fp);
		rewind (fp);
		
		/* read buffer im memory */
		buff = (char* )malloc (len + 1);
		fread (buff, 1, len, fp);
				
		/* call the speech XML parser */
		fprintf (stderr, "%s %d :\nBuffer:\n%s\nSize of buffer: %ld\n", __FILE__, __LINE__, buff, g_utf8_strlen (buff, -1));
		srs_output (buff, len );				
		/* clean up */
		free (buff);
		fclose (fp);
	}
	else
	{
		fprintf (stderr, "\nCould not open file.");
	}
	
	fprintf (stderr,"*** Press CTRL + C  to end ***\n");

	/* while (1) ;	*/
  bonobo_main(); /* need this to receive callbacks */

	srs_terminate();
    return 0;	
}
