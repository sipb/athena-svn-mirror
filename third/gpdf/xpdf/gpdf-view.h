/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  PDF view widget
 *
 *  Copyright (C) 2002 Martin Kretzschmar
 *
 *  Author:
 *    Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GPDF_VIEW_H
#define GPDF_VIEW_H

#include "gpdf-g-switch.h"
#  include <libgnomecanvas/gnome-canvas.h>
#  include <bonobo.h>
#include "gpdf-g-switch.h"
#include "gpdf-control.h"
#include "PDFDoc.h"
#include "Annot.h"


G_BEGIN_DECLS

#define GPDF_TYPE_VIEW            (gpdf_view_get_type ())
#define GPDF_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_VIEW, GPdfView))
#define GPDF_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_VIEW, GPdfViewClass))
#define GPDF_IS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_VIEW))
#define GPDF_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_VIEW))

typedef struct _GPdfViewHistory GPdfViewHistory;
typedef struct _GPdfView        GPdfView;
typedef struct _GPdfViewClass   GPdfViewClass;
typedef struct _GPdfViewPrivate GPdfViewPrivate;
typedef struct _GPdfAnnotsView	GPdfAnnotsView;

struct _GPdfViewHistory {
	double zoom;
	int page; 
	gdouble hval, vval; 
}; 

struct _GPdfView {
	GnomeCanvas parent_object;

	GPdfViewPrivate *priv;
};

struct _GPdfViewClass {
	GnomeCanvasClass parent_class;

	void (*zoom_changed) 	  (GPdfView *gpdf_view, gdouble zoom);
	void (*page_changed) 	  (GPdfView *gpdf_view, gint    page);
	void (*close_requested) (GPdfView *gpdf_view);
	void (*quit_requested)  (GPdfView *gpdf_view);
};

GType      gpdf_view_get_type            (void);

void       gpdf_view_set_pdf_doc         (GPdfView * gpdf_view, 
				          PDFDoc   * pdf_doc);

gint       gpdf_view_get_current_page 	 (GPdfView * gpdf_view); 

void       gpdf_view_scroll_to_top    	 (GPdfView * gpdf_view);

void       gpdf_view_scroll_to_bottom 	 (GPdfView * gpdf_view);

void       gpdf_view_page_prev        	 (GPdfView * gpdf_view);

void       gpdf_view_page_next        	 (GPdfView * gpdf_view);

void       gpdf_view_page_first       	 (GPdfView * gpdf_view);

void       gpdf_view_page_last        	 (GPdfView * gpdf_view);

void       gpdf_view_goto_page        	 (GPdfView * gpdf_view,
				      	  gint       page);

gboolean   gpdf_view_is_first_page    	 (GPdfView * gpdf_view);

gboolean   gpdf_view_is_last_page     	 (GPdfView * gpdf_view);

void       gpdf_view_back_history     	 (GPdfView * gpdf_view);

void       gpdf_view_forward_history  	 (GPdfView * gpdf_view);

gboolean   gpdf_view_is_first_history 	 (GPdfView * gpdf_view);

gboolean   gpdf_view_is_last_history  	 (GPdfView * gpdf_view);

void       gpdf_view_zoom             	 (GPdfView * gpdf_view,
				      	  gdouble    factor,
				       	  gboolean   relative);

void       gpdf_view_zoom_default     	 (GPdfView * gpdf_view);

void       gpdf_view_zoom_in          	 (GPdfView * gpdf_view);

void       gpdf_view_zoom_out         	 (GPdfView * gpdf_view);

void       gpdf_view_zoom_fit_width   	 (GPdfView * gpdf_view,
				      	  gint       width);

void       gpdf_view_zoom_fit         	 (GPdfView * gpdf_view,
				      	  gint       width,
				      	  gint       height);

void	   gpdf_view_bookmark_selected 	 (GPdfView   * gpdf_view,
					  LinkAction * link_action);

void 	   gpdf_view_thumbnail_selected  (GPdfView   * gpdf_view,
					  gint x, gint y,
					  gint w, gint h,
					  gint page); 

void 	   gpdf_view_annotation_selected (GPdfView * gpdf_view,
					  Annot    * annot, 
					  gint       page); 

void 	   gpdf_view_annotation_toggled	 (GPdfView  * gpdf_view,
					  Annot     * annot, 
					  gint        page,
					  gboolean    active);
gboolean   gpdf_view_save_as	   	 (GPdfView    * gpdf_view, 
				      	  const gchar * pathname);
G_END_DECLS

#endif /* GPDF_VIEW_H */
