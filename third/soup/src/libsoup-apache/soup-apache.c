/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-apache.c: Asyncronous Callback-based SOAP Request Queue.
 *
 * Authors:
 *      Alex Graveley (alex@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_HTTPD_H

#include <httpd.h>
#include <http_config.h>
#include <http_core.h>
#include <http_log.h>
#include <http_main.h>
#include <http_protocol.h>

#include <glib.h>
#include <libsoup/soup-server.h>
#include <libsoup/soup-private.h>

GSList *server_list = NULL;

static void 
soup_apache_init (server_rec *s, pool *p)
{
	/* NO-OP */
}

static int
soup_apache_add_header_from_table (gpointer    data, 
				   const char *key, 
				   const char *val)
{
	GHashTable *hash = data;
	soup_message_add_header (hash, key, val);
	return TRUE;
}

static SoupMessage *
soup_apache_message_create (request_rec *r, gchar *read_buf, gint read_len)
{
	SoupMessage *msg;
	SoupContext *ctx;
	char *uri;

	uri = ap_construct_url (r->pool, r->unparsed_uri, r);
	ctx = soup_context_get (uri);
	if (!ctx) return NULL;

	msg = soup_message_new (ctx, r->method);
	if (!msg) return NULL;

	msg->request.owner = SOUP_BUFFER_STATIC;
	msg->request.length = read_len;
	msg->request.body = read_buf;

	ap_table_do (soup_apache_add_header_from_table,
		     (gpointer) msg->request_headers,
		     r->headers_in,
		     NULL);

	return msg;
}

static int
soup_apache_read_request (request_rec *r, gchar **out, gint *len)
{
	int ret = OK;
	gchar *read_buf;
	gint read_len, read_left, chunk;

	ret = ap_setup_client_block (r, REQUEST_CHUNKED_ERROR);
	if (ret != OK) return ret;

	if (!ap_should_client_block (r)) return !OK;

	read_buf = ap_palloc (r->pool, r->remaining + 1);
	read_len = read_left = r->remaining;

	ap_hard_timeout ("soup_apache_read_request", r);

	for (; read_left; read_left -= chunk) {
		chunk = ap_get_client_block (r, 
					     &read_buf [read_len - read_left], 
					     read_left);

		ap_reset_timeout (r);
	
		if (chunk <= 0) return HTTP_INTERNAL_SERVER_ERROR;
	}

	ap_kill_timeout (r);

	/* Null terminate the buffer */
	read_buf[read_len] = '\0';
	
	*out = read_buf;
	*len = read_len;
			
	return OK;
}

static void
soup_apache_add_header (gchar *key, gchar *val, request_rec *r)
{
	ap_table_set (r->headers_out, key, val);
}

static SoupServer *
soup_apache_get_server (request_rec *r)
{
	SoupServer *ret;
	GSList *iter;
	gint req_port;
	SoupProtocol req_proto;

	req_port = ap_get_server_port (r);

	if (!g_strcasecmp (ap_http_method (r), "https"))
		req_proto = SOUP_PROTOCOL_HTTPS;
	else
		req_proto = SOUP_PROTOCOL_HTTP;

	for (iter = server_list; iter; iter = iter->next) {
		ret = iter->data;

		if (ret->port == req_port && ret->proto == req_proto)
			return ret;
	}

	/*
	 * Create SoupServer for new protocol/port combination
	 */
	ret = g_new0 (SoupServer, 1);
	ret->proto = req_proto;
	ret->port = req_port;
	ret->refcnt = 1;

	/*
	 * Run this module's soup_server_init function
	 */
	soup_server_init (ret);

	server_list = g_slist_prepend (server_list, ret);

	return ret;
}

static int 
soup_apache_handler (request_rec *r) 
{
	SoupServerHandler *hand;
	SoupServerAuth auth;
	SoupMessage *msg;
	SoupServer *server;
	gint read_len = 0, ret = OK;
	gchar *read_buf = NULL;
	const gchar *auth_type;

	server = soup_apache_get_server (r);
	if (!server)
		return DECLINED;

	hand = soup_server_get_handler (server, r->uri);
	if (!hand) {
		ap_log_error (APLOG_MARK, 
			      APLOG_INFO, 
			      r->server, 
			      "No handler found for path '%s'.", 
			      r->uri);
		return DECLINED;
	}

	ret = soup_apache_read_request (r, &read_buf, &read_len);
	if (ret != OK) 
		return ret;

	msg = soup_apache_message_create (r, read_buf, read_len);
	if (!msg) 
		return HTTP_INTERNAL_SERVER_ERROR;

	auth_type = ap_auth_type (r);
	if (auth_type) {
		if (!g_strcasecmp (auth_type, "Digest")) {
			/* FIXME: Digest auth currently unsupported */
		} 
		else if (!g_strcasecmp (auth_type, "Basic")) {
			auth.type = SOUP_AUTH_TYPE_BASIC;
			auth.basic.user = r->connection->user;
			ap_get_basic_auth_pw (r, &auth.basic.passwd);
		} 
		else if (!g_strcasecmp (auth_type, "NTLM")) {
			/* FIXME: NTLM auth currently unsupported */
		}
	}

	if (hand->callback) {
		SoupServerContext servctx = {
			msg,
			msg->context->uri->path,
			soup_method_get_id (msg->method),
			auth_type ? &auth : NULL,
			server,
			hand
		};

		/* Call method handler */
		(*hand->callback) (&servctx, msg, hand->user_data);
	}

	if (!msg->errorcode) return DECLINED;

	/* FIXME: Set in soap handler */
	/* r->content_type = "text/xml; charset=utf-8"; */

	r->status_line = ap_psprintf (r->pool, 
				      "%d %s", 
				      msg->errorcode,
				      msg->errorphrase);

	g_hash_table_foreach (msg->response_headers, 
			      (GHFunc) soup_apache_add_header,
			      r);

	ap_hard_timeout ("soup_apache_handler", r);

	ap_send_http_header (r);
	ap_rwrite (msg->response.body, msg->response.length, r);

	ap_kill_timeout (r);

	ret = msg->errorcode == SOUP_ERROR_OK ? OK : !OK;

	soup_message_free (msg);

	return ret;
}

static int 
soup_apache_check_user_id (request_rec *r)
{
	return DECLINED;
}

static int 
soup_apache_auth_checker (request_rec *r)
{
	return DECLINED;
}

static int 
soup_apache_access_checker (request_rec *r)
{
	return DECLINED;
}

static const handler_rec soup_apache_handlers [] =
{
	{"soup-handler", soup_apache_handler},
	{NULL}
};

module MODULE_VAR_EXPORT soup_module =
{
	STANDARD_MODULE_STUFF,
	soup_apache_init,               /* module initializer */
	NULL,                           /* per-directory config creator */
	NULL,                           /* dir config merger */
	NULL,                           /* server config creator */
	NULL,                           /* server config merger */
	NULL,                           /* command table */
	soup_apache_handlers,           /* [9] list of handlers */
	NULL,                           /* [2] filename-to-URI translation */
	soup_apache_check_user_id,      /* [5] check/validate user_id */
	soup_apache_auth_checker,       /* [6] check user_id is valid *here* */
	soup_apache_access_checker,     /* [4] check access by host address */
	NULL,                           /* [7] MIME type checker/setter */
	NULL,                           /* [8] fixups */
	NULL,                           /* [10] logger */
#if MODULE_MAGIC_NUMBER >= 19970103
	NULL,                           /* [3] header parser */
#endif
#if MODULE_MAGIC_NUMBER >= 19970719
	NULL,                           /* process initializer */
#endif
#if MODULE_MAGIC_NUMBER >= 19970728
	NULL,                           /* process exit/cleanup */
#endif
#if MODULE_MAGIC_NUMBER >= 19970902
	NULL                            /* [1] post read_request handling */
#endif
};

#endif /* HAVE_HTTPD_H */
