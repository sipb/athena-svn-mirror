/* srmag.h
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

#ifndef _SRMAG_H_
#define _SRMAG_H_

#include <glib.h>
#include "srmain.h"  
gboolean src_magnifier_init      ();
void src_magnifier_create    (); 
void src_magnifier_terminate (); 
void src_magnifier_send      (gchar *magoutput);

void src_magnifier_config_get_defaults (); 
void src_magnifier_config_get_settings (); 

gboolean src_magnifier_start_panning (SRObject *obj);
gboolean src_magnifier_stop_panning ();


gboolean src_magnifier_set_cursor_state  	 (gboolean cursor_state);
gboolean src_magnifier_set_cursor_name   	 (gchar *cursor_name);
gboolean src_magnifier_set_cursor_size   	 (gint cursor_size);
gboolean src_magnifier_set_cursor_scale  	 (gboolean cursor_scale);
gboolean src_magnifier_set_cursor_color  	 (gint32 cursor_color);

gboolean src_magnifier_set_crosswire_state 	 (gboolean crosswire_state);
gboolean src_magnifier_set_crosswire_clip   	 (gboolean crosswire_clip);
gboolean src_magnifier_set_crosswire_size   	 (gint crosswire_size);
gboolean src_magnifier_set_crosswire_color  	 (gint32 crosswire_color);
    
gboolean src_magnifier_set_zp_left    		 (gint zp_left);
gboolean src_magnifier_set_zp_top     		 (gint zp_top);
gboolean src_magnifier_set_zp_width   		 (gint zp_width);
gboolean src_magnifier_set_zp_height           	 (gint zp_height);

gboolean src_magnifier_set_target    		 (gchar *target);
gboolean src_magnifier_set_source    		 (gchar *source);

gboolean src_magnifier_set_zoom_factor_x    	 (gdouble zoom_factor_x);
gboolean src_magnifier_set_zoom_factor_y    	 (gdouble zoom_factor_y);
gboolean src_magnifier_set_zoom_factor_lock 	 (gboolean zoom_factor_lock);

gboolean src_magnifier_set_invert_on_off    	 (gboolean invert);
gboolean src_magnifier_set_smoothing        	 (gchar *smoothing);

gboolean src_magnifier_set_mouse_tracking_mode   (gchar *mouse_tracking);

gboolean src_magnifier_set_panning_on_off    	 (gboolean _panning);

gboolean src_magnifier_set_alignment_x       	 (gchar *set_alignment_x);
gboolean src_magnifier_set_alignment_y       	 (gchar *set_alignment_y);

gboolean src_magnifier_set_split_screen_horizontal (gboolean horiz_split);


gboolean src_magnifier_set_zoom_factor_lock_on_off 	 ();
gboolean src_magnifier_increase_y_scale			 ();
gboolean src_magnifier_increase_x_scale			 ();
gboolean src_magnifier_decrease_y_scale			 ();
gboolean src_magnifier_decrease_x_scale			 ();
gboolean src_magnifier_adjust_cursor_size		 (gint offset);
gboolean src_magnifier_set_cursor_magnification_on_off   ();
gboolean src_magnifier_adjust_crosswire_size		 (gint offset);
gboolean src_magnifier_set_crosswire_on_off		 ();
gboolean src_magnifier_set_crosswire_clip_on_off	 ();
gboolean src_magnifier_set_cursor_state_on_off		 ();
gboolean src_magnifier_set_panning_state		 ();
gboolean src_magnifier_set_invert_state			 ();



#endif /* _SRMAG_H_ */
