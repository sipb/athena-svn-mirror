/*             NSS keys for the Gaim encryption plugin                    */
/*            Copyright (C) 2001-2003 William Tompkins                    */

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

#ifndef NSSRSA_H
#define NSSRSA_H

#include "gaim-encryption-config.h"

#include <gdk/gdk.h>

#include "nonce.h"

/* suppress warnings: our header defines this, then nss tries to */
#ifdef HAVE_LONG_LONG
#undef HAVE_LONG_LONG
#endif
/* From NSS libraries: */
#include <nss.h>
#include <keyhi.h>

typedef struct {
   SECKEYPrivateKey* priv;
   SECKEYPublicKey* pub;
   Nonce nonce;
} RSA_NSS_KEY;

extern gboolean rsa_nss_init(void);

#endif
