/* srctrl.h
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

#ifndef _SRC_CTRL_H_
#define _SRC_CTRL_H_

#include <glib.h>
#include "libsrconf.h"

gboolean src_cmd_queue_add 	(gchar *cmd);
gboolean src_cmd_queue_remove 	(gchar **cmd);
gboolean src_cmd_queue_process 	();
gboolean src_key_queue_add 	(gchar *key);
gboolean src_key_queue_remove 	(gchar **key);

gboolean src_ctrl_process_key 	(gchar *key);
gboolean src_ctrl_init 		();
gboolean src_ctrl_terminate 	();
gboolean src_ctrl_flat_mode_terminate();

gboolean src_ctrl_position_sensor_action (gint index);
gboolean src_ctrl_optical_sensor_action (gint index);

void 	  src_update_key ();

/**
 *
 * src_key_get_from_gconf
 * Get key list from gconf file
 * return - TRUE if had no error.
 * list	  - readed GSList 
 *
**/
gboolean
src_key_get_from_gconf 	(GSList **list);

/**
 *
 * src_key_delete_from_gconf ()
 * remove a src_key entries from gconf.
 * return - TRUE if succed the remove.(No errors);
 * 
**/
gboolean
src_key_delete_from_gconf ();

gboolean
src_key_load_list ();

#endif /* _SRC_CTRL_H_ */
