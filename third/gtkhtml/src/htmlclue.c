/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 1997 Torben Weis (weis@kde.org)
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
#include "htmlclue.h"
#include "htmlobject.h"
#include "htmlsearch.h"
#include "htmltype.h"


#define HC_CLASS(x) (HTML_CLUE_CLASS (HTML_OBJECT (x)->klass))

HTMLClueClass html_clue_class;
static HTMLObjectClass *parent_class = NULL;

static void set_parent (HTMLObject *o, HTMLObject *tail, HTMLObject *parent);


/* HTMLObject methods.  */

static void
destroy (HTMLObject *o)
{
	HTMLObject *p;
	HTMLObject *next;

	for (p = HTML_CLUE (o)->head; p != NULL; p = next) {
		next = p->next;
		html_object_destroy (p);
	}
	HTML_CLUE (o)->head = NULL;
	HTML_CLUE (o)->tail = NULL;
			
	HTML_OBJECT_CLASS (parent_class)->destroy (o);
}

static guint
get_recursive_length (HTMLObject *self)
{
	HTMLObject *o = HTML_CLUE (self)->head;
	guint len = 0;

	while (o) {
		len += html_object_get_recursive_length (o);
		o = o->next;
	}

	return len;
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	HTML_CLUE (dest)->head = NULL;
	HTML_CLUE (dest)->tail = NULL;
	HTML_CLUE (dest)->curr = NULL;

	HTML_CLUE (dest)->valign = HTML_CLUE (self)->valign;
	HTML_CLUE (dest)->halign = HTML_CLUE (self)->halign;
}

static HTMLObject *
op_helper (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right, guint *len, gboolean cut)
{
	HTMLClue *clue = HTML_CLUE (self);
	HTMLObject *cc;
	HTMLObject *o, *last, *cnext, *child;

	cc   = html_object_dup (self);
	o    = (from) ? HTML_OBJECT (from->data) : clue->head;
	last = (to)   ? HTML_OBJECT (to->data)   : clue->tail;

	if ((o == NULL) && (last == NULL))
		return cc;

	g_assert (o->parent == self);
	g_assert (last->parent == self);

	while (1) {
		cnext = html_object_next_not_slave (o);
		child = cut ? html_object_op_cut (o, e,
					      html_object_get_bound_list (o, from),
					      html_object_get_bound_list (o, to),
					      left ? left->next : NULL, right ? right->next : NULL, len)
			: html_object_op_copy (o, e,
					       html_object_get_bound_list (o, from),
					       html_object_get_bound_list (o, to), len);
		if (child)
			html_clue_append (HTML_CLUE (cc), child);

		if (o == last)
			break;
		o = cnext;
	}

	return cc;
}

static HTMLObject *
op_copy (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, guint *len)
{
	return op_helper (self, e, from, to, NULL, NULL, len, FALSE);
}

static HTMLObject *
op_cut (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right, guint *len)
{
	HTMLObject *rv;
	HTMLClue *clue;

	clue = HTML_CLUE (self);
	rv   = op_helper (self, e, from, to, left, right, len, TRUE);
	if (!clue->head) {
		if (self->parent)
			html_object_remove_child (self->parent, self);
		html_object_destroy (self);
	} else
		html_object_change_set (self, HTML_CHANGE_ALL_CALC);

	return rv;
}

static gboolean
merge (HTMLObject *self, HTMLObject *with, HTMLEngine *e, GList *left, GList *right)
{
	HTMLClue   *clue1, *clue2;

	clue1 = HTML_CLUE (self);
	clue2 = HTML_CLUE (with);

	html_clue_append (clue1, clue2->head);
	clue2->head = NULL;
	clue2->tail = NULL;

	html_object_change_set (self, HTML_CHANGE_ALL_CALC);
	return TRUE;
}

static void
remove_child (HTMLObject *self, HTMLObject *child)
{
	html_clue_remove (HTML_CLUE (self), child);
}

static void
split (HTMLObject *self, HTMLEngine *e, HTMLObject *child, gint offset, gint level, GList **left, GList **right)
{
	HTMLObject *dup;
	HTMLClue *clue;

	clue = HTML_CLUE (self);
	dup  = html_object_dup (self);

	HTML_CLUE (dup)->tail  = HTML_CLUE (self)->tail;
	HTML_CLUE (self)->tail = child->prev;
	if (child->prev)
			child->prev->next = NULL;
	child->prev = NULL;
	if (child == HTML_CLUE (self)->head)
		HTML_CLUE (self)->head = NULL;
	HTML_CLUE (dup)->head  = child;
	set_parent (child, NULL, dup);

	if (self->parent && HTML_OBJECT_TYPE (self->parent) != HTML_TYPE_TABLE)
		html_clue_append_after (HTML_CLUE (self->parent), dup, self);

	*left  = g_list_prepend (*left, self);
	*right = g_list_prepend (*right, dup);

	level--;
	if (level)
		html_object_split (self->parent, e, dup, 0, level, left, right);
}

static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLObject *obj;
	
	if (y + height < o->y - o->ascent || y > o->y + o->descent)
		return;

	tx += o->x;
	ty += o->y - o->ascent;
	
	for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
		if (!(obj->flags & HTML_OBJECT_FLAG_ALIGNED)) {
			html_object_draw (obj,
					  p,
					  x - o->x, y - (o->y - o->ascent),
					  width, height,
					  tx, ty);
		}
	}
}

static void
set_max_ascent (HTMLObject *o,
		HTMLPainter *painter,				
		gint a)
{
	HTMLClue *clue = HTML_CLUE (o);
	HTMLObject *obj;

	if (clue->valign == HTML_VALIGN_MIDDLE) {
		for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
			obj->y = obj->y + ((a - o->ascent) / 2);
		}
	}

	else if (clue->valign == HTML_VALIGN_BOTTOM) {
		for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
			obj->y = obj->y + a - o->ascent;
		}
	}

	o->ascent = a;
}

static void
set_max_descent (HTMLObject *o, HTMLPainter *painter, gint d)
{
	HTMLClue *clue = HTML_CLUE (o);
	HTMLObject *obj;
	
	if (clue->valign == HTML_VALIGN_MIDDLE) {
		for (obj = clue->head; obj != 0; obj = obj->next) {
			obj->y = obj->y + ((d - o->descent) / 2);
		}
	}
	else if (clue->valign == HTML_VALIGN_BOTTOM) {
		for (obj = clue->head; obj != 0; obj = obj->next) {
			obj->y = obj->y + d - o->descent;
		}
	}
	
	o->descent = d;

}

static void
reset (HTMLObject *clue)
{
	HTMLObject *obj;

	for (obj = HTML_CLUE (clue)->head; obj != NULL; obj = obj->next)
		html_object_reset (obj);

	HTML_CLUE (clue)->curr = NULL;

	(* HTML_OBJECT_CLASS (parent_class)->reset) (HTML_OBJECT (clue));
}

static gboolean
calc_size (HTMLObject *o,
	   HTMLPainter *painter)
{
	gboolean changed;

	/* If we have already called calc_size for the children, then just
	   continue from the last object done in previous call. */
	if (HTML_CLUE (o)->curr == NULL) {
		o->ascent = 0;
		HTML_CLUE (o)->curr = HTML_CLUE (o)->head;
	}

	changed = FALSE;

	while (HTML_CLUE (o)->curr != NULL) {
		changed |= html_object_calc_size (HTML_CLUE (o)->curr, painter);
		HTML_CLUE (o)->curr = HTML_CLUE (o)->curr->next;
	}

	/* Remember the last object so that we can start from here next time
	   we are called */
	HTML_CLUE (o)->curr = HTML_CLUE (o)->tail;

	return changed;
}

static gint
calc_preferred_width (HTMLObject *o,
		      HTMLPainter *painter)
{
	gint prefWidth = 0;
	HTMLObject *obj;
	
	for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
		gint w;

		w = html_object_calc_preferred_width (obj, painter);
		if (w > prefWidth)
			prefWidth = w;
	}

	return prefWidth;
}

/* FIXME: This should be in HTMLClueV.  */
static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
	HTMLObject *obj;
	gint minWidth = 0;
	
	for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
		gint w;

		w = html_object_calc_min_width (obj, painter);
		if (w > minWidth)
			minWidth = w;
	}
	
	return minWidth;
}

static HTMLAnchor *
find_anchor (HTMLObject *self, const char *name, gint *x, gint *y)
{
	HTMLClue *clue;
	HTMLObject *obj;
	HTMLAnchor *anchor;

	*x += self->x;
	*y += self->y - self->ascent;

	clue = HTML_CLUE (self);

	for ( obj = clue->head; obj != NULL; obj = obj->next ) {
		if ((anchor = html_object_find_anchor ( obj, name, x, y)) != 0 )
			return anchor;
	}

	*x -= self->x;
	*y -= self->y - self->ascent;

	return 0;
}

static HTMLObject*
check_point (HTMLObject *o,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	HTMLObject *obj;
	HTMLObject *obj2;

	if (x < o->x || x >= o->x + o->width
	    || y >= o->y + o->descent || y < o->y - o->ascent)
		return NULL;

	x = x - o->x;
	y = y - o->y + o->ascent;

	for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
		obj2 = html_object_check_point (obj, painter,
						x, y, offset_return,
						for_cursor);
		if (obj2 != NULL)
			return obj2;
	}

	return NULL;
}

static gint
check_page_split (HTMLObject *self,
		  gint y)
{
	HTMLClue *clue;
	HTMLObject *p;
	gint last_under = 0;

	clue = HTML_CLUE (self);
	for (p = clue->head; p != NULL; p = p->next) {
		gint y1, y2;

		y1 = p->y - p->ascent;
		y2 = p->y + p->descent;

		if (y1 > y)
			return last_under;

		if (y >= y1 && y < y2)
			return html_object_check_page_split (p, y - y1) + y1;
		last_under = y2;
	}

	return y;
}

static void
forall (HTMLObject *self,
	HTMLEngine *e,
	HTMLObjectForallFunc func,
	gpointer data)
{
	HTMLObject *p, *pnext;

	for (p = HTML_CLUE (self)->head; p != NULL; p = pnext) {
		pnext = p->next;
		html_object_forall (p, e, func, data);
	}

	html_object_class.forall (self, e, func, data);
}

static gboolean
is_container (HTMLObject *self)
{
	return TRUE;
}

static gboolean
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	HTMLObject *p;
	HTMLClue *clue;

	clue = HTML_CLUE (self);

	for (p = clue->head; p != NULL; p = p->next) {
		if (! html_object_save (p, state))
		    return FALSE;
	}

	return TRUE;
}

static gboolean
save_plain (HTMLObject *self,
	    HTMLEngineSaveState *state,
	    gint requested_width)
{
	HTMLObject *p;
	HTMLClue *clue;

	clue = HTML_CLUE (self);

	for (p = clue->head; p != NULL; p = p->next) {
		if (! html_object_save_plain (p, state, requested_width))
		    return FALSE;
	}

	return TRUE;
}

/* HTMLClue methods.  */

static gint
get_left_clear (HTMLClue *o, gint y)
{
	return y;
}

static gint
get_right_clear (HTMLClue *o, gint y)
{
	return y;
}

static void
find_free_area (HTMLClue *clue,
		gint y,
		gint width, gint height,
		gint indent, gint *y_pos,
		gint *lmargin, gint *rmargin)
{
	*y_pos = y;
	*lmargin = 0;
	*rmargin = MAX (HTML_OBJECT (clue)->max_width, HTML_OBJECT (clue)->width);
}

static void
append_right_aligned (HTMLClue *clue, HTMLClue *aclue)
{
	/* This needs to be implemented in the subclasses.  */
	g_warning ("`%s' does not implement `append_right_aligned()'.",
		   html_type_name (HTML_OBJECT_TYPE (clue)));
}

static gboolean
appended (HTMLClue *clue, HTMLClue *aclue)
{
	return FALSE;
}

static gboolean
search (HTMLObject *obj, HTMLSearch *info)
{
	HTMLObject *cur;
	HTMLClue *clue = HTML_CLUE (obj);
	gboolean next = FALSE;

	/* search_next? */
	if (html_search_child_on_stack (info, obj)) {
		cur  = html_search_pop (info);
		cur = (info->forward) ? cur->next : cur->prev;
		next = TRUE;
	} else
		cur = (info->forward) ? clue->head : clue->tail;

	while (cur) {
		html_search_push (info, cur);
		if (html_object_search (cur, info))
			return TRUE;

		html_search_pop (info);
		cur = (info->forward) ? cur->next : cur->prev;
	}

	if (next)
		return html_search_next_parent (info);

	return FALSE;
}

static void
append_selection_string (HTMLObject *self,
			 GString *buffer)
{
	HTMLObject *o = HTML_CLUE (self)->head;

	while (o) {
		html_object_append_selection_string (o, buffer);
		o = o->next;
	}
}

static HTMLObject *
head (HTMLObject *self)
{
	return HTML_CLUE (self)->head;
}

static HTMLObject *
tail (HTMLObject *self)
{
	HTMLObject *obj;

	obj = HTML_CLUE (self)->tail;
	return (obj && HTML_OBJECT_TYPE (obj) == HTML_TYPE_TEXTSLAVE) ? html_object_prev_not_slave (obj) : obj;
}


void
html_clue_type_init (void)
{
	html_clue_class_init (&html_clue_class, HTML_TYPE_CLUE, sizeof (HTMLClue));
}

void
html_clue_class_init (HTMLClueClass *klass,
		      HTMLType type,
		      guint size)
{
	HTMLObjectClass *object_class;

	g_return_if_fail (klass != NULL);

	object_class = HTML_OBJECT_CLASS (klass);
	html_object_class_init (object_class, type, size);
	
	/* HTMLObject functions */
	object_class->destroy = destroy;
	object_class->copy = copy;
	object_class->op_copy = op_copy;
	object_class->op_cut = op_cut;
	object_class->merge = merge;
	object_class->remove_child = remove_child;
	object_class->split = split;
	object_class->draw = draw;
	object_class->set_max_ascent = set_max_ascent;
	object_class->set_max_descent = set_max_descent;
	object_class->reset = reset;
	object_class->calc_size = calc_size;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->calc_min_width = calc_min_width;
	object_class->check_point = check_point;
	object_class->check_page_split = check_page_split;
	object_class->find_anchor = find_anchor;
	object_class->forall = forall;
	object_class->is_container = is_container;
	object_class->save = save;
	object_class->save_plain = save_plain;
	object_class->search = search;
	object_class->append_selection_string = append_selection_string;
	object_class->head = head;
	object_class->tail = tail;
	object_class->get_recursive_length = get_recursive_length;

	/* HTMLClue methods.  */
	klass->get_left_clear = get_left_clear;
	klass->get_right_clear = get_right_clear;
	klass->find_free_area = find_free_area;
	klass->append_right_aligned = append_right_aligned;
	klass->appended = appended;

	parent_class = &html_object_class;
}

void
html_clue_init (HTMLClue *clue,
		HTMLClueClass *klass)
{
	HTMLObject *object;

	object = HTML_OBJECT (clue);
	html_object_init (object, HTML_OBJECT_CLASS (klass));

	clue->head = NULL;
	clue->tail = NULL;
	clue->curr = NULL;

	clue->valign = HTML_VALIGN_TOP;
	clue->halign = HTML_HALIGN_LEFT;
}


gint
html_clue_get_left_clear (HTMLClue *clue, gint y)
{
	return (* HC_CLASS (clue)->get_left_clear) (clue, y);
}

gint
html_clue_get_right_clear (HTMLClue *clue, gint y)
{
	return (* HC_CLASS (clue)->get_right_clear) (clue, y);
}

void
html_clue_find_free_area (HTMLClue *clue,
			  gint y,
			  gint width, gint height, gint indent, gint *y_pos,
			  gint *lmargin, gint *rmargin)
{
	(* HC_CLASS (clue)->find_free_area) (clue, y, width, height, indent, y_pos, lmargin, rmargin);
}

void
html_clue_append_right_aligned (HTMLClue *clue, HTMLClue *aclue)
{
	g_assert (clue != NULL);
	g_assert (aclue != NULL);

	html_object_change_set (HTML_OBJECT (clue), HTML_OBJECT (aclue)->change);

	(* HC_CLASS (clue)->append_right_aligned) (clue, aclue);
}

void
html_clue_append_left_aligned (HTMLClue *clue, HTMLClue *aclue)
{
	g_assert (clue != NULL);
	g_assert (aclue != NULL);

	html_object_change_set (HTML_OBJECT (clue), HTML_OBJECT (aclue)->change);

	(* HC_CLASS (clue)->append_left_aligned) (clue, aclue);
}

gboolean
html_clue_appended (HTMLClue *clue, HTMLClue *aclue)
{
	return (* HC_CLASS (clue)->appended) (clue, aclue);
}


/* Utility functions.  */

static HTMLObject *
get_tail (HTMLObject *p)
{
	if (p == NULL)
		return NULL;

	while (p->next != NULL)
		p = p->next;

	return p;
}

static void
set_parent (HTMLObject *o,
	    HTMLObject *tail,
	    HTMLObject *parent)
{
	while (o) {
		html_object_set_parent (o, parent);
		if (o == tail)
			break;
		o = o->next;
	}
}

/**
 * html_clue_append_after:
 * @clue: An HTMLClue.
 * @o: An HTMLObject.
 * @where: A child of @clue.
 * 
 * Insert @o and its successors in @clue after @clue's element @where.
 **/
void
html_clue_append_after (HTMLClue *clue,
			HTMLObject *o,
			HTMLObject *where)
{
	HTMLObject *tail;

	g_return_if_fail (o != NULL);
	if (where == NULL) {
		html_clue_prepend (clue, o);
		return;
	}
	g_return_if_fail (where->parent == HTML_OBJECT (clue));

	html_object_change_set (HTML_OBJECT (clue), o->change);

	tail = get_tail (o);

	if (where->next != NULL)
		where->next->prev = tail;
	tail->next = where->next;

	where->next = o;
	o->prev = where;

	if (where == clue->tail)
		clue->tail = tail;

	set_parent (o, tail, HTML_OBJECT (clue));
}

/**
 * html_clue_append:
 * @clue: An HTMLClue.
 * @o: An HTMLObject.
 * 
 * Append @o and its successors to @clue.
 **/
void
html_clue_append (HTMLClue *clue,
		  HTMLObject *o)
{
	HTMLObject *tail;

	g_return_if_fail (clue != NULL);
	g_return_if_fail (o != NULL);

	html_object_change_set (HTML_OBJECT (clue), o->change);

	tail = get_tail (o);

	if (! clue->head) {
		clue->head = o;
		o->prev = NULL;
	} else {
		clue->tail->next = o;
		o->prev = clue->tail;
	}

	clue->tail = tail;
	tail->next = NULL;

	html_object_set_parent (o, HTML_OBJECT (clue));

	set_parent (o, tail, HTML_OBJECT (clue));
}

/**
 * html_clue_prepend:
 * @clue: An HTMLClue.
 * @o: An HTMLObject.
 * 
 * Prepend @o and its successors to @clue.
 **/
void
html_clue_prepend (HTMLClue *clue,
		   HTMLObject *o)
{
	HTMLObject *tail;

	g_return_if_fail (clue != NULL);
	g_return_if_fail (o != NULL);

	html_object_change_set (HTML_OBJECT (clue), o->change);

	tail = get_tail (o);

	if (! clue->head) {
		clue->head = o;
		clue->tail = tail;
		o->prev = NULL;
	} else {
		o->next = clue->head;
		clue->head->prev = o;
		clue->head = o;
	}

	o->prev = NULL;

	set_parent (o, tail, HTML_OBJECT (clue));
}

/**
 * html_clue_remove:
 * @clue: An HTMLClue.
 * @o: An HTMLObject.
 * 
 * Remove object @o from the clue.
 **/
void
html_clue_remove (HTMLClue *clue,
		  HTMLObject *o)
{
	g_return_if_fail (clue != NULL);
	g_return_if_fail (o != NULL);
	g_return_if_fail (clue == HTML_CLUE (o->parent));

	if (o == clue->head)
		clue->head = o->next;
	if (o == clue->tail)
		clue->tail = o->prev;

	if (o->next != NULL)
		o->next->prev = o->prev;
	if (o->prev != NULL)
		o->prev->next = o->next;

	o->parent = NULL;
	o->prev = NULL;
	o->next = NULL;
}
