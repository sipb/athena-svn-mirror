/* brlinp.h
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

#ifndef __BRLINP_H__
#define __BRLINP_H__

#include <glib.h>

/* Braille Input Parser States */
typedef enum
{
    BIPS_IDLE,
    BIPS_BRL_IN,
    BIPS_KEY,
    BIPS_SENSOR,
    BIPS_SWITCH,	
    BIPS_UNKNOWN
} BRLInParserStates;

/* Braille Input Event Type */
typedef enum
{	
    BIET_UNKNOWN,
    BIET_KEY,
    BIET_SENSOR,
    BIET_SWITCH
} BRLInEventTypes;

/* Braille Input Event */
typedef struct
{
    BRLInEventTypes	event_type;
    union
    {
	gchar     	*key_codes;	
	gchar     	*switch_codes;
	gchar 		*sensor_codes;
    } event_data;	
} BRLInEvent;

/* API */
typedef void (*BRLInCallback) (BRLInEvent* brl_in_event);

gint  brl_in_xml_init      (BRLInCallback callback_proc);	
void brl_in_xml_terminate  ();
void brl_in_xml_parse      (gchar         *buffer, 
			   gint           len);

#endif
