/* $Id: session.c,v 1.1.1.1 2001-01-16 15:26:12 ghudson Exp $
 *
 * Info about a particular listening port and the session (usually auth
 * info and other state info like callbacks) associated with that port.
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
 * Authors: Mike Fleming <mfleming@eazel.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "sock.h"
#include "log.h"
#include "proxy.h"
#include "session.h"

#undef DEBUG_REQUEST

/* state info for a listened port */
static GList *gl_proxy_port_sessions = NULL;


/**********   HTTP request stuff   **********/

void
request_free (HTTPRequest *req)
{
        if (! req) {
                return;
        }

	g_free(req->method);
	g_free(req->uri);
	g_free(req->host);
	g_free(req->path);
	g_free(req->version);

	req->method = req->uri = req->host = req->path = req->version = NULL;
        g_free(req);
}

HTTPRequest *
request_new (void)
{
        HTTPRequest *req;

        req = g_new0(HTTPRequest, 1);
        return req;
}

HTTPRequest *
request_copy (const HTTPRequest *req)
{
	HTTPRequest *ret = NULL;

	if( req ) {
		ret = g_new0 (HTTPRequest,1);

		ret->method 	= g_strdup (req->method);
		ret->uri 	= g_strdup (req->uri);
		ret->host 	= g_strdup (req->host);
		ret->path 	= g_strdup (req->path);
		ret->version 	= g_strdup (req->version);
		ret->port	= req->port;
	}	

	return ret;
}

/* Warning: this code is evil */
static int
request_parse_internal (const char *line, HTTPRequest *req, int url_only)
{
	const char *p, *start;
	char *temp_path = NULL;
	gboolean is_connect_method = FALSE;;

	if (req->port == 0) {
		req->port = 80;
	}

	start = line;
	p = line;

	if (! url_only) {
		/* pull off method */
		for (p = start; (*p) && (*p != ' '); p++);
		if (! *p) {
			return 0;
		}
		req->method = g_strndup (start, p - start);
		start = p+1;

		/* URLs sent for the CONNECT method have no URL scheme */
		is_connect_method = (0 == g_strcasecmp (req->method, "CONNECT" ));
	}

	while (*start == ' ') {
		start++;
	}

	if (! is_connect_method) {
		/* uri ("http")  [may be absent] */
		for (p = start; (*p) && (*p != ':') && (*p != ' '); p++);
		if ((*p == ':') && (*(p+1) == '/') && (*(p+2) == '/')) {
			/* there is a uri */
			req->uri = g_strndup (start, p - start);
			start = p + 3;
		} else {
			p = start;
		}
	}
	
	/* host (only if uri present or if method is CONNECT) */
	if (req->uri || is_connect_method ) {
		for (p = start; (*p) && (*p != ':') && (*p != '/') && (*p != ' '); p++);
		req->host = g_strndup (start, p - start);

		/* port? */
		if (*p == ':') {
			start = p + 1;
			for (p = start; (*p) && (*p != '/') && (*p != ' '); p++);
			if ((p - start - 1) > 0) {
				req->port = atoi (start);
			} else {
				req->port = 80;
			}
		}
	}

	/* path? */
	if (*p == '/') {
		start = p;
		for (p = start; (*p) && (*p != ' '); p++);
		temp_path = g_strndup (start, p - start);
		if (req->path) {
			/* prepend to existing path */
			req->path = g_strdup_printf ("%s%s%s", temp_path, (*(p-1) == '/' ? "" : "/"),
						     (req->path[0] == '/' ? (req->path + 1) : req->path));
			g_free (temp_path);
			temp_path = NULL;
		} else {
			req->path = temp_path;
			temp_path = NULL;
		}
	}

	if ( NULL == req->path) {
		req->path = g_strdup ("");
	}

	/* http version */
	if (! url_only) {
		start = (*p) ? (p + 1) : (p);
		while (*start == ' ') {
			start++;
		}
		if (! *start) {
			/* assume it's just 0.9 */
			req->version = g_strndup ("0.9", 3);
			return 1;
		}
		if (g_strncasecmp (start, "HTTP/", 5) != 0) {
			return 0;
		}
		start += 5;
		for (p = start; (*p) && (*p != ' '); p++);
		req->version = g_strndup (start, p - start);
	}

#ifdef DEBUG_REQUEST
	log ("method : %s", req->method ? req->method : "(none)");
	log ("uri    : %s", req->uri ? req->uri : "(none)");
	log ("host   : %s", req->host ? req->host : "(none)");
	log ("port   : %d", req->port);
	log ("path   : %s", req->path ? req->path : "(none)");
	log ("version: %s", req->version ? req->version : "(none)");
#endif

	return 1;
}

/* parse "http://tortoise.eazel.com:8888/help.cgi" into parts
 * returns 1 on success, 0 if not parsed
 */
int
request_parse_url (const char *line, HTTPRequest *req)
{
	return request_parse_internal (line, req, 1);
}

/* parse "GET http://tortoise.eazel.com:8888/help.cgi HTTP/1.0" into parts
 * returns 1 on success, 0 if not parsed
 */
int
request_parse (const char *line, HTTPRequest *req)
{
	req->method = req->version = req->path = NULL;
	req->uri = req->host = NULL;
	req->port = 0;
	return request_parse_internal (line, req, 0);
}


/**********   proxy sessions   **********/

ProxySession *
session_new (const ProxyCallbackInfo *callbacks, unsigned short port, gpointer user_data, Socket *sock)
{
	ProxySession *session;

	session = g_new0 (ProxySession, 1);
	session->magic 			= SESSION_MAGIC;
	if (callbacks) {
		session->callbacks = *callbacks;
	}
	session->open_count 		= 0;
	session->port 			= port;
	session->user_data 		= user_data;
	session->socket			= sock;
	session->state			= Session_Normal;
	if (config.target_path) {
		session->target_path = g_strdup (config.target_path);
	} else {
		session->target_path = NULL;
	}

	gl_proxy_port_sessions = g_list_append (gl_proxy_port_sessions, session);

	return session;
}

void
session_free (ProxySession *session)
{
	g_return_if_fail (NULL != session);
	g_return_if_fail (IS_SESSION (session));

	if (session->socket) {
		socket_close (session->socket);
	}
	
	gl_proxy_port_sessions = g_list_remove (gl_proxy_port_sessions, session);
	g_free (session->target_path);
	g_free (session);
}

void
session_add_open (ProxySession *session)
{
	g_return_if_fail (NULL != session);
	g_return_if_fail (IS_SESSION (session));

	session->open_count++;
}

static void
session_do_close (ProxySession *session)
{
	g_return_if_fail (NULL != session);
	g_return_if_fail (IS_SESSION (session));

	if (session->callbacks.close_cb) {
		session->callbacks.close_cb (session->user_data, session->port);
	}
	session_free (session);
}

void
session_decrement_open (ProxySession *session)
{
	g_return_if_fail (NULL != session);
	g_return_if_fail (IS_SESSION (session));

	session->open_count--;
	g_assert (session->open_count >= 0);

	if (0 == session->open_count) {
		if (Session_CloseScheduled == session->state) {
			session_do_close (session);
		} else if  (Session_FreezeScheduled == session->state) {
			session->state = Session_Frozen;
			session->freeze_callback (session->freeze_user_data);
		}
	}
}

ProxySession *
session_from_port (unsigned short port)
{
	GList * current_position;
	ProxySession * session = NULL;
	
	for (current_position = gl_proxy_port_sessions ;
		current_position ;
		current_position = g_list_next (current_position)
	) {
		session = (ProxySession *) current_position->data;
		g_assert (IS_SESSION (session));

		if (port == session->port) {
			break;
		}
	}

	return (NULL != current_position) ? session : NULL; 
}

void
session_schedule_close (ProxySession *session)
{
	g_return_if_fail (NULL != session);
	g_return_if_fail (IS_SESSION (session));

	if (session->socket) {
		socket_close (session->socket);
		session->socket = NULL;
	}

	if (session->open_count > 0) {
		session->state = Session_CloseScheduled;
	} else {
		session_do_close (session);
	}
}

void
session_set_target_path (ProxySession *session, const char *target_path)
{
	g_return_if_fail (NULL != session);
	g_return_if_fail (IS_SESSION (session));

	g_free (session->target_path);
	session->target_path = g_strdup (target_path);
}

const char *
session_get_target_path (ProxySession *session)
{
	g_return_val_if_fail (NULL != session, NULL);
	g_return_val_if_fail (IS_SESSION (session), NULL);

	return (const char *) session->target_path;
}

gboolean
session_is_targeted (ProxySession *session)
{
	g_return_val_if_fail (NULL != session, FALSE);
	g_return_val_if_fail (IS_SESSION (session), FALSE);

	return (session->target_path && session->target_path[0]);
}

void
session_schedule_freeze (ProxySession *session, gpointer user_data, ProxyFreezeCb callback)
{
	g_assert ( NULL != session);
	g_assert (Session_Normal == session->state);
	g_assert ( NULL != callback);

	session->freeze_user_data = user_data;
	session->freeze_callback = callback;

	socket_freeze (session->socket);

	if (session->open_count > 0) {
		session->state = Session_FreezeScheduled;
	} else {
		session->state = Session_Frozen;
		callback (session->freeze_user_data);
	}	
}

void
session_thaw (ProxySession *session)
{
	g_assert (NULL != session);
	g_assert (Session_Frozen == session->state);

	socket_thaw (session->socket);
	session->state = Session_Normal;
}
