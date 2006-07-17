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

#ifndef NSS_MGF1_H
#define NSS_MGF1_H

int mgf1(unsigned char* maskee, unsigned int maskee_len,
         unsigned char* seed, unsigned seed_len);
   
#endif
