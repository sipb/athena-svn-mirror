/* $Id: http-connection.c,v 1.1.1.1 2001-01-16 15:26:27 ghudson Exp $
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

#include "http-connection.h"
#include "request.h"
#include "sock.h"
#include "proxy.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef NO_DEBUG_MIRRORING
#include <unistd.h>		/* for getpid for mirroring */
#endif

/*
 * HTTPConnection types
 */ 

struct HTTPConnection {
	guint32		magic;
	HTTPRequest	*request;
	GList		*header_list;
	Socket		*server_socket;
	gboolean	proxy_connect;	/* TRUE if we're connecting to a proxy */
	HttpCallbackFn	callback;
	gpointer	user_data;
};

typedef struct {
	char *		submit_body;
	size_t		cb_submit_body;  /* cb_ means "count byte" */
	HttpCallbackFn	callback;
	gpointer	user_data;
} HTTPConnectSubmit;

#define HTTP_CONNECTION_MAGIC 0x17C3220C
#define IS_HTTP_CONNECTION(conn) ((conn) && HTTP_CONNECTION_MAGIC == (conn)->magic)


/*
 * HttpGetHeaderState types
 */
struct HttpGetHeaderState {
	char *header;
	gboolean end_reached;
};

/*
 * http_connection_read types
 */

typedef enum {
	HTTP_READ_STATUS,
	HTTP_READ_HEADER,
	HTTP_READ_BODY
} HttpReadState;

typedef struct {
	HttpReadState state;

	Socket *socket;

	char *status_line;
	HttpGetHeaderState *header_state;
	GList *header_list;
	GList *body_pieces_list;

	gpointer user_data;
	HttpReadCallbackFn callback;
} HttpReadStateInfo;

/*
 * Function Prototypes
 */

static void http_connection_sock_connect (Socket *sock, int success);
static void http_connection_connect_sock_eof (Socket *sock);
static void http_connection_free (HTTPConnection *http_connection);
static void http_connection_finish (HTTPConnection *http_connection, gboolean success);

static void /* HttpCallbackFn */ connect_submit_http_callback (gpointer user_data, Socket *sock, gboolean success);

static void http_read_state_info_free (HttpReadStateInfo *state_info);
static void /* SocketReadFn */ http_read_sock_read (Socket *sock);
static void /* SocketEofFn */ http_read_sock_eof (Socket *sock);



gboolean
http_parse_status_line (char *status_line, HttpStatusLine *status_struct)
{
	gboolean ret;
	char *curpos;
	
	g_return_val_if_fail ( NULL != status_line, FALSE );
	g_return_val_if_fail ( NULL != status_struct, FALSE );

	ret = TRUE;

	status_struct->str_version = status_line;

	curpos = strchr (status_line, ' ');

	if (NULL == curpos || 0==(curpos + 1)) {
		ret = FALSE;
		goto error;
	}

	*(curpos++) = 0;

#define HTTP_VERSION_BEGIN "HTTP/"

	if (0 != strncmp (status_struct->str_version,  HTTP_VERSION_BEGIN, strlen (HTTP_VERSION_BEGIN))) {
		ret = FALSE;
		goto error;
	}

	status_struct->str_code = curpos;
	
	curpos = strchr (curpos, ' ');

	if (NULL == curpos || 0==(curpos + 1)) {
		ret = FALSE;
		goto error;
	}

	*(curpos++) = 0;

	status_struct->code = atoi (status_struct->str_code);
	status_struct->str_reason = curpos;

	ret = TRUE;

error:
	return ret;
}

/**
 * http_get_headers
 * 
 * retrieve header.  Returns NULL if not yet done, empty string if end of
 * headers reached, or header string if header is parsed
 *
 * *pp_state should be NULL initially.  It should be returned during
 * subequent calls, and is set to NULL again when a header is read or end
 * of headers is reached
 *
 * Assumes newlines have been trimmed from line
 *
 * all non-null return values must be g_free()'d
 */
 
char *
http_get_headers (Socket *sock, HttpGetHeaderState **pp_state)
{
	char *ret = NULL;
	char *line;
	
	g_assert (sock);
	g_assert (IS_SOCKET(sock));
	g_assert (pp_state);

	if ( NULL == *pp_state ) {
		*pp_state = g_new0 (HttpGetHeaderState, 1);
	}

	/* We can only return NULL if socket_getline returns NULL 
	 * Since this is true, we need to make make sure that if the
	 * end has been reached and no header exists to return,
	 * we need to clean up and return "", and not wait until next
	 * call to return NULL
	 */

	if ( ! (*pp_state)->end_reached) {
		for ( line = socket_getline (sock);
		      NULL != line ;
		      line = socket_getline (sock)
		) {
			if ( '\0' == *line ) {
				/* note comment above-- (*pp_state)->header be NULL.
				 * if it is, we need to return the finish code, ""
				 */
				ret = (*pp_state)->header;
				(*pp_state)->header = NULL;
				(*pp_state)->end_reached = TRUE;
				g_free (line);
				break;
			} else if (' ' == *line || '\t' == *line) {
				if ( (*pp_state)->header) {
					char *old;
					old = (*pp_state)->header;
					line = g_strstrip (line);
					(*pp_state)->header = g_strjoin (" ", (*pp_state)->header, line, NULL); 
					g_free (old);
				}
				g_free (line);
			} else {
				if ((*pp_state)->header) {
					ret = (*pp_state)->header;
					(*pp_state)->header = line;
					break;
				} else {
					(*pp_state)->header = line;
				}
			}
		}
	}

	if ((*pp_state)->end_reached && NULL == ret ) {
		http_get_headers_state_free (*pp_state);
		*pp_state = NULL;
		ret = g_strdup ("");
	}

	return ret;
}

/**
 * http_get_headers_state_free
 * 
 * Frees a HttpGetHeaderState struct
 */

void
http_get_headers_state_free (HttpGetHeaderState *p_state)
{
	if( p_state ) {
		g_free (p_state->header);
		g_free (p_state);
	}
}


/**
 * http_parse_authn_header
 *
 * Parses the contents of a WWW-Authenticate header
 *
 * *p_header points to the beginning of the contents in the first call.  It is
 * used as state for subsequent calls
 *
 * upon return:
 *  *p_key is a pointer to the key, if applicable
 *  *p_value is a pointer to the value if applicable
 *  the function returns TRUE if it has reached the ends of the contents ( *p_key 
 *  and *p_value will be NULL) or FALSE if it is still parsing 
 *  (at least *p_key will be non-null)
 *  
 */
   
gboolean
http_parse_authn_header ( /*INOUT*/ char **p_header, /*OUT*/ char **p_key, /*OUT*/ char **p_value )
{
	char *current;

	g_assert (p_header);
	g_assert (p_key);
	g_assert (p_value);

	/* ([^ \t,=])+(=(([^"][^,; \t]+) | ("([^"]+)")))?  */

	current = *p_header;
	*p_key = NULL;
	*p_value = NULL;

	/* Eat whitespace and seperators */
	for ( ; *current && (isspace (*current) || ',' == *current || '=' == *current) ; current++) ;

	if ( ! *current) {
		/* all done */
		return TRUE;
	}

	*p_key = current;

	/* Get key */
	for ( ; *current && ! (isspace (*current) || ',' == *current || '=' == *current) ; current++) ;

	if ( '=' == *current ) {
		*current++ = '\0';

		if ( '"' == *current ) {
			current++;
			*p_value = current;
			for ( ; *current && '"' != *current; current++ );
			if ( '"' == *current ) {
				*current++ = '\0';
			} else {
				*p_value = NULL;
			}
		} else {
			*p_value = current;
			for ( ; *current && ! ( isspace (*current) || ',' == * current || ';' == *current) ; current++ );
			*current++ = '\0';
			if ( '\0' == *p_value ) {
				*p_value = NULL;
			}
		}
	} else {
		*current++ = '\0';
	}
	
	*p_header = current;	

	return FALSE;
}

/**
 * http_connection_connect
 *
 * Make a simple HTTP request, through upstream proxy if applicable
 *
 * Returns with NULL on a DNS failure; callback function will *not* be called
 *
 * http_connection_connect will add a Content-Length header and a Host: header
 * 
 * submit_body will be g_free'd.  header_list will be collected as well
 *
 * "callback" is called on connect.  The user is responsable for all subsequent
 * socket operations.  If "success" is FALSE, then "sock" is NULL and has already
 * been disposed of. 
 */


HTTPConnection *
http_connection_connect (
	const HTTPRequest *request,
	GList *		header_list,
	gpointer 	user_data, 
	HttpCallbackFn 	callback
#ifndef NO_DEBUG_MIRRORING
	,FILE * fm_mirror_arg, FILE * to_mirror_arg
#endif /* DEBUG */ 
) {
	HTTPConnection *http_connection;
	Socket *server_socket;
	gboolean success;
	int err;

	g_return_val_if_fail ( NULL != request, FALSE);

	http_connection = g_new0(HTTPConnection,1);

	http_connection->magic = HTTP_CONNECTION_MAGIC;
	http_connection->request = request_copy (request);

	http_connection->header_list = header_list;

	http_connection->user_data = user_data;
	http_connection->callback = callback;

	http_connection->server_socket = server_socket = socket_new (NULL,0);

	socket_set_data (server_socket, http_connection);
	socket_set_read_fn (server_socket, NULL);
	socket_set_eof_fn (server_socket, http_connection_connect_sock_eof);


#ifndef NO_DEBUG_MIRRORING
	if (config.mirror_log_dir) {
		static size_t http_req_count;
		FILE *fm_mirror, *to_mirror;
		char *fm_mirror_filename, *to_mirror_filename;

		if ( NULL != fm_mirror_arg ) {
			socket_set_mirrors (server_socket, fm_mirror_arg, to_mirror_arg);
		} else {
			fm_mirror_filename = g_strdup_printf ("%s/ep-%d-%d-fm-http-req.log", config.mirror_log_dir, getpid(), http_req_count);
			to_mirror_filename = g_strdup_printf ("%s/ep-%d-%d-to-http-req.log", config.mirror_log_dir, getpid(), http_req_count);
			fm_mirror = fopen (fm_mirror_filename, "w");
			to_mirror = fopen (to_mirror_filename, "w");
			socket_set_mirrors (server_socket, fm_mirror, to_mirror);
			g_free (fm_mirror_filename);
			g_free (to_mirror_filename);

			http_req_count++;
		}
	}
#endif

	if (config.use_ssl && config.upstream_host) {
		http_connection->proxy_connect = FALSE;
		err = socket_connect_proxy_tunnel (
			server_socket, 
			http_connection->request->host, 
			http_connection->request->port,
			TRUE,
			http_connection_sock_connect
		);
		success = (0 == err) ? FALSE : TRUE;
	} else if (config.use_ssl && NULL == config.upstream_host ) {
		http_connection->proxy_connect = FALSE;
		err = socket_connect_ssl (
			server_socket, 
			http_connection->request->host, 
			http_connection->request->port,
			http_connection_sock_connect
		);
		success = (0 == err) ? FALSE : TRUE;
	} else if (!config.use_ssl && config.upstream_host) {
		http_connection->proxy_connect = TRUE;
		err = socket_connect (
			server_socket, 
			config.upstream_host, 
			config.upstream_port,
			http_connection_sock_connect
		);
		success = (0 == err) ? FALSE : TRUE;
	} else {
		http_connection->proxy_connect = FALSE;
		err = socket_connect (
			server_socket, 
			http_connection->request->host, 
			http_connection->request->port,
			http_connection_sock_connect
		);
		success = (0 == err) ? FALSE : TRUE;
	}

	if (!success) {
		http_connection->server_socket = NULL;
		/* http_connection_free does not close socket */
		http_connection_free (http_connection);
		socket_set_read_fn (server_socket, NULL);
		socket_set_eof_fn (server_socket, NULL);
		socket_close (server_socket);
	}

	return success ? http_connection : NULL;
}

/**
 * http_connection_connect_submit
 * 
 * Note, if FALSE is returned, the callback is never called!
 */

HTTPConnection *
http_connection_connect_submit (
	const HTTPRequest *request,
	GList *		header_list,
	char *		submit_body,
	size_t 		cb_submit_body,	/*cb_ means "count bytes' */
	gpointer 	user_data, 
	HttpCallbackFn 	callback
) {
	HTTPConnectSubmit * submit_state;
	HTTPConnection *ret;

	g_return_val_if_fail (NULL != request, FALSE);
	g_return_val_if_fail (NULL != callback, FALSE);

	submit_state = g_new0 (HTTPConnectSubmit, 1);

	submit_state->submit_body 	= submit_body; 
	submit_state->cb_submit_body 	= cb_submit_body; 
	submit_state->user_data 	= user_data;
	submit_state->callback 		= callback;

	/*
	 * Content-Length: header
	 */

	if (submit_state->submit_body) {
		header_list  = 	g_list_prepend(
					header_list, 
					g_strdup_printf (
						"Content-Length: %u", 
						submit_state->cb_submit_body
					)
				);
	}


	ret = 	http_connection_connect (
				request, header_list, (gpointer) submit_state, 
				connect_submit_http_callback
#ifndef NO_DEBUG_MIRRORING
				,NULL, NULL
#endif /* DEBUG */
			);

	if ( NULL == ret ) {
		g_free (submit_state->submit_body);
		submit_state->submit_body = NULL;
		g_free (submit_state);
	}
	
	return ret;
}

/**
 * http_connection_abort
 * 
 * abort a pending HTTP connection.  May only be called after an 
 * http_connection_connect_(submit)? before the HttpCallbackFn is called
 *
 * The HttpCallbackFn for this request will not be called
 */
 
void
http_connection_abort (HTTPConnection *connection)
{
	g_return_if_fail (IS_HTTP_CONNECTION (connection));

	socket_set_read_fn (connection->server_socket, NULL);
	socket_set_eof_fn (connection->server_socket, NULL);
	socket_set_data (connection->server_socket, NULL);
	socket_close (connection->server_socket);

	http_connection_free (connection);
}


/**
 * http_connection_read
 * 
 * async HTTP reader
 */

void
http_connection_read (
	Socket *sock,
	gpointer user_data,
	HttpReadCallbackFn callback
) {
	HttpReadStateInfo *state_info;

	g_assert (NULL != sock);

	state_info = g_new0 (HttpReadStateInfo, 1);
	state_info->state = HTTP_READ_STATUS;
	state_info->socket = sock;
	state_info->user_data = user_data;
	state_info->callback = callback;

	socket_set_read_fn (sock, http_read_sock_read);
	socket_set_eof_fn (sock, http_read_sock_eof);
	socket_set_data (sock, state_info);

	http_read_sock_read (sock);
}


/*******************************************************************
 * http -- Module Methods
 *******************************************************************/
static void
http_connection_sock_connect (Socket *sock, int success)
{
	char *request_line;
	int result;
	HTTPConnection *http_connection;
	GList *current_position;


	g_assert (IS_SOCKET (sock));

	http_connection = socket_get_data (sock);

	g_assert (NULL != http_connection);
	g_assert (IS_HTTP_CONNECTION (http_connection));

	if (!success) {
		http_connection_finish (http_connection, FALSE);
		return;
	}

	if (http_connection->proxy_connect ){
		HTTPRequest *req = http_connection->request;
		
		request_line=g_strdup_printf ("%s %s://%s:%d%s HTTP/%s\r\n",
			req->method,
			req->uri,
			req->host,
			req->port,
			req->path,
			req->version
		);
	} else {
		HTTPRequest *req = http_connection->request;
		
		request_line=g_strdup_printf ("%s %s HTTP/%s\r\n",
			req->method,
			req->path,
			req->version
		);
	}


	result = socket_write (sock, request_line, strlen (request_line));
	g_free (request_line);

	/*
	 * Host: header
	 */

	request_line = g_strdup_printf ("Host: %s:%u\r\n", 
				http_connection->request->host, 
				http_connection->request->port
		       );
	
	result = socket_write (sock, request_line, strlen (request_line));
	g_free (request_line);
	request_line = NULL;

	if ( 1 != result ) {
		http_connection_finish (http_connection, FALSE);
		return;
	}

	
	for ( 	current_position = http_connection->header_list ;
		current_position ; 
		current_position = g_list_next (current_position)
	) {
		result = socket_write (sock, (char *)current_position->data, strlen ((char *)current_position->data));
		result = socket_write (sock, "\r\n", 2);
	}
	result = socket_write (sock, "\r\n", 2);
	
	if ( 1 != result ) {
		http_connection_finish (http_connection, FALSE);
		return;
	}

	http_connection_finish (http_connection, TRUE);

}

static void
http_connection_connect_sock_eof (Socket *sock)
{
	HTTPConnection *http_connection;

	g_assert (IS_SOCKET (sock));
	
	http_connection = socket_get_data (sock);

	g_assert (NULL != http_connection);
	g_assert (IS_HTTP_CONNECTION (http_connection));

	g_return_if_fail ( NULL != http_connection );
	g_return_if_fail ( IS_HTTP_CONNECTION (http_connection) );

	http_connection_finish (http_connection, FALSE);
}

static void
http_connection_free (HTTPConnection *http_connection)
{
	GList * current_node;
	
	/* header_list */
	for (	current_node = http_connection->header_list; 
		NULL != current_node; 
		current_node = g_list_next (current_node)
	) { 
		g_free (current_node->data);
		current_node->data = NULL;
	}
	g_list_free (http_connection->header_list);

	request_free (http_connection->request);
	http_connection->magic = 0;
	g_free (http_connection);
}

static void
http_connection_finish (HTTPConnection *http_connection, gboolean success)
{
	HttpCallbackFn callback_fn;
	gpointer user_data;
	Socket * socket;

	g_assert (IS_HTTP_CONNECTION (http_connection));

	callback_fn = http_connection->callback;
	user_data = http_connection->user_data;
	socket = http_connection->server_socket;

	socket_set_read_fn (socket, NULL);
	socket_set_eof_fn (socket, NULL);
	socket_set_data (socket, NULL);

	if ( NULL == callback_fn || !success ) {
		socket_close (socket);
		socket = NULL;
	}

	http_connection_free (http_connection);

	if (callback_fn) {
		callback_fn (user_data, socket, success);
	}
}

static void /* HttpCallbackFn */
connect_submit_http_callback (gpointer user_data, Socket *sock, gboolean success)
{
	HTTPConnectSubmit *submit_state;
	int result;

	submit_state = (HTTPConnectSubmit *)user_data;

	if (!success) {
		submit_state->callback (submit_state->user_data, sock, FALSE);
		return;
	} else if (submit_state->submit_body) {
		result = socket_write (sock, submit_state->submit_body, submit_state->cb_submit_body);
		g_free (submit_state->submit_body);
		submit_state->submit_body = NULL;
	}

	submit_state->callback (submit_state->user_data, sock, success);

	g_free (submit_state->submit_body);
	g_free (submit_state);	
}

static void
http_read_state_info_free (HttpReadStateInfo *state_info)
{
	GList *list_node;

	socket_set_eof_fn (state_info->socket, NULL);
	socket_close (state_info->socket);
	
	g_free (state_info->status_line);

	http_get_headers_state_free (state_info->header_state);

	for (	list_node = g_list_first (state_info->header_list) ;
		NULL != list_node ;
		list_node = g_list_next (list_node)
	) {
		g_free ((char *)list_node->data);
	}

	for (	list_node = g_list_first (state_info->body_pieces_list) ;
		NULL != list_node ;
		list_node = g_list_next (list_node)
	) {
		g_free ((char *)list_node->data);
	}
}

static void /* SocketReadFn */
http_read_sock_read (Socket *sock)
{
	HttpReadStateInfo *state_info;
	gboolean done;
	char *line;

	state_info = socket_get_data (sock);
	g_assert (state_info);

	done = FALSE;
	while (!done) {
		switch (state_info->state) {

		case HTTP_READ_STATUS:
			line = socket_getline (sock);
			if (NULL == line) {
				done = TRUE;
			} else {
				state_info->status_line = line;
				state_info->state = HTTP_READ_HEADER;
			}
		break;

		case HTTP_READ_HEADER:
			line = http_get_headers (sock, &(state_info->header_state));

			if (NULL == line) {
				done = TRUE;
			} else if ('\0' == *line) {
				g_free (line);
				state_info->state = HTTP_READ_BODY;
			} else {
				state_info->header_list = g_list_prepend (state_info->header_list,  line);

			}
		break;

		case HTTP_READ_BODY: {
			size_t length;
			char *data;

			socket_read (sock, &data, &length);

			if (length > 0) {
				state_info->body_pieces_list 
					= piece_response_add (
						state_info->body_pieces_list, 
						data, 
						length
					);
			}

			done = TRUE;
		}
		break;

		default:
			g_assert(FALSE);
		}
	}
}
static void /* SocketEofFn */
http_read_sock_eof (Socket *sock)
{
	HttpReadStateInfo *state_info;
	size_t body_size;
	char *body;

	state_info = socket_get_data (sock);
	g_assert (state_info);

	body = piece_response_combine (state_info->body_pieces_list, &body_size);

	if (HTTP_READ_BODY == state_info->state) {
		state_info->callback (
			state_info->user_data, 
			state_info->status_line,
			state_info->header_list,
			body, 
			body_size, 
			TRUE
		);
	} else {
		state_info->callback (
			state_info->user_data, 
			state_info->status_line,
			state_info->header_list,
			body, 
			body_size, 
			FALSE
		);
	}

	state_info->status_line = NULL;		/*Callee should free that */

	http_read_state_info_free (state_info);
}


