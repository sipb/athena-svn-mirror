/* baumbrl.c
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

#include "baumbrl.h"
#include "sercomm.h"

/* BAUM DEVICE MAPPINGS */

/* 1. KEY MAPPING */

/*		VARIO80 */
/* 			bit0-6 display keys (top), bit7-17 front keys, bit18-28 back keys, bit19-30 chord keys */
		
/* 		DM80P */
/* 			bit0-6 display keys */

/* 2. SWITCH MAPPING */

/*		DM80P */
/*			bit0-6 */

/*		INKA */
/*			bit0-4 */

/* 3. SENSOR BANK MAPPING (all devices) */

/* horizontal mechnical sensors above main display   - bank 0 */
/* horizontal optical sensors above main display     - bank 1 */
/* vertical optical sensors left                     - bank 2 */
/* vertical optical sensors right                    - bank 3 */
/* horizontal mechnical sensors above status display - bank 4 */
/* horizontal optical sensors above status display   - bank 5 */

/* Baum Devices */
typedef enum 
{
    BAUMDT_GENERIC,
    BAUMDT_VARIO40,
    BAUMDT_VARIO20,
    BAUMDT_VARIO80,
    BAUMDT_DM80p,
    BAUMDT_INKA
} BAUMDeviceType;

/* Device data */
typedef struct
{
    guchar   version;
    gushort  crc;
    guchar   volume;
    guchar   uart_data;
    guchar   speech_data[256];	/* OK 256 ? */
    guchar   speech_info;
    guchar   hos_length;
    guchar   hos_state[11];
    guchar   vos_state[8];
    guchar   hms_length;
    guchar   hms_state[11];
	
    guchar   switch_state;
	
    guchar   key_state;
    guchar   front_key_state;			/* VARIO80 */
    guchar   back_key_state;			/* VARIO80 */
    guchar   chord_key_state;			/* VARIO80 */
    gushort  front_key_state2;		        /* VARIO80 10 front keys model */
    gushort  back_key_state2;		        /* VARIO80 10 front keys model */
    gulong   cumulated_key_state;	        /* all above key states */
	
    guchar   hos_value;	
    guchar   vos_value_left;
    guchar   vos_value_right;
    guchar   hms_value;

    gchar    key_codes[256];	                /* device specific key codes (DK - display, FK - front, BK - back, CK - chord) */
    gchar    switch_codes[256];			/* device specific switch codes (SWxx) */
    gchar    sensor_codes[128];			/* device specific sensor codes (HMS - horizontal mechanical, HOS - horizontal optical sensor, LOS - vertical optical sensor left, ROS - vertical optical sensor right) */
	
    guchar   last_error;
} BAUMDeviceData;

/* Baum Input Parser States */
typedef enum
{	
    BIPS_IDLE = 0, 
    BIPS_EXP_ID,
    BIPS_EXP_VERSION,
    BIPS_EXP_VERSION_2,
    BIPS_EXP_VOLUME,
    BIPS_EXP_UART_DATA,
    BIPS_EXP_SPEECH_DATA,
    BIPS_EXP_SPEECH_INFO,
    BIPS_EXP_HOS_DATA,
    BIPS_EXP_VOS_DATA,
    BIPS_EXP_HMS_DATA,
    BIPS_EXP_SWITCHES, 
    BIPS_EXP_KEYS, 
    BIPS_EXP_FRONT_KEYS, 
    BIPS_EXP_BACK_KEYS,
    BIPS_EXP_CHORD_KEYS,
    BIPS_EXP_FRONT_KEYS_2, 
    BIPS_EXP_BACK_KEYS_2,
    BIPS_EXP_HOS_VALUE, 
    BIPS_EXP_VOS_VALUES, 
    BIPS_EXP_HMS_VALUE,
    BIPS_EXP_ERROR_CODE,
    BIPS_EXP_TEST_1,
    BIPS_EXP_TEST_2,
    BIPS_EXP_TEST_3		
} BIPStates;


/* Globals */
static BRLDevCallback	client_callback = NULL;
static BAUMDeviceType	baum_dev_type = BAUMDT_GENERIC;

static guchar mask8[] =	{0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

static gulong mask32[] = {
	0x00000001,0x00000002,0x00000004,0x00000008,0x00000010,0x00000020,0x00000040,0x00000080,
	0x00000100,0x00000200,0x00000400,0x00000800,0x00001000,0x00002000,0x00004000,0x00008000,	
	0x00010000,0x00020000,0x00040000,0x00080000,0x00100000,0x00200000,0x00400000,0x00800000,
	0x01000000,0x02000000,0x04000000,0x08000000,0x10000000,0x20000000,0x40000000,0x80000000
};


static BIPStates      bip_state = BIPS_IDLE;
static gshort         bip_index = 0;

static BAUMDeviceData baumdd;


void 
clear_device_data (BAUMDeviceData *bdd)
{
    memset (bdd, 0, sizeof(BAUMDeviceData));
}

void reset_bip()
{
    bip_index = 0;
    bip_state = BIPS_IDLE;
}

gshort
check_escape (guchar new_val)
{
    gshort rv = 0;
    static gshort esc = 0;
	
    if (esc)
    {
	/* waiting for the second escape */
	if (new_val == 0x1B)
	{
	    rv = 1; 
	}
	else
	{
	    reset_bip();  /* reset to idle */
	}
	esc = 0;
    }
    else
    {
	/* not waiting for the second escape */
	if (new_val == 0x1B)
	{
	    esc = 1;
	}
	else 
	{
	    rv = 1;
	    esc = 0;
	}
    }

    return rv;
}

gshort
get_no_from_bitmask (guchar *buff, 
		     gshort max_len)
{
    gshort rv = 0, i, j;
	
    for (i = 0; i < max_len; ++i)
    {
	/* fprintf (stderr, "%02x ", Buff[i]); */
	if (buff[i])
	{
	    for (j = 0; j < 8; ++j)
	     {
		if ( buff[i] & mask8[j])
		{
		    break;
		}
	    }
	    rv = 8 * i + j + 1;
	    break;
	}	
    }

    return rv;
}

void
on_keys_changed ()
{
    /* NOTE: called for any display, front, back, chord key changes */
    /* fires when all keys are released */
    gint i, kcix;
	
    BRLEventCode bec;
    BRLEventData bed;

    static gulong pressed_keys = 0;
	
    if (baumdd.cumulated_key_state)
    {
	/* some keys pressed, add the new pressed keys */
	pressed_keys |= baumdd.cumulated_key_state;	
    }
    else
    {
	/* all keys released */
	kcix = 0;
	for (i = 0; i < 32; ++i)
	{		
	    if (pressed_keys & mask32[i])
	    {
		/* key pressed */
		if (0 <= i && i <= 6)
		{
		    /* display keys */
		    kcix += sprintf (&baumdd.key_codes[kcix], "DK%02d", i);
		}		
		if (7 <= i && i <= 17)
		{
		    /* front keys */
		    kcix += sprintf (&baumdd.key_codes[kcix], "FK%02d", i);
		}		
		if (18 <= i && i <= 28)
		{
		    /* back keys */
		    kcix += sprintf (&baumdd.key_codes[kcix], "BK%02d", i);
		}		
		if (19 <= i && i <= 30)
		{
		    /* chord keys */
		    kcix += sprintf (&baumdd.key_codes[kcix], "CK%02d", i);
		}		
	    }	
	}
						
	/* fire the key code event		 */
	bec = BRL_EVCODE_KEY_CODES;
	bed.key_codes = baumdd.key_codes;						
	client_callback (bec, &bed);
		
	/* clear presed_keys */
	pressed_keys = 0;
    }		
}

void
on_sensors_changed (BRLEventData* bed)
{
    BRLEventCode bec;
    gint rise_event = 1;
	
    /* horizontal mechnical sensors above main display   - bank 0 */
    /* horizontal optical sensors above main display     - bank 1 */
    /* vertical optical sensors left                     - bank 2 */
    /* vertical optical sensors right                    - bank 3 */
    /* horizontal mechnical sensors above status display - bank 4 */
    /* horizontal optical sensors above status display   - bank 5 */
	
    /* build the sensor code */
	
    if (bed->sensor.value < 0)
    {
	/* all sensor released */
	baumdd.sensor_codes[0] = '\0';
    }
    else
    {	
  	switch (bed->sensor.technology)
  	{
	    /* one of the sensors is pressed */
 	    case BRL_SENSOR_OPTICAL:
		switch (bed->sensor.bank)
 		    {
 			case 1:	/* horizontal optical sensors above main display */
 			    sprintf (&baumdd.sensor_codes[0], "HOS%02d", bed->sensor.value);
 			break;
 				
 			case 2:	/* vertical optical sensors left */
 			    sprintf (&baumdd.sensor_codes[0], "LOS%02d", bed->sensor.value);
 			break;
 				
 			case 3:	/* vertical optical sensors right */
 			    sprintf (&baumdd.sensor_codes[0], "ROS%02d", bed->sensor.value);
 			break;
 				
 			default:
 			    rise_event = 0;
 			break;
 		    }
 	    break;
 		
 	    case BRL_SENSOR_MECHANICAL:
 		sprintf (&baumdd.sensor_codes[0], "HMS%02d", bed->sensor.value);
 	    break;
 		
 	    default:
 		rise_event = 0;
 	    break;
  	}
    }
	/* add the switch codes and fire the sensor event		 */
    if (rise_event)
    {
	bec = BRL_EVCODE_SENSOR;	
	bed->sensor.sensor_codes = baumdd.sensor_codes;						
	client_callback (bec, bed);
    }		
}

void
on_switch_pad_changed (BRLEventData* bed)
{
    gint i, scix;
    BRLEventCode bec;
	
    /* build the switch_pad codes */
    scix = 0;
    for (i = 0; i < 32; ++i)
    {
	if (baumdd.switch_state & mask32[i])
	{
	    scix += sprintf (&baumdd.switch_codes[scix], "SW%02d", i);
	}		
    }
	
    /* add the switch codes and fire switch_pad event		 */
    /* NOTE: the entire switchpad is sent on every change */

    bec = BRL_EVCODE_SWITCH_PAD;	/* NOTE: includes switch bits and codes */
    bed->switch_pad.switch_codes = baumdd.switch_codes;						
    client_callback (bec, bed);
}


gshort
baum_brl_input_parser (gint new_val)
{
    static guchar       bip_buff[256];
    static BRLEventCode	bec;
    static BRLEventData bed;
    guchar t_new_val = 0;
    static guchar inka_key_to_std_key_bitmap [] = {2, 1, 0, 5, 4, 3, 6, 7};
    gshort i;

    if (!client_callback) 	
	return 0;	/* if we don't have a client callback doesn't make sense to go further */

    bec = BRL_EVCODE_RAW_BYTE;
    bed.raw_byte = new_val;
    client_callback (bec, &bed);

    switch (bip_state)
    {	
	case BIPS_IDLE:		
    	    if (new_val == 0x1b)
	    {
		/* info-block start */
		bip_state = BIPS_EXP_ID;
		bip_index = 0;
	    }
	break;

	case BIPS_EXP_ID:	
	    switch (new_val)
	    {			
		case 0x05:
		    bip_state = BIPS_EXP_VERSION;
		break;
		
		case 0x06:
		    bip_state = BIPS_EXP_VERSION_2;
		break;
		
		case 0x09:
		    bip_state = BIPS_EXP_VOLUME;
		break;
		
		case 0x0a:
		    bip_state = BIPS_EXP_UART_DATA;
		break;
		
		case 0x0b:
		    bip_state = BIPS_EXP_SPEECH_DATA;
		break;
		
		case 0x14:
		    bip_state = BIPS_EXP_SPEECH_INFO;
		break;

		case 0x1B:	/* double escape, framing error, stay idle */
		    bip_state = BIPS_IDLE;
		break;

		case 0x20:
		    bip_state = BIPS_EXP_HOS_DATA;
		break;
		
		case 0x21:
		    bip_state = BIPS_EXP_VOS_DATA;
		break;
		
		case 0x22:
		    if (baum_dev_type == BAUMDT_INKA)
		    {
			/* SPECIAL CASE INKA */
			bip_state = BIPS_EXP_SWITCHES;
		    }
		    else
		    {
			bip_state = BIPS_EXP_HMS_DATA;
		    }
		break;
		
		case 0x23:
		    bip_state = BIPS_EXP_SWITCHES;
		break;
		
		case 0x24:
		    bip_state = BIPS_EXP_KEYS;
		break;
		
		case 0x25:
		    bip_state = BIPS_EXP_HOS_VALUE;
		break;
		
		case 0x26:
		    bip_state = BIPS_EXP_VOS_VALUES;
		break;
		
		case 0x27:
		    bip_state = BIPS_EXP_HMS_VALUE;
		break;

		case 0x28:
		    bip_state = BIPS_EXP_FRONT_KEYS;
		break;

		case 0x29:
		    bip_state = BIPS_EXP_BACK_KEYS;
		break;

		case 0x2B:
		    bip_state = BIPS_EXP_CHORD_KEYS;
		break;

		case 0x2C:
		    bip_state = BIPS_EXP_FRONT_KEYS_2;
		break;

		case 0x2D:
		    bip_state = BIPS_EXP_BACK_KEYS_2;
		break;

		case 0x40:
		    bip_state = BIPS_EXP_ERROR_CODE;
		break;
		
		case 0x62:
		    bip_state = BIPS_EXP_TEST_1;
		break;
		
		case 0x63:
		    bip_state = BIPS_EXP_TEST_2;
		break;
		
		case 0x64:
		    bip_state = BIPS_EXP_TEST_3;
		break;
		
		default:
		    /* unknown packet ID */
		    reset_bip();
		break;
	    }
	break;
	
	case BIPS_EXP_VERSION:
	    if (check_escape (new_val))
	    {
		baumdd.version = new_val;
		reset_bip();
	    }
	break;
		
	case BIPS_EXP_VERSION_2:
	    if (check_escape (new_val))
	    {
		if (bip_index  < sizeof (bip_buff))
		{
		    bip_buff[bip_index++] = new_val;
		    if (bip_index >= 3)
		    {
			baumdd.version = bip_buff[0];
			baumdd.crc = bip_buff[2] + (((short)bip_buff[1]) << 8);			/* !!! TBR !!! */
			reset_bip();
		    }
		}
		else
		{
		    /* buffer overflow */
		    reset_bip();
		}
	    }
	break;
		
	case BIPS_EXP_VOLUME:
	    if (check_escape (new_val))
	    {
		baumdd.volume = new_val;
		reset_bip();
	    }
	break;
		
	case BIPS_EXP_UART_DATA:
	    if (check_escape (new_val))
	    {
		baumdd.uart_data = new_val;
		reset_bip();
	    }
	break;

	case BIPS_EXP_SPEECH_DATA:		
	    if (check_escape (new_val))
	    {		
		if (bip_index < sizeof (bip_buff))
		{
		    bip_buff[bip_index++] = new_val;
		    if (new_val == '\r')
		    {
			memcpy (&baumdd.speech_data[0], &bip_buff[0], bip_index);
			reset_bip();
		    }				
		}
		else
		{
		    /* buffer overflow, reset parser */
		    reset_bip();
		}			
	    }
	break;

	case BIPS_EXP_SPEECH_INFO:
	    if (check_escape (new_val))
	    {
		baumdd.speech_info = new_val;
		reset_bip();
	    }
	break;

	case BIPS_EXP_HOS_DATA:		
	    if (check_escape (new_val))
	    {
		if (bip_index  < sizeof (bip_buff))
		{			
		    bip_buff[bip_index++] = new_val;
		    if (bip_index >= baumdd.hos_length)
		    {
			/* copy to device data */
			memcpy (&baumdd.hos_state[0], &bip_buff[0], baumdd.hos_length);
			/* compute sensor number */
			baumdd.hos_value = get_no_from_bitmask (&baumdd.hos_state[0], baumdd.hos_length);
			bec = BRL_EVCODE_SENSOR;		
			bed.sensor.bank = 1;											/* horizontal optical sensors mapped as bank 1 */
			bed.sensor.value = baumdd.hos_value - 1;        /* 0 based for C ... */
			bed.sensor.associated_display = 0;             	/* associated to main display */
			bed.sensor.technology = BRL_SENSOR_OPTICAL;
			on_sensors_changed (&bed);
			reset_bip();
		    }
		}
		else
		{
		    /* buffer overflow */
		    reset_bip();
		}
	    }
	break;

	case BIPS_EXP_VOS_DATA:		
	    if (check_escape (new_val))
	    {
		if (bip_index  < sizeof (bip_buff))
		{							
		    bip_buff[bip_index++] = new_val;
	
		    if (bip_index >= 8)
		    {			
			/* copy to device data */
			memcpy (&baumdd.vos_state[0], &bip_buff[0], 8);

			/* compute the VOS values */
			baumdd.vos_value_left = get_no_from_bitmask (&baumdd.vos_state[0], 4);
			baumdd.vos_value_right = get_no_from_bitmask (&baumdd.vos_state[4], 4);						

			/* common for both banks */
			bec = BRL_EVCODE_SENSOR;				
			bed.sensor.associated_display = -1;  /* no associated display */
			bed.sensor.technology = BRL_SENSOR_OPTICAL;

			/* left sensor bank */
			bed.sensor.bank = 2;														/* horizontal mechnaical sensors mapped as bank 0 */
			bed.sensor.value = baumdd.vos_value_left - 1;   
			on_sensors_changed (&bed);

			/* right sensor bank */
			bed.sensor.bank = 3;													/* horizontal mechnaical sensors mapped as bank 0 */
			bed.sensor.value = baumdd.vos_value_right - 1;   /* 0 based for C ... */
			on_sensors_changed (&bed);
						
			reset_bip();
		    }
		}
		else
		{
		    reset_bip();
		}
	    }
	break;

	case BIPS_EXP_HMS_DATA:
	    if (check_escape (new_val))
	    {
		if (bip_index < sizeof (bip_buff))
		{			
		    bip_buff[bip_index++] = new_val;
		    if (bip_index >= baumdd.hms_length)
		    {			
			/* copy to device data */
			memcpy (&baumdd.hms_state[0], &bip_buff[0], baumdd.hms_length);
						
			/* compute the value */
			baumdd.hms_value = get_no_from_bitmask (&baumdd.hms_state[0], baumdd.hms_length);									
			bec = BRL_EVCODE_SENSOR;				

			bed.sensor.bank = 0;											/* horizontal mechanical sensors mapped as bank 0 */
			bed.sensor.value = baumdd.hms_value - 1;   /* 0 based for C ... */
			bed.sensor.associated_display = 0;             	/* associated to main display */
			bed.sensor.technology = BRL_SENSOR_MECHANICAL;
				
			on_sensors_changed (&bed);
					
			reset_bip();
		    }
		}
		else
		{
		    reset_bip();
		}
	    }
	break;

	case BIPS_EXP_SWITCHES:						
	    if (check_escape (new_val))
	    {	
		/* special case INKA */
		if (baum_dev_type == BAUMDT_INKA)
		{
		    new_val &= 0x0F;	/* only 4 switches */
		}
		
		/* store the new switch state */
		baumdd.switch_state = new_val;
		bed.switch_pad.switch_bits = baumdd.switch_state;						
		on_switch_pad_changed (&bed);
				
		/* reset the state machine */
		reset_bip();				
	   }	
	break;

	case BIPS_EXP_KEYS:		
	    if (check_escape (new_val))
	    {				
		switch (baum_dev_type)
		{
		    case BAUMDT_DM80p:																	
			/* direct mapping, device has no other keys */
			baumdd.cumulated_key_state = baumdd.key_state= ~new_val & 0x7F;	/* invert the status bit, 1 means make, 0 means break */
		    break;
					
		    case BAUMDT_VARIO40:
		    case BAUMDT_VARIO20:
			/* direct mapping, device has no other keys */
			baumdd.cumulated_key_state = baumdd.key_state = new_val & 0x3F;	
		    break;
					
		    case BAUMDT_VARIO80:										
			baumdd.key_state = new_val  & 0x3F;	
						
			/* map the actual keys togheter with front-, back- and chord keys */
			baumdd.cumulated_key_state = ((gulong) baumdd.key_state & 0x0000003F)+
						      (((gulong ) baumdd.front_key_state) << 6 & 0x0000FFC0)+
						      (((gulong ) baumdd.back_key_state) << 16 & 0x03FF0000)+
					              (((unsigned long ) baumdd.chord_key_state) << 26 & 0x7C000000);
		    break;

		    case BAUMDT_INKA:
			/* special case INKA - different layout */
			new_val ^= 0x3F;
			t_new_val = 0;
						
			for (i = 0; i < 8; ++i)
			{
			    if (new_val & mask8[i])
			    {
				t_new_val |= mask8[inka_key_to_std_key_bitmap[i]];
			    }
			}

			baumdd.key_state = ~t_new_val;	/* invert the status bit, 1 means make, 0 means break; */
						
			/* direct mapping, device has no other keys */
			baumdd.cumulated_key_state = baumdd.key_state;
		    break;

		    case BAUMDT_GENERIC:
		    default:
		    break;
		}

		/* fire the key bits				 */
		bec = BRL_EVCODE_KEY_BITS;
		bed.key_bits = baumdd.cumulated_key_state;						
		client_callback (bec, &bed);
								
		/* call for further processing				 */
		on_keys_changed ();
				
		/* reset the state machine */
		reset_bip();
	    }
	break;
	
	case BIPS_EXP_FRONT_KEYS:		
	    if (check_escape (new_val))
	    {
		/* store the state */
		baumdd.front_key_state = new_val;

		fprintf (stderr, "\nFRONT_KEY: %02x", baumdd.front_key_state);						
				
		/* reset the state machine */
		reset_bip();	
	    }
	break;

	case BIPS_EXP_BACK_KEYS:		
	    if (check_escape (new_val))
	    {
		/* store the state */
		baumdd.back_key_state = new_val;
				
		fprintf (stderr, "\nBACK_KEY: %02x", baumdd.back_key_state);										
				
		/* reset the state machine */
		reset_bip();
			
	    	}
	break;

	case BIPS_EXP_CHORD_KEYS:
	    if (check_escape (new_val))
	    {
		/* store the state */
		baumdd.chord_key_state = new_val;

		fprintf (stderr, "\nCHORD_KEY: %02x", baumdd.chord_key_state);										

		/* reset the state machine */
		reset_bip();		
	    }
	break;

	case BIPS_EXP_FRONT_KEYS_2:		
    	    if (check_escape (new_val))
	    {
		if (bip_index < 2)
		 {
		    bip_buff[bip_index++] = new_val;					
    		    if (bip_index >= 2)
		    {
			fprintf (stderr, "\nFRONT_KEY2");

			reset_bip();
		    }
		}
		else
		{
		    /* buffer overflow */
		    reset_bip();
		}
	    }
	break;

	case BIPS_EXP_BACK_KEYS_2:		
	    if (check_escape (new_val))
	    {
		if (bip_index  < 2)
		{
		    bip_buff[bip_index++] = new_val;
		    if (bip_index >= 2)
		    {
                	fprintf (stderr, "\nBACK_KEY2");
			reset_bip();
		    	}
	    	    }
		else
		{
		    /* buffer overflow */
		    reset_bip();
		}
	    }
	break;

	case BIPS_EXP_HOS_VALUE:		
	    if (check_escape (new_val))
	    {			
		baumdd.hos_value = new_val;				

		/* fprintf (stderr, "\nHOS_VAL2: %d", baumdd.HOSValue);				 */
		bec = BRL_EVCODE_SENSOR;				

		bed.sensor.bank = 1;											/* horizontal optical sensors mapped as bank 1 */
		bed.sensor.value = baumdd.hos_value - 1;   /* 0 based for C ... */
		bed.sensor.associated_display = 0;             	/* associated to main display */
		bed.sensor.technology = BRL_SENSOR_OPTICAL;
				
		on_sensors_changed (&bed);

		reset_bip();
	    }
	break;

	case BIPS_EXP_VOS_VALUES:
	    if (check_escape (new_val))
	    {	
		bip_buff[bip_index++] = new_val;
				
		switch (baum_dev_type)
		{				
		    case BAUMDT_INKA:
			/* special case INKA  !!! TBR !!! */
			bed.sensor.associated_display = -1;   /* no associated display */
			bed.sensor.technology = BRL_SENSOR_OPTICAL;
										
			if (bip_buff[0] >= 65)
			{
			    baumdd.vos_value_right = bip_buff[0] - 65;

			    bed.sensor.bank = 3;																	/* horizontal mechnaical sensors mapped as bank 0 */
			    bed.sensor.value = baumdd.vos_value_right - 1;		/* 0 based for C ... */
			
			    on_sensors_changed (&bed);
			}
			else
			{
			    baumdd.vos_value_left = bip_buff[0];

			    bed.sensor.bank = 2;																/* horizontal mechnaical sensors mapped as bank 0 */
			    bed.sensor.value = baumdd.vos_value_left - 1;   /* 0 based for C ... */
					
			    on_sensors_changed (&bed);
			}		
										
			reset_bip();					
		    break;

		    default:
			/* DM80P and all others */
			if (bip_index >= 2)
			{
			    baumdd.vos_value_left = bip_buff[0];
			    baumdd.vos_value_right = bip_buff[1];

        		    /* common for both banks */
			    bec = BRL_EVCODE_SENSOR;				
			    bed.sensor.associated_display = -1;  /* no associated display */
			    bed.sensor.technology = BRL_SENSOR_OPTICAL;

			    /* left sensor bank */
			    bed.sensor.bank = 2;																/* horizontal mechnaical sensors mapped as bank 0 */
			    bed.sensor.value = baumdd.vos_value_left - 1;   /* 0 based for C ...				 */
			    on_sensors_changed (&bed);

			    /* right sensor bank */
    			    bed.sensor.bank = 3;													/* horizontal mechnaical sensors mapped as bank 0 */
			    bed.sensor.value = baumdd.vos_value_right - 1;   /* 0 based for C ... */
			    on_sensors_changed (&bed);
						
			    reset_bip();
			}	
		    break;
		}
	    }
	break;
		
	case BIPS_EXP_HMS_VALUE:
	    if (check_escape (new_val))
	    {
		baumdd.hms_value = new_val;				
								
		bec = BRL_EVCODE_SENSOR;			
		bed.sensor.bank = 0;		           /* horizontal mechnaical sensors mapped as bank 0 */
		bed.sensor.value = baumdd.hms_value - 1;   /* 0 based for C ... */
		bed.sensor.associated_display = 0;         /* associated to main display */
		bed.sensor.technology = BRL_SENSOR_MECHANICAL;
				
		on_sensors_changed (&bed);

		reset_bip();
	    }
	break;

	case BIPS_EXP_ERROR_CODE:
	{
	    baumdd.last_error = new_val;			
	    fprintf (stderr, "\nBAUM DEVICE ERROR : %d", baumdd.last_error);												
	    reset_bip();
	}
	break;

	default:
	    /* internal error, invalid parser state */
	    reset_bip();
	break;
    }

    return 0;
}

gint 
baum_brl_send_dots (guchar *dots,
		    gshort count,
		    gshort blocking)
{
    gint rv = 0, i, realcnt = 0;
    guchar sendbuff[256];
			
    if (baum_dev_type == BAUMDT_INKA)
    {
	sendbuff[0] = 0x1b;
	sendbuff[1] = 0x01;
	sendbuff[2] = 0x00;	/* special case: INKA needs a pos byte here */
	realcnt = 3;
    }
    else
    {
	sendbuff[0] = 0x1b;
	sendbuff[1] = 0x01;
	realcnt = 2;	
    }

    /* go byte by byte, take care of the "ESC doubling" */
    for (i = 0; i < count; ++i) 
    {
	if (dots[i] == 0x1B)
	{
	    sendbuff [realcnt++] = dots[i];
	}
	sendbuff [realcnt++] = dots[i];
		
    }

    rv = brl_ser_send_data ((gchar*)sendbuff, realcnt, blocking);

    return rv;
}

void baum_brl_close_device ()
{
    /* close serial communication */
    brl_ser_set_callback (NULL);				
    brl_ser_stop_timer ();	
    brl_ser_close_port ();
}

gint
baum_brl_open_device (gchar          *device_name,
		      gshort         port,
		      BRLDevCallback device_callback,
		      BRLDevice      *device)
{
    int rv = 0;

    clear_device_data (&baumdd);

    if ((strcmp ("VARIO", device_name) == 0) ||
	(strcmp ("VARIO40", device_name) == 0))
    {
	device->cell_count = 40;
	device->display_count = 1;
	device->displays[0].start_cell = 0;
	device->displays[0].width = 40;
	device->displays[0].type = BRL_DISP_MAIN;
	device->input_type = BRL_INP_BITS;
	device->key_count = 6;

	baumdd.hms_length = 5;
	baum_dev_type = BAUMDT_VARIO40;
	rv = 1;
    }

    else if (strcmp ("VARIO20", device_name) == 0) 	
    {
	device->cell_count = 20;
	device->display_count = 1;
	device->displays[0].start_cell = 0;
	device->displays[0].width = 20;
	device->displays[0].type = BRL_DISP_MAIN;
	device->input_type = BRL_INP_BITS;
	device->key_count = 6;
	
	baumdd.hms_length = 5;	/* ??? */
	baum_dev_type = BAUMDT_VARIO20;

	rv = 1;
    }

    else if (strcmp ("VARIO80", device_name) == 0) 	
    {		
	device->cell_count = 84;
	device->display_count = 2;
	
	device->displays[0].start_cell = 0;
	device->displays[0].width = 80;
	device->displays[0].type = BRL_DISP_MAIN;
	
	device->displays[1].start_cell = 80;
    	device->displays[1].width = 4;
	device->displays[1].type = BRL_DISP_STATUS;
	
	device->input_type = BRL_INP_BITS;
	device->key_count = 31;
	device->switch_count = 0;
	device->sensor_bank_count = 2;

	baumdd.hms_length = 11;
	baum_dev_type = BAUMDT_VARIO80;
	rv = 1;
    }

    else if (strcmp ("DM80P", device_name) == 0) 	
    {	
	device->cell_count = 84;
	device->display_count = 2;
	
	device->displays[0].start_cell = 0;
	device->displays[0].width = 80;
	device->displays[0].type = BRL_DISP_MAIN;
	
	device->displays[1].start_cell = 80;
	device->displays[1].width = 4;
	device->displays[1].type = BRL_DISP_STATUS;
	
	device->input_type = BRL_INP_BITS;
	device->key_count = 7;
	device->switch_count = 6;
	device->sensor_bank_count = 5;

	baum_dev_type = BAUMDT_DM80p;

	rv = 1;
    }

    else if (strcmp ("INKA", device_name) == 0) 	
    {
	device->cell_count = 44;
	device->display_count = 2;
	
	device->displays[0].start_cell = 0;
	device->displays[0].width = 40;
	device->displays[0].type = BRL_DISP_MAIN;
	
	device->displays[1].start_cell = 40;
	device->displays[1].width = 4;
	device->displays[1].type = BRL_DISP_STATUS;

	device->input_type = BRL_INP_BITS;
	device->key_count = 6;

	baumdd.hms_length = 5;	/* ??? */
	baum_dev_type = BAUMDT_INKA;

	rv = 1;
    }
    
    else
    {
	/* unknown device */
	baum_dev_type = BAUMDT_GENERIC;
    }

    if (rv)
    {
	/* fill device functions */
	device->close_device = baum_brl_close_device;
	device->send_dots = baum_brl_send_dots;

	/* open serial communication */
	if (brl_ser_open_port (port))
	{
	    brl_ser_set_callback (baum_brl_input_parser);				
	    rv = brl_ser_set_comm_param (19200, 'N', 1, 'N');		
	    rv &= brl_ser_init_glib_poll ();		/* SYNC CALLBACK */
	    client_callback = device_callback;

 	    rv &= brl_ser_send_data ("\x1B\x08", 2, 1);
	}	
	else
	{
	    rv = 0;
	}
    }

    return rv;
}
