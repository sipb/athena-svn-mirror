/* mag.h
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

#ifndef _MAG_H
#define _MAG_H

#include <glib.h>

#define ZOOM_FACTOR_MAX		16
#define BORDER_WIDTH_MAX	20


typedef struct
{
    long		left  ;
    long		top   ;
    long		width ;
    long		height;
}MagRectangle;


typedef struct
{
    long		x;
    long		y;
}MagPoint;

typedef struct
{
    gint32		color;
    int			width;
}MagBorderType;

typedef struct
{
    int 		primary_color  ;
    int 		secondary_color;
}MagColorScheme;

typedef struct
{
    gboolean		i_black_and_white;
    gboolean 		i_custom_colors  ;
    MagColorScheme   	custom_colors    ;
}MagInvertType;

typedef struct
{
    gboolean		s_black_and_white;
    gboolean 		s_custom_colors  ;
    MagColorScheme   	custom_colors    ;
}MagSmoothingType;

typedef enum
{
    MAG_TRACKING_FOCUS,
    MAG_TRACKING_FOCUS_PUSH,
    MAG_TRACKING_MOUSE,
    MAG_TRACKING_PANNING,
    MAG_TRACKING_PANNING_CENTERED,
    /*locking a zoomer to a region of screen*/
    MAG_TRACKING_NONE
}MagTrackingType;

typedef enum
{
    MAG_MOUSE_TRACKING_PUSH,
    MAG_MOUSE_TRACKING_CENTERED,
    MAG_MOUSE_TRACKING_PROPORTIONAL,
    MAG_MOUSE_TRACKING_NONE
}MagMouseTrackingType;

typedef enum
{
    MAG_ALIGN_AUTO,
    MAG_ALIGN_CENTER,
    MAG_ALIGN_MIN,
    MAG_ALIGN_MAX,
    MAG_ALIGN_NONE
}MagAlignmentType;


typedef struct
{
    MagAlignmentType x;
    MagAlignmentType y;    
}MagAlignment;


typedef enum
{
    MAG_SMOOTH_ALG_NONE,
    MAG_SMOOTH_ALG_WEIGHT,
    MAG_SMOOTH_ALG_BICUBIC,/* the last 2 are very slow */
    MAG_SMOOTH_ALG_BILINIAR/* maybe there be others, besides weight  */
}MagSmoothingAlgorithm;


typedef MagRectangle MagROI;

typedef struct 
{
	MagRectangle	extents;
/*???	gboolean	is_managed;from AT_SPI */
/*???	gboolean	is_dirty;  from AT_SPI */
	MagBorderType	border;
}MagZP;	

typedef struct
{
/* magnification */
	float			zoom_factor_x;
	float 			zoom_factor_y;

/* color schema*/
	float			contrast;	 
	gboolean		invert;          
/*	MagInvertType		invert_type ;*/

/* smoothing 
	MagSmoothingAlgorithm	smoothing_algorithm;
	MagSmoothingType	smtoothing_type;
*/
	gchar 			*smoothing_type;	
/*alignment*/
	MagAlignment		alignment;
/*tracking mode for focus, mouse, panning*/
	MagTrackingType		tracking;
	MagMouseTrackingType	mouse_tracking;
/* other */
	gboolean		visible;

} MagParameters;


typedef struct
{
	gchar		*ID 	          ;
	
	/* source &  target displays */
	gchar		*source           ;
	gchar		*target           ;	
	
	/* MAG look...maybe this should be a union */
/*	int 		vertical_split    ;
	int 		horizontal_split  ;
	int 		dual_head	  ;
	int 		full_screen       ; 
*/	
	MagZP		zoomer_placement  ; 
	/* MAG param */
	MagROI		region_of_interest;
	MagParameters	params		  ;
} MagZoomer;


/* __ MAG ZOOMER CONSTRUCTOR/DESTRUCTOR ________________________________________*/
MagZoomer* mag_zoomer_new  ();
void 	   mag_zoomer_free (MagZoomer  *mag_zoomer);
void       mag_zoomer_init (MagZoomer  *mag_zoomer);
MagZoomer* mag_zoomer_copy (MagZoomer  *orig_mag_zoomer);
MagZoomer* mag_zoomer_dup  (MagZoomer  *orig_mag_zoomer);
gboolean   mag_zoomer_find (MagZoomer ** mag_zoomer);

/* __ MAG ZOOMER____________ ________________________________________________*/
void       mag_zoomer_set_id (MagZoomer *mag_zoomer, 
			      gchar     *id);
void       mag_zoomer_set_source (MagZoomer *mag_zoomer, 
			    	  gchar     *source);
void       mag_zoomer_set_target (MagZoomer *mag_zoomer, 
			    	  gchar     *target);

/* __ MAG ZOOMER PARAMS METHODS ________________________________________________*/
void mag_zoomer_set_params_zoom_factor    (MagZoomer *mag_zoomer, 
					   gchar     *zoom_factor_x,
					   gchar     *zoom_factor_y);
void mag_zoomer_set_params_invert         (MagZoomer *mag_zoomer, 
					   gchar     *invert);
void mag_zoomer_set_params_contrast       (MagZoomer *mag_zoomer, 
					   gchar     *contrast);
void mag_zoomer_set_params_smoothing_alg  (MagZoomer *mag_zoomer,
					   gchar     *smoothing_algorithm);
void mag_zoomer_set_params_alignment      (MagZoomer *mag_zoomer, 
					   gchar     *alignment_x,
					   gchar     *alignment_y);
void mag_zoomer_set_params_tracking 	  (MagZoomer *mag_zoomer, 
					   gchar     *tracking);
void mag_zoomer_set_params_mouse_tracking (MagZoomer *mag_zoomer, 
					   gchar     *mouse_tracking);
void mag_zoomer_set_params_visible        (MagZoomer *mag_zoomer, 
					   gchar     *visible);

/* __ MAG ZOOMER ROI METHODS ____________________________________________________*/
void mag_zoomer_set_ROI_left   (MagZoomer *mag_zoomer,
				gchar     *ROILeft);
void mag_zoomer_set_ROI_top    (MagZoomer *mag_zoomer, 
				gchar     *ROITop);
void mag_zoomer_set_ROI_width  (MagZoomer *mag_zoomer,
			        gchar     *ROIWidth);
void mag_zoomer_set_ROI_height (MagZoomer *mag_zoomer, 
				gchar     *ROILeft);

/* __ MAG ZOOMER ZP METHODS ____________________________________________________*/
void mag_zoomer_set_ZP_extents_left   (MagZoomer *mag_zoomer, 
				       gchar     *ZPLeft);
void mag_zoomer_set_ZP_extents_top    (MagZoomer *mag_zoomer,
				       gchar     *ZPTop);
void mag_zoomer_set_ZP_extents_width  (MagZoomer *mag_zoomer, 
				       gchar     *ZPWidth);
void mag_zoomer_set_ZP_extents_height (MagZoomer *mag_zoomer,
				       gchar     *ZPHeight);
void mag_zoomer_set_ZP_border_width   (MagZoomer *mag_zoomer,
				       gchar     *border_width);
void mag_zoomer_set_ZP_border_color   (MagZoomer *mag_zoomer,
				       gchar     *border_color);
				       
/* __ MAG ZOOMERS CONTAINER ___________________________________________________*/
void 	   mag_zoomers_init  	  ();
void 	   mag_zoomers_terminate  ();
MagZoomer* mag_get_zoomer 	  (gchar     *zoomer_ID);
void 	   mag_add_zoomer         (MagZoomer *mag_zoomer);
void 	   mag_zoomers_flush      (gpointer key, 
			           gpointer value, 
			           gpointer user_data);
int 	   mag_zoomers_clear             (gchar *clear);
void	   mag_zoomers_set_cursor        (gchar *new_cursor, 
				          gchar *new_cursor_size,
					  gchar *new_cursor_zoom_factor);
void	   mag_zoomers_set_cursor_color  (gchar *cursor_color);					  
void	   mag_zoomers_set_crosswire_size  (gchar *crosswire_size);
void	   mag_zoomers_set_crosswire_color (gchar *crosswire_color);
void	   mag_zoomers_set_crosswire_clip  (gchar *crosswire_clip);


void	   mag_zoomers_set_cursor_scale_on_off	(gchar *on_off);
void	   mag_zoomers_set_cursor_on_off	(gchar *on_off);
void	   mag_zoomers_set_crosswire_on_off	(gchar *on_off);

/* __ API _____________________________________________________________________*/

int  mag_initialize ();
void mag_terminate  ();

#endif
