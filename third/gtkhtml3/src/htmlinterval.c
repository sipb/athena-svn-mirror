/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2000 Helix Code, Inc.
   Authors:           Radek Doulik (rodo@helixcode.com)

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
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlinterval.h"
#include "htmlobject.h"
#include "htmlselection.h"

inline void
html_point_construct (HTMLPoint *p, HTMLObject *o, guint off)
{
	p->object = o;
	p->offset = off;
}

HTMLPoint *
html_point_new (HTMLObject *o, guint off)
{
	HTMLPoint *np = g_new (HTMLPoint, 1);
	html_point_construct (np, o, off);

	return np;
}

inline void
html_point_destroy (HTMLPoint *p)
{
	g_free (p);
}

gboolean
html_point_cursor_object_eq (HTMLPoint *p, HTMLPoint *c)
{
	return p->object == c->object && (!html_object_is_container (p->object) || p->offset == c->offset);
}

inline void
html_point_next_cursor (HTMLPoint *p)
{
	p->object = html_object_next_cursor (p->object, &p->offset);
}

HTMLInterval *
html_interval_new (HTMLObject *from, HTMLObject *to, guint from_offset, guint to_offset)
{
	HTMLInterval *i = g_new (HTMLInterval, 1);

	html_point_construct (&i->from, from, from_offset);
	html_point_construct (&i->to,   to,   to_offset);

	return i;
}

inline HTMLInterval *
html_interval_new_from_cursor (HTMLCursor *a, HTMLCursor *b)
{
	HTMLCursor *begin, *end;

	if (html_cursor_get_position (a) < html_cursor_get_position (b)) {
		begin = a;
		end   = b;
	} else {
		begin = b;
		end   = a;
	}

	return html_interval_new (begin->object, end->object, begin->offset, end->offset);
}

inline HTMLInterval *
html_interval_new_from_points (HTMLPoint *from, HTMLPoint *to)
{
	return html_interval_new (from->object, to->object, from->offset, to->offset);
}

void
html_interval_destroy (HTMLInterval *i)
{
	g_free (i);
}

guint
html_interval_get_length (HTMLInterval *i, HTMLObject *obj)
{
	if (obj != i->from.object && obj != i->to.object)
		return html_object_get_length (obj);	
	if (obj == i->from.object) {
		if (obj == i->to.object)
			return i->to.offset - i->from.offset;
		else
			return html_object_get_length (obj) - i->from.offset;
	} else
		return i->to.offset;
}

guint
html_interval_get_bytes (HTMLInterval *i, HTMLObject *obj)
{
	if (obj != i->from.object && obj != i->to.object)
		return html_object_get_bytes (obj);
	if (obj == i->from.object) {
		if (obj == i->to.object)
			return html_interval_get_to_index (i) - html_interval_get_from_index (i);
		else
			return html_object_get_bytes (obj) - html_interval_get_from_index (i);
	} else
		return html_interval_get_to_index (i);
}

guint
html_interval_get_start (HTMLInterval *i, HTMLObject *obj)
{
	return (obj != i->from.object) ? 0 : i->from.offset;
}

guint
html_interval_get_start_index (HTMLInterval *i, HTMLObject *obj)
{
	return (obj != i->from.object) ? 0 : html_interval_get_from_index (i);
}

static void
select_object (HTMLObject *o, HTMLEngine *e, gpointer data)
{
	HTMLInterval *i = (HTMLInterval *) data;
	HTMLEngine *etop = html_engine_get_top_html_engine (e);

	if (o == i->from.object)
		etop->selected_in = TRUE;
	if (etop->selected_in) {
		gint len;

		len = html_interval_get_length (i, o);
		if (len)
			html_object_select_range (o, e,
						  html_interval_get_start (i, o), len,
						  !html_engine_frozen (e));
	}

	if (o == i->to.object)
		etop->selected_in = FALSE;
}

void
html_interval_select (HTMLInterval *i, HTMLEngine *e)
{
	html_engine_get_top_html_engine (e)->selected_in = FALSE;
	i = html_interval_flat (i);
	html_interval_forall (i, e, select_object, i);
	html_interval_destroy (i);
}

static void
unselect_object (HTMLObject *o, HTMLEngine *e, gpointer data)
{
	if (html_interval_get_length ((HTMLInterval *) data, o))
		html_object_select_range (o, e, 0, 0, !html_engine_frozen (e));
}

void
html_interval_unselect (HTMLInterval *i, HTMLEngine *e)
{
	i = html_interval_flat (i);
	html_interval_forall (i, e, unselect_object, i);
	html_interval_destroy (i);
}

gint
html_interval_get_from_index (HTMLInterval *i)
{
	g_assert (i);

	return html_object_get_index (i->from.object, i->from.offset);
}

gint
html_interval_get_to_index (HTMLInterval *i)
{
	g_assert (i);

	return html_object_get_index (i->to.object, i->to.offset);
}

static GSList *
get_downtree_line (HTMLObject *o)
{
	GSList *list = NULL;

	while (o) {
		list = g_slist_prepend (list, o);
		o = o->parent;
	}

	return list;
}

static HTMLEngine *
do_downtree_lines_intersection (GSList **l1, GSList **l2, HTMLEngine *e)
{
	g_assert ((*l1)->data == (*l2)->data);

	while (*l1 && *l2 && (*l1)->data == (*l2)->data) {
		e = html_object_get_engine (HTML_OBJECT ((*l1)->data), e);
		*l1 = g_slist_remove_link (*l1, *l1);
		*l2 = g_slist_remove_link (*l2, *l2);
	}

	return e;
}

static void
interval_forall (HTMLObject *parent, GSList *from_down, GSList *to_down, HTMLEngine *e, HTMLObjectForallFunc f, gpointer data)
{
	HTMLObject *o, *from, *to;

	from = from_down ? HTML_OBJECT (from_down->data) : html_object_head (parent);
	to   = to_down   ? HTML_OBJECT (to_down->data)   : NULL;

	for (o = from; o; o = html_object_next_not_slave (o)) {
		interval_forall (o,
				 (from_down && o == HTML_OBJECT (from_down->data)) ? from_down->next : NULL,
				 (to_down   && o == HTML_OBJECT (to_down->data))   ? to_down->next   : NULL,
				 html_object_get_engine (o, e), f, data);
		if (o == to)
			break;
	}
	(*f) (parent, e, data);
}

static void
html_point_to_leaf (HTMLPoint *p)
{
	if (html_object_is_container (p->object)) {
		if (p->offset == 0)
			p->object = html_object_get_head_leaf (p->object);
		else if (p->offset == html_object_get_length (p->object)) {
			p->object = html_object_get_tail_leaf (p->object);
			p->offset = html_object_get_length (p->object);
		} else
			g_warning ("Can't transform point to leaf\n");
	}
}

static inline HTMLInterval *
html_interval_dup (HTMLInterval *i)
{
	return html_interval_new_from_points (&i->from, &i->to);
}

HTMLInterval *
html_interval_flat (HTMLInterval *i)
{
	HTMLInterval *ni = html_interval_dup (i);

	html_point_to_leaf (&ni->from);
	html_point_to_leaf (&ni->to);

	return ni;
}

void
html_interval_forall (HTMLInterval *i, HTMLEngine *e, HTMLObjectForallFunc f, gpointer data)
{
	GSList *from_downline, *to_downline;
	HTMLEngine *engine;

	g_return_if_fail (i->from.object);
	g_return_if_fail (i->to.object);

	i = html_interval_flat (i);

	from_downline = get_downtree_line (i->from.object);
	to_downline   = get_downtree_line (i->to.object);
	engine = do_downtree_lines_intersection  (&from_downline, &to_downline, e);

	if (from_downline)
		interval_forall    (HTML_OBJECT (from_downline->data)->parent, from_downline, to_downline,
				    html_object_get_engine (HTML_OBJECT (from_downline->data)->parent, engine), f, data);
	else {
		g_assert (i->from.object == i->to.object);
		html_object_forall (i->from.object, html_object_get_engine (i->from.object, engine), f, data);
	}

	g_slist_free (from_downline);
	g_slist_free (to_downline);
	html_interval_destroy (i);
}

static HTMLObject *
html_object_children_max (HTMLObject *a, HTMLObject *b)
{
	HTMLObject *o;

	g_return_val_if_fail (a->parent, NULL);
	g_return_val_if_fail (b->parent, NULL);
	g_return_val_if_fail (a->parent == b->parent, NULL);

	for (o=a; o; o = html_object_next_not_slave (o))
		if (o == b)
			return b;
	return a;
}

static inline HTMLObject *
html_object_children_min (HTMLObject *a, HTMLObject *b)
{
	return a == html_object_children_max (b, a) ? b : a;
}

HTMLPoint *
html_point_max (HTMLPoint *a, HTMLPoint *b)
{
	GSList *a_downline, *b_downline;
	HTMLPoint *rv = NULL;

	if (a->object == b->object)
		return a->offset < b->offset ? b : a;

	a_downline = get_downtree_line (a->object);
	b_downline = get_downtree_line (b->object);
	do_downtree_lines_intersection (&a_downline, &b_downline, NULL);

	if (a_downline == NULL)
		/* it means that a is parent (container) of b */
		rv = a->offset ? a : b;
	else if (b_downline == NULL)
		/* it means that b is parent (container) of a */
		rv = b->offset ? b : a;
	else
		rv = html_object_children_max (HTML_OBJECT (a_downline->data), HTML_OBJECT (b_downline->data))
			== HTML_OBJECT (a_downline->data) ? a : b;
	g_slist_free (a_downline);
	g_slist_free (b_downline);

	return rv;
}

inline HTMLPoint *
html_point_min (HTMLPoint *a, HTMLPoint *b)
{
	return a == html_point_max (a, b) ? b : a;
}

static HTMLPoint *
max_from (HTMLInterval *a, HTMLInterval *b)
{
	if (!a->from.object)
		return &b->from;
	if (!b->from.object)
		return &a->from;

	return html_point_max (&a->from, &b->from);
}

static HTMLPoint *
min_to (HTMLInterval *a, HTMLInterval *b)
{
	if (!a->to.object)
		return &b->to;
	if (!b->to.object)
		return &a->to;

	return html_point_min (&a->to, &b->to);
}

HTMLInterval *
html_interval_intersection (HTMLInterval *a, HTMLInterval *b)
{
	HTMLPoint *from, *to;

	from = max_from (a, b);
	to   = min_to   (a, b);

	return to == html_point_max (from, to) ?
		html_interval_new_from_points (from, to) : NULL;
}

void *
html_interval_substract (HTMLInterval *a, HTMLInterval *b, HTMLInterval **s1, HTMLInterval **s2)
{
	return NULL;
}

void
html_interval_validate (HTMLInterval *i)
{
	if (&i->from == html_point_max (&i->from, &i->to)) {
		HTMLPoint tmp;

		tmp     = i->from;
		i->from = i->to;
		i->to   = tmp;
	}
}

gboolean
html_point_eq (const HTMLPoint *a, const HTMLPoint *b)
{
	return a->object == b->object && a->offset == b->offset;
}

gboolean
html_interval_eq (const HTMLInterval *a, const HTMLInterval *b)
{
	return html_point_eq (&a->from, &b->from) && html_point_eq (&a->to, &b->to);
}

HTMLObject *
html_interval_get_head (HTMLInterval *i, HTMLObject *o)
{
	return o == i->from.object->parent ? i->from.object : html_object_head (o);
}
