/* $Id: authenticate.c,v 1.1.1.1 2001-01-16 15:26:08 ghudson Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "authenticate.h"
#include "utils.h"
#include "http-connection.h"
#include "digest.h"
#include "sock.h"
#include "log.h"

#include <glib.h>
#include <gnome-xml/entities.h>
#include <gnome-xml/parser.h>
#include <gnome-xml/tree.h>
#include <liboaf/liboaf.h>


/* DEBUG ONLY !!!*/
#undef PHASE2_DOES_GET

/*******************************************************************
 * Types
 *******************************************************************/

#define AUTHENTICATE_STATE_MAGIC 0x61757468
#define IS_AUTHENTICATE_STATE(conn) ((conn) && AUTHENTICATE_STATE_MAGIC == (conn)->magic)

typedef	enum {
	AUTHN_HTTP0,
	AUTHN_HTTP1,
} AuthenticateStateState;

typedef struct {
	guint32	magic;

	/* Arguments */
	char *			username;
	char *			password;
	gpointer 		user_data;
	AuthenticateCallbackFn 	callback_fn;

	/* State */
	HTTPRequest *		request;
	AuthenticateStateState	state;

	/* Return Values */
	DigestState *		p_digest;
} AuthenticateState;

/*******************************************************************
 * Module utils
 *******************************************************************/

/* from impl-eazelproxy.c */
extern gchar * gl_user_level;
extern gboolean gl_user_level_have;

static GList * 
authenticate_add_default_headers (GList *header_list)
{

	if (gl_user_level_have) {
		header_list = g_list_append (header_list, 
				g_strdup_printf ("X-Eazel-User-Level: %s", gl_user_level)
			      );
	}

	header_list = g_list_prepend (header_list, 
			g_strdup_printf ("User-Agent: ammonite/%s", 
			VERSION)
		      );

	return header_list;
}


/*******************************************************************
 * authenticate
 * Asynchronous HTTP digest authenticate routines
 *******************************************************************/


static void authenticate_finish_failed (AuthenticateState *state, long code, const char *http_response);
static void authenticate_finish_success (AuthenticateState *state, char *body, size_t body_size);

static void authenticate_http_callback (gpointer user_data, Socket *sock, gboolean success);

/**
 * authenticate_make_body
 * 
 * Eazel Services wants a list of Trilobite components to be posted
 * during the second phase of the authentication setup, so that the
 * service backend can know what component versions to expect
 */

#ifndef PHASE2_DOES_GET
static char *
authenticate_make_body ()
{

#define AUTHN_REPORT_OAF_QUERY "trilobite:name.defined()"

	CORBA_Environment ev;
	CORBA_unsigned_long i_server_info;
	CORBA_unsigned_long i_property;
	int cb_body_text;
	char * body_text 			= NULL;
	xmlNodePtr trilobite_node 		= NULL;	
	xmlNodePtr misc_node			= NULL;
	xmlNodePtr misc_item_node		= NULL;

	OAF_ServerInfoList *query_results 	= NULL;
	xmlDocPtr body_doc			= NULL;

	char *ret 				= NULL;

	CORBA_exception_init (&ev);

	body_doc = xmlNewDoc ("1.0");

	body_doc->root = xmlNewDocNode (body_doc, NULL, "ammonite_login_report", NULL);

	trilobite_node = xmlNewChild (body_doc->root, NULL, "trilobites", NULL);

	misc_node = xmlNewChild (body_doc->root, NULL, "misc", NULL);
		
	query_results = oaf_query (AUTHN_REPORT_OAF_QUERY, NULL, &ev);

	if ( ! NO_EXCEPTION (&ev) || NULL == query_results ) {
		goto error;
	}

	for( i_server_info = 0; i_server_info < query_results->_length; i_server_info++ ) {
		OAF_ServerInfo *server_info = &(query_results->_buffer[i_server_info]);
		xmlNodePtr trilo_inst_node;

		trilo_inst_node = xmlNewChild (trilobite_node, NULL, "trilobite", NULL);

		xmlSetProp (trilo_inst_node, "iid", server_info->iid);
		
		for ( i_property = 0; i_property < server_info->props._length ; i_property++) {
			OAF_Property *property = &(server_info->props._buffer[i_property]);

			if ( 0 == strcmp ("trilobite:name", property->name) 
				&& OAF_P_STRING == property->v._d 
			) {
				xmlSetProp (trilo_inst_node, "name", 
						property->v._u.value_string
				);
			} else if ( 0 == strcmp ("trilobite:version", property->name) 
				&& OAF_P_STRING == property->v._d 
			) {
				xmlSetProp (trilo_inst_node, "version", 
						property->v._u.value_string);
			} else if ( 0 == strcmp ("trilobite:version", property->name) 
				&& OAF_P_NUMBER == property->v._d 
			) {
				char *version;
				version = g_strdup_printf ("%f", property->v._u.value_number);
				xmlSetProp (trilo_inst_node, "version", version);
				g_free (version);
			}
		}
	}

	if (gl_user_level_have) {
		/* Add Nautilus UserLevel if we have it */
		misc_item_node = xmlNewChild (misc_node, NULL, "item", NULL);
		xmlSetProp (misc_item_node, "name", "nautilus_user_level");

		xmlSetProp (misc_item_node, "value", gl_user_level);
	}

	xmlDocDumpMemory (body_doc, (xmlChar **) &body_text, &cb_body_text);

	/* Note that xmlDocDumpMemory uses malloc not g_malloc */
	if (NULL != body_text) {
		ret = g_strdup (body_text);
		free (body_text);
	}
error:
	if (NULL != body_doc) {xmlFreeDoc (body_doc);}
	if (NULL != query_results) {CORBA_free (query_results);}

	CORBA_exception_free (&ev);

	return ret;
}
#endif /* PHASE2_DOES_GET */

static void
authenticate_state_free (AuthenticateState *state)
{
	if (NULL != state->p_digest) {
		digest_free (state->p_digest);
	}

	/* free rest of state */
	if (state->password) {
		memset (state->password, 0, strlen (state->password));
	}

	g_free (state->username);
	g_free (state->password);

	request_free (state->request);
	
	state->magic = 0;

	g_free (state);
}


static void
authenticate_finish_failed (AuthenticateState *state, long code, const char *http_response)
{
	EazelProxy_AuthnFailInfo fail_info;

	g_return_if_fail (NULL != state);

	fail_info.code = code;
	fail_info.http_result = (NULL != http_response) ? (char *) http_response : "";

	/* issue callback */
	if (state->callback_fn) {
		state->callback_fn (state->user_data, NULL, FALSE, &fail_info, NULL);
	}

	authenticate_state_free (state);
}

static void
authenticate_finish_success (AuthenticateState *state, char *body, size_t body_size)
{
	g_return_if_fail (NULL != state);

	if (state->callback_fn){
		state->callback_fn (state->user_data, state->p_digest, TRUE, NULL, body);
	}
	
	/*make sure digest isn't freed*/ 
	state->p_digest = NULL;

	authenticate_state_free (state);
}


static gboolean
authenticate_init_digest (AuthenticateState *state, GList *header_list)
{
	GList *list_node;
	char *line;

	for (	list_node = g_list_first (header_list) ;
		NULL != list_node ;
		list_node = g_list_next (list_node)
	) {
		line = (char *) list_node->data;
		if (STRING_STARTS_WITH (line, HTTP_AUTHENTICATE_HEADER)) {
			state->p_digest = digest_init (state->username, state->password, line);

			if (NULL != state->p_digest) {
				return TRUE;
			} else {
				return FALSE;
			}
		}
	}

	return FALSE;
}

static void
authenticate_do_phase_2 (AuthenticateState *state)
{
	GList *header_list = NULL;
	char * authn_body = NULL;

	header_list = g_list_prepend (header_list, 
			digest_gen_response_header (
				state->p_digest, 
				state->request->path, 
#ifdef PHASE2_DOES_GET
				"GET"
#else
				"POST"
#endif
			)
	);

#ifdef PHASE2_DOES_GET
	authn_body = NULL;
#else
	header_list = g_list_prepend (header_list, g_strdup ("Content-Type: text/xml"));
	header_list = authenticate_add_default_headers (header_list);

	authn_body = authenticate_make_body();

	u_replace_string(&(state->request->method), g_strdup ("POST"));
#endif

	/* Make confirmation request */
	if ( ! http_connection_connect_submit (state->request, 
			header_list,
			authn_body,
			(NULL == authn_body) ? 0 : strlen (authn_body) ,
			state, 
			authenticate_http_callback)
	) {
		log ("ERROR: Authenticate failed: DNS error");
		authenticate_finish_failed (state, EAZELPROXY_AUTHN_FAIL_NETWORK, NULL);
	}
}

static void
authenticate_http_read_callback /*HttpReadCallbackFn*/ (
	gpointer user_data, 
	char *status_line, 		/*Callee is responsible for freeing */
	GList *header_list,
	char *body,			/*Callee is responsible for freeing */
	size_t body_size,
	gboolean completed		/*TRUE if read completed successfully */
) {
	AuthenticateState *state;
	HttpStatusLine parsed_status_line;

	state = (AuthenticateState *) user_data;

	g_assert (state);
	g_assert (IS_AUTHENTICATE_STATE (state));

	if ( ! completed ) {
		log ("ERROR: Authenticate failed: HTTP response was incomplete");
		authenticate_finish_failed (state, EAZELPROXY_AUTHN_FAIL_NETWORK, NULL);
		goto done;
	}

	switch (state->state) {
	case AUTHN_HTTP0:
		/* FIXME bugzilla.eazel.com 2770: ?Should we accept 200 OK here? */
		if (! http_parse_status_line (status_line, &parsed_status_line)
			|| HTTP_RESPONSE_AUTHN_REQUIRED != parsed_status_line.code
		) {
			log ("ERROR: Authenticate failed: got '%s %s' on initial query instead of 401", 
				parsed_status_line.str_code, parsed_status_line.str_reason
			);
			authenticate_finish_failed (state, EAZELPROXY_AUTHN_FAIL_SERVER, status_line);
			goto done;
		} 

		if ( ! authenticate_init_digest (state, header_list) ) {
			log ("ERROR: Authenticate failed: invalid digest header");
			authenticate_finish_failed (state, EAZELPROXY_AUTHN_FAIL_SERVER, NULL);
			goto done;
		}

		state->state = AUTHN_HTTP1;
		authenticate_do_phase_2 (state);

	break;

	case AUTHN_HTTP1:
		if (! http_parse_status_line (status_line, &parsed_status_line)
			|| HTTP_RESPONSE_OK != parsed_status_line.code
		) {
			log ("Authenticate failed: got '%s %s' on phase2 query instead of 200 OK", 
				parsed_status_line.str_code, parsed_status_line.str_reason
			);
			/* This is a bad username-password */
			authenticate_finish_failed (state, EAZELPROXY_AUTHN_FAIL_AUTHN, NULL);
			goto done;
		} else {
			authenticate_finish_success (state, body, body_size);
			/* authenticate_finish_success frees body */
			body = NULL;			
		}
	break;

	default:
		g_assert (FALSE);
	}

done:
	g_free (status_line);
	g_free (body);
}

static void /*HttpCallbackFn*/
authenticate_http_callback (gpointer user_data, Socket *sock, gboolean success)
{
	AuthenticateState *state;

	state = (AuthenticateState *) user_data;

	g_assert (state);
	g_assert (IS_AUTHENTICATE_STATE (state));

	switch (state->state) {
		case AUTHN_HTTP0:
			if (!success) {
				authenticate_finish_failed (state, EAZELPROXY_AUTHN_FAIL_NETWORK, NULL);
			} else {
				http_connection_read (sock, state, authenticate_http_read_callback);
			}
		break;

		case AUTHN_HTTP1:
			if (!success) {
				authenticate_finish_failed (state, EAZELPROXY_AUTHN_FAIL_NETWORK, NULL);
			} else {
				http_connection_read (sock, state, authenticate_http_read_callback);
			}
		break;

		default:
			g_assert (FALSE);
	
	}
}

void  authenticate_user (
	const char *username,
	const char *password,
	const HTTPRequest *request,
	gpointer user_data,
	AuthenticateCallbackFn callback_fn
) {
	AuthenticateState *state;
	GList *header_list = NULL;

	state = g_new0 (AuthenticateState, 1);

	state->callback_fn = callback_fn;
	state->username = g_strdup (username);
	state->password = g_strdup (password);

	state->user_data = user_data;
	
	state->magic = AUTHENTICATE_STATE_MAGIC;
	state->state = AUTHN_HTTP0;

	state->request = request_copy (request);
	

	u_replace_string (&(state->request->method), g_strdup ("GET"));
	u_replace_string (&(state->request->version), g_strdup ("1.0"));

	if (state->request->uri && (strcmp (state->request->uri, "http") != 0)) {
		u_replace_string (&(state->request->uri), g_strdup ("http"));
	}

	log ("Making authn request to host '%s:%u' path '%s'", request->host, (unsigned int)request->port, request->path);

	header_list = authenticate_add_default_headers (header_list);
	if ( !	http_connection_connect_submit (
			state->request, header_list, NULL, 0, 
			state, authenticate_http_callback
		)
	) {
		authenticate_http_callback (state, NULL, FALSE);
	}
}
