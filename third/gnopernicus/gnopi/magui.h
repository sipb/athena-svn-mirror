/* magui.h
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
#ifndef __MAGUI__
#define __MAGUI__ 

#include "magconf.h"
#include <gnome.h>

gboolean 	magui_load_magnifier_settings_interface		(GtkWidget *parent_window);
gboolean	magui_magnifier_setting_value_add_to_widgets	(Magnifier *magnifier_setting);
gboolean	magui_load_magnification_options_interface	();
gboolean	magui_magnification_options_value_add_to_widgets (Magnifier *magnifier_setting);
gboolean	magui_load_mag_cursor_list_interface	  	();
gchar* 		magui_macro_to_file_name 		  	(gchar *name);
gboolean	magui_cursor_setting_value_add_to_widgets 	(Magnifier *magnifier_setting);
gboolean	magui_set_zoomfactor_x				(Magnifier *magnifier_setting);
gboolean	magui_set_zoomfactor_y				(Magnifier *magnifier_setting);
gboolean	magui_set_zoomfactor_lock			(Magnifier *magnifier_setting);
gboolean	magui_set_crosswire 				(Magnifier *magnifier_setting);
gboolean	magui_set_crosswire_clip 			(Magnifier *magnifier_setting);
gboolean	magui_set_crosswire_size 			(Magnifier *magnifier_setting);
gboolean	magui_set_crosswire_color 			(Magnifier *magnifier_setting);
gboolean	magui_set_cursor 				(Magnifier *magnifier_setting);
gboolean	magui_set_cursor_mag 				(Magnifier *magnifier_setting);
gboolean	magui_set_cursor_size 				(Magnifier *magnifier_setting);
gboolean	magui_set_cursor_color 				(Magnifier *magnifier_setting);
gboolean	magui_set_smoothing 				(Magnifier *magnifier_setting);
gboolean	magui_set_mouse_tracking 			(Magnifier *magnifier_setting);
gboolean	magui_set_invert 				(Magnifier *magnifier_setting);
gboolean	magui_set_panning 				(Magnifier *magnifier_setting);
gboolean	magui_set_border_size 				(Magnifier *magnifier_setting);
gboolean	magui_set_border_color 				(Magnifier *magnifier_setting);
gboolean	magui_set_zp_left 				(Magnifier *magnifier_setting);
gboolean	magui_set_zp_top 				(Magnifier *magnifier_setting);
gboolean	magui_set_zp_height 				(Magnifier *magnifier_setting);
gboolean	magui_set_zp_width 				(Magnifier *magnifier_setting);
#endif
