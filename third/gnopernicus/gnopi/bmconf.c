/* bmconf.c
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
#include "bmconf.h"
#include "bmui.h"
#include "SRMessages.h"
#include "gnopiconf.h"

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>
#include "srintl.h"


extern GConfClient *gnopernicus_client;

/**
* bmconf_gconf_client_init
*
* Add BRAILLE_MONITOR path to gconf listener .
*
* return: TRUE at success
*
**/
gboolean
bmconf_gconf_client_init (void)
{
    sru_return_val_if_fail (gnopiconf_client_add_dir (CONFIG_PATH BRAILLE_MONITOR_PATH), FALSE);
    	
    return TRUE;
}

/**
*
* bmconf_load_default_settings 
*
* Set with default value for Braille Monitor
* 
* return:
*
**/
void 
bmconf_load_default_settings (void)
{
    GConfValue  *value = NULL;
    gint line;
    gint column;
    gchar *position;
    gchar *mode;
    gchar *dot7;
    gchar *dot8;
    gchar *dot78;
    gboolean use_theme_color;
        
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_COLUMN_KEY, 
						NULL);
    
    if (value)
    {
    	column = gconf_value_get_int (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	column = DEFAULT_BRAILLE_MONITOR_COLUMN;
	
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_LINE_KEY, 
						NULL);
    
    if (value)
    {
    	line = gconf_value_get_int (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	line = DEFAULT_BRAILLE_MONITOR_LINE;
	
    bmconf_size_set (line, column);
    
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_PANEL_POSITION_KEY, 
						NULL);
    
    if (value)
    {
    	position = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	position = g_strdup (DEFAULT_BRAILLE_MONITOR_PANEL_POSITION);

    bmconf_position_set (position);
    
    g_free (position);
    
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_USE_THEME_KEY, 
						NULL);
    
    if (value)
    {
    	use_theme_color = gconf_value_get_bool (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	use_theme_color = DEFAULT_BRAILLE_MONITOR_USE_THEME;

    bmconf_use_theme_color_set (use_theme_color);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_MODE_KEY, 
						NULL);
    
    if (value)
    {
    	mode = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	mode = g_strdup (DEFAULT_BRAILLE_MONITOR_MODE);

    bmconf_display_mode_set (mode);
    
    g_free (mode);
    
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT7_KEY, 
						NULL);
    
    if (value)
    {
    	dot7 = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	dot7 = g_strdup (DEFAULT_BRAILLE_MONITOR_DOT7_COLOR);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT8_KEY, 
						NULL);
    
    if (value)
    {
    	dot8 = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	dot8 = g_strdup (DEFAULT_BRAILLE_MONITOR_DOT8_COLOR);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT78_KEY, 
						NULL);
    
    if (value)
    {
    	dot78 = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	dot78 = g_strdup (DEFAULT_BRAILLE_MONITOR_DOT78_COLOR);

    bmconf_colors_set (dot7, dot8, dot78);
    
    g_free (dot7);
    g_free (dot8);
    g_free (dot78);
}


/**
* bmconf_size_get
*
* Return size of braille monitor.
*
* return:
* @line: number of lines in table
* @column: number of columns in table
*
**/
void
bmconf_size_get (gint *line, gint *column)
{
    *column = gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_COLUMN_KEY, 
					      DEFAULT_BRAILLE_MONITOR_COLUMN);
    *line   = gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_LINE_KEY, 
					      DEFAULT_BRAILLE_MONITOR_LINE);
}

/**
* bmconf_position_get
*
* Return position of braille monitor.
*
* return: "TOP" or "BOTTOM"
*
**/
gchar*
bmconf_position_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_PANEL_POSITION_KEY, 
					      DEFAULT_BRAILLE_MONITOR_PANEL_POSITION);
}

/**
* bmconf_use_theme_color_get
*
* Return if use theme color.
*
* return: TRUE or FALSE
*
**/
gboolean
bmconf_use_theme_color_get (void)
{
    return gnopiconf_get_bool_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_USE_THEME_KEY, 
					      DEFAULT_BRAILLE_MONITOR_USE_THEME);
}

/**
* bmconf_display_mode_get
*
* Return display mode of braille monitor.
*
* return: "NORMAL", "BRAILLE" or "DUAL"
*
**/
gchar*
bmconf_display_mode_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_MODE_KEY, 
					      DEFAULT_BRAILLE_MONITOR_MODE);
}

/**
* bmconf_colors_get
*
* @*dot7: Return color for dot7
* @*dot8: Return color for dot8
* @*dot78: Return color for dot78
*
* Return colors for dot types.
*
* return:
*
**/
void
bmconf_colors_get (gchar **dot7, gchar **dot8, gchar **dot78)
{
    *dot7 = gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT7_KEY, 
					    DEFAULT_BRAILLE_MONITOR_DOT7_COLOR);
    *dot8 = gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT8_KEY, 
					    DEFAULT_BRAILLE_MONITOR_DOT8_COLOR);
    *dot78 = gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT78_KEY, 
					     DEFAULT_BRAILLE_MONITOR_DOT78_COLOR);
}


/**
* bmconf_font_size_get
*
* Get mode of window.
*
* return:
*
**/
gint
bmconf_font_size_get (void)
{
    return gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_FONT_SIZE_KEY, 
					    DEFAULT_BRAILLE_MONITOR_FONT_SIZE);
}


/**
* bmconf_size_set
*
* @line: number of lines in table
* @column: number of columns in table
*
* Set size of braille monitor.
*
* return:
*
**/
void
bmconf_size_set (gint line, gint column)
{
    if (!gnopiconf_set_int (column, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_COLUMN_KEY) ||
        !gnopiconf_set_int (line, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_LINE_KEY))
	sru_warning (_("Failed to set display size."));

}

/**
* bmconf_position_set
*
* @position: Position on screen.
*
* Set position of braille monitor.
*
* return:
*
**/
void
bmconf_position_set (const gchar *position)
{
    if (!gnopiconf_set_string (position, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_PANEL_POSITION_KEY))
	sru_warning (_("Failed to set display position."));
}

/**
* bmconf_use_theme_color_set
*
* Use theme color.
*
* return:
*
**/
void
bmconf_use_theme_color_set (gboolean use_theme)
{
    if (!gnopiconf_set_bool (use_theme, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_USE_THEME_KEY))
	sru_warning ("Failed to set \"use system colors\" flag.");
}

/**
* bmconf_display_mode_set
*
* @mode: Display mode. ("NORMAL", "BRAILLE" or "DUAL")
*
* Set display mode of braille monitor.
*
* return:
*
**/
void
bmconf_display_mode_set (const gchar *mode)
{
    if (!gnopiconf_set_string (mode, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_MODE_KEY))
	sru_warning (_("Failed to set display mode."));
}

/**
* bmconf_colors_set
*
* @*dot7: Color for dot7
* @*dot8: Color for dot8
* @*dot78: Color for dot78
*
* Set colors for dot types.
*
* return:
*
**/
void
bmconf_colors_set ( const gchar *dot7, 
		    const gchar *dot8, 
		    const gchar *dot78)
{
    if (!gnopiconf_set_string (dot7, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT7_KEY) ||
	!gnopiconf_set_string (dot8, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT8_KEY) ||
	!gnopiconf_set_string (dot78, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_DOT78_KEY))
    {
	sru_warning (_("Failed to set dot type colors."));
    }
}

/**
* bmconf_font_size_set
*
* @font_size:
*
* return:
*
**/
void
bmconf_font_size_set (gint font_size)
{
    if (!gnopiconf_set_int (font_size, CONFIG_PATH BRAILLE_MONITOR_KEY_PATH BRAILLE_MONITOR_FONT_SIZE_KEY))
    {
	sru_warning (_("Failed to set font size."));
    }
}

