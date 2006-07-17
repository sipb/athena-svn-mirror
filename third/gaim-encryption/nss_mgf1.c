/*
 * Gaim-Encryption MGF-1 Mask Generation Function (see PKCS#1 v2.1)
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
 */

#include <pk11func.h>

/* for g_assert; PORT_Assert seems disabled... */
#include <glib.h>

#include "nss_mgf1.h"

static const SECOidTag Hash_OID = SEC_OID_SHA1;

/* Mask Generation function:  From a seed, produce a variably sized mask, and */
/*   XOR it with the maskee.                                                  */

/* Note- this is an inefficient implementation, as we repeatedly hash the     */
/*         seed.  If we saved the intermediate context, we'd probably save    */
/*         a bunch of time.  But, the NSS exported interface doesn't let us   */
/*         do that easily, so we don't.                                       */

static void memxor (unsigned char* a, unsigned char* b, int len) {
   while (len-- > 0) {
      *a++ ^= *b++;
   }
}

int mgf1(unsigned char* maskee, unsigned int maskee_len,
         unsigned char* seed, unsigned seed_len) {
   
   unsigned char* extended_seed = PORT_Alloc(seed_len + 4);
   unsigned char* hash_out;
   unsigned int hash_len;

   unsigned long int counter = 0;
   unsigned int counter_pos = seed_len;

   unsigned int maskee_pos = 0;
   unsigned int cur_block_size;

   SECStatus rv;

   hash_len = 20;

   hash_out = PORT_Alloc(hash_len);
   PORT_Memcpy(extended_seed, seed, seed_len);
   
   while (maskee_pos < maskee_len) {
      /* Store counter at counter_pos, msb first */
      extended_seed[counter_pos] = (unsigned char) ((counter >> 24) & 0xff);
      extended_seed[counter_pos+1] = (unsigned char) ((counter >> 16) & 0xff);
      extended_seed[counter_pos+2] = (unsigned char) ((counter >> 8) & 0xff);
      extended_seed[counter_pos+3] = (unsigned char) (counter & 0xff);

      rv = PK11_HashBuf(Hash_OID, hash_out, extended_seed, seed_len + 4);
      g_assert(rv == SECSuccess);

      cur_block_size = (maskee_len - maskee_pos);
      if (cur_block_size > hash_len) cur_block_size = hash_len;

      memxor(maskee + maskee_pos, hash_out, cur_block_size);
      maskee_pos += cur_block_size;
      
      ++counter;
   }

   PORT_ZFree(extended_seed, seed_len+4);
   PORT_ZFree(hash_out, hash_len);
   return 1;
}
