/* srmag.c
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

#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "srmain.h"
#include "srmag.h"
#include "magxmlapi.h"
#include "libsrconf.h"
#include "srctrl.h"

#undef SRCORE_PANNING_DEBUG
#undef SRMAG_DEBUG

#define SRC_PANNING_REASON_CARET	1
#define SRC_PANNING_REASON_FOCUS	2
#define SORTED_ON_COORD(a, b, c) ( (a >= 0 && a < b) ? ( (b < c) ? TRUE:FALSE):FALSE) 

typedef struct
{
    gchar		 *id;
    gchar 		 *source;
    gchar 		 *target;
    
    gboolean	 	 cursor_state;
    gchar		 *cursor_name;
    gint		 cursor_size;
    gboolean 	 	 cursor_scale;	
    glong		 cursor_color;
    
    gboolean	 	 crosswire_state;
    gboolean	 	 crosswire_clip;
    gint		 crosswire_size;
    glong		 crosswire_color;
    gint		 border_width;
    glong		 border_color;
    gint 		 zp_left;
    gint 		 zp_top;
    gint 		 zp_right;
    gint 		 zp_bottom;

    	
    gdouble		 zoom_factor_x;
    gdouble     	 zoom_factor_y; 
    gboolean 	 	 zoom_factor_lock;
    /*	float 		 contrast*/	
    gboolean	 	 invert;	
    gchar		 *smoothing;

    gchar 		 *alignment_x;
    gchar 		 *alignment_y;
    gchar 		 *tracking;
    gchar 		 *mouse_tracking;
	
    gboolean	 	 visible;
} SRCMagnifier;

SRCMagnifier	      	*src_magnifier = NULL;
gboolean   		magnifier_initialized = 0;
gboolean	        panning = FALSE;
static SRObject     	*old_panning = NULL;
void src_magnifier_load_values (SRCMagnifier *magnifier);

void
src_magnifier_send (gchar *magoutput)
{
    static gboolean busy = FALSE;

    if (busy)
	return;
    busy = TRUE;
    if (magoutput)
    {
#ifdef SRMAG_DEBUG        
	fprintf (stderr,"\nXML:%s",magoutput);
#endif
	mag_xml_output (magoutput, 
			strlen (magoutput) );
    }
    busy = FALSE;
}



gboolean
src_magnifier_automatic_panning (gpointer data)
{
    static gpointer	last = NULL;
    static SRPoint 	point, start_point;  
    static gint reason;
    static SRRectangle	old_location_line;

    SRRectangle 	location_obj, location_line, location_char;
    SRObject 		*obj    = (SRObject *)data;    
    gchar 		*reason_, *magout = NULL;
    gboolean		stop_panning = FALSE;
    int 		step_x  = 2;/*TBR*/
    
    if (obj)
	sro_add_reference (obj);

    if (last != data)
    {
	if (sro_get_reason (obj, &reason_))
	{
	    if (strcmp (reason_, "focus:") == 0)
		reason = SRC_PANNING_REASON_FOCUS;
	    else
		reason = SRC_PANNING_REASON_CARET;
	    SR_freeString (reason_);
	}
	else
	{
	    sro_release_reference (obj);
	    return FALSE;
	}
    }
    

    if (reason == SRC_PANNING_REASON_CARET)
    {
	SRLong index;
	if (!sro_text_get_caret_offset (obj, 
					&index, 
					SR_INDEX_CONTAINER))
	{
	    sro_release_reference (obj);
	    return FALSE;
	}
	if (!sro_text_get_location_at_index (obj, 
					    index, 
					    &location_char, 
					    SR_INDEX_CONTAINER))
	{
	    sro_release_reference (obj);
	    return FALSE;
	}
    }
    
    if (!sro_get_location (obj, SR_COORD_TYPE_SCREEN, &location_obj, SR_INDEX_CONTAINER))
    {
	sro_release_reference (obj);
	return FALSE;
    }

    if (data != last)
    {
	if (reason == SRC_PANNING_REASON_CARET)
	{
	    point.x = location_char.x;
	    point.y = location_char.y;
	    start_point = point;
	}
	else
	{
	    point.x = location_obj.x;
	    point.y = location_obj.y;
	    start_point = point;
	    
/*	    if (location_line.width < 0)
	    {
		point.x += location_obj.width;
		point.y += location_obj.height;
	    }
*/	}
    }
/*    if (point.y > location_obj.y + location_obj.height)
    {
	sro_release_reference (obj);
	stop_panning = TRUE;
    }
*/    
    if (!sro_text_get_text_location_from_point (obj, 
						&point, 
						SR_COORD_TYPE_SCREEN,
						SR_TEXT_BOUNDARY_LINE, 
						&location_line,
						SR_INDEX_CONTAINER))
    {
        sro_release_reference (obj);
        return FALSE;
    }
    old_location_line = location_line;

    if (location_line.width > 0)
    {
	point.x = MAX (0, MAX (point.x, location_line.x));
	point.y = MAX (0, MAX (point.y, location_line.y));
    }
    else
    {
	point.x = MIN (point.x, location_line.x);
	point.y = MIN (point.y, location_line.y);
    }
    
    if (data == last)
    {
	if (location_line.width > 0)
	    point.x += step_x;
	else
	    point.x -= step_x;
    }
    last = data;

    if (location_line.width > 0)
    {
	if (location_obj.x + location_obj.width < point.x || 
	    location_line.x + location_line.width < point.x)
	{
	    point.x = location_obj.x;
	    point.y = location_line.y + location_line.height;
	    if (!sro_text_get_text_location_from_point (obj, 
						&point, 
						SR_COORD_TYPE_SCREEN,
						SR_TEXT_BOUNDARY_LINE, 
						&location_line,
						SR_INDEX_CONTAINER))
	    {
		sro_release_reference (obj);
		return FALSE;
	    }
	    if (location_line.y == old_location_line.y)
		stop_panning = TRUE;
	}
    }
    else
    {
	if (location_line.x + location_line.width > point.x ||
	    location_obj.x > point.x ||
	    0 > point.x)
	{
	    point.x = location_obj.x + location_obj.width;
	    point.y = location_line.y + location_line.height;
	    if (!sro_text_get_text_location_from_point (obj, 
						&point, 
						SR_COORD_TYPE_SCREEN,
						SR_TEXT_BOUNDARY_LINE, 
						&location_line,
						SR_INDEX_CONTAINER))
	    {
		sro_release_reference (obj);
		return FALSE;
	    }
	    if (location_line.y == old_location_line.y)
		stop_panning = TRUE;
	}
    }
    
#ifdef SRCORE_PANNING_DEBUG
    fprintf (stderr,"\n !!!!!!PANNING is running %d %d %d %d  %d %d",
		    location_line.x,
		    location_line.y,
		    location_line.x + location_line.width ,
		    location_line.y + location_line.height,
		    point.x,
		    point.y);
#endif
    if (stop_panning)
	point = start_point;
    magout = g_strdup_printf ("<MAGOUT><ZOOMER ID =\"%s\" tracking=\"%s\" ROILeft =\"%d\" ROITop =\"%d\" ROIWidth =\"%d\" ROIHeight=\"%d\"></ZOOMER></MAGOUT>",
				"generic_zoomer",
				"panning",
				point.x, point.y, point.x + 1, point.y + 1);

    src_magnifier_send (magout);
    g_free (magout);
    sro_release_reference (obj);
    if (stop_panning)
	return FALSE;
    return TRUE;
}


static gboolean
src_magnifier_panning (SRObject *obj)
{
    static guint 	panning_handle = 0;		
    
    if (panning_handle)
    {
	gtk_timeout_remove (panning_handle);
	panning_handle = 0;
    }
    
    if (old_panning)
	sro_release_reference (old_panning);
    old_panning = obj;
    if (old_panning)
	sro_add_reference (old_panning);
	
    if (panning && old_panning)
    {
	if (obj) panning_handle = gtk_timeout_add (100,
						   src_magnifier_automatic_panning,
						   old_panning);
    }
    return TRUE;
}


gboolean
src_magnifier_start_panning (SRObject *obj)
{
/*    SRObjectRoles role;*/

    sru_assert (obj);		
    
/*    sro_get_role (obj, &role, SR_INDEX_CONTAINER);
    if (role == SR_ROLE_TEXT)*/
    if (sro_is_text (obj, SR_INDEX_CONTAINER) )
    {
	gchar *reason_;
	if (sro_get_reason (obj, &reason_))
	{
	    if (strcmp (reason_, "focus:") == 0 ||
		strcmp (reason_, "object:text-caret-moved") == 0)
	    {	
		src_magnifier_panning (obj);
	    }
	    else
	    {
		src_magnifier_panning (NULL);
	    }
	    SR_freeString (reason_);
	}
    }
    else
    {
	src_magnifier_panning (NULL);
    }
    return TRUE;
}

gboolean
src_magnifier_stop_panning ()
{
    src_magnifier_panning (NULL);
    return TRUE;
}

SRCMagnifier*
src_magnifier_setting_new () 
{
    SRCMagnifier *new_magnifier = NULL;

    new_magnifier = g_new0 (SRCMagnifier, 1);

    return new_magnifier;
}

gboolean 
src_magnifier_init ()
{
    int rv = 0;

    rv = mag_xml_init (NULL);

    srconf_unset_key (SRCORE_MAGNIF_SENSITIVE, SRCORE_PATH);
    SET_SRCORE_CONFIG_DATA (SRCORE_MAGNIF_SENSITIVE, 
			    CFGT_BOOL, 
			    &rv);
    if ( rv )
    {
	    magnifier_initialized = TRUE;
    }
    else
    {
	    magnifier_initialized = FALSE;
	    fprintf (stderr, "\n** Magnifier initialization failed."
			     "\n   (Possible cause : 1. You don't have gnome-mag installed"
			     "\n                     2. GNOME_Magnifier.server file is missing)\n");
    }
    
    src_magnifier = src_magnifier_setting_new ();
    src_magnifier_load_values (src_magnifier);
    old_panning = NULL;
    return rv;    
}

void
src_magnifier_create ()
{

    if (magnifier_initialized)
    {
	    src_magnifier_config_get_settings ();
	    fprintf (stderr, "\n** Magnifier is running.\n");
	    	    
    }

}

void
src_magnifier_terminate ()
{
    mag_xml_terminate();
    fprintf (stderr, "\n** Magnifier terminated.\n");
    if (old_panning)
	sro_release_reference (old_panning);
    old_panning = NULL;
}


static gchar*
src_magnifier_create_mml(const SRCMagnifier *magnifier) 
{
    gchar *mml = NULL;

    if (magnifier == NULL) 			return NULL;
    if (magnifier->cursor_name == NULL) 	return NULL;
    if (magnifier->id == NULL) 			return NULL;

    if (magnifier->source == NULL) 		return NULL;
    if (magnifier->target == NULL) 		return NULL;

    if (magnifier->smoothing == NULL ) 		return NULL;
    if (magnifier->mouse_tracking == NULL)	return NULL;

    mml = g_strdup_printf ( "<MAGOUT cursor=\"%d\""
			    " CursorScale=\"%d\""			    
			    " CursorName=\"%s\""
			    " CursorSize=\"%d\""
			    " CursorColor=\"%ld\""
			    " crosswire=\"%d\""
			    " CrosswireClip=\"%d\""			    
			    " CrosswireSize=\"%d\""
			    " CrosswireColor=\"%ld\">"
			    " <ZOOMER ID=\"%s\" "
			    " source=\"%s\""
			    " target=\"%s\""
			    " BorderWidth=\"%d\""
			    " BorderColor=\"%ld\""
			    " ZPLeft=\"%d\""
			    " ZPTop=\"%d\""
			    " ZPWidth=\"%d\""
			    " ZPHeight=\"%d\""
			    " ZoomFactorX=\"%f\""
			    " ZoomFactorY=\"%f\""
			    " invert=\"%d\""
			    " smoothing=\"%s\""
			    " tracking=\"%s\""
			    " AlignmentX=\"%s\""
			    " AlignmentY=\"%s\""
			    " MouseTracking=\"%s\""
			    " visible=\"%d\"></ZOOMER></MAGOUT>",
			    magnifier->cursor_state,
			    magnifier->cursor_scale,
			    magnifier->cursor_name,
			    magnifier->cursor_size,
			    magnifier->cursor_color,

			    magnifier->crosswire_state,
			    magnifier->crosswire_clip,			    
			    magnifier->crosswire_size,
			    magnifier->crosswire_color,

			    magnifier->id,
			    magnifier->source,
			    magnifier->target,
			    magnifier->border_width,
			    magnifier->border_color,
			    magnifier->zp_left,
			    magnifier->zp_top,
			    magnifier->zp_right,
			    magnifier->zp_bottom,
			    
			    magnifier->zoom_factor_x,
			    magnifier->zoom_factor_y,

			    magnifier->invert,
			    magnifier->smoothing,
			    
			    
			    magnifier->tracking,
			    magnifier->alignment_x,
			    magnifier->alignment_y,
			    magnifier->mouse_tracking,

			    magnifier->visible);
    return mml;
}



void
src_magnifier_load_defaults (SRCMagnifier *magnifier) 
{
    g_assert (magnifier);

    magnifier->cursor_state = DEFAULT_MAGNIFIER_CURSOR		  ;
    magnifier->cursor_name  = g_strdup (DEFAULT_MAGNIFIER_CURSOR_NAME);
    magnifier->cursor_size  = DEFAULT_MAGNIFIER_CURSOR_SIZE		  ;
    magnifier->cursor_scale = DEFAULT_MAGNIFIER_CURSOR_SCALE	  ;
    magnifier->cursor_color = DEFAULT_MAGNIFIER_CURSOR_COLOR	  ;

    magnifier->crosswire_state = DEFAULT_MAGNIFIER_CROSSWIRE;
    magnifier->crosswire_clip  = DEFAULT_MAGNIFIER_CROSSWIRE_CLIP ;    
    magnifier->crosswire_size  = DEFAULT_MAGNIFIER_CROSSWIRE_SIZE ;
    magnifier->crosswire_color = DEFAULT_MAGNIFIER_CROSSWIRE_COLOR;

    magnifier->border_width	  = DEFAULT_MAGNIFIER_BORDER_WIDTH;
    magnifier->border_color   	  = DEFAULT_MAGNIFIER_BORDER_COLOR;
    magnifier->zp_left   	  = DEFAULT_MAGNIFIER_ZP_LEFT;
    magnifier->zp_top   	  = DEFAULT_MAGNIFIER_ZP_TOP;
    magnifier->zp_right   	  = DEFAULT_MAGNIFIER_ZP_WIDTH;
    magnifier->zp_bottom   	  = DEFAULT_MAGNIFIER_ZP_HEIGHT;

    magnifier->id = g_strdup ("generic_zoomer");

    magnifier->source = g_strdup (DEFAULT_MAGNIFIER_SOURCE);
    magnifier->target = g_strdup (DEFAULT_MAGNIFIER_TARGET);
    
    magnifier->zoom_factor_x    = DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY ;
    magnifier->zoom_factor_y    = DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY ;
    magnifier->zoom_factor_lock = DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK;

    magnifier->invert 	   	= DEFAULT_MAGNIFIER_INVERT;
    magnifier->smoothing	= g_strdup (DEFAULT_MAGNIFIER_SMOOTHING);

    magnifier->alignment_x 	= g_strdup (DEFAULT_MAGNIFIER_ALIGNMENT_X);
    magnifier->alignment_y 	= g_strdup (DEFAULT_MAGNIFIER_ALIGNMENT_Y);
    magnifier->tracking 	= g_strdup (DEFAULT_MAGNIFIER_TRACKING);
    magnifier->mouse_tracking   = g_strdup (DEFAULT_MAGNIFIER_MOUSE_TRACKING);

    magnifier->visible 	   	= DEFAULT_MAGNIFIER_VISIBLE;

}

void
src_magnifier_load_values (SRCMagnifier *magnifier) 
{
    gboolean   default_bool_val;
    gint       default_int_val;
    gdouble    default_float_val;
/*    gchar     *default_string_val = NULL;*/
    gint 	size_x,
		size_y;

    g_assert (magnifier);

    default_bool_val = DEFAULT_MAGNIFIER_CURSOR;
    srconf_get_data_with_default (MAGNIFIER_CURSOR, 
				       CFGT_BOOL, 
				       (gpointer)&magnifier->cursor_state, 
				       (gpointer)&default_bool_val, 
				       MAGNIFIER_CONFIG_PATH);

    srconf_get_data_with_default (MAGNIFIER_CURSOR_NAME, 
				      CFGT_STRING, 
				      &magnifier->cursor_name, 
				      (gpointer)DEFAULT_MAGNIFIER_CURSOR_NAME, 
				      MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_CURSOR_SIZE;
    srconf_get_data_with_default (MAGNIFIER_CURSOR_SIZE, 
				       CFGT_INT, 
				       (gpointer)&magnifier->cursor_size, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_bool_val = DEFAULT_MAGNIFIER_CURSOR_SCALE;
    srconf_get_data_with_default (MAGNIFIER_CURSOR_SCALE, 
				       CFGT_BOOL, 
				       (gpointer)&magnifier->cursor_scale, 
				       (gpointer)&default_bool_val, 
				       MAGNIFIER_CONFIG_PATH);
		    
    default_int_val = DEFAULT_MAGNIFIER_CURSOR_COLOR;
    srconf_get_data_with_default (MAGNIFIER_CURSOR_COLOR, 
				       CFGT_INT, 
				       (gpointer)&magnifier->cursor_color, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);


    default_int_val = DEFAULT_MAGNIFIER_CROSSWIRE;
    srconf_get_data_with_default (MAGNIFIER_CROSSWIRE, 
				       CFGT_BOOL, 
				       (gpointer)&magnifier->crosswire_state, 
				       (gpointer)&default_bool_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_bool_val = DEFAULT_MAGNIFIER_CROSSWIRE_CLIP;
    srconf_get_data_with_default (MAGNIFIER_CROSSWIRE_CLIP, 
				       CFGT_BOOL, 
				       (gpointer)&magnifier->crosswire_clip, 
				       (gpointer)&default_bool_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_CROSSWIRE_SIZE;
    srconf_get_data_with_default (MAGNIFIER_CROSSWIRE_SIZE, 
				       CFGT_INT, 
				       (gpointer)&magnifier->crosswire_size, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_CROSSWIRE_COLOR;
    srconf_get_data_with_default (MAGNIFIER_CROSSWIRE_COLOR, 
				       CFGT_INT, 
				       (gpointer)&magnifier->crosswire_color, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);


    default_int_val = DEFAULT_MAGNIFIER_BORDER_WIDTH;
    srconf_get_data_with_default (MAGNIFIER_BORDER_WIDTH, 
				       CFGT_INT, 
				       (gpointer)&magnifier->border_width, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_BORDER_COLOR;
    srconf_get_data_with_default (MAGNIFIER_BORDER_COLOR, 
				       CFGT_INT, 
				       (gpointer)&magnifier->border_color, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_DISPLAY_SIZE_X;
    srconf_get_data_with_default (MAGNIFIER_DISPLAY_SIZE_X, 
				       CFGT_INT, 
				       (gpointer)&size_x, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_DISPLAY_SIZE_Y;
    srconf_get_data_with_default (MAGNIFIER_DISPLAY_SIZE_Y, 
				       CFGT_INT, 
				       (gpointer)&size_y, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_ZP_LEFT;
    srconf_get_data_with_default (MAGNIFIER_ZP_LEFT, 
				       CFGT_INT, 
				       (gpointer)&magnifier->zp_left, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_ZP_TOP;
    srconf_get_data_with_default (MAGNIFIER_ZP_TOP, 
				       CFGT_INT, 
				       (gpointer)&magnifier->zp_top, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_ZP_WIDTH;
    srconf_get_data_with_default (MAGNIFIER_ZP_WIDTH, 
				       CFGT_INT, 
				       (gpointer)&magnifier->zp_right, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_int_val = DEFAULT_MAGNIFIER_ZP_HEIGHT;
    srconf_get_data_with_default (MAGNIFIER_ZP_HEIGHT, 
				       CFGT_INT, 
				       (gpointer)&magnifier->zp_bottom, 
				       (gpointer)&default_int_val, 
				       MAGNIFIER_CONFIG_PATH);

     
    if (!SORTED_ON_COORD (magnifier->zp_left,
			 magnifier->zp_right,
			 size_x) )
    {
	magnifier->zp_left = size_x/2;
	magnifier->zp_right = size_x - 1;
	srconf_set_data (MAGNIFIER_ZP_LEFT, 
		         CFGT_INT, 
			 (gpointer)&magnifier->zp_left, 
  		         MAGNIFIER_CONFIG_PATH);
	srconf_set_data (MAGNIFIER_ZP_WIDTH, 
		         CFGT_INT, 
			 (gpointer)&magnifier->zp_right, 
  		         MAGNIFIER_CONFIG_PATH);
    }			 

    if (!SORTED_ON_COORD (magnifier->zp_top,
			 magnifier->zp_bottom,
			 size_y) )
    {
	magnifier->zp_top = 0;
	magnifier->zp_bottom = size_y - 1;
	srconf_set_data (MAGNIFIER_ZP_TOP, 
		         CFGT_INT, 
			 (gpointer)&magnifier->zp_top, 
  		         MAGNIFIER_CONFIG_PATH);
	srconf_set_data (MAGNIFIER_ZP_HEIGHT, 
		         CFGT_INT, 
			 (gpointer)&magnifier->zp_bottom, 
  		         MAGNIFIER_CONFIG_PATH);
    }			 


    magnifier->id = g_strdup ("generic_zoomer");
/*    if (!srconf_get_data_with_default (MAGNIFIER_ID, 
				         CFGT_STRING, 
					 &magnifier->mag_id, 
					 (gpointer)DEFAULT_MAGNIFIER_ID, 
					 MAGNIFIER_CONFIG_PATH)
	 ) return NULL;
*/
    srconf_get_data_with_default (MAGNIFIER_SOURCE, 
				      CFGT_STRING, 
				      &magnifier->source, 
				      (gpointer)DEFAULT_MAGNIFIER_SOURCE, 
				      MAGNIFIER_CONFIG_PATH);
    
    srconf_get_data_with_default (MAGNIFIER_TARGET, 
				      CFGT_STRING, 
				      &magnifier->target, 
				      (gpointer)DEFAULT_MAGNIFIER_TARGET, 
				      MAGNIFIER_CONFIG_PATH);

    default_float_val = DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY;
    srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_X, 
				       CFGT_FLOAT, 
				       (gpointer)&magnifier->zoom_factor_x, 
				       (gpointer)&default_float_val, 
				       MAGNIFIER_CONFIG_PATH);

    srconf_get_data_with_default (MAGNIFIER_ZOOM_FACTOR_Y, 
				       CFGT_FLOAT, 
				       (gpointer)&magnifier->zoom_factor_y, 
				       (gpointer)&default_float_val, 
				       MAGNIFIER_CONFIG_PATH);
/*    magnifier->mag_zoom_factor_lock = DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK;*/


    default_bool_val = DEFAULT_MAGNIFIER_INVERT;
    srconf_get_data_with_default (MAGNIFIER_INVERT, 
				      CFGT_BOOL, 
				      (gpointer)&magnifier->invert, 
				      (gpointer)&default_bool_val, 
				      MAGNIFIER_CONFIG_PATH);
    
    srconf_get_data_with_default (MAGNIFIER_SMOOTHING, 
				      CFGT_STRING, 
				      &magnifier->smoothing, 
				      (gpointer)DEFAULT_MAGNIFIER_SMOOTHING, 
				      MAGNIFIER_CONFIG_PATH);

    srconf_get_data_with_default (MAGNIFIER_ALIGNMENT_X, 
				      CFGT_STRING, 
				      &magnifier->alignment_x, 
				      (gpointer)DEFAULT_MAGNIFIER_ALIGNMENT_X, 
				      MAGNIFIER_CONFIG_PATH);

    srconf_get_data_with_default (MAGNIFIER_ALIGNMENT_Y, 
				      CFGT_STRING, 
				      &magnifier->alignment_y, 
				      (gpointer)DEFAULT_MAGNIFIER_ALIGNMENT_Y, 
				      MAGNIFIER_CONFIG_PATH);

    srconf_get_data_with_default (MAGNIFIER_TRACKING, 
				      CFGT_STRING, 
				      &magnifier->tracking, 
				      (gpointer)DEFAULT_MAGNIFIER_TRACKING, 
				      MAGNIFIER_CONFIG_PATH);

    srconf_get_data_with_default (MAGNIFIER_MOUSE_TRACKING, 
				      CFGT_STRING, 
				      &magnifier->mouse_tracking, 
				      (gpointer)DEFAULT_MAGNIFIER_MOUSE_TRACKING, 
				      MAGNIFIER_CONFIG_PATH);

    default_bool_val = DEFAULT_MAGNIFIER_VISIBLE;
    srconf_get_data_with_default (MAGNIFIER_VISIBLE, 
				       CFGT_BOOL, 
				       (gpointer)&magnifier->visible, 
				       (gpointer)&default_bool_val, 
				       MAGNIFIER_CONFIG_PATH);

    default_bool_val = DEFAULT_MAGNIFIER_PANNING;
    srconf_get_data_with_default ( MAGNIFIER_PANNING,
				        CFGT_BOOL, 
					(gpointer)&panning, 
					(gpointer)&default_bool_val, 
					MAGNIFIER_CONFIG_PATH);

}

void
src_magnifier_setting_free (SRCMagnifier* magnifier) 
{
    g_assert (magnifier);
    
    g_free (magnifier->cursor_name);
    g_free (magnifier->id);
    g_free (magnifier->source);
    g_free (magnifier->target);
    g_free (magnifier->smoothing);
    g_free (magnifier->alignment_x);
    g_free (magnifier->alignment_y);
    g_free (magnifier->tracking);
    g_free (magnifier->mouse_tracking);
    
    g_free (magnifier);
}


gboolean
src_magnifier_config_set_defaults (SRCMagnifier *magnifier) 
{

    if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_LOCK,
			  CFGT_BOOL, 
			  (gpointer)&magnifier->zoom_factor_lock,
			  MAGNIFIER_CONFIG_PATH)
	) 
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
    
    if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
			  CFGT_FLOAT, 
			  (gpointer)&magnifier->zoom_factor_y,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
    
    if (!srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
			  CFGT_FLOAT, 
			  (gpointer)&magnifier->zoom_factor_x,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
    
    if (!srconf_set_data (MAGNIFIER_MOUSE_TRACKING,
			  CFGT_STRING, 
			  (gpointer)magnifier->mouse_tracking,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		
    if (!srconf_set_data (MAGNIFIER_ALIGNMENT_X, 
			  CFGT_STRING, 
			  (gpointer)magnifier->alignment_x, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_ALIGNMENT_Y, 
			  CFGT_STRING, 
			  (gpointer)magnifier->alignment_y, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_TRACKING, 
			  CFGT_STRING, 
			  (gpointer)magnifier->tracking, 
			  MAGNIFIER_CONFIG_PATH)
	) 
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_CURSOR,
			  CFGT_BOOL, 
			  (gpointer)&magnifier->cursor_state,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_CURSOR_NAME,
			  CFGT_STRING, 
			  (gpointer)magnifier->cursor_name,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_CURSOR_SIZE, 
			  CFGT_INT, 
			  (gpointer)&magnifier->cursor_size, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_CURSOR_SCALE, 
			  CFGT_BOOL, 
			  (gpointer)&magnifier->cursor_scale, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_CURSOR_COLOR, 
			  CFGT_INT, 
			  (gpointer)&magnifier->cursor_color, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_CROSSWIRE_SIZE, 
			  CFGT_INT, 
			  (gpointer)&magnifier->crosswire_size, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_CROSSWIRE,
			  CFGT_BOOL,
			  (gpointer)&magnifier->crosswire_state,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);	
		     
    if (!srconf_set_data (MAGNIFIER_CROSSWIRE_CLIP,
			  CFGT_BOOL,
			  (gpointer)&magnifier->crosswire_clip,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__); 
		     
    if (!srconf_set_data (MAGNIFIER_CROSSWIRE_COLOR, 
			  CFGT_INT, 
			  (gpointer)&magnifier->crosswire_color, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_BORDER_WIDTH, 
			  CFGT_INT, 
			  (gpointer)&magnifier->border_width, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_BORDER_COLOR, 
			  CFGT_INT, 
			  (gpointer)&magnifier->border_color, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_ZP_LEFT, 
			  CFGT_INT, 
			  (gpointer)&magnifier->zp_left, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_ZP_TOP, 
			  CFGT_INT, 
			  (gpointer)&magnifier->zp_top, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_ZP_WIDTH, 
			  CFGT_INT, 
			  (gpointer)&magnifier->zp_right, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_ZP_HEIGHT, 
			  CFGT_INT, 
			  (gpointer)&magnifier->zp_bottom, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_SOURCE, 
			  CFGT_STRING, 
			  (gpointer)magnifier->source, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_TARGET, 
			  CFGT_STRING, 
			  (gpointer)magnifier->target, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    if (!srconf_set_data (MAGNIFIER_PANNING,
			  CFGT_BOOL, 
			  (gpointer)&panning, 
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__); 	
		     
    if (!srconf_set_data (MAGNIFIER_INVERT,
			  CFGT_BOOL, 
			  (gpointer)&magnifier->invert,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_SMOOTHING,
			  CFGT_STRING, 
			  (gpointer)magnifier->smoothing,
			  MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);
		     
    if (!srconf_set_data (MAGNIFIER_VISIBLE, 
			  CFGT_BOOL, 
			  (gpointer)&magnifier->visible, 
			   MAGNIFIER_CONFIG_PATH)
	)
	sru_warning ("Cannot set config data! file: %s, line: %d",
		     __FILE__, __LINE__);

    return TRUE;
}


void
src_magnifier_config_get_defaults () 
{
    SRCMagnifier *magnifier = NULL;

    magnifier = src_magnifier_setting_new ();
    sru_assert (magnifier);
    	
    src_magnifier_load_defaults (magnifier);
    src_magnifier_config_set_defaults (magnifier);
    
    src_magnifier_setting_free (magnifier);
}

void
src_magnifier_config_get_settings ()
{
    SRCMagnifier *magnifier = NULL;
    gchar  	 *xml 	    = NULL;

    magnifier = src_magnifier_setting_new ();
    sru_assert (magnifier);
    
    src_magnifier_load_values (magnifier);

    xml = src_magnifier_create_mml (magnifier);
    src_magnifier_send (xml);

    src_magnifier_setting_free (magnifier);
    g_free (xml);
    xml = NULL;
}

SRCMagnifier*
src_magnifier_get_global_settings ()
{
    sru_assert (magnifier_initialized && src_magnifier);
    return src_magnifier;
}

gboolean
src_magnifier_set_cursor_state (gboolean cursor_state)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (cursor_state != magnifier->cursor_state)
    {
	magnifier->cursor_state = cursor_state;
	
	val = g_strdup_printf ("<MAGOUT cursor=\"%s\"></MAGOUT>",
		    	       cursor_state ? "true" : "false");
	
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_cursor_state_on_off ()
{
    SRCMagnifier *magnifier;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
        
    return src_magnifier_set_cursor_state (!magnifier->cursor_state);
}

gboolean
src_magnifier_set_cursor_name (gchar *cursor_name)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (cursor_name && 
		magnifier   && 
		magnifier->cursor_name);
    sru_assert (magnifier_initialized);
    
    if (strcmp(cursor_name, magnifier->cursor_name) != 0)
    {
	g_free (magnifier->cursor_name);
	magnifier->cursor_name = g_strdup (cursor_name);
	
	val = g_strconcat ("<MAGOUT CursorName=\"",
			   cursor_name,
			   "\"></MAGOUT>", NULL);
			   
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_cursor_size (gint cursor_size)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    cursor_size = (cursor_size > MAX_CURSOR_SIZE) ? MAX_CURSOR_SIZE : 
		  (cursor_size < MIN_CURSOR_SIZE) ? MIN_CURSOR_SIZE : 
		   cursor_size;

    if (cursor_size != magnifier->cursor_size)
    {
	magnifier->cursor_size = cursor_size;
    
	val = g_strdup_printf ("<MAGOUT CursorSize=\"%d\"></MAGOUT>",
			       cursor_size);
			       
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_adjust_cursor_size (gint offset)
{
    SRCMagnifier *magnifier;
    gint cursor_size;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    cursor_size = magnifier->cursor_size + offset;
    
    return src_magnifier_set_cursor_size (cursor_size);
}

gboolean
src_magnifier_set_cursor_scale (gboolean cursor_scale)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (cursor_scale != magnifier->cursor_scale)
    {
	magnifier->cursor_scale = cursor_scale;
	
	val = g_strdup_printf ("<MAGOUT CursorScale=\"%d\"></MAGOUT>",
			       cursor_scale);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_cursor_magnification_on_off ()
{
    SRCMagnifier *magnifier;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (magnifier->cursor_state)
    {
        return src_magnifier_set_cursor_scale (!magnifier->cursor_scale);
    }
    return FALSE;
}

gboolean
src_magnifier_set_cursor_color (gint32 cursor_color)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier && magnifier_initialized);
    
    if (cursor_color != magnifier->cursor_color)
    {
	magnifier->cursor_color = cursor_color;
	
	val = g_strdup_printf ("<MAGOUT CursorColor=\"%u\"></MAGOUT>",
			       cursor_color);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_crosswire_state (gboolean crosswire_state)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier && magnifier_initialized);
    
    if (crosswire_state != magnifier->crosswire_state)
    {
	magnifier->crosswire_state = crosswire_state;
	
	val = g_strdup_printf ("<MAGOUT crosswire=\"%d\"></MAGOUT>",
			       crosswire_state);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_crosswire_on_off ()
{	
    SRCMagnifier *magnifier;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);

    if (magnifier->cursor_state)
    {
	return src_magnifier_set_crosswire_state (!magnifier->crosswire_state);
    }
    return FALSE;
}


gboolean
src_magnifier_set_crosswire_clip (gboolean crosswire_clip)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier && magnifier_initialized);
    
    if (crosswire_clip != magnifier->crosswire_clip)
    {
	magnifier->crosswire_clip = crosswire_clip;
	
	val = g_strdup_printf ("<MAGOUT CrosswireClip=\"%s\"></MAGOUT>",
			       crosswire_clip ? "true" : "false");
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_crosswire_clip_on_off ()
{
    SRCMagnifier *magnifier;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (magnifier->cursor_state &&
	magnifier->crosswire_state)
    {
	return src_magnifier_set_crosswire_clip (!magnifier->crosswire_clip);
    }
    return FALSE;
}


gboolean
src_magnifier_set_crosswire_size (gint crosswire_size)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    crosswire_size = (crosswire_size < MIN_CROSSWIRE_SIZE) ? MIN_CROSSWIRE_SIZE : 
		     (crosswire_size > MAX_CROSSWIRE_SIZE) ? MAX_CROSSWIRE_SIZE :
		      crosswire_size;
		      
    if (crosswire_size != magnifier->crosswire_size)
    {
	
	magnifier->crosswire_size = crosswire_size;
	
	val = g_strdup_printf ("<MAGOUT CrosswireSize=\"%d\"></MAGOUT>",
			       crosswire_size);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_adjust_crosswire_size (gint offset)
{
    SRCMagnifier *magnifier;
    gint crosswire_size;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    crosswire_size = magnifier->crosswire_size + offset;
    
    return src_magnifier_set_crosswire_size (crosswire_size);
}

gboolean
src_magnifier_set_crosswire_color (gint32 crosswire_color)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (crosswire_color != magnifier->crosswire_color)
    {
	magnifier->crosswire_color = crosswire_color;
	
	val = g_strdup_printf ("<MAGOUT CrosswireColor=\"%u\"></MAGOUT>",
			       crosswire_color);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_zp_left (gint zp_left)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (zp_left != magnifier->zp_left)
    {
	magnifier->zp_left = zp_left;
	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZPLeft=\"%u\"></ZOOMER></MAGOUT>",
			       zp_left);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_zp_top (gint zp_top)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (zp_top != magnifier->zp_top)
    {
	magnifier->zp_top = zp_top;
	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZPTop=\"%u\"></ZOOMER></MAGOUT>",
			       zp_top);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_zp_width (gint zp_width)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (zp_width != magnifier->zp_right)
    {
	magnifier->zp_right = zp_width;
	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZPWidth=\"%u\"></ZOOMER></MAGOUT>",
			       zp_width);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_zp_height (gint zp_height)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);

    if (zp_height != magnifier->zp_bottom)
    {
	magnifier->zp_bottom = zp_height;
	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZPHeight=\"%u\"></ZOOMER></MAGOUT>",
			       zp_height);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_target (gchar *target)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);

    sru_assert (target	  && 
		magnifier && 
		magnifier->target);
    sru_assert (magnifier_initialized);
    
    if (strcmp(target, magnifier->target) != 0)
    {
	g_free (magnifier->target);
	magnifier->target = g_strdup (target);
	
	val = g_strconcat ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" target=\"",
			   target,
			   " \"></ZOOMER></MAGOUT>", NULL);
			   
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_source (gchar *source)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();

    sru_assert (source	  && 
		magnifier && 
		magnifier->source);
    sru_assert (magnifier_initialized);
    
    if (strcmp(source, magnifier->source) != 0)
    {
	g_free (magnifier->source);
	magnifier->source = g_strdup (source);
	
	val = g_strconcat ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" source=\"",
			   source,
			   " \"></ZOOMER></MAGOUT>", NULL);
			   
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_zoom_factor_x (gdouble zoom_factor_x)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (zoom_factor_x != magnifier->zoom_factor_x)
    {
	magnifier->zoom_factor_x = zoom_factor_x;
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZoomFactorX=\"%f\"></ZOOMER></MAGOUT>",
			       zoom_factor_x);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_increase_x_scale ()
{
    SRCMagnifier *magnifier;
    gint zoom_factor;
        
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    zoom_factor = magnifier->zoom_factor_x;
    
    zoom_factor++;
    
    zoom_factor = zoom_factor > MAX_ZOOM_FACTOR_X ? MAX_ZOOM_FACTOR_X : zoom_factor;
    
    if (!magnifier->zoom_factor_lock)
        src_magnifier_set_zoom_factor_x (zoom_factor);
    else
    {
	src_magnifier_set_zoom_factor_y (zoom_factor);
	src_magnifier_set_zoom_factor_x (zoom_factor);
    }
    return TRUE;
}

gboolean
src_magnifier_decrease_x_scale ()
{
    SRCMagnifier *magnifier;
    gint zoom_factor;
        
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    zoom_factor = magnifier->zoom_factor_x;
    
    zoom_factor--;
    
    zoom_factor = zoom_factor < MIN_ZOOM_FACTOR_X ? MIN_ZOOM_FACTOR_X : zoom_factor;
    
    if (!magnifier->zoom_factor_lock)
        src_magnifier_set_zoom_factor_x (zoom_factor);
    else
    {
    	src_magnifier_set_zoom_factor_y (zoom_factor);
	src_magnifier_set_zoom_factor_x (zoom_factor);
    }
    return TRUE;
}

gboolean
src_magnifier_set_zoom_factor_y (gdouble zoom_factor_y)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (zoom_factor_y != magnifier->zoom_factor_y)
    {
	magnifier->zoom_factor_y = zoom_factor_y;
	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZoomFactorY=\"%f\"></ZOOMER></MAGOUT>",
			       zoom_factor_y);
		
	g_assert (val);	       
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_increase_y_scale ()
{
    SRCMagnifier *magnifier;
    gint zoom_factor;
        
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    zoom_factor = magnifier->zoom_factor_y;
    
    zoom_factor++;
    
    zoom_factor = zoom_factor > MAX_ZOOM_FACTOR_Y ? MAX_ZOOM_FACTOR_Y : zoom_factor;
    
    if (!magnifier->zoom_factor_lock)
        src_magnifier_set_zoom_factor_y (zoom_factor);
    else
    {
    	src_magnifier_set_zoom_factor_y (zoom_factor);
	src_magnifier_set_zoom_factor_x (zoom_factor);
    }	
    return TRUE;
}

gboolean
src_magnifier_decrease_y_scale ()
{
    SRCMagnifier *magnifier;
    gint zoom_factor;
        
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    zoom_factor = magnifier->zoom_factor_y;
    
    zoom_factor--;
    
    zoom_factor = zoom_factor < MIN_ZOOM_FACTOR_Y ? MIN_ZOOM_FACTOR_Y : zoom_factor;
    
    if (!magnifier->zoom_factor_lock)
        src_magnifier_set_zoom_factor_y (zoom_factor);
    else
    {
    	src_magnifier_set_zoom_factor_y (zoom_factor);
	src_magnifier_set_zoom_factor_x (zoom_factor);
    }
    return TRUE;
}

gboolean
src_magnifier_set_zoom_factor_lock (gboolean zoom_factor_lock)
{
    SRCMagnifier *magnifier;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (zoom_factor_lock != magnifier->zoom_factor_lock)
    {
	magnifier->zoom_factor_lock = zoom_factor_lock;
	
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_zoom_factor_lock_on_off ()
{
    SRCMagnifier *magnifier;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    src_magnifier_set_zoom_factor_lock (!magnifier->zoom_factor_lock);
    
    if (magnifier->zoom_factor_lock)
	src_magnifier_set_zoom_factor_x (magnifier->zoom_factor_x);
	src_magnifier_set_zoom_factor_x (magnifier->zoom_factor_x);

    return TRUE;
}

gboolean
src_magnifier_set_invert_on_off (gboolean invert)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    if (invert != magnifier->invert)
    {
	magnifier->invert = invert;
	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" invert=\"%s\"></ZOOMER></MAGOUT>",
			       invert ? "true" : "false");
	
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean 
src_magnifier_set_invert_state ()
{
    SRCMagnifier *magnifier;
        
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    src_magnifier_set_invert_on_off (!magnifier->invert);
    
    return TRUE;
}

gboolean
src_magnifier_set_smoothing (gchar *smoothing)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    
    sru_assert (smoothing && 
		magnifier && 
		magnifier->smoothing);
    sru_assert (magnifier_initialized);
    
    if (strcmp (magnifier->smoothing, smoothing) != 0)
    {
	g_free (magnifier->smoothing);
	magnifier->smoothing = g_strdup (smoothing);
	
	val = g_strconcat ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" smoothing=\"",
			   smoothing,
			   " \"></ZOOMER></MAGOUT>",NULL);
	
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_mouse_tracking_mode (gchar *mouse_tracking)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();
    
    sru_assert (mouse_tracking && 
		magnifier      && 
		magnifier->mouse_tracking);
    sru_assert (magnifier_initialized);
    
    if (strcmp (magnifier->mouse_tracking, mouse_tracking) != 0)
    {
	g_free (magnifier->mouse_tracking);
	magnifier->mouse_tracking = g_strdup (mouse_tracking);
	
	val = g_strconcat("<MAGOUT><ZOOMER ID=\"generic_zoomer\"  MouseTracking=\"", 
			  mouse_tracking, 
			  "\"></ZOOMER></MAGOUT>", NULL);
	
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_panning_on_off (gboolean _panning)
{
    SRCMagnifier *magnifier;
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);

    if (_panning != panning)
    {
	panning = _panning;
	
	if (!panning)
	    src_magnifier_stop_panning (0);
	
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_panning_state()
{
    return src_magnifier_set_panning_on_off (!panning);
}

gboolean
src_magnifier_set_alignment_x (gchar *alignment_x)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();

    sru_assert (alignment_x && 
	        magnifier   &&
		magnifier->alignment_x);
    sru_assert (magnifier_initialized);
    
    if (strcmp (magnifier->alignment_x, alignment_x) != 0)
    {
	g_free (magnifier->alignment_x);
	magnifier->alignment_x = g_strdup (alignment_x);
	
	val = g_strdup_printf("<MAGOUT><ZOOMER ID=\"generic_zoomer\"  AlignmentX=\"%s\"></ZOOMER></MAGOUT>",
			      alignment_x);	
	
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_alignment_y (gchar *alignment_y)
{
    SRCMagnifier *magnifier;
    gchar *val = NULL;
    
    magnifier = src_magnifier_get_global_settings ();

    sru_assert (alignment_y && 
		magnifier   && 
		magnifier->alignment_y);
    sru_assert (magnifier_initialized);
    
    if (strcmp (magnifier->alignment_y, alignment_y) != 0)
    {
	g_free (magnifier->alignment_y);
	magnifier->alignment_y = g_strdup (alignment_y);
	
	val = g_strdup_printf("<MAGOUT><ZOOMER ID=\"generic_zoomer\"  AlignmentY=\"%s\"></ZOOMER></MAGOUT>",
			      alignment_y);	
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	return TRUE;
    }
    return FALSE;
}

gboolean
src_magnifier_set_split_screen_horizontal (gboolean horiz_split)
{
    gint zp_left, zp_top, zp_height, zp_width;
    gchar *val = NULL;
    SRCMagnifier *magnifier;
    
    
    magnifier = src_magnifier_get_global_settings ();
    sru_assert (magnifier);
    sru_assert (magnifier_initialized);
    
    zp_left = zp_top = 0;
    zp_width  = gdk_screen_width()-1;
    zp_height = gdk_screen_height()/2;
        
    if (horiz_split)
    {
    
	magnifier->zp_left   = zp_left;
	srconf_set_data (MAGNIFIER_ZP_LEFT, 
		         CFGT_INT, 
			 (gpointer)&magnifier->zp_left, 
  		         MAGNIFIER_CONFIG_PATH);
	
	magnifier->zp_top    = zp_top;
	srconf_set_data (MAGNIFIER_ZP_TOP, 
		         CFGT_INT, 
			 (gpointer)&magnifier->zp_top, 
  		         MAGNIFIER_CONFIG_PATH);
	
	magnifier->zp_right  = zp_width;
	srconf_set_data (MAGNIFIER_ZP_WIDTH, 
		         CFGT_INT, 
			 (gpointer)&magnifier->zp_right, 
  		         MAGNIFIER_CONFIG_PATH);
	
	magnifier->zp_bottom = zp_height;
	srconf_set_data (MAGNIFIER_ZP_HEIGHT, 
		         CFGT_INT, 
			 (gpointer)&magnifier->zp_bottom, 
  		         MAGNIFIER_CONFIG_PATH);
	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZPLeft=\"%u\"></ZOOMER></MAGOUT>",
			       zp_left);
    
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);

	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZPTop=\"%u\"></ZOOMER></MAGOUT>",
			       zp_top);

	g_assert (val);
	src_magnifier_send (val);
	g_free (val);

	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZPWidth=\"%u\"></ZOOMER></MAGOUT>",
			       zp_width);
	
	g_assert (val);
    	src_magnifier_send (val);
	g_free (val);
	
	val = g_strdup_printf ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" ZPHeight=\"%u\"></ZOOMER></MAGOUT>",
			       zp_height);
	
	g_assert (val);
	src_magnifier_send (val);
	g_free (val);
	
    }
    return TRUE;
}
