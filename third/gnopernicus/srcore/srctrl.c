/* srctrl.c
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

#include "config.h"
#include "srctrl.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <SRObject.h>
#include <SRLow.h>
#include <gtk/gtk.h>
#include "srmag.h"
#include "srspc.h"
#include "srbrl.h"

#include "srmain.h" 
#include "srpres.h"
#include "screen-review.h"
#include "srintl.h"
#include <gdk/gdkx.h>

#undef SRCTRL_DEBUG

#define ZOOM_FACTOR_INC (0.5)

extern gboolean src_use_magnifier;
extern gboolean src_use_speech;
extern gboolean src_use_braille;
extern gboolean src_use_braille_monitor;
extern gboolean src_enable_format_string;

gchar    *scroll = NULL;

gboolean  src_button_left_pressed   = FALSE;
gboolean  src_button_middle_pressed = FALSE;
gboolean  src_button_right_pressed  = FALSE;

static gboolean src_xevie_present = FALSE;

/*___________________________< MAGNIFIER CONTROL>_____________________________*/
static gboolean
src_mag_lock_xy_scale ()
{
    if (src_use_magnifier) 
    {
	if (login_time)
	{
	    src_magnifier_set_zoom_factor_lock_on_off();
	}
	else
	{
	    gboolean lock = TRUE;
	    gboolean default_lock = DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK;
	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_LOCK, 
					       CFGT_BOOL, 
					       &lock, 
					       (gpointer)&default_lock,
					       MAGNIFIER_CONFIG_PATH)
	    ) return FALSE;
	
	    lock  = !lock;
	    if (lock)
	    {
		gdouble default_val = DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY;
		gdouble zoom_factory;
		gdouble zoom_factorx;
		if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_Y, 
						   CFGT_FLOAT, 
						   &zoom_factory, 
						   (gpointer)&default_val,
						   MAGNIFIER_CONFIG_PATH)
		    ) return FALSE;
		if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_X, 
						   CFGT_FLOAT, 
						   &zoom_factorx, 
						   (gpointer)&default_val,
						   MAGNIFIER_CONFIG_PATH)
		    ) return FALSE;
		
		zoom_factorx =  zoom_factory = (zoom_factory + zoom_factorx) / 2;

		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
				      CFGT_FLOAT, 
				      &zoom_factory,
				      MAGNIFIER_CONFIG_PATH)
		    ) return FALSE;
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
				      CFGT_FLOAT, 
				      &zoom_factory,
				      MAGNIFIER_CONFIG_PATH)
		    ) return FALSE;
	    }
	    if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_LOCK,
	    		          CFGT_BOOL, 
			          &lock,
			          MAGNIFIER_CONFIG_PATH)
		) return FALSE;
	}
    }
    return TRUE;
}

static gboolean
src_mag_increase_y_scale ()
{
   if (src_use_magnifier)
    {
	if (login_time)
	{
	    src_magnifier_increase_y_scale ();
	}
	else
	{
	    gdouble default_val = DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY;
	    gdouble zoom_factory;
	    gboolean zoom_factor_lock;
	    gboolean zoom_factor_lock_def = DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK;

	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_Y, 
					       CFGT_FLOAT, 
					       &zoom_factory, 
					       (gpointer)&default_val,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_LOCK, 
					       CFGT_BOOL, 
					       &zoom_factor_lock, 
					       (gpointer)&zoom_factor_lock_def,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;

	    zoom_factory = ((++zoom_factory) > MAX_ZOOM_FACTOR_Y) ? MAX_ZOOM_FACTOR_Y : zoom_factory;
	
	    if (!zoom_factor_lock)
	    {
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
				      CFGT_FLOAT, 
				      &zoom_factory,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	    }
	    else
	    {
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
				      CFGT_FLOAT, 
				      &zoom_factory,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
				      CFGT_FLOAT, 
				      &zoom_factory,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	    }
	}
    }
    return TRUE;
}


static gboolean
src_mag_decrease_y_scale ()
{
   if (src_use_magnifier)
    {
	if (login_time)
	{
	    src_magnifier_decrease_y_scale ();
	}
	else
	{
	    gdouble default_val = DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY;
	    gdouble zoom_factory;
	    gboolean zoom_factor_lock;
	    gboolean zoom_factor_lock_def = DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK;

	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_Y, 
					       CFGT_FLOAT, 
					       &zoom_factory, 
					       (gpointer)&default_val,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_LOCK, 
					       CFGT_BOOL, 
					       &zoom_factor_lock, 
					       (gpointer)&zoom_factor_lock_def,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;

	    zoom_factory = ((--zoom_factory) < MIN_ZOOM_FACTOR_Y) ? MIN_ZOOM_FACTOR_Y : zoom_factory;

	    if (!zoom_factor_lock)
	    {
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
				      CFGT_FLOAT, 
				      &zoom_factory,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	    }
	    else
	    {
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
				      CFGT_FLOAT, 
				      &zoom_factory,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
				      CFGT_FLOAT, 
				      &zoom_factory,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	    }
	}
    }
    return TRUE;
}


static gboolean
src_mag_increase_x_scale ()
{
    
    if (src_use_magnifier)
    {
	if (login_time)
	{
	    src_magnifier_increase_x_scale ();
	}
	else
	{
	    gdouble default_val = DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY;
	    gdouble zoom_factorx;
	    gboolean zoom_factor_lock;
	    gboolean zoom_factor_lock_def = DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK;

	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_X, 
					       CFGT_FLOAT, 
					       &zoom_factorx, 
					       (gpointer)&default_val,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_LOCK, 
					       CFGT_BOOL, 
					       &zoom_factor_lock, 
					       (gpointer)&zoom_factor_lock_def,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;

	    zoom_factorx = ((++zoom_factorx) > MAX_ZOOM_FACTOR_X) ? MAX_ZOOM_FACTOR_X : zoom_factorx;

	    if (!zoom_factor_lock)
	    {
	        if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
				      CFGT_FLOAT, 
				      &zoom_factorx,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	    }
    	    else
	    {
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
				      CFGT_FLOAT, 
				      &zoom_factorx,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
				      CFGT_FLOAT, 
				      &zoom_factorx,
				      MAGNIFIER_CONFIG_PATH)
	    	   ) return FALSE;
	    }
	}
    }
    return TRUE;
}


static gboolean
src_mag_decrease_x_scale ()
{
    if (src_use_magnifier)
    {
	if (login_time)
	{
	    src_magnifier_decrease_x_scale ();
	}
        else
	{	
            gdouble default_val = DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY;
	    gdouble zoom_factorx;
	    gboolean zoom_factor_lock;
	    gboolean zoom_factor_lock_def = DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK;

	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_X, 
					       CFGT_FLOAT, 
					       &zoom_factorx, 
					       (gpointer)&default_val,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	    if (!srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_LOCK, 
					       CFGT_BOOL, 
					       &zoom_factor_lock, 
					       (gpointer)&zoom_factor_lock_def,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;

	    zoom_factorx = ((--zoom_factorx) < MIN_ZOOM_FACTOR_X) ? MIN_ZOOM_FACTOR_X : zoom_factorx;

	    if (!zoom_factor_lock)
	    {
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
				      CFGT_FLOAT, 
				      &zoom_factorx,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	    }
	    else
	    {
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
				      CFGT_FLOAT, 
				      &zoom_factorx,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
		if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
				      CFGT_FLOAT, 
				      &zoom_factorx,
				      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	    }
	}
    }
    return TRUE;
}


static gboolean
src_mag_mouse_tracking_toggle ()
{
    static int index = -1;
    static gchar *mouse_tracking[] = 
	{
	    "mouse-push",
	    "mouse-centered",
	    "mouse-proportional",
	    "none"
	};
    index = (index + 1) % G_N_ELEMENTS (mouse_tracking);
    if (src_use_magnifier) 
    {
	if (login_time)
	{
	    src_magnifier_set_mouse_tracking_mode (mouse_tracking[index]);
	}
	else
	{
    	    if (!srconf_set_data (MAGNIFIER_MOUSE_TRACKING,
			          CFGT_STRING, 
			          mouse_tracking[index],
			          MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	}
    }
    return TRUE;
}


static gboolean
src_mag_cursor_toggle ()
{
    static int index = -1;
    static gchar *cursors[] = 
	{
/*	    "busy",
	    "cross",
	    "hand",
	    "help",
	    "ibeam",
	    "move",
	    "nesw",
	    "normal",
	    "no",
	    "ns",
	    "up",
	    "we",
	    "crosswire",
*/	    "crosshair",
	    "default",
	    "none"
	};
    gboolean cursor;
    gboolean default_cursor = DEFAULT_MAGNIFIER_CURSOR;
    
    index = (index + 1) % G_N_ELEMENTS (cursors);
    if (login_time)
    {
	src_magnifier_set_cursor_name (cursors[index]);
    }
    else
    {
	if (!srconf_get_data_with_default (MAGNIFIER_CURSOR, 
					   CFGT_BOOL, 
					   &cursor, 
					   (gpointer)&default_cursor,
					   MAGNIFIER_CONFIG_PATH)
	   ) return FALSE;

	if (src_use_magnifier && cursor) 
	{
	   if (!srconf_set_data (MAGNIFIER_CURSOR_NAME,
			         CFGT_STRING, 
			         cursors[index],
			         MAGNIFIER_CONFIG_PATH)
	      ) return FALSE;
	
	}
    }
    return TRUE;
}

static gboolean
src_mag_cursor_adjust_size (gint offset)
{
    if (src_use_magnifier)
    {
	gboolean cursor;
	gboolean default_cursor = DEFAULT_MAGNIFIER_CURSOR;
	if (!srconf_get_data_with_default (MAGNIFIER_CURSOR, 
			    		   CFGT_BOOL, 
				           &cursor, 
				           (gpointer)&default_cursor,
				           MAGNIFIER_CONFIG_PATH)
           ) return FALSE;

        if (cursor) 
        {
    	    gint default_val = DEFAULT_MAGNIFIER_CURSOR_SIZE;
	    gint cursor_size;
	
	if (!srconf_get_data_with_default (MAGNIFIER_CURSOR_SIZE, 
				           CFGT_INT, 
				           &cursor_size, 
				           (gpointer)&default_val,
				    	   MAGNIFIER_CONFIG_PATH)
	   ) return FALSE;
		
	cursor_size = CLAMP (cursor_size + offset,
			     MIN_CROSSWIRE_SIZE,
			     MAX_CROSSWIRE_SIZE);
	if (!srconf_set_data (MAGNIFIER_CURSOR_SIZE, 
		    	      CFGT_INT, 
		    	      &cursor_size, 
		    	      MAGNIFIER_CONFIG_PATH)
	   ) return FALSE;
	}	   	
    }
    return TRUE;
}

static gboolean
src_mag_cursor_decrease_size ()
{
    if (login_time)
        return src_magnifier_adjust_cursor_size (-4);
    else
    	return src_mag_cursor_adjust_size (-4);
}

static gboolean
src_mag_cursor_increase_size ()
{
    if (login_time)
    	return src_magnifier_adjust_cursor_size (+4);
    else
    	return src_mag_cursor_adjust_size (+4);
}

static gboolean
src_mag_cursor_mag_on_off ()
{
    gboolean cursor;
    gboolean default_cursor = DEFAULT_MAGNIFIER_CURSOR;

    if (src_use_magnifier)
    {
	if (login_time)
	{
	    src_magnifier_set_cursor_magnification_on_off ();
	}
	else
	{
	    if (!srconf_get_data_with_default (MAGNIFIER_CURSOR, 
					       CFGT_BOOL, 
					       &cursor, 
					       (gpointer)&default_cursor,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;

            if (cursor)
	    {
		gboolean cursor_mag;
		gboolean default_cursor_mag = DEFAULT_MAGNIFIER_CURSOR_SCALE;

		if (!srconf_get_data_with_default (MAGNIFIER_CURSOR_SCALE, 
					           CFGT_BOOL, 
					           &cursor_mag, 
					           (gpointer)&default_cursor_mag,
						   MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	        cursor_mag = !cursor_mag;
	
		if (!srconf_set_data (MAGNIFIER_CURSOR_SCALE, 
			    	      CFGT_BOOL, 
			              &cursor_mag, 
			    	      MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	    }
	 }
    }
    return TRUE; 
}

static gboolean
src_mag_crosswire_adjust_size (guint offset)
{
    gboolean cursor, crosswire;
    gboolean default_cursor = DEFAULT_MAGNIFIER_CURSOR;
    gboolean default_crosswire = DEFAULT_MAGNIFIER_CROSSWIRE;
    if (!srconf_get_data_with_default (MAGNIFIER_CURSOR, 
				       CFGT_BOOL, 
				       &cursor, 
				       (gpointer)&default_cursor,
				       MAGNIFIER_CONFIG_PATH)
	) return FALSE;
    if (!srconf_get_data_with_default (MAGNIFIER_CROSSWIRE, 
				       CFGT_BOOL, 
				       &crosswire, 
				       (gpointer)&default_crosswire,
				       MAGNIFIER_CONFIG_PATH)
	) return FALSE;

    if (src_use_magnifier && cursor && crosswire)
    {
	gint default_val = DEFAULT_MAGNIFIER_CROSSWIRE_SIZE;
	gint crosswire_size;

	if (!srconf_get_data_with_default (MAGNIFIER_CROSSWIRE_SIZE, 
					   CFGT_INT, 
					   &crosswire_size, 
					   (gpointer)&default_val,
					   MAGNIFIER_CONFIG_PATH)
	    ) return FALSE;

	crosswire_size = CLAMP (crosswire_size + offset,
				MIN_CROSSWIRE_SIZE,
				MAX_CROSSWIRE_SIZE);

	if (!srconf_set_data (MAGNIFIER_CROSSWIRE_SIZE, 
			      CFGT_INT, 
			      &crosswire_size, 
			      MAGNIFIER_CONFIG_PATH)
	    ) return FALSE;
    }
    return TRUE;
}

static gboolean
src_mag_crosswire_decrease_size ()
{
    if (login_time)
    	return src_magnifier_adjust_crosswire_size (-1);
    else
    	return src_mag_crosswire_adjust_size (-1);
    
}

static gboolean
src_mag_crosswire_increase_size ()
{
    if (login_time)
    	return src_magnifier_adjust_crosswire_size (+1);
    else
        return src_mag_crosswire_adjust_size (+1);
}

static gboolean
src_mag_crosswire_on_off ()
{
    gboolean cursor;
    gboolean default_cursor = DEFAULT_MAGNIFIER_CURSOR;
    
    if (src_use_magnifier)
    {
	if (login_time)
	{
	    src_magnifier_set_crosswire_on_off ();
	}
	else
	{
	    if (!srconf_get_data_with_default (MAGNIFIER_CURSOR, 
					       CFGT_BOOL, 
					       &cursor, 
					       (gpointer)&default_cursor,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	    if (cursor)
	    {
		gboolean crosswire;
		gboolean default_crosswire = DEFAULT_MAGNIFIER_CROSSWIRE;
		if (!srconf_get_data_with_default (MAGNIFIER_CROSSWIRE,
						   CFGT_BOOL,
						   &crosswire,
						   (gpointer)&default_crosswire ,
						   MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
	
	    crosswire = !crosswire;

	    if (!srconf_set_data (MAGNIFIER_CROSSWIRE,
			    	  CFGT_BOOL,
			          &crosswire,
			          MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	    }
	}
    }
    return TRUE;
}

static gboolean
src_mag_crosswire_clip_on_off ()
{
    gboolean cursor, crosswire;
    gboolean default_cursor = DEFAULT_MAGNIFIER_CURSOR;
    gboolean default_crosswire = DEFAULT_MAGNIFIER_CROSSWIRE;
    
    if (src_use_magnifier)
    {
	if (login_time)
	{
	    src_magnifier_set_crosswire_clip_on_off ();
	}
	else
	{
	    if (!srconf_get_data_with_default (MAGNIFIER_CURSOR, 
					       CFGT_BOOL, 
					       &cursor, 
					       (gpointer)&default_cursor,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	    if (!srconf_get_data_with_default (MAGNIFIER_CROSSWIRE, 
					       CFGT_BOOL, 
					       &crosswire, 
					       (gpointer)&default_crosswire,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;

	    if (cursor && crosswire)
	    {
		gboolean crosswire_clip;
		gboolean default_crosswire_clip = DEFAULT_MAGNIFIER_CROSSWIRE_CLIP;
		if (!srconf_get_data_with_default (MAGNIFIER_CROSSWIRE_CLIP,
						   CFGT_BOOL,
						   &crosswire_clip,
						   (gpointer)&default_crosswire_clip ,
						   MAGNIFIER_CONFIG_PATH)
		   ) return FALSE;
		crosswire_clip = !crosswire_clip;
	
		if (!srconf_set_data (MAGNIFIER_CROSSWIRE_CLIP,
			    	      CFGT_BOOL,
			    	      &crosswire_clip,
				      MAGNIFIER_CONFIG_PATH)
		   )
		return FALSE;
	    }
	}
    }
    return TRUE;
}


static gboolean
src_mag_cursor_on_off ()
{
    if (src_use_magnifier) 
    {
	gboolean cursor;
	gboolean default_cursor = DEFAULT_MAGNIFIER_CURSOR;
	
	if (login_time)
	{
	    src_magnifier_set_cursor_state_on_off ();
	}
	else
	{
	    if (!srconf_get_data_with_default (MAGNIFIER_CURSOR, 
					       CFGT_BOOL, 
					       &cursor, 
					       (gpointer)&default_cursor,
					       MAGNIFIER_CONFIG_PATH)
		) return FALSE;

	    cursor = !cursor;
	
	    if (!srconf_set_data (MAGNIFIER_CURSOR,
			          CFGT_BOOL, 
			          &cursor,
			          MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	}
    }
    return TRUE;
}


/*automatic panning ("document reading")- timer-based*/
static gboolean
src_mag_panning_on_off ()
{
    if (src_use_magnifier) 
    {
	if (login_time)
	{
	    src_magnifier_set_panning_state ();
	}
	else
	{
	    gboolean panning;
	    gboolean default_panning = DEFAULT_MAGNIFIER_PANNING;
	    if (!srconf_get_data_with_default (MAGNIFIER_PANNING, 
					       CFGT_BOOL, 
					       &panning, 
					       (gpointer)&default_panning, 
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;

	    panning = !panning;

	    if (!srconf_set_data (MAGNIFIER_PANNING,
			          CFGT_BOOL, 
			          &panning, 
			          MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	}
    }
    return TRUE;
}


static gboolean
src_mag_invert_on_off ()
{
    if (src_use_magnifier) 
    {
	if (login_time)
	{
	    src_magnifier_set_invert_state ();
	}
	else
	{
	    gboolean invert = TRUE;
	    gboolean default_invert = DEFAULT_MAGNIFIER_INVERT;
	    if (!srconf_get_data_with_default (MAGNIFIER_INVERT, 
					       CFGT_BOOL, 
					       &invert, 
					       (gpointer)&default_invert,
					       MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;

    	    invert  = !invert;
	
	    if (!srconf_set_data (MAGNIFIER_INVERT,
			          CFGT_BOOL, 
			          &invert,
			          MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	}
    }
    return TRUE;
}

static gboolean
src_mag_default_all ()
{
    if (src_use_magnifier) 
    {
	src_magnifier_config_get_defaults ();
    }
    return TRUE;
}

static gboolean
src_mag_smoothing_toggle ()
{
    static int index = -1;
    static gchar *smoothing[] = 
	{
	    "bilinear",
	    "none",
	};
    index = (index + 1) % G_N_ELEMENTS (smoothing);
    
    
    if (src_use_magnifier) 
    {
	if (login_time)
	{
	    src_magnifier_set_smoothing (smoothing[index]);
	}
	else
	{
	    if (!srconf_set_data (MAGNIFIER_SMOOTHING,
			          CFGT_STRING, 
			          smoothing[index],
			          MAGNIFIER_CONFIG_PATH)
	       ) return FALSE;
	}
    }
    return TRUE;    
}
/*___________________________</MAGNIFIER CONTROL>_____________________________*/

static gboolean 
src_speech_increase_pitch ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_pitch (SRC_MODIF_INCREASE);
}

static gboolean 
src_speech_decrease_pitch ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_pitch (SRC_MODIF_DECREASE);
}

static gboolean 
src_speech_default_pitch ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_pitch (SRC_MODIF_DEFAULT);
}

static gboolean 
src_speech_increase_rate ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_rate (SRC_MODIF_INCREASE);
}

static gboolean 
src_speech_decrease_rate ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_rate (SRC_MODIF_DECREASE);
}

static gboolean 
src_speech_default_rate ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_rate (SRC_MODIF_DEFAULT);
}

static gboolean 
src_speech_increase_volume ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_volume (SRC_MODIF_INCREASE);
}

static gboolean 
src_speech_decrease_volume ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_volume (SRC_MODIF_DECREASE);
}

static gboolean 
src_speech_default_volume ()
{
    if (!src_use_speech)
	return FALSE;

    return src_speech_modify_volume (SRC_MODIF_DEFAULT);
}

static gboolean 
src_speech_default_all ()
{
    if (!src_use_speech)
	return FALSE;

    src_cmd_queue_add ("default pitch");
    src_cmd_queue_add ("default rate");
    src_cmd_queue_add ("default volume");
    src_cmd_queue_process ();

    return TRUE;
}

typedef enum _SRCTrackingMode
{
    SRC_MODE_FOCUS_TRACKING,
    SRC_MODE_FLAT_REVIEW
}SRCTrackingMode;

static SRCTrackingMode src_tracking_mode = SRC_MODE_FOCUS_TRACKING;

extern SRObject *src_focused_sro;
extern SRObject *src_crt_sro;

extern gboolean src_mouse_take;
extern gboolean src_mouse_click;

static gint src_nav_mode = SR_NAV_MODE_WINDOW;

static gboolean
src_nav_logic (SRNavigationDir dir, const char *missing_node)
{
    SRObject *node;
    gboolean rv = FALSE;
    
    if (!src_crt_sro)
	return FALSE;
    
    sro_get_sro (src_crt_sro, dir, &node, src_nav_mode);
    if (node)
    {
	sro_release_reference (src_crt_sro);
	src_crt_sro = node;
	src_cmd_queue_add ("present current object");
	if (src_mouse_take)
	{
	    src_cmd_queue_add ("mouse goto current");
	    if (src_mouse_click)
		src_cmd_queue_add ("mouse left click");
	}
	src_cmd_queue_process ();
	rv = TRUE;
    }
    else
    {
	if (src_use_speech)
            src_say_message (missing_node);
    }

    return rv;
}

static gboolean src_flat_review_next_line ();
static gboolean src_flat_review_prev_line ();

static gint crt_line = 0;
static gboolean visited = FALSE;

static gboolean
src_nav_parent_flat ()
{
    gboolean rv = FALSE;
    
    src_braille_set_offset (0);
    rv = src_flat_review_prev_line();

    return rv;
}


static gboolean
src_nav_parent ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_parent_flat ();
    else
        rv = src_nav_logic (SR_NAV_PARENT, _("no parent"));

    return rv;
}

static gboolean
src_nav_child_flat ()
{
    gboolean rv = FALSE;
    
    src_braille_set_offset (0);
    rv = src_flat_review_next_line();
    
    return rv;
}

static gboolean
src_nav_child ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_child_flat ();
    else
	rv = src_nav_logic (SR_NAV_CHILD, _("no children"));

    return rv;
}

static gboolean src_flat_review_line (gint index);

static gboolean
src_nav_next_flat ()
{
    gboolean rv = FALSE;
    gboolean old_value = src_use_speech;
    gint offset;
    
    offset = src_braille_get_offset ();
    offset ++;
    src_braille_set_offset (offset);
    
    src_use_speech = FALSE;
    src_flat_review_line (crt_line);
    src_use_speech = old_value;

    return rv;
}

static gboolean
src_nav_next ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_next_flat ();
    else
	rv = src_nav_logic (SR_NAV_NEXT, _("no next"));

    return rv;
}


typedef enum
{
    SR_FIND_TEXT,
    SR_FIND_ATTRIBUTE,
    SR_FIND_GRAPHICS,
    SR_FIND_INVALID
}SRFindType;

static gchar*
src_get_text_and_type_from_chunk (const gchar *text, 
				  SRFindType *type, 
				  SRNavigationMode *scope
				  )
{
    gchar **tokens;
    
    tokens = g_strsplit (text, ":", 3);
    
    if (tokens[0] && tokens[1])
    {
	gchar *retval = NULL;
	
	if (!strcmp (tokens[0], "APPLICATION"))
	{
	    *scope = SR_NAV_MODE_APPLICATION;
	}
	else
	if (!strcmp (tokens[0], "DESKTOP"))
	{
	    *scope = SR_NAV_MODE_DESKTOP;
	}
	else
	    *scope = SR_NAV_MODE_WINDOW;

	
	if (!strcmp (tokens[1], "TEXT"))
	{
	    *type = SR_FIND_TEXT;
	}
	else
	if (!strcmp (tokens[1], "ATTRIBUTE"))
	{
	    *type = SR_FIND_ATTRIBUTE;
	}
	else
	if (!strcmp (tokens[1], "GRAPHICS"))
	{
	    *type = SR_FIND_GRAPHICS;
	}
	else
	    *type = SR_FIND_INVALID;
	    
	retval = g_strdup (tokens [2]);
	g_strfreev (tokens);
	return retval;
    }
    else
    {
	*type = SR_FIND_INVALID;
	return NULL;
    }
    
}


static gboolean
src_find_next_flat ()
{
    gboolean 	rv = FALSE;
    
    return rv;
}


static gboolean
src_find_next_logic ()
{
    SRObject 	*rv_obj;
    SRNavigationMode scope;
    SRFindType	     type;
    gboolean 	rv = FALSE;
    gboolean    src_found_object = FALSE;
    gchar 	*find_txt = NULL;
    gchar 	*text;

    if (!src_crt_sro)
	return FALSE;
	
    if (!srconf_get_data_with_default ( SRCORE_FIND_TEXT, 
					CFGT_STRING, 
					&find_txt, 
					NULL,
					SRCORE_PATH)
	) return FALSE;

    if (!find_txt) return FALSE;
    	
    text = src_get_text_and_type_from_chunk (find_txt, &type, &scope);
    
    g_free (find_txt);

    switch (type)
    {
	case SR_FIND_TEXT:
	{
	    if (sro_get_next_text (src_crt_sro, text, &rv_obj, scope))
		    src_found_object = TRUE;
	    else
	    {
		if (src_use_speech)
		    src_say_message (_("Cannot find next text!"));
	    }
	    break;
	}
	case SR_FIND_GRAPHICS:
	{
	    if (sro_get_next_image (src_crt_sro, &rv_obj, scope))
		    src_found_object = TRUE;
	    else
	    {
		if (src_use_speech)
		    src_say_message (_("Cannot find next image!"));
	    }
	    break;
	}
	case SR_FIND_ATTRIBUTE:
	{
	    if (sro_get_next_attributes (src_crt_sro, text, &rv_obj, scope))
		    src_found_object = TRUE;
	    else
	    {
		if (src_use_speech)
		    src_say_message (_("Cannot find next attributes!"));
	    }
	    break;
	}
	case SR_FIND_INVALID:
	{
	    src_found_object = FALSE;
	    break;
	}
    }
    
    if (src_found_object)
    {
	sro_release_reference (src_crt_sro);
	src_crt_sro = rv_obj;
	src_cmd_queue_add ("present current object");
#if 0
	if (src_mouse_take)
	{
	    src_cmd_queue_add ("mouse goto current");
	    if (src_mouse_click)
	        src_cmd_queue_add ("mouse left click");
	}
#endif
	src_cmd_queue_process ();
	rv = TRUE;
    }

    g_free (text);
    
    return rv;
}


static gboolean
src_find_next ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_find_next_flat ();
    else
	rv = src_find_next_logic ();

    return rv;
}


static gboolean
src_find_set ()
{
    srconf_unset_key (SRCORE_UI_COMMAND, SRCORE_PATH);
    if (!srconf_set_data (SRCORE_UI_COMMAND,
			  CFGT_STRING, 
			  "find_set",
			  SRCORE_PATH)
        )
    {
	if (!login_time)
	     return FALSE;
    }
    return TRUE;
}

static gboolean
src_nav_previous_flat ()
{
    gboolean rv = FALSE;
    gboolean old_value = src_use_speech;
    gint offset;
    
    offset = src_braille_get_offset ();
    src_braille_set_offset (MAX (0, offset - 1));
    
    src_use_speech = FALSE;
    src_flat_review_line (crt_line);
    src_use_speech = old_value;
    
    return rv;
}    

static gboolean
src_nav_previous ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_previous_flat ();
    else
	rv = src_nav_logic (SR_NAV_PREV, _("no previous"));

    return rv;
}

static gboolean
src_nav_caret_flat ()
{
    gboolean rv = FALSE;

    return rv;
}

SRLong src_text_index;
static gboolean
src_caret_attributes ()
{
    SRLong index;
    gboolean rv = FALSE;

    if (!src_crt_sro || !sro_is_text (src_crt_sro, SR_INDEX_CONTAINER))
	return FALSE;
    
    if (sro_text_get_caret_offset (src_crt_sro, &index, SR_INDEX_CONTAINER))
	src_text_index = index;
    else
	src_text_index = 0;

    src_cmd_queue_add ("text attributes");
    src_cmd_queue_process ();

    return rv;
}

static gboolean
src_watch_current ()
{
    gboolean rv = FALSE;

    if (src_crt_sro)
	srl_set_watch_for_object (src_crt_sro);
    if (src_use_speech)
    {
        gchar *message;
	message = src_crt_sro ? _("added watch for current object") : 
			_("cannot add watch for current object");
	src_say_message (message);
    }
    return rv;
}

static gboolean
src_unwatch_all ()
{
    srl_unwatch_all_objects ();
    if (src_use_speech)
    {
	src_say_message (_("unwatch all objects"));
    }
    return TRUE;
}

static gboolean
src_nav_caret ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_caret_flat ();
    else
	rv = src_nav_logic (SR_NAV_CARET, _("no caret"));

    return rv;
}

static gboolean
src_nav_first_flat ()
{
    gboolean rv = FALSE;

    return rv;
}

static gboolean
src_nav_first ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_first_flat ();
    else
	rv = src_nav_logic (SR_NAV_FIRST, _("already on first"));

    return rv;
}

static gboolean
src_nav_last_flat ()
{
    gboolean rv = FALSE;
    
    return rv;
}

static gboolean
src_nav_last ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_last_flat ();
    else
	rv = src_nav_logic (SR_NAV_LAST, _("already on last"));

    return rv;
}

static gboolean
src_nav_change_mode ()
{
    gchar *message = NULL;
    switch (src_nav_mode)
    {
	case SR_NAV_MODE_WINDOW:
	    src_nav_mode = SR_NAV_MODE_APPLICATION;
	    message = _("now navigating in application");
	    break;
	case SR_NAV_MODE_APPLICATION:
	    src_nav_mode = SR_NAV_MODE_DESKTOP;
	    message = _("now navigating in desktop");
	    break;
	default:
	    src_nav_mode = SR_NAV_MODE_WINDOW;
	    message = _("now navigating in window");
	    break;    
    }
    if (src_use_speech && message)
    {
	src_say_message (message); 
    }

    return TRUE;
}



static gchar *src_last_cmd = NULL;
static gchar *src_before_last_cmd = NULL;

extern gint src_speech_mode;
static gboolean src_repeat_cnt = 1;

static gint flat_review_x = 0, 
	    flat_review_y = 0;
static gint n_lines = 0;


static gboolean
src_repeat_last ()
{
    gboolean rv = FALSE;
    gint mode, last_mode;
    
    if (!src_use_speech)
    {    
	src_cmd_queue_add (src_before_last_cmd);
	src_cmd_queue_process ();
	return FALSE;
    }	

    if (src_repeat_cnt == 1)
	mode = SRC_SPEECH_SPELL_NORMAL;
    else if (src_repeat_cnt == 2)
	mode = SRC_SPEECH_SPELL_CHAR_BY_CHAR;
    else 
	mode = SRC_SPEECH_SPELL_MILITARY;

    last_mode = src_speech_get_spelling_mode ();
    src_speech_set_spelling_mode (mode);    

    if (src_last_cmd)
    {
	src_cmd_queue_add (src_last_cmd);
	src_cmd_queue_process ();
    }

    src_speech_set_spelling_mode (last_mode);
    rv = TRUE;
    return rv;
}

static gboolean
src_nav_focus_flat ()
{
    gboolean rv = FALSE;

    return rv;
}


static gboolean
src_nav_focus_logic ()
{
    gboolean rv = FALSE;
    
    if (src_focused_sro)
    {
	if (src_crt_sro)
	    sro_release_reference (src_crt_sro);
	src_crt_sro = src_focused_sro;
	if (src_crt_sro)
	    sro_add_reference (src_crt_sro);
	
	src_cmd_queue_add ("present current object");
	if (src_mouse_take)
	{
	    src_cmd_queue_add ("mouse goto current");
	    if (src_mouse_click)
		src_cmd_queue_add ("mouse left click");
	}
	src_cmd_queue_process ();
	rv = TRUE;
    }
    else
    {
	if (src_use_speech)
	    src_say_message (_("no focus")); 
    }
    
    return rv;
}


static gboolean
src_nav_focus ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_focus_flat ();
    else
	rv = src_nav_focus_logic ();

    return rv;
}


static gboolean
src_nav_title_flat ()
{
    gboolean rv = FALSE;
    
    crt_line = 0;	
    rv = src_flat_review_next_line();

    return rv;
}

static gboolean
src_nav_title ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_title_flat ();
    else
	rv = src_nav_logic (SR_NAV_TITLE, _("no title"));

    return rv;
}

static gboolean
src_nav_menubar_flat ()
{
    gboolean rv = FALSE;

    return rv;
}

static gboolean
src_nav_menubar ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_menubar_flat ();
    else
	rv = src_nav_logic (SR_NAV_MENU, _("no menu bar"));

    return rv;
}

static gboolean
src_nav_toolbar_flat ()
{
    gboolean rv = FALSE;
    crt_line = n_lines;
    rv = src_flat_review_next_line();

    return rv;
}

static gboolean
src_nav_toolbar ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_toolbar_flat ();
    else
	rv = src_nav_logic (SR_NAV_TOOLBAR, _("no tool bar"));

    return rv;
}

static gboolean
src_nav_statusbar_flat ()
{
    gboolean rv = FALSE;
    
    crt_line = screen_review_get_focused_line ();	
    rv = src_flat_review_line (crt_line);

    return rv;
}

static gboolean
src_nav_statusbar ()
{
    gboolean rv = FALSE;
    
    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_nav_statusbar_flat ();
    else
	rv = src_nav_logic (SR_NAV_STATUSBAR, _("no status bar"));

    return rv;
}


extern gboolean SPI_generateMouseEvent (long int x, long int y, gchar *name);

static gboolean
src_generateMouseEvent (long int x, long int y, gchar *name)
{
    if (src_xevie_present)
	return SPI_generateMouseEvent (x, y, name);
    gdk_beep ();
    return FALSE;
}

static gboolean
src_mouse_left_press ()
{
    if (src_button_left_pressed) return FALSE;
    src_button_left_pressed = !src_button_left_pressed;
    return src_generateMouseEvent (-1, -1, "b1p");
}

static gboolean
src_mouse_left_release ()
{
    if (!src_button_left_pressed) return FALSE;
    src_button_left_pressed = !src_button_left_pressed;
    return src_generateMouseEvent (-1, -1, "b1r");
}

static gboolean
src_mouse_left_click ()
{
    if (src_button_left_pressed) return FALSE;
    return src_generateMouseEvent (-1, -1, "b1c");
}

/*
static gboolean
src_mouse_left_double_click ()
{
    return src_generateMouseEvent (-1, -1, "b1d");
}
*/

static gboolean
src_mouse_middle_press ()
{
    if (src_button_middle_pressed) return FALSE;
    src_button_middle_pressed = !src_button_middle_pressed;
    return src_generateMouseEvent (-1, -1, "b3p");
}

static gboolean
src_mouse_middle_release ()
{
    if (!src_button_middle_pressed) return FALSE;
    src_button_middle_pressed = !src_button_middle_pressed;
    return src_generateMouseEvent (-1, -1, "b3r");
}

static gboolean
src_mouse_middle_click ()
{
    if (src_button_middle_pressed) return FALSE;
    return src_generateMouseEvent (-1, -1, "b3c");
}

/*
static gboolean
src_mouse_middle_double_click ()
{
    return src_generateMouseEvent (-1, -1, "b3d");
}
*/


static gboolean
src_mouse_right_press ()
{
    if (src_button_right_pressed) return FALSE;
    src_button_right_pressed = !src_button_right_pressed;
    return src_generateMouseEvent (-1, -1, "b2p");
}

static gboolean
src_mouse_right_release ()
{
    if (!src_button_right_pressed) return FALSE;
    src_button_right_pressed = !src_button_right_pressed;
    return src_generateMouseEvent (-1, -1, "b2r");
}

static gboolean
src_mouse_right_click ()
{
    if (src_button_right_pressed) return FALSE;
    return src_generateMouseEvent (-1, -1, "b2c");
}

/*
static gboolean
src_mouse_right_double_click ()
{
    return src_generateMouseEvent (-1, -1, "b2d");
}
*/

static gboolean
src_get_mouse_coordinates_from_sro (SRObject *sro, 
				    gint32 *x, 
				    gint32 *y,
				    gboolean prel)
{
    SRRectangle rect;
    gboolean rv = FALSE;

    if (x)
	*x = -1;
    if (y)
	*y = -1;

    sru_return_val_if_fail (sro && x && y, FALSE);

    if (sro_is_text (sro, SR_INDEX_CONTAINER))
    {
        rv = sro_text_get_location_at_index (sro, src_text_index, &rect, SR_INDEX_CONTAINER);
	if (!rv)
	    rv = sro_text_get_text_location_from_caret (sro, SR_TEXT_BOUNDARY_LINE, SR_COORD_TYPE_SCREEN, &rect, SR_INDEX_CONTAINER);
    }
    
    if (!rv)
    	rv = sro_get_location (sro, SR_COORD_TYPE_SCREEN, &rect, SR_INDEX_CONTAINER);
    
    if (prel)
    {
	*x = rect.x + rect.width / 2;
	*y = rect.y + rect.height / 2;
    }
    else
    {
	*x = rect.x;
	*y = rect.y;
    }
    
    *x = *x - 1;
    *y = *y - 1;
    
    return rv;
}

gboolean
src_mouse_goto_crt_sro ()
{
    gint32 x, y;
    gboolean rv = FALSE;
    SRObject *obj;

    if (!src_crt_sro)
	return FALSE;
    obj = src_crt_sro;
    sro_add_reference (obj);
    if (src_get_mouse_coordinates_from_sro (src_crt_sro, &x, &y, TRUE))
	rv = SPI_generateMouseEvent (x, y, "abs");
    sro_release_reference (obj);
    return rv;
}

static gboolean
src_show_x_coord()
{
    gint32 x, y;

    if (!src_use_speech)
	return FALSE;
    if (!src_crt_sro)
	return FALSE;

    if (src_get_mouse_coordinates_from_sro (src_crt_sro, &x, &y, FALSE))
    {
	gchar *message;
	gchar tmp[10];
	sprintf (tmp, "%d", x);
	message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), tmp);
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	g_free (message);
    	return TRUE;
    }
    return FALSE;
}

static gboolean
src_show_y_coord ()
{
    gint32 x, y;
    
    if (!src_use_speech)
	return FALSE;
    if (!src_crt_sro)
	return FALSE;

    if (src_get_mouse_coordinates_from_sro (src_crt_sro, &x, &y, FALSE))
    {
	gchar *message;
	gchar tmp[10];
	sprintf (tmp, "%d", y);
	message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), tmp);
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	g_free (message);
    	return TRUE;
    }
    return FALSE;

}

static gboolean
src_font_style ()
{
    SRTextAttribute *attr;
    gchar *style = NULL;
    gchar *message;
    if (!src_use_speech)
	return FALSE;
    if (!src_crt_sro)
	return FALSE;
    if (!sro_is_text (src_crt_sro, SR_INDEX_CONTAINER))
	return FALSE;

    if (sro_text_get_attributes_at_index (src_crt_sro, src_text_index, &attr, SR_INDEX_CONTAINER))
    {
	gchar *tmp;
	if (sra_get_attribute_value (attr[0], "style", &tmp))
	{
	    style = g_strdup (tmp);
	    SR_freeString (tmp);
	}
	sra_free (attr);
    }
    
    if (!style)
    	style = g_strdup ("default");
	
    message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), style);
    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
    g_free (message);
    g_free (style);

    return TRUE;
}

static gboolean
src_font_size ()
{
    SRTextAttribute *attr;
    gchar *size = NULL;
    gchar *message;
    if (!src_use_speech)
	return FALSE;
    if (!src_crt_sro)
	return FALSE;
    if (!sro_is_text (src_crt_sro, SR_INDEX_CONTAINER))
	return FALSE;

    if (sro_text_get_attributes_at_index (src_crt_sro, src_text_index, &attr, SR_INDEX_CONTAINER))
    {
	gchar *tmp;
	if (sra_get_attribute_value (attr[0], "size", &tmp))
	{
	    size = g_strdup (tmp);
	    SR_freeString (tmp);
	}
	sra_free (attr);
    }
    
    if (!size)
	size = g_strdup ("default");
	
    message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), size);
    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
    g_free (message);
    g_free (size);

    return TRUE;
}

static gboolean
src_font_name ()
{
    SRTextAttribute *attr;
    gchar *name = NULL;
    gchar *message;
    if (!src_use_speech)
	return FALSE;
    if (!src_crt_sro)
	return FALSE;
    if (!sro_is_text (src_crt_sro, SR_INDEX_CONTAINER))
	return FALSE;

    if (sro_text_get_attributes_at_index (src_crt_sro, src_text_index, &attr, SR_INDEX_CONTAINER))
    {
	gchar *tmp;
	if (sra_get_attribute_value (attr[0], "font-name", &tmp))
	{
	    name = g_strdup (tmp);
	    SR_freeString (tmp);
	}
	sra_free (attr);
    }
    if (!name)
	name = g_strdup ("default");
	
    message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), name);
    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
    g_free (message);
    g_free (name);

    return TRUE;
}

static gboolean
src_text_attributes ()
{
    SRTextAttribute *attr;
    gchar *all = NULL;
    gchar *message;
    if (!src_use_speech)
	return FALSE;
    if (!src_crt_sro)
	return FALSE;
    if (!sro_is_text (src_crt_sro, SR_INDEX_CONTAINER))
	return FALSE;
    if (sro_text_get_attributes_at_index (src_crt_sro, src_text_index, &attr, SR_INDEX_CONTAINER))
    {
	gchar *tmp;
	if (sra_get_attribute_values_string (attr[0], NULL, &tmp))
	{
	    all = g_strdup (tmp);
	    SR_freeString (tmp);
	}
	sra_free (attr);
    }
    if (!all)
	all = g_strdup ("default");
	
    message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), all);
    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
    g_free (message);
    g_free (all);

    return TRUE;
}

static gboolean
src_move_caret ()
{
    if (!src_crt_sro)
	return FALSE;
    if (!sro_is_text (src_crt_sro, SR_INDEX_CONTAINER))
	return FALSE;

    return sro_text_set_caret_offset (src_crt_sro, src_text_index, SR_INDEX_CONTAINER);
}


static gboolean
has_parent_stop_role (SRObject *obj)
{
    static gchar *parent_role[] = {
	    "table",
	    "tree-table",
	    "page-tab",
	    "page-tab-list",
	    "panel", 
	    "filler", 
	    "frame",
	    "menu",
	    "menu-bar", 
	    "tool-bar",
	};
    gint i;
    gchar *role;
    gboolean rv = FALSE;
    
    sru_assert (obj);
    if (!sro_get_role_name (obj, &role, SR_INDEX_CONTAINER))
	return FALSE;
    
    if (strcmp (role, "filler") == 0 ||
	strcmp (role, "panel") == 0)
    {
	SRObject *child;
	
	if (!sro_get_sro (obj, SR_NAV_FIRST, &child, SR_NAV_MODE_WINDOW))
	{
	    child = obj;
	    sro_add_reference (child);
	}
	while (child)
	{
	    SRObject *tmp;
	    gchar *role2;
	    gboolean rv = FALSE;
	    if (child != obj && sro_get_role_name (child, &role2, SR_INDEX_CONTAINER))
	    {
		if (strcmp (role2, "filler") == 0 ||
		    strcmp (role2, "panel") == 0)
			rv = TRUE;
		SR_freeString (role2);
	    }
	    
	    tmp = NULL;
	    if (!rv)
	    	sro_get_sro (child, SR_NAV_NEXT, &tmp, SR_NAV_MODE_WINDOW);
	    sro_release_reference (child);
	    child = tmp;
	    if (rv)
		return TRUE;
	}
	return FALSE;
    }
    
    for (i = 0; i < G_N_ELEMENTS (parent_role); i++)
    {
	if (strcmp (role, parent_role[i]))
	    continue;
	rv = TRUE;
	break;
    }
    SR_freeString (role); 
    
    return rv;
}

static gboolean
has_child_stop_role (SRObject *obj)
{
    static gchar *child_role[] = {
	    "tree-table",
	    "page-tab-list",
	    "status-bar",
	    "tool-bar",
	    "menu-bar",
	    "menu-item",
	    "menu",
	    "text",
	    "label",
	    "table",
	    "push-button",
	    "check-box",
	    "slider",
	    "combo-box",
	    "spin-button",
	    "list",
	    "radio-button",
	    "separator",
	};

    gint i;
    gchar *role;
    gboolean rv = FALSE;
    
    sru_assert (obj);
    if (!sro_get_role_name (obj, &role, SR_INDEX_CONTAINER))
	return FALSE;
    
    for (i = 0; i < G_N_ELEMENTS (child_role); i++)
    {
	if (strcmp (role, child_role[i]))
	    continue;
	rv = TRUE;
	break;
    }
    SR_freeString (role); 
    
    return rv;
}

static gboolean
get_child_stop_role (SRObject *parent, 
		     GSList **role_list,
		     GSList **cnt_list)
{
    SRObject *child;
    sru_assert (parent);
    
    if (!sro_get_sro (parent, SR_NAV_CHILD, &child, SR_NAV_MODE_WINDOW))
	return FALSE;

    while (child)
    {
	SRObject *tmp;
	SRState state;
	sro_get_state (child, &state, SR_INDEX_CONTAINER);
	if (state & SR_STATE_SHOWING)
	{ 
	    if (has_child_stop_role (child))
	    {
		GSList *crt, *crt2;
		gchar *role;
		sro_get_role_name (child, &role, SR_INDEX_CONTAINER);
		for (crt = *role_list, crt2 = *cnt_list; crt; crt = crt->next, crt2 = crt2->next)
		{
		    if (strcmp (role, (gchar*) crt->data) == 0)
		    {
			crt2->data = GINT_TO_POINTER (GPOINTER_TO_INT (crt2->data) + 1);
			break;
		    }
		} 	 
		if (!crt)
		{
		    *role_list = g_slist_append (*role_list, g_strdup (role));
		    *cnt_list  = g_slist_append (*cnt_list, GINT_TO_POINTER (1));
		}
		SR_freeString (role);   
	    }
	    else
		get_child_stop_role (child, role_list, cnt_list);
	}
	    
	if (sro_get_sro (child, SR_NAV_NEXT, &tmp, SR_NAV_MODE_WINDOW))
	{
	    sro_release_reference (child);
	    child = tmp;
	}
	else
	{
	    sro_release_reference (child);
	    child = NULL;
	}
    }
    
    return TRUE;
    
}


static gboolean
src_surroundings_flat ()
{
    gboolean rv = FALSE;
    
    return rv;
}


static gboolean
src_surroundings_logic ()
{
    SRObject *parent;
    gchar *role, *name;
    gchar *message, *on, *curr_obj_role, *curr_obj_name, *from, *parrent_role, *with, 
          *no_items, *in, *window, *window_name, *of, *app_name;
    GSList *role_list, *cnt_list;
    GSList *crt, *crt2;
    
    if (!src_crt_sro)
	return FALSE;
    
    if (!sro_get_sro (src_crt_sro, SR_NAV_PARENT, &parent, SR_NAV_MODE_WINDOW))
	return FALSE;
    
    while (parent && !has_parent_stop_role (parent))
    {
	SRObject *tmp;
	if (!sro_get_sro (parent, SR_NAV_PARENT, &tmp, SR_NAV_MODE_WINDOW))
	{
	    sro_release_reference (parent);
	    parent = NULL;
	    break;
	}
	sro_release_reference (parent);
	parent = tmp;
    }
    
    if (!parent)
	return FALSE;
    role_list = cnt_list = NULL;
    get_child_stop_role (parent, &role_list, &cnt_list);
    
    if (sro_get_role_name (src_crt_sro, &role, SR_INDEX_CONTAINER))
    {  
	on = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("on"));
    	curr_obj_role = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), (gchar *)src_pres_get_role_name_for_speech (role));
	SR_freeString (role);
    }
    else
    {
	on = g_strdup ("");
	curr_obj_role = g_strdup ("");
    }
    
    if (sro_get_name (src_crt_sro, &name, SR_INDEX_CONTAINER))
    {
	curr_obj_name = src_xml_make_part ("TEXT", src_speech_get_voice ("name"), (gchar*)name);
	SR_freeString (name);
    }
    else
    {
	curr_obj_name = g_strdup ("");
    }
    
    if (sro_get_role_name (parent, &role, SR_INDEX_CONTAINER))
    {
	from = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("from a"));
	parrent_role = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), (gchar *)src_pres_get_role_name_for_speech (role));
	with = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("with"));
	SR_freeString (role);
    }
    else
    {
	from = g_strdup ("");
	parrent_role = g_strdup ("");
	with = g_strdup ("");
    }
    
    no_items = g_strdup ("");
    for (crt = role_list, crt2 = cnt_list; crt; crt = crt->next, crt2 = crt2->next)
    {
    	gchar *tmp, *tmp2, *tmp3, *tmp4;
	gchar cnt[10];
	
	tmp = no_items;
	sprintf (cnt, "%d", GPOINTER_TO_INT (crt2->data));
	tmp2 = src_xml_make_part ("TEXT", src_speech_get_voice ("childcount"), cnt);
	tmp3 = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), (gchar *)src_pres_get_role_name_for_speech ((gchar*) crt->data));
	tmp4 = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), ngettext ("item", "items", GPOINTER_TO_INT (crt2->data)));
	no_items = g_strconcat (no_items, tmp2, tmp3, tmp4, NULL);
	
	g_free (tmp);
	g_free (tmp2);
	g_free (tmp3);
	g_free (tmp4);
    }

    if (sro_get_window_name (src_crt_sro, &role, &name, SR_INDEX_CONTAINER))
    {
	in = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("in a"));
	window = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), (gchar*)role);
	if (name)
	    name = g_strconcat (_("named "), name, NULL);
	else
	    name = g_strdup (_("with no name"));    
	window_name = src_xml_make_part ("TEXT", src_speech_get_voice ("name"), (gchar*)name);
	SR_freeString (role);
	SR_freeString (name);
    }   
    else
    {
	in = g_strdup ("");
	window = g_strdup ("");
	window_name = g_strdup ("");  
    }
	
    if (sro_get_app_name (src_crt_sro, &name, SR_INDEX_CONTAINER))
    {
	of = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("of application"));
	app_name = src_xml_make_part ("TEXT", src_speech_get_voice ("name"), (gchar*)name);
	SR_freeString (name);
    } 
    else
    {
	of = g_strdup ("");
	app_name = g_strdup ("");
    }
    
    
    
    message = NULL;
    if (on || curr_obj_role || curr_obj_name || from || parrent_role 
                 || with || no_items || in || window 
		 || window_name || of || app_name)
	message = g_strconcat (on, curr_obj_role, curr_obj_name, 
			    from, parrent_role, with, no_items, 
			    in, window, window_name, of, 
			    app_name, NULL);

    if (message)
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	
#ifdef SRCTRL_DEBUG
    fprintf (stderr, "\n%s", message); 
#endif
    g_free (message);
    g_free (on);
    g_free (curr_obj_role);
    g_free (curr_obj_name);
    g_free (from);
    g_free (parrent_role);
    g_free (with);
    g_free (no_items);
    g_free (in);
    g_free (window);
    g_free (window_name);
    g_free (of);
    g_free (app_name);

    sro_release_reference (parent);
    g_slist_free (cnt_list);
    for (crt = role_list; crt; crt = crt->next)
	g_free (crt->data);
    g_slist_free (role_list);
    
    return TRUE;
}


static gboolean
src_surroundings ()
{
    gboolean rv = FALSE;

    if (!src_use_speech)
	return FALSE;

    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	rv = src_surroundings_flat ();
    else
	rv = src_surroundings_logic ();

    return rv;
}


static gboolean
src_overview ()
{
    GArray *array;
    gchar *message, *tmp, *message1, *message2, *message3, *message4, *message5, *message6;
    gint i;
    gchar *role;

    if (!src_use_speech)
	return FALSE;

    if (!src_focused_sro)
	return FALSE;

    if (!sro_get_surroundings (src_focused_sro, &array))
	return FALSE;

    message1 = message2 = message3 = message4 = message5 = message6 = NULL;
    
    tmp = src_xml_process_string (_("on a"));
    if (tmp)
	message1 = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), tmp);
    g_free (tmp);
    
    sro_get_role_name (src_focused_sro, &role, SR_INDEX_CONTAINER);
    message2 = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), (gchar *)src_pres_get_role_name_for_speech (role));
    SR_freeString (role);
    
    if (array->len > 0)
    {
	role = g_array_index (array, SRRoleCnt *, 0)->role;
	tmp = src_xml_process_string (_("from a"));
	if (tmp)
	    message3 = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), tmp);
	g_free (tmp);
    
	message4 = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), (gchar *)src_pres_get_role_name_for_speech (role));
	
	message5 = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("with"));
    }
    message6 = g_strdup ("");;
    for (i = 1; i < array->len; i++)
    {
	SRRoleCnt *role_cnt;
	gchar *tmp2, *tmp3, *tmp4;
	gchar cnt[10];
	role_cnt = g_array_index (array, SRRoleCnt *, i);
	if (!role_cnt)
	    continue;
	tmp = message6;
	sprintf (cnt, "%d", role_cnt->cnt);
	tmp2 = src_xml_make_part ("TEXT", src_speech_get_voice ("childcount"), cnt);
	tmp3 = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), (gchar *)src_pres_get_role_name_for_speech (role_cnt->role));
	tmp4 = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), ngettext ("item", "items", role_cnt->cnt == 1));
	message6 = g_strconcat (message6, tmp2, tmp3, tmp4, NULL);
	
	g_free (tmp);
	g_free (tmp2);
	g_free (tmp3);
	g_free (tmp4);
    }

    for (i = 0; i < array->len; i++)
    {
	SRRoleCnt *role_cnt;
	
	role_cnt = g_array_index (array, SRRoleCnt *, i);
	if (role_cnt && role_cnt->role)
	    SR_freeString (role_cnt->role);
	    
	g_free (role_cnt);
    }
    g_array_free (array, TRUE);

    message = g_strconcat (message1, message2, message3, 
			    message4, message5, message6, NULL);

    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	
    g_free (message);
    g_free (message1);
    g_free (message2);
    g_free (message3);
    g_free (message4);
    g_free (message5);
    g_free (message6);
    
    return TRUE;
}


static gchar*
src_present_line_for_speech (SRWAccLine *line)
{
    gint cnt, i;
    gchar *spout;

    sru_assert (line);
    sru_assert (line->srw_acc_line);
        
    src_enable_format_string = TRUE;
    cnt = line->srw_acc_line->len;
    spout = g_strdup ("");
    for (i = 0; i < cnt; i++)
    {
        SRWAccCell *cell;

        cell = g_array_index (line->srw_acc_line, SRWAccCell *, i);
	    
        switch (cell->id)
	{
	    case SRW_DELIMITER_CELL:
	    {
		gchar *tmp;
		tmp = spout;
		spout = g_strconcat (spout, " ", NULL);
		g_free (tmp);
	    };
	    break;
	    default: 
	    {
		if (cell->id > 0)
		{
		    gchar *tmp;
		    tmp = spout;
		    spout = g_strconcat (spout, cell->ch, NULL);
		    g_free (tmp);
		}    
	    };
	    break;
	}
    }
    
    if (spout && spout[0])
    {
	gchar *tmp;
	tmp = spout;
	spout = src_xml_format ("TEXT", src_speech_get_voice ("text"), spout);
	g_free (tmp);
    }
    
    return (spout && spout[0]) ? spout : NULL;
}

static gboolean
src_present_line_to_braille_or_brlmon (SRWAccLine *line)
{
    gint cnt, i;
    gchar *brlout, *add;

    sru_assert (line);
    sru_assert (line->srw_acc_line);

    src_enable_format_string = FALSE;

    cnt = line->srw_acc_line->len;
    brlout = g_strdup ("");
    add = g_strdup ("");
    for (i = 0; i < cnt; i++)
    {
	SRWAccCell *cell;
	    
	cell = g_array_index (line->srw_acc_line, SRWAccCell *, i);
	    
	switch (cell->id)
	{
	    default:
		{
		    gchar *tmp;
		    tmp = add;
		    add = g_strconcat (add, cell->ch, NULL);
		    g_free (tmp);
		};
		break;
	    case SRW_FILL_CELL:
		{
		    gchar *tmp;
		    tmp = add;
		    add = g_strconcat (add, " ", NULL);
		    g_free (tmp);
		};
		break;
	    case SRW_DELIMITER_CELL:
		{
		    gchar *tmp, *tmp2;
		    tmp = brlout;
		    tmp2 = add;
		    add = src_xml_format ("TEXT", NULL, add);
		    g_free (tmp2); 
		    brlout = g_strconcat (brlout, 
					    add ? add : "", 
					    "<DOTS>dot1237</DOTS>",
					    NULL);
		    g_free (tmp);
		    g_free (add);
		    add = g_strdup ("");
		};
		break;
	}
    }
    
    if (add[0])
    {
	gchar *tmp, *tmp2;
	tmp = brlout;
	tmp2 = add;
	add = src_xml_format ("TEXT", NULL, add);
	g_free (tmp2); 
	brlout = g_strconcat (brlout, add, NULL);
	g_free (tmp);
    }
    g_free (add);
    
    if (brlout)
    {
	gchar *tmp;
	gchar *braille_translation_table = NULL;
	gchar offset[10];
	gint braille_offset;
	
	tmp = brlout;
	braille_translation_table = src_braille_get_translation_table ();
	braille_offset = src_braille_get_offset ();
	
	sprintf (offset, "%d", braille_offset);
	brlout = g_strconcat ("<BRLOUT language=\"", braille_translation_table, "\" bstyle=\"8\" clear=\"true\">",
				"<BRLDISP role=\"main\" offset=\"", offset, "\" clear=\"true\">",
				brlout, 
				NULL);
	if (scroll)
	{
	    brlout = g_strconcat (brlout, 
	    	            	  "<SCROLL mode=\"", scroll, "\"></SCROLL>",
				  "</BRLDISP>",
				  "</BRLOUT>",
				  NULL);
	}  
	else
	{
	    brlout = g_strconcat (brlout, 
				  "</BRLDISP>",
				  "</BRLOUT>",
				  NULL);
	}			
	g_free (tmp);
	if (src_use_braille)
	    src_braille_send (brlout);
	if (src_use_braille_monitor)
	    src_brlmon_send (brlout);
#ifdef SRCTRL_DEBUG
    fprintf (stderr, "\n%s", brlout); 
#endif
    }
    g_free (brlout);
    
    return TRUE;
}


static gboolean
src_flat_review_line (gint index)
{
    SRWAccLine *line = NULL;

    int y1 = 0, y2 = 0;

    sru_assert (1 <= index && index <= n_lines);
    
    if (visited && (index == 1 || index == n_lines))
    {
	if (index == 1 && src_use_speech)
	{
	    gchar *message;
	    message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("first line"));
	    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	    g_free (message);
	}    
	else if (index == n_lines && src_use_speech)
	{
	    gchar *message;
	    message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("last line"));
	    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	    g_free (message);
	}
	
	if (index == 1 && (src_use_braille || src_use_braille_monitor))
	{
	    if (src_use_braille)
		src_braille_show (_("[first line]"));
	    if (src_use_braille_monitor)
		src_brlmon_show (_("[first line]"));
	}
	else if (index == n_lines && (src_use_braille || src_use_braille_monitor))
	{
	    if (src_use_braille)
		src_braille_show (_("[last line]"));
	    if (src_use_braille_monitor)
		src_brlmon_show (_("[last line]"));
	}
	return TRUE;
    }
	
    line = screen_review_get_line (index, &y1, &y2);
	
    flat_review_y = (y1 + y2) /2;
    SPI_generateMouseEvent (flat_review_x,
			    flat_review_y,
			    "abs");
    if ((!line || line->is_empty) && src_use_speech)
    {
	if (line->is_empty == 1)
	{
	    gchar *message;
	    message = src_xml_make_part ("TEXT", src_speech_get_voice ("empty"), _("empty line"));
	    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	    g_free (message);
	}
	else
	{
	    gchar *aux_str = NULL, *message;
	    aux_str = g_strdup_printf (ngettext ("%d empty line", "%d empty lines", line->is_empty), line->is_empty );
	    message = src_xml_make_part ("TEXT", src_speech_get_voice ("empty"), aux_str);
	    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	    g_free (message);
	    g_free (aux_str);
	}
    }
    else if (line && src_use_speech)
    {
	gchar *spout;
    	spout = src_present_line_for_speech (line);
    	if (spout)
	    src_speech_send_chunk (spout, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	g_free (spout);
    }
    
    if ((!line || line->is_empty) && (src_use_braille || src_use_braille_monitor))
    {
	if (line->is_empty == 1)
	{
	    if (src_use_braille)
		src_braille_show (_("[empty line]"));
	    if (src_use_braille_monitor)
		src_brlmon_show (_("[empty line]"));
	}
	else
	{
	    gchar *aux_str = NULL;
	    aux_str = g_strdup_printf (ngettext ("[%d empty line]", "[%d empty lines]", line->is_empty), line->is_empty );
	    if (src_use_braille)
		src_braille_show (aux_str);
	    if (src_use_braille_monitor)
		src_brlmon_show (aux_str);
	    g_free (aux_str);
	}

    }
    else if (line && (src_use_braille || src_use_braille_monitor))
	src_present_line_to_braille_or_brlmon (line);
	
    if (index == 1 || index == n_lines)
	visited = TRUE;
    else
	visited = FALSE;		

    return TRUE;
}



static gboolean
src_flat_review_next_line()
{
    crt_line++;
    if (crt_line >= n_lines) 
	crt_line = n_lines;

    return src_flat_review_line (crt_line);
}


static gboolean
src_flat_review_prev_line()
{
    crt_line--;
    if (crt_line <= 0) 
	crt_line = 1;

    return src_flat_review_line (crt_line);
}


static gboolean
src_flat_review ()
{
    SRRectangle clip_rectangle = {0,0,300,300};
    SRObject 	*window;
    if (!src_crt_sro)
	return FALSE;
/*    sro_get_sro (src_crt_sro, SR_NAV_PARENT, &parent, src_nav_mode);*/
    sro_get_sro (src_crt_sro, SR_NAV_WINDOW, &window, src_nav_mode);
    if (window)
    {
	gint screen_review_flag = SRW_ALIGNF_ALL;
	if (!srconf_get_data_with_default (SRCORE_SCREEN_REVIEW	, 
					   CFGT_INT, 
					   &screen_review_flag, 
					   (gpointer)&screen_review_flag,
					   SRCORE_PATH)
	    ) return FALSE;

	
    	sro_get_location (window,
			  SR_COORD_TYPE_SCREEN,
			  &clip_rectangle,
			  0);
	sro_release_reference (window);
	n_lines = screen_review_init (&clip_rectangle, 
				      src_crt_sro,
				      screen_review_flag,
				      SRW_SCOPE_WINDOW);
	if (n_lines)
	{				      
	    crt_line = screen_review_get_focused_line ();	
	    flat_review_x = clip_rectangle.x;
	    src_flat_review_line (crt_line);
	}
	else
	{
	    if (src_use_speech)
		src_say_message (_("cannot make flat review for current window"));
	    sru_warning (_("cannot make flat review for current window"));
	    src_ctrl_flat_mode_terminate ();
	    return FALSE;
	}
    }
    else
    {
	if (src_use_speech)
	    src_say_message (_("cannot make flat review for current window"));
    }

    return TRUE;
}


static gboolean
get_hierarchy_from_sro (SRObject *obj, gchar **hierarchy)
{
    gchar *children, *name, *role, *role1;
    SRObject *child;
    SRState state;
    
    sru_assert (obj && hierarchy);
    
    sro_get_state (obj, &state, SR_INDEX_CONTAINER);
    if (!((state & SR_STATE_SHOWING) && (state & SR_STATE_VISIBLE)))
	return *hierarchy ? TRUE : FALSE;
    
    name = role = role1 = NULL;
    sro_get_role_name (obj, &role1, SR_INDEX_CONTAINER);
    if (has_child_stop_role (obj) ||
	    strcmp (role1, "frame") == 0 ||
	    strcmp (role1, "page-tab") == 0)
    {
	if (sro_get_name (obj, &name, SR_INDEX_CONTAINER))
	{
	    gchar *tmp;
	    tmp = name;
	    name = src_xml_format ("TEXT", src_speech_get_voice ("name"), name);
	    SR_freeString (tmp);
	}
	if (sro_get_role_name (obj, &role, SR_INDEX_CONTAINER))
	{
	    gchar *tmp;
	    tmp = role;
	    role = src_xml_format ("TEXT", src_speech_get_voice ("role"), role);
	    SR_freeString (tmp);
	}
    }
    if (role1)
	SR_freeString (role1);
    children = g_strdup ("");
    child = NULL;
    sro_get_sro (obj, SR_NAV_CHILD, &child, SR_NAV_MODE_WINDOW);
    
    while (child)
    {
	SRObject *obj2;
	gchar *tmp;
	
	tmp = NULL;
	get_hierarchy_from_sro (child, &tmp);
	if (tmp)
	{
	    gchar *tmp2;
	    tmp2 = children;
	    children = g_strconcat (children, tmp, NULL);
	    g_free (tmp2);
	    g_free (tmp);
	}
	obj2 = NULL;
	sro_get_sro (child, SR_NAV_NEXT, &obj2, SR_NAV_MODE_WINDOW);
	sro_release_reference (child);
	child = obj2;
    }
    
    if (name || role || children)
    {
	gchar *tmp;
	tmp = *hierarchy;
	*hierarchy = g_strconcat (name ? name : "",
				    role ? role : "",
				    *hierarchy ? *hierarchy : "",
				    children ? children : "",
				    NULL);
	g_free (tmp);
	g_free (role);
	g_free (name);	
	g_free (children);
    }
    
    return *hierarchy ? TRUE : FALSE;	    
}


static gboolean
src_hierarchy_logic ()
{
    SRObject *parent;

    gchar *message;
    
    if (!src_use_speech)
	return FALSE;
    
    if (!src_crt_sro)
	return FALSE;
    
    parent = src_crt_sro;
    sro_add_reference (parent);
    
    while (parent)
    {
	SRObject *tmp;
	if (!sro_get_sro (parent, SR_NAV_PARENT, &tmp, SR_NAV_MODE_WINDOW))
	    break;
	
	sro_release_reference (parent);
	parent = tmp;
    }
    if (!parent)
	return FALSE;
    message = NULL;
    get_hierarchy_from_sro (parent, &message);
    
    sro_release_reference (parent);
    
    if (message)
    	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);

    g_free (message);
    return TRUE;
}

static gboolean
src_hierarchy_flat ()
{
    gint cnt = 0;
    gint i;

    SRRectangle clip_rectangle = {0,0,300,300};
    SRObject 	*window;

    if (!src_use_speech)
	return FALSE;

    if (!src_crt_sro)
	return FALSE;

/*    sro_get_sro (src_crt_sro, SR_NAV_PARENT, &parent, src_nav_mode);*/
    sro_get_sro (src_crt_sro, SR_NAV_WINDOW, &window, src_nav_mode);
    if (window)
    {
    	sro_get_location (window,
			  SR_COORD_TYPE_SCREEN,
			  &clip_rectangle,
			  0);
	sro_release_reference (window);
	
	n_lines = cnt = screen_review_init (&clip_rectangle, src_crt_sro, SRW_ALIGNF_ALL, SRW_SCOPE_WINDOW);
	if (!n_lines)
	{
	    if (src_use_speech)
		src_say_message (_("cannot make flat review for current window"));
	    sru_warning (_("cannot make flat review for current window"));
	    src_ctrl_flat_mode_terminate ();
	    return FALSE;
	}
	
	flat_review_x = clip_rectangle.x;
    }
    for (i = 1; i <= cnt; i++)
    {
	SRWAccLine *line;
	int y1, y2;
	gchar *tmp = NULL;

	line = screen_review_get_line (i, &y1, &y2);

	if (line)
	    tmp = src_present_line_for_speech (line);
	if (tmp && tmp[0])
	    src_speech_send_chunk (tmp, SRC_SPEECH_PRIORITY_IDLE, FALSE);
	g_free (tmp);    
    }

    screen_review_terminate ();

    return TRUE;
}

static gboolean
src_do_default_action ()
{
    gchar *role;
    SRObject *obj;
    gboolean rv = FALSE;

    obj = src_crt_sro;
    
    if (!obj)
	return FALSE;
    
    sro_get_role_name (obj, &role, SR_INDEX_CONTAINER);
    if (strcmp (role, "table-line") == 0)
    {
	rv = sro_action_do_action (obj, "expand or contract", SR_INDEX_CONTAINER);
	if (!rv)
	    rv = sro_action_do_action (obj, "activate", SR_INDEX_CONTAINER);
    }
    else if (strcmp (role, "text") == 0)
    {
	rv = sro_action_do_action (obj, "activate", SR_INDEX_CONTAINER);
    }
    else if ( strcmp (role, "push-button") == 0 	||
	      strcmp (role, "menu-item") == 0 		||
	      strcmp (role, "check-box") == 0 		||
	      strcmp (role, "table-column-header") == 0	||
	      strcmp (role, "radio-button") == 0)
    {
	rv = sro_action_do_action (obj, "click", SR_INDEX_CONTAINER);
    };
    
    return rv;
}


static gboolean
src_ctrl_flat_mode_init ()
{
    gboolean rv = FALSE;
    
    src_braille_set_offset (0);
    src_tracking_mode = SRC_MODE_FLAT_REVIEW;
    if (src_use_speech)
	src_say_message (_("flat review mode")); 

    rv = src_flat_review ();
    
    return rv;
}


gboolean
src_ctrl_flat_mode_terminate ()
{
    if (src_tracking_mode == SRC_MODE_FOCUS_TRACKING)
	return TRUE;
    
    src_tracking_mode = SRC_MODE_FOCUS_TRACKING;
    if (src_use_speech)
	src_say_message (_("focus tracking mode")); 
    
    screen_review_terminate ();

    return TRUE;
}


static gboolean
src_toggle_tracking_mode ()
{
    if (!src_crt_sro)
	return FALSE;

    if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	src_ctrl_flat_mode_terminate ();
    else
	src_ctrl_flat_mode_init ();
/*
    if (src_use_speech)
    {
	if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	    src_say_message (_("flat review mode")); 
	else
	    src_say_message (_("focus tracking mode")); 
    }
*/
    return TRUE;
}


static gboolean
src_detailed_informations ()
{
    if (src_crt_sro)
    {
	src_present_details ();
    }
    return TRUE;
}


static gboolean
src_shutup ()
{
    if (!src_use_speech)
	return FALSE;
    src_repeat_cnt = 1;
    src_speech_shutup ();
    return TRUE;
}


static gboolean
src_pause_resume ()
{
    static gboolean pause = TRUE;
    
    if (!src_use_speech)
	return FALSE;

    if (pause)
    {
	src_speech_pause ();
	src_cmd_queue_add ("shutup");
    }
    else
    {
	src_speech_resume ();
	src_cmd_queue_add ("repeat last");
    }
    src_cmd_queue_process ();
    pause = !pause;

    return TRUE;
}

static gboolean
src_braille_scroll_char_left ()
{
    gboolean old = src_use_speech;
    if (scroll)
	g_free (scroll);
    scroll = g_strdup ("-1");
    src_use_speech = FALSE;
    src_cmd_queue_add ("repeat last");
    src_cmd_queue_process ();
    src_use_speech = old;
    return TRUE; 
}

static gboolean
src_braille_scroll_char_right ()
{
    gboolean old = src_use_speech;
      if (scroll)
	g_free (scroll);
    scroll = g_strdup ("1");
    src_use_speech = FALSE;
    src_cmd_queue_add ("repeat last");
    src_cmd_queue_process ();
    src_use_speech = old;

    return TRUE; 
}


static gboolean
src_braille_scroll_display_left ()
{
    gboolean old = src_use_speech;
    if (scroll)
	g_free (scroll);
    scroll = g_strdup ("-width");
    src_use_speech = FALSE;
    src_cmd_queue_add ("repeat last");
    src_cmd_queue_process ();
    src_use_speech = old;

    return TRUE; 
}


static gboolean
src_braille_scroll_display_right ()
{
    gboolean old = src_use_speech;
    if (scroll)
	g_free (scroll);
    scroll = g_strdup ("width");
    src_use_speech = FALSE;
    src_cmd_queue_add ("repeat last");
    src_cmd_queue_process ();
    src_use_speech = old;

    return TRUE; 
}

static gboolean
src_braille_on_off ()
{
    gboolean braille_on_off = !src_use_braille;
    
    if (!srconf_set_data (SRCORE_BRAILLE_ACTIVE,
			  CFGT_BOOL, 
			  &braille_on_off,
			  SRCORE_PATH)
        ) return FALSE;
    	
    return TRUE;
}

static gboolean
src_speech_on_off ()
{
    gboolean speech_on_off = !src_use_speech;
    
    if (!srconf_set_data (SRCORE_SPEECH_ACTIVE,
			  CFGT_BOOL, 
			  &speech_on_off,
			  SRCORE_PATH)
        ) return FALSE;

    return TRUE;
}

static gboolean
src_magnifier_on_off ()
{
    gboolean magnifier_on_off = !src_use_magnifier;
    
    if (!srconf_set_data (SRCORE_MAGNIF_ACTIVE,
			  CFGT_BOOL, 
			  &magnifier_on_off,
			  SRCORE_PATH)
        ) return FALSE;
	
    return TRUE;
}

static gboolean
src_braille_monitor_on_off ()
{
    gboolean braille_monitor_on_off = !src_use_braille_monitor;
    
    if (!srconf_set_data (SRCORE_BRAILLE_MONITOR_ACTIVE,
			  CFGT_BOOL, 
			  &braille_monitor_on_off,
			  SRCORE_PATH)
        ) return FALSE;

    return TRUE;
}



static struct
{
    gchar *name;
    SRCSampleFunction function;
}src_cmd_function[] = 
    {
  	{"decrease y scale",		src_mag_decrease_y_scale 	},
	{"decrease x scale",		src_mag_decrease_x_scale 	},
	{"increase y scale",		src_mag_increase_y_scale 	},
	{"increase x scale",		src_mag_increase_x_scale 	},
	{"lock xy scale",		src_mag_lock_xy_scale    	},
	{"smoothing toggle",		src_mag_smoothing_toggle    	},
	{"cursor toggle",		src_mag_cursor_toggle    	},
	{"cursor on/off",		src_mag_cursor_on_off  	 	},
	{"cursor mag on/off",		src_mag_cursor_mag_on_off 	},
	{"decrease cursor size",	src_mag_cursor_decrease_size 	},
	{"increase cursor size",	src_mag_cursor_increase_size 	},
	{"invert on/off",		src_mag_invert_on_off  		},
	{"mag default",			src_mag_default_all    	 	},
	{"crosswire on/off",		src_mag_crosswire_on_off  	},
	{"crosswire clip on/off",	src_mag_crosswire_clip_on_off 	},
	{"decrease crosswire size",	src_mag_crosswire_decrease_size	},
	{"increase crosswire size",	src_mag_crosswire_increase_size	},
	{"automatic panning on/off",	src_mag_panning_on_off		},
 	{"mouse tracking toggle",	src_mag_mouse_tracking_toggle	},
  	{"load magnifier defaults",	src_mag_default_all		},
 
	{"decrease pitch",		src_speech_decrease_pitch	},
	{"increase pitch",		src_speech_increase_pitch	},
	{"default pitch",		src_speech_default_pitch 	},
	{"decrease rate",		src_speech_decrease_rate 	},
	{"increase rate",		src_speech_increase_rate 	},
	{"default rate",		src_speech_default_rate  	},
	{"decrease volume",		src_speech_decrease_volume	},
	{"increase volume",		src_speech_increase_volume	},
	{"default volume",		src_speech_default_volume 	},
	{"speech default",		src_speech_default_all   	},

	{"goto parent",			src_nav_parent		 	},
	{"goto child", 			src_nav_child		 	},
	{"goto previous", 		src_nav_previous	 	},
	{"goto next", 			src_nav_next		 	},
	{"repeat last",			src_repeat_last		 	},
	{"goto focus", 			src_nav_focus		 	},
	{"goto title", 			src_nav_title		 	},
	{"goto menu", 			src_nav_menubar		 	},
	{"goto toolbar", 		src_nav_toolbar		 	},
	{"goto statusbar", 		src_nav_statusbar	 	},
	{"widget surroundings", 	src_surroundings		},
	{"goto caret", 			src_nav_caret		 	},
	{"goto first", 			src_nav_first		 	},
	{"goto last", 			src_nav_last		 	},
	{"change navigation mode",	src_nav_change_mode	 	},
	{"toggle tracking mode",	src_toggle_tracking_mode	},

	{"attributes at caret", 	src_caret_attributes	 	},
	{"watch current object", 	src_watch_current	 	},
	{"unwatch all objects", 	src_unwatch_all	 		},

	{"window hierarchy",		src_hierarchy_logic		},
	{"read whole window",	        src_hierarchy_flat		},
	{"detailed informations",	src_detailed_informations	},
	{"do default action",		src_do_default_action		},
	{"window overview",		src_overview			},
	{"find next",			src_find_next			},
	{"find set",			src_find_set			},


	{"mouse left press",		src_mouse_left_press	 	},
	{"mouse left click", 		src_mouse_left_click		},
	{"mouse left release",		src_mouse_left_release		},
	{"mouse right press",		src_mouse_right_press		},
	{"mouse right click", 		src_mouse_right_click		},
	{"mouse right release",		src_mouse_right_release		},
	{"mouse middle press", 		src_mouse_middle_press		},
	{"mouse middle release",	src_mouse_middle_release	},
	{"mouse middle click", 		src_mouse_middle_click		},
	{"mouse goto current", 		src_mouse_goto_crt_sro 		},
	


	{"present current object", 	src_present_crt_sro		},
	{"present current window", 	src_present_crt_window		},
	{"present current tooltip", 	src_present_crt_tooltip		},
	{"present last message", 	src_present_last_message	},
	{"present layer timeout",	src_present_layer_timeout	},
	{"present layer changed",	src_present_layer_changed	},
	{"show x coordinate", 		src_show_x_coord		},
	{"show y coordinate", 		src_show_y_coord		},
	{"font style", 			src_font_style			},
	{"font size", 			src_font_size			},
	{"font name", 			src_font_name			},
	{"text attributes", 		src_text_attributes		},
	{"move caret", 			src_move_caret			},
	{"shutup", 			src_shutup			},
	{"pause/resume",		src_pause_resume		},
	
	{"kbd key", 			src_kb_key_echo			},
	{"kbd punct", 			src_kb_punct_echo		},
	{"kbd space", 			src_kb_space_echo		},
	{"kbd modifier",		src_kb_modifier_echo		},
	{"kbd cursor",			src_kb_cursor_echo		},

	{"char left",			src_braille_scroll_char_left	}, 
	{"char right", 			src_braille_scroll_char_right	},
	{"display left", 		src_braille_scroll_display_left	},
	{"display right", 		src_braille_scroll_display_right},

	{"braille on/off",		src_braille_on_off		},
	{"speech on/off",		src_speech_on_off		},
	{"magnifier on/off",		src_magnifier_on_off		},
	{"braille monitor on/off", 	src_braille_monitor_on_off	}

    };

static GQueue *src_cmd_queue 	= NULL;
static GQueue *src_key_queue 	= NULL;

static GHashTable *src_cmd_function_hash 	= NULL;
static GHashTable *src_key_cmds_hash  		= NULL;



static gboolean
src_cmd_function_hash_init ()
{
    gint i;
    sru_assert (src_cmd_function_hash == NULL);
    src_cmd_function_hash = g_hash_table_new (g_str_hash , g_str_equal);
    for (i = 0; i < G_N_ELEMENTS (src_cmd_function); i++)
    {
	g_hash_table_insert (src_cmd_function_hash, 
				src_cmd_function[i].name,
				src_cmd_function[i].function);
    }
    
    return TRUE;
}


gboolean
src_cmd_queue_add (gchar *cmd)
{
    sru_assert (src_cmd_queue && cmd);
    
    g_queue_push_head (src_cmd_queue, cmd);
    
    return TRUE;
}

gboolean
src_cmd_queue_remove (gchar **cmd)
{
    sru_assert (cmd && src_cmd_queue && !g_queue_is_empty (src_cmd_queue));
    
    *cmd = (gchar *) g_queue_pop_tail (src_cmd_queue);
    
    return TRUE;
}


static gboolean
src_cmd_process (gchar *cmd)
{
    SRCSampleFunction function;

    sru_assert (cmd);
#ifdef SRCTRL_DEBUG
    fprintf (stderr, "\nStart process %s", cmd); 
#endif
    function = g_hash_table_lookup (src_cmd_function_hash, cmd);
    if (function)
	function ();
#ifdef SRCTRL_DEBUG    
    fprintf (stderr, "\nEnd process %s", cmd); 
#endif
    return TRUE;
}



gboolean
src_cmd_queue_process ()
{	
    sru_assert (src_cmd_queue);

    while (!g_queue_is_empty (src_cmd_queue))
    {
	gchar *cmd;
	/* preprocess script stuff */
	src_cmd_queue_remove (&cmd);
	
	if (cmd)
	{
	    static gboolean repeat = FALSE;
	    
	    if (src_last_cmd && strcmp (src_last_cmd, "present layer changed") != 0)
		src_before_last_cmd = src_last_cmd;
		
	    if (strcmp (cmd, "widget surroundings") 	== 0 ||
		strcmp (cmd, "present current object") 	== 0 ||
		strcmp (cmd, "present current window") 	== 0 ||
		strcmp (cmd, "present current tooltip")	== 0 ||
		strcmp (cmd, "present last message") 	== 0 ||
		strcmp (cmd, "present layer timeout") 	== 0 ||
		strcmp (cmd, "present layer changed") 	== 0 ||
		
		strcmp (cmd, "kbd key") 		== 0 ||
		strcmp (cmd, "kbd punct") 		== 0 ||
		strcmp (cmd, "kbd space") 		== 0 ||
		strcmp (cmd, "kbd modifier") 		== 0 ||
		strcmp (cmd, "kbd cursor") 		== 0 ||
		strcmp (cmd, "window hierarchy")	== 0 ||
		strcmp (cmd, "window overview")		== 0 ||
		strcmp (cmd, "detailed informations")	== 0)
	    {
		src_repeat_cnt = repeat ? src_repeat_cnt + 1 : 1; 
		src_last_cmd = cmd;
	    }
	    else if (strcmp (cmd, "repeat last") 	!= 0 &&
		     strcmp (cmd, "shutup") 		!= 0 &&
		     strcmp (cmd, "pause/resume") 	!= 0 &&
		     strcmp (cmd, "char left")		!= 0 &&
		     strcmp (cmd, "char right")		!= 0 &&
		     strcmp (cmd, "display left")	!= 0 &&
		     strcmp (cmd, "display right")	!= 0)
	    {
		src_last_cmd = NULL;
	    }
	    repeat = FALSE;
	    if (strcmp (cmd, "repeat last") == 0)
	    	repeat = TRUE;
	
	    
#ifdef SRCTRL_DEBUG	    
	    fprintf (stderr, "\n   cmd: %s", cmd);	
	    fprintf (stderr, "\n   last_cmd: %s", src_last_cmd);	
	    fprintf (stderr, "\n   before_last_cmd: %s", src_before_last_cmd);	
#endif	    
	    src_cmd_process (cmd);
	    repeat = FALSE;
	}
	/* postprocess script stuff */
    }
    return TRUE;
}


gboolean
src_key_queue_add (gchar *key)
{
    sru_assert (key && src_key_queue);
    
    g_queue_push_head (src_key_queue, key);
    
    return TRUE;
}

gboolean
src_key_queue_remove (gchar **key)
{
    sru_assert (key && src_key_queue && !g_queue_is_empty (src_key_queue));
    
    *key = (gchar *) g_queue_pop_tail (src_key_queue);
    
    return TRUE;
}


static gboolean
src_key_process (gchar *key)
{
    gchar **functions = NULL;
        
    sru_assert (key && src_cmd_queue);

    functions = g_hash_table_lookup (src_key_cmds_hash, key);
    if (functions)
    {
	gint i;
        for (i = 0; functions[i]; i++)
	    src_cmd_queue_add (functions[i]);
	src_cmd_queue_process ();
    }
    
    return TRUE;
}

static gboolean
src_key_queue_process ()
{
    sru_assert (src_key_queue);
    
    while (!g_queue_is_empty (src_key_queue))
    {
	gchar *key;
	src_key_queue_remove (&key);
	/* preprocess script stuff */
	if (key)
	    src_key_process (key);
	/* postprocess script stuff */
    }

    return TRUE;
}


gboolean
src_ctrl_process_key (gchar *key)
{
    sru_assert (key && src_key_queue);
    
    g_queue_push_head (src_key_queue, key);
    src_key_queue_process ();
    
    return TRUE;
}

static void
ctrl_free (gpointer data,
	   gpointer user_data)
{
    g_free (data);
}

static gboolean
src_key_cmds_mapped_add_from_list (gchar *entry, GSList *keys)
{
    GSList *crt;
    sru_assert (src_key_cmds_hash);
    sru_assert (keys && entry && entry[0]);

    for (crt = keys; crt; crt = crt->next)
    {
	gchar *path;
	gint index;
	GSList *functions, *crt2;
	gchar **mapped;
	
	gboolean defa;
	
	if (!crt->data || !((gchar*)crt->data)[0])
	{
	    sru_warning (_("\"%s\" entry has invalid value at position %d."),
				    entry, g_slist_position (keys, crt)); 
	    continue;
	}
	
	path = g_strconcat ((gchar*)crt->data, "/key_list", NULL);
	defa = srconf_get_data_with_default (path, CFGT_LIST, &functions, NULL,
					SRC_KEY_SECTION);
	g_free (path);
	
	if (!functions)
	{
/*	    sru_warning (_("cannot get functions for key \"%s\""), (gchar*)crt->data);
*/	    continue;
	}
	mapped = g_new (gchar*, g_slist_length (functions) + 1);
	index = 0;
	for (crt2 = functions; crt2; crt2 = crt2->next)
	{
	    if (!crt2->data || !((gchar*)crt2->data)[0])
	    {
		sru_warning (_("key \"%s\" has an invalid value at position %d."),
				    (gchar*)crt->data, g_slist_position (functions, crt2));
		continue;
	    }
	    mapped [index] = g_strdup (crt2->data);
	    index++;
	}
	mapped [index] = NULL;

	if (mapped[0])
	    g_hash_table_insert (src_key_cmds_hash, g_strdup(crt->data), mapped);
	else
	    g_free (mapped);
    	g_slist_foreach (functions, ctrl_free, NULL);
	g_slist_free (functions);
    }

    return TRUE;
}


static gboolean
src_key_cmds_hash_init ()
{
    GSList *list;

    sru_assert (!src_key_cmds_hash);

    src_key_cmds_hash = g_hash_table_new_full (g_str_hash , g_str_equal,
				    g_free, (GDestroyNotify)g_strfreev);
    if (!src_key_cmds_hash)
	return FALSE;

    list = NULL;
    if (!srconf_get_data_with_default (SRC_KEY_PAD_LIST , CFGT_LIST,
					&list, NULL, SRC_KEY_PAD_SECTION))
	sru_warning (_("Can not get key pad commands mapping from gconf."));

    if (list)
	src_key_cmds_mapped_add_from_list (SRC_KEY_PAD_SECTION "/"
				SRC_KEY_PAD_LIST, list);
    g_slist_foreach (list, ctrl_free, NULL);
    g_slist_free (list);

    list = NULL;
    if (!srconf_get_data_with_default (SRC_BRAILLE_KEY_LIST , CFGT_LIST,
			    &list, NULL, SRC_BRAILLE_KEY_SECTION))
        sru_warning (_("Can not get braille display commands mapping from gconf."));
    
    if (list)
	src_key_cmds_mapped_add_from_list (SRC_BRAILLE_KEY_SECTION "/"
				SRC_BRAILLE_KEY_LIST, list);
    g_slist_foreach (list, ctrl_free, NULL);
    g_slist_free (list);

    list = NULL;
    if (!srconf_get_data_with_default (SRC_USER_DEF_LIST , CFGT_LIST,
			    &list, NULL, SRC_USER_DEF_SECTION))
	sru_warning (_("Can not get user defined commands mapping from gconf."));
	
    if (list)
	src_key_cmds_mapped_add_from_list (SRC_USER_DEF_SECTION "/"
				SRC_USER_DEF_LIST, list);
    g_slist_foreach (list, ctrl_free, NULL);
    g_slist_free (list);

    return TRUE;
}

gboolean
src_ctrl_key_cmds_reload ()
{
    sru_assert (src_key_cmds_hash);
    
    g_hash_table_destroy (src_key_cmds_hash);
    src_key_cmds_hash = NULL;
    src_key_cmds_hash_init ();

    return TRUE;
}


gboolean
src_ctrl_init ()
{
    int mop, fer, fev;
    src_cmd_function_hash_init ();
    src_key_cmds_hash_init ();
    
    src_cmd_queue = g_queue_new ();
    src_key_queue = g_queue_new ();

    src_last_cmd = NULL;
    src_before_last_cmd = NULL;
    src_repeat_cnt = 1;
    src_xevie_present = XQueryExtension (GDK_DISPLAY (), "XEVIE", &mop, &fev, &fer);

    return TRUE;
}


gboolean
src_ctrl_terminate ()
{
    g_queue_free (src_cmd_queue);
    g_queue_free (src_key_queue);
    
    src_cmd_queue = NULL;
    src_key_queue = NULL;

    g_hash_table_destroy (src_cmd_function_hash);
    g_hash_table_destroy (src_key_cmds_hash);
    
    src_cmd_function_hash = NULL;
    src_key_cmds_hash = NULL;
    
    src_last_cmd = NULL;
    src_before_last_cmd = NULL;
    src_repeat_cnt = 1;
    
    return TRUE;
}

SRObject*
src_ctrl_flat_get_sro_at_index (gint index)
{
    SRWAccLine *line = NULL;
    SRWAccCell *cell;
    int y1 = 0, y2 = 0;

    if (1 > crt_line || crt_line > n_lines)
	return NULL;
    line = screen_review_get_line (crt_line, &y1, &y2);
    if (!line)
	return NULL;
    if (index < 0 || index >= line->srw_acc_line->len)
	return NULL;
    cell = g_array_index (line->srw_acc_line, SRWAccCell *, index);
    if (!cell)
	return NULL;
    sro_add_reference (cell->source);
    return cell->source;
}

static void
src_ctrl_position_sensor_action_flat (gint index)
{
    SRObject *sro;
    gchar *role;
    if (!src_use_speech)
	return;
    sro = src_ctrl_flat_get_sro_at_index (index);
    if (sro && sro_get_role_name (sro, &role, SR_INDEX_CONTAINER))
    {
	gchar *message;
	message = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), role);
	if (message)
	    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	g_free (message);
	SR_freeString (role);
    }
    else
	src_speech_say_message (_("no role available"));	
    sro_release_reference (sro);
}

static void
src_ctrl_position_sensor_action_logic (gint index)
{
    gint braille_position_sensor;
    
    if (!src_crt_sro)
	return;
	
    src_text_index = index >= 0 ? index : 0;

    braille_position_sensor = src_braille_get_position_sensor ();
    switch (braille_position_sensor)
    {
	case 0:
	    break;
	case 1: 
	    src_cmd_queue_add ("mouse goto current");
	    src_cmd_queue_process ();
	    break;
	case 2:
	    src_cmd_queue_add ("mouse goto current");
	    src_cmd_queue_add ("mouse left click");
	    src_cmd_queue_process ();
	    break;
	case 3:
	    src_cmd_queue_add ("mouse goto current");
	    src_cmd_queue_add ("mouse right click");
	    src_cmd_queue_process ();
	    break;
	case 4:
	    src_cmd_queue_add ("mouse goto current");
/*
	    src_cmd_queue_add ("mouse right click");
	    src_cmd_queue_add ("mouse left click");
*/
	    src_cmd_queue_process ();
	    break;
	case 5:
	    src_cmd_queue_add ("move caret");
	    src_cmd_queue_process ();
	    break;
	case 6:
	    src_cmd_queue_add ("show x coordinate");
	    src_cmd_queue_process ();
	    break;
	case 7:
	    src_cmd_queue_add ("show y coordinate");
	    src_cmd_queue_process ();
	    break;
	case 8:
	    src_cmd_queue_add ("font style");
	    src_cmd_queue_process ();
	    break;
	case 9:
	    src_cmd_queue_add ("font name");
	    src_cmd_queue_process ();
	    break;
	case 10:
	    src_cmd_queue_add ("font size");
	    src_cmd_queue_process ();
	    break;
	case 11:
	    sru_message ("sensor index 10 not implemented");
	    break;
	case 12:
	    src_cmd_queue_add ("text attributes");
	    src_cmd_queue_process ();
	    break;
	
	default:
	    sru_assert_not_reached ();
	    break;
    }	
}

#define src_ctrl_optical_sensor_action_flat src_ctrl_position_sensor_action_flat

static void
src_ctrl_optical_sensor_action_logic (gint index)
{
    gint braille_optical_sensor;
    if (!src_crt_sro)
	return;

    src_text_index = index >= 0 ? index : 0;

    braille_optical_sensor = src_braille_get_optical_sensor ();
    switch (braille_optical_sensor)
    {
	case 0:
	    break;
	case 1: 
	    src_cmd_queue_add ("mouse goto current");
	    src_cmd_queue_process ();
	    break;
	case 2:
	    src_cmd_queue_add ("mouse goto current");
	    src_cmd_queue_add ("mouse left click");
	    src_cmd_queue_process ();
	    break;
	case 3:
	    src_cmd_queue_add ("mouse goto current");
	    src_cmd_queue_add ("mouse right click");
	    src_cmd_queue_process ();
	    break;
	case 4:
	    src_cmd_queue_add ("mouse goto current");
/*
	    src_cmd_queue_add ("mouse right click");
	    src_cmd_queue_add ("mouse left click");
*/
	    src_cmd_queue_process ();
	    break;
	case 5:
	    src_cmd_queue_add ("move caret");
	    src_cmd_queue_process ();
	    break;
	case 6:
	    src_cmd_queue_add ("show x coordinate");
	    src_cmd_queue_process ();
	    break;
	case 7:
	    src_cmd_queue_add ("show y coordinate");
	    src_cmd_queue_process ();
	    break;
	case 8:
	    src_cmd_queue_add ("font style");
	    src_cmd_queue_process ();
	    break;
	case 9:
	    src_cmd_queue_add ("font name");
	    src_cmd_queue_process ();
	    break;
	case 10:
	    src_cmd_queue_add ("font size");
	    src_cmd_queue_process ();
	    break;
	case 11:
	    sru_message ("sensor index 10 not implemented");
	    break;
	case 12:
	    src_cmd_queue_add ("text attributes");
	    src_cmd_queue_process ();
	    break;
	default:
	    sru_assert_not_reached ();
	    break;
    }	
}

gboolean
src_ctrl_position_sensor_action (gint index)
{
    if (src_tracking_mode == SRC_MODE_FOCUS_TRACKING)
	src_ctrl_position_sensor_action_logic (index);
    else if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	src_ctrl_position_sensor_action_flat (index);
    else
	sru_assert_not_reached ();

    return TRUE;
}

gboolean
src_ctrl_optical_sensor_action (gint index)
{
    if (src_tracking_mode == SRC_MODE_FOCUS_TRACKING)
	src_ctrl_optical_sensor_action_logic (index);
    else if (src_tracking_mode == SRC_MODE_FLAT_REVIEW)
	src_ctrl_optical_sensor_action_flat (index);
    else
	sru_assert_not_reached ();

    return TRUE;
}
