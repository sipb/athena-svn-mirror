/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-dav.h: DAV client support.
 *
 * Authors:
 *      Alex Graveley (alex@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef SOUP_DAV_H
#define SOUP_DAV_H 1

#include <glib.h>
#include <libsoup/soup-message.h>

typedef struct {
	gchar      *href;

	GSList     *prop_list; /* CONTAINS: SoupDavProp */

	guint       response_code;
	gchar      *response_reason;
	gchar      *response_desc;	
} SoupDavResponse;

SoupDavResponse *soup_dav_response_new  (gchar           *href,
					 guint            response_code,
					 gchar           *response_desc);

SoupDavResponse *soup_dav_propstat_new  (gchar           *href,
					 GSList          *prop_list,
					 gchar           *response_desc);

void             soup_dav_response_free (SoupDavResponse *resp);

typedef struct {
	GSList           *responses; /* CONTAINS: const SoupDavResponse* */
	SoupMessage      *msg;
} SoupDavMultiStatus;

SoupDavMultiStatus *soup_dav_mstat_new          (SoupMessage        *msg);

void                soup_dav_mstat_free         (SoupDavMultiStatus *ms);

void                soup_dav_mstat_serialize    (SoupDavMultiStatus *ms,
						 SoupDataBuffer     *buf);

void                soup_dav_mstat_add_response (SoupDavMultiStatus *ms, 
						 SoupDavResponse    *resp);

typedef struct {
	gchar      *name;
	gchar      *ns_prefix;
	gchar      *ns_uri;

	gchar      *content;

	guint       response_code;
	gchar      *response_reason;
	gchar      *response_desc;
} SoupDavProp;

SoupDavProp *soup_dav_prop_new          (const gchar *name, 
					 const gchar *ns_prefix, 
					 const gchar *ns_uri, 
					 const gchar *content);

void         soup_dav_prop_free         (SoupDavProp *prop);

void         soup_dav_prop_list_free    (GSList      *prop_list);

SoupDavProp *soup_dav_prop_copy         (SoupDavProp *src);

GSList      *soup_dav_prop_list_copy    (GSList      *src);

void         soup_dav_prop_set_content  (SoupDavProp *prop,
					 const gchar *content);

void         soup_dav_prop_set_error    (SoupDavProp *prop,
					 guint        response_code,
					 const gchar *response_reason,
					 const gchar *response_desc);

#define SOUP_LOCK_WRITE "WRITE"
#define SOUP_LOCK_READ  "READ"

typedef struct {
	gchar            *id;
	const gchar      *type;
	gboolean          exclusive;
} SoupDavLock;

#endif /* SOUP_DAV_H */
