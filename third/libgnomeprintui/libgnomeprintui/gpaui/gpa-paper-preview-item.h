/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-paper-preview-item.h: A system print dialog
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    James Henstridge <james@daa.com.au>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 1998 James Henstridge <james@daa.com.au>, 2001-2003 Ximian Inc.
 *
 */

#ifndef __GPA_PAPER_PREVIEW_ITEM__H__
#define __GPA_PAPER_PREVIEW_ITEM__H__

#include <glib.h>

G_BEGIN_DECLS

#define GPA_TYPE_PAPER_PREVIEW_ITEM         (gpa_paper_preview_item_get_type ())
#define GPA_PAPER_PREVIEW_ITEM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_PAPER_PREVIEW_ITEM, GpaPaperPreviewItem))
#define GPA_PAPER_PREVIEW_ITEM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k),    GPA_TYPE_PAPER_PREVIEW_ITEM, GpaPaperPreviewItemClass))
#define GPA_IS_PAPER_PREVIEW_ITEM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_PAPER_PREVIEW_ITEM))
#define GPA_IS_PAPER_PREVIEW_ITEM_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k),    GPA_TYPE_PAPER_PREVIEW_ITEM))
#define GPA_PAPER_PREVIEW_ITEM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  GPA_TYPE_PAPER_PREVIEW_ITEM, GpaPaperPreviewItemClass))

#include <libgnomecanvas/gnome-canvas.h>

typedef struct _GpaPaperPreviewItem GpaPaperPreviewItem;

GType              gpa_paper_preview_item_get_type (void);

GnomeCanvasItem *  gpa_paper_preview_item_new (GnomePrintConfig *config, GtkWidget *widget);
void               gpa_paper_preview_item_set_lm_highlights (GpaPaperPreviewItem *pp,
							     gboolean mt, gboolean mb, gboolean ml, gboolean mr);

void               gpa_paper_preview_item_set_logical_margins ( 
	GpaPaperPreviewItem *pp, 
	gdouble l, gdouble r, gdouble t, gdouble b);

G_END_DECLS

#endif /* __GPA_PAPER_PREVIEW_ITEM_H__ */
