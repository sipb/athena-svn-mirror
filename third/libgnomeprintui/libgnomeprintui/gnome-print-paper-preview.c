/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-paper-selector.c: A paper selector widget
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

#include <math.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <libgnomeprint/private/gpa-node.h>
#include <libgnomeprint/private/gnome-print-private.h>

#include "gnome-print-paper-preview.h"
#include "gpaui/gpa-paper-preview-item.h"

/*
 * GnomePaperPreview widget
 */

struct _GnomePaperPreviewClass {
	GtkHBoxClass parent_class;
};

static void gnome_paper_preview_class_init (GnomePaperPreviewClass *klass);
static void gnome_paper_preview_init (GnomePaperPreview *preview);
static void gnome_paper_preview_finalize (GObject *object);

static void gnome_paper_preview_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static GtkHBoxClass *preview_parent_class;

GType
gnome_paper_preview_get_type (void)
{
	static GType type = 0;
	
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePaperPreviewClass),
			NULL, NULL,
			(GClassInitFunc) gnome_paper_preview_class_init,
			NULL, NULL,
			sizeof (GnomePaperPreview),
			0,
			(GInstanceInitFunc) gnome_paper_preview_init
		};
		type = g_type_register_static (GTK_TYPE_HBOX, "GnomePaperPreview", &info, 0);
	}
	
	return type;
}

static void
gnome_paper_preview_class_init (GnomePaperPreviewClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	preview_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_paper_preview_finalize;

	widget_class->size_allocate = gnome_paper_preview_size_allocate;
}

static void
gnome_paper_preview_init (GnomePaperPreview *preview)
{
	preview->item   = NULL;
	preview->canvas = NULL;
	preview->config = NULL;
}

static void
gnome_paper_preview_construct (GnomePaperPreview *preview, GnomePrintConfig *config)
{
	gtk_widget_push_colormap (gdk_rgb_get_colormap ());
	preview->canvas = gnome_canvas_new_aa ();
	gtk_widget_pop_colormap ();

	preview->item = gpa_paper_preview_item_new (config, preview->canvas);
	gtk_box_pack_start (GTK_BOX (preview), GTK_WIDGET (preview->canvas), TRUE, TRUE, 0);
	gtk_widget_show (GTK_WIDGET (preview->canvas));

	preview->config = NULL;
}

static void
gnome_paper_preview_finalize (GObject *object)
{
	GnomePaperPreview *preview;

	preview = GNOME_PAPER_PREVIEW (object);

	preview->item = NULL;
	preview->canvas = NULL;

	if (preview->config)
		gnome_print_config_unref (preview->config);
	preview->config = NULL;

	G_OBJECT_CLASS (preview_parent_class)->finalize (object);
}

static void
gnome_paper_preview_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GnomePaperPreview *pp;

	pp = GNOME_PAPER_PREVIEW (widget);

	gnome_canvas_set_scroll_region (GNOME_CANVAS (pp->canvas), 0, 0, allocation->width + 50, allocation->height + 50);

	if (((GtkWidgetClass *) preview_parent_class)->size_allocate)
		((GtkWidgetClass *) preview_parent_class)->size_allocate (widget, allocation);

	gnome_canvas_item_request_update (pp->item);
}

GtkWidget *
gnome_paper_preview_new (GnomePrintConfig *config)
{
	GnomePaperPreview *preview;

	g_return_val_if_fail (config != NULL, NULL);
	
	preview = GNOME_PAPER_PREVIEW (g_object_new (GNOME_TYPE_PAPER_PREVIEW, NULL));

	gnome_paper_preview_construct (preview, config);
		
	return GTK_WIDGET(preview);
}
