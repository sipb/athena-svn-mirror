/* alvabrl.c
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

#include "alvabrl.h"
#include "sercomm.h"

/* Supported ALVA Devices */
typedef enum 
{
    ALVADT_GENERIC,
    ALVADT_320,
    ALVADT_340,
    ALVADT_34d,
    ALVADT_380,
    ALVADT_570, 
    ALVADT_544
} ALVADeviceType;

/* Device data type */
typedef struct
{
    guchar version;	
    gulong front_keys;
    gulong display_keys;
    gulong pressed_front_keys;
    gulong pressed_display_keys;
    guchar key_codes[512]; 
    guchar sensor_codes[32];
    guchar last_error;
} ALVADeviceData;

/* ALVA Input Parser States */
typedef enum
{	
    AIPS_IDLE = 0,
    AIPS_EXP_2ND_BYTE, 
    AIPS_EXP_CR_OR_ETX
} AIPStates;

/* Globals */				  									
static gulong mask32[] = {

	0x00000001,0x00000002,0x00000004,0x00000008,0x00000010,0x00000020,0x00000040,0x00000080,
	0x00000100,0x00000200,0x00000400,0x00000800,0x00001000,0x00002000,0x00004000,0x00008000,	
	0x00010000,0x00020000,0x00040000,0x00080000,0x00100000,0x00200000,0x00400000,0x00800000,
	0x01000000,0x02000000,0x04000000,0x08000000,0x10000000,0x20000000,0x40000000,0x80000000

};

static BRLDevCallback		client_callback = NULL;
static ALVADeviceType		alva_dev_type = ALVADT_GENERIC;
static AIPStates		aip_state = AIPS_IDLE;
static gshort			aip_index = 0;
static ALVADeviceData		alvadd;

/* Functions */
static void 
alva_clear_device_data (ALVADeviceData *add)
{
    memset (add, 0, sizeof(ALVADeviceData));
}

static void 
alva_reset_aip ()
{
    aip_index = 0;
    aip_state = AIPS_IDLE;
    alva_clear_device_data (&alvadd);
}

void 
alva_on_keys_changed (guchar key_code_1, 
		      guchar key_code_2)
{
    /* NOTE: called for any display, front, back, chord key changes */
    /* fires when all keys are released */
    gint i, k, kcix;
	
    /* 0  1  2  3  4  5  6  7   8   9   10  11  12   13  14  15 */
    static gint fk_5xx_remap[] = {-1, 0, 5, 2, 1, 4, 3, -1, -1, -1, 6,  7,  8,   9, -1, -1};
	
    BRLEventCode bec;
    BRLEventData bed;
	
    switch (alva_dev_type)
    {
	default:
	break;
	
    	case ALVADT_GENERIC:
	case ALVADT_570:			
	case ALVADT_544:	
	    switch (key_code_1)
	    {				
		case 0x71:	/* front keys and the sensors above the status field */
		    if (key_code_2 == 0x00 || key_code_2 == 0x80)
		    {
			if (key_code_2 & 0x80)
			{
			    /* key release, reset the bit */
			    alvadd.display_keys &= ~mask32[key_code_2 & 0x1F];
			}
			else
			{
			    /* key press, set the bit */
			    alvadd.display_keys |= mask32[key_code_2 & 0x1F];
			    alvadd.pressed_display_keys |= mask32[key_code_2 & 0x1F];
			}
		    }
		    else
		    {
			/* this is a real front key or the 3 left sensor strip pair */
			k = key_code_2 & 0x7F;
			if (k >= 0x20 && k < 0x30)
			{
			    /* sensor strip above status elements, lower row */
			    i = k - 0x20 + 10;
			}
			else if (k >= 0x30)
			{
			    /* sensor strip above status elements, upper row */
			    i = k - 0x30 + 20;
			}
			else
			{						
			    /* actual front keys */
			    i = fk_5xx_remap[key_code_2 & 0x0F];
			}
						
			/* set za bit */			
			if (i >= 0 && i < 32)
			{
			    if (key_code_2 & 0x80)
			    {
				/* key release, reset the bit */
				alvadd.front_keys &= ~mask32[i];
			    }
			    else
			    {
				/* key press, set the bit */
				alvadd.front_keys |= mask32[i];
				alvadd.pressed_front_keys |= mask32[i];
			    }						
			}
		    }								
		break;

		case 0x77:	/* display keys (the 2 x 6 rounded group for 544 satelite) */				
		    k = key_code_2 & 0x1F;										
		    if (key_code_2 & 0x80)
		    {
			/* key release, reset the bit */
			if ((key_code_2 & 0x7F) >= 0x20)
			{
			    /* right block */
			    alvadd.display_keys &= ~mask32[k + 6];
			}
			else
			{
			    /* left block */
			    alvadd.display_keys &= ~mask32[k];
			}
		    }
		    else
		    {
			/* key press, set the bit */
			if ((key_code_2 & 0x7F) >= 0x20)
			{
			    /* right block */
			    alvadd.display_keys |= mask32[k + 6];
			    alvadd.pressed_display_keys |= mask32[k + 6];
			}
			else
			{
			    /* left block */
			    alvadd.display_keys |= mask32[k];
			    alvadd.pressed_display_keys |= mask32[k];
			}						
		    }			
		break;
	    }			
	break;
	
	case ALVADT_320:					
	case ALVADT_340:						
	case ALVADT_34d:					
	case ALVADT_380:					
	    switch (key_code_1)
	    {
		case 0x71:	/* front keys and the sensors above the status field */	
		    if (key_code_2 & 0x80)
		    {
			/* key release, reset the bit */
			alvadd.front_keys &= ~mask32[key_code_2 & 0x1F];
		    }
		    else
		    {
			/* key press, set the bit */
			alvadd.front_keys |= mask32[key_code_2 & 0x1F];
			alvadd.pressed_front_keys |= mask32[key_code_2 & 0x1F];
		    }				
		break;

		case 0x77:	/* display keys (the 2 x 6 rounded groub for 570 satelite) */
		    if (key_code_2 & 0x80)
		    {
			/* key release, reset the bit */
			alvadd.display_keys &= ~mask32[key_code_2 & 0x1F];
		    }
		    else
		    {
			/* key press, set the bit */
			alvadd.display_keys |= mask32[key_code_2 & 0x1F];
			alvadd.pressed_display_keys |= mask32[key_code_2 & 0x1F];
		    }			
		break;
	    }	
 	break;	
    }
		
    /* PROCESS THE KEYMAPS */
    /* fprintf (stderr, "FK:%08lx DK:%08lx\n", alvadd.front_keys, alvadd.display_keys); */
	
    if (alvadd.front_keys == 0 && alvadd.display_keys == 0)
    {
	/* all (!) keys are depressed (both front and display) */			
	kcix = 0;
				
	for (i = 0; i < 32; ++i)
	{			
	    if (alvadd.pressed_display_keys & mask32[i])
	    {
		kcix += sprintf (&alvadd.key_codes[kcix], "DK%02d", i);
	    }
	}
				
	for (i = 0; i < 32; ++i)
	    {			
		if (alvadd.pressed_front_keys & mask32[i])
		{
		    kcix += sprintf (&alvadd.key_codes[kcix], "FK%02d", i);
		}
	    }
				
	/* fire the key code event		 */
	bec = BRL_EVCODE_KEY_CODES;
	bed.key_codes = alvadd.key_codes;						
	client_callback (bec, &bed);
				
	/* clear presed_keys */
	alvadd.pressed_front_keys = 0;											
	alvadd.pressed_display_keys = 0;
    }		
}

void 
alva_on_sensors_changed (guchar sensor_code_1, 
			 guchar sensor_code_2)
{
    BRLEventCode bec;
    BRLEventData bed;
		
    alvadd.sensor_codes[0] = 0;
	
    switch (sensor_code_1)
    {		
	case 0x72:	/* lower sensors row, map as mechanical sensors			 */		
	    if ((sensor_code_2 & 0x80) == 0)
	    {
		/* sensor pressed */
		sprintf (&alvadd.sensor_codes[0], "HMS%02d", sensor_code_2);
	    }
	break;	
		
	case 0x75:	/* upper sensor row, map as optical sensors		 */		
	    if ((sensor_code_2 & 0x80) == 0)
	    {
		/* sensor pressed */
		sprintf (&alvadd.sensor_codes[0], "HOS%02d", sensor_code_2);
	    }
	break;		
    }				
    /* fprintf (stderr, "SC: %s\n", alvadd.sensor_codes); */
	
    bec = BRL_EVCODE_SENSOR;	
    bed.sensor.sensor_codes = alvadd.sensor_codes;						
    client_callback (bec, &bed);
			
}

gshort 
alva_brl_input_parser (gint new_val)
{

    static guchar 	code_1;
    static guchar 	code_2;
	
    static BRLEventCode	bec;
    static BRLEventData	bed;
	
    if (!client_callback) 
	return 0;	  /* if we don't have a client callback doesn't make sense to go further */

    /* raw byte callback */
    bec = BRL_EVCODE_RAW_BYTE;
    bed.raw_byte = new_val;
    client_callback (bec, &bed);

    switch (aip_state)
    {
	/* NOTE: All input is sent as group of two bytes. */
	/* Bit 7 of the 2'nd byte is 0 for the make code and 1 for the break code. */
		
	case AIPS_IDLE:	
	    switch (new_val)
	    {
		case 0x1B:	/* start up package (?) */
		    aip_state = AIPS_EXP_CR_OR_ETX;
		break;
				
		case 0x71:	/* front keys and the sensos above the status field */
		case 0x72:	/* lower sennsor strip row */
		case 0x75:	/* upper sensor strip row */
		case 0x77:	/* display keys (the 2 x 6 rounded groub for 570 satelite) */				
		    code_1 = new_val;
		    aip_state = AIPS_EXP_2ND_BYTE; 						
		break;
	    }		
	break;
	
	case AIPS_EXP_2ND_BYTE:		
	    code_2 = new_val;
	    switch (code_1)
	    {
		case 0x71:	/* front keys ans sensors above the status field */
		case 0x77:	/* display keys (2x6 rounded group for ALVA570) */
		    alva_on_keys_changed (code_1, code_2);
		    aip_state = AIPS_IDLE;
		break;
				
		case 0x72:	/* lower sensors row */
		case 0x75:	/* upper sensosrs row */
		    alva_on_sensors_changed (code_1, code_2);
		    aip_state = AIPS_IDLE;
		break;
	    }				
	break;
	
	case AIPS_EXP_CR_OR_ETX:
	    /* for the moment just discard until end of packet... */
	    if (new_val == 0x0D || new_val == 0x03)
	    {
		aip_state = AIPS_IDLE;
	    }	
	break;
	
	default:
	    /* internal error, invalid parser state */
	    alva_reset_aip();
	break;
    }

    return 0; /* 0-give next byte, 1-repeat last byte */
}

gint 
alva_brl_send_dots (guchar *dots, gshort count, gshort blocking)
{

    gint rv = 0, realcnt;
    guchar sendbuff[256];
			
    realcnt = 0;
	
    sendbuff[realcnt++] = 0x1B;
    sendbuff[realcnt++] = 'B';
    sendbuff[realcnt++] = 0;
	
    switch (alva_dev_type)
    {
	case ALVADT_320:			
	    sendbuff[realcnt++] = 23;
	    rv = 1;
	break;
	
	case ALVADT_340:			
	    sendbuff[realcnt++] = 43;
	    rv = 1;
	break;
	
	case ALVADT_34d:			
	    sendbuff[realcnt++] = 45;
	    rv = 1;
	break;
	
	case ALVADT_380:			
	    sendbuff[realcnt++] = 85;	/* len */
	    rv = 1;
	break;
		
	case ALVADT_570:
	    sendbuff[realcnt++] = 70;		/* ??? */
	    rv = 1;
	break;
		
	case ALVADT_544:
	    sendbuff[realcnt++] = 44;		/* ??? */
	    rv = 1;
	break;
	
	case ALVADT_GENERIC:
	default:
	break;			
    }
	
    memcpy (&sendbuff[realcnt], dots, count);	

    realcnt += count;
		
    sendbuff[realcnt++] = 0x0D;
	
    rv = brl_ser_send_data (sendbuff, realcnt, blocking);
	
    return rv;	
}

void 
alva_brl_close_device ()
{
    /* close serial communication */
    brl_ser_set_callback (NULL);				
    brl_ser_stop_timer ();	
    brl_ser_close_port ();
}

gint 
alva_brl_open_device (gchar          *device_name, 
		      gshort         port, 
		      BRLDevCallback device_callback,
		      BRLDevice      *device)
{
    gint rv = 0;
	
    alva_reset_aip();
    
    if (strcmp ("ALVA320", device_name) == 0)	
    {
	device->cell_count = 23;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 7;
		
	device->display_count = 2;
		
	device->displays[0].start_cell = 0;
	device->displays[0].width = 20;
	device->displays[0].type = BRL_DISP_MAIN;
		
	device->displays[1].start_cell = 20;
	device->displays[1].width = 3;
	device->displays[1].type = BRL_DISP_STATUS;
		
	alva_dev_type = ALVADT_320;
				
	rv = 1;
    }

    else if (strcmp ("ALVA340", device_name) == 0)	
    {
	device->cell_count = 43;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 7;
		
	device->display_count = 2;
		
	device->displays[0].start_cell = 0;
	device->displays[0].width = 40;
	device->displays[0].type = BRL_DISP_MAIN;
		
	device->displays[1].start_cell = 40;
	device->displays[1].width = 3;
	device->displays[1].type = BRL_DISP_STATUS;
		
	alva_dev_type = ALVADT_340;
				
	rv = 1;
    }
    
    else if (strcmp ("ALVA34d", device_name) == 0)	
    {
	device->cell_count = 45;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 7;
		
	device->display_count = 2;
		
	device->displays[0].start_cell = 0;
	device->displays[0].width = 40;
	device->displays[0].type = BRL_DISP_MAIN;
		
	device->displays[1].start_cell = 40;
	device->displays[1].width = 5;
	device->displays[1].type = BRL_DISP_STATUS;
		
	alva_dev_type = ALVADT_34d;
				
	rv = 1;
    }

		
    else if (strcmp ("ALVA380", device_name) == 0)	
    {
	device->cell_count = 85;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 10;
		
	device->display_count = 2;
		
	device->displays[0].start_cell = 5;
	device->displays[0].width = 80;
	device->displays[0].type = BRL_DISP_MAIN;
		
	device->displays[1].start_cell = 0;
	device->displays[1].width = 5;
	device->displays[1].type = BRL_DISP_STATUS;
		
	alva_dev_type = ALVADT_380;
				
	rv = 1;
    }

    else if (strcmp ("ALVA570", device_name) == 0) 	
    {			
	device->cell_count = 70;
    	device->input_type = BRL_INP_MAKE_BREAK_CODES;
    	device->key_count = 22;
		
	device->display_count = 2;
		
    	/* !!! TBR !!! - check this */
	device->displays[0].start_cell = 4;	
	device->displays[0].width = 66;
	device->displays[0].type = BRL_DISP_MAIN;
		
	device->displays[1].start_cell = 0;
	device->displays[1].width = 4;
	device->displays[1].type = BRL_DISP_STATUS;
		
	alva_dev_type = ALVADT_570;
		
	rv = 1;
    }
	
    else if (strcmp ("ALVA544", device_name) == 0) 	
    {				
	device->cell_count = 44;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 24;
		
	device->display_count = 2;
		
	/* !!! TBR !!! - check this */
	device->displays[0].start_cell = 0;	
	device->displays[0].width = 40;
	device->displays[0].type = BRL_DISP_MAIN;
		
	device->displays[1].start_cell = 40;
	device->displays[1].width = 4;
	device->displays[1].type = BRL_DISP_STATUS;
		
	alva_dev_type = ALVADT_544;
		
	rv = 1;
    }

    else
    {
	/* unknown device */
	alva_dev_type = ALVADT_GENERIC;
    }
    
    if (rv)
    {
	/* fill device functions for the upper level */	
	device->send_dots = alva_brl_send_dots;
    	device->close_device = alva_brl_close_device;
		
	/* open serial communication */
	if (brl_ser_open_port (port))
	{
	    brl_ser_set_callback (alva_brl_input_parser);
	    rv = brl_ser_set_comm_param (9600, 'N', 1, 'N');		
	    brl_ser_init_glib_poll ();		
	    client_callback = device_callback;
	}	
	else
	{
	    rv = 0;
	}
    }

    return rv;
}

