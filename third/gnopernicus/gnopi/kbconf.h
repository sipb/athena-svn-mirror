/* kbconf.h
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

#ifndef __KEYBOARD__
#define __KEYBOARD__ 0

#include <glib.h>


/**
 *Keyboard Setting Structure
**/    
typedef struct
{
    /* Mouse settings */
    gboolean take_mouse;
    gboolean simulate_click;
} Keyboard;



/**
 * Initialize keyboard configuration listener		
**/
gboolean 	kbconf_gconf_client_init 	(void);
void 		kbconf_terminate		(Keyboard 	*keyboard);
/**
 * Keyboard settings init					
**/
Keyboard* 	kbconf_setting_init 		(gboolean set_struct);
Keyboard* 	kbconf_setting_new 		(void);
void 		kbconf_setting_free 		(Keyboard	*keyboard);
/**
 * Load default Keyboard structure					
**/
void 		kbconf_load_default_settings	(Keyboard	*keyboard);


/**
 * Set Methods 
**/
void		kbconf_take_mouse_set 		(gboolean mouse_move);
void		kbconf_simulate_click_set 	(gboolean simulate);
void    	kbconf_setting_set 		(void);
/**
 * Get Methods 						
**/
gboolean	kbconf_take_mouse_get		(void);
gboolean	kbconf_simulate_click_get	(void);

#endif
