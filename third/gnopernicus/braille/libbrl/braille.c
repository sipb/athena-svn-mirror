/* braille.c
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
#include <string.h>

#include "libsrconf.h"

#include "braille.h"
#include "baumbrl.h"
#include "alvabrl.h"
#include "handybrl.h"
#ifdef __BRLTTY_SUPPORT__
#include "ttybrl.h"
#endif

typedef struct
{
  gchar *device_id;
  gchar *device_description;
} BRLDevInfo;

/* Globals */
BRLDevInfo   supported_devices[] = {

  {"VARIO20", "BAUM VARIO - 20 cells"},
  {"VARIO40", "BAUM VARIO - 40 cells"},
  {"VARIO80", "BAUM VARIO - 80 cells"},
  {"DM80P",   "BAUM DM80 - 80 cells"},
  {"INKA",    "BAUM INKA"},
  {"ALVA320", "ALVABRAILLE 320"},
  {"ALVA340", "ALVABRAILLE 340"},
  {"ALVA34d", "ALVABRAILLE 34d"},
  {"ALVA380", "ALVABRAILLE 380"},
  {"ALVA544", "ALVABRAILLE 544"},
  {"ALVA570", "ALVABRAILLE 570"},
  {"BRLTTY",  "BRLTTY's BrlAPI"},
  {"HTBRW",   "HandyTech Braille Wave"},
  {"HTBL2",   "HandyTech Braillino"},
  {"HTBS4",   "HandyTech Braille Star 40"},
  {"HTBS8",   "HandyTech Braille Star 80"},
  {"HTMB2",   "HandyTech Modular 24"},
  {"HTMB4",   "HandyTech Modular 44"},
  {"HTMB8",   "HandyTech Modular 84"}

 };

static BRLDevice    *current_device = NULL;
static guchar       *dots = NULL;
static BRLEventProc client_event_proc = NULL;

/* Deviceless Functions */
void
brl_init()
{
    gint i;
    gchar *brldev_key;

    /* publish into GCONF the list of supported devices */
    i = sizeof (supported_devices) / sizeof (BRLDevInfo);
    SET_BRAILLE_CONFIG_DATA (BRAILLE_DEVICE_COUNT, CFGT_INT, &i);

    for (i = 0; i < sizeof (supported_devices) / sizeof(BRLDevInfo); ++i)
    {
	brldev_key = g_strdup_printf ("brldev_%d_ID", i);
	SET_BRAILLE_CONFIG_DATA (brldev_key, CFGT_STRING, supported_devices[i].device_id);
	g_free (brldev_key);

	brldev_key = g_strdup_printf ("brldev_%d_description", i);
	SET_BRAILLE_CONFIG_DATA (brldev_key, CFGT_STRING, supported_devices[i].device_description);
	g_free (brldev_key);
    }
    
    i = 0;
    SET_BRAILLE_CONFIG_DATA (BRAILLE_DEFAULT_DEVICE, CFGT_INT, &i);
}

void
brl_terminate()
{
    brl_close_device();
}

/* Device Related Functions */

/* Device Callback */
void
device_callback (BRLEventCode code,
                 BRLEventData *data)
{
    /* call the client callback */
    /* NOTE: could add some preprocessing here */
    if (client_event_proc)
	client_event_proc (code, data);
}

/* API Functions */
gint
brl_open_device (gchar        *device_name,
	         gint         port,
		 BRLEventProc event_proc)
{
    gint rv = 1;

    /* store the client callback */
    client_event_proc = event_proc;

    /* create an empty BRLDevice structure */
    current_device = calloc (sizeof (BRLDevice), sizeof (guchar));
	
    if (current_device)
    {
	if (strcmp("VARIO", device_name) == 0 ||
	    strcmp("VARIO40", device_name) == 0 ||
	    strcmp("VARIO20", device_name) == 0 ||
	    strcmp("VARIO80", device_name) == 0 ||
	    strcmp("DM80P", device_name) == 0 ||
	    strcmp("INKA", device_name) == 0)
	{
	    rv = baum_brl_open_device (device_name, port, device_callback, current_device);
	}	

	else if (strcmp("ALVA320", device_name) == 0 ||
		 strcmp("ALVA340", device_name) == 0 ||
		 strcmp("ALVA34d", device_name) == 0 ||
		 strcmp("ALVA380", device_name) == 0 ||
		 strcmp("ALVA544", device_name) == 0 ||
		 strcmp("ALVA570", device_name) == 0 )
	{			
	    rv = alva_brl_open_device (device_name, port, device_callback, current_device);			
	}
		
	else if (strcmp("BRLTTY", device_name) == 0)
	{			
	    rv = 0;
#ifdef __BRLTTY_SUPPORT__		
	    rv = brltty_brl_open_device (device_name, port, device_callback, current_device);			
#endif
	}
		
	else if (strcmp("PB40", device_name) == 0)
	{
	    /* rv = tsc_open_device (device_name, port, current_device, device_callback); */
		}

	else if (strcmp("HTBRW", device_name) == 0 ||
		 strcmp("HTBL2", device_name) == 0 ||
		 strcmp("HTBS4", device_name) == 0 ||
		 strcmp("HTBS8", device_name) == 0 ||
		 strcmp("HTMB2", device_name) == 0 ||
		 strcmp("HTMB4", device_name) == 0 ||
		 strcmp("HTMB8", device_name) == 0)
	{
	    rv =  handy_brl_open_device(device_name, port, device_callback, current_device);
	}

	else
	{
	    /* unknown device */
	    fprintf (stderr, "\nbrl_open_device: unknown device");
	    rv = 0;
	}		

	if (rv)
	{
	    dots = calloc (current_device->cell_count, sizeof (guchar));
    	    current_device->send_dots (dots, current_device->cell_count, 1);
	}
	else
	{
	    fprintf (stderr, "\nbrl_open_device: open device failed");	
	    brl_close_device();
	    return 0;
	}
    }

    {
        gint i, cnt;
        cnt = 0;
	for (i = 0; i < current_device->display_count; i++)
	{
	    /*
	    fprintf (stderr, "\nDISPLAY %d from %d to %d", i, 
		    (current_device->displays[i]).start_cell,
		    (current_device->displays[i]).width);
	    */
	    cnt += (current_device->displays[i]).width;
	}
	if (cnt != current_device->cell_count)
	    fprintf (stderr, "\nIncorrect technical data for device %s", device_name);
	    g_assert (cnt == current_device->cell_count);
    }
    return rv;
}

int brl_get_device (BRLDevice *device)
{
	if (current_device)
	{
		memcpy (device, current_device, sizeof (BRLDevice));
		return 1;
	}
	else
	{
		fprintf (stderr, "brl_get_device: no device opened");	/* !!! TBR !!! be more explicit here */
		return 0;
	}

}

void 
brl_close_device ()
{
    if (current_device)
    {		
	if(current_device->close_device)
	    current_device->close_device ();
	free (current_device);
	current_device = NULL;
    }
    client_event_proc = NULL;
}

gshort
brl_get_disp_id (gchar *role,
		 gshort no)
{
    gint i;
    gshort rv = -1;
    gshort type_no = -1;
	
    if (current_device)
    {		
	if (role)
	{
	    /* we have a role, search for that role + displayNo */
	    for (i = 0; i < current_device->display_count ; ++i)
	    {
 		if ((strcasecmp (role, "main") == 0 ) && (current_device->displays[i].type == BRL_DISP_MAIN))
        	{        		
        	    ++type_no;
        	    if (type_no == no)
        	    {
       		    	rv = i;
       			break;
       		    }
       		}
      		else if ((strcasecmp (role, "status") == 0 ) && (current_device->displays[i].type == BRL_DISP_STATUS))
      		{
	      	    ++type_no;
	      	    if (type_no == no)
        	    {
       			rv = i;
       			break;
       		    }
      		}
     		else if ((strcasecmp (role, "auxh") == 0 ) && (current_device->displays[i].type == BRL_DISP_HORIZONTAL))
     		{
     		    ++type_no;
     		    if (type_no == no)
        	    {
       			rv = i;
       			break;
       		    }
     		}
      		else if ((strcasecmp (role, "auxv") == 0 ) && (current_device->displays[i].type == BRL_DISP_VERTICAL))
      		{
      		    ++type_no;
      		    if (type_no == no)
        	    {
       			rv = i;
       			break;
       		    }
      		}
 	    }	
 	}	
 	else
 	{
 	    /* no role, the no is the ID if validated */
 	    if (no < current_device->display_count)
 	    {
 		rv = no;
 	    }
 	}
    }
	
    return rv;	
}

void 
brl_clear_all()
{	
    if (current_device && dots)
    {				
	memset (&dots[0], 0, current_device->cell_count);
    }	
}

void 
brl_clear_display (gshort display)
{
    BRLDisplay *brd;
	
    /* set to 0 all the cells coresponding to the display */
    if (display < (current_device->display_count) &&
		   display >= 0  && current_device && dots)
    {		
	brd = &(current_device->displays[display]);
	memset (&dots[brd->start_cell], 0, brd->width);
    }	
}

gshort
brl_get_display_width (gshort display)
{
    BRLDisplay *brd;
    gshort width = -1;
    
    if (display >=0 && display < current_device->display_count)
    {
	brd = &(current_device->displays[display]);
	width = brd->width;
    }
    return width;
}

void 
brl_set_dots (gshort display,
	      gshort start_cell,
	      guchar *new_dots,
	      gshort cell_count,
	      gshort offset, 
	      gshort cursor_position)
{
    BRLDisplay *brd;

    /* !!! TBI !!! consider offset (i.e. for panning support) */
	
    if (display >= 0 && display < (current_device->display_count) &&			
        current_device && current_device->send_dots && dots && new_dots)
    {
	brd = &(current_device->displays[display]);
	if ((start_cell >= 0) && (start_cell < brd->width))
	{
    	    if ((cell_count - offset) > 0 )  /* accept only offsets smaller then cell count */
    	    {
    		/* automatically adjust the offset to keep cursor visible */
    		if (cursor_position >= 0 && offset == 0)  /* only if cursor AND no explicit offset...*/
    		{
        	    /* adjust the offset to keep cursor visible */
        	    offset = cursor_position - brd->width + 1; /* !!! TBR !!! need something smarter here !!! (i.e consider previous pos) */
        	    if (offset < 0)
			offset = 0;
        	    /* fprintf (stderr, "Auto offset %d\n", offset); */
    		}

		if (cell_count > (brd->width - start_cell + offset))
		    cell_count = brd->width - start_cell + offset;
		if (cell_count < 0)
		    cell_count = 0;
    		/* clamp cell count to actual display width */			

		/* fprintf(stderr, "BRL: set dots: cell_cnt:%d, st:%d, off:%d\n", cell_count, start_cell, offset); */
    		memcpy (&dots[brd->start_cell + start_cell], &new_dots[offset], cell_count - offset);

                /* !!! TBI !!! fire a callback here */
	    }
	}		
    }
}

void 
brl_update_dots (gshort blocking)
{
    if (current_device && dots)
    {
	/* send all dots at once */
	current_device->send_dots(dots, current_device->cell_count, blocking);
    }
}

