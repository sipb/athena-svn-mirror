/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* PDF links canvas layer
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GPDF_LINKS_CANVAS_LAYER_H
#define GPDF_LINKS_CANVAS_LAYER_H

#include "gpdf-g-switch.h"
#  include <libgnomecanvas/gnome-canvas.h>
#include "gpdf-g-switch.h"
#include "Link.h"

G_BEGIN_DECLS

#define GPDF_TYPE_LINKS_CANVAS_LAYER            (gpdf_links_canvas_layer_get_type ())
#define GPDF_LINKS_CANVAS_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_LINKS_CANVAS_LAYER, GPdfLinksCanvasLayer))
#define GPDF_LINKS_CANVAS_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_LINKS_CANVAS_LAYER, GPdfLinksCanvasLayerClass))
#define GPDF_IS_LINKS_CANVAS_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_LINKS_CANVAS_LAYER))
#define GPDF_IS_LINKS_CANVAS_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_LINKS_CANVAS_LAYER))

typedef struct _GPdfLinksCanvasLayer        GPdfLinksCanvasLayer;
typedef struct _GPdfLinksCanvasLayerClass   GPdfLinksCanvasLayerClass;
typedef struct _GPdfLinksCanvasLayerPrivate GPdfLinksCanvasLayerPrivate;

struct _GPdfLinksCanvasLayer {
	GnomeCanvasGroup parent;
	
	GPdfLinksCanvasLayerPrivate *priv;
};

struct _GPdfLinksCanvasLayerClass {
	GnomeCanvasGroupClass parent_class;

	void (*link_clicked) (GPdfLinksCanvasLayer *links_layer, Link *link);

	void (*link_entered) (GPdfLinksCanvasLayer *links_layer, Link *link);

	void (*link_leaved) (GPdfLinksCanvasLayer *links_layer, Link *link);
};

GType gpdf_links_canvas_layer_get_type      (void);
int   gpdf_links_canvas_layer_get_num_links (GPdfLinksCanvasLayer *links_layer);

G_END_DECLS

#endif /* GPDF_LINKS_CANVAS_LAYER_H */
