/* magconf.c
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
#include "SRMessages.h"
#include "magconf.h"
#include "magui.h"
#include "gnopiconf.h"
#include <stdlib.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include "srintl.h"


#define MAGCONF_ZOOM_FACTOR_LIMIT(x) ( x < MIN_ZOOM_FACTOR  ? MIN_ZOOM_FACTOR : ( x > MAX_ZOOM_FACTOR ? MAX_ZOOM_FACTOR : x))

extern GConfClient *gnopernicus_client;
static gint 	magnifier_notify_id;
extern Magnifier *magnifier_setting;

/**
 * Notify handler						
**/
void		magconf_changes_cb		(GConfClient	*client,
						guint		cnxn_id,
						GConfEntry	*entry,
						gpointer	user_data);

/**
 * Create a new gconf client 					
**/
gboolean
magconf_gconf_client_init (void)
{
    GError *error = NULL;
    
    sru_return_val_if_fail (gnopiconf_client_add_dir (CONFIG_PATH MAGNIFIER_PATH), FALSE);

    magnifier_notify_id = gconf_client_notify_add   (gnopernicus_client,
						    CONFIG_PATH MAGNIFIER_PATH
						    ,magconf_changes_cb,
						    NULL, NULL, &error);

    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
	return FALSE;
    }

    return TRUE;
}

gboolean
magconf_copy_option_part (Magnifier *dest, 
			  Magnifier *source)
{
    gboolean rv = TRUE;
    
    (dest->zp).left	= (source->zp).left;
    (dest->zp).top	= (source->zp).top;
    (dest->zp).height	= (source->zp).height;
    (dest->zp).width	= (source->zp).width;
    dest->border_width	= source->border_width;
    dest->border_color	= source->border_color;
    
    return rv;
}

Magnifier*
magconf_setting_clone (Magnifier *magnifier)
{
    Magnifier *value = NULL;
    value = magconf_setting_new ();
    sru_assert (value);    
    value->id		= g_strdup (magnifier->id);
	
    value->source 	= g_strdup (magnifier->source);
    value->target 	= g_strdup (magnifier->target);
    
    (value->zp).left	= (magnifier->zp).left;
    (value->zp).top	= (magnifier->zp).top;
    (value->zp).height	= (magnifier->zp).height;
    (value->zp).width	= (magnifier->zp).width;
    
    value->zoomfactorx	= magnifier->zoomfactorx;
    value->zoomfactory	= magnifier->zoomfactory;
    value->zoomfactor_lock	= magnifier->zoomfactor_lock;
    
    value->visible	= magnifier->visible;
    value->invert	= magnifier->invert;
    value->smoothing	= g_strdup (magnifier->smoothing);
    value->panning	= magnifier->panning;
    
    value->border_width	= magnifier->border_width;
    value->border_color	= magnifier->border_color;
    
    value->cursor_state	= magnifier->cursor_state;
    value->cursor_name	= g_strdup (magnifier->cursor_name);
    value->cursor_size	= magnifier->cursor_size;
    value->cursor_scale = magnifier->cursor_scale;
    value->cursor_color = magnifier->cursor_color;
    
    value->crosswire 	= magnifier->crosswire;
    value->crosswire_clip = magnifier->crosswire_clip;
    value->crosswire_color = magnifier->crosswire_color;
    value->crosswire_size = magnifier->crosswire_size;    
    
    value->mouse_tracking = g_strdup (magnifier->mouse_tracking);
    value->tracking  	  = g_strdup (magnifier->tracking);
    return value;
}

/**
 * Load setting value
**/
Magnifier* 
magconf_setting_init (const gchar *id, 
		      gboolean set_struct)
{
    Magnifier *value = NULL;
    gint size_x, size_y;
    
    value = magconf_setting_new ();
    
    sru_assert (value);
    if (id)	
	value->id	= g_strdup (id);
    else	
	value->id	= g_strdup (DEFAULT_MAGNIFIER_ID);
	
    magconf_get_display_size (&size_x, &size_y);
	
	
    value->source 	= g_strdup (DEFAULT_MAGNIFIER_SOURCE);
    value->target 	= g_strdup (DEFAULT_MAGNIFIER_TARGET);
    (value->zp).left	= size_x / 2;
    (value->zp).top	= DEFAULT_MAGNIFIER_ZP_TOP;
    (value->zp).height	= size_y;
    (value->zp).width	= size_x;

    (value->zp).height	= DEFAULT_MAGNIFIER_ZP_HEIGHT;
    (value->zp).width	= DEFAULT_MAGNIFIER_ZP_WIDTH;
    value->zoomfactorx	= DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY;
    value->zoomfactory	= DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY;
    value->zoomfactor_lock	= DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK;
    
    value->visible	= DEFAULT_MAGNIFIER_VISIBLE;
    value->invert	= DEFAULT_MAGNIFIER_INVERT;
    value->smoothing	= g_strdup (DEFAULT_MAGNIFIER_SMOOTHING);
    value->panning	= DEFAULT_MAGNIFIER_PANNING;
    
    value->border_width	= DEFAULT_MAGNIFIER_BORDER_WIDTH;
    value->border_color	= DEFAULT_MAGNIFIER_BORDER_COLOR;
    
    value->cursor_state	= DEFAULT_MAGNIFIER_CURSOR;
    value->cursor_name	= g_strdup (DEFAULT_MAGNIFIER_CURSOR_NAME);
    value->cursor_size	= DEFAULT_MAGNIFIER_CURSOR_SIZE;
    value->cursor_scale = DEFAULT_MAGNIFIER_CURSOR_SCALE;
    value->cursor_color = DEFAULT_MAGNIFIER_CURSOR_COLOR;
    
    value->crosswire 	= DEFAULT_MAGNIFIER_CROSSWIRE;
    value->crosswire_clip = DEFAULT_MAGNIFIER_CROSSWIRE_CLIP;
    value->crosswire_color = DEFAULT_MAGNIFIER_CROSSWIRE_COLOR;
    value->crosswire_size = DEFAULT_MAGNIFIER_CROSSWIRE_SIZE;    
    
    value->mouse_tracking = g_strdup (DEFAULT_MAGNIFIER_MOUSE_TRACKING);
    value->tracking  	  = g_strdup (DEFAULT_MAGNIFIER_TRACKING);
    
    value->alignment_x   = g_strdup (DEFAULT_MAGNIFIER_ALIGNMENT_X);
    value->alignment_y   = g_strdup (DEFAULT_MAGNIFIER_ALIGNMENT_Y);
        
    if (!set_struct)
	return value;
    
    magconf_setting_free (magnifier_setting);
    magnifier_setting = value;
    
    return value;
}

void 
magconf_terminate (Magnifier *magnifier_setting)
{

    magconf_setting_free (magnifier_setting);	
	
    gconf_client_notify_remove (gnopernicus_client, magnifier_notify_id);
    
    if (gconf_client_dir_exists (gnopernicus_client, CONFIG_PATH MAGNIFIER_PATH, NULL))
	gconf_client_remove_dir (gnopernicus_client, CONFIG_PATH MAGNIFIER_PATH, NULL);

}


/**
 * Create a new Magnifier structure
**/
Magnifier* 
magconf_setting_new (void)
{
    Magnifier *new_magnifier = NULL ;
    
    new_magnifier = ( Magnifier * ) g_new0 ( Magnifier, 1);
    
    if (!new_magnifier) 
	sru_error (_("Unable to allocate memory."));
    
    return new_magnifier ;
}

/**
 * Free Magnifier structure
**/
void 
magconf_setting_free (Magnifier* magnifier)
{
    if (!magnifier)
	return;
    g_free (magnifier->id);
    g_free (magnifier->cursor_name);
    g_free (magnifier->smoothing);
    g_free (magnifier->mouse_tracking);
    g_free (magnifier->alignment_x);
    g_free (magnifier->alignment_y);
    g_free (magnifier->tracking);
    g_free (magnifier->source);
    g_free (magnifier->target);
    g_free (magnifier);
    magnifier = NULL;
}


/**
 * Set Methods 
**/
gboolean
magconf_zoomfactor_lock_set (gboolean zoomfactor_lock)
{
    gchar *path;
    path = g_strdup_printf ("%s/%s/%s", 
			    CONFIG_PATH MAGNIFIER_PATH, 
			    DEFAULT_MAGNIFIER_SCHEMA, 
			    DEFAULT_MAGNIFIER_ID);
			    
    gnopiconf_set_bool_in_section  (zoomfactor_lock,
				    path,
				    MAGNIFIER_ZOOM_FACTOR_LOCK);
    
    g_free (path);
    return TRUE;
}

Magnifier*
magconf_load_zoomer_from_schema (const gchar *schema,
				 gchar *zoomer)
{
    Magnifier *magnifier = NULL;
    gchar *path = NULL;
    gint size_x, size_y;
    
    sru_return_val_if_fail (schema, NULL);    
    sru_return_val_if_fail (zoomer, NULL);    
    
    magnifier = magconf_setting_new ();
    
    sru_assert (magnifier);
    
    magconf_get_display_size (&size_x, &size_y);
    
    path = g_strdup_printf ("%s/%s/%s", 
			    CONFIG_PATH MAGNIFIER_PATH, 
			    schema, 
			    zoomer);
    
    magnifier->id 		= gnopiconf_get_string_from_section_with_default ( 	
								    path, 	
								    MAGNIFIER_ID, 		
								    zoomer);
    magnifier->source 		= gnopiconf_get_string_from_section_with_default (
								    path, 	
								    MAGNIFIER_SOURCE, 		
								    DEFAULT_MAGNIFIER_SOURCE);

    magnifier->target 		= gnopiconf_get_string_from_section_with_default ( 	
								    path, 	
								    MAGNIFIER_TARGET, 		
								    DEFAULT_MAGNIFIER_TARGET);

    (magnifier->zp).left 	= gnopiconf_get_int_from_section_with_default (
								path, 	
								MAGNIFIER_ZP_LEFT, 	
								size_x / 2);
    (magnifier->zp).top 	= gnopiconf_get_int_from_section_with_default (
								path, 	
								MAGNIFIER_ZP_TOP, 	
								DEFAULT_MAGNIFIER_ZP_TOP);
    (magnifier->zp).width 	= gnopiconf_get_int_from_section_with_default (
								path, 	
								MAGNIFIER_ZP_WIDTH, 	
								size_x);
    (magnifier->zp).height 	= gnopiconf_get_int_from_section_with_default (
								path, 	
								MAGNIFIER_ZP_HEIGHT, 	
								size_y);
    magnifier->visible 		= gnopiconf_get_bool_from_section_with_default (
								path,	
								MAGNIFIER_VISIBLE, 	
								DEFAULT_MAGNIFIER_VISIBLE);
    magnifier->invert 		= gnopiconf_get_bool_from_section_with_default (
								path, 	
								MAGNIFIER_INVERT, 	
								DEFAULT_MAGNIFIER_INVERT);
    magnifier->panning 		= gnopiconf_get_bool_from_section_with_default (
								path, 	
								MAGNIFIER_PANNING, 	
								DEFAULT_MAGNIFIER_PANNING);

    magnifier->zoomfactorx	= gnopiconf_get_double_from_section_with_default (
								path, 	
								MAGNIFIER_ZOOM_FACTOR_X, 	
								DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY);
    magnifier->zoomfactory	= gnopiconf_get_double_from_section_with_default (
								path, 	
								MAGNIFIER_ZOOM_FACTOR_Y, 	
								DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY);
    magnifier->border_width 	= gnopiconf_get_int_from_section_with_default (
								path,	
								MAGNIFIER_BORDER_WIDTH, 	
								DEFAULT_MAGNIFIER_BORDER_WIDTH);
    magnifier->border_color 	= gnopiconf_get_int_from_section_with_default (
								path, 	
								MAGNIFIER_BORDER_COLOR, 	
								DEFAULT_MAGNIFIER_BORDER_COLOR);
    magnifier->cursor_state 	= gnopiconf_get_bool_from_section_with_default (
								path, 	
								MAGNIFIER_CURSOR, 	
								DEFAULT_MAGNIFIER_CURSOR);
    magnifier->cursor_name	= gnopiconf_get_string_from_section_with_default (
								    path, 	
								    MAGNIFIER_CURSOR_NAME, 	
								    DEFAULT_MAGNIFIER_CURSOR_NAME);
    magnifier->cursor_size  	= gnopiconf_get_int_from_section_with_default 	(
								path, 	
								MAGNIFIER_CURSOR_SIZE, 	
								DEFAULT_MAGNIFIER_CURSOR_SIZE);

    magnifier->cursor_scale   	= gnopiconf_get_bool_from_section_with_default (
								path, 	
								MAGNIFIER_CURSOR_SCALE, 	
								DEFAULT_MAGNIFIER_CURSOR_SCALE);
    magnifier->cursor_color   	= gnopiconf_get_int_from_section_with_default (
								path, 	
								MAGNIFIER_CURSOR_COLOR, 	
								DEFAULT_MAGNIFIER_CURSOR_COLOR);

    magnifier->zoomfactor_lock  = gnopiconf_get_bool_from_section_with_default (
								path, 	
								MAGNIFIER_ZOOM_FACTOR_LOCK,
								DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK);
    magnifier->smoothing  	= gnopiconf_get_string_from_section_with_default (
								path, 	
								MAGNIFIER_SMOOTHING,	
								DEFAULT_MAGNIFIER_SMOOTHING);
    magnifier->crosswire_clip  	= gnopiconf_get_bool_from_section_with_default (
								path, 	
								MAGNIFIER_CROSSWIRE_CLIP,	
								DEFAULT_MAGNIFIER_CROSSWIRE_CLIP);
    magnifier->crosswire_size  	= gnopiconf_get_int_from_section_with_default (
								path, 	
								MAGNIFIER_CROSSWIRE_SIZE,	
								DEFAULT_MAGNIFIER_CROSSWIRE_SIZE);
    magnifier->crosswire_color  = gnopiconf_get_int_from_section_with_default (
								path, 	
								MAGNIFIER_CROSSWIRE_COLOR,
								DEFAULT_MAGNIFIER_CROSSWIRE_COLOR);
    magnifier->crosswire  	= gnopiconf_get_bool_from_section_with_default (
								path, 	
								MAGNIFIER_CROSSWIRE,	
								DEFAULT_MAGNIFIER_CROSSWIRE);
    magnifier->mouse_tracking  	= gnopiconf_get_string_from_section_with_default (
								path, 	
								MAGNIFIER_MOUSE_TRACKING,	
								DEFAULT_MAGNIFIER_MOUSE_TRACKING);    
    magnifier->alignment_x 	= gnopiconf_get_string_from_section_with_default (
								path,
								MAGNIFIER_ALIGNMENT_X,
								DEFAULT_MAGNIFIER_ALIGNMENT_X);
    magnifier->alignment_y 	= gnopiconf_get_string_from_section_with_default (
								path,
								MAGNIFIER_ALIGNMENT_Y,
								DEFAULT_MAGNIFIER_ALIGNMENT_Y);

    magnifier->tracking  	= gnopiconf_get_string_from_section_with_default (
								path, 	
								MAGNIFIER_TRACKING,	
								DEFAULT_MAGNIFIER_TRACKING);    
    
    magnifier->zoomfactory = MAGCONF_ZOOM_FACTOR_LIMIT (magnifier->zoomfactory);
    magnifier->zoomfactorx = MAGCONF_ZOOM_FACTOR_LIMIT (magnifier->zoomfactorx);
    
    g_free (path);
    return magnifier;
}

gboolean
magconf_set_setting_in_schema (	const gchar *id,
				const gchar *schema,
				const gchar *key,
				GConfValueType data_type,
				gpointer data)
{
    gchar *path;
    
    sru_return_val_if_fail (key, FALSE);
    sru_return_val_if_fail (id, FALSE);

    path = g_strdup_printf ("%s/%s/%s", 
			    CONFIG_PATH MAGNIFIER_PATH, 
			    schema, 
			    id);    
    g_message ("setting gconf key %s", key);

    
    switch (data_type)
    {
	case GCONF_VALUE_STRING:
	    gnopiconf_set_string_in_section ((gchar*)data, path, key);
	    break;
	case GCONF_VALUE_INT:
	    gnopiconf_set_int_in_section (*((gint*)data), path, key);
	    break;
	case GCONF_VALUE_BOOL:
	    gnopiconf_set_bool_in_section (*((gboolean*)data), path, key);
	    break;
	case GCONF_VALUE_FLOAT:
	    gnopiconf_set_double_in_section (*((gdouble*)data), path, key);
	    break;
	case GCONF_VALUE_LIST:
	default:
	    break;
    }
    g_free (path);
    
    return TRUE;
}

gint
magconf_get_color_old (const gchar *schema,
		       const gchar *key)
{
    gchar *path;
    gint data;
    
    sru_return_val_if_fail (key, FALSE);
    sru_return_val_if_fail (schema, FALSE);

    if (magnifier_setting->crosswire_color) 
	data = magnifier_setting->crosswire_color;
    else
	data = 0;

    path = g_strdup_printf ("%s/%s/generic_zoomer", 
			    CONFIG_PATH MAGNIFIER_PATH, 
			    schema);
    
    data = gnopiconf_get_int_from_section_with_default (path, key, data);
    g_free (path);
    
    return data;
}


gboolean
magconf_save_item_in_schema (const gchar *schema,
			    GConfValueType type,
			    const gchar    *key,
			    gpointer data)
{
    gchar *path = NULL;
    
    sru_return_val_if_fail (key, FALSE);
    sru_return_val_if_fail (schema, FALSE);
    
    path = g_strdup_printf ("%s/%s/generic_zoomer", 
			    CONFIG_PATH MAGNIFIER_PATH, 
			    schema); 
    
    switch (type)
    {
	case GCONF_VALUE_BOOL:
		gnopiconf_set_bool_in_section (*((gboolean*)data), 		
						path, 	
						key);
	    break;
	case GCONF_VALUE_STRING:
	        gnopiconf_set_string_in_section ((gchar*)data, 		
						path, 	
						key);
	    break;
	case GCONF_VALUE_INT:
		gnopiconf_set_int_in_section (*((gint*)data), 		
						path, 	
						key);
	    break;
	case GCONF_VALUE_FLOAT:
		gnopiconf_set_double_in_section (*((gdouble*)data), 		
						 path, 	
						 key);
	    break;
	default:
	 break;
    }

    g_free (path);
    
    return TRUE;
}

gboolean
magconf_save_zoomer_in_schema (const gchar *schema,
			       const Magnifier *magnifier)
{
    gchar *path;
    sru_return_val_if_fail (magnifier, FALSE);
    sru_return_val_if_fail (magnifier->id, FALSE);
    sru_return_val_if_fail (schema, FALSE);

    path = g_strdup_printf ("%s/%s/%s", 
			    CONFIG_PATH MAGNIFIER_PATH, 
			    schema, 
			    magnifier->id);
    
    
    gnopiconf_set_string_in_section (magnifier->id, path, 	
					MAGNIFIER_ID);
    gnopiconf_set_string_in_section (magnifier->source , path, 	
					MAGNIFIER_SOURCE);
    gnopiconf_set_string_in_section (magnifier->target , path, 	
					MAGNIFIER_TARGET);

    gnopiconf_set_int_in_section ((magnifier->zp).left, path, 	
				    MAGNIFIER_ZP_LEFT);
    gnopiconf_set_int_in_section ((magnifier->zp).top, 	path, 	
				    MAGNIFIER_ZP_TOP);
    gnopiconf_set_int_in_section ((magnifier->zp).width, path, 	
				    MAGNIFIER_ZP_WIDTH);
    gnopiconf_set_int_in_section ((magnifier->zp).height, path, 	
				    MAGNIFIER_ZP_HEIGHT);
			
    gnopiconf_set_bool_in_section (magnifier->visible, 	path, 	
				    MAGNIFIER_VISIBLE);
    gnopiconf_set_bool_in_section (magnifier->invert, 	path, 	
				    MAGNIFIER_INVERT);
    gnopiconf_set_bool_in_section (magnifier->panning, 	path, 	
				    MAGNIFIER_PANNING);

			
    gnopiconf_set_double_in_section (magnifier->zoomfactorx, path, 	
				     MAGNIFIER_ZOOM_FACTOR_X);
    gnopiconf_set_double_in_section (magnifier->zoomfactory, path, 	
				     MAGNIFIER_ZOOM_FACTOR_Y);
    gnopiconf_set_bool_in_section (magnifier->zoomfactor_lock, path, 
				    MAGNIFIER_ZOOM_FACTOR_LOCK);

    gnopiconf_set_int_in_section (magnifier->border_width, path, 	
				    MAGNIFIER_BORDER_WIDTH);
    gnopiconf_set_int_in_section ((gint)magnifier->border_color, path, 	
				    MAGNIFIER_BORDER_COLOR);
			
    gnopiconf_set_bool_in_section (magnifier->cursor_state, path, 	
				    MAGNIFIER_CURSOR);
    gnopiconf_set_string_in_section (magnifier->cursor_name, path, 	
				    MAGNIFIER_CURSOR_NAME);
    gnopiconf_set_int_in_section (magnifier->cursor_size, path, 	
				    MAGNIFIER_CURSOR_SIZE);
    gnopiconf_set_bool_in_section (magnifier->cursor_scale, path, 	
				    MAGNIFIER_CURSOR_SCALE);

    gnopiconf_set_int_in_section ((gint)magnifier->cursor_color, path, 	
				    MAGNIFIER_CURSOR_COLOR);

    gnopiconf_set_string_in_section (magnifier->smoothing, path, 	
				    MAGNIFIER_SMOOTHING);
    gnopiconf_set_bool_in_section (magnifier->crosswire_clip, path, 
				    MAGNIFIER_CROSSWIRE_CLIP);
    gnopiconf_set_int_in_section ((gint)magnifier->crosswire_color, path, 
				    MAGNIFIER_CROSSWIRE_COLOR);
    gnopiconf_set_int_in_section (magnifier->crosswire_size, path, 
				    MAGNIFIER_CROSSWIRE_SIZE);
    gnopiconf_set_bool_in_section (magnifier->crosswire, path, 	
				    MAGNIFIER_CROSSWIRE);
			
    gnopiconf_set_string_in_section (magnifier->mouse_tracking, path, 
					MAGNIFIER_MOUSE_TRACKING);
    gnopiconf_set_string_in_section (magnifier->alignment_x, path, 
					MAGNIFIER_ALIGNMENT_X);
    gnopiconf_set_string_in_section (magnifier->alignment_y, path, 
					MAGNIFIER_ALIGNMENT_Y);
    gnopiconf_set_string_in_section (magnifier->tracking, path, 
					MAGNIFIER_TRACKING);
    
    g_free (path);
    
    return TRUE;
}

void
magconf_get_display_size (gint *size_x,
			  gint *size_y)
{
    *size_x = gdk_screen_width () - 1;
    *size_y = gdk_screen_height () - 1;
}

void
magconf_changes_cb			(GConfClient	*client,
					guint		cnxn_id,
					GConfEntry	*entry,
					gpointer	user_data)
{	
	gchar *key = NULL;
	key = (gchar*) gconf_entry_get_key (entry);

	sru_return_if_fail (entry);
	if (!magnifier_setting)
	    return;
	if (!entry->value)
	    return;
		
	sru_debug ("magconf_changes_cb:Entry key:%s",gconf_entry_get_key(entry));
	sru_debug ("magconf_changes_cb:Entry value type:%i",entry->value->type);
	switch(entry->value->type)
	{
    	    case GCONF_VALUE_INT:    sru_debug ("magconf_changes_cb:Entry value:%i",gconf_value_get_int(entry->value));break;
    	    case GCONF_VALUE_FLOAT:  sru_debug ("magconf_changes_cb:Entry value:%f",gconf_value_get_float(entry->value));break;
    	    case GCONF_VALUE_STRING: sru_debug ("magconf_changes_cb:Entry value:%s",gconf_value_get_string(entry->value));break;
    	    case GCONF_VALUE_BOOL:   sru_debug ("magconf_changes_cb:Entry value:%i",gconf_value_get_bool(entry->value));break;
	    default: sru_debug ("magconf_changes_cb:Other Entry value:");break;	
	}

	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_ZOOM_FACTOR_LOCK) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_BOOL)
	    {
		magnifier_setting->zoomfactor_lock = 
		    gconf_value_get_bool (entry->value);
		magui_set_zoomfactor_lock (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_ZOOM_FACTOR_X) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_FLOAT)
	    {
		magnifier_setting->zoomfactorx = 
		    gconf_value_get_float (entry->value);
		magui_set_zoomfactor_x(magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_ZOOM_FACTOR_Y) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_FLOAT)
	    {
		magnifier_setting->zoomfactory = 
		    gconf_value_get_float (entry->value);
		magui_set_zoomfactor_y(magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_ZP_LEFT) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
		(magnifier_setting->zp).left = 
		    gconf_value_get_int (entry->value);
		magui_set_zp_left (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_ZP_TOP) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
	        (magnifier_setting->zp).top = 
		    gconf_value_get_int (entry->value);
		magui_set_zp_top (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_ZP_HEIGHT) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
		(magnifier_setting->zp).height = 
		    gconf_value_get_int (entry->value);
		magui_set_zp_height (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_ZP_WIDTH) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
		(magnifier_setting->zp).width = 
		    gconf_value_get_int (entry->value);
		magui_set_zp_width (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CURSOR) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_BOOL)
	    {
	        magnifier_setting->cursor_state = 
		    gconf_value_get_bool (entry->value);
		magui_set_cursor (magnifier_setting);
    	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CURSOR_SIZE) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
	        magnifier_setting->cursor_size = 
		    gconf_value_get_int (entry->value);
		magui_set_cursor_size (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CURSOR_NAME) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_STRING)
	    {
		g_free (magnifier_setting->cursor_name);
		magnifier_setting->cursor_name = 
		    g_strdup (gconf_value_get_string (entry->value));
	    }
	}
	else    
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CURSOR_SCALE) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_BOOL)
	    {
	        magnifier_setting->cursor_scale = 
		    gconf_value_get_bool (entry->value);
		magui_set_cursor_mag (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CURSOR_COLOR) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
		magnifier_setting->cursor_color = 
		    gconf_value_get_int (entry->value);
		magui_set_cursor_color (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_INVERT) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_BOOL)
	    {
	        magnifier_setting->invert = 
		    gconf_value_get_bool (entry->value);
		magui_set_invert (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_PANNING) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_BOOL)
	    {
	        magnifier_setting->panning = 
		    gconf_value_get_bool (entry->value);
		magui_set_panning (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CROSSWIRE_SIZE) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
		magnifier_setting->crosswire_size = 
		    gconf_value_get_int (entry->value);
		magui_set_crosswire_size (magnifier_setting);
	    }
	}
	else    
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CROSSWIRE_CLIP) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_BOOL)
	    {
		magnifier_setting->crosswire_clip = 
		    gconf_value_get_bool (entry->value);
		magui_set_crosswire_clip (magnifier_setting);
	    }
	}
	else    
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CROSSWIRE) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_BOOL)
	    {
		magnifier_setting->crosswire = 
		    gconf_value_get_bool (entry->value);
		magui_set_crosswire (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_CROSSWIRE_COLOR) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
		magnifier_setting->crosswire_color = 
		    gconf_value_get_int (entry->value);
		magui_set_crosswire_color (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_SMOOTHING) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_STRING)
	    {
	    	g_free (magnifier_setting->smoothing);
		magnifier_setting->smoothing = 
		    g_strdup (gconf_value_get_string (entry->value));
		magui_set_smoothing (magnifier_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_MOUSE_TRACKING) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_STRING)
	    {
		g_free (magnifier_setting->mouse_tracking);
		magnifier_setting->mouse_tracking = 
		    g_strdup (gconf_value_get_string (entry->value));
		magui_set_mouse_tracking (magnifier_setting);
	    }
	}
/*	if (strcmp (key, CONFIG_PATH MAGNIFIER_PATH MAGNIFIER_SCHEMA1 MAGNIFIER_FOCUS_TRACKING) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_STRING)
	    {
		g_free (magnifier_setting->focus_tracking);
		magnifier_setting->focus_tracking = 
		    g_strdup (gconf_value_get_string (entry->value));
		magui_set_focus_tracking (magnifier_setting);
	    }
	}
*/
}
