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
#include <string.h>
#include <glib.h>
#include <libxml/parser.h>
#include "brlxmlapi.h"
#include "brlinp.h"


/* GLOBALS */
GMainLoop* g_loop  = NULL;

/* CODE */
void 
brl_input_event (BRLInEvent* brl_in_event)
{
    /* Braille event comes here */
	
    switch (brl_in_event->event_type)
    {
	/* NOTE: brl_in_event.event_data.XXX_codes can be NULL ! (check it!) */
	case BIET_KEY:
	    if (brl_in_event->event_data.key_codes)
	    {	
		fprintf (stderr, "KEY: %s\n", brl_in_event->event_data.key_codes);
		/* TEST - 6 keys pressed -> exit loop -> terminate app */
		if ((g_strcasecmp(brl_in_event->event_data.key_codes, "DK00DK01DK02DK03DK04DK05") == 0) ||
		    (g_strcasecmp(brl_in_event->event_data.key_codes, "FK00FK01FK02FK03FK04FK05") == 0))
		{
		    g_main_quit	(g_loop);
		}
	    }
	    else
	    {
		fprintf (stderr, "ALL KEYS RELEASED\n");
	    }
	break;
		
	case BIET_SENSOR:		
	    if (brl_in_event->event_data.sensor_codes)
	    {
		fprintf (stderr, "SENSOR: %s\n", brl_in_event->event_data.sensor_codes);
	    }
	    else
	    {
		fprintf (stderr, "ALL SENSORS RELEASED\n");
	    }
	break;
		
	case BIET_SWITCH:		
	    if (brl_in_event->event_data.switch_codes)
	    {
		fprintf (stderr, "SWITCH: %s\n", brl_in_event->event_data.switch_codes);
	    }
	    else
	    {
		fprintf (stderr, "ALL SWITCHED ARE OFF\n");
	    }		
	break;
		
	default:
	    fprintf (stderr, "UNKNOWN BRAILLE EVENT");
	break;

    }
}

/* Braille input callback */
void 
brl_xml_input_proc (gchar *buffer, 
		    gint  len)
{	
    /* fprintf (stderr, "XML:%s\n", buffer);	 */
	
    /* Braille input received as XML, give it to the parser */
    brl_in_xml_parse (buffer, len);			
}

/* Main */
int main (int argc, char** argv)
{
    gint i, len, port = 1;      /* 1 - default port number*/
    gchar *buff, *device = "VARIO";	/* VARIO - default device name*/
	
    FILE* fp;	
	
    printf ("Braille tester begin.\n");			
    if (argc > 1)
    {
	/* Get the device name and the port number form the command line parameters */
	for (i = 1; i < argc; ++i)
	{
	    /* fprintf (stderr, "%s\n", argv[i]);			 */
	    if (strstr (argv[i], "-d"))
	    {				
		/* fprintf (stderr, "DEVICE:%s\n", &argv[i][2]); */
		device = &argv[i][2];
		continue;
	    }
						
	    if (strstr (argv[i], "-p"))
	    {			
		/* fprintf (stderr, "PORT:%s\n", &argv[i][2] );				 */
		port = atoi(&argv[i][2]);
		continue;
	    }	
	}
		 		
 	/* Initialize the XML input parser */
 	brl_in_xml_init (brl_input_event);
 		
 	/* Initialize the Braille component */
 	if (brl_xml_init (device, port, brl_xml_input_proc))
 	{
 	    /* Device initializaton succeeded */
 		 		 		
 	    /* Load the test XML file and send it to the Braille component. */
 	    fp = fopen ("brl_out_test.xml", "rt"); 	
 	    if (fp)
 	    {
 		/* get file len */
 		fseek (fp, 0, SEEK_END);
 		len = ftell (fp);
 		rewind (fp);
 		
 		/* allocate buff and read file in memory */
 		buff = (char* )malloc (len + 1);
 		fread (buff, 1, len, fp);
 		
 		/* fprintf (stderr, "len:%d, %s\n", len, buff); */
 		
 		/* OUTPUT TO BRAILLE */
 		brl_xml_output(buff, len);
 				
 		/* clean up */
 		free (buff);
 		fclose (fp);
 	    }
 	    else
 	    {
 		fprintf (stderr, "Could not open test file\n");
 	    }
 			
 	    /* Create a GNOME loop */
 	    g_loop = g_main_new (TRUE);
 	    if (g_loop)
 	    {
		/* Enter the GNOME loop (to poll for Braille input) and stay there until */
		/* g_main_quit is called from the Braille input callback. */
 				
 		printf ("Entering the GNOME loop...\n");
 		g_main_run (g_loop);		
 		printf ("GNOME loop exited.\n"); 	    	
 		g_main_destroy (g_loop);
 		g_loop = NULL;
 	    }
 	    else
 	    {
 		fprintf (stderr, "Error on creating GNOME loop\n");
	    }
 	
 	    /* terminate the output module */
 	    brl_xml_terminate (); 		 		 		
 	    brl_xml_terminate ();	/* NOTE: 2'nd call just to test robustness */
	}
 	/* terminate the input module */
 	brl_in_xml_terminate ();
 	brl_in_xml_terminate (); /* NOTE: 2'nd call just to test robustness */	
    }	
    else
    {
	/* no command line parameter, show param */
	printf ("\nUSAGE: ./tester -d<device name> [-p<port number>]\n\n");	
    }
	
    printf ("Braille tester end.\n");	
	
    return 0;
}
