/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

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
#include <gal/unicode/gunicode.h>
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cursor.h"
#include "htmlengine-search.h"
#include "htmlinterval.h"
#include "htmlsearch.h"
#include "htmlselection.h"
#include "htmltextslave.h"

static HTMLEngine *
get_root_engine (HTMLEngine *e)
{
	return e->widget->iframe_parent ? GTK_HTML (e->widget->iframe_parent)->engine : e;
}

static void
add_iframe_off (HTMLEngine *e, gint *x, gint *y)
{
	g_assert (e);
	g_assert (e->widget);

	if (e->widget->iframe_parent) {
		*x += e->widget->iframe_parent->allocation.x;
		*y += e->widget->iframe_parent->allocation.y;
	}
}

static void
move_to_found (HTMLSearch *info)
{
	HTMLEngine *e = info->engine;
	HTMLEngine *ep = get_root_engine (info->engine);
	HTMLObject *first = HTML_OBJECT (info->found->data);
	HTMLObject *last = HTML_OBJECT (g_list_last (info->found)->data);
	HTMLTextSlave *slave;
	gint x, y, ex, ey, w, h;
	gint nx = e->x_offset;
	gint ny = e->y_offset;

	/* x,y is top-left corner, ex+w,ey+h is bottom-right */
	html_object_calc_abs_position (first, &x, &y);
	add_iframe_off (e, &x, &y);

	/* find slave where starts selection and get its coordinates as upper-left corner */
	while (first->next && HTML_OBJECT_TYPE (first->next) == HTML_TYPE_TEXTSLAVE) {
		first = first->next;
		slave = HTML_TEXT_SLAVE (first);
		if (slave->posStart+slave->posLen >= info->start_pos) {
			html_object_calc_abs_position (HTML_OBJECT (slave), &x, &y);
			add_iframe_off (e, &x, &y);
			break;
		}
	}

	/* the same with last */
	html_object_calc_abs_position (last, &ex, &ey);

	while (last->next && HTML_OBJECT_TYPE (last->next) == HTML_TYPE_TEXTSLAVE) {
		last = last->next;
		slave = HTML_TEXT_SLAVE (last);
		if (slave->posStart+slave->posLen >= info->start_pos) {
			html_object_calc_abs_position (HTML_OBJECT (slave), &ex, &ey);
			add_iframe_off (e, &ex, &ey);
			break;
		}
	}

	y  -= first->ascent;
	ex += last->width;
	ey += last->descent;
	w = ex - x;
	h = ey - y;

	/* now calculate gtkhtml adustments */
	if (x <= ep->x_offset)
		nx = x;
	else if (x + w > ep->x_offset + ep->width)
		nx = x + w - ep->width;

	if (y <= ep->y_offset)
		ny = y;
	else if (y + h > ep->y_offset + ep->height)
		ny = y + h - ep->height;

	/* finally adjust them if they changed */
	if (ep->x_offset != nx)
		gtk_adjustment_set_value (GTK_LAYOUT (ep->widget)->hadjustment, nx);

	if (ep->y_offset != ny)
		gtk_adjustment_set_value (GTK_LAYOUT (ep->widget)->vadjustment, ny);
}

static void
display_search_results (HTMLSearch *info)
{
	HTMLEngine *e = info->engine;

	if (!info->found)
		return;
	if (e->editable) {
		html_engine_hide_cursor (e);
		html_engine_disable_selection (e);
		html_cursor_jump_to (e->cursor, e, HTML_OBJECT (info->found->data), info->start_pos);
		html_engine_set_mark (e);
		html_cursor_jump_to (e->cursor, e, info->last, info->stop_pos);
		html_engine_show_cursor (e);
	} else {
		html_engine_select_interval (e, html_interval_new (HTML_OBJECT (info->found->data), info->last,
								   info->start_pos, info->stop_pos));
		move_to_found (info);
	}
}

gboolean
html_engine_search (HTMLEngine *e, const gchar *text,
		    gboolean case_sensitive, gboolean forward, gboolean regular)
{
	HTMLSearch *info;
	HTMLObject *p;

	if (e->search_info) {
		html_search_destroy (e->search_info);
	}

	info = e->search_info = html_search_new (e, text, case_sensitive, forward, regular);

	p = HTML_OBJECT (e->search_info->stack->data)->parent;
	if (html_object_search (p ? p : e->clue, info)) {
		display_search_results (info);
		return TRUE;
	} else
		return FALSE;
}

void
html_engine_search_set_forward (HTMLEngine *e, gboolean forward)
{
	html_search_set_forward (e->search_info, forward);
}

gboolean
html_engine_search_next (HTMLEngine *e)
{
	HTMLSearch *info = e->search_info;
	gboolean retval = FALSE;

	if (!info)
		return FALSE;

	if (html_engine_get_editable (e)) {
		gchar *text = g_strdup (info->text);

		retval = html_engine_search (e, text, info->case_sensitive, info->forward, info->regular);
		g_free (text);
	} else {
		if (info->stack)
			retval = html_object_search (HTML_OBJECT (info->stack->data), info);
		else {
			html_search_push (info, e->clue);
			retval = html_object_search (e->clue, info);
		}
		if (retval)
			display_search_results (info);
		else {
			html_search_pop (info);
			html_engine_disable_selection (e);
		}
	}

	return retval;
}

gboolean
html_engine_search_incremental (HTMLEngine *e, const gchar *text, gboolean forward)
{
	HTMLSearch *info = e->search_info;	

	if (info) {
		html_search_set_forward (info, forward);
		html_search_set_text (info, text);
		if (info->found)
			info->start_pos += ((info->forward) ? -1 : g_utf8_strlen (text, -1));
		return html_engine_search_next (e);
	} else
		return html_engine_search (e, text, FALSE, forward, FALSE);
}
