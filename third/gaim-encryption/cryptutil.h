/*        Misc utility functions for the Gaim-Encryption plugin           */
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

#ifndef CRYPTUTIL_H
#define CRYPTUTIL_H

#include "debug.h"
#include "gaim.h"

#define MSG_HUNK_SIZE 126
#define CRYPT_HUNK_SIZE 256

/* Utility Functions: */

/* Convert a byte array to ascii-encoded character array.                     */
void GE_bytes_to_str(unsigned char* str, unsigned char* bytes, int numbytes);

/* Convert a byte array to hex like a5:38:49:...   .                          */
/* returns number of chars in char array.  No null termination!               */
/* int GE_bytes_to_colonstr(unsigned char* hex, unsigned char* bytes, int numbytes); */

/* Convert ascii-encoded bytes in a null terminated char* into a byte array */
unsigned int GE_str_to_bytes(unsigned char* bytes, unsigned char* hex);

/* Strip returns from a block encoded string */
GString* GE_strip_returns(GString* s);

/* Zero out a string (use for plaintext before freeing memory) */
void GE_clear_string(unsigned char* s);

/* Escape all spaces in name so it can go in a key file */
void GE_escape_name(GString* name);

/* Reverse the previous escaping.  Since it will only get shorter, allow char* */
void GE_unescape_name(char* name);

/* Returns true if the message starts with an HTML link */
gboolean GE_msg_starts_with_link(const char* c);

/* Split a message (hopefully on a space) so we can send it in pieces */
GSList *GE_message_split(char *message, int limit);
#endif
