#ifndef _ELOADER_HTTP_H_
#define _ELOADER_HTTP_H_

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

#include "eloader.h"
#include "e-http-client.h"

BEGIN_GNOME_DECLS

#define ELOADER_HTTP_TYPE (eloader_http_get_type ())
#define ELOADER_HTTP(o) (GTK_CHECK_CAST ((o), ELOADER_HTTP_TYPE, ELoaderHTTP))
#define IS_LOADER_HTTP(o) (GTK_CHECK_TYPE ((o), ELOADER_HTTP_TYPE))

typedef struct _ELoaderHTTP ELoaderHTTP;
typedef struct _ELoaderHTTPClass ELoaderHTTPClass;
typedef struct _ELHPage ELHPage;

struct _ELoaderHTTP {
	ELoader loader;
	guint cache : 1;
	gchar * url;
	gint hops;
	EHTTPClient * client;
	guint iid;
};

struct _ELoaderHTTPClass {
	ELoaderClass parent_class;
};

GtkType eloader_http_get_type (void);

ELoader * eloader_http_new_get (EBrowser * ebr, const gchar * url, GtkHTMLStream * stream);
ELoader * eloader_http_new_post (EBrowser * ebr, const gchar * url, const gchar * encoding, GtkHTMLStream * stream);

END_GNOME_DECLS

#endif
