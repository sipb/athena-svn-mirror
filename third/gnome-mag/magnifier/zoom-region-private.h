/*
 * AT-SPI - Assistive Technology Service Provider Interface
 * (Gnome Accessibility Project; http://developer.gnome.org/projects/gap)
 *
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef ZOOM_REGION_PRIVATE_H_
#define ZOOM_REGION_PRIVATE_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _ZoomRegionPrivate {
	GList *q;
	/* bounds of currently exposed region, in target pixmap coords */
	GNOME_Magnifier_RectBounds exposed_bounds; 
	/* bounds of current viewport, not including borders, in view coords */
	GNOME_Magnifier_RectBounds exposed_viewport; 
	/* bounds of valid ('gettable') source area, in source coords */
	GNOME_Magnifier_RectBounds source_area;
	gpointer parent;
	GtkWidget *w;
	GtkWidget *border;
	GdkDrawable *source_drawable;
        GdkGC     *default_gc;
        GdkGC     *paint_cursor_gc;
        GdkGC     *crosswire_gc;
	GdkPixbuf *source_pixbuf_cache;
	GdkPixbuf *scaled_pixbuf;
	GdkPixmap *pixmap;
	GdkPixmap *cursor_backing_pixels;
	GdkRectangle cursor_backing_rect;
	GdkPoint last_cursor_pos;
	GdkInterpType gdk_interp_type;
	GdkGC *border_gc;
	gulong expose_handler_id;
	gboolean test;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ZOOM_REGION_PRIVATE_H_ */
