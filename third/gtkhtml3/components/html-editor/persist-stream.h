/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2002 Ximian, Inc.
   Authors:           Radek Doulik (rodo@ximian.com)

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

#ifndef _GTK_HTML_PERSIST_STREAM_H_
#define _GTK_HTML_PERSIST_STREAM_H_

#include <bonobo/bonobo-persist.h>
#include "gtkhtml-types.h"

G_BEGIN_DECLS

struct _GtkHTMLPersistStream;
typedef struct _GtkHTMLPersistStream GtkHTMLPersistStream;
typedef struct _GtkHTMLPersistStreamPrivate GtkHTMLPersistStreamPrivate;

#define GTK_HTML_TYPE_PERSIST_STREAM        (gtk_html_persist_stream_get_type ())
#define GTK_HTML_PERSIST_STREAM(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_HTML_TYPE_PERSIST_STREAM, \
									 GtkHTMLPersistStream))
#define GTK_HTML_PERSIST_STREAM_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), GTK_HTML_TYPE_PERSIST_STREAM, \
								     GtkHTMLPersistStreamClass))
#define GTK_HTML_IS_PERSIST_STREAM(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_HTML_TYPE_PERSIST_STREAM))
#define GTK_HTML_IS_PERSIST_STREAM_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GTK_HTML_TYPE_PERSIST_STREAM))

struct _GtkHTMLPersistStream {
	BonoboPersist parent;

	GtkHTML *html;
};

typedef struct {
	BonoboPersistClass parent_class;

	POA_Bonobo_PersistStream__epv epv;

} GtkHTMLPersistStreamClass;

GType            gtk_html_persist_stream_get_type (void);
BonoboObject    *gtk_html_persist_stream_new      (GtkHTML *html);

G_END_DECLS

#endif
