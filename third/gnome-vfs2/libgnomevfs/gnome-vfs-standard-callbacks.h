/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*

   Copyright (C) 2001 Eazel, Inc
   Copyright (C) 2002 Seth Nickell

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Michael Fleming <mfleming@eazel.com>
           Seth Nickell <snickell@stanford.edu>
*/

#ifndef GNOME_VFS_STANDARD_CALLBACKS_H
#define GNOME_VFS_STANDARD_CALLBACKS_H

#include <glib/gtypes.h>
#include <libgnomevfs/gnome-vfs-uri.h>

G_BEGIN_DECLS

/*
 * defined callback structures
 */

/*
 * hook name: "simple-authentication"
 * In arg: GnomeVFSModuleCallbackAuthenticationIn *
 * Out arg: GnomeVFSModuleCallbacAuthenticationOut *
 * 
 * Called when access to a URI requires a username/password
 */
#define GNOME_VFS_MODULE_CALLBACK_AUTHENTICATION "simple-authentication"

/*
 * hook name: "http:proxy-authentication"
 * In arg: GnomeVFSModuleCallbackAuthenticationIn *
 * Out arg: GnomeVFSModuleCallbackAuthenticationOut *
 * 
 * Called when access to an HTTP proxy requires a username/password
 */
#define GNOME_VFS_MODULE_CALLBACK_HTTP_PROXY_AUTHENTICATION "http:proxy-authentication"

typedef struct {
	char *uri;		/* Full URI of operation */
	char *realm;		/* for HTTP auth, NULL for others */
	gboolean previous_attempt_failed;
				/* TRUE if there were credentials specified
				 * for this request, but they resulted in
				 * an authorization error. 
				 * ("you gave me the wrong pw!")
				 * 
				 * FALSE if there were no credentials specified
				 * but they are required to continue
				 * 
				 */
	enum {
		AuthTypeBasic,	/* Password will be transmitted unencrypted */
		AuthTypeDigest	/* Digest is transferred, not plaintext credentials */		
	} auth_type;

	/* Reserved "padding" to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;

} GnomeVFSModuleCallbackAuthenticationIn;

typedef struct {
	char *username;		/* will be freed by g_free,
				 * NULL indicates no auth should be provided;
				 * if the request requires authn, the operation
				 * will fail with a GNOME_VFS_ERROR_ACCESS_DENIED
				 * code
				 */
	char *password;		/* will be freed by g_free */

	/* Reserved "padding" to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;

} GnomeVFSModuleCallbackAuthenticationOut;

/*
 * hook name: "http:send-additional-headers"
 * In arg: GnomeVFSModuleCallbackAdditionalHeadersIn *
 * Out arg: GnomeVFSModuleCallbackAdditionalHeadersOut *
 *
 * Called before sending headers to an HTTP server.
 */

#define GNOME_VFS_MODULE_CALLBACK_HTTP_SEND_ADDITIONAL_HEADERS "http:send-additional-headers"

typedef struct {
	GnomeVFSURI *uri;		/* URI of operation */

	/* Reserved "padding" to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;
} GnomeVFSModuleCallbackAdditionalHeadersIn;

typedef struct {
	GList *headers;			/* list of headers, will be freeed */

	/* Reserved "padding" to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;
} GnomeVFSModuleCallbackAdditionalHeadersOut;

/*
 * hook name: "http:received-headers"
 * In arg: GnomeVFSModuleCallbackReceivedHeadersIn *
 * Out arg: GnomeVFSModuleCallbackReceivedHeadersOut *
 *
 * Called after receiving the HTTP headers from the server.
 */

#define GNOME_VFS_MODULE_CALLBACK_HTTP_RECEIVED_HEADERS "http:received-headers"

typedef struct {
	GnomeVFSURI *uri;		/* URI of operation */
	GList *headers;			/* list of headers */

	/* Reserved "padding" to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;
} GnomeVFSModuleCallbackReceivedHeadersIn;

typedef struct {
	int dummy;

	/* Reserved "padding" to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;
} GnomeVFSModuleCallbackReceivedHeadersOut;

/*
 * hook name: "status-message"
 * In arg: GnomeVFSModuleCallbackStatusMessageIn *
 * Out arg: GnomeVFSModuleCallbackStatusMessageOut *
 * 
 * Called when a GnomeVFS API or module has a status message to return to
 * the user.
 */

#define GNOME_VFS_MODULE_CALLBACK_STATUS_MESSAGE "status-message"

typedef struct {
	char *uri;		/* Full URI of operation */
	char *message;		/* A message indicating the current state or
				 * NULL if there is no message */
	int percentage;		/* Percentage indicating completeness 0-100 or
				 * -1 if there is no progress percentage to
				 * report */

	/* Reserved "padding" to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;
} GnomeVFSModuleCallbackStatusMessageIn;

typedef struct {
	int dummy; /* empty structs not allowed */

	/* Reserved "padding" to avoid future breaks in ABI compatibility */
	void *reserved1;
	void *reserved2;
} GnomeVFSModuleCallbackStatusMessageOut;

G_END_DECLS

#endif /* GNOME_VFS_STANDARD_CALLBACKS_H */
