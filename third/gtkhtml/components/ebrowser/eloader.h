#ifndef _ELOADER_H_
#define _ELOADER_H_

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

#include <libgnome/gnome-defs.h>
#include <gtkhtml-stream.h>
#include "ebrowser-widget.h"

BEGIN_GNOME_DECLS

typedef enum {
	ELOADER_OK,
	ELOADER_ERROR
} ELoaderStatus;

#if 0
typedef enum {
	ELOADER_STANDARD,
	ELOADER_ROOT
} ELoaderType;

/* Freshen cache */

#define ELOADER_RELOAD (1 << 4)
#endif

#define ELOADER_TYPE (eloader_get_type ())
#define ELOADER(o) (GTK_CHECK_CAST ((o), ELOADER_TYPE, ELoader))
#define IS_ELOADER(o) (GTK_CHECK_TYPE ((o), ELOADER_TYPE))

typedef struct _ELoader ELoader;
typedef struct _ELoaderClass ELoaderClass;

struct _ELoader {
	GtkObject object;
	EBrowser * ebrowser;
	GtkHTMLStream * stream;
	gchar * sufix;
};

struct _ELoaderClass {
	GtkObjectClass parent_class;

	void (* connect) (ELoader * eloader, const gchar * url, const gchar * content_type);
	void (* done) (ELoader * eloader, ELoaderStatus status);
	void (* set_status) (ELoader * eloader, const gchar * status);
};

GtkType eloader_get_type (void);

void eloader_construct (ELoader * eloader, EBrowser * ebrowser, GtkHTMLStream * stream);

void eloader_set_stream (ELoader * eloader, GtkHTMLStream * stream);
void eloader_set_sufix (ELoader * eloader, const gchar * sufix);

/* Private methods */

void eloader_connect (ELoader * eloader, const gchar * url, const gchar * content_type);
void eloader_done (ELoader * eloader, ELoaderStatus status);
void eloader_set_status (ELoader * eloader, const gchar * status);

#if 0
/* This is DANGEROUS */

void eloader_change_stream (ELoader * el, GtkHTMLStream * stream);
#endif

END_GNOME_DECLS

#endif
