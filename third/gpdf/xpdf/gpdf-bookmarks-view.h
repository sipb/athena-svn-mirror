/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  PDF view widget
 *
 *  Copyright (C) 2003 Remi Cohen-Scali
 *
 *  Author:
 *    Remi Cohen-Scali <Remi@Cohen-Scali.com>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GPDF_BOOKMARKS_VIEW_H
#define GPDF_BOOKMARKS_VIEW_H

#include <gpdf-g-switch.h>
#  include <glib-object.h>
#  include <gtk/gtkscrolledwindow.h>
#include <gpdf-g-switch.h>

#include "gpdf-view.h"
#include "PDFDoc.h"

G_BEGIN_DECLS

#define GPDF_BOOKMARKS_VIEW_PAGE_ID	    "bookmarks"

#define GPDF_TYPE_BOOKMARKS_VIEW	    (gpdf_bookmarks_view_get_type ())
#define GPDF_BOOKMARKS_VIEW(obj)	    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_BOOKMARKS_VIEW, GPdfBookmarksView))
#define GPDF_BOOKMARKS_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_BOOKMARKS_VIEW, GPdfBookmarksViewClass))
#define GPDF_IS_BOOKMARKS_VIEW(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_BOOKMARKS_VIEW))
#define GPDF_IS_BOOKMARKS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_BOOKMARKS_VIEW))

typedef struct _GPdfBookmarksView	 GPdfBookmarksView;
typedef struct _GPdfBookmarksViewClass	 GPdfBookmarksViewClass;
typedef struct _GPdfBookmarksViewPrivate GPdfBookmarksViewPrivate;

struct _GPdfBookmarksView {
	GtkScrolledWindow window;

	GPdfBookmarksViewPrivate *priv;
};

struct _GPdfBookmarksViewClass {
	GtkScrolledWindowClass parent_class;

	void (*bookmark_selected) (GPdfBookmarksView *gpdf_bookmarks, gint page);

	void (*ready) (GPdfBookmarksView *gpdf_bookmarks);
};

GType	   gpdf_bookmarks_view_get_type	      (void) G_GNUC_CONST;
GtkWidget* gpdf_bookmarks_view_new	      (GPdfControl *control, GtkWidget *);
void	   gpdf_bookmarks_view_set_pdf_doc    (GPdfBookmarksView*,
					       PDFDoc*);
void	   gpdf_bookmarks_view_show	      (GPdfBookmarksView*);
GtkWidget* gpdf_bookmarks_view_get_tools_menu (GPdfBookmarksView*);

G_END_DECLS

#endif /* GPDF_BOOKMARKS_VIEW_H */
