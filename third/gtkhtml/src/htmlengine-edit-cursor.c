/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

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
#include <gtk/gtkmain.h>
#include "gtkhtml-im.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit-cursor.h"
#include "htmlengine-edit-table.h"
#include "htmlengine-edit-tablecell.h"
#include "htmlimage.h"
#include "htmlobject.h"
#include "htmltable.h"


#define BLINK_TIMEOUT 500

static GdkColor table_stipple_active_on      = { 0, 0,      0,      0xffff };
static GdkColor table_stipple_active_off     = { 0, 0xffff, 0xffff, 0xffff };
static GdkColor table_stipple_non_active_on  = { 0, 0xaaaa, 0xaaaa, 0xaaaa };
static GdkColor table_stipple_non_active_off = { 0, 0xffff, 0xffff, 0xffff };

static GdkColor cell_stipple_active_on      = { 0, 0xffff, 0xffff, 0 };
static GdkColor cell_stipple_active_off     = { 0, 0,      0,      0 };
static GdkColor cell_stipple_non_active_on  = { 0, 0xaaaa, 0xaaaa, 0xaaaa };
static GdkColor cell_stipple_non_active_off = { 0, 0xffff, 0xffff, 0xffff };

static GdkColor image_stipple_active_on      = { 0, 0xffff, 0,      0 };
static GdkColor image_stipple_active_off     = { 0, 0,      0,      0 };


void
html_engine_hide_cursor  (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	if (engine->editable && engine->cursor_hide_count == 0)
		html_engine_draw_cursor_in_area (engine, 0, 0, -1, -1);

	engine->cursor_hide_count++;
}

void
html_engine_show_cursor  (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	if (engine->cursor_hide_count > 0) {
		engine->cursor_hide_count--;
		if (engine->editable && engine->cursor_hide_count == 0)
			html_engine_draw_cursor_in_area (engine, 0, 0, -1, -1);
	}
}

static inline void
move_rect (HTMLEngine *engine, gint *x1, gint *y1, gint *x2, gint *y2)
{
	*x1 = *x1 + engine->leftBorder - engine->x_offset;
	*y1 = *y1 + engine->topBorder - engine->y_offset;
	*x2 = *x2 + engine->leftBorder - engine->x_offset;
	*y2 = *y2 + engine->topBorder - engine->y_offset;
}

static gboolean
clip_rect (HTMLEngine *engine, gint x, gint y, gint width, gint height, gint *x1, gint *y1, gint *x2, gint *y2)
{
	if (*x1 >= x + width || *y1 >= y + height || *x2 < x || *y2 < y)
		return FALSE;

	if (*x2 >= x + width)
		*x2 = x + width - 1;
	if (*y2 >= y + height)
		*y2 = y + height - 1;

	if (*x1 < x)
		*x1 = x;
	if (*y1 < y)
		*y1 = y;

	return TRUE;
}

static void
draw_cursor_rectangle (HTMLEngine *e, gint x1, gint y1, gint x2, gint y2,
		       GdkColor *on_color, GdkColor *off_color,
		       gint offset)
{
	GdkGC *gc;
	GdkColor color;
	gint8 dashes [2] = { 2, 2 };

	move_rect (e, &x1, &y1, &x2, &y2);

	gc = gdk_gc_new (e->window);
	color = *on_color;
	gdk_color_alloc (gdk_window_get_colormap (e->window), &color);
	gdk_gc_set_foreground (gc, &color);
	color = *off_color;
	gdk_color_alloc (gdk_window_get_colormap (e->window), &color);
	gdk_gc_set_background (gc, &color);
	gdk_gc_set_line_attributes (gc, 1, GDK_LINE_DOUBLE_DASH, GDK_CAP_ROUND, GDK_JOIN_ROUND);
	gdk_gc_set_dashes (gc, offset, dashes, 2);
	gdk_draw_rectangle (e->window, gc, 0, x1, y1, x2 - x1, y2 - y1);
	gdk_gc_unref (gc);
}

static gint cursor_enabled = TRUE;

static inline void
refresh_under_cursor (HTMLEngine *e, HTMLCursorRectangle *cr, gboolean *enabled)
{
	*enabled = cursor_enabled = FALSE;
	html_engine_draw (e, e->leftBorder + cr->x1 - e->x_offset, e->topBorder + cr->y1 - e->y_offset,
			  cr->x2 - cr->x1 + 1, cr->y2 - cr->y1 + 1);
	*enabled = cursor_enabled = TRUE;
}

#define COLORS(x) animate ? &x ## _stipple_active_on  : &x ## _stipple_non_active_on, \
                  animate ? &x ## _stipple_active_off : &x ## _stipple_non_active_off

static void
html_engine_draw_image_cursor (HTMLEngine *e)
{
	HTMLCursorRectangle *cr;
	HTMLObject *io;
	static gboolean enabled = TRUE;

	if (!enabled)
		return;

	cr    = &e->cursor_image;
	io    = e->cursor->object;

	if (io && HTML_IS_IMAGE (e->cursor->object)) {
		HTMLImage *image;
		static gint offset = 3;

		image = HTML_IMAGE (io);
		if (io != cr->object) {
			if (cr->object)
				refresh_under_cursor (e, cr, &enabled);
			cr->object = io;
		}

		html_object_calc_abs_position (io, &cr->x1, &cr->y2);
		cr->x2  = cr->x1 + io->width - 1;
		cr->y2 --;
		cr->y1  = cr->y2 - (io->ascent + io->descent) + 1;

		draw_cursor_rectangle (e, cr->x1, cr->y1, cr->x2, cr->y2,
				       &image_stipple_active_on, &image_stipple_active_off, offset);
		if (!offset)
			offset = 3;
		else
			offset--;
	} else
		if (cr->object) {
			refresh_under_cursor (e, cr, &enabled);
			cr->object = NULL;
		}
}

void
html_engine_draw_cell_cursor (HTMLEngine *e)
{
	HTMLCursorRectangle *cr;
	HTMLTableCell *cell;
	HTMLObject    *co;
	static gboolean enabled = TRUE;

	if (!enabled)
		return;

	cr   = &e->cursor_cell;
	cell = html_engine_get_table_cell (e);
	co   = HTML_OBJECT (cell);

	if (cell) {
		static gint offset = 0;
		gboolean animate;

		if (co != cr->object) {
			if (cr->object)
				refresh_under_cursor (e, cr, &enabled);
			cr->object = co;
		}

		html_object_calc_abs_position (co, &cr->x1, &cr->y2);
		cr->x2  = cr->x1 + co->width - 1;
		cr->y2 -= 2;
		cr->y1  = cr->y2 - (co->ascent + co->descent - 2);

		animate = !HTML_IS_IMAGE (e->cursor->object);
		if (animate) {
			offset++;
			offset %= 4;
		}
		draw_cursor_rectangle (e, cr->x1, cr->y1, cr->x2, cr->y2, COLORS (cell), offset);
	} else
		if (cr->object) {
			refresh_under_cursor (e, cr, &enabled);
			cr->object = NULL;
		}
}

void
html_engine_draw_table_cursor (HTMLEngine *e)
{
	HTMLCursorRectangle *cr;
	HTMLTable *table;
	HTMLObject *to;
	static gboolean enabled = TRUE;

	if (!enabled)
		return;

	cr    = &e->cursor_table;
	table = html_engine_get_table (e);
	to    = HTML_OBJECT (table);

	if (table) {
		static gint offset = 0;
		gboolean animate;

		if (to != cr->object) {
			if (cr->object)
				refresh_under_cursor (e, cr, &enabled);
			cr->object = to;
		}

		html_object_calc_abs_position (to, &cr->x1, &cr->y2);
		cr->x2 = cr->x1 + to->width - 1;
		cr->y2 --;
		cr->y1 = cr->y2 - (to->ascent + to->descent - 1);

		animate = HTML_IS_TABLE (e->cursor->object) && !html_engine_get_table_cell (e);
		if (animate) {
			offset++;
			offset %= 4;
		}
		draw_cursor_rectangle (e, cr->x1, cr->y1, cr->x2, cr->y2, COLORS (table), offset);
	} else
		if (cr->object) {
			refresh_under_cursor (e, cr, &enabled);
			cr->object = NULL;
		}
}

void
html_engine_draw_cursor_in_area (HTMLEngine *engine,
				 gint x, gint y,
				 gint width, gint height)
{
	HTMLObject *obj;
	guint offset;
	gint x1, y1, x2, y2;

	g_assert (engine->editable);

	if (engine->editable && (engine->cursor_hide_count <= 0 && !engine->thaw_idle_id)) {
		html_engine_draw_table_cursor (engine);
		html_engine_draw_cell_cursor (engine);
		html_engine_draw_image_cursor (engine);
	}

	if (!cursor_enabled || engine->cursor_hide_count > 0 || ! engine->editable || engine->thaw_idle_id)
		return;

	obj = engine->cursor->object;
	if (obj == NULL)
		return;

	offset = engine->cursor->offset;

	if (width < 0 || height < 0) {
		width = engine->width;
		height = engine->height;
		x = 0;
		y = 0;
	}

	html_object_get_cursor (obj, engine->painter, offset, &x1, &y1, &x2, &y2);
	move_rect (engine, &x1, &y1, &x2, &y2);
	if (clip_rect (engine, x, y, width, height, &x1, &y1, &x2, &y2)) {
		gdk_draw_line (engine->window, engine->invert_gc, x1, y1, x2, y2);
#ifdef GTK_HTML_USE_XIM
		gtk_html_im_position_update (engine->widget, x2, y2);
#endif
	}
}


/* Blinking cursor implementation.  */

static gint
blink_timeout_cb (gpointer data)
{
	HTMLEngine *engine;

	g_return_val_if_fail (HTML_IS_ENGINE (data), FALSE);

	engine = HTML_ENGINE (data);

	engine->blinking_status = ! engine->blinking_status;

	if (engine->blinking_status)
		html_engine_show_cursor (engine);
	else
		html_engine_hide_cursor (engine);

	return TRUE;
}

void
html_engine_setup_blinking_cursor (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));
	g_return_if_fail (engine->blinking_timer_id == 0);

	html_engine_show_cursor (engine);
	engine->blinking_status = FALSE;

	blink_timeout_cb (engine);
	engine->blinking_timer_id = gtk_timeout_add (BLINK_TIMEOUT, blink_timeout_cb, engine);
}

void
html_engine_stop_blinking_cursor (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));
	g_return_if_fail (engine->blinking_timer_id != 0);

	if (engine->blinking_status) {
		html_engine_hide_cursor (engine);
		engine->blinking_status = FALSE;
	}

	gtk_timeout_remove (engine->blinking_timer_id);
	engine->blinking_timer_id = 0;
}

void
html_engine_reset_blinking_cursor (HTMLEngine *engine)
{
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));
	g_return_if_fail (engine->blinking_timer_id != 0);

	if (engine->blinking_status)
		return;

	html_engine_show_cursor (engine);
	engine->blinking_status = TRUE;
	gtk_timeout_remove (engine->blinking_timer_id);
	engine->blinking_timer_id = gtk_timeout_add (BLINK_TIMEOUT, blink_timeout_cb, engine);
}
