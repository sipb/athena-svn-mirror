/* mag_ctrl.h
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

#ifndef MAG_CTRL_H_
#define MAG_CTRL_H_

#include "magnifier/GNOME_Magnifier.h"
#include "mag.h"
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

GNOME_Magnifier_Magnifier get_magnifier (void);

/*MAGNIFIER*/
void magnifier_set_source_screen (GNOME_Magnifier_Magnifier 	magnifier, 
				  gchar 			*source_display_screen);
void magnifier_set_target_screen (GNOME_Magnifier_Magnifier 	magnifier, 
				  gchar *target_display_screen);

void magnifier_get_source (GNOME_Magnifier_Magnifier magnifier, MagRectangle *source_rect);
void magnifier_get_target (GNOME_Magnifier_Magnifier magnifier, MagRectangle *target_rect);
void magnifier_set_target (GNOME_Magnifier_Magnifier magnifier, MagRectangle *target_rect);

/*CURSORS*/
void magnifier_set_cursor (GNOME_Magnifier_Magnifier magnifier, 
			    gchar *cursor_name,
			   int    cursor_size,
			   float  cursor_zoom_factor);
void magnifier_set_cursor_color (GNOME_Magnifier_Magnifier	magnifier, 
				 guint32     			cursor_color);			   
/*CROSSWIRE*/
void magnifier_set_crosswire_size  (GNOME_Magnifier_Magnifier 	magnifier, 
				    int      			crosswire_size) ;
void magnifier_set_crosswire_color (GNOME_Magnifier_Magnifier 	magnifier, 
				    guint32    			crosswire_color);
void magnifier_set_crosswire_clip  (GNOME_Magnifier_Magnifier 	magnifier, 
				    gboolean 			crosswire_clip) ;

/*ZOOMER*/
void magnifier_get_viewport (GNOME_Magnifier_Magnifier magnifier,
			     int zoom_region,
			     MagRectangle *viewport_rect);
void magnifier_set_zoomer_properties (GNOME_Magnifier_Magnifier magnifier, int zoom_region,
			              MagZoomer *zoomer_data);
void magnifier_set_roi (GNOME_Magnifier_Magnifier magnifier, int zoom_region, 
			MagRectangle *roi);
void magnifier_set_magnification (GNOME_Magnifier_Magnifier magnifier, int   zoom_region, 
				  float mag_factor_x, 
				  float mag_factor_y);

void magnifier_set_invert (GNOME_Magnifier_Magnifier magnifier, int zoom_region, 
			   int invert);

void magnifier_set_is_managed (GNOME_Magnifier_Magnifier magnifier, int zoom_region, 
			       int is_managed);

void magnifier_set_smoothing_type (GNOME_Magnifier_Magnifier magnifier, int    zoom_region, 
			      gchar *smoothing_type);

void magnifier_set_contrast (GNOME_Magnifier_Magnifier magnifier, int   zoom_region, 
			     float contrast);

void magnifier_set_border (GNOME_Magnifier_Magnifier magnifier, int  zoom_region, 
			   int  border_size,
			   long border_color);

void magnifier_set_alignment (GNOME_Magnifier_Magnifier magnifier, int zoom_region, 
			      int alignment_x,
			      int alignment_y);

void magnifier_set_smoothing_type (GNOME_Magnifier_Magnifier magnifier, int   zoom_region, 
			           gchar *smoothing_type);

void magnifier_resize_region (GNOME_Magnifier_Magnifier magnifier, int zoom_region,
			      MagRectangle *viewport);
int  magnifier_create_region (GNOME_Magnifier_Magnifier magnifier, float zx, 
			      float zy, 
			      MagRectangle *roi,
			      MagRectangle *viewport);			      
void magnifier_clear_all_regions (GNOME_Magnifier_Magnifier magnifier);
void magnifier_exit(GNOME_Magnifier_Magnifier magnifier);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* MAG_CTRL_H_ */

