/* $Id: authenticate.h,v 1.1.1.1 2001-01-16 15:26:01 ghudson Exp $
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
 * Authors:  Michael Fleming <mfleming@eazel.com>
 *	     Robey Pointer <robey@eazel.com>
 */

#ifndef EAZELPROXY_AUTHN_H
#define EAZELPROXY_AUTHN_H

#include "impl-eazelproxy.h"
#include "session.h"

#include <libammonite.h>	/* for authn fail codes*/

/*
 * Types
 */

typedef void (*AuthenticateCallbackFn) (
		gpointer user_data, 
		DigestState *p_digest, 		/* NOTE: calleee must free! */
		gboolean success, 
		const EazelProxy_AuthnFailInfo *fail_info,
		char *authn_post_response	/* NOTE: calleee must free! */
);


/*
 * Async Authenticate Functions
 */

void  authenticate_user (
	const char *username,
	const char *password,
	const HTTPRequest *request,
	gpointer user_data,
	AuthenticateCallbackFn callback_fn
);

#endif /* EAZELPROXY_AUTHN_H */
