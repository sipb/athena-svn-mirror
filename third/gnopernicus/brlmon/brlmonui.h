/* brlmonui.h
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

#ifndef _BRLMONUI_
#define _BRLMONUI_

#include 	<gnome.h>
#include 	<glade/glade.h>
#include	<gconf/gconf-value.h>

#define BRAILLE_MONITOR_PATH 			"/apps/gnopernicus/brlmon"
#define BRAILLE_MONITOR_PORT_GCONF_KEY 		"port"
#define BRAILLE_MONITOR_MODE_GCONF_KEY		"display_mode"
#define BRAILLE_MONITOR_USE_THEME_GCONF_KEY	"use_theme_color"

#define BRAILLE_MONITOR_DOT7_GCONF_KEY 	 	"dot7"
#define BRAILLE_MONITOR_DOT8_GCONF_KEY 	 	"dot8"
#define BRAILLE_MONITOR_DOT78_GCONF_KEY 	"dot78"

#define BRAILLE_MONITOR_PANEL_POSITION_GCONF_KEY "brlmon_position"
#define BRAILLE_MONITOR_IS_PANEL_GCONF_KEY 	"is_brlmon_panel"
#define BRAILLE_MONITOR_COLUMN_GCONF_KEY	"cell_column"
#define BRAILLE_MONITOR_LINE_GCONF_KEY		"cell_line"

#define BRAILLE_MONITOR_FONT_SIZE_GCONF_KEY	"font_size"

#define DEFAULT_BRAILLE_MONITOR_PANEL_POSITION   "TOP"
#define DEFAULT_MODE				 "NORMAL"
#define DEFAULT_DOT78_COLOR			"#000000007D00"
#define DEFAULT_DOT7_COLOR			"#7D0000000000"
#define DEFAULT_DOT8_COLOR			"#00007D000000"
#define DEFAULT_LINE				2
#define DEFAULT_COLUMN				40

/**
 *
 * Max numbers of cells on display
 *
**/
#define PANEL_LENGHT 2 * 120


#define DEFAULT_PORT	7000

/**
 *
 * Type of dots
 *
**/

typedef enum
{
    DOTNONE = 0,
    DOT78,
    DOT7,
    DOT8
} DotType;

typedef enum 
{
    ROLE_MAIN,
    ROLE_STATUS,
    ROLE_OTHER
} DisplayRole;

typedef enum
{
    MODE_NORMAL = 0,
    MODE_BRAILLE,
    MODE_DUAL,
    MODE_TYPE_NUMBER
} ModeType;

ModeType brlmon_get_mode_type_from_string (const gchar *mode);
gboolean brlmon_load_colors (void);
/**
 *
 * Move cursor in 1st cell
 *
**/
void brlmon_cursor_pos_clean (void);

/**
 *
 * Clean old_pos
 *
**/
void brlmon_old_pos_clean (void);

/**
 *
 * Clean display
 *
**/
void brlmon_clean_panel	(void);

/**
 *
 * Show text on display from current position
 * text - text to show
 * role - role of tag (Status of text)
 *
**/
void brlmon_print_text_from_cur_pos (gchar *text, DisplayRole role);

/**
 *
 * Create display table.
 *
**/
void brlmon_create_text_area	(gint line, gint column);

/**
 *
 * Set cursor pos on display
 * pos - position on display
 *
**/
void brlmon_cursor_pos	(gint pos);

/**
 *
 * Set dot type.
 *
**/
void brlmon_set_typedot (DotType val);

/**
 *
 * APIs with configuration files
 *
**/
gboolean brlmon_gconf_client_init (void);

gboolean brlmon_set_int (gint val, const gchar *key);
gint 	 brlmon_get_int_with_default (const gchar *key, gint def);
gboolean brlmon_set_string (const gchar *val, const gchar *key);
gchar* 	 brlmon_get_string_with_default (const gchar *key, gchar *def);

/**
 *
 * Load glade interface
 *
**/
gboolean brlmon_load_interface	(void);
#endif
