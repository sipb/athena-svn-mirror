/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-cell-renderer-pixbuf-list.c - A cell renderer for pixbuf lists 

   Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
   
   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Anders Carlsson <andersca@gnu.org>
*/

#include <config.h>

#include "eel-cell-renderer-pixbuf-list.h"

#include <eel/eel-gtk-macros.h>

struct EelCellRendererPixbufListDetails {
	GValueArray *value_array;
};

static void eel_cell_renderer_pixbuf_list_get_property  (GObject                    *object,
							 guint                       param_id,
							 GValue                     *value,
							 GParamSpec                 *pspec);
static void eel_cell_renderer_pixbuf_list_set_property  (GObject                    *object,
							 guint                       param_id,
							 const GValue               *value,
							 GParamSpec                 *pspec);

static void eel_cell_renderer_pixbuf_list_init       (EelCellRendererPixbufList      *cell);
static void eel_cell_renderer_pixbuf_list_class_init (EelCellRendererPixbufListClass *klass);
static void eel_cell_renderer_pixbuf_list_get_size   (GtkCellRenderer                *cell,
						      GtkWidget                     *widget,
						      GdkRectangle                  *rectangle,
						      gint                          *x_offset,
						      gint                          *y_offset,
						      gint                          *width,
						      gint                          *height);
static void eel_cell_renderer_pixbuf_list_render     (GtkCellRenderer               *cell,
						      GdkWindow                     *window,
						      GtkWidget                     *widget,
						      GdkRectangle                  *background_area,
						      GdkRectangle                  *cell_area,
						      GdkRectangle                  *expose_area,
						      guint                          flags);

enum {
	PROP_ZERO,
	PROP_PIXBUFS,
};

EEL_CLASS_BOILERPLATE (EelCellRendererPixbufList, eel_cell_renderer_pixbuf_list, GTK_TYPE_CELL_RENDERER)

static void
eel_cell_renderer_pixbuf_list_init (EelCellRendererPixbufList *cell)
{
	cell->details = g_new0 (EelCellRendererPixbufListDetails, 1);
}

static void
eel_cell_renderer_pixbuf_list_class_init (EelCellRendererPixbufListClass *klass)
{
	GtkCellRendererClass *cell_class;
	GObjectClass *object_class;
	
	cell_class = (GtkCellRendererClass *)klass;
	object_class = (GObjectClass *)klass;
	
	object_class->get_property = eel_cell_renderer_pixbuf_list_get_property;
	object_class->set_property = eel_cell_renderer_pixbuf_list_set_property;

	cell_class->get_size = eel_cell_renderer_pixbuf_list_get_size;
	cell_class->render = eel_cell_renderer_pixbuf_list_render;

	g_object_class_install_property (object_class,
					 PROP_PIXBUFS,
					 g_param_spec_value_array ("pixbufs", NULL, NULL,
								   g_param_spec_object ("pixbuf", NULL, NULL,
											GDK_TYPE_PIXBUF,
											G_PARAM_READWRITE),
								   G_PARAM_READWRITE));
					 
}

static void
eel_cell_renderer_pixbuf_list_get_property (GObject *object, guint param_id, GValue *value, GParamSpec *pspec)
{
	EelCellRendererPixbufList *cell;
	
	cell = EEL_CELL_RENDERER_PIXBUF_LIST (object);
	
	switch (param_id) {
	case PROP_PIXBUFS:
		g_value_set_boxed (value, cell->details->value_array);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
eel_cell_renderer_pixbuf_list_set_property (GObject *object, guint param_id, const GValue *value, GParamSpec *pspec)
{
	EelCellRendererPixbufList *cell;
	
	cell = EEL_CELL_RENDERER_PIXBUF_LIST (object);
	
	switch (param_id) {
	case PROP_PIXBUFS:
		if (cell->details->value_array != NULL) {
			g_value_array_free (cell->details->value_array);
		}

		if (g_value_get_boxed (value) == NULL) {
			return;
		}
		
		cell->details->value_array = g_value_array_copy (g_value_get_boxed (value));
		g_object_notify (object, "pixbufs");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
	
}

static void
eel_cell_renderer_pixbuf_list_get_size (GtkCellRenderer *cell,
					GtkWidget *widget,
					GdkRectangle *cell_area,
					gint *x_offset, gint *y_offset,
					gint *width, gint *height)
{
	EelCellRendererPixbufList *list;
	
	int cell_width;
	int cell_height;
	guint i;

	list = EEL_CELL_RENDERER_PIXBUF_LIST (cell);
	
	cell_width = 0;
	cell_height = 0;

	if (list->details->value_array) {
		for (i = 0; i < list->details->value_array->n_values; i++) {
			cell_width += gdk_pixbuf_get_width (g_value_get_object (&list->details->value_array->values[i]));
			cell_height = MAX (cell_height, gdk_pixbuf_get_height (g_value_get_object (&list->details->value_array->values[i])));
		}
	}

	cell_width += (gint) cell->xpad * 2;
	cell_height += (gint) cell->ypad * 2;

	if (y_offset != NULL) {
		*y_offset = 0;
	}

	if (x_offset != NULL) {
		*x_offset = 0;
	}

	if (cell_area != NULL) {
		if (x_offset != NULL) {
			*x_offset = cell->xalign * (cell_area->width - cell_width - (2 * cell->xpad));
			*x_offset = MAX (*x_offset, 0) + cell->xpad;
		}

		if (y_offset != NULL) {
			*y_offset = cell->yalign * (cell_area->height - cell_height - (2 * cell->ypad));
			*y_offset = MAX (*y_offset, 0) + cell->ypad;
		}
	}
	
	if (width != NULL) {
		*width = cell_width;
	}
	if (height != NULL) {
		*height = cell_height;
	}
}

static void
eel_cell_renderer_pixbuf_list_render (GtkCellRenderer *cell,
				      GdkWindow *window,
				      GtkWidget *widget,
				      GdkRectangle *background_area,
				      GdkRectangle *cell_area,
				      GdkRectangle *expose_area,
				      guint        flags)
{
	GdkPixbuf *pixbuf;
	EelCellRendererPixbufList *list;
	int x_offset;
	int y_offset;
	int height;
	guint i;
	
	list = EEL_CELL_RENDERER_PIXBUF_LIST (cell);

	if (list->details->value_array == NULL) {
		return;
	}

	eel_cell_renderer_pixbuf_list_get_size (cell, widget, cell_area,
						&x_offset,
						&y_offset,
						NULL,
						&height);

	x_offset = x_offset + cell_area->x;
	y_offset = y_offset + cell_area->y;

	for (i = 0; i < list->details->value_array->n_values; i++) {
		GdkRectangle pix_rect;

		pixbuf = g_value_get_object (&list->details->value_array->values[i]);
		pix_rect.width = gdk_pixbuf_get_width (pixbuf);
		pix_rect.height = gdk_pixbuf_get_height (pixbuf);
		pix_rect.x = x_offset;
		pix_rect.y = y_offset + (height - pix_rect.height)/2;

		if (pix_rect.x + pix_rect.width + cell->xpad > background_area->width + background_area->x)
			break;

		gdk_pixbuf_render_to_drawable_alpha (pixbuf,
						     window,
						     0, 0,
						     pix_rect.x,
						     pix_rect.y,
						     pix_rect.width,
						     pix_rect.height,
						     GDK_PIXBUF_ALPHA_FULL,
						     0,
						     GDK_RGB_DITHER_NORMAL,
						     0, 0);

		x_offset += gdk_pixbuf_get_width (pixbuf);
	}
}


GtkCellRenderer *
eel_cell_renderer_pixbuf_list_new (void)
{
	return g_object_new (EEL_TYPE_CELL_RENDERER_PIXBUF_LIST, NULL);
}
