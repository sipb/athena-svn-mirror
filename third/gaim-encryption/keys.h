/*  Protocol-independent Key structures                                   */
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

#ifndef KEYS_H
#define KEYS_H

#include "cryptproto.h"

#include "debug.h"
#include "gaim.h"
#include "conversation.h"


#define KEY_DIGEST_LENGTH 10
#define KEY_FINGERPRINT_LENGTH 59

#define MAX_KEY_STORLEN 8000   /* The maximum length of a key stored in a file (in chars) */

struct crypt_key {
   crypt_proto* proto;
   proto_union store;                         /* Protocol dependent key data          */
   /*   enum {Public, Private} type; */
   char length[6];                            /* string: Size of key (for ui display) */
   char digest[KEY_DIGEST_LENGTH];            /* Top 10 hex digits of modulus         */
   char fingerprint[KEY_FINGERPRINT_LENGTH];  /* SHA-1 hash of modulus, as 12:34:56...*/
   /* Why have both digest and fingerprint?  Well a) historical b) practicality       */
   /*  digest is insecure as a means of verifying that keys are actually the same     */
   /*  fingerprint is too long to include with every message                          */
};
typedef struct crypt_key crypt_key;

struct key_ring_data {
   char name[64];
   GaimAccount* account;
   crypt_key* key;
};
typedef struct key_ring_data key_ring_data;
typedef GSList key_ring;

/* List of all the keys we know about */
extern key_ring *GE_buddy_ring, *GE_saved_buddy_ring, *GE_my_priv_ring, *GE_my_pub_ring;
static const char Private_key_file[] = "id.priv";
static const char Public_key_file[] = "id";
static const char Buddy_key_file[] = "known_keys";

/*The key routines: */
crypt_key * GE_find_key_by_name(key_ring *, const char *name, GaimAccount* acct);
crypt_key * GE_find_own_key_by_name(key_ring **, char *name, GaimAccount *acct, GaimConversation *conv);
void        GE_debug_dump_keyring(key_ring *);
key_ring *  GE_find_key_node_by_name(key_ring *, const char *name, GaimAccount* acct);
void        GE_received_key(char *keystr, char *name, GaimAccount* acct, GaimConversation *conv, char** orig_msg);
key_ring *  GE_load_keys(const char *);
void        GE_save_keys(key_ring *, char *, char *);
void        GE_key_rings_init(void);
key_ring*   GE_add_key_to_ring(key_ring*, key_ring_data*);
void        GE_add_key_to_file(const char *filename, key_ring_data* key);
key_ring*   GE_del_key_from_ring(key_ring* ring, const char* name, GaimAccount* acct);
void        GE_del_key_from_file(const char *filename, const char *name, GaimAccount *acct);
void        GE_del_one_key_from_file(const char *filename, int key_num, const char *name);
key_ring*   GE_clear_ring(key_ring*);
void        GE_make_private_pair(crypt_proto* proto, const char* name, GaimAccount* acct, int keylength);

#endif
