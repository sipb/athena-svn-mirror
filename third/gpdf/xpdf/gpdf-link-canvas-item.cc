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

#include <aconf.h>
#include "gpdf-util.h"
#include "gpdf-link-canvas-item.h"
#include "gpdf-g-switch.h"
#  include <glib/gi18n.h>
#  include <libgnome/gnome-macros.h>
#  include "gpdf-marshal.h"
#include "gpdf-g-switch.h"
#include "Link.h"

BEGIN_EXTERN_C

struct _GPdfLinkCanvasItemPrivate {
	Link *link;
	GdkWindow *window;
	gboolean using_hand_cursor;
};

enum {
	CLICKED,
	ENTER,
	LEAVE,
	LAST_SIGNAL
};

static guint gpdf_link_canvas_item_signals [LAST_SIGNAL];

enum {
	PROP_0,
	PROP_LINK,
	PROP_USING_HAND_CURSOR
};

#define PARENT_TYPE GNOME_TYPE_CANVAS_RECT
GPDF_CLASS_BOILERPLATE (GPdfLinkCanvasItem, gpdf_link_canvas_item,
			GnomeCanvasRect, PARENT_TYPE);

void
gpdf_link_canvas_item_click (GPdfLinkCanvasItem *link_item)
{
	g_signal_emit (G_OBJECT (link_item),
		       gpdf_link_canvas_item_signals [CLICKED], 0,
		       link_item->priv->link);
}

void
gpdf_link_canvas_item_mouse_enter (GPdfLinkCanvasItem *link_item)
{
	g_signal_emit (G_OBJECT (link_item),
		       gpdf_link_canvas_item_signals [ENTER], 0,
		       link_item->priv->link);
}

void
gpdf_link_canvas_item_mouse_leave (GPdfLinkCanvasItem *link_item)
{
	g_signal_emit (G_OBJECT (link_item),
		       gpdf_link_canvas_item_signals [LEAVE], 0,
		       link_item->priv->link);
}

static void
gpdf_link_canvas_item_enter (GPdfLinkCanvasItem *link_item, Link *link)
{
	if (link_item->priv->window != NULL) {
		GdkCursor *cursor;

		cursor = gdk_cursor_new_for_display (gdk_display_get_default(),
						     GDK_HAND2);
		gdk_window_set_cursor (link_item->priv->window, cursor);
		gdk_cursor_unref (cursor);
	}

	link_item->priv->using_hand_cursor = TRUE;
}

static void
gpdf_link_canvas_item_leave (GPdfLinkCanvasItem *link_item, Link *link)
{
	if (link_item->priv->window != NULL) {
		gdk_window_set_cursor (link_item->priv->window, NULL);
	}

	link_item->priv->using_hand_cursor = FALSE;
}

static void
gpdf_link_canvas_item_clicked (GPdfLinkCanvasItem *link_item, Link *link)
{
	gpdf_link_canvas_item_leave (link_item, link);
}

static int
gpdf_link_canvas_item_event (GnomeCanvasItem *item, GdkEvent *event)
{
	GPdfLinkCanvasItem *link_item;

	g_return_val_if_fail (GPDF_IS_LINK_CANVAS_ITEM (item), FALSE);

	link_item = GPDF_LINK_CANVAS_ITEM (item);
	link_item->priv->window = ((GdkEventAny *)event)->window;


	switch (event->type) {
	case GDK_ENTER_NOTIFY:
		gpdf_link_canvas_item_mouse_enter (link_item);
		return TRUE;
	case GDK_LEAVE_NOTIFY:
		gpdf_link_canvas_item_mouse_leave (link_item);
		return TRUE;
	case GDK_BUTTON_PRESS:
		if (event->button.button > 1)
			return FALSE;
		gpdf_link_canvas_item_click (link_item);
		return TRUE;
	default:
		return FALSE;
	}
}

static void
gpdf_link_canvas_item_set_link (GPdfLinkCanvasItem *link_item, Link *link)
{
	double x1, x2, y1, y2, w;

	link_item->priv->link = link;

	w = 1.0;
	link->getRect (&x1, &y1, &x2, &y2);
	g_object_set (G_OBJECT (link_item),
		      "x1", x1, "y1", y1,
		      "x2", x2, "y2", y2,
		      "width_units", w,
		      "fill_color_rgba", 0x00000000U,
		      NULL);		      
}

static void
gpdf_link_canvas_item_get_property (GObject *object, guint param_id,
				    GValue *value, GParamSpec *pspec)
{
	GPdfLinkCanvasItem *link_item;

	g_return_if_fail (GPDF_IS_LINK_CANVAS_ITEM (object));

	link_item = GPDF_LINK_CANVAS_ITEM (object);

	switch (param_id) {
	case PROP_USING_HAND_CURSOR:
		g_value_set_boolean (value, link_item->priv->using_hand_cursor);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_link_canvas_item_set_property (GObject *object, guint param_id,
				    const GValue *value, GParamSpec *pspec)
{
	GPdfLinkCanvasItem *link_item;

	g_return_if_fail (GPDF_IS_LINK_CANVAS_ITEM (object));

	link_item = GPDF_LINK_CANVAS_ITEM (object);

	switch (param_id) {
	case PROP_LINK:
		gpdf_link_canvas_item_set_link (
			link_item,
			reinterpret_cast <Link *> (
				g_value_get_pointer (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_link_canvas_item_dispose (GObject *object)
{
	GPdfLinkCanvasItem *link_item;

	g_return_if_fail (GPDF_IS_LINK_CANVAS_ITEM (object));

	link_item = GPDF_LINK_CANVAS_ITEM (object);

	/* empty */

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gpdf_link_canvas_item_finalize (GObject *object)
{
	GPdfLinkCanvasItem *link_item;

	g_return_if_fail (GPDF_IS_LINK_CANVAS_ITEM (object));

	link_item = GPDF_LINK_CANVAS_ITEM (object);

	if (link_item->priv) {
		g_free (link_item->priv);
		link_item->priv = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_link_canvas_item_class_init (GPdfLinkCanvasItemClass *klass)
{
	GObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	object_class = G_OBJECT_CLASS (klass);
	item_class = GNOME_CANVAS_ITEM_CLASS (klass);

	object_class->dispose = gpdf_link_canvas_item_dispose;
	object_class->finalize = gpdf_link_canvas_item_finalize;
	object_class->set_property = gpdf_link_canvas_item_set_property;
	object_class->get_property = gpdf_link_canvas_item_get_property;

	item_class->event = gpdf_link_canvas_item_event;

	klass->clicked = gpdf_link_canvas_item_clicked;
	klass->enter = gpdf_link_canvas_item_enter;
	klass->leave = gpdf_link_canvas_item_leave;

	g_object_class_install_property (
		object_class, PROP_LINK,
		g_param_spec_pointer ("link",
				      _("Link"),
				      _("Link"),
				      (GParamFlags)(G_PARAM_WRITABLE)));

	g_object_class_install_property (
		object_class, PROP_USING_HAND_CURSOR,
		g_param_spec_boolean ("using_hand_cursor",
				      _("UsingHandCursor"),
				      _("UsingHandCursor"),
				      FALSE,
				      (GParamFlags)(G_PARAM_READABLE)));

	gpdf_link_canvas_item_signals [CLICKED] =
		g_signal_new ("clicked",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfLinkCanvasItemClass,
					       clicked),
			      NULL, NULL,
			      gpdf_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);

	gpdf_link_canvas_item_signals [ENTER] =
		g_signal_new ("enter",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfLinkCanvasItemClass,
					       enter),
			      NULL, NULL,
			      gpdf_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);

	gpdf_link_canvas_item_signals [LEAVE] =
		g_signal_new ("leave",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfLinkCanvasItemClass,
					       leave),
			      NULL, NULL,
			      gpdf_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);
}

static void
gpdf_link_canvas_item_instance_init (GPdfLinkCanvasItem *link_item)
{
	link_item->priv = g_new0 (GPdfLinkCanvasItemPrivate, 1);
}

END_EXTERN_C
