/* $Id: request.c,v 1.1.1.1 2001-01-16 15:26:36 ghudson Exp $
 *
 * Handles incoming request callbacks, buffers headers, and passes
 * the request on to the upstream server.
 *
 * Copyright (C) 2000 Eazel, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Authors: Robey Pointer <robey@eazel.com>
 *          Mike Fleming <mfleming@eazel.com>
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "request.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <glib.h>

#include "sock.h"
#include "proxy.h"
#include "log.h"
#include "utils.h"
#include "session.h"
#include "http-connection.h"

/* server socket's meta data -> client socket */
/* client socket's meta data -> Connection */
/* listen socket's meta data -> ProxySession */

/*********************************
 * State machine map
 * -> are async calls
 * 
 * proxy_new_connection
 * 	-> client socket callbacks (proxy_get_request, proxy_get_request_eof)
 * 
 * proxy_get_request
 * 	-> client socket callbacks (proxy_get_headers, )
 * 
 * proxy_get_headers
 * 	when complete:
 * 		proxy_do_connect
 * 
 * proxy_do_connect
 * 	-> client socket callbacks (NULL, proxy_abort_connect)
 * 	for normal connections:
 * 		http_connection_connect 
 * 			-> proxy_http_connect_callback
 * 	OR for 'CONNECT' method
 * 		proxy_handle_connect_method
 * 			socket_connect(_proxy_tunnel)?
 * 				-> proxy_method_connect_callback
 * 
 * proxy_method_connect_callback
 * 	-> client socket callbacks (NULL, proxy_client_eof)
 * 	-> server socket callbacks (NULL, proxy_server_eof)
 * 	socket_tunnel BOTH
 * 	
 * proxy_http_connect_callback
 * 	-> server socket callbacks (proxy_read_response_headers, proxy_server_eof)
 * 	-> client socket callbacks (NULL, proxy_client_eof)
 * 	socket_tunnel LEFT_TO_RIGHT
 * 
 * proxy_read_response_headers
 * 	call to proxy_reponse_headers_write
 * 	socket_tunnel BOTH
 **********************************/

/* for giving each proxy connection a unique id */
static int connection_id_counter = 0;

/* state info on a new connection */
typedef struct {
	guint32 	magic;
	int 		id;
        HTTPRequest *	request;

	HTTPConnection * http_connection;
        Socket *	server;
        gboolean	upstream_proxy;	/* TRUE if the http request has to be phrased for an upstream proxy */
	HttpGetHeaderState *p_header_state_request;
	GList *		header_list_request;	/* all header lines seen so far */

	HttpGetHeaderState *p_header_state_response;
	GList *		header_list_response;
	char *		response_line;

	gpointer	connection_user_data;
	ProxySession	*session;
} Connection;

#define CONNECTION_MAGIC	0xC0223C71
#define IS_CONNECTION(conn)	((conn) && (conn)->magic == CONNECTION_MAGIC)

static int
http_error_log(Socket *client_sock, int err, char *msg)
{
	Connection *conn;

	conn = (Connection *) socket_get_data (client_sock);
	g_assert (IS_CONNECTION (conn));
	
	log ("%d returning error '%d %s'", conn->id,  400, "Failed to connect to remote host.");
	return http_error (client_sock, 400, "Failed to connect to remote host.");
}

/*******************************************************************
 * proxy_tunnel
 * a "sock_connet" wrapper for connecting through upstream
 * HTTP servers via the CONNECT method
 *******************************************************************/
typedef struct {
	void * 		orig_user_data;
	char *		host_real;
	int		port_real;
	gboolean 	setup_ssl;
	SocketConnectFn	connectfn;
} ProxyTunnelState;

static void proxy_tunnel_sock_connect (Socket *sock, int success);
static void proxy_tunnel_finish (ProxyTunnelState *state, Socket *sock, int success);
static void proxy_tunnel_state_free (ProxyTunnelState * state);
static void proxy_tunnel_sock_eof (Socket *sock);
static void proxy_tunnel_sock_read (Socket *sock);
static void proxy_tunnel_sock_read_headers (Socket *sock);
static void proxy_new_connection(Socket *sock, gpointer user_data, const char *host, int port);

/**
 *
 * socket_connect_proxy_tunnel
 * 
 * Establish a connection through an upstream HTTP proxy via an HTTP "CONNECT"
 * method.
 *
 * Uses default upstream proxy
 * 
 * Sets read and EOF functions to NULL when finally done
 *
 */
 
int
socket_connect_proxy_tunnel(
	Socket *sock, 
	const char *name, 
	int port,
	gboolean setup_ssl,
	SocketConnectFn connectfn
) {
	ProxyTunnelState *state;
	int ret;

	g_return_val_if_fail (IS_SOCKET (sock), 0);

	state = g_new0 (ProxyTunnelState, 1);
	state -> orig_user_data = socket_get_data (sock);
	state -> port_real = port;
	state -> host_real = g_strdup (name);
	state -> connectfn = connectfn;
	state -> setup_ssl = setup_ssl;

	socket_set_data (sock, state);
	socket_set_read_fn (sock, NULL);
	socket_set_eof_fn (sock, NULL);

	ret = socket_connect (
		sock, 
		config.upstream_host, 
		config.upstream_port,
		proxy_tunnel_sock_connect
	);

	if (1 != ret) {
		g_free (state);
	}

	return ret;
}

static void
proxy_tunnel_finish (ProxyTunnelState *state, Socket *sock, int success)
{
	SocketConnectFn callback;
	
	socket_set_data (sock, state->orig_user_data);
	socket_set_read_fn (sock, NULL);
	socket_set_eof_fn (sock, NULL);

	callback = state->connectfn;
	proxy_tunnel_state_free (state);

	if (callback) {
		(*callback)(sock, success);
	}
}

static void
proxy_tunnel_state_free (ProxyTunnelState * state)
{
	g_free (state->host_real);
	g_free (state);
}

static void
proxy_tunnel_sock_connect (Socket *sock, int success)
{
	ProxyTunnelState *state;
	
	g_assert (IS_SOCKET (sock));

	state = (ProxyTunnelState *) socket_get_data (sock);
	
	if (!success) {
		proxy_tunnel_finish (state, sock, success);
	} else {
		char *request;

		request = g_strdup_printf ("CONNECT %s:%d HTTP/1.0\r\n\r\n", state->host_real, state->port_real);
		socket_write (sock, request, strlen(request));
		socket_set_read_fn (sock, proxy_tunnel_sock_read);
		socket_set_eof_fn (sock, proxy_tunnel_sock_eof);

	}

}

static void
proxy_tunnel_sock_eof (Socket *sock)
{
	ProxyTunnelState *state;
	
	g_assert (IS_SOCKET (sock));

	state = (ProxyTunnelState *) socket_get_data (sock);

	g_assert (NULL != state);
	
	proxy_tunnel_finish (state, sock, 0);
}


static void
proxy_tunnel_sock_read (Socket *sock)
{
	ProxyTunnelState *state;
	char *line;
	HttpStatusLine http_status_line;

	g_assert (IS_SOCKET (sock));

	state = (ProxyTunnelState *)socket_get_data (sock);

	g_assert (NULL != state);

	line = socket_getline (sock);

	if (! line ) {
		return;
	}

	if ( http_parse_status_line (line, &http_status_line) && (200 == http_status_line.code) ) {
		g_free (line);

		socket_set_read_fn (sock, proxy_tunnel_sock_read_headers);
		proxy_tunnel_sock_read_headers (sock);
	} else {
		g_free (line);

		proxy_tunnel_finish (state, sock, 0);
	}
	
}

static void
proxy_tunnel_sock_read_headers (Socket *sock)
{
	ProxyTunnelState *state;
	char *line;
	int success;

	g_assert (IS_SOCKET (sock));
	
	state = (ProxyTunnelState *)socket_get_data (sock);

	g_assert (NULL != state);


	/* Gobble any headers returned from server as a result of the CONNECT method */
	while ((line = socket_getline (sock)) && *line) {
		g_free (line);
	}

	if (NULL == line) {
		return;
	}

#ifdef HAVE_OPENSSL
	if (state->setup_ssl) {
		success = socket_begin_ssl (sock);
	} else {
		success = 1;
	}
#else /* HAVE_OPENSSL */
	g_assert ( !state->setup_ssl );
	success = 1;
#endif /*  HAVE_OPENSSL */
	
	proxy_tunnel_finish (state, sock, success);

}


static void 
connection_free(Connection *conn)
{
	GList *current_node;

	g_return_if_fail (NULL != conn);
	g_assert (IS_CONNECTION (conn));

        request_free (conn->request);

	http_get_headers_state_free (conn->p_header_state_request);

	/* header_list_request */
	for (	current_node = conn->header_list_request; 
		NULL != current_node; 
		current_node = g_list_next (current_node)
	) { 
		g_free (current_node->data);
		current_node->data = NULL;
	}
	g_list_free (conn->header_list_request);

	http_get_headers_state_free (conn->p_header_state_response);

	/* header_list_response */
	for (	current_node = conn->header_list_response; 
		NULL != current_node; 
		current_node = g_list_next (current_node)
	) { 
		g_free (current_node->data);
		current_node->data = NULL;
	}
	g_list_free (conn->header_list_response);

	g_free (conn->response_line);

	session_decrement_open (conn->session);

	conn->magic = 0;
	g_free (conn);
}

static Connection *
connection_new(void)
{
        Connection *conn;

        conn = g_new0(Connection, 1);
	conn->magic = CONNECTION_MAGIC;
	conn->id = ++connection_id_counter;
        return conn;
}


/**
 * proxy_client_eof
 * 
 * proxy and server are both connected; EOF received from client
 */

static void  /* SocketEofFn */
proxy_client_eof (Socket *client_sock)
{
	Connection *conn = (Connection *) socket_get_data (client_sock);

	g_assert (IS_SOCKET (client_sock));
	g_assert (IS_CONNECTION (conn));

	/* Note that we're not closing the server socket--socket_tunnel does that*/

	log ("%d finished", conn->id);

	/* Set server socket's data to NULL (it points to us) */
	if (conn->server) {
		socket_set_data (conn->server, NULL);
	}

        /* free Connection struct, socket is about to die */
        connection_free (conn);
	socket_set_data (client_sock, NULL);
}

/**
 * proxy_server_eof
 * 
 * proxy and server are both connected; EOF received from server
 */

static void /* SocketEofFn */
proxy_server_eof(Socket *server_sock)
{
	guint32 xferd, recvd;
	Socket *client_sock;

	client_sock = (Socket *)socket_get_data (server_sock);

	g_assert (NULL == client_sock || IS_SOCKET (client_sock));

	/* client socket could've already been closed */
	/* but if it wasn't, remove ourself from the Connection struct */
	if (client_sock) {
		Connection *connection;
		connection = (Connection *)socket_get_data (client_sock);

		g_assert ( IS_CONNECTION(connection) );

		connection->server = NULL;
	}

	/* Note that we're not closing the server socket--socket_tunnel does that*/

	socket_get_byte_counts (server_sock, &xferd, &recvd);
	stats.num_tx += xferd;
	stats.num_rx += recvd;

        socket_set_data (server_sock, NULL);
}


static void
proxy_response_headers_write (Socket *socket_client, char *response_line, GList *header_list)
{
	int err;
	GList *current_position;
	
	err = socket_write (socket_client, response_line, strlen (response_line));
	err = socket_write (socket_client, "\r\n", 2);

	for ( 	current_position = header_list ;
		current_position ; 
		current_position = g_list_next (current_position)
	) {
		err = socket_write (socket_client, (char *)current_position->data, strlen ((char *)current_position->data));
		err = socket_write (socket_client, "\r\n", 2);
	}
	err = socket_write (socket_client, "\r\n", 2);
}


/**
 * proxy_read_response_headers
 * 
 * Request has been sent; parsing response headers
 *
 */
 
static void /* SocketReadFn */
proxy_read_response_headers (Socket *server_sock)
{
	Socket *client_sock;
	Connection *conn;
	gboolean done;
	char *line;

	client_sock = (Socket *) socket_get_data (server_sock);

	if ( NULL == client_sock ) {
		return;
	}

	g_assert (IS_SOCKET (client_sock));

	conn = (Connection *) socket_get_data (client_sock);

	g_assert (IS_CONNECTION (conn));

	/* First, read the response line*/
	if (NULL == conn->response_line) {
		conn->response_line = socket_getline (server_sock);

		if (NULL == conn->response_line) {
			return;
		}
	}

	/* Second, read the response headers*/
	done = FALSE;

	while (!done) {
		line = http_get_headers (server_sock, &(conn->p_header_state_response));

		if (NULL == line) {
			/*buffer's empty; still more headers*/
			done = TRUE;
		} else if ('\0' == *line) {
			/* headers have finished */
			done = TRUE;
			g_free (line);
			line = NULL;

			/* Freeze the sockets so the callback may cause event loop iterations */
			socket_freeze (server_sock);
			socket_freeze (client_sock);
			if (conn->session->callbacks.response_cb) {
				conn->session->callbacks.response_cb (
					conn->session->user_data,
					conn->connection_user_data,
					conn->session->port,  
					&(conn->response_line), 
					&(conn->header_list_response)
				);
			}
			socket_thaw (client_sock);
			socket_thaw (server_sock);

			log ("%d Response was '%s'", conn->id, conn->response_line);

			proxy_response_headers_write (client_sock, conn->response_line, conn->header_list_response);

			socket_set_read_fn (server_sock, NULL);
			socket_tunnel (server_sock, client_sock, SOCKET_TUNNEL_BOTH);
		} else {
			conn->header_list_response = g_list_append (conn->header_list_response, line); 
			line = NULL;
		}
	}
}
        
/* client socket has disconnected before server socket finished connecting */
static void /* SocketEofFn */
proxy_abort_connect(Socket *sock)
{
	Connection *conn = (Connection *) socket_get_data (sock);

	g_assert (IS_CONNECTION (conn));

	log ("%d client aborted", conn->id);

	if (NULL != conn->http_connection) {
		http_connection_abort (conn->http_connection);
	}

	/* This should only be created by CONNECT method requests */
	if (conn->server) {
		g_assert (IS_SOCKET (conn->server));
		socket_set_eof_fn (conn->server, NULL);
		socket_close (conn->server);
		conn->server = NULL;
	}

	connection_free (conn);
	socket_set_data (sock, NULL);
}


/**
 * proxy_method_connect_callback
 * socket connect callback for HTTP CONNECT method
 */
static void /* SocketConnectFn */
proxy_method_connect_callback(Socket *server_sock, int success)
{
	char *out;
	Socket *client_sock;
	Connection *conn;

	client_sock = (Socket *)socket_get_data (server_sock);
	g_assert (IS_SOCKET (client_sock));
	conn = (Connection *) socket_get_data (client_sock);
	g_assert (IS_CONNECTION (conn));

	if ( ! success ) {
	        log ("%d error connecting via 'CONNECT'", conn->id);
		out = "HTTP/1.0 400 Connect Failed\r\n\r\n";
		socket_write (client_sock, out, strlen (out));

		socket_set_eof_fn (client_sock, NULL);
		/* closes server_sock too */
		proxy_abort_connect (client_sock);
		socket_close (client_sock);
	} else {
		out = "HTTP/1.0 200 OK\r\n\r\n";
		socket_write (client_sock, out, strlen (out));
	        log ("%d connected via 'CONNECT': tunneling", conn->id);
	        socket_set_eof_fn (server_sock, proxy_server_eof);
		socket_set_read_fn (client_sock, NULL);
		socket_set_eof_fn (client_sock, proxy_client_eof);
		socket_tunnel (client_sock, server_sock, SOCKET_TUNNEL_BOTH);
	}
}

/**
 * proxy_http_connect_callback
 * http_connection callback after connection triggered in
 * proxy_do_connect
 */
static void /* HttpCallbackFn */
proxy_http_connect_callback (gpointer user_data, Socket *server_sock, gboolean success)
{
	Connection *conn;
	Socket *client_sock;

	client_sock = (Socket *) user_data;
	g_assert (IS_SOCKET (client_sock));
	conn = (Connection *) socket_get_data (client_sock);
	g_assert (IS_CONNECTION (conn));

	conn->http_connection = NULL;

	if ( ! success) {
                log ("%d connect failed", conn->id);
                conn->server = NULL;
		http_error_log (client_sock, 400, "Can't connect to remote website.");

		socket_set_eof_fn (client_sock, NULL);
		proxy_abort_connect (client_sock);
                socket_close (client_sock);
                return;
	}		

	socket_set_data (server_sock, client_sock);
	conn->server = server_sock;

        log ("%d connected: reading response", conn->id);
        socket_set_eof_fn (server_sock, proxy_server_eof);
	socket_set_read_fn (server_sock, proxy_read_response_headers);
        socket_set_read_fn (client_sock, NULL );
	socket_set_eof_fn (client_sock, proxy_client_eof);

	socket_tunnel (client_sock, server_sock, SOCKET_TUNNEL_LEFT_TO_RIGHT);

	proxy_read_response_headers (server_sock);

}

/**
 * proxy_handle_connect_method
 * Handle the case where a client has specified a CONNECT method
 */
static void
proxy_handle_connect_method (Connection *conn, Socket *client_sock)
{
	int err;

	conn->server = socket_new (NULL, 0);

	if (! conn->server) {
		http_error_log (client_sock, 500, "Internal socket error.");
		proxy_abort_connect (client_sock);
		socket_set_eof_fn (client_sock, NULL);
		socket_close (client_sock);
		return;
	}

#ifndef NO_DEBUG_MIRRORING
	if (config.mirror_log_dir) {
		char *in_mirror_filename, *out_mirror_filename;
		FILE *in_mirror, *out_mirror;

		in_mirror_filename = g_strdup_printf ("%s/ep-%d-%d-fm-server.log", config.mirror_log_dir, getpid(), conn->id);
		out_mirror_filename = g_strdup_printf ("%s/ep-%d-%d-to-server.log", config.mirror_log_dir, getpid(), conn->id);
		in_mirror = fopen (in_mirror_filename, "w");
		out_mirror = fopen (out_mirror_filename, "w");
		socket_set_mirrors (conn->server, out_mirror, in_mirror);
		g_free (in_mirror_filename);
		g_free (out_mirror_filename);
	}
#endif

	socket_set_data (conn->server, client_sock);
	
	if (config.upstream_host) {
		err = socket_connect_proxy_tunnel (
			conn->server, 
			conn->request->host, 
			conn->request->port,
			FALSE,
			proxy_method_connect_callback
		);
	} else {
		err = socket_connect (
			conn->server,
			conn->request->host, 
			conn->request->port,
			proxy_method_connect_callback
		);
	}
	
	if ( 0 == err ) {
		http_error_log (client_sock, 400, "Failed to connect to remote host.");

		socket_set_eof_fn (client_sock, NULL);
		proxy_abort_connect (client_sock);
		socket_close (client_sock);
	}
}

static void
proxy_do_connect(Socket *client_sock)
{
	Connection *conn = (Connection *) socket_get_data (client_sock);
#ifndef NO_DEBUG_MIRRORING
	FILE *in_mirror = NULL, *out_mirror = NULL;
#endif

	g_assert (IS_CONNECTION (conn));

	if ( ! (session_is_targeted (conn->session) || conn->request->host ) ) {
		http_error_log (client_sock, 400, "No remote host name supplied.");
		socket_set_eof_fn (client_sock, NULL);
		proxy_abort_connect (client_sock);
		socket_close (client_sock);
		return;
	}

	/* Are we redirecting all requests to a single host? */
	if (session_is_targeted (conn->session)) {
		/* override parts of the request with the target path */
		if ( ! request_parse_url (session_get_target_path (conn->session), conn->request) ) {
			http_error_log (client_sock, 400, "Proxy config error: invalid target hostname url.");
			socket_set_eof_fn (client_sock, NULL);
			proxy_abort_connect (client_sock);
			socket_close (client_sock);
			return;
		}
	}

	/* Add "Connection: close" (proxy_get_headers has removed any Connection headers( */
	conn->header_list_request = g_list_prepend (conn->header_list_request, g_strdup ("Connection: close"));

	/* Force the HTTP version to 1.0 */
	u_replace_string (&(conn->request->version), g_strdup("1.0")); 

	/* Make Session callback
	 * Note that the callback is allowed to modify headers and the HTTPRequest
	 * The callback is also allowed to cause the event loop to iterate;
	 * any socket EOF events will be queued until the sockets are thawed
	 */

	socket_freeze (client_sock);
	if (conn->session->callbacks.request_cb) {
		conn->connection_user_data =	conn->session->callbacks.request_cb (
							conn->session->user_data, 
							conn->session->port, 
							conn->request, 
							&(conn->header_list_request)
						);
	}
	socket_thaw (client_sock);

	/* server socket's ancillary data points to the client socket */
	socket_set_read_fn (client_sock, NULL);
	socket_set_eof_fn (client_sock, proxy_abort_connect);

	/* If we're dealing with a CONNECT method, make the appropriate
	 * socket-level connection
	 */
	if (g_strcasecmp (conn->request->method, "CONNECT") == 0) {
		proxy_handle_connect_method (conn, client_sock);
	} else {
#ifndef NO_DEBUG_MIRRORING
		if (config.mirror_log_dir) {
			char *in_mirror_filename, *out_mirror_filename;

			in_mirror_filename = g_strdup_printf ("%s/ep-%d-%d-fm-server.log", config.mirror_log_dir, getpid(), conn->id);
			out_mirror_filename = g_strdup_printf ("%s/ep-%d-%d-to-server.log", config.mirror_log_dir, getpid(), conn->id);
			in_mirror = fopen (in_mirror_filename, "w");
			out_mirror = fopen (out_mirror_filename, "w");
			g_free (in_mirror_filename);
			g_free (out_mirror_filename);
		}
#endif

		conn->http_connection 
			= http_connection_connect (
				conn->request, 
				conn->header_list_request, 
				(gpointer) client_sock,
				proxy_http_connect_callback
#ifndef NO_DEBUG_MIRRORING
				,out_mirror, in_mirror
#endif
			);
		/* this gets gobbled by http_connection_connect */
		conn->header_list_request = NULL;

		if ( NULL == conn->http_connection ) {
			http_error_log (client_sock, 400, "Failed to connect to remote host.");
			socket_set_eof_fn (client_sock, NULL);
			proxy_abort_connect (client_sock);
			socket_close (client_sock);
		}
	}

	return;
}

static void /* SocketReadFn */
proxy_get_headers(Socket *sock)
{
	Connection *conn = (Connection *) socket_get_data (sock);
	char *line;
	gboolean done;

	g_assert (IS_CONNECTION (conn));

	done = FALSE;

	while (!done) {
		line = http_get_headers (sock, &(conn->p_header_state_request));

		if (NULL == line) {
			/*buffers empty; still more headers*/
			done = TRUE;
		} else if ('\0' == *line) {
			/*headers have finished*/
			done = TRUE;
			g_free (line);
			line = NULL;			
			proxy_do_connect (sock);
		} else {
			if (STRING_STARTS_WITH (line, "Host:")) {
				/* remove any "Host:" lines */
				g_free (line);
				line = NULL;
				continue;
			}

			if (STRING_STARTS_WITH (line, "Connection:")) {
				/* remove any "Connection:" lines
				 * so that HTTP/1.1 clients/servers don't try
				 * to use persistant connections
				 */
				g_free (line);
				line = NULL;
				continue;
			}

			conn->header_list_request = g_list_append (conn->header_list_request, line); 
			line = NULL;
		}
	}
}

/* read callback to get the request */
static void /*SocketReadFn*/
proxy_get_request (Socket *client_sock)
{
	Connection *conn = (Connection *) socket_get_data (client_sock);
	char *line;

	g_assert (IS_CONNECTION (conn));

	line = socket_getline (client_sock);

	if (! line) {
		/* not finished typing yet */
		goto out;
	}
        conn->request = request_new ();
        if (! request_parse (line, conn->request)) {
		http_error_log (client_sock, 400, "Invalid request format.");
		log ("%d invalid request format", conn->id);
		g_free (line);
		goto close_out;
	}
	g_free (line);

	if ((g_strcasecmp (conn->request->method, "CONNECT") == 0) &&
	    (session_is_targeted (conn->session))) {
		/* disallow CONNECT if we're forcing all outbound to a remote host */
		http_error_log (client_sock, 400, "The CONNECT method is not allowed.");
		log ("%d attempted CONNECT; denied.", conn->id);
		goto close_out;
	}

	if (conn->request->uri) {
		/* handle a few "special" uri types */
		if (strcasecmp (conn->request->uri, "proxy-die") == 0) {
			http_error_log (client_sock, 500, "The proxy server is shutting down.");
			log ("%d received proxy-die request; dying...", conn->id);
			kill (getpid (), SIGTERM);
			goto close_out;
		}
		if (strcasecmp (conn->request->uri, "proxy-stats") == 0) {
			show_stats (client_sock);
			goto close_out;
		}

		if (strcasecmp (conn->request->uri, "http") != 0) {
			http_error_log (client_sock, 500, "This proxy only handles HTTP URI's.");
			log ("%d tried a non-HTTP URI", conn->id);
			goto close_out;
		}
		if (! conn->request->port) {
			conn->request->port = 80;
		}

		log ("%d request: %s http://%s:%d%s", conn->id, conn->request->method,
		     conn->request->host, conn->request->port, conn->request->path);
	} else {
		if (conn->request->path) {
			log ("%d request: %s %s", conn->id, conn->request->method,
			     conn->request->path);
		} else {
			log ("%d request: %s %s:%d", conn->id, conn->request->method,
			     conn->request->host, conn->request->port);
		}
	}

	/* get headers */
	socket_set_read_fn (client_sock, proxy_get_headers);
	proxy_get_headers (client_sock);

out:
	return;

close_out:
	socket_set_eof_fn (client_sock, NULL);
	proxy_abort_connect (client_sock);
	socket_close (client_sock);
	return;
}

static /* SocketEofFn */
void proxy_get_request_eof(Socket *client_sock)
{
	Connection *conn = (Connection *)socket_get_data(client_sock);

	g_assert (IS_CONNECTION (conn));

	if (conn) {
		/* free Connection struct, socket is about to die */
		log("%d lost connection", conn->id);
                if (conn->server) {
                	socket_set_eof_fn (conn->server, NULL);
			socket_close(conn->server);
                }
                connection_free(conn);
		socket_set_data(client_sock, NULL);
	}
}

/**
 * proxy_listen
 * 
 * Set up HTTP proxy to listen on specified port.  if "port" is 0, 
 * an ephemeral port is selected
 *
 * The callback list is optional
 * 
 * A return value of "0" indicated failure.  Otherwise, the port being
 * listened on is returned
 */

unsigned short
proxy_listen (unsigned short port, gpointer user_data, const ProxyCallbackInfo * callbacks, const char *target_path)
{
	Socket *listen_socket = NULL;
	ProxySession *session;

	if ( 0 == port ) {
		for (port = PORT_START; port < PORT_END; port++) {
			listen_socket = socket_new("127.0.0.1", port);
			if (listen_socket) {
				/* success, bail */
				break;
			}
		}
		if (port == PORT_END) {
			log("unable to find any listening port in the range [%d,%d]", PORT_START, PORT_END);
			port = 0;
			goto error;
		}
	} else {
		listen_socket = socket_new("127.0.0.1", port);
		if (NULL == listen_socket) {
			log ("Couldn't listen on port %d", port);
			port = 0;
			goto error;
		}
	}

	session = session_new (callbacks, port, user_data, listen_socket);
	if (target_path && target_path[0]) {
		session_set_target_path (session, target_path);
	}

	socket_set_data (listen_socket, session);
	socket_listen (listen_socket, proxy_new_connection);

error:
	return port;
}

/**
 * proxy_listen_close
 * 
 * Close a listen port
 * 
 * Note that the listen port is closed immmediately, but there still may be
 * outstanding requests that originated from this port
 * 
 * A client will continue to receive callbacks until a close_cb is called.
 */

void
proxy_listen_close (unsigned short port)
{
	ProxySession *session;

	session = session_from_port (port);

	if (NULL == session) {
		g_warning ("asked to close session on invalid port %d\n", port);
		return;
	}

	session_schedule_close (session);
}


static void
proxy_new_connection(Socket *client_sock, gpointer user_data, const char *host, int port)
{
	Connection *conn;
	ProxySession *session;

	session = (ProxySession *)user_data;
	g_assert ( IS_SESSION (session) );

	if (! eazel_check_connection(client_sock->fd)) {
		log("attempted connection from an invalid UID");
		socket_set_eof_fn (client_sock, NULL);
		socket_close (client_sock);
		return;
	}

	conn = connection_new();

	log("%d new connection: %s/%d", conn->id, host, port);

	conn->session = session;
	session_add_open (session);

#ifndef NO_DEBUG_MIRRORING
	if (config.mirror_log_dir) {
		FILE *out_mirror;
		FILE *in_mirror;
		char *in_mirror_filename;
		char *out_mirror_filename;

		in_mirror_filename = g_strdup_printf ("%s/ep-%d-%d-fm-client.log", config.mirror_log_dir, getpid(), conn->id);
		out_mirror_filename = g_strdup_printf ("%s/ep-%d-%d-to-client.log", config.mirror_log_dir, getpid(), conn->id);
		in_mirror = fopen (in_mirror_filename, "w");
		out_mirror = fopen (out_mirror_filename, "w");
		socket_set_mirrors (client_sock, out_mirror, in_mirror);
		g_free (in_mirror_filename);
		g_free (out_mirror_filename);
	}
#endif

	socket_set_data(client_sock, conn);
	socket_set_read_fn(client_sock, proxy_get_request);
	socket_set_eof_fn(client_sock, proxy_get_request_eof);
}

