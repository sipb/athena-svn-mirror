/* $Id: ammonite-general.c,v 1.1.1.1 2001-01-16 15:26:26 ghudson Exp $
 * 
 * ammonite-general.c:
 * 
 * General utility functions for using the Eazel ammonite authentication system.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "libammonite.h"
#include <orb/orbit.h>
#include <liboaf/liboaf.h>
#include <stdlib.h>

#ifdef DEBUG
#define DEBUG_MSG(x) my_debug_printf x
#include <stdio.h>

static void
my_debug_printf(char *fmt, ...)
{
	va_list args;
	gchar * out;

	g_assert (fmt);

	va_start (args, fmt);

	out = g_strdup_vprintf (fmt, args);

	fprintf (stderr, "DEBUG: %s\n", out);

	g_free (out);
	va_end (args);
}

#else
#define DEBUG_MSG(x) 
#endif /* DEBUG */

/*
 * Global Variables
 */

static EazelProxy_UserControl gl_user_control = CORBA_OBJECT_NIL;
static PortableServer_POA gl_poa = CORBA_OBJECT_NIL;

/*
 * EazelProxy_User extention functions
 */
void
EazelProxy_User_copy (EazelProxy_User *dest, const EazelProxy_User *src)
{
	g_assert (src);
	g_assert (dest);

        dest->user_name = CORBA_string_dup (src->user_name);
        dest->login_state = src->login_state;
        dest->proxy_port = src->proxy_port;
        dest->is_default = src->is_default;
	dest->services_redirect_uri = CORBA_string_dup (src->services_redirect_uri);
	dest->services_login_path = CORBA_string_dup (src->services_login_path);
	dest->login_http_response = CORBA_string_dup (src->login_http_response);
}


EazelProxy_User *
EazelProxy_User_duplicate (const EazelProxy_User *original )
{
        EazelProxy_User *ret;

        if ( NULL == original ) {
                return NULL;
        }

        ret = EazelProxy_User__alloc();

	EazelProxy_User_copy (ret, original);

        return ret;
}


void
EazelProxy_AuthnFailInfo_copy (EazelProxy_AuthnFailInfo *dest, const EazelProxy_AuthnFailInfo *src)
{
	g_assert (src);
	g_assert (dest);

	dest->code = src->code;
	dest->http_result = CORBA_string_dup (src->http_result);
}

EazelProxy_AuthnFailInfo * 
EazelProxy_AuthnFailInfo_duplicate (const EazelProxy_AuthnFailInfo *original)
{
        EazelProxy_AuthnFailInfo *ret;

        if ( NULL == original ) {
                return NULL;
        }

        ret = EazelProxy_AuthnFailInfo__alloc();

	EazelProxy_AuthnFailInfo_copy (ret, original);

        return ret;
}

/* 
 * Utility Functions
 */


void
ammonite_url_free (AmmoniteParsedURL * to_free)
{
	if ( NULL != to_free ) {
		g_free (to_free->scheme);
		g_free (to_free->user);
		g_free (to_free->realm);
		g_free (to_free->resource);
		g_free (to_free);
	}
}

/* {scheme}://user@realm/<resource> 			*/
/* {scheme}:/<resource> means 'default user and realm'	*/

AmmoniteParsedURL *
ammonite_url_parse (const char *url)
{
	const char *scheme;
	const char *current;
	AmmoniteParsedURL *result;
	gboolean success;

	g_return_val_if_fail ( NULL != url, NULL);

	success = FALSE;
	current = url;
	result = g_new0 (AmmoniteParsedURL, 1);

	/* ([^:]*):/(/(([^@]+)(@([^/]*))?)?/)?(.*)
	 *          ^        ^            ^ resource
	 *          user     realm
	 *   ^ scheme
	 */

	/* Scheme portion */
	scheme = url;
	current = strchr (current, (unsigned char)':');

	if (NULL == current) {
		success = FALSE;
		goto error;
	}

	current++;
	result->scheme = g_strndup (url, current - scheme);

	/* User portion */

	if ( '\0' == *current || '/' != *current ) {
		success = FALSE;
		goto error;
	}

	if ( '/' == *(current+1) && '/' == *(current+2) ) {
		current+=2;
	} else if ( '/' == *(current+1) ) {
		const char *user;

		current += 2;
		user = current;

		while ( '\0' != *current && '@' != *current && '/' != *current ) {
			current++;
		}

		if ( current == user ) {
			/* allow empty resource */
			success = TRUE;
			result->resource = g_strdup ("/");
			goto error;
		}

		result->user = g_strndup (user, current - user);
		user = NULL;

		if ( '\0' == *current ) {
			/* allow empty resource */
			success = TRUE;
			result->resource = g_strdup ("/");
			goto error;
		}
		
		if ( '@' == *current ) {
			const char *realm;
			
			realm = ++current;

			while ( '\0' != *current && '/' != *current ) {
				current++;
			}

			if ( '\0' == *current || current == realm ) {
				/* allow empty resource */
				success = TRUE;
				result->resource = g_strdup ("/");
				goto error;
			}
			
			result->realm = g_strndup (realm, current-realm);

			g_assert ('/' == *current);
		}
	}

	result->resource = g_strdup (current);

	success = TRUE;
error:

	if ( ! success ) {
		ammonite_url_free (result);
		return NULL;
	} else {
		return result;
	}
}

/* scheme should have trailing colon */
static char *
make_new_uri (const char * scheme, EazelProxy_User *user_info, const AmmoniteParsedURL *parsed)
{
	g_return_val_if_fail ( NULL != user_info, NULL );
	g_return_val_if_fail ( NULL != parsed, NULL );

	if (EazelProxy_AUTHENTICATED == user_info->login_state 
	    && 0 != user_info->proxy_port
	) {
		return g_strdup_printf ("%s//localhost:%d%s",
				scheme, user_info->proxy_port, parsed->resource );
	} else {
		return NULL;
	}
}

/* FIXME: bugzilla.eazel.com 2850: should cache the result and follow with a Listener */
/* FIXME: bugzilla.eazel.com 2850: should support realms */
static EazelProxy_User *
usercontrol_find_user (const char *user, const char *realm)
{
	CORBA_Environment ev;
	EazelProxy_User *ret = NULL;
	EazelProxy_UserList *userlist = NULL;
	CORBA_unsigned_long i;

	CORBA_exception_init (&ev);

	userlist = EazelProxy_UserControl_get_active_users (gl_user_control, &ev);

	if ( NULL == userlist || CORBA_NO_EXCEPTION != ev._major ) {
		goto error;
	}

	for (i = 0 ; i < userlist->_length ; i++ ) {
		EazelProxy_User *current;

		current = userlist->_buffer + i;
		if ( NULL != current->user_name
			&& 0 == strcmp (current->user_name, user)
		) {
			ret = EazelProxy_User_duplicate (current);
			break;
		} 
	} 

error:
	CORBA_free (userlist);
	CORBA_exception_free (&ev);

	return ret;
}

/*
 * Public Library Functions
 */


gboolean
ammonite_init (PortableServer_POA poa)
{
	gboolean ret;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	gl_poa = poa;
	gl_user_control = (EazelProxy_UserControl) oaf_activate_from_id (IID_EAZELPROXY, 0, NULL, &ev);

	ret = (CORBA_NO_EXCEPTION == ev._major);

	CORBA_exception_free (&ev);

	return ret;
}

void
ammonite_shutdown (void)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	
	if (CORBA_OBJECT_NIL != gl_user_control) {
		CORBA_Object_release (gl_user_control, &ev);
	}

	CORBA_exception_free (&ev);
}

EazelProxy_UserControl
ammonite_get_user_control (void)
{
	return gl_user_control;
}

PortableServer_POA
ammonite_get_poa (void)
{
	return gl_poa;
}

/* FIXME respawn/reconnect to ammonite when it goes away */
AmmoniteError
ammonite_http_url_for_eazel_url (const char *orig_url, /* OUT */ char ** new_url)
{
	AmmoniteParsedURL *parsed 	= NULL;
	EazelProxy_User *user_info 	= NULL;
	char *ret			= NULL;
	AmmoniteError err		= ERR_Success;
	CORBA_Environment ev;

	g_return_val_if_fail (NULL != new_url, EAZELPROXY_AUTHN_FAIL_ARG_INVALID);
	g_return_val_if_fail (NULL != orig_url, EAZELPROXY_AUTHN_FAIL_ARG_INVALID);
	g_return_val_if_fail (CORBA_OBJECT_NIL != gl_user_control, EAZELPROXY_AUTHN_FAIL_ARG_INVALID);


	*new_url = NULL;

	CORBA_exception_init (&ev);
	
	parsed = ammonite_url_parse (orig_url);
	if ( NULL == parsed ) {
		ret = NULL;
		err = ERR_BadURL;
		goto error;
	}

	/* Use the default user? */
	if (NULL == parsed->user) {
		user_info = EazelProxy_UserControl_get_default_user (gl_user_control, &ev);
		if (CORBA_USER_EXCEPTION == ev._major) {
			err = ERR_UserNotLoggedIn;
			user_info = NULL;
		} else if (CORBA_SYSTEM_EXCEPTION == ev._major) {
			err = ERR_CORBA;
			user_info = NULL;
		}

	} else {
		/* A user was specified in the URL */
		user_info = usercontrol_find_user (parsed->user, parsed->realm);
		if ( NULL == user_info) {
			err = ERR_UserNotLoggedIn;
		}
	}

	if ( NULL == user_info ) {
		ret = NULL;
		g_assert (err != ERR_Success);
	} else {
		ret = make_new_uri (":", user_info, parsed);
	}

	CORBA_free (user_info);
	user_info = NULL;

	*new_url = ret;
	ret = NULL;	
error:
	ammonite_url_free (parsed);
	CORBA_exception_free (&ev);

	return err;
}

static gboolean
is_http_localhost (const char *uri, /*OUT*/ unsigned *p_port,  /*OUT*/ const char ** p_path)
{
	static const char * uri_prefix_1 = "http://localhost:";
	static const char * uri_prefix_2 = "http://127.0.0.1:";

	const char * port_marker;
	const char * port_end;

	*p_port = 0;
	*p_path = NULL;	

	if (NULL == uri) {
		return FALSE;
	}

	if (0 == strncmp (uri, uri_prefix_1, strlen (uri_prefix_1))) {
		port_marker = uri +strlen (uri_prefix_1);
	} else if (0 == strncmp (uri, uri_prefix_1, strlen (uri_prefix_2))) {
		port_marker = uri +strlen (uri_prefix_2);
	} else {
		return FALSE;
	}

	*p_port = strtoul (port_marker, (char **)&port_end, 10);

	if (NULL == port_end) {
		return FALSE;
	}

	*p_path = port_end;

	return TRUE;
}

AmmoniteError 
ammonite_eazel_url_for_http_url (const char *orig_url, /* OUT */ char ** p_new_url)
{
	unsigned port;
	const char *path;
	EazelProxy_UserList *users;
	CORBA_Environment ev;
	CORBA_unsigned_long i;
	EazelProxy_User *cur;

	CORBA_exception_init (&ev);

	*p_new_url = NULL;

	if ( ! is_http_localhost (orig_url, &port, &path)) {
		return ERR_BadURL;
	}
	
	/* FIXME this is really inefficient.  I should keep this list 
	 * locally and update via UserListener
	 */

	users = EazelProxy_UserControl_get_active_users (gl_user_control, &ev);

	if (CORBA_NO_EXCEPTION != ev._major) {
		CORBA_exception_free (&ev);
		return ERR_CORBA;
	}


	for (i = 0; i < users->_length ; i++) {
		cur = users->_buffer + i;

		if (cur->proxy_port == port) {
			break;
		}
	}

	if (i < users->_length) {
		if (cur->is_default) {
			*p_new_url = g_strconcat ("eazel-services://", path, NULL);
		} else {
			*p_new_url = g_strconcat ("eazel-services://", 
							cur->user_name, path, NULL);
		}

	} else {
		CORBA_free (users);
		return ERR_UserNotLoggedIn;
	}

	CORBA_free (users);
	CORBA_exception_free (&ev);

	return ERR_Success;
}

const char *
ammonite_fail_code_to_string (CORBA_long code)
{

	static const char * fail_code_strings[] = {
		"(invalid)",
		"Argument Invalid",
		"User Exists",
		"Insufficient Resources",
		"Authentication Failure",
		"Network Error",
		"Unexpected Server Response",
		"User Canceled" 
	};

	if (code >= 0 && code <= (sizeof (fail_code_strings) / sizeof (fail_code_strings[0]))) {
		return fail_code_strings [code];
	}

	return "(unknown)";
}

/*
 * ammonite_get_default_user_username
 *
 * Returns username of the currently logged-in default Eazel Service User
 * or NULL if there isn't one
 */

char *
ammonite_get_default_user_username (EazelProxy_UserControl user_control)
{
	CORBA_Environment       ev;
	EazelProxy_User         *user;
	char *			ret = NULL;

	CORBA_exception_init (&ev);

	if (CORBA_OBJECT_NIL != user_control) {

		user = EazelProxy_UserControl_get_default_user (user_control, &ev);

		if (CORBA_NO_EXCEPTION != ev._major) {
			DEBUG_MSG (("No Eazel Service User is currently logged in"));
			ret = NULL;
		} else {
			DEBUG_MSG (("Default Eazel Service User is '%s'", user->user_name));
			ret = g_strdup (user->user_name);
			CORBA_free (user);
		}
	}

	CORBA_exception_free (&ev);
	return ret;
}
