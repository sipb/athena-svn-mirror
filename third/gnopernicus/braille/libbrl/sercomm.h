/* sercomm.h
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

#ifndef __SERCOMM_H__
#define __SERCOMM_H__

#include <glib.h>

typedef gshort (*BRLSerCallback) (gint byte);

/* Internal */
void brl_ser_sig_alarm       (gint   sig);
gint  brl_ser_read_data      (gchar  *data_buff,
			     gint    max_len);

/* API */
gint  brl_ser_open_port      (gint   port);
gint  handy_set_comm_param   ();            /* for HandyTech devices */
gint  brl_ser_set_comm_param (glong  baud_rate,
			     gchar   parity,
			     gshort  stop_bits,
			     gchar   flow_ctrl);
gint  brl_ser_close_port     ();

gint  brl_ser_start_timer    (glong interval);
gint  brl_ser_stop_timer     ();

gint  brl_ser_init_glib_poll ();
gint  brl_ser_exit_glib_poll ();

gint  brl_ser_send_data      (gchar *data,
			     gint   data_size,
			     gshort blocking);
void brl_ser_set_callback    (BRLSerCallback callback);

#endif
