/*
 * AT-SPI - Assistive Technology Service Provider Interface
 * (Gnome Accessibility Project; http://developer.gnome.org/projects/gap)
 *
 * Copyright 2002 Sun Microsystems Inc.
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

#ifndef MAGNIFIER_PRIVATE_H_
#define MAGNIFIER_PRIVATE_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <X11/Xlib.h>
#include <gtk/gtkwindow.h>

struct _MagnifierPrivate {
	GtkWidget *w;
	GtkWidget *canvas;
	GdkWindow *root;
	GdkDrawable *cursor;
        int cursor_default_size_x;
        int cursor_default_size_y;
	gboolean crosswire;
	GdkBitmap *cursor_mask;
 	int cursor_x;
	int cursor_y;
	int cursor_hotspot_x;
	int cursor_hotspot_y;
        gboolean use_source_cursor;
        GHashTable *cursorlist;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MAGNIFIER_PRIVATE_H_ */
