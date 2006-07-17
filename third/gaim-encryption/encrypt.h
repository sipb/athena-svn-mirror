/*                   Gaim encryption plugin                               */
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

#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <time.h>

#ifndef NO_CONFIG
#include "gaim-encryption-config.h"
#endif

#include <conversation.h>
#define ENC_WEBSITE "http://gaim-encryption.sourceforge.net"


typedef struct GE_SentMessage
{
   time_t time;
   gchar* id;
   gchar* msg;
} GE_SentMessage;


void GE_send_stored_msgs(GaimAccount*, const char *who);
void GE_delete_stored_msgs(GaimAccount*, const char *who);
void GE_show_stored_msgs(GaimAccount*, const char* who, char** return_msg);
void GE_resend_msg(GaimAccount*, const char* who, gchar*);

#endif
