/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-env.c: SOAP environment
 *
 * Authors:
 *      Rodrigo Moya (rodrigo@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <string.h>

#include "soup-env.h"

typedef enum {
	NO_AUTH = 0,
	USER_PASS,
	CALLBACK
} SoupEnvAuthType;

typedef struct {
	SoupEnvAuthType    type;

	gchar             *user;
	gchar             *passwd;

	SoupAuthorizeFn    callback;
	gpointer           user_data;
} SoupEnvAuth;

struct _SoupEnv {
	SoupFault         *fault;
	SoupEnvAuth        auth;
	GSList            *send_hdrs;
	GSList            *recv_hdrs;
	SoupMessage       *msg;
	gboolean           free_msg;
	SoupServerContext *serv_ctx;
};

SoupEnv *
soup_env_new (void)
{
	SoupEnv *env;
	env = g_new0 (SoupEnv, 1);
	return env;
}

SoupEnv *
soup_env_new_server (SoupMessage       *msg,
		     SoupServerContext *ctx)
{
	SoupEnv *env;
	env = g_new0 (SoupEnv, 1);

	env->msg = msg;
	env->free_msg = FALSE;
	env->serv_ctx = ctx;

	return env;
}

static void
free_soap_header (gpointer elem, gpointer not_used) 
{
	SoupSOAPHeader *hdr = elem;
	g_strdup (hdr->name);
	g_strdup (hdr->ns_uri);
	g_strdup (hdr->value);
	g_strdup (hdr->actor_uri);
	g_free (hdr);
}

void
soup_env_free (SoupEnv *env)
{
	g_return_if_fail (env != NULL);

	if (env->fault)
		soup_fault_free (env->fault);

	if (env->msg && env->free_msg)
		soup_message_free (env->msg);

	if (env->auth.type != NO_AUTH) {
		g_free (env->auth.user);
		g_free (env->auth.passwd);
	}

	g_slist_foreach (env->recv_hdrs, free_soap_header, NULL);
	g_slist_free (env->recv_hdrs);

	g_slist_foreach (env->send_hdrs, free_soap_header, NULL);
	g_slist_free (env->send_hdrs);

	g_free ((gpointer) env);
}

SoupEnv *
soup_env_copy (SoupEnv *src)
{
	SoupEnv *env;
	GSList *iter;

	g_return_val_if_fail (src != NULL, NULL);

	env = g_new0 (SoupEnv, 1);

	if (src->msg) {
		env->msg = soup_message_copy (src->msg);
		env->free_msg = TRUE;
	}

	for (iter = src->send_hdrs; iter; iter = iter->next) {
		SoupSOAPHeader *hdr = iter->data;
		soup_env_add_header (env, hdr); 
	}

	switch (src->auth.type) {
	case USER_PASS:
		soup_env_set_auth (env, src->auth.user, src->auth.passwd);
		break;
	case CALLBACK:
		soup_env_set_auth_callback (env, 
					    src->auth.callback, 
					    src->auth.user_data);
		break;
	case NO_AUTH:
		break;
	}

	if (src->fault)
		env->fault = 
			soup_fault_new (soup_fault_get_code (src->fault),
					soup_fault_get_string (src->fault),
					soup_fault_get_actor (src->fault),
					soup_fault_get_detail (src->fault));

	/* NOTE: Server context and received SOAP headers are not copied */
	return env;
}

SoupFault *
soup_env_get_fault (SoupEnv *env)
{
	g_return_val_if_fail (env != NULL, NULL);
	return env->fault;
}

void
soup_env_set_fault (SoupEnv *env, SoupFault *fault)
{
	g_return_if_fail (env != NULL);
	g_return_if_fail (fault != NULL);

	if (env->fault)
		soup_fault_free (fault);

	env->fault = soup_fault_new (soup_fault_get_code (fault),
				     soup_fault_get_string (fault),
				     soup_fault_get_actor (fault),
				     soup_fault_get_detail (fault));
}

void
soup_env_clear_fault (SoupEnv *env)
{
	g_return_if_fail (env != NULL);

	if (env->fault) {
		soup_fault_free (env->fault);
		env->fault = NULL;
	}
}

SoupMessage *
soup_env_get_message (SoupEnv *env)
{
	g_return_val_if_fail (env != NULL, NULL);

	if (!env->msg) {
		env->msg = soup_message_new (NULL, NULL);
		env->free_msg = TRUE;
	}

	return env->msg;
}

SoupServerContext *
soup_env_get_server_context (SoupEnv *env)
{
	g_return_val_if_fail (env != NULL, NULL);
	return env->serv_ctx;
}

void 
soup_env_set_auth (SoupEnv           *env,
		   const gchar       *user,
		   const gchar       *passwd)
{
	g_return_if_fail (env != NULL);

	env->auth.type   = USER_PASS;
	env->auth.user   = g_strdup (user);
	env->auth.passwd = g_strdup (passwd);
}

void
soup_env_set_auth_callback (SoupEnv           *env,
			    SoupAuthorizeFn    callback,
			    gpointer           user_data)
{
	g_return_if_fail (env != NULL);

	env->auth.type      = CALLBACK;
	env->auth.callback  = callback;
	env->auth.user_data = user_data;
}

const GSList *
soup_env_list_headers (SoupEnv *env)
{
	g_return_val_if_fail (env != NULL, NULL);
	return env->recv_hdrs;
}

SoupSOAPHeader *
soup_env_get_header (SoupEnv           *env,
		     const gchar       *name)
{
	GSList *iter;
	gchar *uri = NULL, *attr = NULL;

	g_return_val_if_fail (env != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	attr = strrchr (name, '/');
	if (attr && attr [1]) {
		uri = (char *) name;
		attr++;
	}

	for (iter = env->recv_hdrs; iter; iter = iter->next) {
		SoupSOAPHeader *hdr = iter->data;

		if ((uri && hdr->ns_uri &&
		     !g_strncasecmp (hdr->ns_uri, uri, uri - attr) &&
		     !g_strcasecmp (hdr->name, attr)) ||
		    (!uri && !g_strcasecmp (hdr->name, name)))
			return hdr;
	}

	return NULL;
}

static SoupSOAPHeader *
copy_header (SoupSOAPHeader *hdr)
{
	SoupSOAPHeader *nhdr;

	nhdr = g_new0 (SoupSOAPHeader, 1);
	nhdr->name      = g_strdup (hdr->name);
	nhdr->ns_uri    = g_strdup (hdr->ns_uri);
	nhdr->value     = g_strdup (hdr->value);
	nhdr->actor_uri = g_strdup (hdr->actor_uri);

	return nhdr;
}

void
soup_env_add_header (SoupEnv           *env,
		     SoupSOAPHeader    *hdr)
{
	SoupSOAPHeader *nhdr;

	g_return_if_fail (env != NULL);

	nhdr = copy_header (hdr);
	env->send_hdrs = g_slist_append (env->send_hdrs, nhdr);
}

void 
soup_env_add_recv_header (SoupEnv           *env,
			  SoupSOAPHeader    *hdr)
{
	SoupSOAPHeader *nhdr;

	g_return_if_fail (env != NULL);

	nhdr = copy_header (hdr);
	env->send_hdrs = g_slist_append (env->recv_hdrs, nhdr);	
}

const GSList *
soup_env_list_send_headers  (SoupEnv *env)
{
	g_return_val_if_fail (env != NULL, NULL);
	return env->send_hdrs;
}
