/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.

   Copyright (C) 1999, 2000 Helix Code, Inc.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <gtk/gtksignal.h>
#include "htmlengine-edit-cursor.h"
#include "htmlcolor.h"
#include "htmldrawqueue.h"
#include "htmlengine.h"
#include "htmlgdkpainter.h"
#include "htmlimage.h"
#include "htmlpainter.h"
#include "htmlobject.h"
#include "htmltextslave.h"
#include "htmlsettings.h"
#include "gtkhtml.h"


/* HTMLDrawQueueClearElement handling.  */

static HTMLDrawQueueClearElement *
clear_element_new (gint            x,
		   gint            y,
		   guint           width,
		   guint           height,
		   const GdkColor *background_color)
{
	HTMLDrawQueueClearElement *new;

	new = g_new (HTMLDrawQueueClearElement, 1);

	new->x = x;
	new->y = y;
	new->width = width;
	new->height = height;

	/* GDK color API non const-correct.  */
	new->background_color = gdk_color_copy ((GdkColor *) background_color);

	new->background_image = NULL;
	new->background_image_x_offset = 0;
	new->background_image_y_offset = 0;

	return new;
}

static void
clear_element_destroy (HTMLDrawQueueClearElement *elem)
{
	g_return_if_fail (elem != NULL);

	if (elem->background_color != NULL)
		gdk_color_free (elem->background_color);

	if (elem->background_image != NULL)
		gdk_pixbuf_unref (elem->background_image);

	g_free (elem);
}


HTMLDrawQueue *
html_draw_queue_new (HTMLEngine *engine)
{
	HTMLDrawQueue *new;

	g_return_val_if_fail (engine != NULL, NULL);

	new = g_new (HTMLDrawQueue, 1);

	new->engine = engine;

	new->elems = NULL;
	new->last = NULL;

	new->clear_elems = NULL;
	new->clear_last = NULL;

	return new;
}

void
html_draw_queue_destroy (HTMLDrawQueue *queue)
{
	GList *p;

	g_return_if_fail (queue != NULL);

	for (p = queue->elems; p != NULL; p = p->next) {
		HTMLObject *obj;

		obj = p->data;
		obj->redraw_pending = FALSE;
	}

	g_list_free (queue->elems);

	g_free (queue);
}

void
html_draw_queue_add (HTMLDrawQueue *queue, HTMLObject *object)
{
	g_return_if_fail (queue != NULL);
	g_return_if_fail (object != NULL);

	if (object->redraw_pending)
		return;

	object->redraw_pending = TRUE;

	queue->last = g_list_append (queue->last, object);

	if (queue->elems == NULL && queue->clear_elems == NULL)
		g_signal_emit_by_name (queue->engine, "draw_pending");

	if (queue->elems == NULL)
		queue->elems = queue->last;
	else
		queue->last = queue->last->next;
}


static void
add_clear (HTMLDrawQueue *queue,
	   HTMLDrawQueueClearElement *elem)
{
	queue->clear_last = g_list_append (queue->clear_last, elem);
	if (queue->elems == NULL && queue->clear_elems == NULL)
		g_signal_emit_by_name (queue->engine, "draw_pending");

	if (queue->clear_elems == NULL)
		queue->clear_elems = queue->clear_last;
	else
		queue->clear_last = queue->clear_last->next;
}

void
html_draw_queue_add_clear (HTMLDrawQueue *queue,
			   gint x,
			   gint y,
			   guint width,
			   guint height,
			   const GdkColor *background_color)
{
	HTMLDrawQueueClearElement *new;

	g_return_if_fail (queue != NULL);
	g_return_if_fail (background_color != NULL);

	new = clear_element_new (x, y, width, height, background_color);
	add_clear (queue, new);
}

static void
draw_obj (HTMLDrawQueue *queue,
	  HTMLObject *obj)
{
	HTMLEngine *e;
	gint x1, y1, x2, y2;
	gint tx, ty;

	if (obj->width == 0 || obj->ascent + obj->descent == 0)
		return;

	e = queue->engine;
	e->clue->x = html_engine_get_left_border (e);
	e->clue->y = html_engine_get_top_border (e) + e->clue->ascent;

	html_object_engine_translation (obj, e, &tx, &ty);
	if (html_object_engine_intersection (obj, e, tx, ty, &x1, &y1, &x2, &y2)) {
		GdkRectangle paint;

		paint.x = x1;
		paint.y = y1;
		paint.width = x2 - x1;
		paint.height = y2 - y1;
		gdk_window_invalidate_rect (HTML_GDK_PAINTER (e->painter)->window, &paint, FALSE);
	} 
}

static void
clear (HTMLDrawQueue *queue,
       HTMLDrawQueueClearElement *elem)
{
	HTMLEngine *e;
	gint x1, y1, x2, y2;

	e = queue->engine;
	e->clue->x = html_engine_get_left_border (e);
	e->clue->y = html_engine_get_top_border (e) + e->clue->ascent;

	x1 = elem->x;
	y1 = elem->y;

	x2 = x1 + elem->width;
	y2 = y1 + elem->height;

	if (html_engine_intersection (e, &x1, &y1, &x2, &y2)) {
		GdkRectangle paint;

		paint.x = x1;
		paint.y = y1;
		paint.width = x2 - x1;
		paint.height = y2 - y1;
		gdk_window_invalidate_rect (HTML_GDK_PAINTER (e->painter)->window, &paint, FALSE);
	} 
}

void
html_draw_queue_clear (HTMLDrawQueue *queue)
{
	GList *p;

	for (p = queue->elems; p != NULL; p = p->next) {
		HTMLObject *obj = HTML_OBJECT (p->data);
			
		obj->redraw_pending = FALSE;
		if (obj->free_pending) {
			g_free (obj);
			p->data = (gpointer)0xdeadbeef;
		}
	}

	g_list_free (queue->clear_elems);
	g_list_free (queue->elems);

	queue->clear_elems = NULL;
	queue->clear_last = NULL;
	queue->elems = NULL;
	queue->last = NULL;
}

void
html_draw_queue_flush (HTMLDrawQueue *queue)
{
	GList *p;
	GdkVisual *vis;

	/* check to make sure we have something to draw on */

	vis = queue->engine->window ? gdk_drawable_get_visual (queue->engine->window): NULL;

	/* Draw clear areas.  */

	for (p = queue->clear_elems; p != NULL; p = p->next) {
		HTMLDrawQueueClearElement *clear_elem;

		clear_elem = p->data;
		if (vis)
			clear (queue, clear_elem);
		clear_element_destroy (clear_elem);
	}


	/* Draw objects.  */

	if (vis) {
		for (p = queue->elems; p != NULL; p = p->next) {
			HTMLObject *obj = HTML_OBJECT (p->data);
			
			if (obj->redraw_pending && !obj->free_pending) {
				draw_obj (queue, obj);
				obj->redraw_pending = FALSE;
			}
		}
	}
	html_draw_queue_clear (queue);
}









