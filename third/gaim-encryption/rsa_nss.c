/*             NSS keys for the Gaim encryption plugin                    */
/*             Copyright (C) 2001 William Tompkins                        */

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

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk/gtkplug.h>

#include <debug.h>
#include <gaim.h>
#include <gtkdialogs.h>

#include "glib/gmain.h"

#include <string.h>
#include <assert.h>

#ifdef _WIN32
#include "win32dep.h"
#endif

#include "rsa_nss.h"

#include <nspr.h>
#include <nss.h>
#include <ssl.h>
#include <secmod.h>
#include <pk11func.h>
#include <keyhi.h>
#include <nssb64.h>


#include "nls.h"
#include "nss_mgf1.h"
#include "nss_oaep.h"
#include "nss_pss.h"
#include "cryptutil.h"
#include "keys.h"
#include "cryptproto.h"
#include "state_ui.h"

char* rsa_nss_proto_string="NSS 1.0";

crypt_proto* rsa_nss_proto;

/*Functions exported through crypt_proto structure */
static int              rsa_nss_encrypt(unsigned char**, unsigned char*, int, crypt_key*);
static int              rsa_nss_decrypt(unsigned char**, unsigned char*, int, crypt_key*);
static int              rsa_nss_sign(unsigned char**, unsigned char*, int, crypt_key*, crypt_key*);
static int              rsa_nss_auth(unsigned char**, unsigned char*, int, crypt_key*, const char* name);
static crypt_key*       rsa_nss_make_key_from_str(unsigned char *key_str);
static GString*         rsa_nss_key_to_gstr(crypt_key* inkey);
static unsigned char*   rsa_nss_parseable(unsigned char* key);
static crypt_key*       rsa_nss_parse_sent_key(unsigned char *key_str);
static GString*         rsa_nss_make_sendable_key(crypt_key* inkey, const char* name);
static gchar*           rsa_nss_make_key_id(crypt_key* inkey);

void                    rsa_nss_gen_key_pair(crypt_key **, crypt_key **,
                                             const char* name, int keysize);
static void             rsa_nss_free(crypt_key*);
static crypt_key*       rsa_nss_make_pub_from_priv(crypt_key* priv);
static int              rsa_nss_calc_unencrypted_size(struct crypt_key*, int);
static int              rsa_nss_calc_unsigned_size(struct crypt_key*, int);

/* internals */
void rsa_nss_test(crypt_key *pub, crypt_key *priv);


gboolean rsa_nss_init() {

   gboolean nss_loaded_ok = FALSE;

  	GaimPlugin *plugin = gaim_plugins_find_with_name("NSS");

   if (plugin != NULL) {
      nss_loaded_ok = gaim_plugin_is_loaded(plugin);
      if (!nss_loaded_ok) {
         nss_loaded_ok = gaim_plugin_load(plugin);
      }
   }

   if (!nss_loaded_ok) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Initializing NSS without Gaim support\n");
      /* Gaim doesn't seem to have NSS support: try to load it ourselves: */
      PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);
      NSS_NoDB_Init(NULL);
      
      /* TODO: Fix this so autoconf does the work trying to find this lib. */
      SECMOD_AddNewModule("Builtins",
#ifndef _WIN32
                          LIBDIR "/libnssckbi.so",
#else
                          "nssckbi.dll",
#endif
                          0, 0);
      NSS_SetDomesticPolicy();

      /*       gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", _("Can't load the NSS plugin\n")); */
      /*       GE_error_window(_("Gaim was not compiled with the NSS plugin enabled.  " */
      /*                       "Gaim-Encryption requires the NSS plugin to function.")); */
      /*       return FALSE; */
   }

   rsa_nss_proto = g_malloc(sizeof(crypt_proto));
   crypt_proto_list = g_slist_prepend(crypt_proto_list, rsa_nss_proto);

   rsa_nss_proto->encrypt = rsa_nss_encrypt;
   rsa_nss_proto->decrypt = rsa_nss_decrypt;
   rsa_nss_proto->sign = rsa_nss_sign;
   rsa_nss_proto->auth = rsa_nss_auth;
      
   rsa_nss_proto->make_key_from_str = rsa_nss_make_key_from_str;
   rsa_nss_proto->key_to_gstr = rsa_nss_key_to_gstr;
   rsa_nss_proto->parseable = rsa_nss_parseable;
   rsa_nss_proto->parse_sent_key = rsa_nss_parse_sent_key;
   rsa_nss_proto->make_sendable_key = rsa_nss_make_sendable_key;
   rsa_nss_proto->make_key_id = rsa_nss_make_key_id;
   rsa_nss_proto->gen_key_pair = rsa_nss_gen_key_pair;
   rsa_nss_proto->free = rsa_nss_free;
   rsa_nss_proto->make_pub_from_priv = rsa_nss_make_pub_from_priv;
   rsa_nss_proto->calc_unencrypted_size = rsa_nss_calc_unencrypted_size;
   rsa_nss_proto->calc_unsigned_size = rsa_nss_calc_unsigned_size;
   rsa_nss_proto->name = rsa_nss_proto_string;

   return TRUE;
}

static void rsa_nss_free(crypt_key* key){
   if (key->store.rsa_nss.pub) {
      SECKEY_DestroyPublicKey(key->store.rsa_nss.pub);
      key->store.rsa_nss.pub = 0;
   }
   if (key->store.rsa_nss.priv) {
      SECKEY_DestroyPrivateKey(key->store.rsa_nss.priv);
      key->store.rsa_nss.priv = 0;
   }
}

static SECItem*
get_random_iv(CK_MECHANISM_TYPE mechType)
{
   int        iv_size = PK11_GetIVLength(mechType);
   SECItem   *iv;
   SECStatus  rv; 
   
   iv = PORT_ZNew(SECItem);
   g_assert(iv != 0);
   g_assert(iv_size != 0);

   iv->data = PORT_NewArray(unsigned char, iv_size);
   g_assert(iv->data != 0);
   iv->len = iv_size;
   rv = PK11_GenerateRandom(iv->data, iv->len);
   g_assert(rv == SECSuccess);

   return iv;
}

static void generate_digest(char* digest, SECKEYPublicKey* key) {
   SECItem *hash = PK11_MakeIDFromPubKey(&key->u.rsa.modulus);
   int i = 0, digestPos = 0;
   
   while (i < hash->len && digestPos < KEY_DIGEST_LENGTH) {
      sprintf(digest + digestPos, "%02x", hash->data[i]);
      ++i;
      digestPos += 2;
   }
}

static void generate_fingerprint(char* print, SECKEYPublicKey* key) {
   SECItem *hash = PK11_MakeIDFromPubKey(&key->u.rsa.modulus);
   int i;
   
   for (i= 0; i < hash->len - 1; ++i) {
      sprintf(print + (3*i), "%02x:", hash->data[i]);
   }
   sprintf(print + 3 * (hash->len - 1), "%02x", hash->data[(hash->len - 1)]);
}

static SECKEYPublicKey *
copy_public_rsa_key(SECKEYPublicKey *pubk) {
   SECKEYPublicKey *copyk;
   PRArenaPool *arena;
   SECStatus rv;
   
   arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
   g_assert(arena != NULL);
   
   copyk = (SECKEYPublicKey *) PORT_ArenaZAlloc (arena, sizeof (SECKEYPublicKey));
   g_assert(copyk != NULL);

   copyk->arena = arena;
   copyk->keyType = pubk->keyType;

   copyk->pkcs11Slot = NULL;   /* go get own reference */
   copyk->pkcs11ID = CK_INVALID_HANDLE;
   
   rv = SECITEM_CopyItem(arena, &copyk->u.rsa.modulus,
                         &pubk->u.rsa.modulus);
   g_assert(rv == SECSuccess);
   
   rv = SECITEM_CopyItem (arena, &copyk->u.rsa.publicExponent,
                          &pubk->u.rsa.publicExponent);
   g_assert(rv == SECSuccess);
   
   return copyk;
}

void rsa_nss_gen_key_pair(crypt_key **pub_key, crypt_key **priv_key,
                          const char* name, int keysize) {

   GtkWidget *status_window, *main_box, *label1, *label2;
   char labelText[1000];
   PK11RSAGenParams rsaParams;
   PK11SlotInfo *slot;

   CK_MECHANISM_TYPE multiType[2] = {CKM_RSA_PKCS_KEY_PAIR_GEN, CKM_DES_CBC_PAD};

   /* Create the widgets */

   GAIM_DIALOG(status_window);
   gtk_window_set_wmclass(GTK_WINDOW(status_window), "keygen", "Gaim");
   gtk_widget_realize(status_window);
   gtk_container_set_border_width(GTK_CONTAINER(status_window), 10);
   gtk_widget_set_size_request(status_window, 350, 100);
   gtk_window_set_title(GTK_WINDOW(status_window), "Status");
   main_box = gtk_vbox_new(FALSE, 0);
   gtk_container_add(GTK_CONTAINER(status_window), main_box);
   gtk_widget_show(main_box);
   
   g_snprintf(labelText, sizeof(labelText), _("Generating RSA Key Pair for %s"),
              name);
   label1 = gtk_label_new (labelText);
   label2 = gtk_label_new (_("This may take a little bit..."));
   
   gtk_container_add (GTK_CONTAINER (main_box), label1);
   gtk_widget_show(label1);
   gtk_container_add (GTK_CONTAINER (main_box), label2);
   gtk_widget_show(label2);
   
   gtk_widget_show (status_window);

   // I don't understand: if I remove one of these
   // two loops, the contents of the status window don't
   // get drawn.  Hmm...
   while (gtk_events_pending()) {
      gtk_main_iteration_do(FALSE);
   }
   gtk_main_iteration();
   while (gtk_events_pending()) {
      gtk_main_iteration_do(FALSE);
   }
   
   *priv_key = g_malloc(sizeof(crypt_key));

   rsaParams.keySizeInBits = keysize;
   rsaParams.pe = 65537L;
  
   slot = PK11_GetBestSlotMultiple(multiType, 2, 0);
   
   /* Generate "session" (first FALSE), "not sensitive" (next FALSE) key */
   (*priv_key)->store.rsa_nss.priv = 
      PK11_GenerateKeyPair(slot, CKM_RSA_PKCS_KEY_PAIR_GEN, &rsaParams,
                           &(*priv_key)->store.rsa_nss.pub,
                           PR_FALSE, PR_FALSE, 0);

   if (!(*priv_key)->store.rsa_nss.priv) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption",
                 _("Could not generate key.  NSS Error: %d\n"),
                 PORT_GetError());
      exit(0);
   }

   (*priv_key)->proto = rsa_nss_proto;
   g_snprintf((*priv_key)->length, sizeof((*priv_key)->length), "%d", keysize);

   generate_digest((*priv_key)->digest, (*priv_key)->store.rsa_nss.pub);
   generate_fingerprint((*priv_key)->fingerprint, (*priv_key)->store.rsa_nss.pub);

   (*pub_key) = rsa_nss_make_pub_from_priv(*priv_key);

   gtk_widget_hide(status_window);
   gtk_widget_destroy(status_window);
}

unsigned char* rsa_nss_parseable(unsigned char *key) {
   /* If the key is ours, return a pointer to the ':' after our token */
   /* otherwise return 0                                              */
   /* if we were more sophisticated, we could look for older versions */
   /* of our protocol here, and accept them too.                      */
   if (strncmp(rsa_nss_proto_string, key, strlen(rsa_nss_proto_string)) == 0) {
      return key + strlen(rsa_nss_proto_string);
   } else {
      return 0;
   }
}


static GString* append_priv_key_to_gstr(GString *str, SECKEYPrivateKey* priv) {

   /* for now, we hope that everyone can use DES3.  This is for wrapping   */
   /* private keys, which isn't actually secure at this point anyways      */
   /* (security provided by the OS: no one else can read/write the keyfile */
   const CK_MECHANISM_TYPE SymEncryptionType = CKM_DES3_CBC_PAD;

   PK11SlotInfo *symSlot;
   PK11SymKey *symKey;

   SECItem symKeyItem;  /* storage space for binary key import */
   unsigned char symKeyData[24] = {0}; 

   SECItem *iv = NULL;  /* IV for CBC encoding                 */

   SECItem exportedKey; /* storage space for exported key */
   unsigned char exportedKeyData[5000] = {0};

   char* tmpstr;
   int errCode;

   if (priv == 0) return str;

   /* Wrap key using a null symmetric key.  When/If we add password protection to
      keys, we can generate the symmetric key from a hashed password instead */
   symSlot = PK11_GetBestSlot(SymEncryptionType, NULL);
   g_assert(symSlot != 0);
   
   symKeyItem.data = &symKeyData[0];
   symKeyItem.len = sizeof(symKeyData);
   
   symKey  = PK11_ImportSymKey(symSlot, PK11_GetKeyGen(SymEncryptionType),
                               PK11_OriginUnwrap, CKA_WRAP, &symKeyItem, NULL);
   
   iv = get_random_iv(SymEncryptionType);
   
   exportedKey.len = sizeof(exportedKeyData);
   exportedKey.data = exportedKeyData;
   
   errCode = PK11_WrapPrivKey(symSlot, symKey, priv, SymEncryptionType, iv,
                              &exportedKey, NULL);

   g_assert(errCode == SECSuccess);

   
   g_string_append(str, ",");

   tmpstr = NSSBase64_EncodeItem(0, 0, 0, iv);
   g_string_append(str, tmpstr);
   PORT_Free(tmpstr);
   
   g_string_append(str, ",");
   
   tmpstr = NSSBase64_EncodeItem(0, 0, 0, &exportedKey);
   g_string_append(str, tmpstr);
   PORT_Free(tmpstr);
   
   g_string_append(str, ",");
   
   PK11_FreeSymKey(symKey);
   PORT_Free(iv->data);
   PORT_Free(iv);

   /* The Base64 routine may have inserted lots of return chars into the string: */
   /*   take them out.                                                           */
   GE_strip_returns(str);

   return str;
}

static GString* append_pub_key_to_gstr(GString *str, SECKEYPublicKey* pub) {
   char *tmpstr;
   SECItem *exportedKey;
   
   if (pub == 0) return str;

   exportedKey = SECKEY_EncodeDERSubjectPublicKeyInfo(pub);
   //   exportedKey = PK11_DEREncodePublicKey(pub);
   
   tmpstr = NSSBase64_EncodeItem(0, 0, 0, exportedKey);
   g_string_append(str, tmpstr);

   PORT_Free(tmpstr);
   PORT_Free(exportedKey->data);
   PORT_Free(exportedKey);

   /* The Base64 routine may have inserted lots of return chars into the string: take them out. */
   GE_strip_returns(str);

   return str;
}


GString* rsa_nss_make_sendable_key(crypt_key* inkey, const char* name) {
   GString *outString = g_string_new("");

   gchar* nonce_str = GE_new_incoming_nonce(name);
   g_string_append(outString, nonce_str);

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Sending Nonce with key: %s\n", nonce_str);
   
   g_free(nonce_str);

   g_string_append(outString, ",");
   
   append_pub_key_to_gstr(outString, inkey->store.rsa_nss.pub);

   return outString;
}

gchar* rsa_nss_make_key_id(crypt_key* inkey) {
   return GE_nonce_to_str(&inkey->store.rsa_nss.nonce);
}

crypt_key* rsa_nss_parse_sent_key(unsigned char *key_str) {
   gchar** split_key = g_strsplit(key_str, ",", 2);
   crypt_key* key;
   

   if ((split_key[0] == 0) || (split_key[1] == 0)) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Error parsing RSANSS nonce/key\n");
      return 0;
   }
   
   key = rsa_nss_make_key_from_str(split_key[1]);

   if (key == 0) return 0;

   GE_str_to_nonce(&key->store.rsa_nss.nonce, split_key[0]);
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Received Nonce with key: %s\n", split_key[0]);

   g_strfreev(split_key);

   return key;
}

GString* rsa_nss_key_to_gstr(crypt_key* inkey) {
   GString *outString = g_string_new("");
   
   append_pub_key_to_gstr(outString, inkey->store.rsa_nss.pub);

   append_priv_key_to_gstr(outString, inkey->store.rsa_nss.priv);

   return outString;
}

crypt_key* rsa_nss_make_key_from_str(unsigned char *key_str){
   gchar **split_key;
   
   crypt_key* key = g_malloc(sizeof(crypt_key));

   /* For Private keys: */
   const CK_MECHANISM_TYPE SymEncryptionType = CKM_DES3_CBC_PAD;
   PK11SlotInfo *symSlot;
   PK11SymKey *symKey;
   SECItem *pubKeyValue;
   SECItem symKeyItem;  /* storage space for binary key import */
   unsigned char symKeyData[24] = {0};
   SECItem *iv = 0, *wrappedKey = 0, label;
   CK_ATTRIBUTE_TYPE attribs[3] = { CKA_SIGN, CKA_DECRYPT, CKA_SIGN_RECOVER };
   const int NumAttribs = 3;
   int cur_piece;

   CERTSubjectPublicKeyInfo *certPubKeyInfo;

   /* key_str looks like "KKKKK" or "KKKKK,NNNN,MMMM", where */
   /* KKKKK is the Base64 encoding of the public key, or              */
   /* NNNN is the Base64 encoding of the IV, and                      */
   /* MMMM is the Base64 encoding of the encrypted private key        */


   key->proto = rsa_nss_proto;
   
   split_key = g_strsplit(key_str, ",", 3);

   key->store.rsa_nss.pub = 0;
   key->store.rsa_nss.priv = 0;

   cur_piece = 0;

   // Check for public key part, and get it:

   if (split_key[cur_piece] == 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "(%d) %s",
                 1, _("Error parsing RSANSS key\n"));
      g_free(key);
      g_strfreev(split_key);
      return 0;
   }

   wrappedKey = NSSBase64_DecodeBuffer(0, 0, split_key[cur_piece], strlen(split_key[cur_piece]));

   if (wrappedKey == 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "(%d) %s",
                 2, _("Error parsing RSANSS key\n"));
      g_free(key);
      g_strfreev(split_key);
      return 0;
   }

   certPubKeyInfo = SECKEY_DecodeDERSubjectPublicKeyInfo(wrappedKey);

   PORT_Free(wrappedKey->data);
   PORT_Free(wrappedKey);
   
   if (certPubKeyInfo == NULL) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "(%d) %s",
                 3, _("Error parsing RSANSS key\n"));
      g_free(key);
      g_strfreev(split_key);
      return 0;
   }
   
   key->store.rsa_nss.pub = SECKEY_ExtractPublicKey(certPubKeyInfo);

   if (key->store.rsa_nss.pub == NULL) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "(%d) %s",
                 4, _("Error parsing RSANSS key\n"));
      g_free(key);
      g_strfreev(split_key);
      return 0;
   }

   SECKEY_DestroySubjectPublicKeyInfo(certPubKeyInfo);

   generate_digest(key->digest, key->store.rsa_nss.pub);
   generate_fingerprint(key->fingerprint, key->store.rsa_nss.pub);

   g_snprintf(key->length, sizeof(key->length), "%d",
              8 * SECKEY_PublicKeyStrength(key->store.rsa_nss.pub));

   if (split_key[++cur_piece] == 0) {
      /* No private part, so return a public key: */
      g_strfreev(split_key);
      return key;
   }

   /* ------------------------------------------------------------------------ */
   /*  Extract Private key:                                                    */
   /*                                                                          */

   iv = NSSBase64_DecodeBuffer(0, 0, split_key[cur_piece],
                               strlen(split_key[cur_piece]));

   if (split_key[++cur_piece] == 0) {
      /* only part of a private key */
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "(%d) %s",
                 5, _("Error parsing RSANSS key\n"));
      g_free(key);
      g_strfreev(split_key);
      return 0;
   }

   wrappedKey =  NSSBase64_DecodeBuffer(0, 0, split_key[cur_piece],
                                        strlen(split_key[cur_piece]));

   if ((iv == 0) || (wrappedKey == 0)) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "(%d) %s",
                 6, _("Error parsing RSANSS key\n"));
      g_free(key);
      g_strfreev(split_key);
      return 0;
   }

   pubKeyValue = SECITEM_DupItem(&key->store.rsa_nss.pub->u.rsa.modulus);

   symSlot = PK11_GetBestSlot(SymEncryptionType, NULL);
   g_assert(symSlot != 0);
   
   symKeyItem.data = &symKeyData[0];
   symKeyItem.len = sizeof(symKeyData);
   
   symKey  = PK11_ImportSymKey(symSlot, PK11_GetKeyGen(SymEncryptionType),
                               PK11_OriginUnwrap, CKA_WRAP, &symKeyItem, NULL);

   if (!symKey) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "(%d) %s",
                 7, _("Error parsing RSANSS key\n"));
      g_strfreev(split_key);

      SECKEY_DestroyPublicKey(key->store.rsa_nss.pub);
      SECITEM_FreeItem (pubKeyValue, PR_TRUE);
      g_free(key);
      return 0;
   }

   label.data = NULL; label.len = 0;

   key->store.rsa_nss.priv =
      PK11_UnwrapPrivKey(symSlot, symKey, SymEncryptionType, iv,
                         wrappedKey, &label, pubKeyValue,
                         PR_FALSE, PR_FALSE, CKK_RSA, attribs, NumAttribs, 0);
   
   SECITEM_FreeItem (pubKeyValue, PR_TRUE);

   if (key->store.rsa_nss.priv == 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "(%d) %s",
                 8, _("Error parsing RSANSS key\n"));
      g_strfreev(split_key);

      SECKEY_DestroyPublicKey(key->store.rsa_nss.pub);
      g_free(key);
      return 0;
   }
   /* should sanity check public/private pair */

   g_strfreev(split_key);

   return key;
}   


crypt_key* rsa_nss_make_pub_from_priv(crypt_key* priv_key) {
   crypt_key* pub_key = g_malloc(sizeof(crypt_key));
   
   pub_key->proto = rsa_nss_proto;
   strcpy(pub_key->length, priv_key->length);
   strncpy(pub_key->digest, priv_key->digest, KEY_DIGEST_LENGTH);
   strncpy(pub_key->fingerprint, priv_key->fingerprint, KEY_FINGERPRINT_LENGTH);

   pub_key->store.rsa_nss.pub = copy_public_rsa_key(priv_key->store.rsa_nss.pub);
   pub_key->store.rsa_nss.priv = 0;
   
   return pub_key;
}

int rsa_nss_encrypt(unsigned char** encrypted, unsigned char* msg, int msg_len,
                    crypt_key* pub_key){

   SECKEYPublicKey * key = pub_key->store.rsa_nss.pub;

   int modulus_len = SECKEY_PublicKeyStrength(key);

   int unpadded_block_len = oaep_max_unpadded_len(modulus_len);

   int num_blocks = ((msg_len - 1) / unpadded_block_len) + 1;

   unsigned char* msg_cur, *encrypt_cur;

   int msg_block_len;

   unsigned char *padded_block = g_malloc(modulus_len);

   int ret;
   SECStatus rv;

   //   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Starting Encrypt\n");
   
   *encrypted = g_malloc(num_blocks * modulus_len);

   msg_cur = msg;
   encrypt_cur = *encrypted;
   
   while (msg_cur < msg + msg_len) {

      msg_block_len = unpadded_block_len;
      if (msg_cur + msg_block_len > msg + msg_len) {
         msg_block_len = msg + msg_len - msg_cur;
      }

      ret = oaep_pad_block(padded_block, modulus_len, msg_cur, msg_block_len);
      if (!ret) {
         g_free(padded_block);
         g_free(*encrypted);
         *encrypted = 0;
         return 0;
      }
      rv = PK11_PubEncryptRaw(key, encrypt_cur, padded_block,
                              modulus_len, 0);
      if (rv != SECSuccess) {
         g_free(padded_block);
         g_free(*encrypted);
         *encrypted = 0;
         return 0;
      }

      msg_cur += msg_block_len;
      encrypt_cur += modulus_len;
   }

   g_free(padded_block);
   //   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Ending Encrypt\n");

   return (encrypt_cur - *encrypted);
}

int rsa_nss_decrypt(unsigned char** decrypted, unsigned char* msg, int msg_len,
                    crypt_key* priv_key){
   
   SECKEYPrivateKey * key = priv_key->store.rsa_nss.priv;

   int modulus_len = SECKEY_PublicKeyStrength(priv_key->store.rsa_nss.pub);

   int unpadded_block_len = oaep_max_unpadded_len(modulus_len);

   int num_blocks = msg_len / modulus_len;

   unsigned char* msg_cur, *decrypt_cur;
   
   int decrypt_block_size;

   unsigned char *padded_block = g_malloc(modulus_len);

   int ret;
   SECStatus rv;
   
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Starting Decrypt\n");

   *decrypted = g_malloc(num_blocks * unpadded_block_len + 1);

   msg_cur = msg;
   decrypt_cur = *decrypted;
   
   if (num_blocks * modulus_len != msg_len) {  /* not an even number of blocks */
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Not a multiple of block len: %d %d %d\n",
                 num_blocks, modulus_len, msg_len);
      g_free(padded_block);
      g_free(*decrypted);
      *decrypted = 0;
      return 0;
   }

   while (msg_cur < msg + msg_len) {

      rv = PK11_PubDecryptRaw(key, padded_block, &decrypt_block_size, modulus_len,
                              msg_cur, modulus_len);

      if (rv != SECSuccess) {
         gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "PubDecryptRaw failed %d\n", rv);
         g_free(padded_block);
         g_free(*decrypted);
         *decrypted = 0;
         return 0;
      }

      g_assert(decrypt_block_size == modulus_len); /* for now. Don't understand how */
                                                   /* this could not be true        */

      ret = oaep_unpad_block(decrypt_cur, &decrypt_block_size, padded_block, modulus_len);
      if (!ret) {
         gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "OAEP unpadding failed\n");
         return 0;
      }

      msg_cur += modulus_len;
      decrypt_cur += decrypt_block_size;
   }

   /* Null terminate what just came out, in case someone tries to use it as a string */
   *decrypt_cur = 0;

   //   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Ending Decrypt\n");
   return (decrypt_cur - *decrypted);
}

int rsa_nss_sign(unsigned char** signed_msg, unsigned char* msg, int msg_len,
                 crypt_key* priv_key, crypt_key* pub_key) {

   SECKEYPrivateKey * key = priv_key->store.rsa_nss.priv;
   int modulus_len = SECKEY_PublicKeyStrength(priv_key->store.rsa_nss.pub);
   unsigned char *sig_pos, *tmp_sig;

   SECStatus rv;
   int out_block_size;

   const int salt_len = 20;

   gchar *nonce_str = GE_nonce_to_str(&pub_key->store.rsa_nss.nonce);
   int nonce_len = strlen(nonce_str);
   
   GE_incr_nonce(&pub_key->store.rsa_nss.nonce);

   *signed_msg = g_malloc(msg_len + modulus_len + nonce_len + 1);
   tmp_sig = g_malloc(modulus_len);
   memcpy(*signed_msg, nonce_str, nonce_len);
   (*signed_msg)[nonce_len]=':';

   memcpy(*signed_msg + nonce_len + 1, msg, msg_len);
   sig_pos = *signed_msg + msg_len + nonce_len + 1 ;
   
   
   pss_generate_sig(tmp_sig, modulus_len, *signed_msg, msg_len + nonce_len + 1, salt_len);

   rv = PK11_PubDecryptRaw(key, sig_pos, &out_block_size, modulus_len,
                           tmp_sig, modulus_len);

   if (rv != SECSuccess) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "PK11_PubDecryptRaw Failed\n");
      g_free(*signed_msg);
      *signed_msg = 0;
      return 0;
   }

   g_assert(out_block_size == modulus_len);  /* dunno why they yield out_block_size */

   g_free(tmp_sig);
   
   return msg_len + nonce_len + 1 + modulus_len;
}

// Returns length of authed string, or 0 if not authenticated
// g_malloc's space for the authed string, and null terminates the result
// If returning zero, may return a message ID (nonce) as the authed string

int rsa_nss_auth(unsigned char** authed, unsigned char* msg, int msg_len,
                 crypt_key* pub_key, const char* name) {
   
   SECKEYPublicKey * key = pub_key->store.rsa_nss.pub;
   int modulus_len = SECKEY_PublicKeyStrength(key);
   unsigned char *tmp_sig;

   SECStatus rv;
   int verified;

   gchar *nonce_msg, **nonce_msg_split;

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Starting Auth\n");

   *authed = 0;

   if (msg_len < modulus_len) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Bad msg_len in Auth\n");
      return -1;
   }

   tmp_sig = g_malloc(modulus_len);

   rv = PK11_PubEncryptRaw(key, tmp_sig, msg + msg_len - modulus_len,
                           modulus_len, 0);

   if (rv != SECSuccess) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "PK11_PubEncryptRaw Failed\n");
      g_free(tmp_sig);
      return -1;
   }

   /*   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Auth 2\n"); */

   verified = pss_check_sig(tmp_sig, modulus_len, msg, msg_len - modulus_len);

   g_free(tmp_sig);
   
   if (!verified) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", _("Bad signature on message (len %d, mod %d)\n"),
                 msg_len, modulus_len);
      return -1;
   }

   /*    gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Auth 3\n"); */

   nonce_msg = g_strndup(msg, msg_len - modulus_len);
   nonce_msg_split = g_strsplit(nonce_msg, ":", 2);
   g_free(nonce_msg);

   /*    gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Auth 4\n"); */

   if ((nonce_msg_split[0] == 0) || (nonce_msg_split[1] == 0)) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "No Nonce in message\n");
      g_strfreev(nonce_msg_split);
      return -1;
   }         
   
   /*    gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Auth 5\n"); */

   if (!GE_check_incoming_nonce(name, nonce_msg_split[0])) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "Bad Nonce in message\n");
      *authed = g_strdup(nonce_msg_split[0]);
      
      g_strfreev(nonce_msg_split);
      return -1;
   }
   
   /*    gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Auth 6\n"); */

   *authed = nonce_msg_split[1];
   g_free(nonce_msg_split[0]);
   g_free(nonce_msg_split);

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "Auth End\n");

   return strlen(*authed);
}

int rsa_nss_calc_unencrypted_size(struct crypt_key* key, int insize) {
   int modulus_len = SECKEY_PublicKeyStrength(key->store.rsa_nss.pub);
   int unpadded_block_len = oaep_max_unpadded_len(modulus_len);

   int num_blocks = insize / modulus_len; /* floor: max number of blocks that could fit */

   return num_blocks * unpadded_block_len;
}

int rsa_nss_calc_unsigned_size(struct crypt_key* key, int insize) {
   int modulus_len, nonce_len;
   
   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "calc_unsigned_size\n");

   modulus_len = SECKEY_PublicKeyStrength(key->store.rsa_nss.pub);
   nonce_len = GE_nonce_str_len();

   gaim_debug(GAIM_DEBUG_INFO, "gaim-encryption", "modulus_len:%d:%d\n", modulus_len, nonce_len);
   
   if (insize < modulus_len + nonce_len) return 0;
   return insize - modulus_len - nonce_len - 1;  /* -1 from ":" after nonce */
}


