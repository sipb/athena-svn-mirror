
/* $Id: impl-eazelproxy.h,v 1.1.1.1 2001-01-16 15:26:02 ghudson Exp $
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

#ifndef _IMPL_EAZELPROXY_H_
#define _IMPL_EAZELPROXY_H_

#include "eazelproxy.h"
#include "digest.h"

#include <libammonite.h>	/* for IID_EAZELPROXY codes*/

/*
 * Types
 */

typedef struct User User;


/*
 * Global Variables
 */
extern User *gl_user_default;
extern GList *gl_user_list;
extern EazelProxy_UserControl gl_object_usercontrol;

/*
 * CORBA gunk for EazelProxy interface
 */
 
extern void init_impl_eazelproxy (CORBA_ORB orb);

extern void shutdown_impl_eazelproxy (void);

/*
 * Made public for eazelproxy-change-password.c
 */

void listener_broadcast_user_logout (const User * user);
void user_deactivate (User *user);
void user_set_login_state (User *user, EazelProxy_LoginState state);
EazelProxy_User *user_get_EazelProxy_User (User *user);
DigestState *user_get_digest_state (User *user);

#endif /* _IMPL_EAZELPROXY_H_ */
