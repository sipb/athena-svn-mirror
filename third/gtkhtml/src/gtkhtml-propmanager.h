/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2002 Ximian Inc.
   Authors:           Larry Ewing <lewing@ximian.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHcANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __GTK_HTML_PROPMANAGER_H__
#define __GTK_HTML_PROPMANAGER_H__

#include <gtk/gtkobject.h>
#include <gconf/gconf-client.h>
#include <glade/glade.h>
#include "gtkhtml-properties.h"

#define GTK_TYPE_HTML_PROPMANAGER                (gtk_html_propmanager_get_type ())
#define GTK_HTML_PROPMANAGER(w)                  (GTK_CHECK_CAST ((w), GTK_TYPE_HTML_PROPMANAGER, GtkHTMLPropmanager))
#define GTK_HTML_PROPMANAGER_CLASS(klass)        (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_HTML_PROPMANAGER, GtkHTMLPropmanagerClass))

#define GTK_IS_HTML_PROPMANAGER(w)               (GTK_CHECK_TYPE ((w), GTK_TYPE_HTML_PROPMANAGER))

typedef struct _GtkHTMLPropmanagerPrivate GtkHTMLPropmanagerPrivate;
typedef struct _GtkHTMLPropmanagerClass GtkHTMLPropmanagerClass;
typedef struct _GtkHTMLPropmanager GtkHTMLPropmanager;

struct _GtkHTMLPropmanager {
	GtkObject object;

	GtkHTMLPropmanagerPrivate *priv;
	GConfClient *client;
};

struct _GtkHTMLPropmanagerClass {
	GtkObjectClass parent_class;
	
	void (*changed)(GtkHTMLPropmanager *);
};

GtkType          gtk_html_propmanager_get_type (void);
GtkObject *      gtk_html_propmanager_new (GConfClient *client);
gboolean         gtk_html_propmanager_set_gui (GtkHTMLPropmanager *pman, GladeXML *xml, GHashTable *nametable);
void             gtk_html_propmanager_apply (GtkHTMLPropmanager *pman);
void             gtk_html_propmanager_reset (GtkHTMLPropmanager *pman);

void             gtk_html_propmanager_set_names (GtkHTMLPropmanager *pman, char *names[][2]);
void             gtk_html_propmanager_set_nametable (GtkHTMLPropmanager *pman, GHashTable *table);
#endif /* __GTK_HTML_PROPMANAGER_H__ */



