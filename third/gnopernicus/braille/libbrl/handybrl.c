/* handybrl.c
 *
 * Copyright 2003 HandyTech Elektronik GmbH
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
#include "handybrl.h"
#include "sercomm.h"


#define MAX_BUFFER 90
#define MAX_GET_BUFFER 10
#define MAX_KEYCODE_LENGHT 30


/* Supportet HandyTech devices */
typedef enum 
{
    HANDY_BRW=0, 
    HANDY_BL2, 
    HANDY_BS4, 
    HANDY_BS8, 
    HANDY_MB2, 
    HANDY_MB4, 
    HANDY_MB8
} HANDYDeviceType;

/* key-states */
typedef enum 
{
    KEY_IDLE=0,
    KEY_PRESSED,
    KEY_RELINQUISHED,
    KEY_EVALUATED
} HANDYKeyStates;

/* keys */
typedef enum
{
    K_F1=0, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, /* braille keys */                 
    K_UP, K_DOWN, K_ENTER, K_ESC, K_SPACE, K_RSPACE,  /* function keys*/             
    K_B9, K_B10, K_B11, K_B12, K_B13, K_B14,          /* 16er block */
    K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9,
    K_R01, K_R02, K_03, K_R04, K_R05, K_R06, K_R07, K_R08, K_R09, K_R10,    /* courser routing over textmodules */
    K_R11, K_R12, K_13, K_R14, K_R15, K_R16, K_R17, K_R18, K_R19, K_R20,
    K_R21, K_R22, K_23, K_R24, K_R25, K_R26, K_R27, K_R28, K_R29, K_R30,
    K_R31, K_R32, K_33, K_R34, K_R35, K_R36, K_R37, K_R38, K_R39, k_R40,
    K_R41, K_R42, K_43, K_R44, K_R45, K_R46, K_R47, K_R48, K_R49, K_R50,
    K_R51, K_R52, K_53, K_R54, K_R55, K_R56, K_R57, k_R58, k_R59, K_R60,
    K_R61, K_R62, k_63, K_R64, K_R65, K_R66, K_R67, K_R68, K_R69, K_R70,
    K_R71, K_R72, K_73, K_R74, K_R75, K_R76, K_R77, K_R78, K_R79, K_R80,
    K_S1, K_S2, K_S3, K_S4,                                                 /* courser routing over statusmodules */ 
    MAX_KEYS
} HANDYKeys;


/* HandyTech Device Data Type */
typedef struct 
{
    HANDYDeviceType  name; 	
    guchar           cell_count;
    guchar           id_byte;       /* Identifier Byte of Braille-Device*/
} HANDYDeviceData;


static BRLDevCallback   client_callback = NULL;
static HANDYDeviceData  handy_device_data; 
static guchar           getbuffer[MAX_GET_BUFFER];
static gshort           gb_index = 0;
static HANDYKeyStates   key_array[MAX_KEYS];

/* check, is frame complete */
gshort 
is_complete_frame()
{
    static gint countdown = -1;
	
    if (countdown == 0)  /* frame complete */
    {
	countdown = -1;
	if (getbuffer[getbuffer[2]+3] == 0x16) 
	    return 1;
	else
	     gb_index = 0; /* begin new frame */
    }
    else
    {
	if ((getbuffer[2] != 0) && (countdown < 0)) /* set countdown */
	{
	    countdown = getbuffer[2];
	}
	else if (countdown >= 0)
	{
	    countdown--;
	}
    }
    return 0;
}

/* set evaluated keys to preddes keys */
void
refresh_evaluated_to_pressed()
{
    gint i;

    for (i=0; i <= MAX_KEYS; i++)
    {
	if (key_array[i] == KEY_EVALUATED)
	    key_array[i] = KEY_PRESSED;
    }
}


/* check, which key-combination is pressed */
gint
on_key_changed()
{
    gint i;
    guchar do_evaluation = 0;

    for (i=1; i < getbuffer[2]; i++)
    {
    	switch (getbuffer[i+3])
	{	
	    /* braille-keys*/
	    case 0x03:                                     /* B1 bzw. 7 */
		key_array[K_F7] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x83:
		if (key_array[K_F7] != KEY_EVALUATED)
		{
		    key_array[K_F7] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_F7] = KEY_IDLE;
	    break;
	    
	    case 0x07:                                     /* B2 bzw. 3 */
		key_array[K_F3] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x87:
		if (key_array[K_F3] != KEY_EVALUATED)
		{
		    key_array[K_F3] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else key_array[K_F3] = KEY_IDLE;
	    break;
	    
	    case 0x0B:                                     /* B3 bzw. 2 */
		key_array[K_F2] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x8B:
		if (key_array[K_F2] != KEY_EVALUATED)
		{
		    key_array[K_F2] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else key_array[K_F2] = KEY_IDLE;
	    break;
	    
	    case 0x0F:                                     /* B4 bzw. 1 */
		key_array[K_F1] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x8F:
		if (key_array[K_F1] != KEY_EVALUATED)
		{
		    key_array[K_F1] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_F1] = KEY_IDLE;
	    break;
	    
	    
	    case 0x13:                                     /* B5 bzw. 4 */
		key_array[K_F4] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x93:
		if (key_array[K_F4] != KEY_EVALUATED)
		{
		    key_array[K_F4] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_F4] = KEY_IDLE;
	    break;
	    
	    case 0x17:                                     /* B6 bzw. 5 */
		key_array[K_F5] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x97:
		if (key_array[K_F5] != KEY_EVALUATED)
		{
		    key_array[K_F5] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_F5] = KEY_IDLE;
	    break;
	    
	    case 0x1B:                                     /* B7 bzw. 6 */
		key_array[K_F6] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x9B:
		if (key_array[K_F6] != KEY_EVALUATED)
		{
		    key_array[K_F6] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_F6] = KEY_IDLE;
	    break;
	    
	    case 0x1F:                                     /* B8 bzw. 8 */
		key_array[K_F8] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x9F:
		if (key_array[K_F8] != KEY_EVALUATED)
		{
		    key_array[K_F8] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_F8] = KEY_IDLE;
	    break;

		/* function keys */
	    case 0x04:                                     /* up */
		key_array[K_UP] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x84:
		if (key_array[K_UP] != KEY_EVALUATED)
		{
		    key_array[K_UP] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_UP] = KEY_IDLE;
	    break;
	    
	    case 0x08:                                     /* down */
		key_array[K_DOWN] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x88:
		if (key_array[K_DOWN] != KEY_EVALUATED)
		{
		    key_array[K_DOWN] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_DOWN] = KEY_IDLE;
	    break;
	    
	    case 0x14:                                     /* enter */
		key_array[K_ENTER] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x94:
		if (key_array[K_ENTER] != KEY_EVALUATED)
		{
		    key_array[K_ENTER] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_ENTER] = KEY_IDLE;
	    break;
	    
	    case 0x0C:                                     /* esc */
		key_array[K_ESC] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x8C:
		if (key_array[K_ESC] != KEY_EVALUATED)
		{
		    key_array[K_ESC] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_ESC] = KEY_IDLE;
	    break;
	    
	    case 0x10:                                     /* space */
		key_array[K_SPACE] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x90:
		if (key_array[K_SPACE] != KEY_EVALUATED)
		{
		    key_array[K_SPACE] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_SPACE] = KEY_IDLE;
	    break;
	    
	    case 0x18:                                     /* space right */
		key_array[K_RSPACE] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x98:
		if (key_array[K_RSPACE] != KEY_EVALUATED)
		{
		    key_array[K_RSPACE] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_RSPACE] = KEY_IDLE;
	    break;

	    /* 16er block */
	    case 0x12:                                     /* B9 */
		key_array[K_B9] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x92:
		if (key_array[K_B9] != KEY_EVALUATED)
		{
		    key_array[K_B9] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_B9] = KEY_IDLE;
	    break;
	    
	    case 0x02:                                     /* B10 */
		key_array[K_B10] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x82:
		if (key_array[K_B10] != KEY_EVALUATED)
		{
		    key_array[K_B10] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_B10] = KEY_IDLE;
	    break;
	    
	    case 0x11:                                     /* B11 */
		key_array[K_B11] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x91:
		if (key_array[K_B11] != KEY_EVALUATED)
		{
		    key_array[K_B11] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else
		    key_array[K_B11] = KEY_IDLE;
	    break;
	    
	    case 0x01:                                     /* B12 */
		key_array[K_B12] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x81:
		if (key_array[K_B12] != KEY_EVALUATED)
		{
		    key_array[K_B12] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
	        else 
		    key_array[K_B12] = KEY_IDLE;
	    break;
	    
	    case 0x09:                                     /* B13 */
		key_array[K_B13] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x89:
		if (key_array[K_B13] != KEY_EVALUATED)
		{
		    key_array[K_B13] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_B13] = KEY_IDLE;
	    break;
	    
	    case 0x0D:                                     /* B14 */
		key_array[K_B14] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x8D:
		if (key_array[K_B14] != KEY_EVALUATED)
		{
		    key_array[K_B14] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_B14] = KEY_IDLE;
	    break;
	    
	    case 0x05:                                     /* 0 */
		key_array[K_0] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x85:
		if (key_array[K_0] != KEY_EVALUATED)
		{
		    key_array[K_0] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_0] = KEY_IDLE;
	    break;
	    
	    case 0x15:                                     /* 1 */
		key_array[K_1] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x95:
		if (key_array[K_1] != KEY_EVALUATED)
		{
		    key_array[K_1] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_1] = KEY_IDLE;
	    break;
	    
	    case 0x19:                                     /* 2 */
		key_array[K_2] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x99:
		if (key_array[K_2] != KEY_EVALUATED)
		{
		    key_array[K_2] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_2] = KEY_IDLE;
	    break;
	    
	    case 0x1D:                                     /* 3 */
		key_array[K_3] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x9D:
		if (key_array[K_3] != KEY_EVALUATED)
		{
		    key_array[K_3] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_3] = KEY_IDLE;
	    break;
	    
	    case 0x06:                                     /* 4 */
		key_array[K_4] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x86:
		if (key_array[K_4] != KEY_EVALUATED)
		{
		    key_array[K_4] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_4] = KEY_IDLE;
	    break;
	    
	    case 0x0A:                                     /* 5 */
		/* keyArray[K_5] = KEY_PRESSED; Problem: read maps 0x0d -> 0x0a */
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x8A:
		if (key_array[K_5] != KEY_EVALUATED)
		{
		    key_array[K_5] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_5] = KEY_IDLE;
	    break;
	    
	    case 0x0E:                                     /* 6 */
		key_array[K_6] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x8E:
		if (key_array[K_6] != KEY_EVALUATED)
		{
		    key_array[K_6] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_6] = KEY_IDLE;
	    break;
	    
	    case 0x16:                                     /* 7 */
		key_array[K_7] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x96:
		if (key_array[K_7] != KEY_EVALUATED)
		{
		    key_array[K_7] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_7] = KEY_IDLE;
	    break;
	    
	    case 0x1A:                                     /* 8 */
		key_array[K_8] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x9A:
		if (key_array[K_8] != KEY_EVALUATED)
		{
		    key_array[K_8] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_8] = KEY_IDLE;
	    break;
	    
	    case 0x1E:                                     /* 9 */
		key_array[K_9] = KEY_PRESSED;
             	refresh_evaluated_to_pressed();
	    break;
	    
	    case 0x9E:
		if (key_array[K_9] != KEY_EVALUATED)
		{
		    key_array[K_9] = KEY_RELINQUISHED;
		    do_evaluation = 1;
		} 
		else 
		    key_array[K_9] = KEY_IDLE;
	    break;
	    		
	    /* courser routing */
	    default:
	    {
		gint j;
		for (j=0x20; j < (0x20 + handy_device_data.cell_count); j++)
		{
		    if (getbuffer[i+3] == j) 
		    {
			key_array[K_R01 + (j-0x20)] = KEY_PRESSED;
			refresh_evaluated_to_pressed();
		    }
		    else if (getbuffer[i+3] == (j | 0x80))
		    {
			if (key_array[K_R01+(j-0x20)] != KEY_EVALUATED)
			{
			    key_array[K_R01+(j-0x20)] = KEY_RELINQUISHED;
			    do_evaluation = 1;
			} 
			else key_array[K_R01+(j-0x20)] = KEY_IDLE;	
		    }
		}
	    }
	}
    }
    return do_evaluation;
}


/* evaluate the pressed key-combination */
gshort 
handy_brl_input_parser (gint new_val)
{
    static BRLEventCode bec;
    static BRLEventData bed;

    if (!client_callback) 
	return 0; /* no client callback, no sense to go further */
    bec = BRL_EVCODE_RAW_BYTE;
    bed.raw_byte = new_val;
    client_callback(bec, &bed);

    /* clear getbuffer */
    if (gb_index == 0) 
	memset(getbuffer, 0x00, MAX_GET_BUFFER);
	
    getbuffer[gb_index++] = new_val;

    if ((gb_index == 1) && (new_val != 0x79)) 
	gb_index = 0; /* if not framed -> ignore */

    if (is_complete_frame())
    {
	if (getbuffer[3] == 0x04)   /* key pressed */
	{
	    if (on_key_changed())
	    {
		gint i, index = 0;
		gchar key_code[MAX_KEYCODE_LENGHT];
			
		for (i=0; i < MAX_KEYCODE_LENGHT; i++)
		    key_code[i] = '\0';
				
		for (i=0; i < MAX_KEYS; i++) /* new state of keys*/
		{
		    /* prevent overflowe */
		    if (index > (MAX_KEYCODE_LENGHT-6)) 
			index = MAX_KEYCODE_LENGHT-6;

		    /* relinquish the keys */
		    if (key_array[i] != KEY_IDLE)
		    {
			switch (i)
			{
			    case K_UP:
				index += sprintf(&key_code[index], "Up");
			    break;	
			    
			    case K_DOWN:
				index += sprintf(&key_code[index], "Down");
			    break;
			    
			    case K_ENTER:
				index += sprintf(&key_code[index], "Enter");
			    break;
			    
			    case K_ESC:
				index += sprintf(&key_code[index], "Esc");
			    break;	
			    
			    case K_SPACE:
				index += sprintf(&key_code[index], "Space");
			    break;
			    
			    case K_RSPACE:
				index += sprintf(&key_code[index], "rSpace");
			    break;
			    
			    default:
			    {
				gint j;

				/* braille-keys DK01 - DK08 */
				if ((i >= K_F1) && (i <= K_F8))   
				    index += sprintf(&key_code[index], "DK%02d", i-K_F1+1);

				/* 16er Block Bxx & NUMx*/
				if ((i >= K_B9) && (i <= K_B14))
				    index += sprintf(&key_code[index], "B%02d", i-K_B9+9);
				if ((i >= K_0) && (i <= K_9))
				    index += sprintf(&key_code[index], "NUM%d", i-K_0);

				/* courser routing over textmodules HSMxx*/
				for (j=K_R01; j <= K_R80; j++)
				{
				    if (i == j) 
					index += sprintf(&key_code[index], "HMS%02d", j-K_R01);
				}

				/* courser routing over statusmodules Sx*/
				for (j=K_S1; j <= K_S4; j++)
				{
				    if (i == j) 
					index += sprintf(&key_code[index], "S%1d", j-K_S1);
				}
			    }
			}
		    }

		    /* set new state of keys */
		    if (key_array[i] == KEY_RELINQUISHED) 
			key_array[i] = KEY_IDLE;  
		    if (key_array[i] == KEY_PRESSED) 
			key_array[i] = KEY_EVALUATED;
		}

		/* adjust key-commands for Braille Star and Braillino */
		if (handy_device_data.name == HANDY_BL2 ||
		    handy_device_data.name == HANDY_BS4 || 
		    handy_device_data.name == HANDY_BS8)
		{
		    if (!strcmp(key_code, "Enter"))
			sprintf(key_code, "Down");
		    else if (!strcmp(key_code, "Esc")) 
			sprintf(key_code, "Up");
		    else if (!strcmp(key_code, "EnterEsc")) 
			sprintf(key_code, "Esc");
		    else if (!strcmp(key_code, "UpDown")) 
			sprintf(key_code, "Enter");
		}
	
		if (!strncmp(key_code, "HMS", 3))
		{
		    bec = BRL_EVCODE_SENSOR;   /* curser routing */
		    bed.sensor.sensor_codes = key_code;
		    client_callback(bec, &bed);
		} 
		else
		{
		    bec = BRL_EVCODE_KEY_CODES;                        /* other braille key */
		    bed.key_codes = key_code;
		    client_callback(bec, &bed);
		}

		fprintf(stderr, "-- %s --\n", key_code);

	    }
	}
	gb_index = 0; /* begin new frame */
    }

    return 0; /* 0-give next byte, 1-repeat last byte */
}


/* sends line to braille device */
gint
handy_brl_send_dots (guchar *dots, 
		     gshort count, 
		     gshort blocking)
{
    gint rv = 0;
    guchar sendbuffer[MAX_BUFFER];

    /* clear sendbuffer */
    memset(sendbuffer, 0x00, MAX_BUFFER);

    /* composing buffer to send */
    sendbuffer[0] = 0x79;
    sendbuffer[1] = handy_device_data.id_byte;
    sendbuffer[2] = handy_device_data.cell_count+1;
    sendbuffer[3] = 0x01;
    memcpy (&sendbuffer[4], dots, count);
    sendbuffer[handy_device_data.cell_count+4] = 0x16;
	
    /* sending buffer to display */
    rv = brl_ser_send_data (sendbuffer, handy_device_data.cell_count+5, blocking);

    return rv;
}


/* close braille driver*/
void
handy_brl_close_device ()
{
    /* close seial communication */
    brl_ser_set_callback (NULL);
    brl_ser_stop_timer ();
    brl_ser_close_port ();
}


/* open braille driver */
gint 
handy_brl_open_device (gchar          *device_name, 
		       gshort         port,
		       BRLDevCallback device_callback,
		       BRLDevice      *device) 
{
    gint rv = 0;

    /* Braille Wave */
    if (strcmp ("HTBRW", device_name) == 0)
    {
	device->cell_count = 40;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 13;

	device->display_count = 1;

	device->displays[0].start_cell = 0;
    	device->displays[0].width = 40;
	device->displays[0].type = BRL_DISP_MAIN;

    	handy_device_data.name = HANDY_BRW;
	handy_device_data.cell_count = 40;
	handy_device_data.id_byte = 0x05;

    	rv = 1;
    }

    /* Braillino */
    else if (strcmp ("HTBL2", device_name) == 0)
    {
	device->cell_count = 20;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 14;

	device->display_count = 1;

	device->displays[0].start_cell = 0;
	device->displays[0].width = 20;
	device->displays[0].type = BRL_DISP_MAIN;

	handy_device_data.name = HANDY_BL2;
    	handy_device_data.cell_count = 20;
    	handy_device_data.id_byte = 0x72;

	rv = 1;
    }

    /* Braille Star 40 */
    else if (strcmp ("HTBS4", device_name) == 0)
    {
	device->cell_count = 40;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 14;	

	device->display_count = 1;

	device->displays[0].start_cell = 0;
	device->displays[0].width = 40;
	device->displays[0].type = BRL_DISP_MAIN;

	handy_device_data.name = HANDY_BS4;
	handy_device_data.cell_count = 40;
	handy_device_data.id_byte = 0x74;		    

    	rv = 1;
    }

    /* Braille Star 80 */
    else if (strcmp ("HTBS8", device_name) == 0)
    {
	device->cell_count = 80;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 30;

	device->display_count = 1;

	device->displays[0].start_cell = 0;
	device->displays[0].width = 80;
	device->displays[0].type = BRL_DISP_MAIN;
		
	handy_device_data.name = HANDY_BS8;
	handy_device_data.cell_count = 80;
	handy_device_data.id_byte = 0x78;
		
	rv = 1;
    }

    /* Modular 44 */
    else if (strcmp ("HTMB4", device_name) == 0)
    {
	device->cell_count = 44;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 26;

	device->display_count = 2;

	device->displays[0].start_cell = 4;
	device->displays[0].width = 40;
	device->displays[0].type = BRL_DISP_MAIN;
		
	device->displays[1].start_cell = 0;
	device->displays[1].width = 4;
	device->displays[1].type = BRL_DISP_STATUS;

	handy_device_data.name = HANDY_MB4;
	handy_device_data.cell_count = 44;
	handy_device_data.id_byte = 0x89;

	rv = 1;
    }

    /* Modular 84 */
    else if (strcmp ("HTMB8", device_name) == 0)
    {
	device->cell_count = 84;
	device->input_type = BRL_INP_MAKE_BREAK_CODES;
	device->key_count = 26;

	device->display_count = 2;

	device->displays[0].start_cell = 4;
	device->displays[0].width = 80;
	device->displays[0].type = BRL_DISP_MAIN;
		
	device->displays[1].start_cell = 0;
	device->displays[1].width = 4;
	device->displays[1].type = BRL_DISP_STATUS;

	handy_device_data.name = HANDY_MB8;
	handy_device_data.cell_count = 84;
	handy_device_data.id_byte = 0x88;

	rv = 1;
    }

    else
    {
	/* Error: unknown device */
	rv = 0;
    }

    if (rv) /* device found */
    {
	/* fill device function with callback-functions */
	device->send_dots = handy_brl_send_dots;
	device->close_device = handy_brl_close_device;

	/* open serial communication */
	if (brl_ser_open_port (port))
	{
	    /* set serial parameter */
	    rv = handy_set_comm_param ();
	    
	    brl_ser_set_callback (handy_brl_input_parser);

	    /* reset braille-device */
	    rv = brl_ser_send_data ("\xFF", 1, 0);

	    /* set all keys to KEY_IDLE */			
	
	    brl_ser_init_glib_poll ();
	    client_callback = device_callback;
	}
	else 
	{
	    /* Error: Cannot open port */
	    rv = 0;
	}
    }
    return rv;
}
