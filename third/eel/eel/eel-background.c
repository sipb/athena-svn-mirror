/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   eel-background.c: Object for the background of a widget.
 
   Copyright (C) 2000 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Darin Adler <darin@eazel.com>
*/

#include <config.h>
#include "eel-background.h"

#include "eel-gdk-extensions.h"
#include "eel-gdk-pixbuf-extensions.h"
#include "eel-glib-extensions.h"
#include "eel-gnome-extensions.h"
#include "eel-gtk-macros.h"
#include "eel-lib-self-check-functions.h"
#include "eel-string.h"
#include <gtk/gtkprivate.h>
#include <gtk/gtkselection.h>
#include <gtk/gtksignal.h>
#include <libart_lgpl/art_rgb.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomevfs/gnome-vfs-async-ops.h>
#include <math.h>
#include <stdio.h>

static void     eel_background_class_init                (gpointer       klass);
static void     eel_background_init                      (gpointer       object,
							  gpointer       klass);
static void     eel_background_finalize                  (GObject       *object);
static void     eel_background_start_loading_image       (EelBackground *background,
							  gboolean       emit_appearance_change,
							  gboolean       load_async);
static gboolean eel_background_is_image_load_in_progress (EelBackground *background);

EEL_CLASS_BOILERPLATE (EelBackground, eel_background, GTK_TYPE_OBJECT)

enum {
	APPEARANCE_CHANGED,
	SETTINGS_CHANGED,
	IMAGE_LOADING_DONE,
	RESET,
	LAST_SIGNAL
};

/* This is the size of the GdkRGB dither matrix, in order to avoid
 * bad dithering when tiling the gradient
 */
#define GRADIENT_PIXMAP_TILE_SIZE 128

static guint signals[LAST_SIGNAL];

struct EelBackgroundDetails {
	char *color;
	
	int gradient_num_pixels;
	guchar *gradient_buffer;
	gboolean gradient_is_horizontal;

	gboolean is_solid_color;
	GdkColor solid_color;

	gboolean constant_size;
	
	char *image_uri;
	GdkPixbuf *image;
	int image_width_unscaled;
	int image_height_unscaled;
	EelPixbufLoadHandle *load_image_handle;
	gboolean emit_after_load;
	EelBackgroundImagePlacement image_placement;

	/* The image_rect is the area (canvas relative) the image will cover.
	 * Note: image_rect_width/height are not always the same as the image's
	 * width and height - e.g. if the image is tiled the image rect covers
	 * the whole background.
	 */
	int image_rect_x;
	int image_rect_y;
	int image_rect_width;
	int image_rect_height;

	/* Realized data: */
	GdkPixmap *background_pixmap;
	int background_entire_width;
	int background_entire_height;
	GdkColor background_color;
	gboolean background_changes_with_size;
};

static void
eel_background_class_init (gpointer klass)
{
	GObjectClass *object_class;
	EelBackgroundClass *background_class;

	object_class = G_OBJECT_CLASS (klass);
	background_class = EEL_BACKGROUND_CLASS (klass);

	signals[APPEARANCE_CHANGED] =
		g_signal_new ("appearance_changed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
			      G_STRUCT_OFFSET (EelBackgroundClass,
					       appearance_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[SETTINGS_CHANGED] =
		g_signal_new ("settings_changed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
			      G_STRUCT_OFFSET (EelBackgroundClass,
					       settings_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	signals[IMAGE_LOADING_DONE] =
		g_signal_new ("image_loading_done",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
			      G_STRUCT_OFFSET (EelBackgroundClass,
					       image_loading_done),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_BOOLEAN);
	signals[RESET] =
		g_signal_new ("reset",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
			      G_STRUCT_OFFSET (EelBackgroundClass,
					       reset),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	object_class->finalize = eel_background_finalize;
}

static void
eel_background_init (gpointer object, gpointer klass)
{
	EelBackground *background;

	background = EEL_BACKGROUND (object);

	background->details = g_new0 (EelBackgroundDetails, 1);
	background->details->constant_size = FALSE;
	background->details->is_solid_color = TRUE;
}

/* The safe way to clear an image from a background is:
 * 		eel_background_set_image_uri (NULL);
 * This fn is a private utility - it does NOT clear
 * the details->image_uri setting.
 */
static void
eel_background_remove_current_image (EelBackground *background)
{
	if (background->details->image != NULL) {
		g_object_unref (background->details->image);
		background->details->image = NULL;
	}
}

static void
eel_background_finalize (GObject *object)
{
	EelBackground *background;

	background = EEL_BACKGROUND (object);

	eel_cancel_gdk_pixbuf_load (background->details->load_image_handle);
	background->details->load_image_handle = NULL;

	g_free (background->details->color);
	g_free (background->details->gradient_buffer);
	g_free (background->details->image_uri);
	eel_background_remove_current_image (background);

	if (background->details->background_pixmap != NULL) {
		g_object_unref (background->details->background_pixmap);
		background->details->background_pixmap = NULL;
	}

	g_free (background->details);

	EEL_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

void
eel_background_set_is_constant_size (EelBackground *background,
				     gboolean       constant_size)
{
	g_return_if_fail (EEL_IS_BACKGROUND (background));
	
	background->details->constant_size = constant_size;
}

EelBackgroundImagePlacement
eel_background_get_image_placement (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), EEL_BACKGROUND_TILED);

	return background->details->image_placement;
}

static gboolean
eel_background_set_image_placement_no_emit (EelBackground *background,
					    EelBackgroundImagePlacement new_placement)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);

	if (new_placement != background->details->image_placement) {

		if (eel_background_is_image_load_in_progress (background)) {
			/* We try to be smart and keep using the current image for updates
			 * while a new image is loading. However, changing the placement
			 * foils these plans.
			 */
			eel_background_remove_current_image (background);
		}

		background->details->image_placement = new_placement;
		return TRUE;
	} else {
		return FALSE;
	}
}

void
eel_background_set_image_placement (EelBackground              *background,
				    EelBackgroundImagePlacement new_placement)
{
	if (eel_background_set_image_placement_no_emit (background, new_placement)) {
		g_signal_emit (G_OBJECT (background),
			       signals[SETTINGS_CHANGED], 0);
		g_signal_emit (G_OBJECT (background),
			       signals[APPEARANCE_CHANGED], 0);
	}
}

EelBackground *
eel_background_new (void)
{
	return EEL_BACKGROUND (g_object_new (EEL_TYPE_BACKGROUND, NULL));
}
 
static void
reset_cached_color_info (EelBackground *background)
{
	background->details->gradient_num_pixels = 0;
	
	background->details->is_solid_color = !eel_gradient_is_gradient (background->details->color);
	
	if (background->details->is_solid_color) {
		g_free (background->details->gradient_buffer);
		background->details->gradient_buffer = NULL;
		eel_gdk_color_parse_with_white_default (background->details->color, &background->details->solid_color);
	} else {
		/* If color is still a gradient, don't g_free the buffer, eel_background_ensure_gradient_buffered
		 * uses g_realloc to try to reuse it.
		 */
		background->details->gradient_is_horizontal = eel_gradient_is_horizontal (background->details->color);
	}
}

static void
eel_background_ensure_gradient_buffered (EelBackground *background, int dest_width, int dest_height)
{
	int num_pixels;

	guchar *buff_ptr;
	guchar *buff_limit;

	GdkColor cur_color;

	char* color_spec;
	const char* spec_ptr;

	if (background->details->is_solid_color) {
		return;
	}

	num_pixels = background->details->gradient_is_horizontal ? dest_width : dest_height;

	if (background->details->gradient_num_pixels == num_pixels) {
		return;
	}

	background->details->gradient_num_pixels = num_pixels;
	background->details->gradient_buffer = g_realloc (background->details->gradient_buffer, num_pixels * 3);
	
	buff_ptr   = background->details->gradient_buffer;
	buff_limit = background->details->gradient_buffer + num_pixels * 3;

	spec_ptr = background->details->color;
	
	color_spec = eel_gradient_parse_one_color_spec (spec_ptr, NULL, &spec_ptr);
	eel_gdk_color_parse_with_white_default (color_spec, &cur_color);
	g_free (color_spec);

	while (spec_ptr != NULL && buff_ptr < buff_limit) {
		int percent;
		int fill_pos;
		int fill_width;
		int dr, dg, db;
		guchar *fill_limit;
		GdkColor new_color;
	
		color_spec = eel_gradient_parse_one_color_spec (spec_ptr, &percent, &spec_ptr);
		eel_gdk_color_parse_with_white_default (color_spec, &new_color);
		g_free (color_spec);

		dr = new_color.red   - cur_color.red;
		dg = new_color.green - cur_color.green;
		db = new_color.blue  - cur_color.blue;

		fill_pos   = 1;
		fill_limit = MIN (background->details->gradient_buffer + 3 * ((num_pixels * percent) / 100), buff_limit);
		fill_width = (fill_limit - buff_ptr) / 3;
		
		while (buff_ptr < fill_limit) {
			*buff_ptr++ = (cur_color.red   + (dr * fill_pos) / fill_width) >> 8;
			*buff_ptr++ = (cur_color.green + (dg * fill_pos) / fill_width) >> 8;
			*buff_ptr++ = (cur_color.blue  + (db * fill_pos) / fill_width) >> 8;
			++fill_pos;
		}
		cur_color = new_color;
	}

	/* fill in the remainder */
	art_rgb_fill_run (buff_ptr, cur_color.red, cur_color.green, cur_color.blue, (buff_limit - buff_ptr) / 3);	
}

static void
canvas_gradient_helper_v (const GnomeCanvasBuf *buf, const art_u8 *gradient_buff)
{
	int width  = buf->rect.x1 - buf->rect.x0;
	int height = buf->rect.y1 - buf->rect.y0;

	art_u8 *dst       = buf->buf;
	art_u8 *dst_limit = buf->buf + height * buf->buf_rowstride;

	gradient_buff += buf->rect.y0 * 3;
	
	while (dst < dst_limit) {
		art_u8 r = *gradient_buff++;
		art_u8 g = *gradient_buff++;
		art_u8 b = *gradient_buff++;
 		art_rgb_fill_run (dst, r, g, b, width);
		dst += buf->buf_rowstride;
	}
}

static void
canvas_gradient_helper_h (const GnomeCanvasBuf *buf, const art_u8 *gradient_buff)
{
	int width  = buf->rect.x1 - buf->rect.x0;
	int height = buf->rect.y1 - buf->rect.y0;

	art_u8 *dst       = buf->buf;
	art_u8 *dst_limit = buf->buf + height * buf->buf_rowstride;

	int copy_bytes_per_row = width * 3;

	gradient_buff += buf->rect.x0 * 3;
	
	while (dst < dst_limit) {
 		memcpy (dst, gradient_buff, copy_bytes_per_row);
		dst += buf->buf_rowstride;
	}
}

static void
fill_canvas_from_gradient_buffer (const GnomeCanvasBuf *buf, const EelBackground *background)
{
	g_return_if_fail (background->details->gradient_buffer != NULL);

	/* FIXME bugzilla.eazel.com 4876: This hack is needed till we fix background
	 * scolling.
	 *
	 * I.e. currently you can scroll off the end of the gradient - and we
	 * handle this by pegging it the the last rgb value.
	 *
	 * It might be needed permanently after depending on how this is fixed.
	 * If we tie gradients to the boundry of icon placement (as opposed to
	 * window size) then when dragging an icon you might scroll off the
	 * end of the gradient - which will get recaluated after the drop.
	 */
	if (background->details->gradient_is_horizontal) {
		if (buf->rect.x1 > background->details->gradient_num_pixels) {
			art_u8 *rgb888 = background->details->gradient_buffer + (background->details->gradient_num_pixels - 1) * 3;
			GnomeCanvasBuf gradient = *buf;
			GnomeCanvasBuf overflow = *buf;
			gradient.rect.x1 =  gradient.rect.x0 < background->details->gradient_num_pixels ? background->details->gradient_num_pixels : gradient.rect.x0;
			overflow.buf += (gradient.rect.x1 - gradient.rect.x0) * 3;
			overflow.rect.x0 = gradient.rect.x1;
			eel_gnome_canvas_fill_rgb (&overflow, rgb888[0], rgb888[1], rgb888[2]);
			canvas_gradient_helper_h (&gradient, background->details->gradient_buffer);
			return;
		}
	} else {
		if (buf->rect.y1 > background->details->gradient_num_pixels) {
			art_u8 *rgb888 = background->details->gradient_buffer + (background->details->gradient_num_pixels - 1) * 3;
			GnomeCanvasBuf gradient = *buf;
			GnomeCanvasBuf overflow = *buf;
			gradient.rect.y1 = gradient.rect.y0 < background->details->gradient_num_pixels ? background->details->gradient_num_pixels : gradient.rect.y0;
			overflow.buf += (gradient.rect.y1 - gradient.rect.y0) * gradient.buf_rowstride;
			overflow.rect.y0 = gradient.rect.y1;
			eel_gnome_canvas_fill_rgb (&overflow, rgb888[0], rgb888[1], rgb888[2]);
			canvas_gradient_helper_v (&gradient, background->details->gradient_buffer);
			return;
		}
	}

	(background->details->gradient_is_horizontal ? canvas_gradient_helper_h : canvas_gradient_helper_v) (buf, background->details->gradient_buffer);
}

/* Initializes a pseudo-canvas buf so canvas drawing routines can be used to draw into a pixbuf.
 */
static void
canvas_buf_from_pixbuf (GnomeCanvasBuf* buf, GdkPixbuf *pixbuf, int x, int y, int width, int height)
{
	buf->buf =  gdk_pixbuf_get_pixels (pixbuf);
	buf->buf_rowstride =  gdk_pixbuf_get_rowstride (pixbuf);
	buf->rect.x0 = x;
	buf->rect.y0 = y;
	buf->rect.x1 = x + width;
	buf->rect.y1 = y + height;
	buf->bg_color = 0xFFFFFFFF;
	buf->is_bg = TRUE;
	buf->is_buf = FALSE;
}

static gboolean
eel_background_image_totally_obscures (EelBackground *background)
{
	if (background->details->image == NULL || gdk_pixbuf_get_has_alpha (background->details->image)) {
		return FALSE;
	}

	switch (background->details->image_placement) {
	case EEL_BACKGROUND_TILED:
	case EEL_BACKGROUND_SCALED:
		return TRUE;
	default:
		g_assert_not_reached ();
		/* fall through */
	case EEL_BACKGROUND_CENTERED:
	case EEL_BACKGROUND_SCALED_ASPECT:
		/* It's possible that the image totally obscures in this case, but we don't
		 * have enough info to know. So we guess conservatively.
		 */
		return FALSE;
	}
}

static void
eel_background_ensure_image_scaled (EelBackground *background, int dest_width, int dest_height)
{
	/* Avoid very bad gdk-pixbuf scaling behaviour on 1x1 (not yet
	 * sized) windows and large backrounds
	 */
	if (background->details->image == NULL ||
	    (dest_width == 1 && dest_height == 1)) {
		background->details->image_rect_x = 0;
		background->details->image_rect_y = 0;
		background->details->image_rect_width = 0;
		background->details->image_rect_height = 0;
	} else if (!eel_background_is_image_load_in_progress (background)){
		int image_width;
		int image_height;
		int fit_width;
		int fit_height;
		gboolean cur_scaled;
		gboolean reload_image;
		GdkPixbuf *scaled_pixbuf;

		image_width = gdk_pixbuf_get_width (background->details->image);
		image_height = gdk_pixbuf_get_height (background->details->image);
		cur_scaled = image_width != background->details->image_width_unscaled ||
			     image_height != background->details->image_height_unscaled;
		reload_image = FALSE;
		scaled_pixbuf = NULL;
	
		switch (background->details->image_placement) {
		case EEL_BACKGROUND_TILED:
		case EEL_BACKGROUND_CENTERED:
			reload_image = cur_scaled;
			break;
		case EEL_BACKGROUND_SCALED:
			if (image_width != dest_width || image_height != dest_height) {
				if (cur_scaled) {
					reload_image = TRUE;
				} else {
					scaled_pixbuf = gdk_pixbuf_scale_simple (background->details->image, dest_width, dest_height, GDK_INTERP_BILINEAR);
					g_object_unref (background->details->image);
					background->details->image = scaled_pixbuf;
					image_width = gdk_pixbuf_get_width (scaled_pixbuf);
					image_height = gdk_pixbuf_get_height (scaled_pixbuf);
				}
			}
			break;
		case EEL_BACKGROUND_SCALED_ASPECT:
			eel_gdk_scale_to_fit_factor (background->details->image_width_unscaled,
						     background->details->image_height_unscaled,
						     dest_width, dest_height,
						     &fit_width, &fit_height);
			if (image_width != fit_width || image_height != fit_height) {
				if (cur_scaled) {
					reload_image = TRUE;
				} else {
					scaled_pixbuf = eel_gdk_pixbuf_scale_to_fit (background->details->image, dest_width, dest_height);
					g_object_unref (background->details->image);
					background->details->image = scaled_pixbuf;
					image_width = gdk_pixbuf_get_width (scaled_pixbuf);
					image_height = gdk_pixbuf_get_height (scaled_pixbuf);
				}
			}
			break;
		}

		if (reload_image) {
			g_object_unref (background->details->image);
			background->details->image = NULL;
			eel_background_start_loading_image (background, TRUE, TRUE);
			background->details->image_rect_x = 0;
			background->details->image_rect_y = 0;
			background->details->image_rect_width = 0;
			background->details->image_rect_height = 0;
		} else if (background->details->image_placement == EEL_BACKGROUND_TILED) {
			background->details->image_rect_x = 0;
			background->details->image_rect_y = 0;
			background->details->image_rect_width = dest_width;
			background->details->image_rect_height = dest_height;
		} else {
			background->details->image_rect_x = (dest_width - image_width) / 2;
			background->details->image_rect_y = (dest_height - image_height) / 2;
			background->details->image_rect_width = image_width;
			background->details->image_rect_height = image_height;
		}
	}
}

static gboolean
get_pixmap_size (EelBackground    *background,
		 int               entire_width,
		 int               entire_height,
		 int              *pixmap_width,
		 int              *pixmap_height,
		 gboolean         *changes_with_size)
{
	*pixmap_width = 0;	
	*pixmap_height = 0;
	*changes_with_size = ! background->details->constant_size;
	
	if (background->details->image == NULL) {
		if (background->details->is_solid_color) {
			*changes_with_size = FALSE;
			return FALSE;
		}
		if (background->details->gradient_is_horizontal) {
			*pixmap_width = entire_width;
			*pixmap_height = GRADIENT_PIXMAP_TILE_SIZE;
		} else {
			*pixmap_width = GRADIENT_PIXMAP_TILE_SIZE;
			*pixmap_height = entire_height;
		}
		return TRUE;
	} else if (!eel_background_is_image_load_in_progress (background)) {
		switch (background->details->image_placement) {
		case EEL_BACKGROUND_TILED:
			if (background->details->is_solid_color || !gdk_pixbuf_get_has_alpha (background->details->image)) {
				*pixmap_width = background->details->image_width_unscaled;
				*pixmap_height = background->details->image_height_unscaled;
				*changes_with_size = FALSE;
			} else {
				if (background->details->gradient_is_horizontal) {
					*pixmap_width = entire_width;
					*pixmap_height = background->details->image_height_unscaled;
				} else {
					*pixmap_width = background->details->image_width_unscaled;
					*pixmap_height = entire_height;
				}
			}
			break;
		case EEL_BACKGROUND_CENTERED:
		case EEL_BACKGROUND_SCALED:
		case EEL_BACKGROUND_SCALED_ASPECT:
			*pixmap_width = entire_width;
			*pixmap_height = entire_height;
		}
		return TRUE;
	}
	return FALSE;
}


static void
eel_background_unrealize (EelBackground *background)
{
	if (background->details->background_pixmap != NULL) {
		g_object_unref (background->details->background_pixmap);
		background->details->background_pixmap = NULL;
	}
	background->details->background_entire_width = 0;
	background->details->background_entire_height = 0;
}

static void
eel_background_ensure_realized (EelBackground *background, GdkWindow *window,
				int entire_width, int entire_height)
{
	GdkColor color;
	int pixmap_width, pixmap_height;
	char *start_color_spec;
 	GdkPixmap *pixmap;
	GdkGC *gc;
	GtkWidget *widget;
	GtkStyle *style;

	/* Try to parse the color spec.  If we fail, default to the style's color */

	start_color_spec = eel_gradient_get_start_color_spec (background->details->color);

	if (start_color_spec && eel_gdk_color_parse (start_color_spec, &color))
		background->details->background_color = color;
	else {
		/* Get the widget to which the window belongs and its style as well */
		gdk_window_get_user_data (window, (void **) &widget);
		g_assert (widget != NULL);

		style = gtk_widget_get_style (widget);
		background->details->background_color = style->bg[GTK_STATE_NORMAL];
	}

	g_free (start_color_spec);

	/* If the pixmap doesn't change with the window size, never update
	 * it again.
	 */
	if (background->details->background_pixmap != NULL &&
	    !background->details->background_changes_with_size) {
		return;
	}

	/* If the window size is the same as last time, don't update */
	if (entire_width == background->details->background_entire_width &&
	    entire_height == background->details->background_entire_height) {
		return;
	}

	if (background->details->background_pixmap != NULL) {
		g_object_unref (background->details->background_pixmap);
		background->details->background_pixmap = NULL;
	}
	
	if (get_pixmap_size (background, entire_width, entire_height,
			      &pixmap_width, &pixmap_height, &background->details->background_changes_with_size)) {
		pixmap = gdk_pixmap_new (window, pixmap_width, pixmap_height, -1);
		gc = gdk_gc_new (pixmap);
		eel_background_pre_draw (background,  entire_width, entire_height);
		eel_background_draw (background, pixmap, gc,
				     0, 0, 0, 0,
				     pixmap_width, pixmap_height);
		g_object_unref (gc);
		background->details->background_pixmap = pixmap;
	}

	background->details->background_entire_width = entire_width;
	background->details->background_entire_height = entire_height;
}

GdkPixmap *
eel_background_get_pixmap_and_color (EelBackground *background,
				     GdkWindow     *window,
				     int            entire_width,
				     int            entire_height,
				     GdkColor      *color,
				     gboolean      *changes_with_size)
{
	eel_background_ensure_realized (background, window, entire_width, entire_height);
	
	*color = background->details->background_color;
	*changes_with_size = background->details->background_changes_with_size;
	
	if (background->details->background_pixmap != NULL) {
		return g_object_ref (background->details->background_pixmap);
	} 
	return NULL;
}

void
eel_background_expose (GtkWidget                   *widget,
		       GdkEventExpose              *event)
{
	GdkColor color;
	int window_width;
	int window_height;
	gboolean changes_with_size;
	GdkPixmap *pixmap;
	GdkGC *gc;
	GdkGCValues gc_values;
	GdkGCValuesMask value_mask;

	EelBackground *background;
	
	if (event->window != widget->window) {
		return;
	}
	
	background = eel_get_widget_background (widget);

	gdk_drawable_get_size (widget->window, &window_width, &window_height);
	
	pixmap = eel_background_get_pixmap_and_color (background,
						      widget->window,
						      window_width,
						      window_height,
						      &color,
						      &changes_with_size);
	
        if (!changes_with_size) {
                /* The background was already drawn by X, since we set
                 * the GdkWindow background/back_pixmap.
                 * No need to draw it again. */
                if (pixmap) {
                        g_object_unref (pixmap);
                }
                return;
        }
 
	if (pixmap) {
		gc_values.tile = pixmap;
		gc_values.ts_x_origin = 0;
		gc_values.ts_y_origin = 0;
		gc_values.fill = GDK_TILED;
		value_mask = GDK_GC_FILL | GDK_GC_TILE | GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN;
	} else {
		gdk_rgb_find_color (gtk_widget_get_colormap (widget), &color);
		gc_values.foreground = color;
		gc_values.fill = GDK_SOLID;
		value_mask = GDK_GC_FILL | GDK_GC_FOREGROUND;
	}
	
	gc = gdk_gc_new_with_values (widget->window, &gc_values, value_mask);
	
	gdk_gc_set_clip_rectangle (gc, &event->area);

	gdk_draw_rectangle (widget->window, gc, TRUE, 0, 0, window_width, window_height);
	
	g_object_unref (gc);
	
	if (pixmap) {
		g_object_unref (pixmap);
	}
}

void
eel_background_pre_draw (EelBackground *background, int entire_width, int entire_height)
{
	eel_background_ensure_image_scaled (background, entire_width, entire_height);
	eel_background_ensure_gradient_buffered (background, entire_width, entire_height);
}

void
eel_background_draw (EelBackground *background,
		     GdkDrawable *drawable, GdkGC *gc,
		     int src_x, int src_y,
		     int dest_x, int dest_y,
		     int dest_width, int dest_height)
{
	int x, y;
	int x_canvas, y_canvas;
	int width, height;
	
	GnomeCanvasBuf buffer;
	GdkPixbuf *pixbuf;

	/* Non-aa background drawing is done by faking up a GnomeCanvasBuf
	 * and passing it to the aa code.
	 *
	 * The width and height were chosen to match those used by gnome-canvas
	 * (IMAGE_WIDTH_AA & IMAGE_HEIGHT_AA in libgnomecanvas/gnome-canvas.c)
	 * They're not required to match - so this could be changed if necessary.
	 */
	static const int PIXBUF_WIDTH = 256;
	static const int PIXBUF_HEIGHT = 64;

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, PIXBUF_WIDTH, PIXBUF_HEIGHT);

	/* x & y are relative to the drawable
	 */
	for (y = 0; y < dest_height; y += PIXBUF_HEIGHT) {
		for (x = 0; x < dest_width; x += PIXBUF_WIDTH) {

			width = MIN (dest_width - x, PIXBUF_WIDTH);
			height = MIN (dest_height - y, PIXBUF_HEIGHT);

			x_canvas = src_x + x;
			y_canvas = src_y + y;

			canvas_buf_from_pixbuf (&buffer, pixbuf, x_canvas, y_canvas, width, height);
			eel_background_draw_aa (background, &buffer);
			gdk_pixbuf_render_to_drawable (pixbuf, drawable, gc,
						       0, 0,
						       dest_x + x, dest_y + y,
						       width, height,
						       GDK_RGB_DITHER_MAX, dest_x + x, dest_y + y);
		}
	}
	
	g_object_unref (pixbuf);
}

void
eel_background_draw_to_drawable (EelBackground *background,
				 GdkDrawable *drawable, GdkGC *gc,
				 int drawable_x, int drawable_y,
				 int drawable_width, int drawable_height,
				 int entire_width, int entire_height)
{
	eel_background_pre_draw (background, entire_width, entire_height);
	eel_background_draw (background, drawable, gc,
			     drawable_x, drawable_y,
			     drawable_x, drawable_y,
			     drawable_width, drawable_height);
}

void
eel_background_draw_to_pixbuf (EelBackground *background,
			       GdkPixbuf *pixbuf,
			       int pixbuf_x,
			       int pixbuf_y,
			       int pixbuf_width,
			       int pixbuf_height,
			       int entire_width,
			       int entire_height)
{
	GnomeCanvasBuf fake_buffer;

	g_return_if_fail (background != NULL);
	g_return_if_fail (pixbuf != NULL);

	canvas_buf_from_pixbuf (&fake_buffer, pixbuf, pixbuf_x, pixbuf_y, pixbuf_width, pixbuf_height);

	eel_background_draw_to_canvas (background,
				       &fake_buffer,
				       entire_width,
				       entire_height);
}

/* fill the canvas buffer with a tiled pixbuf */
static void
draw_pixbuf_tiled_aa (GdkPixbuf *pixbuf, GnomeCanvasBuf *buffer)
{
	int x, y;
	int start_x, start_y;
	int tile_width, tile_height;
	
	tile_width = gdk_pixbuf_get_width (pixbuf);
	tile_height = gdk_pixbuf_get_height (pixbuf);
	
	start_x = buffer->rect.x0 - (buffer->rect.x0 % tile_width);
	start_y = buffer->rect.y0 - (buffer->rect.y0 % tile_height);

	for (y = start_y; y < buffer->rect.y1; y += tile_height) {
		for (x = start_x; x < buffer->rect.x1; x += tile_width) {
			eel_gnome_canvas_draw_pixbuf (buffer, pixbuf, x, y);
		}
	}
}

/* draw the background on the anti-aliased canvas */
void
eel_background_draw_aa (EelBackground *background, GnomeCanvasBuf *buffer)
{	
	g_return_if_fail (EEL_IS_BACKGROUND (background));

	/* If the image has alpha - we always draw the gradient behind it.
	 * In principle, we could do better by having already drawn gradient behind
	 * the scaled image. However, this would add a significant amount of
	 * complexity to our image scaling/caching logic. I.e. it would tie the
	 * scaled image to a location on the screen because it holds a gradient.
	 * This is especially problematic for tiled images with alpha.
	 */
	if (!background->details->image ||
	     gdk_pixbuf_get_has_alpha (background->details->image) ||
	     buffer->rect.x0  < background->details->image_rect_x ||
	     buffer->rect.y0  < background->details->image_rect_y ||
	     buffer->rect.x1  > (background->details->image_rect_x + background->details->image_rect_width) ||
	     buffer->rect.y1  > (background->details->image_rect_y + background->details->image_rect_height)) {
		if (background->details->is_solid_color) {
			eel_gnome_canvas_fill_rgb (buffer,
							background->details->solid_color.red >> 8,
							background->details->solid_color.green >> 8,
							background->details->solid_color.blue >> 8);
		} else {
			fill_canvas_from_gradient_buffer (buffer, background);
		}
	}

	if (background->details->image != NULL) {
		switch (background->details->image_placement) {
		case EEL_BACKGROUND_TILED:
			draw_pixbuf_tiled_aa (background->details->image, buffer);
			break;
		default:
			g_assert_not_reached ();
			/* fall through */
		case EEL_BACKGROUND_CENTERED:
		case EEL_BACKGROUND_SCALED:
		case EEL_BACKGROUND_SCALED_ASPECT:
			/* Since the image has already been scaled, all these cases
			 * can be treated identically.
			 */
			eel_gnome_canvas_draw_pixbuf (buffer,
							   background->details->image,
							   background->details->image_rect_x,
							   background->details->image_rect_y);
			break;
		}
	}
					
	buffer->is_bg  = FALSE;
	buffer->is_buf = TRUE;
}

void
eel_background_draw_to_canvas (EelBackground *background,
			       GnomeCanvasBuf *buffer,
			       int entire_width,
			       int entire_height)
{
	eel_background_pre_draw (background, entire_width, entire_height);
	eel_background_draw_aa (background, buffer);
}

char *
eel_background_get_color (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), NULL);

	return g_strdup (background->details->color);
}

char *
eel_background_get_image_uri (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), NULL);

	return g_strdup (background->details->image_uri);
}

static gboolean
eel_background_set_color_no_emit (EelBackground *background,
				  const char *color)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);

	if (eel_strcmp (background->details->color, color) == 0) {
		return FALSE;
	}
	g_free (background->details->color);
	background->details->color = g_strdup (color);
	reset_cached_color_info (background);

	return TRUE;
}

void
eel_background_set_color (EelBackground *background,
			  const char *color)
{
	if (eel_background_set_color_no_emit (background, color)) {
		g_signal_emit (G_OBJECT (background), signals[SETTINGS_CHANGED], 0);
		if (!eel_background_image_totally_obscures (background)) {
			g_signal_emit (GTK_OBJECT (background), signals[APPEARANCE_CHANGED], 0);
		}
	}
}

static void
eel_background_load_image_callback (GnomeVFSResult error,
				    GdkPixbuf *pixbuf,
				    gpointer callback_data)
{
	EelBackground *background;

	background = EEL_BACKGROUND (callback_data);

	background->details->load_image_handle = NULL;

	eel_background_remove_current_image (background);

	/* Just ignore errors. */
	if (pixbuf != NULL) {
		g_object_ref (pixbuf);
		background->details->image = pixbuf;
		background->details->image_width_unscaled = gdk_pixbuf_get_width (pixbuf);
		background->details->image_height_unscaled = gdk_pixbuf_get_height (pixbuf);
	}

	g_signal_emit (G_OBJECT (background), signals[IMAGE_LOADING_DONE], 0,
		       pixbuf != NULL || background->details->image_uri == NULL);

	if (background->details->emit_after_load) {
		g_signal_emit (GTK_OBJECT (background), signals[APPEARANCE_CHANGED], 0);
	}
}

static gboolean
eel_background_is_image_load_in_progress (EelBackground *background)
{
	return background->details->load_image_handle != NULL;
}

static void
eel_background_cancel_loading_image (EelBackground *background)
{
	if (eel_background_is_image_load_in_progress (background)) {
		eel_cancel_gdk_pixbuf_load (background->details->load_image_handle);
		background->details->load_image_handle = NULL;
		g_signal_emit (GTK_OBJECT (background), signals[IMAGE_LOADING_DONE], 0, FALSE);
	}
}

static void
eel_background_start_loading_image (EelBackground *background, gboolean emit_appearance_change,
				    gboolean load_async)
{
	GdkPixbuf *pixbuf;
	background->details->emit_after_load = emit_appearance_change;

	if (background->details->image_uri != NULL) {
		if (load_async) {
			background->details->load_image_handle = eel_gdk_pixbuf_load_async (background->details->image_uri,
											    GNOME_VFS_PRIORITY_DEFAULT,
											    eel_background_load_image_callback,
											    background);
		}
		else {
			pixbuf = eel_gdk_pixbuf_load (background->details->image_uri);
			eel_background_load_image_callback (0, pixbuf, background);

			/* load_image_callback refs the pixbuf so we unref it here */
			if (pixbuf != NULL) {
				g_object_unref (pixbuf);
			}
		}
	} else {
		eel_background_load_image_callback (0, NULL, background);
	}
}

static gboolean
eel_background_set_image_uri_helper (EelBackground *background,
				     const char *image_uri,
				     gboolean emit_setting_change,
				     gboolean emit_appearance_change,
				     gboolean load_async)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);

	if (eel_strcmp (background->details->image_uri, image_uri) == 0) {
		return FALSE;
	}

	eel_background_cancel_loading_image (background);
	
	g_free (background->details->image_uri);
	background->details->image_uri = g_strdup (image_uri);

	/* We do not get rid of the current image here. This gets done after the new
	 * image loads - in eel_background_load_image_callback. This way the
	 * current image can be used if an update is needed before the load completes.
	 */
	
	eel_background_start_loading_image (background, emit_appearance_change, load_async);
	
	if (emit_setting_change) {
		g_signal_emit (GTK_OBJECT (background), signals[SETTINGS_CHANGED], 0);
	}

	return TRUE;
}

void
eel_background_set_image_uri (EelBackground *background, const char *image_uri)
{
	eel_background_set_image_uri_helper (background, image_uri, TRUE, TRUE, TRUE);
}

void
eel_background_set_image_uri_sync (EelBackground *background, const char *image_uri)
{
	eel_background_set_image_uri_helper (background, image_uri, TRUE, TRUE, FALSE);
}

static void
set_image_and_color_image_loading_done_callback (EelBackground *background, gboolean successful_load, char *color)
{
	g_signal_handlers_disconnect_by_func
		(G_OBJECT (background),
		 G_CALLBACK (set_image_and_color_image_loading_done_callback), color);

	eel_background_set_color_no_emit (background, color);

	g_free (color);
	
	/* We always emit , even if the color didn't change, because the image change
	 * relies on us doing it here.
	 */
	g_signal_emit (G_OBJECT (background), signals[SETTINGS_CHANGED], 0);
	g_signal_emit (G_OBJECT (background), signals[APPEARANCE_CHANGED], 0);
}

/* Use this fn to set both the image and color and avoid flash. The color isn't
 * changed till after the image is done loading, that way if an update occurs
 * before then, it will use the old color and image.
 */
static void
eel_background_set_image_uri_and_color (EelBackground *background, const char *image_uri, const char *color)
{
	char *color_copy;

	if (eel_strcmp (background->details->color, color) == 0 &&
	    eel_strcmp (background->details->image_uri, image_uri) == 0) {
		return;
	}

	color_copy = g_strdup (color);

	g_signal_connect (background, "image_loading_done",
			  G_CALLBACK (set_image_and_color_image_loading_done_callback), color_copy);
			    
	/* set_image_and_color_image_loading_done_callback must always be called
	 * because we rely on it to:
	 *  - disconnect the image_loading_done signal handler
	 *  - emit SETTINGS_CHANGED & APPEARANCE_CHANGED
	 *  - free color_copy
	 *  - prevent the common cold
	 */
	     
	/* We use eel_background_set_image_uri_helper because its
	 * return value (if false) tells us whether or not we need to
	 * call set_image_and_color_image_loading_done_callback ourselves.
	 */
	if (!eel_background_set_image_uri_helper (background, image_uri, FALSE, FALSE, TRUE)) {
		set_image_and_color_image_loading_done_callback (background, TRUE, color_copy);
	}
}

void
eel_background_receive_dropped_background_image (EelBackground *background,
						 const char *image_uri)
{
	/* Currently, we only support tiled images. So we set the placement.
	 * We rely on eel_background_set_image_uri_and_color to emit
	 * the SETTINGS_CHANGED & APPEARANCE_CHANGE signals.
	 */
	eel_background_set_image_placement_no_emit (background, EEL_BACKGROUND_TILED);
	
	eel_background_set_image_uri_and_color (background, image_uri, NULL);
}

/**
 * eel_background_is_set:
 * 
 * Check whether the background's color or image has been set.
 */
gboolean
eel_background_is_set (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);

	return background->details->color != NULL
		|| background->details->image_uri != NULL;
}

/* Returns false if the image is still loading, true
 * if it's done loading or there is no image.
 */
gboolean
eel_background_is_loaded (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);
	
	return background->details->image_uri == NULL ||
		   (!eel_background_is_image_load_in_progress (background) && background->details->image != NULL);
}

/**
 * eel_background_reset:
 *
 * Emit the reset signal to forget any color or image that has been
 * set previously.
 */
void
eel_background_reset (EelBackground *background)
{
	g_return_if_fail (EEL_IS_BACKGROUND (background));

	g_signal_emit (GTK_OBJECT (background), signals[RESET], 0);
}

static void
draw_background_callback (GnomeCanvas *canvas, GdkDrawable *drawable,
			  int x, int y, int width, int height)
{
	EelBackground *background;
	GdkGCValuesMask value_mask;
	GdkGCValues gc_values;
	GdkGC *gc;

	background = eel_get_widget_background (GTK_WIDGET (canvas));
	if (background == NULL) {
		return;
	}

	
	eel_background_ensure_realized (background, GTK_WIDGET (canvas)->window,
					MAX (GTK_LAYOUT (canvas)->hadjustment->upper, GTK_WIDGET (canvas)->allocation.width),
					MAX (GTK_LAYOUT (canvas)->vadjustment->upper, GTK_WIDGET (canvas)->allocation.height));


	/* Create a new gc each time.
	 * If this is a speed problem, we can create one and keep it around,
	 * but it's a bit more complicated to ensure that it's always compatible
	 * with whatever drawable is passed in.
	 */
	if (background->details->background_pixmap != NULL) {
		gc_values.tile = background->details->background_pixmap;
		gc_values.ts_x_origin = -x;
		gc_values.ts_y_origin = -y;
		gc_values.fill = GDK_TILED;
		value_mask = GDK_GC_FILL | GDK_GC_TILE |
			GDK_GC_TS_X_ORIGIN | GDK_GC_TS_Y_ORIGIN;
	} else {
		gc_values.foreground = background->details->background_color;
		gdk_rgb_find_color (gtk_widget_get_colormap (GTK_WIDGET (canvas)), &gc_values.foreground);
		gc_values.fill = GDK_SOLID;
		value_mask = GDK_GC_FILL | GDK_GC_FOREGROUND;
	}
		
    	gc = gdk_gc_new_with_values (drawable, &gc_values, value_mask);
	gdk_draw_rectangle (drawable, gc, TRUE, 0, 0, width, height);
	g_object_unref (gc);

	/* We don't want the draw from the base class to happen. */
	g_signal_stop_emission_by_name (canvas, "draw_background");
}

static void
render_background_callback (GnomeCanvas *canvas, GnomeCanvasBuf *buffer)
{
	EelBackground *background;
			
	background = eel_get_widget_background (GTK_WIDGET (canvas));
	if (background == NULL) {
		return;
	}

	eel_background_pre_draw (background,
				 GTK_WIDGET (canvas)->allocation.width,
				 GTK_WIDGET (canvas)->allocation.height);

	eel_background_draw_aa (background, buffer);
	
	/* We don't want the render from the base class to happen. */
	g_signal_stop_emission_by_name (canvas, "render_background");
}

static void
eel_background_set_up_widget (EelBackground *background, GtkWidget *widget)
{
	GtkStyle *style;
	GdkPixmap *pixmap;
	GdkColor color;
	int window_width;
	int window_height;
	gboolean changes_with_size;

	if (!GTK_WIDGET_REALIZED (widget)) {
		return;
	}

	gdk_drawable_get_size (widget->window, &window_width, &window_height);
	
	pixmap = eel_background_get_pixmap_and_color (background,
						      widget->window,
						      window_width,
						      window_height,
						      &color, 
						      &changes_with_size);

	style = gtk_widget_get_style (widget);
	
	gdk_rgb_find_color (style->colormap, &color);
	
	if (pixmap && !changes_with_size) {
		gdk_window_set_back_pixmap (widget->window, pixmap, FALSE);
	} else {
		gdk_window_set_background (widget->window, &color);
	}

	if (pixmap) {
		g_object_unref (pixmap);
	}
}

static void
eel_background_set_up_canvas (GtkWidget *widget)
{
	if (!GNOME_IS_CANVAS (widget)) {
		return;
	}
	if (g_object_get_data (G_OBJECT (widget), "eel_background_canvas_is_set_up") != NULL) {
		return;
	}

	gnome_canvas_set_dither (GNOME_CANVAS (widget), GDK_RGB_DITHER_MAX);

	g_signal_connect (widget, "draw_background",
			  G_CALLBACK (draw_background_callback), NULL);
	g_signal_connect (widget, "render_background",
			  G_CALLBACK (render_background_callback), NULL);

	g_object_set_data (G_OBJECT (widget), "eel_background_canvas_is_set_up", widget);
}

static void
eel_widget_background_changed (GtkWidget *widget, EelBackground *background)
{
	eel_background_unrealize (background);
	eel_background_set_up_widget (background, widget);
	eel_background_set_up_canvas (widget);

	gtk_widget_queue_draw (widget);
}

/* Callback used when the style of a widget changes.  We have to regenerate its
 * EelBackgroundStyle so that it will match the chosen GTK+ theme.
 */
static void
widget_style_set_cb (GtkWidget *widget, GtkStyle *previous_style, gpointer data)
{
	EelBackground *background;
	
	background = EEL_BACKGROUND (data);
	
	eel_widget_background_changed (widget, background);
}

static void
widget_realize_cb (GtkWidget *widget, gpointer data)
{
	EelBackground *background;
	
	background = EEL_BACKGROUND (data);
	
	eel_background_set_up_widget (background, widget);
}

/* Gets the background attached to a widget.

   If the widget doesn't already have a EelBackground object,
   this will create one. To change the widget's background, you can
   just call eel_background methods on the widget.

   If the widget is a canvas, nothing more needs to be done.  For
   normal widgets, you need to call eel_background_expose() from your
   expose handler to draw the background.

   Later, we might want a call to find out if we already have a background,
   or a way to share the same background among multiple widgets; both would
   be straightforward.
*/
EelBackground *
eel_get_widget_background (GtkWidget *widget)
{
	gpointer data;
	EelBackground *background;

	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	/* Check for an existing background. */
	data = g_object_get_data (G_OBJECT (widget), "eel_background");
	if (data != NULL) {
		g_assert (EEL_IS_BACKGROUND (data));
		return data;
	}

	/* Store the background in the widget's data. */
	background = eel_background_new ();
	g_object_ref (background);
	gtk_object_sink (GTK_OBJECT (background));
	g_object_set_data_full (G_OBJECT (widget), "eel_background",
				background, g_object_unref);

	/* Arrange to get the signal whenever the background changes. */
	g_signal_connect_object (background, "appearance_changed",
				 G_CALLBACK (eel_widget_background_changed), widget, G_CONNECT_SWAPPED);
	eel_widget_background_changed (widget, background);

	g_signal_connect_object (widget, "style_set",
				 G_CALLBACK (widget_style_set_cb),
				 background,
				 0);
	g_signal_connect_object (widget, "realize",
				 G_CALLBACK (widget_realize_cb),
				 background,
				 0);

	return background;
}

gboolean
eel_widget_has_attached_background (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

	return g_object_get_data (G_OBJECT (widget), "eel_background") != NULL;
}

GtkWidget *
eel_gtk_widget_find_background_ancestor (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	while (widget != NULL) {
		if (eel_widget_has_attached_background (widget)) {
			return widget;
		}

		widget = widget->parent;
	}

	return NULL;
}

/* determine if a background is darker or lighter than average, to help clients know what
   colors to draw on top with */
gboolean
eel_background_is_dark (EelBackground *background)
{
	GdkColor color, end_color;
	int intensity;
	char *start_color_spec, *end_color_spec;
	guint32 argb;
	guchar a;

	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);
	
	if (background->details->is_solid_color) {
		eel_gdk_color_parse_with_white_default (background->details->color, &color);	
	} else {
		start_color_spec = eel_gradient_get_start_color_spec (background->details->color);
		eel_gdk_color_parse_with_white_default (start_color_spec, &color);	
		g_free (start_color_spec);

		end_color_spec = eel_gradient_get_end_color_spec (background->details->color);
		eel_gdk_color_parse_with_white_default (end_color_spec, &end_color);
		g_free (end_color_spec);
		
		color.red = (color.red + end_color.red) / 2;
		color.green = (color.green + end_color.green) / 2;
		color.blue = (color.blue + end_color.blue) / 2;
	}
	if (background->details->image != NULL) {
		argb = eel_gdk_pixbuf_average_value (background->details->image);
		
		a = argb >> 24;

		color.red = (color.red * (0xFF - a) + ((argb >> 16) & 0xFF) * 0x101 * a) / 0xFF;
		color.green = (color.green * (0xFF - a) + ((argb >> 8) & 0xFF) * 0x101 * a) / 0xFF;
		color.blue = (color.blue * (0xFF - a) + (argb & 0xFF) * 0x101 * a) / 0xFF;
	}
	
	intensity = (color.red * 77 + color.green * 150 + color.blue * 28) >> 16;		 
	return intensity < 160; /* biased slightly to be dark */
}
   
/* handle dropped colors */
void
eel_background_receive_dropped_color (EelBackground *background,
				      GtkWidget *widget,
				      int drop_location_x,
				      int drop_location_y,
				      const GtkSelectionData *selection_data)
{
	guint16 *channels;
	char *color_spec;
	char *new_gradient_spec;
	int left_border, right_border, top_border, bottom_border;

	g_return_if_fail (EEL_IS_BACKGROUND (background));
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (selection_data != NULL);

	/* Convert the selection data into a color spec. */
	if (selection_data->length != 8 || selection_data->format != 16) {
		g_warning ("received invalid color data");
		return;
	}
	channels = (guint16 *) selection_data->data;
	color_spec = g_strdup_printf ("#%02X%02X%02X",
				      channels[0] >> 8,
				      channels[1] >> 8,
				      channels[2] >> 8);

	/* Figure out if the color was dropped close enough to an edge to create a gradient.
	   For the moment, this is hard-wired, but later the widget will have to have some
	   say in where the borders are.
	*/
	left_border = 32;
	right_border = widget->allocation.width - 32;
	top_border = 32;
	bottom_border = widget->allocation.height - 32;
	if (drop_location_x < left_border && drop_location_x <= right_border) {
		new_gradient_spec = eel_gradient_set_left_color_spec (background->details->color, color_spec);
	} else if (drop_location_x >= left_border && drop_location_x > right_border) {
		new_gradient_spec = eel_gradient_set_right_color_spec (background->details->color, color_spec);
	} else if (drop_location_y < top_border && drop_location_y <= bottom_border) {
		new_gradient_spec = eel_gradient_set_top_color_spec (background->details->color, color_spec);
	} else if (drop_location_y >= top_border && drop_location_y > bottom_border) {
		new_gradient_spec = eel_gradient_set_bottom_color_spec (background->details->color, color_spec);
	} else {
		new_gradient_spec = g_strdup (color_spec);
	}
	
	g_free (color_spec);

	eel_background_set_image_uri_and_color (background, NULL, new_gradient_spec);

	g_free (new_gradient_spec);
}

/* self check code */

#if !defined (EEL_OMIT_SELF_CHECK)

void
eel_self_check_background (void)
{
	EelBackground *background;

	background = eel_background_new ();

	eel_background_set_color (background, NULL);
	eel_background_set_color (background, "");
	eel_background_set_color (background, "red");
	eel_background_set_color (background, "red-blue");
	eel_background_set_color (background, "red-blue:h");

	gtk_object_sink (GTK_OBJECT (background));
}

#endif /* !EEL_OMIT_SELF_CHECK */
