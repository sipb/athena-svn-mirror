/* srmain.h
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

#ifndef _SRMAIN_H_
#define _SRMAIN_H_

#include "glib.h"
#include "SRObject.h"

typedef gboolean (*SRCSampleFunction) ();

gchar* src_xml_process_string 	(gchar *str_);
gchar* src_xml_make_part 	(gchar *tag, gchar *attr, gchar *text);
gchar* src_xml_format 		(gchar *tag, gchar *attr, gchar *text);

gboolean src_present_crt_sro	   ();
gboolean src_present_crt_window	   ();
gboolean src_present_crt_tooltip   ();
gboolean src_present_last_message  ();
gboolean src_present_layer_timeout ();
gboolean src_present_layer_changed ();
gboolean src_present_details 	   ();
gboolean src_kb_key_echo	   ();
gboolean src_kb_punct_echo	   ();
gboolean src_kb_space_echo	   ();
gboolean src_kb_modifier_echo	   ();
gboolean src_kb_cursor_echo	   ();


gboolean src_say_message (const gchar *message);
gboolean login_time;


#endif /* _SRMAIN_H_ */
