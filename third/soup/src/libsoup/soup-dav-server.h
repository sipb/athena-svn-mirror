/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-dav-server.h: DAV server support.
 *
 * Authors:
 *      Alex Graveley (alex@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef SOUP_DAV_SERVER_H
#define SOUP_DAV_SERVER_H 1

#include <glib.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-server.h>
#include <libsoup/soup-dav.h>

typedef struct {
	gboolean  (*uri_exists)         (SoupServerContext    *ctx, 
					 const gchar          *path,
					 gpointer              user_data);

	gboolean  (*is_collection)      (SoupServerContext    *ctx, 
					 const gchar          *path,
					 gpointer              user_data);

	GSList   *(*opt_describe_locks) (SoupServerContext    *ctx, 
					 const gchar          *path,
					 gpointer              user_data);

	gboolean  (*opt_lock)           (SoupServerContext    *ctx,  
					 const gchar          *path,
					 const SoupDavLock    *lock, 
					 gpointer              user_data);

	gboolean  (*opt_unlock)         (SoupServerContext    *ctx,  
					 const gchar          *path,
					 const SoupDavLock    *lock, 
					 gpointer              user_data);

	gboolean  (*create_collection)  (SoupServerContext    *ctx,
					 const gchar          *path,
					 gpointer              user_data);

	gboolean  (*create_doc)         (SoupServerContext    *ctx,  
					 const gchar          *path,
					 const SoupDataBuffer *buf, 
					 gpointer              user_data);

	gboolean  (*delete_collection)  (SoupServerContext    *ctx, 
					 const gchar          *path,
					 gpointer              user_data);

	gboolean  (*delete_doc)         (SoupServerContext    *ctx, 
					 const gchar          *path,
					 gpointer              user_data);

	gboolean  (*can_delete)         (SoupServerContext    *ctx, 
					 const gchar          *path,
					 gpointer              user_data);

	GSList   *(*list_contents)      (SoupServerContext    *ctx, 
					 const gchar          *path,
					 gpointer              user_data);

	gboolean  (*get_content)        (SoupServerContext    *ctx,  
					 const gchar          *path,
					 SoupDataBuffer       *out_buf, 
					 gpointer              user_data);

	gboolean  (*set_content)        (SoupServerContext    *ctx,  
					 const gchar          *path,
					 SoupDataBuffer       *in_buf, 
					 gpointer              user_data);

	gboolean  (*get_dav_prop)       (SoupServerContext    *ctx,  
					 const gchar          *path,
					 SoupDavProp          *out_prop, 
					 gpointer              user_data);

	gboolean  (*set_dav_prop)       (SoupServerContext    *ctx,  
					 const gchar          *path,
					 const SoupDavProp    *prop, 
					 gpointer              user_data);

	GSList   *(*list_custom_props)  (SoupServerContext    *ctx, 
					 const gchar          *path,
					 gpointer              user_data);

	gboolean  (*get_custom_prop)    (SoupServerContext    *ctx,  
					 const gchar          *path,
					 SoupDavProp          *out_prop, 
					 gpointer              user_data);

	gboolean  (*set_custom_prop)    (SoupServerContext    *ctx,  
					 const gchar          *path,
					 const SoupDavProp    *prop, 
					 gpointer              user_data);

	gboolean  (*delete_custom_prop) (SoupServerContext    *ctx,  
					 const gchar          *path,
					 const SoupDavProp    *prop, 
					 gpointer              user_data);

	void      (*opt_move)           (SoupServerContext    *ctx,
					 const gchar          *path,
					 const gchar          *new_path,
					 gboolean              overwrite,
					 gpointer              user_data);

	void      (*opt_copy)           (SoupServerContext    *ctx,
					 const gchar          *path,
					 const gchar          *new_path,
					 gboolean              overwrite,
					 gpointer              user_data);
} SoupDavServerHandlers;

SoupMethodId soup_dav_server_process  (SoupServerContext     *ctx,
				       SoupDavServerHandlers *handlers,
				       gpointer               user_data);

#endif /* SOUP_DAV_SERVER_H */
