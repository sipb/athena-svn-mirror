/* $Id: http-connection.h,v 1.1.1.1 2001-01-16 15:26:27 ghudson Exp $
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

#ifndef _HTTP_CONNECTION_H_
#define _HTTP_CONNECTION_H_
#include "request.h"
#include <stdlib.h>
#include <glib.h>

/*
 * Some HTTP defines
 */

#define HTTP_RESPONSE_OK 200
#define HTTP_RESPONSE_AUTHN_REQUIRED 401
#define HTTP_AUTHENTICATE_HEADER "WWW-Authenticate:"


/*
 * Simple HTTP connection
 */

typedef struct HTTPConnection HTTPConnection;

/* Callback functions */
typedef void (*HttpCallbackFn)(gpointer user_data, Socket *sock, gboolean success);
typedef void (*HttpReadCallbackFn) (
	gpointer user_data, 
	char *status_line,	 	/*Callee is responsible for freeing */
	GList *header_list,		
	char *body,			/*Callee is responsible for freeing */
	size_t body_size,
	gboolean completed		/*TRUE if read completed successfully */
);

HTTPConnection *
http_connection_connect (
	const HTTPRequest *request,
	GList *		header_list,
	gpointer 	user_data, 
	HttpCallbackFn 	callback
#ifndef NO_DEBUG_MIRRORING
	,FILE * fm_mirror, FILE * to_mirror
#endif /* DEBUG */ 
);

HTTPConnection *
http_connection_connect_submit (
	const HTTPRequest *request,
	GList *		header_list,
	char *		submit_body,
	size_t 		cb_submit_body,	/*cb_ means "count bytes' */
	gpointer 	user_data, 
	HttpCallbackFn 	callback
);

void
http_connection_abort (HTTPConnection *connection);

void
http_connection_read (
	Socket *sock,
	gpointer user_data,
	HttpReadCallbackFn callback
);

/*
 * Simple HTTP functions
 */

typedef struct {
	char *str_version;
	char *str_code;
	int code;
	char *str_reason;
} HttpStatusLine;

gboolean http_parse_status_line (char *status_line, HttpStatusLine *status_struct);

typedef struct HttpGetHeaderState HttpGetHeaderState;

char * http_get_headers (Socket *sock, HttpGetHeaderState **pp_state);
void http_get_headers_state_free (HttpGetHeaderState *p_state);
gboolean http_parse_authn_header ( /*INOUT*/ char **p_header, /*OUT*/ char **p_key, /*OUT*/ char **p_value );


#endif /* _HTTP_CONNECTION_H_ */
