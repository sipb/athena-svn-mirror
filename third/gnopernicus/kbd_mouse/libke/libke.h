/* libke.h
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
#ifndef _LIBKE_H
#define _LIBKE_H

#include <glib.h>
#include "SREvent.h"


#define KE_LAYER_TIME_OUT 1
#define KE_LAYER_CHANGED 0

/**
 * KeyboardEchoCB:
 *
 * prototype of the function used for communication.
 * The SREvent structure is used to encapsulate the character string 
 * received. The content of this string depends on the keyboard echo 
 * mode, which can be letter, word or auto. The flags is not used yet.
 *
**/
#define KeyboardEchoCB SROnEventProc

/**
 * SRHotkeyKeyModifiers:
 *
 * an enum used in SRHotkeyData indicating the key modifiers 
**/
typedef enum {
    SRHOTKEY_ALT 	= (1 << 0),
    SRHOTKEY_CTRL 	= (1 << 1),
    SRHOTKEY_SHIFT	= (1 << 2)
} SRHotkeyKeyModifiers;

/**
 * SRHotkeyData:
 *
 * the structure encapsulated in SREvent
 *
**/
typedef struct _SRHotkeyData
{
    unsigned long keyID;
    unsigned int modifiers;
    gchar *keystring; 
} SRHotkeyData;


int 	ke_init (KeyboardEchoCB kecb);
void 	ke_terminate ();
void 	ke_config_changed ();
void 	ke_set_default_configuration ();

#endif
