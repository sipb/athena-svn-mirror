/* $Id: digest.h,v 1.1.1.1 2001-01-16 15:26:01 ghudson Exp $
 * 
 * Copyright (C) 2000 Eazel, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author:  Michael Fleming <mfleming@eazel.com>
 *
 */

#ifndef DIGEST_H
#define DIGEST_H

typedef struct DigestState DigestState;

DigestState *  digest_init ( const char *username, const char *passwd , char *authn_header );
void digest_change_password ( DigestState *p_digest_state, const char *passwd);
char * digest_gen_response_header (DigestState *digest, const char *uri, const char *http_method);
void digest_free (DigestState * digest);


#endif /* DIGEST_H */
