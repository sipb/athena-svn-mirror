/*  Key acceptance UI bits                                                */
/*             Copyright (C) 2001-2003 William Tompkins                   */

/* This plugin is free software, distributed under the GNU General Public */
/* License.                                                               */
/* Please see the file "COPYING" distributed with the Gaim source code    */
/* for more details                                                       */
/*                                                                        */
/*                                                                        */
/*    This software is distributed in the hope that it will be useful,    */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*   General Public License for more details.                             */

/*   To compile and use:                                                  */
/*     See INSTALL file.                                                  */

#ifndef KEY_UI_H
#define KEY_UI_H

#include "keys.h"

void GE_choose_accept_unknown_key(key_ring_data* newkey, gchar* resend_msg_id, GaimConversation *conv);
void GE_choose_accept_conflict_key(key_ring_data* newkey, gchar* resend_msg_id, GaimConversation *conv);

#endif
