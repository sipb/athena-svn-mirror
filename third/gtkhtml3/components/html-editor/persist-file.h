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

#ifndef _GTK_HTML_PERSIST_FILE_H_
#define _GTK_HTML_PERSIST_FILE_H_

#include <bonobo/bonobo-persist.h>
#include "gtkhtml-types.h"

G_BEGIN_DECLS

struct _GtkHTMLPersistFile;
typedef struct _GtkHTMLPersistFile GtkHTMLPersistFile;
typedef struct _GtkHTMLPersistFilePrivate GtkHTMLPersistFilePrivate;

#define GTK_HTML_TYPE_PERSIST_FILE        (gtk_html_persist_file_get_type ())
#define GTK_HTML_PERSIST_FILE(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), GTK_HTML_TYPE_PERSIST_FILE, \
									 GtkHTMLPersistFile))
#define GTK_HTML_PERSIST_FILE_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), GTK_HTML_TYPE_PERSIST_FILE, \
								     GtkHTMLPersistFileClass))
#define GTK_HTML_IS_PERSIST_FILE(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), GTK_HTML_TYPE_PERSIST_FILE))
#define GTK_HTML_IS_PERSIST_FILE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GTK_HTML_TYPE_PERSIST_FILE))

struct _GtkHTMLPersistFile {
	BonoboPersist parent;

	GtkHTML *html;
};

typedef struct {
	BonoboPersistClass parent_class;

	POA_Bonobo_PersistFile__epv epv;

} GtkHTMLPersistFileClass;

GType            gtk_html_persist_file_get_type (void);
BonoboObject    *gtk_html_persist_file_new      (GtkHTML *html);

G_END_DECLS

#endif
