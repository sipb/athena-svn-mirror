/* srbrl.h
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

#ifndef _SRBRL_H_
#define _SRBRL_H_

#include <glib.h>


#include "srmain.h"

#define MAX_BRL_ITEMS 	62
#define MAX_ROLE_ITEMS 	7
#define BUFFER_SIZE 	256

typedef struct
{
    gint 	eventtype;
    gchar 	source[BUFFER_SIZE];
}BrlPackage;

gboolean src_braille_init 	();
void src_braille_terminate();
gboolean src_braille_restart();
void src_braille_send 	(gchar *brloutput);
void src_braille_show 	(gchar *message);

gboolean src_braille_translation_table_exist (const gchar *table);

gboolean src_braille_monitor_init ();
void src_braille_monitor_terminate();
void src_brlmon_send	(gchar *monoutput);
void src_brlmon_show 	(gchar *message);

void src_brlmon_init	();
void src_brlmon_quit	();
void src_brlmon_terminate();

void	 src_braille_get_defaults	   ();
gboolean src_braille_set_device 	   (gchar *device);
gboolean src_braille_set_style 		   (gchar *style);
gboolean src_braille_set_cursor_style 	   (gchar *cursor_style);
gboolean src_braille_set_translation_table (gchar *translation_table);

gboolean src_braille_set_port_no 	   (gint port);
gboolean src_braille_set_optical_sensor	   (gint optical_sensor);
gboolean src_braille_set_position_sensor   (gint position_sensor);
gboolean src_braille_set_offset 	   (gint offset);

/*Fixme -start*/

gchar* src_braille_get_device 		 ();
gchar* src_braille_get_style 		 ();
gchar* src_braille_get_cursor_style 	 ();
gchar* src_braille_get_translation_table ();

gint src_braille_get_port_no 	   	 ();
gint src_braille_get_optical_sensor	 ();
gint src_braille_get_position_sensor   	 ();
gint src_braille_get_offset 	   	 ();


/*Fixme -end*/

#endif /* _SRBRL_H_ */

