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

#ifndef MAGNIFIER_H_
#define MAGNIFIER_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <X11/Xlib.h>
#include <gdk/gdk.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-property-bag.h>
#include <login-helper/login-helper.h>
#include "GNOME_Magnifier.h"

#define MAGNIFIER_TYPE         (magnifier_get_type ())
#define MAGNIFIER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), MAGNIFIER_TYPE, Magnifier))
#define MAGNIFIER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), MAGNIFIER_TYPE, MagnifierClass))
#define IS_MAGNIFIER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), MAGNIFIER_TYPE))
#define IS_MAGNIFIER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), MAGNIFIER_TYPE))

#define MAGNIFIER_OAFIID "OAFIID:GNOME_Magnifier_Magnifier:0.9"

typedef struct _MagnifierPrivate MagnifierPrivate;

typedef struct {
        BonoboObject parent;
	BonoboPropertyBag *property_bag;
	GdkDisplay *source_display;
	GdkDisplay *target_display;
	int source_screen_num;
	int target_screen_num;
	GList *zoom_regions;
	gint crosswire_size;
	guint32 crosswire_color;
	gboolean crosswire_clip;
	gchar *cursor_set;
	gint cursor_size_x;
	gint cursor_size_y;
	guint32 cursor_color;
	float cursor_scale_factor;
	GNOME_Magnifier_RectBounds source_bounds;
	GNOME_Magnifier_RectBounds target_bounds;
	GNOME_Magnifier_Point cursor_hotspot;
	MagnifierPrivate *priv;
} Magnifier;

typedef struct {
        BonoboObjectClass parent_class;
        POA_GNOME_Magnifier_Magnifier__epv epv;
} MagnifierClass;

GdkDrawable *magnifier_get_cursor    (Magnifier *magnifier);
GType        magnifier_get_type      (void);
GdkWindow   *magnifier_get_root      (Magnifier *magnifier);
Magnifier   *magnifier_new           (gboolean override_redirect);
gboolean     magnifier_error_check   (void);
void         magnifier_notify_damage (Magnifier *magnifier, XRectangle *rect);
gboolean     magnifier_source_has_damage_extension (Magnifier *magnifier);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MAGNIFIER_H_ */
