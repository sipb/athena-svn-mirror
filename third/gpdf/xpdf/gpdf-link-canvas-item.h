/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* PDF link canvas item
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

#ifndef GPDF_LINK_CANVAS_ITEM_H
#define GPDF_LINK_CANVAS_ITEM_H

#include "gpdf-g-switch.h"
#  include <libgnomecanvas/gnome-canvas-rect-ellipse.h>
#include "gpdf-g-switch.h"
#include <Link.h>

G_BEGIN_DECLS

#define GPDF_TYPE_LINK_CANVAS_ITEM            (gpdf_link_canvas_item_get_type ())
#define GPDF_LINK_CANVAS_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_LINK_CANVAS_ITEM, GPdfLinkCanvasItem))
#define GPDF_LINK_CANVAS_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_LINK_CANVAS_ITEM, GPdfLinkCanvasItemClass))
#define GPDF_IS_LINK_CANVAS_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_LINK_CANVAS_ITEM))
#define GPDF_IS_LINK_CANVAS_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_LINK_CANVAS_ITEM))

typedef struct _GPdfLinkCanvasItem        GPdfLinkCanvasItem;
typedef struct _GPdfLinkCanvasItemClass   GPdfLinkCanvasItemClass;
typedef struct _GPdfLinkCanvasItemPrivate GPdfLinkCanvasItemPrivate;

struct _GPdfLinkCanvasItem {
	GnomeCanvasRect parent;
	
	GPdfLinkCanvasItemPrivate *priv;
};

struct _GPdfLinkCanvasItemClass {
	GnomeCanvasRectClass parent_class;

	void (*clicked) (GPdfLinkCanvasItem *link_item, Link *link);
	void (*enter)   (GPdfLinkCanvasItem *link_item, Link *link);
	void (*leave)   (GPdfLinkCanvasItem *link_item, Link *link);
};

GType gpdf_link_canvas_item_get_type (void);


/* emit signals (for easy testing) */
void  gpdf_link_canvas_item_click       (GPdfLinkCanvasItem *link_item);
void  gpdf_link_canvas_item_mouse_enter (GPdfLinkCanvasItem *link_item);
void  gpdf_link_canvas_item_mouse_leave (GPdfLinkCanvasItem *link_item);

G_END_DECLS

#endif /* GPDF_LINK_CANVAS_ITEM_H */
