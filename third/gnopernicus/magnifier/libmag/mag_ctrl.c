/* mag_ctrl.c
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

#include "mag_ctrl.h"
#include <bonobo/bonobo-exception.h>
#include <bonobo-activation/bonobo-activation.h>
#include <libbonobo.h>
#include <string.h>

static CORBA_Environment ev;
MagRectangle zp_rect;

gchar *source_display = NULL;
gchar *target_display = NULL;

void
check_return_value (CORBA_Environment 	*env, 
		    int 		line)
{
    if ( (env) && (env)->_major != CORBA_NO_EXCEPTION) 
    {
	fprintf (stderr,"\n\ngnopernicus-magnifier : Exception \"%s\" occured at %d line.",
			bonobo_exception_get_text (env), line);
	 CORBA_exception_free (env);
    }
}


GNOME_Magnifier_Magnifier
get_magnifier (void)
{
    CORBA_Object		oclient;
    char 			*obj_id;
    CORBA_Environment 		ev;

    CORBA_exception_init (&ev);
  
    obj_id = "OAFIID:GNOME_Magnifier_Magnifier:0.9";

    oclient = bonobo_activation_activate_from_id (obj_id, 
						  0, 
						  NULL, 
						  &ev);
    if (ev._major != CORBA_NO_EXCEPTION) 
    {
	fprintf (stderr,
		   ("Activation error: during magnifier activation: %s\n"),
		   CORBA_exception_id (&ev) );
	CORBA_exception_free (&ev);
    }

    if (CORBA_Object_is_nil (oclient, &ev) )
    {
	g_warning ("Could not locate magnifier");
    }
    
    source_display = NULL;
    target_display = NULL;    
    
    return (GNOME_Magnifier_Magnifier) oclient;
}

void
magnifier_exit (GNOME_Magnifier_Magnifier	magnifier)
{
    if (magnifier)
    {
	GNOME_Magnifier_Magnifier_clearAllZoomRegions (magnifier,
						       &ev);
	check_return_value (&ev, __LINE__);
/*	GNOME_Magnifier_Magnifier_unref (magnifier, 
					 &ev);
*/	GNOME_Magnifier_Magnifier_dispose (magnifier, &ev);
    }
    g_free (source_display);
    g_free (target_display);
}

/*________________________SOURCE/TARGET____________________________________*/
void
magnifier_set_source_screen (GNOME_Magnifier_Magnifier 	magnifier,
			     gchar 			*source_display_screen)
{
    g_free (source_display);
    source_display = g_strdup (source_display_screen);
    if (magnifier)
    {
	GNOME_Magnifier_Magnifier__set_SourceDisplay (magnifier,
						      source_display_screen,
						      &ev);
	check_return_value (&ev, __LINE__);
    }
}

void
magnifier_set_target_screen (GNOME_Magnifier_Magnifier 	magnifier,
			     gchar 			*target_display_screen)
{
    g_free (target_display);
    target_display = g_strdup (target_display_screen);
    if (magnifier)
    {
	    GNOME_Magnifier_Magnifier__set_TargetDisplay (magnifier,
						    	  target_display_screen,
						    	  &ev);
	    check_return_value (&ev, __LINE__);
    }
}


void 
magnifier_get_source (GNOME_Magnifier_Magnifier 	magnifier,
		      MagRectangle	 		*source_rect)
{
    GNOME_Magnifier_RectBounds 	*rectangle 	= NULL;
    CORBA_any 			*rectangle_any 	= NULL;
    Bonobo_PropertyBag 		properties;


    if (magnifier)
    {
	properties = GNOME_Magnifier_Magnifier_getProperties (magnifier,
			    				      &ev);
	check_return_value (&ev, __LINE__);
	if (properties)
	{
	    rectangle_any = Bonobo_PropertyBag_getValue (properties,
    			    				 "source-display-bounds",
  							 &ev);
	    check_return_value (&ev, __LINE__);
	    rectangle = (GNOME_Magnifier_RectBounds *) rectangle_any->_value;
	    if (rectangle)
	    {
		if (source_rect)
		{
		    
		    source_rect->left   = (long) rectangle->x1;
		    source_rect->top    = (long) rectangle->y1;
		    source_rect->width  = (long) rectangle->x2;
		    source_rect->height = (long) rectangle->y2;
		}
		CORBA_free (rectangle);
		rectangle = NULL;
	    }
	    else
	    {
		source_rect->left   = -1;
		source_rect->top    = -1;
		source_rect->width  = -1;
		source_rect->height = -1;
	    }/*end if (rectangle)*/
	}
	else
	{
	    fprintf (stderr, "\nmagnifier_get_source : Properties are NIL");
	}/*end if (properties) */
	bonobo_object_release_unref (properties, NULL);
    }
    else
    {
	fprintf (stderr, "\nmagnifier_get_source : magnifier is NIL");  
    }/*end if (magnifier)*/

}

void magnifier_get_target (GNOME_Magnifier_Magnifier	magnifier,
			   MagRectangle 		*target_rect)
{
    GNOME_Magnifier_RectBounds	*rectangle 	= NULL;
    CORBA_any 			*rectangle_any 	= NULL;
    Bonobo_PropertyBag 		properties;

    if (magnifier)
    {    
	properties = GNOME_Magnifier_Magnifier_getProperties (magnifier,
			    				      &ev);
	check_return_value (&ev, __LINE__);
	if (properties)
	{
	    rectangle_any = Bonobo_PropertyBag_getValue (properties,
    			    				 "target-display-bounds",
  							 &ev);
    	    check_return_value (&ev, __LINE__);
	    rectangle = (GNOME_Magnifier_RectBounds *) rectangle_any->_value;

	    if (rectangle)
	    {
		if (target_rect)
		{
		    target_rect->left   = (long) rectangle->x1;
		    target_rect->top    = (long) rectangle->y1;
		    target_rect->width  = (long) rectangle->x2;
		    target_rect->height = (long) rectangle->y2;
		}
		CORBA_free (rectangle);
		rectangle = NULL;
	    }
	    else
	    {
		target_rect->left   = -1;
		target_rect->top    = -1;
		target_rect->width  = -1;
		target_rect->height = -1;
	    }		
	}
	bonobo_object_release_unref (properties, NULL);
    }
}

void magnifier_set_target (GNOME_Magnifier_Magnifier	magnifier,
			   MagRectangle 		*target_rect)
{
    GNOME_Magnifier_RectBounds	*rectangle 	= NULL;
    CORBA_any 			*rectangle_any	= NULL;
    Bonobo_PropertyBag 		properties;

    if (magnifier)
    {    
	properties = GNOME_Magnifier_Magnifier_getProperties (magnifier,
			    				      &ev);
	check_return_value (&ev, __LINE__);
	if (properties)
	{
	    rectangle = GNOME_Magnifier_RectBounds__alloc ();
	    rectangle_any = CORBA_any__alloc ();
	
	    rectangle->x1 = (CORBA_long) target_rect->left;
	    rectangle->y1 = (CORBA_long) target_rect->top;
	    rectangle->x2 = (CORBA_long) target_rect->width;
	    rectangle->y2 = (CORBA_long) target_rect->height;
	
	    rectangle_any->_type = TC_GNOME_Magnifier_RectBounds;
	    rectangle_any->_value = ORBit_copy_value (rectangle,
						  TC_GNOME_Magnifier_RectBounds);
	    Bonobo_PropertyBag_setValue (properties,
    			                 "target-display-bounds",
				        rectangle_any,
  				        &ev);
	    check_return_value (&ev, __LINE__);
	}
	bonobo_object_release_unref (properties, NULL);
    }
}

/*_________________________________CURSORS____________________________________*/
void
magnifier_set_cursor (GNOME_Magnifier_Magnifier	magnifier,
		      gchar 			*cursor_name,
		      int    			cursor_size,
		      float  			cursor_zoom_factor)
{
    Bonobo_PropertyBag	properties;

    if (magnifier)
    {
	properties = GNOME_Magnifier_Magnifier_getProperties (magnifier,
							      &ev);
	if (properties)
	{

	    if (cursor_name)
	    {
		BonoboArg *arg = bonobo_arg_new (BONOBO_ARG_STRING);
		BONOBO_ARG_SET_STRING (arg, cursor_name);
		Bonobo_PropertyBag_setValue (properties,
    			    		     "cursor-set",
					     arg,
					     &ev);
		check_return_value (&ev, __LINE__);
	    }
/*		if (cursor_zoom_factor != 1.0F)				*/
	    Bonobo_PropertyBag_setValue (properties,
    			       "cursor-scale-factor",
				bonobo_arg_new_from (BONOBO_ARG_FLOAT,
						     &cursor_zoom_factor),
				&ev);
/*	else
	if (cursor_size > 0)
	{
*/
	    Bonobo_PropertyBag_setValue (properties,
    			    		    "cursor-size",
					     bonobo_arg_new_from
					        (BONOBO_ARG_INT,
						 &cursor_size),
					     &ev);
	    check_return_value (&ev, __LINE__);
	}
	bonobo_object_release_unref (properties, NULL);
    }
}

void
magnifier_set_cursor_color (GNOME_Magnifier_Magnifier	magnifier,
			    guint32   			cursor_color)		    
{
    Bonobo_PropertyBag	properties;

    if (magnifier)
    {
	properties = GNOME_Magnifier_Magnifier_getProperties (magnifier,
							      &ev);
	if (properties)
	{

	    if (cursor_color >= 0)
	    {
		bonobo_pbclient_set_ulong (properties,
    					 "cursor-color",
				    	  cursor_color,
				    	  &ev);

		check_return_value (&ev, __LINE__);
	    }
	    bonobo_object_release_unref (properties, NULL);
	}
    }
}

/*_____________________________CROSSWIRE____________________________________*/
void
magnifier_set_crosswire_size (GNOME_Magnifier_Magnifier 	magnifier,
			      int    				crosswire_size)
{
    Bonobo_PropertyBag 	properties;

    if (magnifier)
    {
	properties = GNOME_Magnifier_Magnifier_getProperties (magnifier,
			    				      &ev);
	if (properties)
	{
	    if (crosswire_size >= 0)
	    {
		bonobo_pbclient_set_long (properties,
    			    		 "crosswire-size",
					  crosswire_size,
					  &ev);
		check_return_value (&ev, __LINE__);
	    }
	}
	bonobo_object_release_unref (properties, &ev);
    }
}

void
magnifier_set_crosswire_color (GNOME_Magnifier_Magnifier 	magnifier,
			       guint32 				crosswire_color)
{
    Bonobo_PropertyBag properties;

    if (magnifier)
    {
	properties = GNOME_Magnifier_Magnifier_getProperties (magnifier,
							      &ev);
	if (properties)
	{
	    if (crosswire_color >= 0)
	    {
		bonobo_pbclient_set_long (properties,
    					 "crosswire-color",
				    	  crosswire_color,
				    	  &ev);
		check_return_value (&ev, __LINE__);
	    }
	}
	bonobo_object_release_unref (properties, &ev);
    }
}

void
magnifier_set_crosswire_clip (GNOME_Magnifier_Magnifier 	magnifier,
			      gboolean 				crosswire_clip)
{
    Bonobo_PropertyBag 	properties;
    
    if (magnifier)
    {
	properties = GNOME_Magnifier_Magnifier_getProperties
    			    (magnifier,
			     &ev);
	if (properties)
	{
    	    bonobo_pbclient_set_boolean (properties,
    		    		        "crosswire-clip",
				        crosswire_clip,
				        &ev);
	    check_return_value (&ev, __LINE__);
	}
	bonobo_object_release_unref (properties, &ev);
    }
}

/*_____________________________ZOOMER PROPERTIES____________________________________*/

static MagRectangle *
mag_rect_clip_to_source (MagRectangle *roi)
{    
    static MagRectangle temp;
    
    g_assert (source_display);
    g_assert (target_display);
    
    if (strcmp (source_display, target_display) == 0)
    {
	if ((roi->width > zp_rect.left) && (roi->width < zp_rect.width))
	{
	    if ((roi->top < zp_rect.height) && (roi->height > zp_rect.top))
	    {
		temp.width = zp_rect.left;
		temp.top = roi->top;
		temp.left = roi->left - (roi->width - zp_rect.left);
		temp.height = roi->height;
	    }
	else
	    return roi;    
	}
	else
	    if ((roi->left < zp_rect.width) && (roi->left > zp_rect.left))
	    {
		if ((roi->top < zp_rect.height) && (roi->height > zp_rect.top))
		{
		    temp.width = (zp_rect.width - roi->left) + roi->width;
		    temp.top = roi->top;
		    temp.left = zp_rect.width;
		    temp.height = roi->height;
		}
    		else
		    return roi;    
	    }
	    else 
		return roi;
	return &temp;
    }
    else
	return roi;
}


void
magnifier_set_roi (GNOME_Magnifier_Magnifier 	magnifier,
		   int 				zoom_region,
		   MagRectangle 		*roi)
{
    GNOME_Magnifier_ZoomRegionList 	*regions;
    
    
    roi = mag_rect_clip_to_source (roi);

    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier,
							    &ev);
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{
	    if (regions->_buffer[zoom_region])
    	    {
		GNOME_Magnifier_ZoomRegion_setROI (regions->_buffer[zoom_region],
					    	   (GNOME_Magnifier_RectBounds *) roi,
					    	   &ev);
		check_return_value (&ev, __LINE__);					       
    	    }
    	    else
    	    {
    		fprintf (stderr, "\n%s-%d, Region is NIL",__FILE__, __LINE__);
    	    }
	}
    }
}

void magnifier_get_viewport (GNOME_Magnifier_Magnifier	magnifier,
			    int 			zoom_region,
			    MagRectangle 		*viewport_rect)
{
    GNOME_Magnifier_ZoomRegionList 	*regions;
    GNOME_Magnifier_RectBounds		*rectangle 	= NULL;
    CORBA_any 				*rectangle_any 	= NULL;
    Bonobo_PropertyBag 			properties;

    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier,
							    &ev);
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{
	    if (regions->_buffer[zoom_region])
    	    {
		properties = GNOME_Magnifier_ZoomRegion_getProperties
				(regions->_buffer[zoom_region],
			        &ev);
		if (properties)
		{
		    rectangle_any = Bonobo_PropertyBag_getValue (properties,
    			    					"viewport",
  								&ev);
		    check_return_value (&ev, __LINE__);
		    rectangle = (GNOME_Magnifier_RectBounds *) 
				rectangle_any->_value;
		    if (rectangle)
		    {
			if (viewport_rect)
			{
			    viewport_rect->left   = (long) rectangle->x1;
			    viewport_rect->top    = (long) rectangle->y1;
			    viewport_rect->width  = (long) rectangle->x2;
			    viewport_rect->height = (long) rectangle->y2;
			}
			CORBA_free (rectangle);
			rectangle = NULL;
		    }
		    else
		    {
			viewport_rect->left   = -1;
			viewport_rect->top    = -1;
			viewport_rect->width  = -1;
			viewport_rect->height = -1;
		    }/*end if (rectangle)*/
		}
		else
		{
		    fprintf (stderr, "\nmagnifier_get_viewport : Properties are NIL");
		}/*end if (properties) */
		bonobo_object_release_unref (properties, &ev);
	    }
	    else
	    {
	        fprintf (stderr, "\nmagnifier_get_viewport : region is NIL");
	    }/*end if (magnifier)*/
	}
    }
}

void
magnifier_set_is_managed  (GNOME_Magnifier_Magnifier	magnifier,
			   int 				zoom_region,
			   int 				is_managed)
{
    GNOME_Magnifier_ZoomRegionList 	*regions;
    Bonobo_PropertyBag 			properties;

    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier,
							    &ev);
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{
    	    if ( (CORBA_Object *)(regions->_buffer)[zoom_region])
    	    {
    		properties = GNOME_Magnifier_ZoomRegion_getProperties
    				( (regions->_buffer)[zoom_region],
			         &ev);
    	        bonobo_pbclient_set_boolean (properties,
					    "is-managed",
					     is_managed,
					     &ev);
		bonobo_object_release_unref (properties, &ev);
    	    }
	}
    }
}

void
magnifier_set_invert (GNOME_Magnifier_Magnifier	magnifier,
		      int 			zoom_region,
		      int 			invert)
{
    Bonobo_PropertyBag			properties;
    GNOME_Magnifier_ZoomRegionList 	*regions;

    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier,
							    &ev);
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{		  
    	    if ( (regions->_buffer)[zoom_region] != CORBA_OBJECT_NIL)
    	    {
    		properties = GNOME_Magnifier_ZoomRegion_getProperties
    			    ( (regions->_buffer) [zoom_region],
			     &ev);

    	        bonobo_pbclient_set_boolean (properties,
    					    "inverse-video",
					     invert,
					     &ev);		    
		bonobo_object_release_unref (properties, &ev);
    	    }
	}				    
    }				    
}

void
magnifier_set_smoothing_type (GNOME_Magnifier_Magnifier 	magnifier,
			      int    				zoom_region,
			      gchar 				*smoothing_type)
{
    GNOME_Magnifier_ZoomRegionList 	*regions;
    Bonobo_PropertyBag 			properties;
  
    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier,
							    &ev);
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{		  
    	    if ( ( (CORBA_Object *) (regions->_buffer) )[zoom_region] != CORBA_OBJECT_NIL)
    	    {
    		properties = GNOME_Magnifier_ZoomRegion_getProperties
    				( (regions->_buffer)[zoom_region],
			        &ev);
		if (smoothing_type)
    		{
		    bonobo_pbclient_set_string (properties,
    						"smoothing-type",
						smoothing_type,
						&ev);
		}

		bonobo_object_release_unref (properties, &ev);		
    	    }
	}				    
    }				    
}

void
magnifier_set_contrast (GNOME_Magnifier_Magnifier 	magnifier,
			int  				zoom_region,
			float 				contrast)
{
    GNOME_Magnifier_ZoomRegionList 	*regions;
    Bonobo_PropertyBag 			properties;  

    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions
    		( magnifier,
		  &ev);
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{		  
    	    if ( ( (CORBA_Object *) (regions->_buffer) )[zoom_region] != CORBA_OBJECT_NIL)
    	    {
    		properties = GNOME_Magnifier_ZoomRegion_getProperties
    				( (regions->_buffer)[zoom_region],
			        &ev);

    	        bonobo_pbclient_set_float (properties,
    					  "contrast",
					   contrast,
					   &ev);

		bonobo_object_release_unref (properties, &ev);					   
    	    }
	}				    
    }				    
}


void
magnifier_set_border (GNOME_Magnifier_Magnifier 	magnifier,
		     int  				zoom_region,
		     int  				border_size,
		     long 				border_color)
{
    GNOME_Magnifier_ZoomRegionList 	*regions;
    Bonobo_PropertyBag 			properties;

    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier,
							    &ev);
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{
    	    if ( ( (CORBA_Object *) (regions->_buffer) )[zoom_region] != CORBA_OBJECT_NIL)
    	    {
    		properties = GNOME_Magnifier_ZoomRegion_getProperties
    				( (regions->_buffer)[zoom_region],
			         &ev);

		if ( border_size > -1)
    	    	    bonobo_pbclient_set_long (properties,
    					     "border-size",
					      border_size,
					      &ev);

		if ( border_color > -1)
    	    	    bonobo_pbclient_set_long (properties,
    					     "border-color",
					      border_color,
					      &ev);
    		bonobo_object_release_unref (properties, NULL);
	    }
	}
    }
}

void
magnifier_set_alignment (GNOME_Magnifier_Magnifier	magnifier,
			int  				zoom_region,
			int 				alignment_x,
		        int 				alignment_y)
{
    GNOME_Magnifier_ZoomRegionList 	*regions;
    Bonobo_PropertyBag 			properties;

    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier,
    							    &ev);
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{		  
	    if ( ( (CORBA_Object *) (regions->_buffer) )[zoom_region] != CORBA_OBJECT_NIL)
    	    {
    		properties = GNOME_Magnifier_ZoomRegion_getProperties
    			    ( (regions->_buffer)[zoom_region],
			     &ev);

		if ( alignment_x > -1)
    	    	    bonobo_pbclient_set_long (properties,
    					     "x-alignment",
					      alignment_x,
					      &ev);

		if ( alignment_y > -1)
    	    	    bonobo_pbclient_set_long (properties,
    					     "y-alignment",
					      alignment_y,
					      &ev);
    		bonobo_object_release_unref (properties, NULL);					      

    	    }
	}				    
    }				    
}


void
magnifier_resize_region (GNOME_Magnifier_Magnifier	magnifier,
			 int 				zoom_region, 
			 MagRectangle 			*viewport)
{
    GNOME_Magnifier_ZoomRegionList	*regions;
    
    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions (magnifier,
		    					    &ev);
        check_return_value (&ev, __LINE__);		  
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1 <= regions->_length) )
	{		  
    	    if (regions->_buffer[zoom_region] != CORBA_OBJECT_NIL)
    	    {
    		GNOME_Magnifier_ZoomRegion_moveResize 
		    (regions->_buffer[zoom_region],
		     (GNOME_Magnifier_RectBounds *) viewport,
		     &ev);
		check_return_value (&ev, __LINE__);					       
    	    }
    	    else
    	    {
    		fprintf (stderr, "\n%s-%d, Region is NIL",__FILE__, __LINE__);
    	    }
	}				    
    }				    
}

void
magnifier_clear_all_regions (GNOME_Magnifier_Magnifier	magnifier)
{
    if (magnifier) 
    {
	GNOME_Magnifier_Magnifier_clearAllZoomRegions (magnifier,
						       &ev);
	check_return_value (&ev, __LINE__);
    }
}

void
magnifier_unref (GNOME_Magnifier_Magnifier 	magnifier)
{
    if (magnifier) 
    {
	GNOME_Magnifier_Magnifier_unref (magnifier,
					 &ev);
	check_return_value (&ev, __LINE__);
    }
}

int
magnifier_create_region (GNOME_Magnifier_Magnifier 	magnifier,
			 float 				zx, 
			 float 				zy, 
			 MagRectangle 			*roi,
			 MagRectangle 			*viewport)
{

    GNOME_Magnifier_ZoomRegion 	region;
    int 			retval = -1;

    if (magnifier)
    {
	region = GNOME_Magnifier_Magnifier_createZoomRegion
		(magnifier,
		 zx,
		 zy,
		 (GNOME_Magnifier_RectBounds *) roi,
		 (GNOME_Magnifier_RectBounds *) viewport,
		 &ev);
	check_return_value (&ev, __LINE__);		 

	if ( region != CORBA_OBJECT_NIL ) 
	{
	    retval = (short) GNOME_Magnifier_Magnifier_addZoomRegion 
			    (magnifier,
			     region,
			     &ev);
	    check_return_value (&ev, __LINE__);			     
	}
    }
    return  retval;
}

void
magnifier_set_magnification (GNOME_Magnifier_Magnifier	magnifier,
			     int   			zoom_region, 
			     float 			mag_factor_x, 
			     float 			mag_factor_y)
{
    GNOME_Magnifier_ZoomRegionList	*regions;
  
    if (magnifier)
    {
	regions = GNOME_Magnifier_Magnifier_getZoomRegions 
    		( magnifier,
		  &ev);
        check_return_value (&ev, __LINE__);		  
	if (regions 		&& 
	    regions->_length 	&& 
	    (zoom_region + 1  <= regions->_length) )
	{		   
    	    if (regions->_buffer[zoom_region])
    	    {
    		GNOME_Magnifier_ZoomRegion_setMagFactor (regions->_buffer[zoom_region],
					    	     mag_factor_x,
						     mag_factor_y,
					    	     &ev);
		check_return_value (&ev, __LINE__);					       
    	    }
    	    else
    	    {
/*    		fprintf (stderr, "\n%s-%d, Region is NIL",__FILE__, __LINE__);*/
    	    }
	}				    
    }				    
}


