/* brlui.h
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

#ifndef __BRLUI__
#define __BRLUI__ 

#include <gnome.h>
#include "brlconf.h"

gboolean	brlui_load_braille_settings			(GtkWidget *parent_window);
/**
 * Braille device
**/
gboolean	brlui_load_braille_device			(Braille *braille_setting);
gboolean	brlui_braille_device_value_add_to_widgets	(Braille *braille_setting);

/**
 * Translation Table
**/
gboolean	brlui_load_translation_table			(Braille *braille_setting);
gboolean    	brlui_translation_table_value_add_to_widgets	(Braille *braille_setting);
/**
 * Braille Style
**/
gboolean	brlui_load_braille_style			(Braille *braille_setting);
gboolean    	brlui_braille_style_value_add_to_widgets	(Braille *braille_setting);

/**
 * Cursor Settings
**/
gboolean	brlui_load_cursor_settings			(Braille *braille_setting);
gboolean    	brlui_cursor_setting_value_add_to_widgets	(Braille *braille_setting);
/**
 * Attribute Settings
**/
gboolean	brlui_load_attribute_settings			(Braille *braille_setting);
gboolean    	brlui_attribute_setting_value_add_to_widgets	(Braille *braille_setting);
/**
 * Braille Fill Char
**/
gboolean	brlui_load_braille_fill_char			(Braille *braille_setting);
gboolean	brlui_braille_fill_char_value_add_to_widgets	(Braille *braille_setting);

/**
 * Status Cell
**/
gboolean	brlui_load_status_cell				(Braille *braille_setting);
gboolean    	brlui_status_cell_value_add_to_widgets		(Braille *braille_setting);

gboolean 	brlui_load_braille_key_mapping	(Braille *braille_setting);
gboolean 	brlui_braille_key_mapping_value_add_to_widgets (Braille *braille_setting);

#endif
