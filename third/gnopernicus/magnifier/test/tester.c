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
#include <stdlib.h>
#include "magxmlapi.h"
/*#include <gtk/gtk.h>*/
#include <unistd.h>

int
main (int argc, char **argv)
{
	FILE* fp;
	int len, i = 0;
	char* buff;
	
while (i < 5)
{
	i++;

	fprintf (stderr, "\n%dnth time\n",i);
	sru_init();
		
	mag_xml_init (NULL);
	sleep(1);	
	fp = fopen ("magml.xml", "rt");
	
	if (fp)
	{
		/* get file len */
		fseek (fp, 0, SEEK_END);
		len = ftell (fp);
		rewind (fp);
		
		/* read buffer im memory */
		buff = (char* )malloc (len + 1);
		fread (buff, 1, len, fp);

/*		fprintf (stderr, "\n %s",buff);*/
		fprintf (stderr, "\n Before mag_xml_output %d",len);
		/* call the magnifier XML parser */
		mag_xml_output(buff, len);
		fprintf (stderr, "\n After mag_xml_output");
		/* clean up */
		free (buff);
		buff = NULL;
		fclose (fp);
		free (fp);
		fp = NULL;
	}
	else
	{
		fprintf (stderr, "Could not open file\n");
	}

	mag_xml_terminate();
	
	fprintf (stderr,"\n\nThe magnifier is stopped...\n");
}

/*
	printf ("*** Press CTRL + C  to end ***\n");

	while (1) ;	
*/
	return 0;
}
