/*
 * Gaim-Encryption PSS signature routines, from PKCS#1 v2.1
 *
 * Copyright (C) 2003 William Tompkins
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "pk11func.h"
#include "keyhi.h"

/* for g_assert; PORT_Assert seems disabled... */
#include <glib.h>
#include <debug.h>

#include "nss_mgf1.h"
#include "nss_pss.h"

static const SECOidTag Hash_OID = SEC_OID_SHA1;
static const unsigned int hlen = 20;  /* SHA1 hash length */


/* Generate a signature block (not including the msg) in the specified space */
/* salt_len is typically = hlen, or 0                                        */
int pss_generate_sig(unsigned char* sig, unsigned int sig_len,
                     const unsigned char* msg, unsigned int msg_len, int salt_len) {
   
   /* see PKCS#1 v2.1 for a pretty picture.  We construct the signature   */
   /* left to right, in a very straightforward way.                       */
   /* Since the (variably sized) padding is on the left, we first figure  */
   /* out where everything is, going right to left.                       */

   unsigned char* bc_pos = sig + sig_len - 1;
   unsigned char* final_hash_pos = bc_pos - hlen;
   unsigned char* salt_pos = final_hash_pos - salt_len;
   
   int padding2_size = (salt_pos - sig);

   unsigned char* m_prime;

   SECStatus rv;

   /* assuming a modulus size that is a multiple of 8 bits, PS must have at */
   /* least one 0 starting off, plus the 1 that denotes the end of PS       */

   if (padding2_size <= 1) return 0;
   
   /* Construct PS */
   PORT_Memset(sig, 0, padding2_size - 1);
   sig[padding2_size - 1] = 1;

   /* Construct Salt */
   rv = PK11_GenerateRandom(salt_pos, salt_len);
   g_assert(rv == SECSuccess);

   /* Construct M': */
   /*   If we were clever and had an easy way to incrementally hash things, */
   /*   we could avoid actually making M' and just use the pieces parts     */
   /*   where they lie.  Oh well.                                           */

   m_prime = PORT_Alloc(8 + hlen + salt_len);
   g_assert(m_prime != 0);

   /*     Padding1 inside M' */
   PORT_Memset(m_prime, 0, 8);
   
   /*     mHash inside M'    */
   rv = PK11_HashBuf(Hash_OID, m_prime + 8, (unsigned char*)msg, msg_len);
   g_assert(rv == SECSuccess);

   /*     salt inside M'     */
   PORT_Memcpy(m_prime + 8 + hlen, salt_pos, salt_len);

   /* Hash M' into final_hash_pos */
   rv = PK11_HashBuf(Hash_OID, final_hash_pos, m_prime, 8 + hlen + salt_len);
   g_assert(rv == SECSuccess);

   PORT_Free(m_prime);
   /* Why 0xbc?  One of the great mysteries...*/
   *bc_pos = 0xbc;

   /* Almost done: mask everything before the hash with the hash */
   mgf1(sig, final_hash_pos - sig, final_hash_pos, hlen);

   /* Mask probably screwed up our starting zero byte, zero it */
   sig[0] = 0;

   return 1;
}

/* Destructively verify that the the signature block corresponds to    */
/* the given message                                                   */
int pss_check_sig(unsigned char* sig, unsigned int sig_len,
                  const unsigned char* msg, unsigned int msg_len) {

   unsigned char* bc_pos = sig + sig_len - 1;
   unsigned char* final_hash_pos = bc_pos - hlen;
   unsigned char* salt_pos;
   int salt_len;

   unsigned char* m_prime;
   unsigned char* check_hash;
   int hashcmp;

   SECStatus rv;

   if (sig[sig_len - 1] != 0xbc) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "No 0xBC in sig\n");
      return 0;
   }

   if (sig[0] != 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "First byte of sig nonzero\n");
      return 0;
   }

   mgf1(sig, final_hash_pos - sig, final_hash_pos, hlen);
   
   /* Walk down the padding looking for the 01 that marks the salt */
   salt_pos = sig+1;
   while ((salt_pos < final_hash_pos) && (*salt_pos == 0)) {
      ++salt_pos;
   }
   if (salt_pos == final_hash_pos) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "no 0x01 for salt\n");
      return 0;
   }

   if (*salt_pos != 1) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "no 0x01 for salt (2)\n");
      return 0;
   }
   ++salt_pos;

   salt_len = final_hash_pos - salt_pos;
   
   /* Construct M' using the salt we just regained */
   m_prime = PORT_Alloc(8 + hlen + salt_len);
   g_assert(m_prime != 0);

   /*     Padding1 inside M' */
   PORT_Memset(m_prime, 0, 8);
   
   /*     mHash inside M'    */
   rv = PK11_HashBuf(Hash_OID, m_prime + 8, (unsigned char*)msg, msg_len);
   g_assert(rv == SECSuccess);

   /*     salt inside M'     */
   PORT_Memcpy(m_prime + 8 + hlen, salt_pos, salt_len);

   /* Hash M' into check_hash */
   check_hash = PORT_Alloc(hlen);
   g_assert(m_prime != 0);

   rv = PK11_HashBuf(Hash_OID, check_hash, m_prime, 8 + hlen + salt_len);
   g_assert(rv == SECSuccess);

   PORT_Free(m_prime);
   
   hashcmp = memcmp(check_hash, final_hash_pos, hlen);

   PORT_Free(check_hash);

   if (hashcmp != 0) {
      gaim_debug(GAIM_DEBUG_ERROR, "gaim-encryption", "bad hash in sig\n");
      return 0;
   }

   return 1;
}

void pss_test() {
      int mod_size = 512/8;
   
   unsigned char data[4096/8];
   unsigned char sig[4096/8];
   int data_size;
   SECStatus rv;

   /* overkill, but what the hey.  */

   while (mod_size <= 4096/8) {
      rv = PK11_GenerateRandom(data, sizeof(data));
      g_assert(rv == SECSuccess);

      for (data_size = 0; data_size <= 1000; ++data_size) {
         g_assert( pss_generate_sig(sig, mod_size, data, data_size, hlen) );
         g_assert( pss_check_sig(sig, mod_size, data, data_size) );

         g_assert( pss_generate_sig(sig, mod_size, data, data_size, 0) );
         g_assert( pss_check_sig(sig, mod_size, data, data_size) );
      }
      mod_size *= 2;
   }
}

