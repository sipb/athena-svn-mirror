/* brlconf.h
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

#ifndef __BRAILLE__
#define __BRAILLE__ 0


#include <glib.h>
#include "libsrconf.h"

/**
 *Braille Setting Structure					
**/
typedef struct
    {
	gint 	port_no;
	gchar 	*translation_table;
	gchar  	*braille_style;
	gchar 	*cursor_style;
	gint  	attributes;
	gchar   *fill_char;  /* changeable */
	gchar	*status_cell;
	gint	optical;
	gint	position;
    } Braille;

Braille* 	brlconf_setting_init 		(gboolean set_struct);
Braille* 	brlconf_setting_new 		(void);
void 		brlconf_load_default_settings	(Braille* braille);
void		brlconf_setting_clean		(Braille* braille);
void 		brlconf_setting_free 		(Braille* braille);
void 		brlconf_terminate		(Braille *braille);
gboolean 	brlconf_gconf_client_init 	(void);
void		brlconf_device_list_free 	(void);
/**
 * Set Methods 							
**/
void 		brlconf_setting_set 		(const Braille *braille);
void 		brlconf_style_set 		(const gchar *style);
void 		brlconf_cursor_style_set 	(const gchar *cursor);
void 		brlconf_translation_table_set 	(const gchar *table);
void 		brlconf_attributes_set 		(gint dots);
void 		brlconf_device_set 		(const gchar *device);
void 		brlconf_fill_char_set 		(const gchar *texture);
void 		brlconf_port_no_set 		(gint port_no);
void 		brlconf_optical_sensor_set 	(gint sensor);
void 		brlconf_position_sensor_set 	(gint sensor);
void 		brlconf_status_cell_set 	(const gchar *status);

/**
 * Get Methods 
**/
gchar* 		brlconf_style_get 		(void);
gchar* 		brlconf_cursor_style_get 	(void);
gchar* 		brlconf_translation_table_get 	(void);
gint 		brlconf_attributes_get 		(void);
gchar* 		brlconf_device_get 		(void);
gchar* 		brlconf_fill_char_get 		(void);
gint 		brlconf_port_no_get 		(void);
gint 		brlconf_optical_sensor_get 	(void);
gint 		brlconf_position_sensor_get 	(void);
gchar* 		brlconf_status_cell_get 	(void);
void		brlconf_active_device_list_get 	(void);

const gchar*	brlconf_device_id_return 	(const gchar *device);
const gchar*	brlconf_return_active_device 	(void);
#endif
