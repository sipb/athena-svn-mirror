/* switchapi.c 
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <sys/ioctl.h> 
#include <fcntl.h> 
#include <string.h> 
#include <errno.h> 
 
#include <gtk/gtk.h> 

#ifdef __linux__
#include <linux/joystick.h> 
#endif

#include "gtkjoyswitch.h" 
#include "switchapi.h" 
#include "gok-scanner.h"
#include "gok-log.h"
 
#ifdef __linux__
/* static */ 
static void joystick_event(gpointer data, gint fdsource, GdkInputCondition cond); 
static JoySwitchData * joydata; 
#endif
 
/** 
* initSwitchApi 
* 
* Initializes the swithc API. This must be called before using the switches. 
* 
* returns: The number of switches that the device can possibly use. Returns -1  
* if the switch API can't be initialized (so don't use it). 
**/ 
gint initSwitchApi()  
{ 
#ifdef __linux__
	static gboolean bInitialized = FALSE; 
	gchar devname[]="/dev/js0"; 
	gchar name[128] = "Unknown"; 
 
	if (bInitialized == TRUE) 
	{ 
		gok_log_x ("Warning: Calling initSwitchApi after it's already initialized!\n"); 
		return -1; 
	} 
	 
	joydata=(JoySwitchData *)g_malloc(sizeof(JoySwitchData)); 
 
	memset(joydata, 0, sizeof(JoySwitchData)); 
 
	/* open joystick device */ 
	joydata->joystickfd= open(devname, O_RDONLY); 
 
	if (joydata->joystickfd < 0) 
	{ 
		if ( errno == ENODEV ) 
		{ 
/*			printf("Note: No joystick connected in initSwitchApi\n"); */
		} 
		else 
		{ 
			gok_log_x ("Error: Couldn't open '%s' in initSwitchApi\n", devname); 
			perror(devname); 
		} 
		return -1; 
	} 
 
	/* load properties */  
	fcntl(joydata->joystickfd, F_SETFL, O_NONBLOCK); 
	ioctl(joydata->joystickfd, JSIOCGVERSION, &joydata->version); 
	ioctl(joydata->joystickfd, JSIOCGBUTTONS, &joydata->num_buttons); 
	ioctl(joydata->joystickfd, JSIOCGNAME(128), name); 
	joydata->device_name = g_strdup(name); 
	joydata->gdktag = gdk_input_add(joydata->joystickfd, GDK_INPUT_READ, joystick_event, joydata); 
 
	/* connect the switch events to the gok scanner */ 
	if (joydata->num_buttons >= 1) 
	{ 
		registerSwitchDownListener (0, gok_scanner_on_switch1_down); 
		registerSwitchUpListener (0, gok_scanner_on_switch1_up); 
	} 
	if (joydata->num_buttons >= 2) 
	{ 
		registerSwitchDownListener (1, gok_scanner_on_switch2_down); 
		registerSwitchUpListener (1, gok_scanner_on_switch2_up); 
	} 
	if (joydata->num_buttons >= 3) 
	{ 
		registerSwitchDownListener (2, gok_scanner_on_switch3_down); 
		registerSwitchUpListener (2, gok_scanner_on_switch3_up); 
	} 
	if (joydata->num_buttons >= 4) 
	{ 
		registerSwitchDownListener (3, gok_scanner_on_switch4_down); 
		registerSwitchUpListener (3, gok_scanner_on_switch4_up); 
	} 
	if (joydata->num_buttons >= 5) 
	{ 
		registerSwitchDownListener (4, gok_scanner_on_switch5_down); 
		registerSwitchUpListener (4, gok_scanner_on_switch5_up); 
	} 
	 
	bInitialized = TRUE; 
	 
	return joydata->num_buttons; 
#else
	return 0;
#endif
} 
 
/** 
* registerSwitchDownListener 
* 
* Registers a switch down listener. 
* 
* returns: Always zero. 
**/ 
gint registerSwitchDownListener (gint switch_num, void* callback) { 
#ifdef __linux__
	joydata->callback_down[switch_num] = callback; 
#endif
	return 0; 
} 
 
/** 
* registerSwitchUpListener 
* 
* Registers a switch up listener. 
* 
* returns: Always zero. 
**/ 
gint registerSwitchUpListener (gint switch_num, void* callback) { 
#ifdef __linux__
	joydata->callback_up[switch_num] = callback; 
#endif
	return 0; 
} 
 
/** 
* deregisterSwitchDownListener 
* 
* Removes a switch down listener. 
* 
* returns: Always zero. 
**/ 
gint deregisterSwitchDownListener (gint switch_num)  { 
#ifdef __linux__
	joydata->callback_down[switch_num] = NULL; 
#endif
	return 0; 
} 

/** 
* deregisterSwitchUpListener 
* 
* Removes a switch up listener. 
* 
* returns: Always zero. 
**/ 
gint deregisterSwitchUpListener (gint switch_num) { 
#ifdef __linux__
	joydata->callback_up[switch_num] = NULL; 
#endif
	return 0; 
} 

/** 
* closeSwitchApi 
* 
* Closes the switch API. This must be called at the end of the program. 
* 
* returns: Always zero. 
**/ 
gint closeSwitchApi() 
{ 
#ifdef __linux__
	if (joydata->gdktag) 
	{  
		gdk_input_remove(joydata->gdktag); 
	} 
	if (joydata->joystickfd != -1)  
	{ 
		close(joydata->joystickfd); 
	} 
	if (joydata->device_name)  
	{ 
		g_free(joydata->device_name); 
	} 
	 
	g_free(joydata); 
#endif	 
	return 0; 
} 
 
/** 
* joystick_event 
* 
*  
* 
* returns: Always zero. 
**/ 
#ifdef __linux__
static void joystick_event(gpointer data, gint fdsource, GdkInputCondition cond) 
{ 
	struct js_event js; 
	int readrc; 
	JoySwitchData * joydata; 
 
	joydata = data; 
	if (joydata == 0) 
	{ 
		gok_log_x ("Warning: Improper callback registration, %u, in joystick_event\n", __LINE__); 
		return; 
	} 
 
	readrc = read(fdsource, &js, sizeof(struct js_event)); 
	if (readrc == sizeof(struct js_event)) 
	{ 
		if ((js.type&3) == JS_EVENT_BUTTON) 
		{ 
			joydata->buttons[js.number] = js.value; 
			 
			/*check the current state  */
			if (js.value == 1) 
			{ 
				if (joydata->callback_down[js.number]) 
				{ 
					joydata->callback_down[js.number](js.type, js.number, joydata->callback_data); 
				} 
			} 
			else 	/* the button is up */ 
			{ 	 
				if (joydata->callback_up[js.number]) 
				{ 
					joydata->callback_up[js.number](js.type, js.number, joydata->callback_data); 
				} 
			}	 
		}		 
	} 
	else 
	{  
		gok_log_x ("source(%d), %d bytes read\n", fdsource, readrc); 
	} 
} 
#endif
 
