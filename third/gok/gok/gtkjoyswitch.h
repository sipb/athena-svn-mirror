/* switchapi.h
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto 
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
 
#ifndef __gtkjoyswitch_h 
#define __gtkjoyswitch_h 
 
#include <gtk/gtk.h> 
 
#define JOYSTICK_MAXB	16 
 
typedef void (*joystick_callback)(gint, gint, gpointer);  
	 
typedef struct _JoySwitchData { 
	int joystickfd; 
	int version; 
	int num_buttons; 
	char * device_name; 
	joystick_callback callback_up[JOYSTICK_MAXB]; 
	joystick_callback callback_down[JOYSTICK_MAXB]; 
	gpointer callback_data; 
	gint gdktag; 
	int buttons[JOYSTICK_MAXB];  
} JoySwitchData; 
 
 
#endif 
