/* $Id: session.h,v 1.1.1.1 2001-01-16 15:26:05 ghudson Exp $
 *
 * Copyright (C) 2000  Eazel, Inc
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
 */

#ifndef _SESSION_H_
#define _SESSION_H_

#include "sock.h"

/*
 * Types
 */

/* GET http://www.lag.net:80/index.html HTTP/1.0 */
typedef struct {
	char *method;	/* GET */
	char *uri;	/* http */
	char *host;	/* www.lag.net */
	int port;	/* 80 */
	char *path;	/* /index.html */
	char *version;	/* 1.0 */
} HTTPRequest;

typedef gpointer (*ProxyRequestCb) (gpointer user_data, unsigned short port, HTTPRequest *request,
				GList **p_header_list);
typedef void (*ProxyResponseCb) (gpointer user_data, gpointer connection_user_data, unsigned short port, 
				 char **p_status_line, GList **p_header_list);
typedef void (*ProxyCloseCb) (gpointer user_data, unsigned short port);

typedef void (*ProxyFreezeCb) (gpointer freeze_user_data);

typedef struct ProxyCallbackInfo {
	ProxyRequestCb request_cb;
	ProxyResponseCb response_cb;
	ProxyCloseCb close_cb;
} ProxyCallbackInfo;

typedef enum {
	Session_Normal,
	Session_CloseScheduled,
	Session_FreezeScheduled,
	Session_Frozen
} ProxySessionState;

typedef struct {
	guint32			magic;
	ProxyCallbackInfo 	callbacks;
	size_t 			open_count;
	unsigned short		port;
	gpointer		user_data;
	gboolean		close_scheduled;
	Socket *		socket;
	ProxySessionState	state;
	gpointer		freeze_user_data;
	ProxyFreezeCb		freeze_callback;
	char *			target_path;
} ProxySession; 

#define SESSION_MAGIC		(((guint32)'S')<<24 | ((guint32)'E')<<16 | ((guint32)'S')<<8 | ((guint32)'S'))
#define IS_SESSION(session)	((session) && (session->magic == SESSION_MAGIC))


/*
 * Functions
 */

/* HTTPQuery functions */
void request_free (HTTPRequest *req);
HTTPRequest * request_new (void);
HTTPRequest * request_copy (const HTTPRequest *req);
int request_parse (const char *line, HTTPRequest *req);
int request_parse_url (const char *line, HTTPRequest *req);

ProxySession *session_new (const ProxyCallbackInfo *callbacks, unsigned short port, gpointer user_data, Socket *sock);
void session_free (ProxySession *session);
void session_add_open (ProxySession *session);
void session_decrement_open (ProxySession *session);
ProxySession *session_from_port (unsigned short port);
void session_schedule_close (ProxySession *session);
void session_set_target_path (ProxySession *session, const char *target_path);
const char *session_get_target_path (ProxySession *session);
gboolean session_is_targeted (ProxySession *session);

void session_schedule_freeze (ProxySession *session, gpointer user_data, ProxyFreezeCb callback);
void session_thaw (ProxySession *session);

#endif /*_REQUEST_H_*/
