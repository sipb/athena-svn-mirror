/*
 * Gaim-Encryption OAEP padding routines, from PKCS#1 v2.1
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

#include <pk11func.h>
#include <keyhi.h>

/* for g_assert; PORT_Assert seems disabled... */
#include <glib.h>

#include "nss_mgf1.h"
#include "nss_oaep.h"

static const unsigned char SHA1_NullHash[20] = {0xda, 0x39, 0xa3, 0xee,
                                                0x5e, 0x6b, 0x4b, 0x0d,
                                                0x32, 0x55, 0xbf, 0xef,
                                                0x95, 0x60, 0x18, 0x90,
                                                0xaf, 0xd8, 0x07, 0x09};

static const unsigned int hlen = 20;  /* SHA1 hash length */

int oaep_pad_block(unsigned char* padded_data, unsigned int padded_len,
                   const unsigned char* data, unsigned int data_len) {


   unsigned char* seed_pos = padded_data + 1;
   unsigned char* db_pos = seed_pos + hlen;
   unsigned char* lhash_pos = db_pos;
   unsigned char* ps_pos = lhash_pos + hlen;
   unsigned char* msg_pos = padded_data + padded_len - data_len;
   unsigned char* padded_end = padded_data + padded_len; /* one AFTER end */
   int ps_len = msg_pos - ps_pos;

   SECStatus rv;

   *padded_data = 0;

   /* fill seed_pos with hlen random bytes */
   rv = PK11_GenerateRandom(seed_pos, hlen);
   g_assert(rv == SECSuccess);

   /* fill lhash_pos with sha-1 constant => empty label*/
   PORT_Memcpy(lhash_pos, SHA1_NullHash, hlen);

   /* fill ps with 00 00 00 ... 00 01 */
   if (ps_len < 1) return 0;
   PORT_Memset(ps_pos, 0, ps_len - 1);
   ps_pos[ps_len - 1] = 1;

   /* fill msg_pos with data */
   PORT_Memcpy(msg_pos, data, data_len);

   /* Do the masking */


   mgf1(db_pos, padded_end - db_pos, seed_pos, hlen);

   mgf1(seed_pos, hlen, db_pos, padded_end - db_pos);

   return 1;
}

int oaep_unpad_block(unsigned char* unpadded_data, unsigned int * unpadded_len,
                     unsigned char* orig_padded_data, unsigned padded_len) {

   unsigned char* padded_data = PORT_Alloc(padded_len);

   unsigned char* seed_pos = padded_data + 1;
   unsigned char* db_pos = seed_pos + hlen;
   unsigned char* lhash_pos = db_pos;
   unsigned char* ps_pos = lhash_pos + hlen;
   unsigned char* padded_end = padded_data + padded_len;
   unsigned char* msg_pos;

   PORT_Memcpy(padded_data, orig_padded_data, padded_len);

   *unpadded_len = 0;

   mgf1(seed_pos, hlen, db_pos, padded_len - (db_pos - padded_data));
   
   mgf1(db_pos, padded_len - (db_pos - padded_data),
        seed_pos, hlen);


   if ((PORT_Memcmp(lhash_pos, SHA1_NullHash, hlen) != 0) ||
       (*padded_data != 0)) {
      PORT_ZFree(padded_data, padded_len);
      return 0;
   }

   msg_pos = ps_pos;
   while ((msg_pos < padded_end) && (*msg_pos == 0)) {
      ++msg_pos;
   }

   if ((msg_pos == padded_end) || (*msg_pos != 1)) {
      PORT_ZFree(padded_data, padded_len);
      return 0;
   }

   msg_pos++;
   
   *unpadded_len = padded_len + padded_data - msg_pos;
   PORT_Memcpy(unpadded_data, msg_pos, *unpadded_len);

   PORT_ZFree(padded_data, padded_len);

   return 1;
}

unsigned int oaep_max_unpadded_len(unsigned int padded_len) {
   int extrastuff = 2 *hlen + 2;
   
   if (padded_len < extrastuff) return 0;
   return padded_len - extrastuff;
}


void oaep_test() {
   int mod_size = 512/8;
   
   unsigned char data[4096/8];
   unsigned char pad_data[4096/8];
   int data_size;
   unsigned char data_out[4096/8];
   int data_out_len;
   SECStatus rv;

   /* overkill, but what the hey.  */

   while (mod_size <= 4096/8) {
      rv = PK11_GenerateRandom(data, oaep_max_unpadded_len(mod_size));
      g_assert(rv == SECSuccess);

      for (data_size = 0; data_size <= oaep_max_unpadded_len(mod_size); ++data_size) {
         g_assert( oaep_pad_block(pad_data, mod_size, data, data_size) );
         g_assert( oaep_unpad_block(data_out, &data_out_len, pad_data, mod_size) );
         g_assert( memcmp(data_out, data, data_size) == 0);
         g_assert( data_size == data_out_len);
      }
      mod_size *= 2;
   }
}
