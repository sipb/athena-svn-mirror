/*                    Wrapper for encryption protocols                    */
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

#ifndef CRYPTPROTO_H
#define CRYPTPROTO_H

#include "debug.h"
#include "gaim.h"

#include "rsa_nss.h"



/* Defined so that keys.h can use it: */
typedef union {
/*   rsa_crypt_key rsa; */
/*   RSA* rsa_ssl;      */
   RSA_NSS_KEY rsa_nss;
} proto_union;

struct crypt_key;

struct crypt_proto {
   /*Crypto operations: each returns the length, and g_malloc's the first argument for you */
   int (*encrypt) (unsigned char** encrypted, unsigned char* msg, int msg_len,
                   struct crypt_key* key);
   int (*decrypt) (unsigned char** decrypted, unsigned char* msg, int msg_len,
                   struct crypt_key* key);
   int (*sign)    (unsigned char** signedmsg, unsigned char* msg, int msg_len,
                   struct crypt_key* key, struct crypt_key* to_key);
   int (*auth)    (unsigned char** authed, unsigned char* msg, int msg_len,
                   struct crypt_key* key, const char* name);


   int (*calc_unencrypted_size) (struct crypt_key* key, int size);
   int (*calc_unsigned_size)    (struct crypt_key* key, int size);

   /* Key <-> String operations */

   struct crypt_key* (*make_key_from_str)  (unsigned char *);
   GString*          (*key_to_gstr)        (struct crypt_key* key);

   unsigned char *   (*parseable)          (unsigned char *keymsg);
   struct crypt_key* (*parse_sent_key)     (unsigned char *);
   GString*          (*make_sendable_key)  (struct crypt_key* key, const char* name);

   gchar*            (*make_key_id)        (struct crypt_key* key);
   /* Key creation / destruction */

   struct crypt_key* (*make_pub_from_priv) (struct crypt_key* priv_key);
   void              (*free)               (struct crypt_key*);
   void              (*gen_key_pair)       (struct crypt_key **, struct crypt_key **,
                                            const char* name, 
                                            int keysize);
   /* Name of the protocol */
   unsigned char* name;
};

typedef struct crypt_proto crypt_proto;

extern GSList*  crypt_proto_list;

int            GE_calc_unencrypted_size(struct crypt_key* enc_key,
                                        struct crypt_key* sign_key,
                                        int size);
unsigned char* GE_encrypt(unsigned char* msg, struct crypt_key* key);
unsigned char* GE_decrypt(unsigned char* msg, struct crypt_key* key);
void           GE_encrypt_signed(unsigned char** out, unsigned char* msg, struct crypt_key* key1,
                                 struct crypt_key* key2);
int            GE_decrypt_signed(unsigned char** authed, unsigned char* msg, struct crypt_key* key1,
                                 struct crypt_key* key2, const char* name);
GString*       GE_key_to_gstr(struct crypt_key* key);

void           GE_free_key(struct crypt_key*);

GString*       GE_make_sendable_key(struct crypt_key* key, const char* name);
gchar*         GE_make_key_id(struct crypt_key* key);

#endif
