/* -*- Mode: C; tab-width: 8; indent-tabs-mode: 8; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*
  @NOTATION@
 */
/*
 * EelCanvas widget - Tk-like canvas widget for Gnome
 *
 * EelCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@gimp.org>
 */

/*
 * TO-DO list for the canvas:
 *
 * - Allow to specify whether EelCanvasImage sizes are in units or pixels (scale or don't scale).
 *
 * - Implement a flag for eel_canvas_item_reparent() that tells the function to keep the item
 *   visually in the same place, that is, to keep it in the same place with respect to the canvas
 *   origin.
 *
 * - GC put functions for items.
 *
 * - Widget item (finish it).
 *
 * - GList *eel_canvas_gimme_all_items_contained_in_this_area (EelCanvas *canvas, Rectangle area);
 *
 * - Retrofit all the primitive items with microtile support.
 *
 * - Curve support for line item.
 *
 * - Arc item (Havoc has it; to be integrated in EelCanvasEllipse).
 *
 * - Sane font handling API.
 *
 * - Get_arg methods for items:
 *   - How to fetch the outline width and know whether it is in pixels or units?
 */

#include <config.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <gdk/gdkprivate.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include "eel-canvas.h"
#include "eel-i18n.h"

#include "eel-marshal.h"



static void eel_canvas_request_update (EelCanvas      *canvas);
static void group_add                   (EelCanvasGroup *group,
					 EelCanvasItem  *item);
static void group_remove                (EelCanvasGroup *group,
					 EelCanvasItem  *item);
static void
eel_canvas_group_child_bounds (EelCanvasGroup *group, EelCanvasItem *item);


/*** EelCanvasItem ***/

/* Some convenience stuff */
#define GCI_UPDATE_MASK (EEL_CANVAS_UPDATE_REQUESTED | EEL_CANVAS_UPDATE_DEEP)
#define GCI_EPSILON 1e-18

enum {
	ITEM_PROP_0,
	ITEM_PROP_PARENT
};

enum {
	ITEM_EVENT,
	ITEM_LAST_SIGNAL
};

static void eel_canvas_item_class_init     (EelCanvasItemClass *class);
static void eel_canvas_item_init           (EelCanvasItem      *item);
static int  emit_event                       (EelCanvas *canvas, GdkEvent *event);

static guint item_signals[ITEM_LAST_SIGNAL];

static GtkObjectClass *item_parent_class;


/**
 * eel_canvas_item_get_type:
 *
 * Registers the &EelCanvasItem class if necessary, and returns the type ID
 * associated to it.
 *
 * Return value:  The type ID of the &EelCanvasItem class.
 **/
GType
eel_canvas_item_get_type (void)
{
	static GType canvas_item_type = 0;

	if (!canvas_item_type) {
		static const GTypeInfo canvas_item_info = {
			sizeof (EelCanvasItemClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) eel_canvas_item_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (EelCanvasItem),
			0,              /* n_preallocs */
			(GInstanceInitFunc) eel_canvas_item_init
		};

		canvas_item_type = g_type_register_static (gtk_object_get_type (),
							   "EelCanvasItem",
							   &canvas_item_info,
							   0);
	}

	return canvas_item_type;
}

/* Object initialization function for EelCanvasItem */
static void
eel_canvas_item_init (EelCanvasItem *item)
{
	item->object.flags |= EEL_CANVAS_ITEM_VISIBLE;
}

/**
 * eel_canvas_item_new:
 * @parent: The parent group for the new item.
 * @type: The object type of the item.
 * @first_arg_name: A list of object argument name/value pairs, NULL-terminated,
 * used to configure the item.  For example, "fill_color", "black",
 * "width_units", 5.0, NULL.
 * @Varargs:
 *
 * Creates a new canvas item with @parent as its parent group.  The item is
 * created at the top of its parent's stack, and starts up as visible.  The item
 * is of the specified @type, for example, it can be
 * eel_canvas_rect_get_type().  The list of object arguments/value pairs is
 * used to configure the item.
 *
 * Return value: The newly-created item.
 **/
EelCanvasItem *
eel_canvas_item_new (EelCanvasGroup *parent, GType type, const gchar *first_arg_name, ...)
{
	EelCanvasItem *item;
	va_list args;

	g_return_val_if_fail (EEL_IS_CANVAS_GROUP (parent), NULL);
	g_return_val_if_fail (g_type_is_a (type, eel_canvas_item_get_type ()), NULL);

	item = EEL_CANVAS_ITEM (g_object_new (type, NULL));

	va_start (args, first_arg_name);
	eel_canvas_item_construct (item, parent, first_arg_name, args);
	va_end (args);

	return item;
}


/* Performs post-creation operations on a canvas item (adding it to its parent
 * group, etc.)
 */
static void
item_post_create_setup (EelCanvasItem *item)
{
	GtkObject *obj;

	obj = GTK_OBJECT (item);

	group_add (EEL_CANVAS_GROUP (item->parent), item);

	eel_canvas_item_request_redraw (item);
	item->canvas->need_repick = TRUE;
}

/* Set_property handler for canvas items */
static void
eel_canvas_item_set_property (GObject *gobject, guint param_id,
				const GValue *value, GParamSpec *pspec)
{
	EelCanvasItem *item;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (gobject));

	item = EEL_CANVAS_ITEM (gobject);

	switch (param_id) {
	case ITEM_PROP_PARENT:
		if (item->parent != NULL) {
		    g_warning ("Cannot set `parent' argument after item has "
			       "already been constructed.");
		} else if (g_value_get_object (value)) {
			item->parent = EEL_CANVAS_ITEM (g_value_get_object (value));
			item->canvas = item->parent->canvas;
			item_post_create_setup (item);
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, param_id, pspec);
		break;
	}
}

/* Get_property handler for canvas items */
static void
eel_canvas_item_get_property (GObject *gobject, guint param_id,
				GValue *value, GParamSpec *pspec)
{
	EelCanvasItem *item;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (gobject));

	item = EEL_CANVAS_ITEM (gobject);

	switch (param_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, param_id, pspec);
		break;
	}
}

/**
 * eel_canvas_item_construct:
 * @item: An unconstructed canvas item.
 * @parent: The parent group for the item.
 * @first_arg_name: The name of the first argument for configuring the item.
 * @args: The list of arguments used to configure the item.
 *
 * Constructs a canvas item; meant for use only by item implementations.
 **/
void
eel_canvas_item_construct (EelCanvasItem *item, EelCanvasGroup *parent,
			     const gchar *first_arg_name, va_list args)
{
	g_return_if_fail (EEL_IS_CANVAS_GROUP (parent));
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	item->parent = EEL_CANVAS_ITEM (parent);
	item->canvas = item->parent->canvas;

	g_object_set_valist (G_OBJECT (item), first_arg_name, args);

	item_post_create_setup (item);
}


/* If the item is visible, requests a redraw of it. */
static void
redraw_if_visible (EelCanvasItem *item)
{
	if (item->object.flags & EEL_CANVAS_ITEM_VISIBLE)
		eel_canvas_item_request_redraw (item);
}

/* Standard object dispose function for canvas items */
static void
eel_canvas_item_dispose (GObject *object)
{
	EelCanvasItem *item;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (object));

	item = EEL_CANVAS_ITEM (object);

	redraw_if_visible (item);

	/* Make the canvas forget about us */

	if (item == item->canvas->current_item) {
		item->canvas->current_item = NULL;
		item->canvas->need_repick = TRUE;
	}

	if (item == item->canvas->new_current_item) {
		item->canvas->new_current_item = NULL;
		item->canvas->need_repick = TRUE;
	}

	if (item == item->canvas->grabbed_item) {
		item->canvas->grabbed_item = NULL;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
	}

	if (item == item->canvas->focused_item)
		item->canvas->focused_item = NULL;

	/* Normal destroy stuff */

	if (item->object.flags & EEL_CANVAS_ITEM_MAPPED)
		(* EEL_CANVAS_ITEM_GET_CLASS (item)->unmap) (item);

	if (item->object.flags & EEL_CANVAS_ITEM_REALIZED)
		(* EEL_CANVAS_ITEM_GET_CLASS (item)->unrealize) (item);

	if (item->parent)
		group_remove (EEL_CANVAS_GROUP (item->parent), item);

	G_OBJECT_CLASS (item_parent_class)->dispose (object);
}

/* Realize handler for canvas items */
static void
eel_canvas_item_realize (EelCanvasItem *item)
{
	GTK_OBJECT_SET_FLAGS (item, EEL_CANVAS_ITEM_REALIZED);

	eel_canvas_item_request_update (item);
}

/* Unrealize handler for canvas items */
static void
eel_canvas_item_unrealize (EelCanvasItem *item)
{
	GTK_OBJECT_UNSET_FLAGS (item, EEL_CANVAS_ITEM_REALIZED);
}

/* Map handler for canvas items */
static void
eel_canvas_item_map (EelCanvasItem *item)
{
	GTK_OBJECT_SET_FLAGS (item, EEL_CANVAS_ITEM_MAPPED);
}

/* Unmap handler for canvas items */
static void
eel_canvas_item_unmap (EelCanvasItem *item)
{
	GTK_OBJECT_UNSET_FLAGS (item, EEL_CANVAS_ITEM_MAPPED);
}

/* Update handler for canvas items */
static void
eel_canvas_item_update (EelCanvasItem *item, double i2w_dx, double i2w_dy, int flags)
{
	GTK_OBJECT_UNSET_FLAGS (item, EEL_CANVAS_ITEM_NEED_UPDATE);
	GTK_OBJECT_UNSET_FLAGS (item, EEL_CANVAS_ITEM_NEED_DEEP_UPDATE);
}

#define noHACKISH_AFFINE

/*
 * This routine invokes the update method of the item
 * Please notice, that we take parent to canvas pixel matrix as argument
 * unlike virtual method ::update, whose argument is item 2 canvas pixel
 * matrix
 *
 * I will try to force somewhat meaningful naming for affines (Lauris)
 * General naming rule is FROM2TO, where FROM and TO are abbreviations
 * So p2cpx is Parent2CanvasPixel and i2cpx is Item2CanvasPixel
 * I hope that this helps to keep track of what really happens
 *
 */

static void
eel_canvas_item_invoke_update (EelCanvasItem *item,
				 double i2w_dx,
				 double i2w_dy,
				 int flags)
{
	int child_flags;

	child_flags = flags;

	/* apply object flags to child flags */
	child_flags &= ~EEL_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & EEL_CANVAS_ITEM_NEED_UPDATE)
		child_flags |= EEL_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & EEL_CANVAS_ITEM_NEED_DEEP_UPDATE)
		child_flags |= EEL_CANVAS_UPDATE_DEEP;

	if (child_flags & GCI_UPDATE_MASK) {
		if (EEL_CANVAS_ITEM_GET_CLASS (item)->update)
			EEL_CANVAS_ITEM_GET_CLASS (item)->update (item, i2w_dx, i2w_dy, child_flags);
	}
}

/*
 * This routine invokes the point method of the item.
 * The arguments x, y should be in the parent item local coordinates.
 *
 * This is potentially evil, as we are relying on matrix inversion (Lauris)
 */

static double
eel_canvas_item_invoke_point (EelCanvasItem *item, double x, double y, int cx, int cy, EelCanvasItem **actual_item)
{
	/* Calculate x & y in item local coordinates */

	if (EEL_CANVAS_ITEM_GET_CLASS (item)->point)
		return EEL_CANVAS_ITEM_GET_CLASS (item)->point (item, x, y, cx, cy, actual_item);

	return 1e18;
}

/**
 * eel_canvas_item_set:
 * @item: A canvas item.
 * @first_arg_name: The list of object argument name/value pairs used to configure the item.
 * @Varargs:
 *
 * Configures a canvas item.  The arguments in the item are set to the specified
 * values, and the item is repainted as appropriate.
 **/
void
eel_canvas_item_set (EelCanvasItem *item, const gchar *first_arg_name, ...)
{
	va_list args;

	va_start (args, first_arg_name);
	eel_canvas_item_set_valist (item, first_arg_name, args);
	va_end (args);
}


/**
 * eel_canvas_item_set_valist:
 * @item: A canvas item.
 * @first_arg_name: The name of the first argument used to configure the item.
 * @args: The list of object argument name/value pairs used to configure the item.
 *
 * Configures a canvas item.  The arguments in the item are set to the specified
 * values, and the item is repainted as appropriate.
 **/
void
eel_canvas_item_set_valist (EelCanvasItem *item, const gchar *first_arg_name, va_list args)
{
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	g_object_set_valist (G_OBJECT (item), first_arg_name, args);

#if 0
	/* I commented this out, because item implementations have to schedule update/redraw */
	redraw_if_visible (item);
#endif

	item->canvas->need_repick = TRUE;
}


/**
 * eel_canvas_item_move:
 * @item: A canvas item.
 * @dx: Horizontal offset.
 * @dy: Vertical offset.
 *
 * Moves a canvas item by creating an affine transformation matrix for
 * translation by using the specified values. This happens in item
 * local coordinate system, so if you have nontrivial transform, it
 * most probably does not do, what you want.
 **/
void
eel_canvas_item_move (EelCanvasItem *item, double dx, double dy)
{
        g_return_if_fail (item != NULL);
        g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

        if (!EEL_CANVAS_ITEM_GET_CLASS (item)->translate) {
                g_warning ("Item type %s does not implement translate method.\n",
                           g_type_name (GTK_OBJECT_TYPE (item)));
                return;
        }

        (* EEL_CANVAS_ITEM_GET_CLASS (item)->translate) (item, dx, dy);

        item->canvas->need_repick = TRUE;

	if (!(item->object.flags & EEL_CANVAS_ITEM_NEED_DEEP_UPDATE)) {
		item->object.flags |= EEL_CANVAS_ITEM_NEED_DEEP_UPDATE;
		if (item->parent != NULL)
			eel_canvas_item_request_update (item->parent);
		else
			eel_canvas_request_update (item->canvas);
	}

}

/* Convenience function to reorder items in a group's child list.  This puts the
 * specified link after the "before" link. Returns TRUE if the list was changed.
 */
static gboolean
put_item_after (GList *link, GList *before)
{
	EelCanvasGroup *parent;

	if (link == before)
		return FALSE;

	parent = EEL_CANVAS_GROUP (EEL_CANVAS_ITEM (link->data)->parent);

	if (before == NULL) {
		if (link == parent->item_list)
			return FALSE;

		link->prev->next = link->next;

		if (link->next)
			link->next->prev = link->prev;
		else
			parent->item_list_end = link->prev;

		link->prev = before;
		link->next = parent->item_list;
		link->next->prev = link;
		parent->item_list = link;
	} else {
		if ((link == parent->item_list_end) && (before == parent->item_list_end->prev))
			return FALSE;

		if (link->next)
			link->next->prev = link->prev;

		if (link->prev)
			link->prev->next = link->next;
		else {
			parent->item_list = link->next;
			parent->item_list->prev = NULL;
		}

		link->prev = before;
		link->next = before->next;

		link->prev->next = link;

		if (link->next)
			link->next->prev = link;
		else
			parent->item_list_end = link;
	}
	return TRUE;
}


/**
 * eel_canvas_item_raise:
 * @item: A canvas item.
 * @positions: Number of steps to raise the item.
 *
 * Raises the item in its parent's stack by the specified number of positions.
 * If the number of positions is greater than the distance to the top of the
 * stack, then the item is put at the top.
 **/
void
eel_canvas_item_raise (EelCanvasItem *item, int positions)
{
	GList *link, *before;
	EelCanvasGroup *parent;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));
	g_return_if_fail (positions >= 0);

	if (!item->parent || positions == 0)
		return;

	parent = EEL_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->item_list, item);
	g_assert (link != NULL);

	for (before = link; positions && before; positions--)
		before = before->next;

	if (!before)
		before = parent->item_list_end;

	if (put_item_after (link, before)) {
		redraw_if_visible (item);
		item->canvas->need_repick = TRUE;
	}
}


/**
 * eel_canvas_item_lower:
 * @item: A canvas item.
 * @positions: Number of steps to lower the item.
 *
 * Lowers the item in its parent's stack by the specified number of positions.
 * If the number of positions is greater than the distance to the bottom of the
 * stack, then the item is put at the bottom.
 **/
void
eel_canvas_item_lower (EelCanvasItem *item, int positions)
{
	GList *link, *before;
	EelCanvasGroup *parent;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));
	g_return_if_fail (positions >= 1);

	if (!item->parent || positions == 0)
		return;

	parent = EEL_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->item_list, item);
	g_assert (link != NULL);

	if (link->prev)
		for (before = link->prev; positions && before; positions--)
			before = before->prev;
	else
		before = NULL;

	if (put_item_after (link, before)) {
		redraw_if_visible (item);
		item->canvas->need_repick = TRUE;
	}
}


/**
 * eel_canvas_item_raise_to_top:
 * @item: A canvas item.
 *
 * Raises an item to the top of its parent's stack.
 **/
void
eel_canvas_item_raise_to_top (EelCanvasItem *item)
{
	GList *link;
	EelCanvasGroup *parent;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	if (!item->parent)
		return;

	parent = EEL_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->item_list, item);
	g_assert (link != NULL);

	if (put_item_after (link, parent->item_list_end)) {
		redraw_if_visible (item);
		item->canvas->need_repick = TRUE;
	}
}


/**
 * eel_canvas_item_lower_to_bottom:
 * @item: A canvas item.
 *
 * Lowers an item to the bottom of its parent's stack.
 **/
void
eel_canvas_item_lower_to_bottom (EelCanvasItem *item)
{
	GList *link;
	EelCanvasGroup *parent;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	if (!item->parent)
		return;

	parent = EEL_CANVAS_GROUP (item->parent);
	link = g_list_find (parent->item_list, item);
	g_assert (link != NULL);

	if (put_item_after (link, NULL)) {
		redraw_if_visible (item);
		item->canvas->need_repick = TRUE;
	}
}

/**
 * eel_canvas_item_send_behind:
 * @item: A canvas item.
 * @behind_item: The canvas item to put item behind, or NULL
 *
 * Moves item to a in the position in the stacking order so that
 * it is placed immediately below behind_item, or at the top if
 * behind_item is NULL.
 **/
void
eel_canvas_item_send_behind (EelCanvasItem *item,
			     EelCanvasItem *behind_item)
{
	GList *item_list;
	int item_position, behind_position;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	if (behind_item == NULL) {
		eel_canvas_item_raise_to_top (item);
		return;
	}

	g_return_if_fail (EEL_IS_CANVAS_ITEM (behind_item));
	g_return_if_fail (item->parent == behind_item->parent);

	item_list = EEL_CANVAS_GROUP (item->parent)->item_list;

	item_position = g_list_index (item_list, item);
	g_assert (item_position != -1);
	behind_position = g_list_index (item_list, behind_item);
	g_assert (behind_position != -1);
	g_assert (item_position != behind_position);

	if (item_position == behind_position - 1) {
		return;
	}

	if (item_position < behind_position) {
		eel_canvas_item_raise (item, (behind_position - 1) - item_position);
	} else {
		eel_canvas_item_lower (item, item_position - behind_position);
	}
}

/**
 * eel_canvas_item_show:
 * @item: A canvas item.
 *
 * Shows a canvas item.  If the item was already shown, then no action is taken.
 **/
void
eel_canvas_item_show (EelCanvasItem *item)
{
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	if (!(item->object.flags & EEL_CANVAS_ITEM_VISIBLE)) {
		item->object.flags |= EEL_CANVAS_ITEM_VISIBLE;
		eel_canvas_item_request_redraw (item);
		item->canvas->need_repick = TRUE;
	}
}


/**
 * eel_canvas_item_hide:
 * @item: A canvas item.
 *
 * Hides a canvas item.  If the item was already hidden, then no action is
 * taken.
 **/
void
eel_canvas_item_hide (EelCanvasItem *item)
{
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	if (item->object.flags & EEL_CANVAS_ITEM_VISIBLE) {
		item->object.flags &= ~EEL_CANVAS_ITEM_VISIBLE;
		eel_canvas_item_request_redraw (item);
		item->canvas->need_repick = TRUE;
	}
}


/**
 * eel_canvas_item_grab:
 * @item: A canvas item.
 * @event_mask: Mask of events that will be sent to this item.
 * @cursor: If non-NULL, the cursor that will be used while the grab is active.
 * @etime: The timestamp required for grabbing the mouse, or GDK_CURRENT_TIME.
 *
 * Specifies that all events that match the specified event mask should be sent
 * to the specified item, and also grabs the mouse by calling
 * gdk_pointer_grab().  The event mask is also used when grabbing the pointer.
 * If @cursor is not NULL, then that cursor is used while the grab is active.
 * The @etime parameter is the timestamp required for grabbing the mouse.
 *
 * Return value: If an item was already grabbed, it returns %GDK_GRAB_ALREADY_GRABBED.  If
 * the specified item was hidden by calling eel_canvas_item_hide(), then it
 * returns %GDK_GRAB_NOT_VIEWABLE.  Else, it returns the result of calling
 * gdk_pointer_grab().
 **/
int
eel_canvas_item_grab (EelCanvasItem *item, guint event_mask, GdkCursor *cursor, guint32 etime)
{
	int retval;

	g_return_val_if_fail (EEL_IS_CANVAS_ITEM (item), GDK_GRAB_NOT_VIEWABLE);
	g_return_val_if_fail (GTK_WIDGET_MAPPED (item->canvas), GDK_GRAB_NOT_VIEWABLE);

	if (item->canvas->grabbed_item)
		return GDK_GRAB_ALREADY_GRABBED;

	if (!(item->object.flags & EEL_CANVAS_ITEM_VISIBLE))
		return GDK_GRAB_NOT_VIEWABLE;

	retval = gdk_pointer_grab (item->canvas->layout.bin_window,
				   FALSE,
				   event_mask,
				   NULL,
				   cursor,
				   etime);

	if (retval != GDK_GRAB_SUCCESS)
		return retval;

	item->canvas->grabbed_item = item;
	item->canvas->grabbed_event_mask = event_mask;
	item->canvas->current_item = item; /* So that events go to the grabbed item */

	return retval;
}


/**
 * eel_canvas_item_ungrab:
 * @item: A canvas item that holds a grab.
 * @etime: The timestamp for ungrabbing the mouse.
 *
 * Ungrabs the item, which must have been grabbed in the canvas, and ungrabs the
 * mouse.
 **/
void
eel_canvas_item_ungrab (EelCanvasItem *item, guint32 etime)
{
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	if (item->canvas->grabbed_item != item)
		return;

	item->canvas->grabbed_item = NULL;

	gdk_pointer_ungrab (etime);
}


/**
 * eel_canvas_item_w2i:
 * @item: A canvas item.
 * @x: X coordinate to convert (input/output value).
 * @y: Y coordinate to convert (input/output value).
 *
 * Converts a coordinate pair from world coordinates to item-relative
 * coordinates.
 **/
void
eel_canvas_item_w2i (EelCanvasItem *item, double *x, double *y)
{
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));
	g_return_if_fail (x != NULL);
	g_return_if_fail (y != NULL);

	item = item->parent;
	while (item) {
		if (EEL_IS_CANVAS_GROUP (item)) {
			*x -= EEL_CANVAS_GROUP (item)->xpos;
			*y -= EEL_CANVAS_GROUP (item)->ypos;
		}

		item = item->parent;
	}
}


/**
 * eel_canvas_item_i2w:
 * @item: A canvas item.
 * @x: X coordinate to convert (input/output value).
 * @y: Y coordinate to convert (input/output value).
 *
 * Converts a coordinate pair from item-relative coordinates to world
 * coordinates.
 **/
void
eel_canvas_item_i2w (EelCanvasItem *item, double *x, double *y)
{
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));
	g_return_if_fail (x != NULL);
	g_return_if_fail (y != NULL);

	item = item->parent;
	while (item) {
		if (EEL_IS_CANVAS_GROUP (item)) {
			*x += EEL_CANVAS_GROUP (item)->xpos;
			*y += EEL_CANVAS_GROUP (item)->ypos;
		}

		item = item->parent;
	}
}

/* Returns whether the item is an inferior of or is equal to the parent. */
static int
is_descendant (EelCanvasItem *item, EelCanvasItem *parent)
{
	for (; item; item = item->parent)
		if (item == parent)
			return TRUE;

	return FALSE;
}

/**
 * eel_canvas_item_reparent:
 * @item: A canvas item.
 * @new_group: A canvas group.
 *
 * Changes the parent of the specified item to be the new group.  The item keeps
 * its group-relative coordinates as for its old parent, so the item may change
 * its absolute position within the canvas.
 **/
void
eel_canvas_item_reparent (EelCanvasItem *item, EelCanvasGroup *new_group)
{
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));
	g_return_if_fail (EEL_IS_CANVAS_GROUP (new_group));

	/* Both items need to be in the same canvas */
	g_return_if_fail (item->canvas == EEL_CANVAS_ITEM (new_group)->canvas);

	/* The group cannot be an inferior of the item or be the item itself --
	 * this also takes care of the case where the item is the root item of
	 * the canvas.  */
	g_return_if_fail (!is_descendant (EEL_CANVAS_ITEM (new_group), item));

	/* Everything is ok, now actually reparent the item */

	g_object_ref (GTK_OBJECT (item)); /* protect it from the unref in group_remove */

	redraw_if_visible (item);

	group_remove (EEL_CANVAS_GROUP (item->parent), item);
	item->parent = EEL_CANVAS_ITEM (new_group);
	group_add (new_group, item);

	/* Redraw and repick */

	redraw_if_visible (item);
	item->canvas->need_repick = TRUE;

	g_object_unref (GTK_OBJECT (item));
}

/**
 * eel_canvas_item_grab_focus:
 * @item: A canvas item.
 *
 * Makes the specified item take the keyboard focus, so all keyboard events will
 * be sent to it.  If the canvas widget itself did not have the focus, it grabs
 * it as well.
 **/
void
eel_canvas_item_grab_focus (EelCanvasItem *item)
{
	EelCanvasItem *focused_item;
	GdkEvent ev;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));
	g_return_if_fail (GTK_WIDGET_CAN_FOCUS (GTK_WIDGET (item->canvas)));

	focused_item = item->canvas->focused_item;

	if (focused_item) {
		ev.focus_change.type = GDK_FOCUS_CHANGE;
		ev.focus_change.window = GTK_LAYOUT (item->canvas)->bin_window;
		ev.focus_change.send_event = FALSE;
		ev.focus_change.in = FALSE;

		emit_event (item->canvas, &ev);
	}

	item->canvas->focused_item = item;
	gtk_widget_grab_focus (GTK_WIDGET (item->canvas));

	if (focused_item) {                                                     
		ev.focus_change.type = GDK_FOCUS_CHANGE;                        
		ev.focus_change.window = GTK_LAYOUT (item->canvas)->bin_window;
		ev.focus_change.send_event = FALSE;                             
		ev.focus_change.in = TRUE;                                      

		emit_event (item->canvas, &ev);                          
	}                               
}


/**
 * eel_canvas_item_get_bounds:
 * @item: A canvas item.
 * @x1: Leftmost edge of the bounding box (return value).
 * @y1: Upper edge of the bounding box (return value).
 * @x2: Rightmost edge of the bounding box (return value).
 * @y2: Lower edge of the bounding box (return value).
 *
 * Queries the bounding box of a canvas item.  The bounds are returned in the
 * coordinate system of the item's parent.
 **/
void
eel_canvas_item_get_bounds (EelCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	double tx1, ty1, tx2, ty2;

	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	tx1 = ty1 = tx2 = ty2 = 0.0;

	/* Get the item's bounds in its coordinate system */

	if (EEL_CANVAS_ITEM_GET_CLASS (item)->bounds)
		(* EEL_CANVAS_ITEM_GET_CLASS (item)->bounds) (item, &tx1, &ty1, &tx2, &ty2);

	/* Return the values */

	if (x1)
		*x1 = tx1;

	if (y1)
		*y1 = ty1;

	if (x2)
		*x2 = tx2;

	if (y2)
		*y2 = ty2;
}


/**
 * eel_canvas_item_request_update
 * @item: A canvas item.
 *
 * To be used only by item implementations.  Requests that the canvas queue an
 * update for the specified item.
 **/
void
eel_canvas_item_request_update (EelCanvasItem *item)
{
	if (item->object.flags & EEL_CANVAS_ITEM_NEED_UPDATE)
		return;

	item->object.flags |= EEL_CANVAS_ITEM_NEED_UPDATE;

	if (item->parent != NULL) {
		/* Recurse up the tree */
		eel_canvas_item_request_update (item->parent);
	} else {
		/* Have reached the top of the tree, make sure the update call gets scheduled. */
		eel_canvas_request_update (item->canvas);
	}
}

/**
 * eel_canvas_item_request_update
 * @item: A canvas item.
 *
 * Convenience function that informs a canvas that the specified item needs
 * to be repainted. To be used by item implementations
 **/
void
eel_canvas_item_request_redraw (EelCanvasItem *item)
{
	eel_canvas_request_redraw (item->canvas,
				   item->x1, item->y1,
				   item->x2 + 1, item->y2 + 1);
}



/*** EelCanvasGroup ***/


enum {
	GROUP_PROP_0,
	GROUP_PROP_X,
	GROUP_PROP_Y
};


static void eel_canvas_group_class_init  (EelCanvasGroupClass *class);
static void eel_canvas_group_init        (EelCanvasGroup      *group);
static void eel_canvas_group_set_property(GObject               *object, 
					    guint                  param_id,
					    const GValue          *value,
					    GParamSpec            *pspec);
static void eel_canvas_group_get_property(GObject               *object,
					    guint                  param_id,
					    GValue                *value,
					    GParamSpec            *pspec);

static void eel_canvas_group_destroy     (GtkObject             *object);

static void   eel_canvas_group_update      (EelCanvasItem *item,
					      double           i2w_dx,
					      double           i2w_dy,
					      int              flags);
static void   eel_canvas_group_realize     (EelCanvasItem *item);
static void   eel_canvas_group_unrealize   (EelCanvasItem *item);
static void   eel_canvas_group_map         (EelCanvasItem *item);
static void   eel_canvas_group_unmap       (EelCanvasItem *item);
static void   eel_canvas_group_draw        (EelCanvasItem *item, GdkDrawable *drawable,
					      GdkEventExpose *expose);
static double eel_canvas_group_point       (EelCanvasItem *item, double x, double y,
					      int cx, int cy,
					      EelCanvasItem **actual_item);
static void   eel_canvas_group_translate   (EelCanvasItem *item, double dx, double dy);
static void   eel_canvas_group_bounds      (EelCanvasItem *item, double *x1, double *y1,
					      double *x2, double *y2);


static EelCanvasItemClass *group_parent_class;


/**
 * eel_canvas_group_get_type:
 *
 * Registers the &EelCanvasGroup class if necessary, and returns the type ID
 * associated to it.
 *
 * Return value:  The type ID of the &EelCanvasGroup class.
 **/
GType
eel_canvas_group_get_type (void)
{
	static GType group_type = 0;

	if (!group_type) {
		static const GTypeInfo group_info = {
			sizeof (EelCanvasGroupClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) eel_canvas_group_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (EelCanvasGroup),
			0,              /* n_preallocs */
			(GInstanceInitFunc) eel_canvas_group_init

	
		};

		group_type = g_type_register_static (eel_canvas_item_get_type (),
						     "EelCanvasGroup",
						     &group_info,
						     0);
	}

	return group_type;
}

/* Class initialization function for EelCanvasGroupClass */
static void
eel_canvas_group_class_init (EelCanvasGroupClass *class)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	EelCanvasItemClass *item_class;

	gobject_class = (GObjectClass *) class;
	object_class = (GtkObjectClass *) class;
	item_class = (EelCanvasItemClass *) class;

	group_parent_class = gtk_type_class (eel_canvas_item_get_type ());

	gobject_class->set_property = eel_canvas_group_set_property;
	gobject_class->get_property = eel_canvas_group_get_property;

	g_object_class_install_property
		(gobject_class, GROUP_PROP_X,
		 g_param_spec_double ("x",
				      _("X"),
				      _("X"),
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
	g_object_class_install_property
		(gobject_class, GROUP_PROP_Y,
		 g_param_spec_double ("y",
				      _("Y"),
				      _("Y"),
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	object_class->destroy = eel_canvas_group_destroy;

	item_class->update = eel_canvas_group_update;
	item_class->realize = eel_canvas_group_realize;
	item_class->unrealize = eel_canvas_group_unrealize;
	item_class->map = eel_canvas_group_map;
	item_class->unmap = eel_canvas_group_unmap;
	item_class->draw = eel_canvas_group_draw;
	item_class->point = eel_canvas_group_point;
	item_class->translate = eel_canvas_group_translate;
	item_class->bounds = eel_canvas_group_bounds;
}

/* Object initialization function for EelCanvasGroup */
static void
eel_canvas_group_init (EelCanvasGroup *group)
{
	group->xpos = 0.0;
	group->ypos = 0.0;
}

/* Set_property handler for canvas groups */
static void
eel_canvas_group_set_property (GObject *gobject, guint param_id,
				 const GValue *value, GParamSpec *pspec)
{
	EelCanvasItem *item;
	EelCanvasGroup *group;

	g_return_if_fail (EEL_IS_CANVAS_GROUP (gobject));

	item = EEL_CANVAS_ITEM (gobject);
	group = EEL_CANVAS_GROUP (gobject);

	switch (param_id) {
	case GROUP_PROP_X:
		group->xpos = g_value_get_double (value);
		break;

	case GROUP_PROP_Y:
		group->ypos = g_value_get_double (value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, param_id, pspec);
		break;
	}
}

/* Get_property handler for canvas groups */
static void
eel_canvas_group_get_property (GObject *gobject, guint param_id,
				 GValue *value, GParamSpec *pspec)
{
	EelCanvasItem *item;
	EelCanvasGroup *group;
	int recalc;

	g_return_if_fail (EEL_IS_CANVAS_GROUP (gobject));

	item = EEL_CANVAS_ITEM (gobject);
	group = EEL_CANVAS_GROUP (gobject);

	recalc = FALSE;
	
	switch (param_id) {
	case GROUP_PROP_X:
		g_value_set_double (value, group->xpos);
		recalc = TRUE;
		break;

	case GROUP_PROP_Y:
		g_value_set_double (value, group->ypos);
		recalc = TRUE;
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, param_id, pspec);
		break;
	}
	
	if (recalc) {
		eel_canvas_group_child_bounds (group, NULL);

		if (item->parent)
			eel_canvas_group_child_bounds (EEL_CANVAS_GROUP (item->parent), item);
	}
}

/* Destroy handler for canvas groups */
static void
eel_canvas_group_destroy (GtkObject *object)
{
	EelCanvasGroup *group;
	EelCanvasItem *child;
	GList *list;

	g_return_if_fail (EEL_IS_CANVAS_GROUP (object));

	group = EEL_CANVAS_GROUP (object);

	list = group->item_list;
	while (list) {
		child = list->data;
		list = list->next;

		gtk_object_destroy (GTK_OBJECT (child));
	}

	if (GTK_OBJECT_CLASS (group_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (group_parent_class)->destroy) (object);
}

/* Update handler for canvas groups */
static void
eel_canvas_group_update (EelCanvasItem *item, double i2w_dx, double i2w_dy, int flags)
{
	EelCanvasGroup *group;
	GList *list;
	EelCanvasItem *i;
	double bbox_x0, bbox_y0, bbox_x1, bbox_y1;
	gboolean first = TRUE;

	group = EEL_CANVAS_GROUP (item);

	(* group_parent_class->update) (item, i2w_dx, i2w_dy, flags);

	bbox_x0 = 0;
	bbox_y0 = 0;
	bbox_x1 = 0;
	bbox_y1 = 0;

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		eel_canvas_item_invoke_update (i, i2w_dx + group->xpos, i2w_dy + group->ypos, flags);

		if (first) {
			first = FALSE;
			bbox_x0 = i->x1;
			bbox_y0 = i->y1;
			bbox_x1 = i->x2;
			bbox_y1 = i->y2;
		} else {
			bbox_x0 = MIN (bbox_x0, i->x1);
			bbox_y0 = MIN (bbox_y0, i->y1);
			bbox_x1 = MAX (bbox_x1, i->x2);
			bbox_y1 = MAX (bbox_y1, i->y2);
		}
	}
	item->x1 = bbox_x0;
	item->y1 = bbox_y0;
	item->x2 = bbox_x1;
	item->y2 = bbox_y1;
}

/* Realize handler for canvas groups */
static void
eel_canvas_group_realize (EelCanvasItem *item)
{
	EelCanvasGroup *group;
	GList *list;
	EelCanvasItem *i;

	group = EEL_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		if (!(i->object.flags & EEL_CANVAS_ITEM_REALIZED))
			(* EEL_CANVAS_ITEM_GET_CLASS (i)->realize) (i);
	}

	(* group_parent_class->realize) (item);
}

/* Unrealize handler for canvas groups */
static void
eel_canvas_group_unrealize (EelCanvasItem *item)
{
	EelCanvasGroup *group;
	GList *list;
	EelCanvasItem *i;

	group = EEL_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		if (i->object.flags & EEL_CANVAS_ITEM_REALIZED)
			(* EEL_CANVAS_ITEM_GET_CLASS (i)->unrealize) (i);
	}

	(* group_parent_class->unrealize) (item);
}

/* Map handler for canvas groups */
static void
eel_canvas_group_map (EelCanvasItem *item)
{
	EelCanvasGroup *group;
	GList *list;
	EelCanvasItem *i;

	group = EEL_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		if (!(i->object.flags & EEL_CANVAS_ITEM_MAPPED))
			(* EEL_CANVAS_ITEM_GET_CLASS (i)->map) (i);
	}

	(* group_parent_class->map) (item);
}

/* Unmap handler for canvas groups */
static void
eel_canvas_group_unmap (EelCanvasItem *item)
{
	EelCanvasGroup *group;
	GList *list;
	EelCanvasItem *i;

	group = EEL_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		i = list->data;

		if (i->object.flags & EEL_CANVAS_ITEM_MAPPED)
			(* EEL_CANVAS_ITEM_GET_CLASS (i)->unmap) (i);
	}

	(* group_parent_class->unmap) (item);
}

/* Draw handler for canvas groups */
static void
eel_canvas_group_draw (EelCanvasItem *item, GdkDrawable *drawable,
			 GdkEventExpose *expose)
{
	EelCanvasGroup *group;
	GList *list;
	EelCanvasItem *child = 0;

	group = EEL_CANVAS_GROUP (item);

	for (list = group->item_list; list; list = list->next) {
		child = list->data;

		if ((child->object.flags & EEL_CANVAS_ITEM_VISIBLE) &&
		    (EEL_CANVAS_ITEM_GET_CLASS (child)->draw)) {
			GdkRectangle child_rect;
			
			child_rect.x = child->x1;
			child_rect.y = child->y1;
			child_rect.width = child->x2 - child->x1 + 1;
			child_rect.height = child->y2 - child->y1 + 1;

			if (gdk_region_rect_in (expose->region, &child_rect) != GDK_OVERLAP_RECTANGLE_OUT)
				(* EEL_CANVAS_ITEM_GET_CLASS (child)->draw) (child, drawable, expose);
		}
	}
}

/* Point handler for canvas groups */
static double
eel_canvas_group_point (EelCanvasItem *item, double x, double y, int cx, int cy,
			  EelCanvasItem **actual_item)
{
	EelCanvasGroup *group;
	GList *list;
	EelCanvasItem *child, *point_item;
	int x1, y1, x2, y2;
	double gx, gy;
	double dist, best;
	int has_point;

	group = EEL_CANVAS_GROUP (item);

	x1 = cx - item->canvas->close_enough;
	y1 = cy - item->canvas->close_enough;
	x2 = cx + item->canvas->close_enough;
	y2 = cy + item->canvas->close_enough;

	best = 0.0;
	*actual_item = NULL;

	gx = x - group->xpos;
	gy = y - group->ypos;

	dist = 0.0; /* keep gcc happy */

	for (list = group->item_list; list; list = list->next) {
		child = list->data;

		if ((child->x1 > x2) || (child->y1 > y2) || (child->x2 < x1) || (child->y2 < y1))
			continue;

		point_item = NULL; /* cater for incomplete item implementations */

		if ((child->object.flags & EEL_CANVAS_ITEM_VISIBLE)
		    && EEL_CANVAS_ITEM_GET_CLASS (child)->point) {
			dist = eel_canvas_item_invoke_point (child, gx, gy, cx, cy, &point_item);
			has_point = TRUE;
		} else
			has_point = FALSE;

		if (has_point
		    && point_item
		    && ((int) (dist * item->canvas->pixels_per_unit + 0.5)
			<= item->canvas->close_enough)) {
			best = dist;
			*actual_item = point_item;
		}
	}

	return best;
}

void
eel_canvas_group_translate (EelCanvasItem *item, double dx, double dy)
{
        EelCanvasGroup *group;

        group = EEL_CANVAS_GROUP (item);

        group->xpos += dx;
        group->ypos += dy;
}

/* Bounds handler for canvas groups */
static void
eel_canvas_group_bounds (EelCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	EelCanvasGroup *group;
	EelCanvasItem *child;
	GList *list;
	double tx1, ty1, tx2, ty2;
	double minx, miny, maxx, maxy;
	int set;

	group = EEL_CANVAS_GROUP (item);

	/* Get the bounds of the first visible item */

	child = NULL; /* Unnecessary but eliminates a warning. */

	set = FALSE;

	for (list = group->item_list; list; list = list->next) {
		child = list->data;

		if (child->object.flags & EEL_CANVAS_ITEM_VISIBLE) {
			set = TRUE;
			eel_canvas_item_get_bounds (child, &minx, &miny, &maxx, &maxy);
			break;
		}
	}

	/* If there were no visible items, return an empty bounding box */

	if (!set) {
		*x1 = *y1 = *x2 = *y2 = 0.0;
		return;
	}

	/* Now we can grow the bounds using the rest of the items */

	list = list->next;

	for (; list; list = list->next) {
		child = list->data;

		if (!(child->object.flags & EEL_CANVAS_ITEM_VISIBLE))
			continue;

		eel_canvas_item_get_bounds (child, &tx1, &ty1, &tx2, &ty2);

		if (tx1 < minx)
			minx = tx1;

		if (ty1 < miny)
			miny = ty1;

		if (tx2 > maxx)
			maxx = tx2;

		if (ty2 > maxy)
			maxy = ty2;
	}

	/* Make the bounds be relative to our parent's coordinate system */

	if (item->parent) {
		minx += group->xpos;
		miny += group->ypos;
		maxx += group->xpos;
		maxy += group->ypos;
	}
	
	*x1 = minx;
	*y1 = miny;
	*x2 = maxx;
	*y2 = maxy;
}

/* Adds an item to a group */
static void
group_add (EelCanvasGroup *group, EelCanvasItem *item)
{
	g_object_ref (GTK_OBJECT (item));
	gtk_object_sink (GTK_OBJECT (item));

	if (!group->item_list) {
		group->item_list = g_list_append (group->item_list, item);
		group->item_list_end = group->item_list;
	} else
		group->item_list_end = g_list_append (group->item_list_end, item)->next;

	if (group->item.object.flags & EEL_CANVAS_ITEM_REALIZED)
		(* EEL_CANVAS_ITEM_GET_CLASS (item)->realize) (item);

	if (group->item.object.flags & EEL_CANVAS_ITEM_MAPPED)
		(* EEL_CANVAS_ITEM_GET_CLASS (item)->map) (item);
}

/* Removes an item from a group */
static void
group_remove (EelCanvasGroup *group, EelCanvasItem *item)
{
	GList *children;

	g_return_if_fail (EEL_IS_CANVAS_GROUP (group));
	g_return_if_fail (EEL_IS_CANVAS_ITEM (item));

	for (children = group->item_list; children; children = children->next)
		if (children->data == item) {
			if (item->object.flags & EEL_CANVAS_ITEM_MAPPED)
				(* EEL_CANVAS_ITEM_GET_CLASS (item)->unmap) (item);

			if (item->object.flags & EEL_CANVAS_ITEM_REALIZED)
				(* EEL_CANVAS_ITEM_GET_CLASS (item)->unrealize) (item);

			/* Unparent the child */

			item->parent = NULL;
			g_object_unref (GTK_OBJECT (item));

			/* Remove it from the list */

			if (children == group->item_list_end)
				group->item_list_end = children->prev;

			group->item_list = g_list_remove_link (group->item_list, children);
			g_list_free (children);
			break;
		}
}

static void
eel_canvas_group_child_bounds (EelCanvasGroup *group, EelCanvasItem *item)
{
#if 0
	EelCanvasItem *gitem;
	GList *list;
	int first;

	g_return_if_fail (group != NULL);
	g_return_if_fail (EEL_IS_CANVAS_GROUP (group));
	g_return_if_fail (!item || EEL_IS_CANVAS_ITEM (item));

	gitem = EEL_CANVAS_ITEM (group);

	if (item) {
		/* Just add the child's bounds to whatever we have now */

		if ((item->x1 == item->x2) || (item->y1 == item->y2))
			return; /* empty bounding box */

		if (item->x1 < gitem->x1)
			gitem->x1 = item->x1;

		if (item->y1 < gitem->y1)
			gitem->y1 = item->y1;

		if (item->x2 > gitem->x2)
			gitem->x2 = item->x2;

		if (item->y2 > gitem->y2)
			gitem->y2 = item->y2;

		/* Propagate upwards */

		if (gitem->parent)
			eel_canvas_group_child_bounds (EEL_CANVAS_GROUP (gitem->parent), gitem);
	} else {
		/* Rebuild every sub-group's bounds and reconstruct our own bounds */

		for (list = group->item_list, first = TRUE; list; list = list->next, first = FALSE) {
			item = list->data;

			if (EEL_IS_CANVAS_GROUP (item))
				eel_canvas_group_child_bounds (EEL_CANVAS_GROUP (item), NULL);
			else if (EEL_CANVAS_ITEM_CLASS (item->object.klass)->reconfigure)
					(* EEL_CANVAS_ITEM_CLASS (item->object.klass)->reconfigure) (item);

			if (first) {
				gitem->x1 = item->x1;
				gitem->y1 = item->y1;
				gitem->x2 = item->x2;
				gitem->y2 = item->y2;
			} else {
				if (item->x1 < gitem->x1)
					gitem->x1 = item->x1;

				if (item->y1 < gitem->y1)
					gitem->y1 = item->y1;

				if (item->x2 > gitem->x2)
					gitem->x2 = item->x2;

				if (item->y2 > gitem->y2)
					gitem->y2 = item->y2;
			}
		}
	}
#endif
}


/*** EelCanvas ***/


enum {
	DRAW_BACKGROUND,
	LAST_SIGNAL
};

static void eel_canvas_class_init          (EelCanvasClass *class);
static void eel_canvas_init                (EelCanvas      *canvas);
static void eel_canvas_destroy             (GtkObject        *object);
static void eel_canvas_map                 (GtkWidget        *widget);
static void eel_canvas_unmap               (GtkWidget        *widget);
static void eel_canvas_realize             (GtkWidget        *widget);
static void eel_canvas_unrealize           (GtkWidget        *widget);
static void eel_canvas_size_allocate       (GtkWidget        *widget,
					      GtkAllocation    *allocation);
static gint eel_canvas_button              (GtkWidget        *widget,
					      GdkEventButton   *event);
static gint eel_canvas_motion              (GtkWidget        *widget,
					      GdkEventMotion   *event);
static gint eel_canvas_expose              (GtkWidget        *widget,
					      GdkEventExpose   *event);
static gint eel_canvas_key                 (GtkWidget        *widget,
					      GdkEventKey      *event);
static gint eel_canvas_crossing            (GtkWidget        *widget,
					      GdkEventCrossing *event);
static gint eel_canvas_focus_in            (GtkWidget        *widget,
					      GdkEventFocus    *event);
static gint eel_canvas_focus_out           (GtkWidget        *widget,
					      GdkEventFocus    *event);
static void eel_canvas_request_update_real (EelCanvas      *canvas);
static void eel_canvas_draw_background     (EelCanvas      *canvas,
					      int               x,
					      int               y,
					      int               width,
					      int               height);


static GtkLayoutClass *canvas_parent_class;

static guint canvas_signals[LAST_SIGNAL];

/**
 * eel_canvas_get_type:
 *
 * Registers the &EelCanvas class if necessary, and returns the type ID
 * associated to it.
 *
 * Return value:  The type ID of the &EelCanvas class.
 **/
GType
eel_canvas_get_type (void)
{
	static GType canvas_type = 0;

	if (!canvas_type) {
		static const GTypeInfo canvas_info = {
			sizeof (EelCanvasClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) eel_canvas_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (EelCanvas),
			0,              /* n_preallocs */
			(GInstanceInitFunc) eel_canvas_init
		};

		canvas_type = g_type_register_static (gtk_layout_get_type (),
						      "EelCanvas",
						      &canvas_info,
						      0);
	}

	return canvas_type;
}

static void
eel_canvas_get_property (GObject    *object, 
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
eel_canvas_set_property (GObject      *object, 
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/* Class initialization function for EelCanvasClass */
static void
eel_canvas_class_init (EelCanvasClass *klass)
{
	GObjectClass   *gobject_class;
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	gobject_class = (GObjectClass *)klass;
	object_class  = (GtkObjectClass *) klass;
	widget_class  = (GtkWidgetClass *) klass;

	canvas_parent_class = gtk_type_class (gtk_layout_get_type ());

	gobject_class->set_property = eel_canvas_set_property;
	gobject_class->get_property = eel_canvas_get_property;

	object_class->destroy = eel_canvas_destroy;

	widget_class->map = eel_canvas_map;
	widget_class->unmap = eel_canvas_unmap;
	widget_class->realize = eel_canvas_realize;
	widget_class->unrealize = eel_canvas_unrealize;
	widget_class->size_allocate = eel_canvas_size_allocate;
	widget_class->button_press_event = eel_canvas_button;
	widget_class->button_release_event = eel_canvas_button;
	widget_class->motion_notify_event = eel_canvas_motion;
	widget_class->expose_event = eel_canvas_expose;
	widget_class->key_press_event = eel_canvas_key;
	widget_class->key_release_event = eel_canvas_key;
	widget_class->enter_notify_event = eel_canvas_crossing;
	widget_class->leave_notify_event = eel_canvas_crossing;
	widget_class->focus_in_event = eel_canvas_focus_in;
	widget_class->focus_out_event = eel_canvas_focus_out;

	klass->draw_background = eel_canvas_draw_background;
	klass->request_update = eel_canvas_request_update_real;

	canvas_signals[DRAW_BACKGROUND] =
		g_signal_new ("draw_background",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EelCanvasClass, draw_background),
			      NULL, NULL,
			      eel_marshal_VOID__INT_INT_INT_INT,
			      G_TYPE_NONE, 4, 
			      G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
}

/* Callback used when the root item of a canvas is destroyed.  The user should
 * never ever do this, so we panic if this happens.
 */
static void
panic_root_destroyed (GtkObject *object, gpointer data)
{
	g_error ("Eeeek, root item %p of canvas %p was destroyed!", object, data);
}

/* Object initialization function for EelCanvas */
static void
eel_canvas_init (EelCanvas *canvas)
{
	GTK_WIDGET_SET_FLAGS (canvas, GTK_CAN_FOCUS);

	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (canvas), FALSE);

	canvas->scroll_x1 = 0.0;
	canvas->scroll_y1 = 0.0;
	canvas->scroll_x2 = canvas->layout.width;
	canvas->scroll_y2 = canvas->layout.height;

	canvas->pixels_per_unit = 1.0;

	canvas->pick_event.type = GDK_LEAVE_NOTIFY;
	canvas->pick_event.crossing.x = 0;
	canvas->pick_event.crossing.y = 0;

	gtk_layout_set_hadjustment (GTK_LAYOUT (canvas), NULL);
	gtk_layout_set_vadjustment (GTK_LAYOUT (canvas), NULL);

	/* Create the root item as a special case */

	canvas->root = EEL_CANVAS_ITEM (g_object_new (eel_canvas_group_get_type (), NULL));
	canvas->root->canvas = canvas;

	g_object_ref (GTK_OBJECT (canvas->root));
	gtk_object_sink (GTK_OBJECT (canvas->root));

	canvas->root_destroy_id = g_signal_connect (GTK_OBJECT (canvas->root), "destroy",
						    (GtkSignalFunc) panic_root_destroyed,
						    canvas);

	canvas->need_repick = TRUE;
}

/* Convenience function to remove the idle handler of a canvas */
static void
remove_idle (EelCanvas *canvas)
{
	if (canvas->idle_id == 0)
		return;

	gtk_idle_remove (canvas->idle_id);
	canvas->idle_id = 0;
}

/* Removes the transient state of the canvas (idle handler, grabs). */
static void
shutdown_transients (EelCanvas *canvas)
{
	/* We turn off the need_redraw flag, since if the canvas is mapped again
	 * it will request a redraw anyways.  We do not turn off the need_update
	 * flag, though, because updates are not queued when the canvas remaps
	 * itself.
	 */
	if (canvas->need_redraw) {
		canvas->need_redraw = FALSE;
	}

	if (canvas->grabbed_item) {
		canvas->grabbed_item = NULL;
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
	}

	remove_idle (canvas);
}

/* Destroy handler for EelCanvas */
static void
eel_canvas_destroy (GtkObject *object)
{
	EelCanvas *canvas;

	g_return_if_fail (EEL_IS_CANVAS (object));

	/* remember, destroy can be run multiple times! */

	canvas = EEL_CANVAS (object);

	if (canvas->root_destroy_id) {
		g_signal_handler_disconnect (GTK_OBJECT (canvas->root), canvas->root_destroy_id);
		canvas->root_destroy_id = 0;
	}
	if (canvas->root) {
		g_object_unref (GTK_OBJECT (canvas->root));
		canvas->root = NULL;
	}

	shutdown_transients (canvas);

	if (GTK_OBJECT_CLASS (canvas_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (canvas_parent_class)->destroy) (object);
}

/**
 * eel_canvas_new:
 * @void:
 *
 * Creates a new empty canvas.  If you wish to use the
 * &EelCanvasImage item inside this canvas, then you must push the gdk_imlib
 * visual and colormap before calling this function, and they can be popped
 * afterwards.
 *
 * Return value: A newly-created canvas.
 **/
GtkWidget *
eel_canvas_new (void)
{
	return GTK_WIDGET (g_object_new (eel_canvas_get_type (), NULL));
}

/* Map handler for the canvas */
static void
eel_canvas_map (GtkWidget *widget)
{
	EelCanvas *canvas;

	g_return_if_fail (EEL_IS_CANVAS (widget));

	/* Normal widget mapping stuff */

	if (GTK_WIDGET_CLASS (canvas_parent_class)->map)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->map) (widget);

	canvas = EEL_CANVAS (widget);

	/* Map items */

	if (EEL_CANVAS_ITEM_GET_CLASS (canvas->root)->map)
		(* EEL_CANVAS_ITEM_GET_CLASS (canvas->root)->map) (canvas->root);
}

/* Unmap handler for the canvas */
static void
eel_canvas_unmap (GtkWidget *widget)
{
	EelCanvas *canvas;

	g_return_if_fail (EEL_IS_CANVAS (widget));

	canvas = EEL_CANVAS (widget);

	shutdown_transients (canvas);

	/* Unmap items */

	if (EEL_CANVAS_ITEM_GET_CLASS (canvas->root)->unmap)
		(* EEL_CANVAS_ITEM_GET_CLASS (canvas->root)->unmap) (canvas->root);

	/* Normal widget unmapping stuff */

	if (GTK_WIDGET_CLASS (canvas_parent_class)->unmap)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->unmap) (widget);
}

/* Realize handler for the canvas */
static void
eel_canvas_realize (GtkWidget *widget)
{
	EelCanvas *canvas;

	g_return_if_fail (EEL_IS_CANVAS (widget));

	/* Normal widget realization stuff */

	if (GTK_WIDGET_CLASS (canvas_parent_class)->realize)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->realize) (widget);

	canvas = EEL_CANVAS (widget);

	gdk_window_set_events (canvas->layout.bin_window,
			       (gdk_window_get_events (canvas->layout.bin_window)
				 | GDK_EXPOSURE_MASK
				 | GDK_BUTTON_PRESS_MASK
				 | GDK_BUTTON_RELEASE_MASK
				 | GDK_POINTER_MOTION_MASK
				 | GDK_KEY_PRESS_MASK
				 | GDK_KEY_RELEASE_MASK
				 | GDK_ENTER_NOTIFY_MASK
				 | GDK_LEAVE_NOTIFY_MASK
				 | GDK_FOCUS_CHANGE_MASK));

	/* Create our own temporary pixmap gc and realize all the items */

	canvas->pixmap_gc = gdk_gc_new (canvas->layout.bin_window);

	(* EEL_CANVAS_ITEM_GET_CLASS (canvas->root)->realize) (canvas->root);
}

/* Unrealize handler for the canvas */
static void
eel_canvas_unrealize (GtkWidget *widget)
{
	EelCanvas *canvas;

	g_return_if_fail (EEL_IS_CANVAS (widget));

	canvas = EEL_CANVAS (widget);

	shutdown_transients (canvas);

	/* Unrealize items and parent widget */

	(* EEL_CANVAS_ITEM_GET_CLASS (canvas->root)->unrealize) (canvas->root);

	g_object_unref (canvas->pixmap_gc);
	canvas->pixmap_gc = NULL;

	if (GTK_WIDGET_CLASS (canvas_parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->unrealize) (widget);
}

/* Handles scrolling of the canvas.  Adjusts the scrolling and zooming offset to
 * keep as much as possible of the canvas scrolling region in view.
 */
static void
scroll_to (EelCanvas *canvas, int cx, int cy)
{
	int scroll_width, scroll_height;
	int right_limit, bottom_limit;
	int old_zoom_xofs, old_zoom_yofs;
	int changed_x = FALSE, changed_y = FALSE;
	int canvas_width, canvas_height;

	canvas_width = GTK_WIDGET (canvas)->allocation.width;
	canvas_height = GTK_WIDGET (canvas)->allocation.height;

	scroll_width = floor ((canvas->scroll_x2 - canvas->scroll_x1) * canvas->pixels_per_unit + 0.5);
	scroll_height = floor ((canvas->scroll_y2 - canvas->scroll_y1) * canvas->pixels_per_unit + 0.5);

	right_limit = scroll_width - canvas_width;
	bottom_limit = scroll_height - canvas_height;

	old_zoom_xofs = canvas->zoom_xofs;
	old_zoom_yofs = canvas->zoom_yofs;

	if (right_limit < 0) {
		cx = 0;
		if (canvas->center_scroll_region) {
			canvas->zoom_xofs = (canvas_width - scroll_width) / 2;
			scroll_width = canvas_width;
		} else {
			canvas->zoom_xofs = 0;
		}
	} else if (cx < 0) {
		cx = 0;
		canvas->zoom_xofs = 0;
	} else if (cx > right_limit) {
		cx = right_limit;
		canvas->zoom_xofs = 0;
	} else
		canvas->zoom_xofs = 0;

	if (bottom_limit < 0) {
		cy = 0;
		if (canvas->center_scroll_region) {
			canvas->zoom_yofs = (canvas_height - scroll_height) / 2;
			scroll_height = canvas_height;
		} else {
			canvas->zoom_yofs = 0;
		}
	} else if (cy < 0) {
		cy = 0;
		canvas->zoom_yofs = 0;
	} else if (cy > bottom_limit) {
		cy = bottom_limit;
		canvas->zoom_yofs = 0;
	} else
		canvas->zoom_yofs = 0;

	if ((canvas->zoom_xofs != old_zoom_xofs) || (canvas->zoom_yofs != old_zoom_yofs)) {
		/* This can only occur, if either canvas size or widget size changes */
		/* So I think we can request full redraw here */
		/* More stuff - we have to mark root as needing fresh affine (Lauris) */
		if (!(canvas->root->object.flags & EEL_CANVAS_ITEM_NEED_DEEP_UPDATE)) {
			canvas->root->object.flags |= EEL_CANVAS_ITEM_NEED_DEEP_UPDATE;
			eel_canvas_request_update (canvas);
		}
		gtk_widget_queue_draw (GTK_WIDGET (canvas));
	}

	if (((int) canvas->layout.hadjustment->value) != cx) {
		canvas->layout.hadjustment->value = cx;
		changed_x = TRUE;
	}

	if (((int) canvas->layout.vadjustment->value) != cy) {
		canvas->layout.vadjustment->value = cy;
		changed_y = TRUE;
	}

	if ((scroll_width != (int) canvas->layout.width) || (scroll_height != (int) canvas->layout.height)) {
		gtk_layout_set_size (GTK_LAYOUT (canvas), scroll_width, scroll_height);
	}
	
	/* Signal GtkLayout that it should do a redraw. */
	if (changed_x)
		g_signal_emit_by_name (GTK_OBJECT (canvas->layout.hadjustment), "value_changed");
	if (changed_y)
		g_signal_emit_by_name (GTK_OBJECT (canvas->layout.vadjustment), "value_changed");
}

/* Size allocation handler for the canvas */
static void
eel_canvas_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	EelCanvas *canvas;

	g_return_if_fail (EEL_IS_CANVAS (widget));
	g_return_if_fail (allocation != NULL);

	if (GTK_WIDGET_CLASS (canvas_parent_class)->size_allocate)
		(* GTK_WIDGET_CLASS (canvas_parent_class)->size_allocate) (widget, allocation);

	canvas = EEL_CANVAS (widget);

	/* Recenter the view, if appropriate */

	canvas->layout.hadjustment->page_size = allocation->width;
	canvas->layout.hadjustment->page_increment = allocation->width / 2;

	canvas->layout.vadjustment->page_size = allocation->height;
	canvas->layout.vadjustment->page_increment = allocation->height / 2;

	scroll_to (canvas,
		   canvas->layout.hadjustment->value,
		   canvas->layout.vadjustment->value);

	g_signal_emit_by_name (GTK_OBJECT (canvas->layout.hadjustment), "changed");
	g_signal_emit_by_name (GTK_OBJECT (canvas->layout.vadjustment), "changed");
}

/* Emits an event for an item in the canvas, be it the current item, grabbed
 * item, or focused item, as appropriate.
 */

static int
emit_event (EelCanvas *canvas, GdkEvent *event)
{
	GdkEvent ev;
	gint finished;
	EelCanvasItem *item;
	EelCanvasItem *parent;
	guint mask;

	/* Perform checks for grabbed items */

	if (canvas->grabbed_item &&
	    !is_descendant (canvas->current_item, canvas->grabbed_item)) {
                g_warning ("emit_event() returning FALSE!\n");
		return FALSE;
        }

	if (canvas->grabbed_item) {
		switch (event->type) {
		case GDK_ENTER_NOTIFY:
			mask = GDK_ENTER_NOTIFY_MASK;
			break;

		case GDK_LEAVE_NOTIFY:
			mask = GDK_LEAVE_NOTIFY_MASK;
			break;

		case GDK_MOTION_NOTIFY:
			mask = GDK_POINTER_MOTION_MASK;
			break;

		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
			mask = GDK_BUTTON_PRESS_MASK;
			break;

		case GDK_BUTTON_RELEASE:
			mask = GDK_BUTTON_RELEASE_MASK;
			break;

		case GDK_KEY_PRESS:
			mask = GDK_KEY_PRESS_MASK;
			break;

		case GDK_KEY_RELEASE:
			mask = GDK_KEY_RELEASE_MASK;
			break;

		default:
			mask = 0;
			break;
		}

		if (!(mask & canvas->grabbed_event_mask))
			return FALSE;
	}

	/* Convert to world coordinates -- we have two cases because of diferent
	 * offsets of the fields in the event structures.
	 */

	ev = *event;

	switch (ev.type)
        {
	case GDK_ENTER_NOTIFY:
	case GDK_LEAVE_NOTIFY:
		eel_canvas_window_to_world (canvas,
					      ev.crossing.x, ev.crossing.y,
					      &ev.crossing.x, &ev.crossing.y);
		break;

	case GDK_MOTION_NOTIFY:
                eel_canvas_window_to_world (canvas,
                                              ev.motion.x, ev.motion.y,
                                              &ev.motion.x, &ev.motion.y);
                break;

	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
                eel_canvas_window_to_world (canvas,
                                              ev.motion.x, ev.motion.y,
                                              &ev.motion.x, &ev.motion.y);
                break;

	case GDK_BUTTON_RELEASE:
		eel_canvas_window_to_world (canvas,
					      ev.motion.x, ev.motion.y,
					      &ev.motion.x, &ev.motion.y);
		break;

	default:
		break;
	}

	/* Choose where we send the event */

	item = canvas->current_item;

	if (canvas->focused_item
	    && ((event->type == GDK_KEY_PRESS) ||
		(event->type == GDK_KEY_RELEASE) ||
		(event->type == GDK_FOCUS_CHANGE)))
		item = canvas->focused_item;

	/* The event is propagated up the hierarchy (for if someone connected to
	 * a group instead of a leaf event), and emission is stopped if a
	 * handler returns TRUE, just like for GtkWidget events.
	 */

	finished = FALSE;

	while (item && !finished) {
		g_object_ref (GTK_OBJECT (item));

		g_signal_emit (
		       GTK_OBJECT (item), item_signals[ITEM_EVENT], 0,
			&ev, &finished);
		
		parent = item->parent;
		g_object_unref (GTK_OBJECT (item));

		item = parent;
	}

	return finished;
}

/* Re-picks the current item in the canvas, based on the event's coordinates.
 * Also emits enter/leave events for items as appropriate.
 */
static int
pick_current_item (EelCanvas *canvas, GdkEvent *event)
{
	int button_down;
	double x, y;
	int cx, cy;
	int retval;

	retval = FALSE;

	/* If a button is down, we'll perform enter and leave events on the
	 * current item, but not enter on any other item.  This is more or less
	 * like X pointer grabbing for canvas items.
	 */
	button_down = canvas->state & (GDK_BUTTON1_MASK
				       | GDK_BUTTON2_MASK
				       | GDK_BUTTON3_MASK
				       | GDK_BUTTON4_MASK
				       | GDK_BUTTON5_MASK);
	if (!button_down)
		canvas->left_grabbed_item = FALSE;

	/* Save the event in the canvas.  This is used to synthesize enter and
	 * leave events in case the current item changes.  It is also used to
	 * re-pick the current item if the current one gets deleted.  Also,
	 * synthesize an enter event.
	 */
	if (event != &canvas->pick_event) {
		if ((event->type == GDK_MOTION_NOTIFY) || (event->type == GDK_BUTTON_RELEASE)) {
			/* these fields have the same offsets in both types of events */

			canvas->pick_event.crossing.type       = GDK_ENTER_NOTIFY;
			canvas->pick_event.crossing.window     = event->motion.window;
			canvas->pick_event.crossing.send_event = event->motion.send_event;
			canvas->pick_event.crossing.subwindow  = NULL;
			canvas->pick_event.crossing.x          = event->motion.x;
			canvas->pick_event.crossing.y          = event->motion.y;
			canvas->pick_event.crossing.mode       = GDK_CROSSING_NORMAL;
			canvas->pick_event.crossing.detail     = GDK_NOTIFY_NONLINEAR;
			canvas->pick_event.crossing.focus      = FALSE;
			canvas->pick_event.crossing.state      = event->motion.state;

			/* these fields don't have the same offsets in both types of events */

			if (event->type == GDK_MOTION_NOTIFY) {
				canvas->pick_event.crossing.x_root = event->motion.x_root;
				canvas->pick_event.crossing.y_root = event->motion.y_root;
			} else {
				canvas->pick_event.crossing.x_root = event->button.x_root;
				canvas->pick_event.crossing.y_root = event->button.y_root;
			}
		} else
			canvas->pick_event = *event;
	}

	/* Don't do anything else if this is a recursive call */

	if (canvas->in_repick)
		return retval;

	/* LeaveNotify means that there is no current item, so we don't look for one */

	if (canvas->pick_event.type != GDK_LEAVE_NOTIFY) {
		/* these fields don't have the same offsets in both types of events */

		if (canvas->pick_event.type == GDK_ENTER_NOTIFY) {
			x = canvas->pick_event.crossing.x;
			y = canvas->pick_event.crossing.y;
		} else {
			x = canvas->pick_event.motion.x;
			y = canvas->pick_event.motion.y;
		}

		/* canvas pixel coords */

		cx = (int) (x + 0.5);
		cy = (int) (y + 0.5);

		/* world coords */
		eel_canvas_c2w (canvas, cx, cy, &x, &y);

		/* find the closest item */
		if (canvas->root->object.flags & EEL_CANVAS_ITEM_VISIBLE)
			eel_canvas_item_invoke_point (canvas->root, x, y, cx, cy,
							&canvas->new_current_item);
		else
			canvas->new_current_item = NULL;
	} else
		canvas->new_current_item = NULL;

	if ((canvas->new_current_item == canvas->current_item) && !canvas->left_grabbed_item)
		return retval; /* current item did not change */

	/* Synthesize events for old and new current items */

	if ((canvas->new_current_item != canvas->current_item)
	    && (canvas->current_item != NULL)
	    && !canvas->left_grabbed_item) {
		GdkEvent new_event;
		EelCanvasItem *item;

		item = canvas->current_item;

		new_event = canvas->pick_event;
		new_event.type = GDK_LEAVE_NOTIFY;

		new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
		new_event.crossing.subwindow = NULL;
		canvas->in_repick = TRUE;
		retval = emit_event (canvas, &new_event);
		canvas->in_repick = FALSE;
	}

	/* new_current_item may have been set to NULL during the call to emit_event() above */

	if ((canvas->new_current_item != canvas->current_item) && button_down) {
		canvas->left_grabbed_item = TRUE;
		return retval;
	}

	/* Handle the rest of cases */

	canvas->left_grabbed_item = FALSE;
	canvas->current_item = canvas->new_current_item;

	if (canvas->current_item != NULL) {
		GdkEvent new_event;

		new_event = canvas->pick_event;
		new_event.type = GDK_ENTER_NOTIFY;
		new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
		new_event.crossing.subwindow = NULL;
		retval = emit_event (canvas, &new_event);
	}

	return retval;
}

/* Button event handler for the canvas */
static gint
eel_canvas_button (GtkWidget *widget, GdkEventButton *event)
{
	EelCanvas *canvas;
	int mask;
	int retval;

	g_return_val_if_fail (EEL_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	retval = FALSE;

	canvas = EEL_CANVAS (widget);

	/*
	 * dispatch normally regardless of the event's window if an item has
	 * has a pointer grab in effect
	 */
	if (!canvas->grabbed_item && event->window != canvas->layout.bin_window)
		return retval;

	switch (event->button) {
	case 1:
		mask = GDK_BUTTON1_MASK;
		break;
	case 2:
		mask = GDK_BUTTON2_MASK;
		break;
	case 3:
		mask = GDK_BUTTON3_MASK;
		break;
	case 4:
		mask = GDK_BUTTON4_MASK;
		break;
	case 5:
		mask = GDK_BUTTON5_MASK;
		break;
	default:
		mask = 0;
	}

	switch (event->type) {
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
		/* Pick the current item as if the button were not pressed, and
		 * then process the event.
		 */
		canvas->state = event->state;
		pick_current_item (canvas, (GdkEvent *) event);
		canvas->state ^= mask;
		retval = emit_event (canvas, (GdkEvent *) event);
		break;

	case GDK_BUTTON_RELEASE:
		/* Process the event as if the button were pressed, then repick
		 * after the button has been released
		 */
		canvas->state = event->state;
		retval = emit_event (canvas, (GdkEvent *) event);
		event->state ^= mask;
		canvas->state = event->state;
		pick_current_item (canvas, (GdkEvent *) event);
		event->state ^= mask;
		break;

	default:
		g_assert_not_reached ();
	}

	return retval;
}

/* Motion event handler for the canvas */
static gint
eel_canvas_motion (GtkWidget *widget, GdkEventMotion *event)
{
	EelCanvas *canvas;

	g_return_val_if_fail (EEL_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	canvas = EEL_CANVAS (widget);

	if (event->window != canvas->layout.bin_window)
		return FALSE;

	canvas->state = event->state;
	pick_current_item (canvas, (GdkEvent *) event);
	return emit_event (canvas, (GdkEvent *) event);
}

/* Key event handler for the canvas */
static gint
eel_canvas_key (GtkWidget *widget, GdkEventKey *event)
{
	EelCanvas *canvas;
	
	g_return_val_if_fail (EEL_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	canvas = EEL_CANVAS (widget);
	
	return emit_event (canvas, (GdkEvent *) event);
}


/* Crossing event handler for the canvas */
static gint
eel_canvas_crossing (GtkWidget *widget, GdkEventCrossing *event)
{
	EelCanvas *canvas;

	g_return_val_if_fail (EEL_IS_CANVAS (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	canvas = EEL_CANVAS (widget);

	if (event->window != canvas->layout.bin_window)
		return FALSE;

	canvas->state = event->state;
	return pick_current_item (canvas, (GdkEvent *) event);
}

/* Focus in handler for the canvas */
static gint
eel_canvas_focus_in (GtkWidget *widget, GdkEventFocus *event)
{
	EelCanvas *canvas;

	GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);

	canvas = EEL_CANVAS (widget);

	if (canvas->focused_item)
		return emit_event (canvas, (GdkEvent *) event);
	else
		return FALSE;
}

/* Focus out handler for the canvas */
static gint
eel_canvas_focus_out (GtkWidget *widget, GdkEventFocus *event)
{
	EelCanvas *canvas;

	GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

	canvas = EEL_CANVAS (widget);

	if (canvas->focused_item)
		return emit_event (canvas, (GdkEvent *) event);
	else
		return FALSE;
}

/* Expose handler for the canvas */
static gint
eel_canvas_expose (GtkWidget *widget, GdkEventExpose *event)
{
	EelCanvas *canvas;

	canvas = EEL_CANVAS (widget);

	if (!GTK_WIDGET_DRAWABLE (widget) || (event->window != canvas->layout.bin_window)) return FALSE;

#ifdef VERBOSE
	g_print ("Expose\n");
#endif
	/* If there are any outstanding items that need updating, do them now */
	if (canvas->idle_id) {
		g_source_remove (canvas->idle_id);
		canvas->idle_id = 0;
	}
	if (canvas->need_update) {
		eel_canvas_item_invoke_update (canvas->root, 0, 0, 0);

		canvas->need_update = FALSE;
	}

	/* Hmmm. Would like to queue antiexposes if the update marked
	   anything that is gonna get redrawn as invalid */
	
	
	g_signal_emit (G_OBJECT (canvas), canvas_signals[DRAW_BACKGROUND], 0, 
		       event->area.x, event->area.y,
		       event->area.width, event->area.height);
	
	if (canvas->root->object.flags & EEL_CANVAS_ITEM_VISIBLE)
		(* EEL_CANVAS_ITEM_GET_CLASS (canvas->root)->draw) (canvas->root,
								      canvas->layout.bin_window,
								      event);



	/* Chain up to get exposes on child widgets */
	GTK_WIDGET_CLASS (canvas_parent_class)->expose_event (widget, event);

	return TRUE;
}

static void
eel_canvas_draw_background (EelCanvas *canvas,
			      int x, int y, int width, int height)
{
	/* By default, we use the style background. */
	gdk_gc_set_foreground (canvas->pixmap_gc,
			       &GTK_WIDGET (canvas)->style->bg[GTK_STATE_NORMAL]);
	gdk_draw_rectangle (canvas->layout.bin_window,
			    canvas->pixmap_gc,
			    TRUE,
			    x, y,
			    width, height);
}

static void
do_update (EelCanvas *canvas)
{
	/* Cause the update if necessary */

	if (canvas->need_update) {
		eel_canvas_item_invoke_update (canvas->root, 0, 0, 0);

		canvas->need_update = FALSE;
	}

	/* Pick new current item */

	while (canvas->need_repick) {
		canvas->need_repick = FALSE;
		pick_current_item (canvas, &canvas->pick_event);
	}
}

/* Idle handler for the canvas.  It deals with pending updates and redraws. */
static gint
idle_handler (gpointer data)
{
	EelCanvas *canvas;

	GDK_THREADS_ENTER ();

	canvas = EEL_CANVAS (data);
	do_update (canvas);

	/* Reset idle id */
	canvas->idle_id = 0;

	GDK_THREADS_LEAVE ();

	return FALSE;
}

/* Convenience function to add an idle handler to a canvas */
static void
add_idle (EelCanvas *canvas)
{
	if (!canvas->idle_id) {
		canvas->idle_id = g_idle_add (idle_handler, canvas);
	}
}

/**
 * eel_canvas_root:
 * @canvas: A canvas.
 *
 * Queries the root group of a canvas.
 *
 * Return value: The root group of the specified canvas.
 **/
EelCanvasGroup *
eel_canvas_root (EelCanvas *canvas)
{
	g_return_val_if_fail (EEL_IS_CANVAS (canvas), NULL);

	return EEL_CANVAS_GROUP (canvas->root);
}


/**
 * eel_canvas_set_scroll_region:
 * @canvas: A canvas.
 * @x1: Leftmost limit of the scrolling region.
 * @y1: Upper limit of the scrolling region.
 * @x2: Rightmost limit of the scrolling region.
 * @y2: Lower limit of the scrolling region.
 *
 * Sets the scrolling region of a canvas to the specified rectangle.  The canvas
 * will then be able to scroll only within this region.  The view of the canvas
 * is adjusted as appropriate to display as much of the new region as possible.
 **/
void
eel_canvas_set_scroll_region (EelCanvas *canvas, double x1, double y1, double x2, double y2)
{
	double wxofs, wyofs;
	int xofs, yofs;

	g_return_if_fail (EEL_IS_CANVAS (canvas));

	if ((canvas->scroll_x1 == x1) && (canvas->scroll_y1 == y1) &&
	    (canvas->scroll_x2 == x2) && (canvas->scroll_y2 == y2)) {
		return;
	}
	
	/*
	 * Set the new scrolling region.  If possible, do not move the visible contents of the
	 * canvas.
	 */

	eel_canvas_c2w (canvas,
			  GTK_LAYOUT (canvas)->hadjustment->value + canvas->zoom_xofs,
			  GTK_LAYOUT (canvas)->vadjustment->value + canvas->zoom_yofs,
			  /*canvas->zoom_xofs,
			  canvas->zoom_yofs,*/
			  &wxofs, &wyofs);

	canvas->scroll_x1 = x1;
	canvas->scroll_y1 = y1;
	canvas->scroll_x2 = x2;
	canvas->scroll_y2 = y2;

	eel_canvas_w2c (canvas, wxofs, wyofs, &xofs, &yofs);

	scroll_to (canvas, xofs, yofs);

	canvas->need_repick = TRUE;

	if (!(canvas->root->object.flags & EEL_CANVAS_ITEM_NEED_DEEP_UPDATE)) {
		canvas->root->object.flags |= EEL_CANVAS_ITEM_NEED_DEEP_UPDATE;
		eel_canvas_request_update (canvas);
	}
}


/**
 * eel_canvas_get_scroll_region:
 * @canvas: A canvas.
 * @x1: Leftmost limit of the scrolling region (return value).
 * @y1: Upper limit of the scrolling region (return value).
 * @x2: Rightmost limit of the scrolling region (return value).
 * @y2: Lower limit of the scrolling region (return value).
 *
 * Queries the scrolling region of a canvas.
 **/
void
eel_canvas_get_scroll_region (EelCanvas *canvas, double *x1, double *y1, double *x2, double *y2)
{
	g_return_if_fail (EEL_IS_CANVAS (canvas));

	if (x1)
		*x1 = canvas->scroll_x1;

	if (y1)
		*y1 = canvas->scroll_y1;

	if (x2)
		*x2 = canvas->scroll_x2;

	if (y2)
		*y2 = canvas->scroll_y2;
}

void
eel_canvas_set_center_scroll_region (EelCanvas *canvas,
				     gboolean center_scroll_region)
{
	g_return_if_fail (EEL_IS_CANVAS (canvas));

	canvas->center_scroll_region = center_scroll_region != 0;

	scroll_to (canvas,
		   canvas->layout.hadjustment->value,
		   canvas->layout.vadjustment->value);
}


/**
 * eel_canvas_set_pixels_per_unit:
 * @canvas: A canvas.
 * @n: The number of pixels that correspond to one canvas unit.
 *
 * Sets the zooming factor of a canvas by specifying the number of pixels that
 * correspond to one canvas unit.
 **/
void
eel_canvas_set_pixels_per_unit (EelCanvas *canvas, double n)
{
	double cx, cy;
	int x1, y1;
	int center_x, center_y;

	g_return_if_fail (EEL_IS_CANVAS (canvas));
	g_return_if_fail (n > EEL_CANVAS_EPSILON);

	center_x = GTK_WIDGET (canvas)->allocation.width / 2;
	center_y = GTK_WIDGET (canvas)->allocation.height / 2;

	/* Find the coordinates of the screen center in units. */
	cx = (canvas->layout.hadjustment->value + center_x) / canvas->pixels_per_unit + canvas->scroll_x1 + canvas->zoom_xofs;
	cy = (canvas->layout.vadjustment->value + center_y) / canvas->pixels_per_unit + canvas->scroll_y1 + canvas->zoom_yofs;

	/* Now calculate the new offset of the upper left corner. */
	x1 = ((cx - canvas->scroll_x1) * n) - center_x;
	y1 = ((cy - canvas->scroll_y1) * n) - center_y;

	canvas->pixels_per_unit = n;

	if (!(canvas->root->object.flags & EEL_CANVAS_ITEM_NEED_DEEP_UPDATE)) {
		canvas->root->object.flags |= EEL_CANVAS_ITEM_NEED_DEEP_UPDATE;
		eel_canvas_request_update (canvas);
	}
	
	scroll_to (canvas, x1, y1);

	canvas->need_repick = TRUE;
}

/**
 * eel_canvas_scroll_to:
 * @canvas: A canvas.
 * @cx: Horizontal scrolling offset in canvas pixel units.
 * @cy: Vertical scrolling offset in canvas pixel units.
 *
 * Makes a canvas scroll to the specified offsets, given in canvas pixel units.
 * The canvas will adjust the view so that it is not outside the scrolling
 * region.  This function is typically not used, as it is better to hook
 * scrollbars to the canvas layout's scrolling adjusments.
 **/
void
eel_canvas_scroll_to (EelCanvas *canvas, int cx, int cy)
{
	g_return_if_fail (EEL_IS_CANVAS (canvas));

	scroll_to (canvas, cx, cy);
}

/**
 * eel_canvas_get_scroll_offsets:
 * @canvas: A canvas.
 * @cx: Horizontal scrolling offset (return value).
 * @cy: Vertical scrolling offset (return value).
 *
 * Queries the scrolling offsets of a canvas.  The values are returned in canvas
 * pixel units.
 **/
void
eel_canvas_get_scroll_offsets (EelCanvas *canvas, int *cx, int *cy)
{
	g_return_if_fail (EEL_IS_CANVAS (canvas));

	if (cx)
		*cx = canvas->layout.hadjustment->value;

	if (cy)
		*cy = canvas->layout.vadjustment->value;
}

/**
 * eel_canvas_update_now:
 * @canvas: A canvas.
 *
 * Forces an immediate update and redraw of a canvas.  If the canvas does not
 * have any pending update or redraw requests, then no action is taken.  This is
 * typically only used by applications that need explicit control of when the
 * display is updated, like games.  It is not needed by normal applications.
 */
void
eel_canvas_update_now (EelCanvas *canvas)
{
	g_return_if_fail (EEL_IS_CANVAS (canvas));

	if (!(canvas->need_update || canvas->need_redraw))
		return;
	remove_idle (canvas);
	do_update (canvas);
}

/**
 * eel_canvas_get_item_at:
 * @canvas: A canvas.
 * @x: X position in world coordinates.
 * @y: Y position in world coordinates.
 *
 * Looks for the item that is under the specified position, which must be
 * specified in world coordinates.
 *
 * Return value: The sought item, or NULL if no item is at the specified
 * coordinates.
 **/
EelCanvasItem *
eel_canvas_get_item_at (EelCanvas *canvas, double x, double y)
{
	EelCanvasItem *item;
	double dist;
	int cx, cy;

	g_return_val_if_fail (EEL_IS_CANVAS (canvas), NULL);

	eel_canvas_w2c (canvas, x, y, &cx, &cy);

	dist = eel_canvas_item_invoke_point (canvas->root, x, y, cx, cy, &item);
	if ((int) (dist * canvas->pixels_per_unit + 0.5) <= canvas->close_enough)
		return item;
	else
		return NULL;
}

/* Queues an update of the canvas */
static void
eel_canvas_request_update (EelCanvas *canvas)
{
	EEL_CANVAS_GET_CLASS (canvas)->request_update (canvas);
}

static void
eel_canvas_request_update_real (EelCanvas *canvas)
{
	canvas->need_update = TRUE;
	add_idle (canvas);
}

/**
 * eel_canvas_request_redraw:
 * @canvas: A canvas.
 * @x1: Leftmost coordinate of the rectangle to be redrawn.
 * @y1: Upper coordinate of the rectangle to be redrawn.
 * @x2: Rightmost coordinate of the rectangle to be redrawn, plus 1.
 * @y2: Lower coordinate of the rectangle to be redrawn, plus 1.
 *
 * Convenience function that informs a canvas that the specified rectangle needs
 * to be repainted.  The rectangle includes @x1 and @y1, but not @x2 and @y2.
 * To be used only by item implementations.
 **/
void
eel_canvas_request_redraw (EelCanvas *canvas, int x1, int y1, int x2, int y2)
{
	GdkRectangle bbox;

	g_return_if_fail (EEL_IS_CANVAS (canvas));

	if (!GTK_WIDGET_DRAWABLE (canvas) || (x1 >= x2) || (y1 >= y2)) return;

	bbox.x = x1;
	bbox.y = y1;
	bbox.width = x2 - x1;
	bbox.height = y2 - y1;

	gdk_window_invalidate_rect (canvas->layout.bin_window,
				    &bbox, FALSE);
}

/**
 * eel_canvas_w2c:
 * @canvas: A canvas.
 * @wx: World X coordinate.
 * @wy: World Y coordinate.
 * @cx: X pixel coordinate (return value).
 * @cy: Y pixel coordinate (return value).
 *
 * Converts world coordinates into canvas pixel coordinates.
 **/
void
eel_canvas_w2c (EelCanvas *canvas, double wx, double wy, int *cx, int *cy)
{
	double zoom;

	g_return_if_fail (EEL_IS_CANVAS (canvas));
	
	zoom = canvas->pixels_per_unit;
	
	if (cx)
		*cx = floor ((wx - canvas->scroll_x1)*zoom + canvas->zoom_xofs + 0.5);
	if (cy)
		*cy = floor ((wy - canvas->scroll_y1)*zoom + canvas->zoom_yofs + 0.5);
}

/**
 * eel_canvas_w2c:
 * @canvas: A canvas.
 * @world: rectangle in world coordinates.
 * @canvas: rectangle in canvase coordinates.
 *
 * Converts rectangles in world coordinates into canvas pixel coordinates.
 **/
void
eel_canvas_w2c_rect_d (EelCanvas *canvas,
			 double *x1, double *y1,
			 double *x2, double *y2)
{
	eel_canvas_w2c_d (canvas,
			    *x1, *y1,
			    x1, y1);
	eel_canvas_w2c_d (canvas,
			    *x2, *y2,
			    x2, y2);
}



/**
 * eel_canvas_w2c_d:
 * @canvas: A canvas.
 * @wx: World X coordinate.
 * @wy: World Y coordinate.
 * @cx: X pixel coordinate (return value).
 * @cy: Y pixel coordinate (return value).
 *
 * Converts world coordinates into canvas pixel coordinates.  This version
 * returns coordinates in floating point coordinates, for greater precision.
 **/
void
eel_canvas_w2c_d (EelCanvas *canvas, double wx, double wy, double *cx, double *cy)
{
	double zoom;

	g_return_if_fail (EEL_IS_CANVAS (canvas));

	zoom = canvas->pixels_per_unit;
	
	if (cx)
		*cx = (wx - canvas->scroll_x1)*zoom + canvas->zoom_xofs;
	if (cy)
		*cy = (wy - canvas->scroll_y1)*zoom + canvas->zoom_yofs;
}


/**
 * eel_canvas_c2w:
 * @canvas: A canvas.
 * @cx: Canvas pixel X coordinate.
 * @cy: Canvas pixel Y coordinate.
 * @wx: X world coordinate (return value).
 * @wy: Y world coordinate (return value).
 *
 * Converts canvas pixel coordinates to world coordinates.
 **/
void
eel_canvas_c2w (EelCanvas *canvas, int cx, int cy, double *wx, double *wy)
{
	double zoom;

	g_return_if_fail (EEL_IS_CANVAS (canvas));

	zoom = canvas->pixels_per_unit;
	
	if (wx)
		*wx = (cx - canvas->zoom_xofs)/zoom + canvas->scroll_x1;
	if (wy)
		*wy = (cy - canvas->zoom_yofs)/zoom + canvas->scroll_y1;
}


/**
 * eel_canvas_window_to_world:
 * @canvas: A canvas.
 * @winx: Window-relative X coordinate.
 * @winy: Window-relative Y coordinate.
 * @worldx: X world coordinate (return value).
 * @worldy: Y world coordinate (return value).
 *
 * Converts window-relative coordinates into world coordinates.  You can use
 * this when you need to convert mouse coordinates into world coordinates, for
 * example.
 * Window coordinates are really the same as canvas coordinates now, but this
 * function is here for backwards compatibility reasons.
 **/
void
eel_canvas_window_to_world (EelCanvas *canvas, double winx, double winy,
			      double *worldx, double *worldy)
{
	g_return_if_fail (EEL_IS_CANVAS (canvas));

	if (worldx)
		*worldx = canvas->scroll_x1 + ((winx - canvas->zoom_xofs)
					       / canvas->pixels_per_unit);

	if (worldy)
		*worldy = canvas->scroll_y1 + ((winy - canvas->zoom_yofs)
					       / canvas->pixels_per_unit);
}


/**
 * eel_canvas_world_to_window:
 * @canvas: A canvas.
 * @worldx: World X coordinate.
 * @worldy: World Y coordinate.
 * @winx: X window-relative coordinate.
 * @winy: Y window-relative coordinate.
 *
 * Converts world coordinates into window-relative coordinates.
 * Window coordinates are really the same as canvas coordinates now, but this
 * function is here for backwards compatibility reasons.
 **/
void
eel_canvas_world_to_window (EelCanvas *canvas, double worldx, double worldy,
			      double *winx, double *winy)
{
	g_return_if_fail (EEL_IS_CANVAS (canvas));

	if (winx)
		*winx = (canvas->pixels_per_unit)*(worldx - canvas->scroll_x1) + canvas->zoom_xofs;

	if (winy)
		*winy = (canvas->pixels_per_unit)*(worldy - canvas->scroll_y1) + canvas->zoom_yofs;
}



/**
 * eel_canvas_get_color:
 * @canvas: A canvas.
 * @spec: X color specification, or NULL for "transparent".
 * @color: Returns the allocated color.
 *
 * Allocates a color based on the specified X color specification.  As a
 * convenience to item implementations, it returns TRUE if the color was
 * allocated, or FALSE if the specification was NULL.  A NULL color
 * specification is considered as "transparent" by the canvas.
 *
 * Return value: TRUE if @spec is non-NULL and the color is allocated.  If @spec
 * is NULL, then returns FALSE.
 **/
int
eel_canvas_get_color (EelCanvas *canvas, const char *spec, GdkColor *color)
{
	GdkColormap *colormap;

	g_return_val_if_fail (EEL_IS_CANVAS (canvas), FALSE);
	g_return_val_if_fail (color != NULL, FALSE);

	if (!spec) {
		color->pixel = 0;
		color->red = 0;
		color->green = 0;
		color->blue = 0;
		return FALSE;
	}

	gdk_color_parse (spec, color);

	colormap = gtk_widget_get_colormap (GTK_WIDGET (canvas));

	gdk_rgb_find_color (colormap, color);

	return TRUE;
}

/**
 * eel_canvas_get_color_pixel:
 * @canvas: A canvas.
 * @rgba: RGBA color specification.
 *
 * Allocates a color from the RGBA value passed into this function.  The alpha
 * opacity value is discarded, since normal X colors do not support it.
 *
 * Return value: Allocated pixel value corresponding to the specified color.
 **/
gulong
eel_canvas_get_color_pixel (EelCanvas *canvas, guint rgba)
{
	GdkColormap *colormap;
	GdkColor color;

	g_return_val_if_fail (EEL_IS_CANVAS (canvas), 0);

	color.red = ((rgba & 0xff000000) >> 16) + ((rgba & 0xff000000) >> 24);
	color.green = ((rgba & 0x00ff0000) >> 8) + ((rgba & 0x00ff0000) >> 16);
	color.blue = (rgba & 0x0000ff00) + ((rgba & 0x0000ff00) >> 8);
	color.pixel = 0;

	colormap = gtk_widget_get_colormap (GTK_WIDGET (canvas));

	gdk_rgb_find_color (colormap, &color);

	return color.pixel;
}


/* FIXME: This function is not useful anymore */
/**
 * eel_canvas_set_stipple_origin:
 * @canvas: A canvas.
 * @gc: GC on which to set the stipple origin.
 *
 * Sets the stipple origin of the specified GC as is appropriate for the canvas,
 * so that it will be aligned with other stipple patterns used by canvas items.
 * This is typically only needed by item implementations.
 **/
void
eel_canvas_set_stipple_origin (EelCanvas *canvas, GdkGC *gc)
{
	g_return_if_fail (EEL_IS_CANVAS (canvas));
	g_return_if_fail (GDK_IS_GC (gc));

	gdk_gc_set_ts_origin (gc, 0, 0);
}

static gboolean
boolean_handled_accumulator (GSignalInvocationHint *ihint,
			     GValue                *return_accu,
			     const GValue          *handler_return,
			     gpointer               dummy)
{
	gboolean continue_emission;
	gboolean signal_handled;
	
	signal_handled = g_value_get_boolean (handler_return);
	g_value_set_boolean (return_accu, signal_handled);
	continue_emission = !signal_handled;
	
	return continue_emission;
}

/* Class initialization function for EelCanvasItemClass */
static void
eel_canvas_item_class_init (EelCanvasItemClass *class)
{
	GObjectClass *gobject_class;

	gobject_class = (GObjectClass *) class;

	item_parent_class = gtk_type_class (gtk_object_get_type ());

	gobject_class->set_property = eel_canvas_item_set_property;
	gobject_class->get_property = eel_canvas_item_get_property;

	g_object_class_install_property
		(gobject_class, ITEM_PROP_PARENT,
		 g_param_spec_object ("parent", NULL, NULL,
				      EEL_TYPE_CANVAS_ITEM,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	item_signals[ITEM_EVENT] =
		g_signal_new ("event",
			      G_TYPE_FROM_CLASS (class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EelCanvasItemClass, event),
			      boolean_handled_accumulator, NULL,
			      eel_marshal_BOOLEAN__BOXED,
			      G_TYPE_BOOLEAN, 1,
			      GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);

	gobject_class->dispose = eel_canvas_item_dispose;

	class->realize = eel_canvas_item_realize;
	class->unrealize = eel_canvas_item_unrealize;
	class->map = eel_canvas_item_map;
	class->unmap = eel_canvas_item_unmap;
	class->update = eel_canvas_item_update;
}
