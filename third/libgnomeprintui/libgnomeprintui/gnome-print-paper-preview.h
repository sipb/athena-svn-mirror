/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GNOME_PRINT_PAPER_PREVIEW_H__
#define __GNOME_PRINT_PAPER_PREVIEW_H__

/*
 *  gnome-print-paper-preview.h: Paper preview widget
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
 *
 *  Copyright (C) 1998 James Henstridge <james@daa.com.au>
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#include <libgnomeprint/gnome-print-config.h>
#include <libgnomecanvas/gnome-canvas.h>

#define GNOME_TYPE_PAPER_PREVIEW         (gnome_paper_preview_get_type ())
#define GNOME_PAPER_PREVIEW(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PAPER_PREVIEW, GnomePaperPreview))
#define GNOME_PAPER_PREVIEW_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k),    GNOME_TYPE_PAPER_PREVIEW, GnomePaperPreviewClass))
#define GNOME_IS_PAPER_PREVIEW(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PAPER_PREVIEW))
#define GNOME_IS_PAPER_PREVIEW_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k),    GNOME_TYPE_PAPER_PREVIEW))
#define GNOME_PAPER_PREVIEW_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  GNOME_TYPE_PAPER_PREVIEW, GnomePaperPreviewClass))

typedef struct _GnomePaperPreview      GnomePaperPreview;
typedef struct _GnomePaperPreviewClass GnomePaperPreviewClass;

/* Has to go into the .c file */
struct _GnomePaperPreview {
	GtkHBox box;

	GtkWidget *canvas;

	GnomeCanvasItem *item;

	GnomePrintConfig *config;
};


GType             gnome_paper_preview_get_type (void);

GtkWidget *       gnome_paper_preview_new (GnomePrintConfig *config);

G_END_DECLS

#endif /* __GNOME_PRINT_PAPER_PREVIEW_H__ */
