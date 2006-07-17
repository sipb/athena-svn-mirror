/*     Fake wrapper to illustrate using a different encryption protocol   */
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

#include "cryptproto.h"
#include "gpg.h"
#include "cryptutil.h"
#include "keys.h"


char* gpg_proto_string="GPG 1.00";

crypt_proto* gpg_proto;

/*Functions exported through crypt_proto structure */
static int              gpg_encrypt(unsigned char** encrypted, unsigned char* msg, int msg_len,
                                    crypt_key* inkey);
static int              gpg_decrypt(unsigned char** decrypted, unsigned char* msg, int msg_len,
                                    crypt_key* inkey);
static int              gpg_sign(unsigned char** signedmsg, unsigned char* msg, int msg_len,
                                 crypt_key* key, crypt_key* tokey);
static int              gpg_auth(unsigned char** authed, unsigned char* msg, int msg_len,
                                 crypt_key* key, const char* name);
static crypt_key*       gpg_make_key_from_str(unsigned char *key_str);
static GString*         gpg_key_to_gstr(crypt_key* inkey);
static unsigned char*   gpg_parseable(unsigned char* key);

void gpg_init(int isdefault) {
	gpg_proto = g_malloc(sizeof(crypt_proto));
	crypt_proto_list = g_slist_prepend(crypt_proto_list, gpg_proto);

   gpg_proto->encrypt = gpg_encrypt;
   gpg_proto->decrypt = gpg_decrypt;
   gpg_proto->sign = gpg_sign;
   gpg_proto->auth = gpg_auth;
   gpg_proto->make_key_from_str = gpg_make_key_from_str;
   gpg_proto->key_to_gstr = gpg_key_to_gstr;
	gpg_proto->parseable = gpg_parseable;
   gpg_proto->name = gpg_proto_string;
}

static int  gpg_encrypt(unsigned char** encrypted, unsigned char* msg, int msg_len,
                        crypt_key* inkey)
{ return 0;}

static int gpg_decrypt(unsigned char** decrypted, unsigned char* msg, int msg_len,
                       crypt_key* inkey)
{ return 0;}

static int gpg_sign(unsigned char** signedmsg, unsigned char* msg, int msg_len, crypt_key* key, crypt_key* tokey)
{ return 0;}

static int gpg_auth(unsigned char** authed, unsigned char* msg, int msg_len, crypt_key* key, const char* name)
{ return 0;}

static crypt_key* gpg_make_key_from_str(unsigned char *key_str)
 { return 0;}
static GString* gpg_key_to_gstr(crypt_key* key)
 { return 0;}
static unsigned char* gpg_parseable(unsigned char* key)
 { return 0; }




