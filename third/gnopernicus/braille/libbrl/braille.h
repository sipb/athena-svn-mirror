/* braille.h
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

#ifndef __BRAILLE_H__
#define __BRAILLE_H__

#include <glib.h>

#define MAXBRCELL	256
#define MAXBRDISP	8

typedef enum
{
    BRL_EVCODE_RAW_BYTE,
    BRL_EVCODE_KEY_BITS,
    BRL_EVCODE_KEY_CODES,
    BRL_EVCODE_SWITCH_PAD,
    BRL_EVCODE_SENSOR   
} BRLEventCode;

typedef enum
{
    BRL_SENSOR_UNKNOWN,
    BRL_SENSOR_OPTICAL,
    BRL_SENSOR_MECHANICAL
} BRLSensorTechnology;

typedef enum 
{
    BRL_DISP_UNDEFINED,
    BRL_DISP_MAIN,
    BRL_DISP_STATUS,
    BRL_DISP_HORIZONTAL,
    BRL_DISP_VERTICAL
} BRLDisplayType;

typedef union
{
    guchar	             raw_byte;
    gulong	             key_bits;  /* fired every time one of the key changed status */
    gchar*	             key_codes; /* fired only when all keys are released */

    struct
    {
	gulong  	     switch_bits;
	gchar* 		     switch_codes;
    } switch_pad;
	
    struct
    {		
	gshort   	     bank;
	gshort 		     value;
	gshort               associated_display;
	BRLSensorTechnology  technology;		
	gchar*               sensor_codes;
    } sensor;
} BRLEventData;

typedef void (* BRLDevCallback) (BRLEventCode code,
			         BRLEventData *data);

typedef struct
{
    gshort          start_cell;
    gshort          width;	
    BRLDisplayType  type;		
} BRLDisplay;

typedef void  (* BRLDevCloseDeviceProc) ();
typedef gint  (* BRLDevSendDotsProc)    (guchar *dots,
				        gshort length,
				        gshort blocking);

typedef enum 
{
    BRL_INP_BITS,
    BRL_INP_MAKE_BREAK_CODES,
    BRL_INP_MAKE_CODES
} BRLInputType ;

typedef struct
{
    gshort			cell_count;
    gshort			display_count;
    BRLDisplay			displays[MAXBRDISP];
    BRLInputType		input_type;	
    gshort 			key_count;		
    gshort			switch_count;
    gshort			sensor_bank_count;
    BRLDevCloseDeviceProc	close_device;
    BRLDevSendDotsProc		send_dots;		
} BRLDevice;

typedef struct
{
    gulong		attribute;
    guchar		mask;
    guchar 		dots;
} BRLAttrMapping;
		
typedef void (* BRLEventProc) (BRLEventCode event_code,
			      BRLEventData *event_data);

/* *** API Functions *** */

void   brl_init        ();
void   brl_terminate   ();

/* DEVICE LEVEL */
gint   brl_open_device   (gchar       *device_name, 
		         gint         port,
		         BRLEventProc braille_event_proc);	
gint   brl_get_device    (BRLDevice   *device);
void   brl_close_device  ();

/* DISPLAY LEVEL */
void   brl_clear_all         ();
void   brl_clear_display     (gshort      display);
gshort brl_get_display_width (gshort      display);
void   brl_set_dots          (gshort      display, 
			     gshort       start_cell,
			     guchar       *new_dots,
			     gshort       cell_count,
			     gshort 	  offset,
			     gshort       cursor_position);
void   brl_update_dots       (gshort      blocking);
gshort brl_get_disp_id       (gchar       *role,
		             gshort       no);

/* LOWEST LEVEL */
gint brl_send_raw_data  (guchar      *bytes,
                        gint         count,
		        gshort       blocking);

#endif
