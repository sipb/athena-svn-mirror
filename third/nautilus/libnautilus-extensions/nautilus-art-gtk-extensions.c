/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* nautilus-art-gtk-extensions.c - Access gtk/gdk attributes as libart rectangles.

   Copyright (C) 2000 Eazel, Inc.

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

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>

#include "nautilus-art-gtk-extensions.h"

/**
 * nautilus_irect_assign_gdk_rectangle:
 * @gdk_rectangle: The source GdkRectangle.
 *
 * Return value: An ArtIRect representation of the GdkRectangle.
 *
 * This is a very simple conversion of rectangles from the Gdk to the Libart
 * universe.  This is useful in code that does clipping (or other operations)
 * using libart and has a GdkRectangle to work with - for example expose_event()
 * in GtkWidget's. 
 */
ArtIRect
nautilus_irect_assign_gdk_rectangle (const GdkRectangle *gdk_rectangle)
{
	ArtIRect irect;

	g_return_val_if_fail (gdk_rectangle != NULL, NAUTILUS_ART_IRECT_EMPTY);

	irect.x0 = gdk_rectangle->x;
	irect.y0 = gdk_rectangle->y;
	irect.x1 = irect.x0 + (int) gdk_rectangle->width;
	irect.y1 = irect.y0 + (int) gdk_rectangle->height;

	return irect;
}

/**
 * nautilus_irect_screen_get_frame:
 *
 * Return value: An ArtIRect representing the screen frame.
 *
 */
ArtIRect
nautilus_irect_screen_get_frame (void)
{
	ArtIRect screen_frame;

	screen_frame.x0 = 0;
	screen_frame.y0 = 0;
	screen_frame.x1 = gdk_screen_width ();
	screen_frame.y1 = gdk_screen_width ();
	
	g_assert (screen_frame.x1 > 0);
	g_assert (screen_frame.y1 > 0);

	return screen_frame;
}

/**
 * nautilus_irect_gdk_window_get_bounds:
 * @gdk_window: The source GdkWindow.
 *
 * Return value: An ArtIRect representation of the given GdkWindow's geometry
 * relative to its parent in the Gdk window hierarchy.
 *
 */
ArtIRect
nautilus_irect_gdk_window_get_bounds (const GdkWindow *gdk_window)
{
	ArtIRect bounds;
	int width;
	int height;

	g_return_val_if_fail (gdk_window != NULL, NAUTILUS_ART_IRECT_EMPTY);

	gdk_window_get_position ((GdkWindow *) gdk_window, &bounds.x0, &bounds.y0);
	gdk_window_get_size ((GdkWindow *) gdk_window, &width, &height);

	bounds.x1 = bounds.x0 + width;
	bounds.y1 = bounds.y0 + height;

	return bounds;
}

/**
 * nautilus_irect_gdk_window_get_bounds:
 * @gdk_window: The source GdkWindow.
 *
 * Return value: An ArtIRect representation of the given GdkWindow's geometry
 * relative to the screen.
 *
 */
ArtIRect
nautilus_irect_gdk_window_get_screen_relative_bounds (const GdkWindow *gdk_window)
{
	ArtIRect screen_bounds;
	int width;
	int height;
	
	g_return_val_if_fail (gdk_window != NULL, NAUTILUS_ART_IRECT_EMPTY);

	if (!gdk_window_get_origin ((GdkWindow *) gdk_window,
				    &screen_bounds.x0,
				    &screen_bounds.y0)) {
		return NAUTILUS_ART_IRECT_EMPTY;
	}
	
	gdk_window_get_size ((GdkWindow *) gdk_window, &width, &height);
	
	screen_bounds.x1 = screen_bounds.x0 + width;
	screen_bounds.y1 = screen_bounds.y0 + height;
	
	return screen_bounds;
}

/**
 * nautilus_irect_gtk_widget_get_bounds:
 * @gtk_widget: The source GtkWidget.
 *
 * Return value: An ArtIRect representation of the given GtkWidget's geometry
 * relative to its parent.  In the Gtk universe this is known as "allocation."
 *
 */
ArtIRect
nautilus_irect_gtk_widget_get_bounds (const GtkWidget *gtk_widget)
{
	ArtIRect bounds;

	g_return_val_if_fail (GTK_IS_WIDGET (gtk_widget), NAUTILUS_ART_IRECT_EMPTY);
	
	nautilus_art_irect_assign (&bounds, 
				   gtk_widget->allocation.x,
				   gtk_widget->allocation.y,
				   (int) gtk_widget->allocation.width,
				   (int) gtk_widget->allocation.height);
	
	return bounds;
}

/**
 * nautilus_irect_gtk_widget_get_frame:
 * @gtk_widget: The source GtkWidget.
 *
 * Return value: An ArtIRect representation of the given GtkWidget's dimensions.
 *
 */
ArtIRect
nautilus_irect_gtk_widget_get_frame (const GtkWidget *gtk_widget)
{
	ArtIRect frame;
	
	g_return_val_if_fail (GTK_IS_WIDGET (gtk_widget), NAUTILUS_ART_IRECT_EMPTY);
	
	nautilus_art_irect_assign (&frame, 
				   0,
				   0,
				   (int) gtk_widget->allocation.width,
				   (int) gtk_widget->allocation.height);
	
	return frame;
}

/**
 * nautilus_irect_gdk_window_clip_dirty_area_to_screen:
 * @gdk_window: The GdkWindow that the damage occured on.
 * @dirty_area: The dirty area as an ArtIRect.
 *
 * Return value: An ArtIRect of the dirty area clipped to the screen.
 *
 * This function is useful to do less work in expose_event() GtkWidget methods.
 * It also ensures that any drawing that the widget does is actually onscreen.
 */
ArtIRect
nautilus_irect_gdk_window_clip_dirty_area_to_screen (const GdkWindow *gdk_window,
						     const ArtIRect *dirty_area)
{
	ArtIRect clipped;
	ArtIRect screen_frame;
	ArtIRect bounds;
	ArtIRect screen_relative_bounds;
	int dirty_width;
	int dirty_height;

	g_return_val_if_fail (gdk_window != NULL, NAUTILUS_ART_IRECT_EMPTY);
	g_return_val_if_fail (dirty_area != NULL, NAUTILUS_ART_IRECT_EMPTY);

	dirty_width = dirty_area->x1 - dirty_area->x0;
	dirty_height = dirty_area->y1 - dirty_area->y0;

	g_return_val_if_fail (dirty_width > 0, NAUTILUS_ART_IRECT_EMPTY);
	g_return_val_if_fail (dirty_height > 0, NAUTILUS_ART_IRECT_EMPTY);

	screen_frame = nautilus_irect_screen_get_frame ();
	bounds = nautilus_irect_gdk_window_get_bounds (gdk_window);
	screen_relative_bounds = nautilus_irect_gdk_window_get_screen_relative_bounds (gdk_window);
	
	/* Window is obscured by left edge of screen */
	if ((screen_relative_bounds.x0 + dirty_area->x0) < 0) {
		int clipped_width = screen_relative_bounds.x0 + dirty_area->x0 + dirty_width;
		clipped.x0 = dirty_area->x0 + dirty_width - clipped_width;
		clipped.x1 = clipped.x0 + clipped_width;
	} else {
		clipped.x0 = dirty_area->x0;
		clipped.x1 = clipped.x0 + dirty_width;
	}
	
	/* Window is obscured by right edge of screen */
	if (screen_relative_bounds.x1 > screen_frame.x1) {
 		int obscured_width;
		
		obscured_width = 
			screen_relative_bounds.x0 + dirty_area->x0 + dirty_width - screen_frame.x1;
		
		if (obscured_width > 0) {
			clipped.x1 -= obscured_width;
		}
	}

	/* Window is obscured by top edge of screen */
	if ((screen_relative_bounds.y0 + dirty_area->y0) < 0) {
		int clipped_height = screen_relative_bounds.y0 + dirty_area->y0 + dirty_height;
		clipped.y0 = dirty_area->y0 + dirty_height - clipped_height;
		clipped.y1 = clipped.y0 + clipped_height;
	} else {
		clipped.y0 = dirty_area->y0;
		clipped.y1 = clipped.y0 + dirty_height;
	}
	
	/* Window is obscured by bottom edge of screen */
	if (screen_relative_bounds.y1 > screen_frame.y1) {
 		int obscured_height;
		
		obscured_height = 
			screen_relative_bounds.y0 + dirty_area->y0 + dirty_height - screen_frame.y1;
		
		if (obscured_height > 0) {
			clipped.y1 -= obscured_height;
		}
	}

	if (art_irect_empty (&clipped)) {
		clipped = NAUTILUS_ART_IRECT_EMPTY;
	}

	return clipped;
}
