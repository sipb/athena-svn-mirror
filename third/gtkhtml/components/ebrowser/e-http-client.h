#ifndef _E_HTTP_CLIENT_H_
#define _E_HTTP_CLIENT_H_

/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Lauris Kaplinski  <lauris@helixcode.com>
*/

#include <ghttp.h>
#include <gtk/gtkobject.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

#define E_HTTP_CLIENT_TYPE (e_http_client_get_type ())
#define E_HTTP_CLIENT(o) (GTK_CHECK_CAST ((o), E_HTTP_CLIENT_TYPE, EHTTPClient))
#define E_IS_HTTP_CLIENT(o) (GTK_CHECK_TYPE ((o), E_HTTP_CLIENT_TYPE))

typedef struct _EHTTPClient EHTTPClient;
typedef struct _EHTTPClientClass EHTTPClientClass;

typedef enum {
	E_HTTP_CLIENT_NONE,
	E_HTTP_CLIENT_CONNECTED,
	E_HTTP_CLIENT_DONE
} EHTTPClientState;

typedef enum {
	E_HTTP_CLIENT_OK,
	E_HTTP_CLIENT_ERROR
} EHTTPClientStatus;

struct _EHTTPClient {
	GtkObject object;
	/* State machine */
	EHTTPClientState state;
	/* Connection parameters */
	gchar * url;
	gchar * body;
	/* Result and interesting headers */
	gint result;
	/* Our request */
	ghttp_request * request;
	gint hops;
	gint pos;
	guint iid, sid;
};

struct _EHTTPClientClass {
	GtkObjectClass parent_class;

	void (* connect) (EHTTPClient * client, gint result);
	void (* get_data) (EHTTPClient * client, gint pos, gint length);
	void (* done) (EHTTPClient * client, EHTTPClientStatus status);
	void (* set_status) (EHTTPClient * client, const gchar * status);
};

GtkType e_http_client_get_type (void);

EHTTPClient * e_http_client_new_get (const gchar * url, const gchar * proxy);
EHTTPClient * e_http_client_new_post (const gchar * url, const gchar * encoding, const gchar * proxy);

#define e_http_client_ref(o) gtk_object_ref (GTK_OBJECT (o))
#define e_http_client_unref(o) gtk_object_unref (GTK_OBJECT (o))

const gchar * e_http_client_get_header (EHTTPClient * client, const gchar * header);
const gchar * e_http_client_get_buffer (EHTTPClient * client);
gint e_http_client_get_content_length (EHTTPClient * client);
gint e_http_client_get_position (EHTTPClient * client);

END_GNOME_DECLS

#endif
