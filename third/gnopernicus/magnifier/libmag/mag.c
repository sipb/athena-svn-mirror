/* mag.c
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>/*4 sleep only*/
#include <gdk/gdk.h>
#include "mag_ctrl.h"
#include "mag.h"
#include "magxmlapi.h"
#include "libsrconf.h"

#define ZOOM_FACTOR_X 2.0
#define ZOOM_FACTOR_Y 2.0
#define INVERT	    FALSE
#define VISIBLE	    TRUE

#define ROI_LEFT     0
#define ROI_TOP      0
#define ROI_WIDTH    0
#define ROI_HEIGHT   0

#define ZP_LEFT      512
#define ZP_TOP       0
#define ZP_WIDTH     1024
#define ZP_HEIGHT    768

#define BORDER_WIDTH 10
#define BORDER_COLOR  1

#define CURSOR_SIZE		32
#define CURSOR_SIZE_INVALID	0
#define CROSSWIRE_SIZE		2
#define CURSOR_ZOOM_FACTOR	2.0

#define NOT_OUT			(0)  

#define OUT_LEFT		(1<<0)  
#define OUT_RIGHT		(1<<1)
#define OUT_TOP			(1<<2)
#define OUT_BOTTOM		(1<<3)
#define OUT_X 			(OUT_LEFT | OUT_RIGHT)
#define OUT_Y 			(OUT_TOP  | OUT_BOTTOM)
	
/* __ GLOBALS _________________________________________________________________*/

GHashTable* mag_zoomers_hash_table;
GNOME_Magnifier_Magnifier magnifier;
/* regarding the cursor :
this will be the same for the entire collention of zoomers, so for the entire schema
schema = a collection of zoomers
*/
gboolean zoom_factor_changed = TRUE;
gboolean zoom_factor_was_changed = FALSE; /* does the same thing as zoom_factor_changed but only for the zoom factor*/

static gboolean cursor		= TRUE;
static gboolean cursor_scale	= TRUE;
static gboolean crosswire	= TRUE;

static gchar	*cursor_name	= NULL;
static int	cursor_size	= 0;
static guint32	cursor_color	= 0    ;/*color as AlphaRedGreenBlue*/
static float	cursor_zoom_factor = 1;

int      	crosswire_size  = 0    ;/*thickness of crosswire cursor*/
guint32		crosswire_color = 0    ;/*color as AlphaRedGreenBlue*/
gboolean	crosswire_clip  = FALSE;/*whenever to insert the cursor over the crosswire or not*/

MagRectangle display_size;
MagRectangle source_rect;
static MagRectangle rect;
static MagRectangle mouse_tracked_roi;

static MagRectangle roi_viewport;
extern MagRectangle zp_rect;

/*__DEBUG FUNCION*/
void 
mag_zoomer_content(MagZoomer *mag_zoomer)
{
    fprintf (stderr, "\nZoomer ID = %s:", mag_zoomer->ID);
    fprintf (stderr, "\nparams:");

    fprintf (stderr, "\n\tzoom_factor_x = %4g", mag_zoomer->params.zoom_factor_x);
    fprintf (stderr, "\n\tzoom_factor_y = %4g", mag_zoomer->params.zoom_factor_y);
    fprintf (stderr, "\n\tinvert        = %4d", mag_zoomer->params.invert);    
    fprintf (stderr, "\n\tcursor        = %4d", cursor);
    fprintf (stderr, "\n\tvisible       = %4d", mag_zoomer->params.visible);    

    fprintf (stderr, "\nROI:");
    fprintf (stderr, "\n\troi left     = %4ld", mag_zoomer->region_of_interest.left);
    fprintf (stderr, "\n\troi top      = %4ld", mag_zoomer->region_of_interest.top );   
    fprintf (stderr, "\n\troi width    = %4ld", mag_zoomer->region_of_interest.width ); 
    fprintf (stderr, "\n\troi height   = %4ld", mag_zoomer->region_of_interest.height );

    fprintf(stderr, "\nextens:");
    fprintf(stderr, "\n\tzp left      = %4ld",mag_zoomer->zoomer_placement.extents.left); 
    fprintf(stderr, "\n\tzp top       = %4ld",mag_zoomer->zoomer_placement.extents.top);
    fprintf(stderr, "\n\tzp widt      = %4ld",mag_zoomer->zoomer_placement.extents.width);  
    fprintf(stderr, "\n\tzp height    = %4ld",mag_zoomer->zoomer_placement.extents.height);

    fprintf(stderr, "\n\tborder width = %4d",mag_zoomer->zoomer_placement.border.width );
    fprintf(stderr, "\n\tborder color = %4u\n",mag_zoomer->zoomer_placement.border.color );

    fprintf(stderr, "\n\tcursor name  = %s",cursor_name);
    fprintf(stderr, "\n\tcursor size  = %4d\n",cursor_size);
    fprintf(stderr, "\n\tcursor scale = %4g\n",cursor_zoom_factor);
    

    sleep(2);

}

#if 0
/* __ MAG CB ___________ ______________________________________________________*/
static void 
call_mag_event_cb (char *buffer)
{
    SREvent *event = NULL;
    if (!buffer) return;
    if (strlen(buffer) == 0) return;
    if ( (event = sre_new() ) != NULL)
    {
	event->data = g_strdup (buffer);
	event->type = SR_EVENT_MAG;
	event->data_destructor = g_free;
	if (event->data)
	{
	    if (mag_event_cb != NULL) mag_event_cb (event, 0);
	}
	sre_release_reference (event);
    }
    
}
#endif

/* __ MAG ZOOMER METHODS ______________________________________________________*/

MagZoomer * 
mag_zoomer_new ()
{
	MagZoomer *mag_zoomer;
	mag_zoomer = g_malloc0 (sizeof (MagZoomer));
	
	mag_zoomer_init(mag_zoomer);
#ifdef MAG_DEBUG
	mag_zoomer_content(mag_zoomer);
#endif	
	return mag_zoomer;
}

void 
mag_zoomer_init (MagZoomer *mag_zoomer)
{
	if (mag_zoomer->ID) g_free (mag_zoomer->ID);	
	mag_zoomer->ID = g_strdup ("generic_zoomer");

	mag_zoomer->params.zoom_factor_x = ZOOM_FACTOR_X;
	mag_zoomer->params.zoom_factor_y = ZOOM_FACTOR_Y;
	mag_zoomer->params.invert        = INVERT;
	mag_zoomer->params.visible       = VISIBLE;
	mag_zoomer->params.tracking	 = MAG_TRACKING_FOCUS;
	mag_zoomer->params.mouse_tracking= MAG_MOUSE_TRACKING_PUSH;
	mag_zoomer->params.alignment.x   = MAG_ALIGN_AUTO;
	mag_zoomer->params.alignment.y   = MAG_ALIGN_AUTO;


	mag_zoomer->region_of_interest.left   = ROI_LEFT;
	mag_zoomer->region_of_interest.top    = ROI_TOP;
	mag_zoomer->region_of_interest.width  = ROI_WIDTH;
	mag_zoomer->region_of_interest.height = ROI_HEIGHT;

	mag_zoomer->zoomer_placement.extents.left   = rect.left;
	mag_zoomer->zoomer_placement.extents.top    = rect.top;
	mag_zoomer->zoomer_placement.extents.width  = rect.width;
	mag_zoomer->zoomer_placement.extents.height = rect.height;

	mag_zoomer->zoomer_placement.border.width   = BORDER_WIDTH;
	mag_zoomer->zoomer_placement.border.color   = BORDER_COLOR;
}

void 
mag_zoomer_free (MagZoomer *mag_zoomer)
{
	/* free the malloc fields...make clean */
	if (mag_zoomer)
	{
	    if (mag_zoomer->params.smoothing_type)
	    {
		g_free (mag_zoomer->params.smoothing_type);
		mag_zoomer->params.smoothing_type = NULL;
	    }
	    if (mag_zoomer->ID)
	    {
		g_free (mag_zoomer->ID);
		mag_zoomer->ID = NULL;
	    }
	    g_free (mag_zoomer);
	    mag_zoomer = NULL;
	}
}

MagZoomer *
mag_zoomer_copy (MagZoomer *orig_mag_zoomer)
{
	MagZoomer *mag_zoomer;
	
	mag_zoomer = g_malloc0 (sizeof (MagZoomer));
	memcpy (mag_zoomer, orig_mag_zoomer, sizeof (MagZoomer));

	mag_zoomer->ID = g_strdup (orig_mag_zoomer->ID);	

	mag_zoomer->params.smoothing_type = g_strdup (orig_mag_zoomer->params.smoothing_type);	

	return mag_zoomer;
}

MagZoomer * 
mag_zoomer_dup (MagZoomer *orig_mag_zoomer)
{
	return (MagZoomer *)orig_mag_zoomer;
}

gboolean 
mag_zoomer_find (MagZoomer **mag_zoomer)
{
	
	MagZoomer *zoomer_found = NULL;
	gboolean   rv     	= FALSE;
	
	
	if (*mag_zoomer && (*mag_zoomer)->ID)
	{
    	/* search zoomer in the container first */
		zoomer_found = mag_get_zoomer ( (*mag_zoomer)->ID);			    
		if (zoomer_found)
		{
			/* zoomer found */
			 mag_zoomer_free (*mag_zoomer);
			 *mag_zoomer = (MagZoomer *)mag_zoomer_dup (zoomer_found);
			 rv = TRUE;
		}						   
	}						
	return rv;
}

void 
mag_zoomer_set_id (MagZoomer *mag_zoomer, 
		   gchar     *id)
{
	if (mag_zoomer->ID) 
	{
	    g_free (mag_zoomer->ID);	
	    mag_zoomer->ID = NULL;
	}
	mag_zoomer->ID = g_strdup (id);
}

void 
mag_zoomer_set_source (MagZoomer *mag_zoomer, 
			gchar     *source)
{
	if (mag_zoomer->source) 
	{
	    g_free (mag_zoomer->source);	
	    mag_zoomer->source = NULL;
	}
	mag_zoomer->source = g_strdup (source);

	magnifier_set_source_screen (magnifier, 
			    	     mag_zoomer->source);
}

void 
mag_zoomer_set_target (MagZoomer *mag_zoomer, 
			gchar     *target)
{
	if (mag_zoomer->target) 
	{
	    g_free (mag_zoomer->target);	
	    mag_zoomer->target = NULL;
	}
	mag_zoomer->target = g_strdup (target);

	magnifier_set_target_screen (magnifier, 
			    	     mag_zoomer->target);
}

/* __ MAG ZOOMER PARAMS METHODS ________________________________________________*/
void 
mag_zoomer_set_params_zoom_factor (MagZoomer *mag_zoomer, 
				   gchar     *zoom_factor_x, 
				   gchar     *zoom_factor_y)
{
	/* 
	    if zoom_factor_x[y] is NULL the old value is not updated
	    zoom_factor_x = "+2" -> increase x_factor with 2
	    zoom_factor_x = "-2" -> decrease y_factor with 2 
	*/
	if (zoom_factor_x)
	{
	/*check if the zoom factor was changed - 
	needed for the focus/mouse tracking logic*/
		zoom_factor_changed = TRUE;
		zoom_factor_was_changed = TRUE;
	    if ( (strncasecmp ( "+", zoom_factor_x, 1) != 0) &&
		 (strncasecmp ( "-", zoom_factor_x, 1) != 0)	     
	       ) 
		    mag_zoomer->params.zoom_factor_x = 0;
	    mag_zoomer->params.zoom_factor_x += atof(zoom_factor_x);
	}
	
	if (zoom_factor_y)
	{
	/*check if the zoom factor was changed - 
	needed for the focus/mouse tracking logic*/
	    	zoom_factor_changed = TRUE;
		zoom_factor_was_changed = TRUE;
	    if ( (strncasecmp ( "+", zoom_factor_y, 1) != 0) &&
		 (strncasecmp ( "-", zoom_factor_y, 1) != 0)	     
	       ) 
		    mag_zoomer->params.zoom_factor_y = 0;
	    mag_zoomer->params.zoom_factor_y += atof(zoom_factor_y);
	}
	
	if (mag_zoomer->params.zoom_factor_x < 1) mag_zoomer->params.zoom_factor_x = 1;
	if (mag_zoomer->params.zoom_factor_y < 1) mag_zoomer->params.zoom_factor_y = 1;

	magnifier_set_magnification (magnifier, 
				     0, 
				     mag_zoomer->params.zoom_factor_x,
				     mag_zoomer->params.zoom_factor_y);
}

void 
mag_zoomer_set_params_invert (MagZoomer *mag_zoomer, 
			      gchar     *invert)
{
	
#ifdef MAG_DEBUG
	fprintf (stderr, "Invertion becomes %s", invert);
#endif
	if ( (g_strcasecmp(invert, "yes")  == 0) ||
	     (g_strcasecmp(invert, "true") == 0) ||
	     (g_strcasecmp(invert, "1")    == 0))
	{
		mag_zoomer->params.invert = TRUE;
	}
	else
	{	
		mag_zoomer->params.invert = FALSE;
	}
	magnifier_set_invert (magnifier, 0, (int)mag_zoomer->params.invert);	
}

void 
mag_zoomer_set_params_contrast (MagZoomer *mag_zoomer, 
			        gchar     *contrast)
{
	mag_zoomer->params.contrast = atof (contrast);	
}

gboolean
check_for_focus_tracking_none  (MagZoomer *mag_zoomer)
{
    if((mag_zoomer->params.alignment.x == MAG_ALIGN_NONE) &&
       (mag_zoomer->params.alignment.y == MAG_ALIGN_NONE) && zoom_factor_was_changed)
       return TRUE;
    else
	return FALSE;
}

void 
mag_zoomer_set_params_tracking (MagZoomer *mag_zoomer, 
			    	gchar     *tracking)
{
	if  (g_strcasecmp(tracking, "focus")  == 0) 
	{
	    if (check_for_focus_tracking_none (mag_zoomer))
		mag_zoomer->params.tracking = MAG_TRACKING_MOUSE;	
	    else
	    	mag_zoomer->params.tracking = MAG_TRACKING_FOCUS;	
	}
	else
	{
	    if  (g_strcasecmp(tracking, "mouse")  == 0) 
	    {
		mag_zoomer->params.tracking = MAG_TRACKING_MOUSE;
		mouse_tracked_roi.left = mag_zoomer->region_of_interest.left;
		mouse_tracked_roi.top = mag_zoomer->region_of_interest.top;
		mouse_tracked_roi.width = mag_zoomer->region_of_interest.width;
		mouse_tracked_roi.height = mag_zoomer->region_of_interest.height;
	    }
	    else
	    {
		if  (g_strcasecmp(tracking, "panning")  == 0) 
		{
		    mag_zoomer->params.tracking = MAG_TRACKING_PANNING;
		}
		else
		{
		    mag_zoomer->params.tracking = MAG_TRACKING_NONE;
		}
	    }
	}
}

void 
mag_zoomer_set_params_mouse_tracking (MagZoomer *mag_zoomer, 
			    	      gchar     *mouse_tracking)
{
	if  (g_strcasecmp(mouse_tracking, "mouse-push")  == 0) 
	{
	    mag_zoomer->params.mouse_tracking = MAG_MOUSE_TRACKING_PUSH;	
	}
	else
	{
	    if  (g_strcasecmp(mouse_tracking, "mouse-centered")  == 0) 
	    {
		mag_zoomer->params.mouse_tracking = MAG_MOUSE_TRACKING_CENTERED;
	    }
	    else
	    {
		if  (g_strcasecmp(mouse_tracking, "mouse-proportional")  == 0) 
		{
		    mag_zoomer->params.mouse_tracking = MAG_MOUSE_TRACKING_PROPORTIONAL;	
		}
		else
		{
		    mag_zoomer->params.mouse_tracking = MAG_MOUSE_TRACKING_NONE;
		}
	    }
	}
}

void 
mag_zoomer_set_params_alignment (MagZoomer *mag_zoomer, 
			         gchar     *alignment_x,
				 gchar	   *alignment_y)
{
	if (alignment_x) 
	{
	    if  (g_strcasecmp(alignment_x, "none")  == 0) 
	    {
		mag_zoomer->params.alignment.x = MAG_ALIGN_NONE;	
	    }
	    else
	    {
		if  (g_strcasecmp(alignment_x, "centered")  == 0) 
		{
		    mag_zoomer->params.alignment.x = MAG_ALIGN_CENTER;
		}
		else
		{
		    if  (g_strcasecmp(alignment_x, "min")  == 0) 
		    {
	    	        mag_zoomer->params.alignment.x = MAG_ALIGN_MIN;		
		    }
		    else
		    {
			if  (g_strcasecmp(alignment_x, "max")  == 0) 
			{
	    		    mag_zoomer->params.alignment.x = MAG_ALIGN_MAX;		
			}
			else
			{
	    		    mag_zoomer->params.alignment.x = MAG_ALIGN_AUTO;		
			}
		    }
		}
	    }
	}

	if (alignment_y) 
	{
	    if  (g_strcasecmp(alignment_y, "none")  == 0) 
	    {
		mag_zoomer->params.alignment.y = MAG_ALIGN_NONE;	
	    }
	    else
	    {
		if  (g_strcasecmp(alignment_y, "centered")  == 0) 
		{
		    mag_zoomer->params.alignment.y = MAG_ALIGN_CENTER;
		}
		else
		{
		    if  (g_strcasecmp(alignment_y, "min")  == 0) 
		    {
	    	        mag_zoomer->params.alignment.y = MAG_ALIGN_MIN;		
		    }
		    else
		    {
			if  (g_strcasecmp(alignment_y, "max")  == 0) 
			{
	    		    mag_zoomer->params.alignment.y = MAG_ALIGN_MAX;		
			}
			else
			{
	    		    mag_zoomer->params.alignment.y = MAG_ALIGN_AUTO;		
			}
		    }
		}
	    }
	}
}

void 
mag_zoomer_set_params_visible (MagZoomer *mag_zoomer, 
			       gchar     *visible)
{
	
	if ( (g_strcasecmp(visible, "yes")  == 0) ||
	     (g_strcasecmp(visible, "true") == 0) ||
	     (g_strcasecmp(visible, "1")    == 0))
	{
		mag_zoomer->params.visible = TRUE;
	}
	else
	{	
		mag_zoomer->params.visible = FALSE;
	}
}


void 
mag_zoomer_set_params_smoothing_alg (MagZoomer *mag_zoomer, 
				     gchar     *smoothing_algorithm)
{
	if (mag_zoomer->params.smoothing_type) 
	{
	    g_free (mag_zoomer->params.smoothing_type);	
	    mag_zoomer->params.smoothing_type = NULL;
	}

	mag_zoomer->params.smoothing_type = g_strdup (smoothing_algorithm);
	
	magnifier_set_smoothing_type (magnifier,
				      0,
				      mag_zoomer->params.smoothing_type);
}

void 
mag_zoomer_set_ROI_left (MagZoomer *mag_zoomer, 
			 gchar     *ROILeft)
{
	int roi_left = atoi (ROILeft);

	if (check_for_focus_tracking_none (mag_zoomer))
	    roi_left = mouse_tracked_roi.left;

        mag_zoomer->region_of_interest.left = roi_left;
}	

void 
mag_zoomer_set_ROI_top (MagZoomer *mag_zoomer, 
			gchar     *ROITop)
{
	int roi_top = atoi (ROITop);

	if (check_for_focus_tracking_none (mag_zoomer))
	    roi_top = mouse_tracked_roi.top;
        
	mag_zoomer->region_of_interest.top = roi_top;
	
}

void 
mag_zoomer_set_ROI_width (MagZoomer *mag_zoomer, 
			  gchar     *ROIWidth)
{
	int roi_width = atoi (ROIWidth);
	
	if (check_for_focus_tracking_none (mag_zoomer))
	    roi_width = mouse_tracked_roi.width;
        
	mag_zoomer->region_of_interest.width = roi_width;
}

void 
mag_zoomer_set_ROI_height (MagZoomer *mag_zoomer, 
			   gchar     *ROIHeight)
{
	int roi_height = atoi (ROIHeight);
	
	if (check_for_focus_tracking_none (mag_zoomer))
	{
	    roi_height = mouse_tracked_roi.height;
	    zoom_factor_was_changed = FALSE;
	}
        
	mag_zoomer->region_of_interest.height = roi_height;
}

/* __ MAG ZOOMER ZP METHODS ____________________________________________________*/
void 
mag_zoomer_set_ZP_extents_left (MagZoomer *mag_zoomer, 
				gchar     *ZPLeft)
{
    MagRectangle temp_rect;
    int zp_left;
    
    if (!ZPLeft)
	return;

    zp_left = atoi (ZPLeft);
    
    zp_rect.left = zp_left;    
    
    zoom_factor_changed = TRUE;
    
    if ( zp_left > display_size.left)
	(mag_zoomer->zoomer_placement).extents.left = zp_left;
    else
	(mag_zoomer->zoomer_placement).extents.left = display_size.left;    

    magnifier_set_target (magnifier,
			 &((mag_zoomer->zoomer_placement).extents));
    /* get_source has to be called every time set_target is called , 
    because the source changes*/			 
    magnifier_get_source (magnifier,
			      &source_rect);
			 
    temp_rect.left = temp_rect.top = 0;
    temp_rect.width = mag_zoomer->zoomer_placement.extents.width - mag_zoomer->zoomer_placement.extents.left;
    temp_rect.height = mag_zoomer->zoomer_placement.extents.height - mag_zoomer->zoomer_placement.extents.top;

    magnifier_resize_region (magnifier,
			     0,
			     &temp_rect);			    
}

void mag_zoomer_set_ZP_extents_top (MagZoomer *mag_zoomer, 
				    gchar     *ZPTop)
{
    MagRectangle temp_rect;
    int zp_top;
    
    if (!ZPTop)
	return;

    zp_top = atoi (ZPTop);
    
    zp_rect.top = zp_top;
    
    zoom_factor_changed = TRUE;

    if ( zp_top > display_size.top)
	mag_zoomer->zoomer_placement.extents.top = zp_top;
    else
	mag_zoomer->zoomer_placement.extents.top = display_size.top;    

    magnifier_set_target (magnifier,
			 &((mag_zoomer->zoomer_placement).extents));
			 
    magnifier_get_source (magnifier,
			      &source_rect);

    temp_rect.left = temp_rect.top = 0;
    temp_rect.width = mag_zoomer->zoomer_placement.extents.width - mag_zoomer->zoomer_placement.extents.left;
    temp_rect.height = mag_zoomer->zoomer_placement.extents.height - mag_zoomer->zoomer_placement.extents.top;

    magnifier_resize_region (magnifier,
			     0,
			     &temp_rect);
}

void 
mag_zoomer_set_ZP_extents_width (MagZoomer *mag_zoomer, 
				 gchar     *ZPWidth)
{
    MagRectangle temp_rect;
    int zp_width;
    
    if (!ZPWidth)
	return;
	
    zp_width = atoi (ZPWidth);
    
    zp_rect.width = zp_width;

    zoom_factor_changed = TRUE;

    if ( zp_width < display_size.width)
        mag_zoomer->zoomer_placement.extents.width = zp_width;
    else
        mag_zoomer->zoomer_placement.extents.width = display_size.width;	

    magnifier_set_target (magnifier,
			 &((mag_zoomer->zoomer_placement).extents));
			 
    magnifier_get_source (magnifier,
			      &source_rect);

    temp_rect.left = temp_rect.top = 0;
    temp_rect.width = mag_zoomer->zoomer_placement.extents.width - mag_zoomer->zoomer_placement.extents.left;
    temp_rect.height = mag_zoomer->zoomer_placement.extents.height - mag_zoomer->zoomer_placement.extents.top;

    magnifier_resize_region (magnifier,
			     0,
			     &temp_rect);
}

void 
mag_zoomer_set_ZP_extents_height (MagZoomer *mag_zoomer, 
				  gchar     *ZPHeight)
{
    MagRectangle temp_rect;
    int zp_height;
    
    if (!ZPHeight)
	return;

    zp_height = atoi (ZPHeight);
    
    zp_rect.height = zp_height;
    
    zoom_factor_changed = TRUE;
    	
    if ( zp_height < display_size.height)
        mag_zoomer->zoomer_placement.extents.height = zp_height;
    else
        mag_zoomer->zoomer_placement.extents.height = display_size.height;	    

    magnifier_set_target (magnifier,
			 &((mag_zoomer->zoomer_placement).extents));
    
    magnifier_get_source (magnifier,
			      &source_rect);

    temp_rect.left = temp_rect.top = 0;
    temp_rect.width = mag_zoomer->zoomer_placement.extents.width - mag_zoomer->zoomer_placement.extents.left;
    temp_rect.height = mag_zoomer->zoomer_placement.extents.height - mag_zoomer->zoomer_placement.extents.top;

    magnifier_resize_region (magnifier,
			     0,
			     &temp_rect);
}

void 
mag_zoomer_set_ZP_border_width (MagZoomer *mag_zoomer, 
				gchar     *border_width)
{
	mag_zoomer->zoomer_placement.border.width = atoi (border_width);
}

void 
mag_zoomer_set_ZP_border_color (MagZoomer *mag_zoomer, 
				gchar     *border_color)
{
	mag_zoomer->zoomer_placement.border.color = (gint32)strtoll (border_color, &border_color, 10);
}

/* __ MAG ZOOMERS CONTAINER ___________________________________________________*/
void 
mag_zoomers_init ()
{
	mag_zoomers_hash_table = g_hash_table_new (g_str_hash, g_str_equal);
	
	
}

void 
mag_zoomers_flush (gpointer key, 
		   gpointer value, 
		   gpointer user_data)
{
	g_free (key);
	key = NULL;

	g_free (value);
	value = NULL;
}


int 
mag_zoomers_clear (gchar *clear)
{
	
	if ( (g_strcasecmp(clear, "yes")  == 0) ||
	     (g_strcasecmp(clear, "true") == 0) ||
	     (g_strcasecmp(clear, "1")    == 0))
	{

/*FIXME
		mag_zoomers_terminate();clear the hash table
    		magnifier_clear_all_regions();clear all regions from the screen
		mag_zoomers_init ();
*/
		return 1;
	}
	return 0;
}

void 
mag_zoomers_set_crosswire_size (gchar *size) 
{
	if (size) 
	{
	    crosswire_size = atoi(size);    
	    if (crosswire)
		magnifier_set_crosswire_size (magnifier,
				    	      crosswire_size);
	}
}

void 
mag_zoomers_set_crosswire_color (gchar *color) 
{
    if (color)
    {
	crosswire_color = (guint32)strtoll(color, &color, 10);

	if (crosswire)
	    magnifier_set_crosswire_color (magnifier,
					   crosswire_color);
    }
}

void 
mag_zoomers_set_crosswire_clip (gchar *clip) 
{

	if ( (g_strcasecmp(clip, "no")    == 0) ||
	     (g_strcasecmp(clip, "false") == 0) ||
	     (g_strcasecmp(clip, "0")     == 0))
	{
	    crosswire_clip = FALSE;
	}
	else
	{
	    crosswire_clip = TRUE;
	}
	if (crosswire)
    	    magnifier_set_crosswire_clip (magnifier,
				    	  crosswire_clip);
}

void 
mag_zoomers_set_cursor (gchar *new_cursor_name, 
		        gchar *new_cursor_size,
			gchar *new_cursor_zoom_factor)
{
    if (new_cursor_name && cursor_name)
    {
	g_free (cursor_name);
	cursor_name = NULL;
    }
    if (new_cursor_name)
	 cursor_name = g_strdup (new_cursor_name);
    if (new_cursor_size)
	cursor_size = atoi (new_cursor_size);
    if (new_cursor_zoom_factor)
	cursor_zoom_factor = atof (new_cursor_zoom_factor);
	
#ifdef DEBUG
	fprintf (stderr, "\nmag : Cursor name %s, cursor size %d, cursor_zoom_factor %2g",
			 cursor_name, cursor_size, cursor_zoom_factor);
#endif
	if (cursor)
	    if (cursor_scale)
	    {
		    magnifier_set_cursor (magnifier,
				  cursor_name, 
				  CURSOR_SIZE_INVALID, 
				  cursor_zoom_factor);
	    }
	    else
	    {
		magnifier_set_cursor (magnifier,
				  cursor_name, 
				  cursor_size, 
				  cursor_zoom_factor);
	    }
	else
	    magnifier_set_cursor (magnifier,"none", 0, 0);

}

void
mag_zoomers_set_cursor_color (gchar *color)
{
    if (color)
    {
	cursor_color = (guint32)strtoll(color, &color, 10);

	if (cursor)
	{
	    magnifier_set_cursor_color (magnifier,
					cursor_color);
	    if (cursor_scale)
	    {
		    magnifier_set_cursor (magnifier,
				  cursor_name, 
				  CURSOR_SIZE_INVALID, 
				  cursor_zoom_factor);
	    }
	    else
	    {
		magnifier_set_cursor (magnifier,
				  cursor_name, 
				  cursor_size, 
				  cursor_zoom_factor);
	    }
	    
	}
    }
}


void 
mag_zoomers_set_cursor_scale_on_off (gchar *cursor_scale_on_off)
{
	if ( (g_strcasecmp(cursor_scale_on_off, "no")    == 0) ||
	     (g_strcasecmp(cursor_scale_on_off, "false") == 0) ||
	     (g_strcasecmp(cursor_scale_on_off, "0")     == 0))
	{
	    cursor_scale = TRUE;
	}
	else
	{
	    cursor_scale = FALSE;
	}

	if (cursor_scale)
	{
	    magnifier_set_cursor (magnifier,
				  cursor_name, 
				  CURSOR_SIZE_INVALID, 
				  cursor_zoom_factor);
	}
	else
	{
	    magnifier_set_cursor (magnifier,
				  cursor_name, 
				  cursor_size, 
				  cursor_zoom_factor);
	}
	

}




void 
mag_zoomers_set_cursor_on_off (gchar *cursor_on_off)
{
/*	fprintf (stderr, "\n%s %d : cursor on_off %s", 
			    __FILE__,
			    __LINE__,
			    cursor_on_off);
*/			    
	if ( (g_strcasecmp(cursor_on_off, "no")    == 0) ||
	     (g_strcasecmp(cursor_on_off, "false") == 0) ||
	     (g_strcasecmp(cursor_on_off, "0")     == 0))
	{
	    cursor = FALSE;
	}
	else
	{
	    cursor = TRUE;
	}

	if (cursor)
	{
	    if (cursor_scale)
    		magnifier_set_cursor (magnifier,
				    cursor_name, 
				    CURSOR_SIZE_INVALID, 
				    cursor_zoom_factor);
	    else
		magnifier_set_cursor (magnifier,
				    cursor_name, 
				    cursor_size, 
				    cursor_zoom_factor);
	}
	else
	{
	    magnifier_set_cursor (magnifier,
				  "none", 
				  0, 
				  1);
	}

}

void 
mag_zoomers_set_crosswire_on_off (gchar *crosswire_on_off)
{
/*
	fprintf (stderr, "\n%s %d : crosswire_on_off %s size=%d", 
			    __FILE__,
			    __LINE__,
			    crosswire_on_off,
			    crosswire_size);
*/			    
	if ( (g_strcasecmp(crosswire_on_off, "no")    == 0) ||
	     (g_strcasecmp(crosswire_on_off, "false") == 0) ||
	     (g_strcasecmp(crosswire_on_off, "0")     == 0))
	{
	    crosswire = FALSE;
	}
	else
	{
	    crosswire = TRUE;
	}

	if (crosswire)
	{
	    magnifier_set_crosswire_size (magnifier,
					  crosswire_size);
	}
	else
	{
	    magnifier_set_crosswire_size (magnifier,
					  0);
	}

}

void 
mag_zoomers_terminate ()
{
	if (!mag_zoomers_hash_table) return;

	g_hash_table_foreach (mag_zoomers_hash_table, 
			      mag_zoomers_flush, 
			      NULL);
	g_hash_table_destroy (mag_zoomers_hash_table);
	mag_zoomers_hash_table = NULL;
	
	g_free (cursor_name);
	cursor_name = NULL;
}

MagZoomer *
mag_get_zoomer (gchar *zoomer_ID)
{
	MagZoomer *rv = NULL;
	if (mag_zoomers_hash_table)
	    rv = (MagZoomer *) g_hash_table_lookup (mag_zoomers_hash_table, 
						    zoomer_ID);
	return rv;
}

void mag_mouse_tracking_logic (MagZoomer *rv, int out)
{
    int width, height;

 	width  = (rv->zoomer_placement.extents.width -
		  rv->zoomer_placement.extents.left) / (rv->params.zoom_factor_x);
	height = (rv->zoomer_placement.extents.height-
		  rv->zoomer_placement.extents.top) / (rv->params.zoom_factor_y);

	switch (rv->params.mouse_tracking)
	{

	    case MAG_MOUSE_TRACKING_PUSH:
		if (out) 
		{
		    if (out & OUT_RIGHT)
		    {
			roi_viewport.width = rv->region_of_interest.width;
			roi_viewport.left = roi_viewport.width - width ;
		    }
		    if (out & OUT_BOTTOM)
		    {
			roi_viewport.height = rv->region_of_interest.height;
			roi_viewport.top = roi_viewport.height - height;
		    }
		    if (out & OUT_LEFT)
		    {
			roi_viewport.left = rv->region_of_interest.left;
			roi_viewport.width = roi_viewport.left + width ;
		    }
		    if (out & OUT_TOP)
		    {
			roi_viewport.top = rv->region_of_interest.top;
			roi_viewport.height = roi_viewport.top + height;
		    }

		    magnifier_set_roi (magnifier,
			    	       0, 
			               &roi_viewport);
		}
		break;/*MAG_TRACKING_MOUSE_PUSH*/

	    case MAG_MOUSE_TRACKING_CENTERED:
		roi_viewport.left = (rv->region_of_interest.left  +  
				     rv->region_of_interest.width - 
				     width) / 2;
		roi_viewport.top  = (rv->region_of_interest.top    +  
				     rv->region_of_interest.height - 
				     height) / 2;	
		roi_viewport.width   = roi_viewport.left + width;
		roi_viewport.height  = roi_viewport.top  + height;

		magnifier_set_roi (magnifier,
			           0, 
			           &roi_viewport);
		break;/*MAG_TRACKING_MOUSE_CENTERED*/
		    
	    case MAG_MOUSE_TRACKING_PROPORTIONAL:
		roi_viewport.left = rv->region_of_interest.left -
				    rv->region_of_interest.left * width /
				     (source_rect.width - source_rect.left);
		roi_viewport.top  = rv->region_of_interest.top -
				    rv->region_of_interest.top * height /
				     (source_rect.height - source_rect.top);
    		roi_viewport.width   = roi_viewport.left + width;
		roi_viewport.height  = roi_viewport.top  + height;
#ifdef MAG_DEBUG	
		fprintf (stderr, "\n ROI big NEW proportional :[%ld %ld %ld %ld]\n",
			roi_viewport.left,
			roi_viewport.top,
			roi_viewport.width,
			roi_viewport.height);
#endif
		magnifier_set_roi (magnifier,
			           0, 
			           &roi_viewport);
		break;/*MAG_TRACKING_MOUSE_PROPORTIONAL*/

	    default :
		break;
	}
}


void mag_tracking_logic (MagZoomer *rv, int out)
{
    MagPoint roi_point;
    int align_x, align_y;
    int width, height;

 	width  = (rv->zoomer_placement.extents.width-
		  rv->zoomer_placement.extents.left) / (rv->params.zoom_factor_x);
	height = (rv->zoomer_placement.extents.height-
		  rv->zoomer_placement.extents.top) / (rv->params.zoom_factor_y);

#ifdef MAG_DEBUG
	fprintf (stderr, "\n1 ROI :[%ld %ld %ld %ld]\n",
			rv->region_of_interest.left,
			rv->region_of_interest.top,
			rv->region_of_interest.width,
			rv->region_of_interest.height);

	fprintf (stderr, "\n2 ROI big OLD :[%ld %ld %ld %ld]\n",
			roi_viewport.left,
			roi_viewport.top,
			roi_viewport.width,
			roi_viewport.height);


	fprintf (stderr, "\n3 FOCUS tracking logic tracking=%d w=%d h=%d xf=%g yf=%g\n",
			rv->params.tracking,
			width,
			height,
			rv->params.zoom_factor_x,
			rv->params.zoom_factor_y);
#endif
	switch (rv->params.tracking)
	{
	    case MAG_TRACKING_FOCUS:
		
		if (out || 
		    rv->params.alignment.x !=  MAG_ALIGN_AUTO ||
		    rv->params.alignment.y !=  MAG_ALIGN_AUTO) 
		{
		    if ( (out & OUT_X) ||
			 rv->params.alignment.x !=  MAG_ALIGN_AUTO) 
		    {
			align_x = rv->params.alignment.x;
			if (align_x == MAG_ALIGN_AUTO)
			{
			    if ( out & OUT_LEFT)
				align_x = MAG_ALIGN_MIN;
			    if ( out & OUT_RIGHT)
			    {
				if (width >= 
				    ( - rv->region_of_interest.left  +  
				    rv->region_of_interest.width) )
				    align_x = MAG_ALIGN_MAX;
				else
				    align_x = MAG_ALIGN_MIN;
			    }
			}    
			switch (align_x)
			{			
			    case MAG_ALIGN_CENTER:
				roi_point.x = (rv->region_of_interest.left  +  
				    	   rv->region_of_interest.width -
					   width) / 2;
				break;
			    case MAG_ALIGN_MIN:
				roi_point.x = rv->region_of_interest.left;
				break;
	    		    case MAG_ALIGN_MAX:
				roi_point.x = rv->region_of_interest.width - width;
				break;
			    default:
			    	roi_point.x = roi_viewport.left;
				break;
			}
			roi_viewport.left = roi_point.x ;
			roi_viewport.width = roi_viewport.left + width;
		    }    
		    if ( (out & OUT_Y) ||
			rv->params.alignment.y !=  MAG_ALIGN_AUTO) 
		    {
			align_y = rv->params.alignment.y;
			
			if (align_y == MAG_ALIGN_AUTO)
			{
			    if ( out & OUT_TOP)
				align_y = MAG_ALIGN_MIN;
			    if ( out & OUT_BOTTOM)
				align_y = MAG_ALIGN_MAX;
			}
			switch (align_y)
	    		{
		    	    case MAG_ALIGN_CENTER:
				roi_point.y = (rv->region_of_interest.top +  
				       rv->region_of_interest.height -
				       height) / 2;
			    break;
			    case MAG_ALIGN_MIN:
				roi_point.y = rv->region_of_interest.top;
				break;
			    case MAG_ALIGN_MAX:
				roi_point.y = rv->region_of_interest.height - height;	    
				break;
			    default:
			    	roi_point.y = roi_viewport.top;	    
				break;
			}
			roi_viewport.top  = roi_point.y ;	
			roi_viewport.height  = roi_viewport.top  + height;
		    }
#ifdef MAG_DEBUG
	fprintf (stderr, "\n4 ROI big NEW :[%ld %ld %ld %ld]\n",
			roi_viewport.left,
			roi_viewport.top,
			roi_viewport.width,
			roi_viewport.height);
#endif
		magnifier_set_roi (magnifier,
			    		0, 
			        	&roi_viewport);

		}

		break;

	    case MAG_TRACKING_FOCUS_PUSH:
		if (out) 
		{
		    if (out & OUT_LEFT)
		    {
			roi_viewport.left = rv->region_of_interest.left;
			roi_viewport.width = roi_viewport.left + width ;
		    }
		    if (out & OUT_TOP)
		    {
			roi_viewport.top = rv->region_of_interest.top;
			roi_viewport.height = roi_viewport.top + height;
		    }
		    if (out & OUT_RIGHT)
		    {
			roi_viewport.width = rv->region_of_interest.width;
			roi_viewport.left = roi_viewport.width - width ;
		    }
		    if (out & OUT_BOTTOM)
		    {
			roi_viewport.height = rv->region_of_interest.height;
			roi_viewport.top = roi_viewport.height - height;
		    }
		    magnifier_set_roi (magnifier,
			    	       0, 
			               &roi_viewport);
		}
		break;

	    case MAG_TRACKING_PANNING_CENTERED:
		/*TBR - NOW centered*/
		roi_viewport.left = (rv->region_of_interest.left  +  
				     rv->region_of_interest.width - 
				     width) / 2;
		roi_viewport.top  = (rv->region_of_interest.top    +  
				     rv->region_of_interest.height - 
				     height) / 2;	
		roi_viewport.width   = roi_viewport.left + width;
		roi_viewport.height  = roi_viewport.top  + height;
		magnifier_set_roi (magnifier,
			           0, 
			           &roi_viewport);
	    case MAG_TRACKING_PANNING:
		/*TBR - aligned min*/
		roi_viewport.left =  rv->region_of_interest.left;
		roi_viewport.top  =  rv->region_of_interest.top;
		roi_viewport.width   = roi_viewport.left + width;
		roi_viewport.height  = roi_viewport.top  + height;
		magnifier_set_roi (magnifier,
			           0, 
			           &roi_viewport);


		break;
	    case MAG_TRACKING_MOUSE:		
	        mag_mouse_tracking_logic (rv, out);
		break;
	    default :
		break;
	}

}

void 
mag_set_properties (MagZoomer *rv)
{

    int  out = 0;
	
	if (zoom_factor_changed)
	{
	    out = OUT_LEFT | OUT_RIGHT | OUT_TOP | OUT_BOTTOM;
    	    cursor_zoom_factor = (rv->params.zoom_factor_x + rv->params.zoom_factor_y)/2;
#ifdef MAG_DEBUG
	    fprintf (stderr,"\n%s %d (mag_set_properties): zoom_factor_changed %d, cursor_zoom_factor %2g",
			    __FILE__,
			    __LINE__,
			    zoom_factor_changed,
			    cursor_zoom_factor);
#endif
	    if (cursor_scale)
	    {

		magnifier_set_cursor (magnifier,
				  cursor_name, 
				  CURSOR_SIZE_INVALID, 
				  cursor_zoom_factor);
	    }
	    zoom_factor_changed = FALSE;
	}
	else
	{	
	    if (rv->region_of_interest.width <= roi_viewport.left)
		out = out | OUT_LEFT;/*OUT_LEFT_LEFT */
	    else
	    {
		if (rv->region_of_interest.left >= roi_viewport.width)
		    out = out | OUT_RIGHT; /*OUT_RIGHT_RIGHT*/
		else
		{
		    if (rv->region_of_interest.left < roi_viewport.left)
		    {
			if (rv->region_of_interest.width < roi_viewport.width)
			    out = out | OUT_LEFT; /*OUT_LEFT*/
			else
			    out = out | OUT_LEFT; /*OUT_LEFT_RIGHT*/
		    }
		    else
		    {
			if (rv->region_of_interest.width > roi_viewport.width)
			    out = out | OUT_RIGHT; /*OUT_RIGHT*/		    
		    }
		    
		}
	    }	
	
	    if (rv->region_of_interest.height <= roi_viewport.top)
		out = out | OUT_TOP;/*OUT_LEFT_LEFT */
	    else
	    {
		if (rv->region_of_interest.top >= roi_viewport.height)
		    out = out | OUT_BOTTOM; /*OUT_RIGHT_RIGHT*/
		else
		{
		    if (rv->region_of_interest.top < roi_viewport.top)
		    {
			if (rv->region_of_interest.height < roi_viewport.height)
			    out = out | OUT_TOP; /*OUT_LEFT*/
			else
			    out = out | OUT_TOP; /*OUT_LEFT_RIGHT*/
		    }
		    else
		    {
			if (rv->region_of_interest.height > roi_viewport.height)
			    out = out | OUT_BOTTOM; /*OUT_RIGHT*/		    
		    }
		    
		}
	    }	
	}

#ifdef MAG_DEBUG
	fprintf (stderr, "\n0 OUT : left=%d  top=%d right=%d bottom=%d\n",
			out & OUT_LEFT,
			out & OUT_TOP,
			out & OUT_RIGHT,
			out & OUT_BOTTOM);
#endif
	mag_tracking_logic       (rv, out);
}

void 
mag_add_zoomer (MagZoomer *mag_zoomer)
{
	MagZoomer *rv = NULL;

	if (mag_zoomer && mag_zoomer->ID)
	{
		/* search zoomer in the container first */
		rv = mag_get_zoomer (mag_zoomer->ID);
		if (rv)
		{
#ifdef MAG_DEBUG
			fprintf (stderr, "\nmag : Zoomer found : ");
			mag_zoomer_content (rv);
#endif
		}
		else
		{
#ifdef MAG_DEBUG
			fprintf (stderr, "\nmag : Create new zoomer");
#endif
			/* zoomer not found, create new copy of the zoomer*/
			rv = mag_zoomer_copy (mag_zoomer);
#ifdef MAG_DEBUG
			fprintf (stderr, "\nmag : Create new zoomer...done");
#endif
			/* add copy to the other MAG zoomers (mag_zoomers_hash_table owns it) */
			g_hash_table_insert (mag_zoomers_hash_table,
					     rv->ID,
					     rv);
#ifdef MAG_DEBUG
			fprintf (stderr, "\nmag : before magnifier_create_region");
#endif
/*there are some bugs in gnome-mag (I suppose) so this will remain commented while XXX?*/
/*			magnifier_create_region (rv->params.zoom_factor_x,
						 rv->params.zoom_factor_y,
						 &(rv->region_of_interest),
						 &(rv->zoomer_placement.extents));
*/
#ifdef MAG_DEBUG
			fprintf (stderr, "\nmag : after magnifier_create_region");
#endif


		}
		/*TBReviewed*/
		mag_set_properties (rv);

	}
}

/* __ API _____________________________________________________________________*/

int 
mag_initialize (MagEventCB mag_event)
{
	MagRectangle roi;
	MagRectangle target_rect;

	mag_zoomers_init ();
	magnifier = get_magnifier ();
	if (!magnifier) 
	    return FALSE;

	magnifier_get_source (magnifier,
			      &source_rect);
#ifdef MAG_DEBUG
	fprintf (stderr, "\n%s %d : source [%ld %ld %ld %ld]",
			 __FILE__,
			 __LINE__,
			 source_rect.left,
			 source_rect.top,
			 source_rect.width,
			 source_rect.height);
#endif

	magnifier_get_target (magnifier,
			      &target_rect);
#ifdef MAG_DEBUG
	fprintf (stderr, "\n%s %d : target [%ld %ld %ld %ld]",
			 __FILE__,
			 __LINE__,
			 target_rect.left,
			 target_rect.top,
			 target_rect.width,
			 target_rect.height);
#endif
	display_size.left = display_size.top = DEFAULT_SCREEN_MIN_SIZE;
	display_size.width = gdk_screen_width () - 1;
	display_size.height = gdk_screen_height () - 1;
	
        srconf_set_data (MAGNIFIER_DISPLAY_SIZE_X, 
			 CFGT_INT, 
			 (gpointer)&display_size.width, 
			 MAGNIFIER_CONFIG_PATH);
	

        srconf_set_data (MAGNIFIER_DISPLAY_SIZE_Y, 
			 CFGT_INT, 
			 (gpointer)&display_size.height, 
			  MAGNIFIER_CONFIG_PATH);

/*vertical split*/
	 rect.top =  target_rect.top;
	 rect.left = (target_rect.width- target_rect.left) / 2;
	 rect.width = target_rect.width/2;
	 rect.height = target_rect.height;
	
	magnifier_clear_all_regions (magnifier);

	magnifier_set_target (magnifier,
			      &rect);
	 roi.top =  0;
	 roi.left = 0;
	 roi.width = target_rect.width/2 ;
	 roi.height = target_rect.height;

	magnifier_create_region (magnifier, 2.0, 2.0, &roi , &roi);			 

	magnifier_get_viewport (magnifier, 0, &rect);
	
	return TRUE;
}

void 
mag_terminate ()
{
	mag_zoomers_terminate ();/*clear the hash table*/

	magnifier_exit (magnifier);
/*	magnifier = NULL*/
#ifdef MAG_DEBUG
	fprintf(stderr,"\nTerminate the magnifier session\n");
#endif
}
