/* $Id: eazelproxy-change-password.c,v 1.1.1.1 2001-01-16 15:26:35 ghudson Exp $
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

#include "eazelproxy-change-password.h"

#include "eazelproxy.h"
#include "log.h"
#include "utils.h"
#include "http-connection.h"
#include "digest.h"
#include "request.h"
#include "sock.h"
#include "session.h"

#include <libammonite.h>
#include <orb/orbit.h>
#include <glib.h>
#include <gnome-xml/entities.h>
#include <gnome-xml/parser.h>
#include <gnome-xml/tree.h>

/*
 * Types
 */

#define DEFAULT_SET_PASSWORD_PATH "account/password/change"
/* #define DEFAULT_SET_PASSWORD_PATH "chpw.pl" */

typedef enum {
	SetPW_Submitting,
	SetPW_ReadStatus,
	SetPW_ReadHeader,
	SetPW_ReadBody
} SetUserPasswordState;

typedef struct {
	SetUserPasswordState	state;
	EazelProxy_AuthnInfo *	authninfo;
	User *			user;
	gchar *			new_password;
	HttpGetHeaderState *	header_state;
	EazelProxy_AuthnCallback callback;
	GList *			body_pieces_list;
} SetUserPasswordInfo;


/*
 * Function Prototypes
 */

static void
set_user_password_state_info_free (SetUserPasswordInfo *info);

static void
set_user_password_failed_force_logout (
	SetUserPasswordInfo *state_info, 
	CORBA_long code, 
	const char * response
);

static void
set_user_password_failed (
	SetUserPasswordInfo *state_info, 
	CORBA_long code, 
	const char * response
);

static void
set_user_password_success (SetUserPasswordInfo *state_info);

static void /*HttpCallbackFn*/
set_user_password_http_callback (gpointer user_data, Socket *sock, gboolean success);

static void
set_user_password_submit_request (SetUserPasswordInfo *state_info);

/*
 * Implementation
 */

static void
set_user_password_state_info_free (SetUserPasswordInfo *info)
{
	CORBA_Environment ev;
	/* Note: doesn't free "user"--its expected that this is a global object */

	CORBA_exception_init (&ev);

	CORBA_free (info->authninfo);
	if (info->new_password) {
		memset (info->new_password, 0, strlen (info->new_password));
	}
	g_free (info->new_password);
	CORBA_Object_release (info->callback, &ev);
	http_get_headers_state_free (info->header_state);
	piece_response_free (info->body_pieces_list);
	g_free (info);

	CORBA_exception_free (&ev);
}

static void
set_user_password_failed_no_thaw (
	SetUserPasswordInfo *state_info, 
	CORBA_long code, 
	const char * response
) {
	CORBA_Environment ev;
	EazelProxy_AuthnFailInfo fail_info;

	g_return_if_fail (NULL != state_info);

	user_set_login_state (state_info->user, EazelProxy_AUTHENTICATED);

	fail_info.code = code;
	fail_info.http_result = (NULL != response) ? (char *)response : "";

	CORBA_exception_init (&ev);

	EazelProxy_AuthnCallback_failed (
		state_info->callback, 
		user_get_EazelProxy_User(state_info->user), 
		&fail_info,
		&ev
	);

	set_user_password_state_info_free (state_info);

	CORBA_exception_free (&ev);
}

static void
set_user_password_failed (
	SetUserPasswordInfo *state_info, 
	CORBA_long code, 
	const char * response
) {
	EazelProxy_User * user = user_get_EazelProxy_User (state_info->user);
	set_user_password_failed_no_thaw (state_info, code, response);
	session_thaw (session_from_port (user->proxy_port));
}

/**
 * set_user_password_failed_force_logout
 *
 * this is sort of a hack.  Basically, if we submit a change password
 * request but don't get a response, we don't know if the server got 
 * the request and committed or not.  Since we can't handle getting a 401
 * back from the server, we log out the user in this case, thus forcing
 * them to log back in with the modal dialog.  This case will
 * occur only very very rarely.
 */
static void
set_user_password_failed_force_logout (
	SetUserPasswordInfo *state_info, 
	CORBA_long code, 
	const char * response
) {
	User *user;

	user = state_info->user;
	
	listener_broadcast_user_logout (user);

	set_user_password_failed_no_thaw (state_info, code, response);

	user_deactivate (user);
}


static void
set_user_password_success (SetUserPasswordInfo *state_info)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	user_set_login_state (state_info->user, EazelProxy_AUTHENTICATED);

	digest_change_password ( user_get_digest_state (state_info->user), state_info->new_password );

	session_thaw (session_from_port (user_get_EazelProxy_User(state_info->user)->proxy_port));
	
	EazelProxy_AuthnCallback_succeeded (
		state_info->callback, 
		user_get_EazelProxy_User (state_info->user), 
		&ev
	);

	set_user_password_state_info_free (state_info);

	CORBA_exception_free (&ev);
}


/* <?xml version="1.0" encoding="utf-8" ?>
 * <response code="%d">[localized text]</response>
 *
 * The codes should be as follows:
 *
 * code    significance            localized message
 * 0       OK--pw changed          <none required>
 * 1       bad password            <explain why the password is bad--this will be displayed directly>
 * 2       new pw's do not match   <none required>  (I'll never get this error since I'll check the PW's client-side)
 * 3       Old password incorrect  <none required>
 */

#define CHPW_RESPONSE_OK	0
#define CHPW_RESPONSE_BAD_PW	1
#define CHPW_RESPONSE_BAD_OLDPWD 3

static void
set_user_password_http_read_callback /*HttpReadCallbackFn*/ (
	gpointer user_data, 
	char *status_line, 		/*Callee is responsible for freeing */
	GList *header_list,
	char *body,			/*Callee is responsible for freeing */
	size_t body_size,
	gboolean completed		/*TRUE if read completed successfully */
) {
	SetUserPasswordInfo *state_info;
	HttpStatusLine parsed_status_line;
	xmlDocPtr body_doc = NULL;
	xmlChar * code_property = NULL;
	long code;
	char * text;
	xmlChar * code_property_out;
	char *body_copy = NULL;
	char *body_copy_ptr = NULL;

	state_info = (SetUserPasswordInfo *)user_data;

	g_assert (NULL != state_info);

	if ( ! completed ) {
		log ("WARN: Change PW: Unexpected EOF");

		set_user_password_failed_force_logout (
			state_info,
			EAZELPROXY_AUTHN_FAIL_NETWORK,
			NULL
		);
		goto done;
	}

	if (! http_parse_status_line (status_line, &parsed_status_line)
	    || HTTP_RESPONSE_OK != parsed_status_line.code
	) {
		log ("Change password failed; HTTP response %s: '%s'", parsed_status_line.str_code,
		     parsed_status_line.str_reason);
		set_user_password_failed (
			state_info,
			EAZELPROXY_AUTHN_FAIL_SERVER,
			status_line
		);
		goto done;
	}

#define EVIL_PR2_HACK

	/* evil, EVIL bug in libxml!
	 * even if you pass in a length to the xml parser, it requires the buffer to be null-terminated.
	 * so we need to copy the buffer and add a null terminator. :(
	 */
	body_copy = g_malloc (body_size+1);
	memcpy (body_copy, body, body_size);
	body_copy[body_size] = 0;

	/* [insert usual complaints about how fragile and useless libxml is...] */
	for (body_copy_ptr = body_copy; (*body_copy_ptr) && (*body_copy_ptr != '<'); body_copy_ptr++)
		;

	body_doc = xmlParseMemory (body_copy_ptr, body_size - (body_copy_ptr - body_copy));

	if ( NULL == body_doc ) {
		log ("Change PW: XML response was bad");
		log ("XML was: '%s'", body_copy_ptr);
		set_user_password_failed_force_logout (
						       state_info,
						       EAZELPROXY_AUTHN_FAIL_SERVER,
						       NULL
						       );
		goto done;
	}

#ifdef EVIL_PR2_HACK
	/* so... what ammonite is expecting is nothing like what triggerfish is sending.
	 * parse what triggerfish sends and translate to a code.
	 */

	if (body_doc != NULL) {
		/* ALSO: libxml is completely unable to parse this http-equiv block and i don't have time to
		 * mess with it.  so, i give up, we'll search for substrings.
		 */
		if (strstr (body_copy_ptr, "/services") != NULL) {
			code = CHPW_RESPONSE_OK;
			text = "You are the luckiest person alive.";
			log ("Change PW OK");
		} else {
			code = CHPW_RESPONSE_BAD_PW;
			text = "The new password is wrong, for some reason.";
			log ("Change PW BAD");
		}
	}
	code_property_out = code_property_out;
#else

	if ( 	NULL == body_doc->root ||
		0 != g_strcasecmp ( body_doc->root->name, "response" )
		|| NULL == ( code_property = xmlGetProp (body_doc->root, "code") )
	) {
		log ("Change PW: XML response was bad (no response/code key?)");
		log ("XML was: '%s'", body_copy_ptr);
		set_user_password_failed_force_logout (
			state_info,
			EAZELPROXY_AUTHN_FAIL_SERVER,
			NULL
		);
		goto done;
	}

	/*
	 * You cannot simply access the content via ->content
	 *  if the content is buffered (also to access from
	 *   the xmlBufferPtr-stuff)!
	 */
	#ifdef XML_USE_BUFFER_CONTENT
	text = xmlNodeGetContent(body_doc->root);
	#else
	text = body_doc->root->content;
	#endif
	code = strtol (code_property, (char **)&code_property_out, 10);

	if (code_property == code_property_out) {
		log ("Change PW: XML response was bad (couldn't decode code)");
		log ("XML was: '%s'", body_copy_ptr);
		set_user_password_failed_force_logout (
			state_info,
			EAZELPROXY_AUTHN_FAIL_SERVER,
			NULL
		);
		goto done;
	} 
#endif

	if ( CHPW_RESPONSE_OK == code ) {
		log ("Change PW: Success!");
		set_user_password_success (state_info);
	} else {
		log ("Change PW: Got fail code %d text '%s'", code, text);
		set_user_password_failed (
			state_info,
			EAZELPROXY_PASSWORD_FAIL_OFFSET + code,
			text
		);
	}

	if (code_property) {
		free (code_property);	
	}

done:
	g_free (body_copy);
	g_free (status_line);
	g_free (body);
	if (body_doc) {
		xmlFreeDoc (body_doc);
	}
}

static void /*HttpCallbackFn*/
set_user_password_http_callback (gpointer user_data, Socket *sock, gboolean success)
{
	SetUserPasswordInfo *state_info;

	g_return_if_fail (NULL != user_data);

	state_info = (SetUserPasswordInfo *)user_data;

	if ( ! success ) {
		/* Cleans up state_info as well */
		log ("WARN: Change PW: HTTP error");
		set_user_password_failed_force_logout (
			state_info,
			EAZELPROXY_AUTHN_FAIL_NETWORK,
			NULL
		);
	} else {	

		http_connection_read (sock, state_info, set_user_password_http_read_callback);	
	}
}

static void
set_user_password_submit_request (SetUserPasswordInfo *state_info)
{
	HTTPRequest *request;
	char *post_data;
	GList *header_list = NULL;
	char *old_password_escaped = NULL, *new_password_escaped = NULL;

	g_return_if_fail (NULL != state_info);

	request = request_new();

	request_parse_url (user_get_EazelProxy_User (state_info->user)->services_redirect_uri, request);

	u_concat_replace_string (&request->path, DEFAULT_SET_PASSWORD_PATH);

	u_replace_string (&(request->method), g_strdup ("POST"));
	u_replace_string (&(request->version), g_strdup ("1.0"));
	u_replace_string (&(request->uri), g_strdup ("http"));

	old_password_escaped = util_url_encode (state_info->authninfo->password);
	new_password_escaped = util_url_encode (state_info->new_password);

	post_data = 	g_strdup_printf (
				"_oldpwd=%s&_newpwd=%s&_newpwd2=%s",
				old_password_escaped,
				new_password_escaped,
				new_password_escaped
			);

	g_free (old_password_escaped);
	old_password_escaped = NULL;
	g_free (new_password_escaped);
	new_password_escaped = NULL;
		
	header_list = g_list_prepend (header_list, g_strdup ("Content-Type: application/x-www-form-urlencoded"));
	header_list = g_list_prepend (header_list, 
			g_strdup_printf ("User-Agent: ammonite/%s", 
			VERSION)
		      );

	header_list = g_list_prepend (header_list, 
			digest_gen_response_header (
				user_get_digest_state (state_info->user), 
				request->path, 
				"POST"
			)
	);

	log ("Change PW: Making change password request to host '%s' path '%s'", request->host, request->path);

	/* Make password change request request */
	if ( ! http_connection_connect_submit (
			request, 
			header_list,
			post_data,
			(NULL == post_data) ? 0 : strlen (post_data) ,
			state_info, 
			set_user_password_http_callback)
	) {
		/* Couldn't make the HTTP connection */

		log ("ERROR: Change PW: DNS error\n");

		set_user_password_failed (
			state_info,
			EAZELPROXY_AUTHN_FAIL_NETWORK,
			NULL
		);
		/* and we're done with this request*/
	}

	request_free (request);
}

static void /*ProxyFreezeCb*/
frozen_session_cb(gpointer freeze_user_data)
{
	set_user_password_submit_request ((SetUserPasswordInfo *) freeze_user_data);
}

void
set_user_password (User *user, EazelProxy_AuthnInfo *authninfo, const char * new_password,  EazelProxy_AuthnCallback callback)
{
	SetUserPasswordInfo *state_info;

	user_set_login_state (user, EazelProxy_PASSWORD_CHANGING);

	/* Create state info */

	state_info = g_new0 (SetUserPasswordInfo, 1);
	state_info->new_password = g_strdup (new_password);
	state_info->callback = callback;
	state_info->state = SetPW_Submitting;

	state_info->authninfo = authninfo;
	state_info->user = user;
	
	session_schedule_freeze (
		session_from_port (user_get_EazelProxy_User (user)->proxy_port), 
		state_info, 
		frozen_session_cb
	);
}


