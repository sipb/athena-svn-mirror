/*             Nonces for the Gaim encryption plugin                      */
/*            Copyright (C) 2001-2003 William Tompkins                    */

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

#ifndef NONCE_H
#define NONCE_H

#include "glib.h"

typedef unsigned char Nonce[24];

void GE_nonce_map_init();

void GE_str_to_nonce(Nonce* nonce, unsigned char* nonce_str);
gchar* GE_nonce_to_str(Nonce* nonce);

void GE_incr_nonce(Nonce* nonce);

gchar* GE_new_incoming_nonce(const char* name);
int GE_check_incoming_nonce(const char* name, char* nonce_str);

int GE_nonce_str_len();

#endif
