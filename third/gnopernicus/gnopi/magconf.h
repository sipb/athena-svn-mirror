/* magconf.h
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

#ifndef __MAGNIFIER__
#define __MAGNIFIER__

#include <glib.h>
#include "libsrconf.h"

#define MAGNIFIER_SCHEMA1		"/schema1/generic_zoomer/"
#define MAGNIFIER_CROSSWIRE_COLOR_INVERT "crosswire_color_invert"

#define MIN_X_SCREEN_POS 0
#define MAX_X_SCREEN_POS 1024

#define MIN_Y_SCREEN_POS 0
#define MAX_Y_SCREEN_POS 768

#define MIN_ZP_WIDTH_VAL 10
#define MIN_ZP_HEIGHT_VAL 10

#define MAX_ZOOM_FACTOR	16
#define MIN_ZOOM_FACTOR  1

/**
 *Magnifier Setting Structure
**/    


typedef struct
    {
	gint		left  ;
	gint		top   ;
	gint		width ;
	gint		height;
}MagRectangle;



typedef struct
    {
	gchar		*id;
	gchar 		*source;
	gchar 		*target;
	
	MagRectangle	zp;
	
	gboolean	visible;
	gboolean	invert;
	gboolean	panning;
	
	gdouble		zoomfactorx;
	gdouble		zoomfactory;
	gboolean 	zoomfactor_lock;
	
	gchar		*smoothing;
	gboolean	contrast;
	
	gint		border_width;
	gulong		border_color;
	
	gboolean	cursor_state;
	gint		cursor_size;
	gchar		*cursor_name;
	gboolean 	cursor_scale;
	gulong 		cursor_color;

	gboolean	crosswire;	
	gboolean	crosswire_clip;
	gint		crosswire_size;
	gulong		crosswire_color;

	gchar		*alignment_x;
	gchar		*alignment_y;
	
	gchar 		*tracking;
	gchar		*mouse_tracking;
    } Magnifier;



/**
 * Initialize magnifier configuration listener		
**/
gboolean 	magconf_gconf_client_init 		();

/**
 * Magnifier settings init					
**/
Magnifier* 	magconf_setting_init 		(const gchar *id,gboolean set_struct);

/**
 * Create a new Magnifier structure				
**/
Magnifier* 	magconf_setting_new 		();

/**
 * Load default Speech structure					
**/
void 		magconf_load_default_settings 	(Magnifier* magnifier);

/**
 * Free Magnifier structure					
**/
void 		magconf_setting_free 		(Magnifier* magnifier);

/**
 * Terminate Magnifier configure					
**/
void 		magconf_terminate		(Magnifier *magnifier_setting);

gboolean	magconf_copy_option_part 	(Magnifier *dest, 
						Magnifier *source);
Magnifier*	magconf_setting_clone 		(Magnifier *magnifier);
gboolean	magconf_zoomfactor_lock_set 	(gboolean zoomfactor_lock);
gboolean	magconf_save_item_in_schema 	(const gchar *schema,
						GConfValueType type,
						const gchar    *key,
						gpointer data);
gint		magconf_get_color_old 		(const gchar *schema,
						const gchar *key);

Magnifier*	magconf_load_zoomer_from_schema (const gchar *schema,
						gchar *zoomer);
gboolean	magconf_save_zoomer_in_schema	(const gchar *schema,
						const Magnifier *magnifier);
gboolean	magconf_set_setting_in_schema 	(const gchar *id,
						const gchar *schema,
						const gchar *key,
						GConfValueType data_type,
						gpointer data);
void		magconf_get_display_size 	(gint *size_x,
						gint *size_y);

#endif
