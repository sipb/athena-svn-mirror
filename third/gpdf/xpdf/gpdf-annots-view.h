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

#ifndef GPDF_ANNOTS_VIEW_H
#define GPDF_ANNOTS_VIEW_H

#define GPDF_ANNOTS_VIEW_PAGE_ID	 "annots"

#ifdef USE_ANNOTS_VIEW

#include <gpdf-g-switch.h>
#  include <glib-object.h>
#  include <gtk/gtkscrolledwindow.h>
#include <gpdf-g-switch.h>

#include "gpdf-view.h"
#include "PDFDoc.h"
#include "Annot.h"

G_BEGIN_DECLS

#define GPDF_TYPE_ANNOTS_VIEW	         (gpdf_annots_view_get_type ())
#define GPDF_ANNOTS_VIEW(obj)	    	 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_ANNOTS_VIEW, GPdfAnnotsView))
#define GPDF_ANNOTS_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_ANNOTS_VIEW, GPdfAnnotsViewClass))
#define GPDF_IS_ANNOTS_VIEW(obj)	 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_ANNOTS_VIEW))
#define GPDF_IS_ANNOTS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_ANNOTS_VIEW))

typedef struct _GPdfAnnotsView	      GPdfAnnotsView;
typedef struct _GPdfAnnotsViewClass   GPdfAnnotsViewClass;
typedef struct _GPdfAnnotsViewPrivate GPdfAnnotsViewPrivate;

struct _GPdfAnnotsView {
	GtkScrolledWindow window;

	GPdfAnnotsViewPrivate *priv;
};

struct _GPdfAnnotsViewClass {
	GtkScrolledWindowClass parent_class;

	void (*annot_selected) (GPdfAnnotsView *gpdf_annots, Annot *annot, gint page);

	void (*annot_toggled) (GPdfAnnotsView *gpdf_annots, Annot *annot, gint page);

	void (*ready) (GPdfAnnotsView *gpdf_annots);
};

GType	   gpdf_annots_view_get_type	   (void) G_GNUC_CONST;
GtkWidget* gpdf_annots_view_new	   	   (GPdfControl *, GtkWidget *);
void	   gpdf_annots_view_set_pdf_doc    (GPdfAnnotsView*, PDFDoc*);
void	   gpdf_annots_view_show	   (GPdfAnnotsView*);
GtkWidget* gpdf_annots_view_get_tools_menu (GPdfAnnotsView*);
gboolean   gpdf_annots_view_is_annot_displayed (Annot *annot, gpointer user_data);

G_END_DECLS

#endif /* USE_ANNOTS_VIEW */

#endif /* GPDF_ANNOTS_VIEW_H */
