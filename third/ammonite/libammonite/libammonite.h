/* $Id: libammonite.h,v 1.1.1.1 2001-01-16 15:26:26 ghudson Exp $
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

#ifndef _LIBAMMONITE_H_
#define _LIBAMMONITE_H_

#include "eazelproxy.h"

/*
 * Codes for AuthnFailInfo
 */

#define EAZELPROXY_AUTHN_FAIL_ARG_INVALID		1
#define EAZELPROXY_AUTHN_FAIL_USER_EXISTS		2
/* Failure due to insufficient resources (eg memory) */
#define EAZELPROXY_AUTHN_FAIL_RESOURCE			3
/* Authentication failed; username or password invalid */
#define EAZELPROXY_AUTHN_FAIL_AUTHN			4
/* Failure making network connection for authentication */
#define EAZELPROXY_AUTHN_FAIL_NETWORK			5
/* Unexpected server response */
#define EAZELPROXY_AUTHN_FAIL_SERVER			6
/* For prompt_authenticate_user, the user hit "cancel" */
#define EAZELPROXY_AUTHN_FAIL_USER_CANCELLED		7

/* Password change errors */
#define EAZELPROXY_PASSWORD_FAIL_OFFSET			100
/* The new password is too weak */
#define EAZELPROXY_PASSWORD_FAIL_INSECURE_PASSWORD	101
/* the original password was incorrect */
#define EAZELPROXY_PASSWORD_FAIL_OLD_PASSWORD		103

#define IID_EAZELPROXY "OAFIID:eazel_proxy:83ed924a-0465-4f53-8013-894f61582750"

/*
 * Types
 */

typedef struct {
	char * scheme;
	char * user;
	char * realm;
	char * resource;
} AmmoniteParsedURL;

typedef enum {
	ERR_Success,
	ERR_UserNotLoggedIn,
	ERR_BadURL,
	ERR_CORBA
} AmmoniteError;

/*
 * General ammonite functions
 */

void EazelProxy_User_copy (EazelProxy_User *dest, const EazelProxy_User *src);
EazelProxy_User * EazelProxy_User_duplicate (const EazelProxy_User *original);

void EazelProxy_AuthnFailInfo_copy (EazelProxy_AuthnFailInfo *dest, const EazelProxy_AuthnFailInfo *src);
EazelProxy_AuthnFailInfo * EazelProxy_AuthnFailInfo_duplicate (const EazelProxy_AuthnFailInfo *original);

gboolean	ammonite_init (PortableServer_POA poa);
void		ammonite_shutdown (void);

EazelProxy_UserControl ammonite_get_user_control (void);
PortableServer_POA ammonite_get_poa (void);

void		ammonite_url_free (AmmoniteParsedURL *to_free);
AmmoniteParsedURL * ammonite_url_parse (const char *url);

AmmoniteError ammonite_http_url_for_eazel_url (const char *orig_url, /* OUT */ char ** new_url);

AmmoniteError ammonite_eazel_url_for_http_url (const char *orig_url, /* OUT */ char ** new_url);

const char * ammonite_fail_code_to_string (CORBA_long code);

/*
 * Wrapper for EazelProxy::AuthnCallback interface
 */

typedef struct {
	void (*succeeded) (const EazelProxy_User *user, gpointer user_data, CORBA_Environment *ev);
	void (*failed) (const EazelProxy_User *user, const EazelProxy_AuthnFailInfo *info, gpointer user_data, CORBA_Environment *ev );
} AmmoniteAuthCallbackWrapperFuncs;

EazelProxy_AuthnCallback ammonite_auth_callback_wrapper_new (PortableServer_POA poa, const AmmoniteAuthCallbackWrapperFuncs *funcs, gpointer user_data);
void ammonite_auth_callback_wrapper_free (PortableServer_POA poa, EazelProxy_AuthnCallback object);

/*
 * Wrapper for EazelProxy::Listener interface
 */

typedef struct {
	void (*user_authenticated) (const EazelProxy_User *user, gpointer user_data, CORBA_Environment *ev);
	void (*user_authenticated_no_longer) (const EazelProxy_User *user, const EazelProxy_AuthnFailInfo *info, gpointer user_data, CORBA_Environment *ev);
	void (*user_logout) (const EazelProxy_User *user, gpointer user_data, CORBA_Environment *ev);
} AmmoniteUserListenerWrapperFuncs;

EazelProxy_UserListener ammonite_user_listener_wrapper_new (PortableServer_POA poa, const AmmoniteUserListenerWrapperFuncs *funcs, gpointer user_data);
void ammonite_user_listener_wrapper_free (PortableServer_POA poa, EazelProxy_UserListener object);

/*
 * Wrapper for EazelProxy::UserPrompter interface
 */

typedef struct {
	CORBA_boolean (*prompt_authenticate) (const EazelProxy_User *user, const EazelProxy_AuthnPromptKind kind, EazelProxy_AuthnInfo **authninfo, gpointer user_data, CORBA_Environment *ev); 
} AmmoniteUserPrompterWrapperFuncs;

EazelProxy_UserPrompter
ammonite_userprompter_wrapper_new (
	PortableServer_POA poa, 
	const AmmoniteUserPrompterWrapperFuncs *funcs,
	gpointer user_data
);
void ammonite_userprompter_wrapper_free (PortableServer_POA poa, EazelProxy_UserPrompter object);

char *		ammonite_get_default_user_username (EazelProxy_UserControl user_control);

char		*ammonite_who_is_logged_in (EazelProxy_UserControl user_control);
gboolean	ammonite_am_i_logged_in (EazelProxy_UserControl user_control);

#endif /* _LIBAMMONITE_H_ */

