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

#ifndef GPDF_THUMBNAILS_VIEW_H
#define GPDF_THUMBNAILS_VIEW_H

#include <gpdf-g-switch.h>
#  include <glib-object.h>
#  include <gtk/gtkscrolledwindow.h>
#include <gpdf-g-switch.h>

#include "gpdf-view.h"
#include "PDFDoc.h"

G_BEGIN_DECLS

#define GPDF_THUMBNAILS_VIEW_PAGE_ID	     "thumbnails"

#define GPDF_TYPE_THUMBNAILS_VIEW	     (gpdf_thumbnails_view_get_type ())
#define GPDF_THUMBNAILS_VIEW(obj)	     (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_THUMBNAILS_VIEW, GPdfThumbnailsView))
#define GPDF_THUMBNAILS_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_THUMBNAILS_VIEW, GPdfThumbnailsViewClass))
#define GPDF_IS_THUMBNAILS_VIEW(obj)	     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_THUMBNAILS_VIEW))
#define GPDF_IS_THUMBNAILS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_THUMBNAILS_VIEW))

typedef struct _GPdfThumbnailsView	  GPdfThumbnailsView;
typedef struct _GPdfThumbnailsViewClass	  GPdfThumbnailsViewClass;
typedef struct _GPdfThumbnailsViewPrivate GPdfThumbnailsViewPrivate;

struct _GPdfThumbnailsView {
	GtkScrolledWindow window;

	GPdfThumbnailsViewPrivate *priv;
};

struct _GPdfThumbnailsViewClass {
	GtkScrolledWindowClass parent_class;

	void (*thumbnail_selected) (GPdfThumbnailsView *gpdf_thumbnails, gint x, gint y, gint w, gint h, gint page);

	void (*ready) (GPdfThumbnailsView *gpdf_thumbnails);
};

GType	   gpdf_thumbnails_view_get_type       (void) G_GNUC_CONST;
GtkWidget* gpdf_thumbnails_view_new	       (GPdfControl *, GtkWidget *);
void	   gpdf_thumbnails_view_set_pdf_doc    (GPdfThumbnailsView*, PDFDoc*);
GtkWidget* gpdf_thumbnails_view_get_tools_menu (GPdfThumbnailsView*);

G_END_DECLS

#endif /* GPDF_THUMBNAILS_VIEW_H */
