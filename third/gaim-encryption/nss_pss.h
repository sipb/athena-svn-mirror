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

/* Generate a signature block (not including the msg) in the specified space */
int pss_generate_sig(unsigned char* sig, unsigned int sig_len,
                     const unsigned char* msg, unsigned int msg_len, int saltlen);

/* Verify that the the signature block corresponds to the given message */
int pss_check_sig(unsigned char* sig, unsigned int sig_len,
                  const unsigned char* msg, unsigned int msg_len);

void pss_test();
