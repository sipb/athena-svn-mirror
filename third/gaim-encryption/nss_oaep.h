/*
 * Gaim-Encryption OAEP padding routines
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

#ifndef NSS_OAEP_H
#define NSS_OAEP_H

int oaep_pad_block(unsigned char* padded_data, unsigned int padded_len,
                   const unsigned char* data, unsigned int data_len);

int oaep_unpad_block(unsigned char* unpadded_data_start, unsigned int * unpadded_len,
                     unsigned char* padded_data, unsigned padded_len);

unsigned int oaep_max_unpadded_len(unsigned int padded_len);

void oaep_test();

#endif
