/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-env.h: SOAP environment
 *
 * Authors:
 *      Rodrigo Moya (rodrigo@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef SOUP_ENV_H
#define SOUP_ENV_H

#include <libsoup/soup-fault.h>
#include <libsoup/soup-server.h>
#include <libsoup/soup-misc.h>

typedef struct _SoupEnv SoupEnv;

#define SOUP_ENV_IS_FAULT(_env) \
	(soup_env_get_fault (_env) != NULL)

#define SOUP_ENV_IS_TRANSPORT_ERROR(_env) \
	(SOUP_MESSAGE_IS_ERROR (soup_env_get_message (_env)))

#define SOUP_ENV_IS_ERROR(_env) \
	(SOUP_ENV_IS_TRANSPORT_ERROR (_env) || SOUP_ENV_IS_FAULT (_env))

SoupEnv           *soup_env_new                (void);
void               soup_env_free               (SoupEnv           *env);
SoupEnv           *soup_env_copy               (SoupEnv           *env);

SoupFault         *soup_env_get_fault          (SoupEnv           *env);
void               soup_env_set_fault          (SoupEnv           *env,
						SoupFault         *fault);
void               soup_env_clear_fault        (SoupEnv           *env);

SoupMessage       *soup_env_get_message        (SoupEnv           *env);

SoupServerContext *soup_env_get_server_context (SoupEnv           *env);

void               soup_env_set_auth           (SoupEnv           *env,
						const gchar       *user,
						const gchar       *passwd);

void               soup_env_set_auth_callback  (SoupEnv           *env,
						SoupAuthorizeFn    callback,
						gpointer           user_data);

/* SOAP Headers */
typedef struct {
	char     *name;
	char     *ns_uri;
	char     *value;
	gboolean  must_understand;
	char     *actor_uri;
} SoupSOAPHeader;

/* Incoming Headers */
const GSList      *soup_env_list_headers       (SoupEnv           *env);

SoupSOAPHeader    *soup_env_get_header         (SoupEnv           *env,
						const gchar       *name);

/* Outgoing Headers */
void               soup_env_add_header         (SoupEnv           *env,
						SoupSOAPHeader    *hdr);

/* Internal Usage */
SoupEnv           *soup_env_new_server         (SoupMessage       *msg,
						SoupServerContext *ctx);
const GSList      *soup_env_list_send_headers  (SoupEnv           *env);
void               soup_env_add_recv_header    (SoupEnv           *env,
						SoupSOAPHeader    *hdr);

#endif
