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

#ifndef ZOOM_REGION_H_
#define ZOOM_REGION_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <bonobo/bonobo-object.h>
#include "GNOME_Magnifier.h"

#define ZOOM_REGION_TYPE         (zoom_region_get_type ())
#define ZOOM_REGION(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), ZOOM_REGION_TYPE, ZoomRegion))
#define ZOOM_REGION_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), ZOOM_REGION_TYPE, ZoomRegionClass))
#define IS_ZOOM_REGION(o)       (G_TYPE_CHECK__INSTANCE_TYPE ((o), ZOOM_REGION_TYPE))
#define IS_ZOOM_REGION_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), ZOOM_REGION_TYPE))

typedef GList * (*CoalesceFunc)(GList *, int);

typedef struct _ZoomRegionPrivate ZoomRegionPrivate;

typedef struct {
        BonoboObject parent;
	BonoboPropertyBag *properties;
	gboolean invert;
	gboolean is_managed;
	gboolean cache_source;
	gchar *smoothing;
	gfloat contrast;
	gfloat xscale;
	gfloat yscale;
	gint border_size;
	guint32 border_color; /* A-RGB, 8 bits each, MSB==alpha */
	gint x_align_policy;  /* TODO: enums here */
	gint y_align_policy;
	GNOME_Magnifier_ZoomRegion_ScrollingPolicy smooth_scroll_policy;
        /* bounds of viewport, in target magnifier window coords */
	GNOME_Magnifier_RectBounds roi;
	GNOME_Magnifier_RectBounds viewport; 
	ZoomRegionPrivate *priv;
	CoalesceFunc coalesce_func;
	gint timing_iterations;
	gboolean timing_output;
	gint timing_pan_rate;
	gboolean exit_magnifier;
} ZoomRegion;

typedef struct {
        BonoboObjectClass parent_class;
        POA_GNOME_Magnifier_ZoomRegion__epv epv;
} ZoomRegionClass;

GType     zoom_region_get_type (void);
ZoomRegion *zoom_region_new     (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ZOOM_REGION_H_ */
