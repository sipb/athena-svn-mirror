/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.

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

/* This is the object that defines a paragraph in the HTML document.  */

/* WARNING: it must always be the child of a clue.  */

#include <config.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <gal/widgets/e-unicode.h>

#include "gtkhtml-properties.h"

#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlclue.h"
#include "htmlclueflow.h"
#include "htmlcluealigned.h"
#include "htmlentity.h"
#include "htmlengine-edit.h"
#include "htmlengine-save.h"
#include "htmllinktext.h"
#include "htmlpainter.h"
#include "htmlplainpainter.h"
#include "htmlsearch.h"
#include "htmlselection.h"
#include "htmltable.h"
#include "htmltablecell.h"
#include "htmltext.h"
#include "htmltextslave.h"	/* FIXME */
#include "htmlvspace.h"


HTMLClueFlowClass html_clueflow_class;
static HTMLClueClass *parent_class = NULL;

#define HCF_CLASS(x) HTML_CLUEFLOW_CLASS (HTML_OBJECT (x)->klass)
#define HTML_IS_PLAIN_PAINTER(obj)              (GTK_CHECK_TYPE ((obj), HTML_TYPE_PLAIN_PAINTER))

inline HTMLHAlignType html_clueflow_get_halignment (HTMLClueFlow *flow);
static gchar * get_item_number_str (HTMLClueFlow *flow);


static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	HTML_CLUEFLOW (dest)->style = HTML_CLUEFLOW (self)->style;
	HTML_CLUEFLOW (dest)->level = HTML_CLUEFLOW (self)->level;
	HTML_CLUEFLOW (dest)->item_type = HTML_CLUEFLOW (self)->item_type;
	HTML_CLUEFLOW (dest)->item_number = HTML_CLUEFLOW (self)->item_number;
}

static inline gboolean
is_item (HTMLClueFlow *flow)
{
	return flow && flow->style == HTML_CLUEFLOW_STYLE_LIST_ITEM;
}

static inline gboolean
items_are_relative (HTMLObject *self, HTMLObject *next_object)
{
	HTMLClueFlow *flow, *next;

	if (!self || !next_object)
		return FALSE;
	flow = HTML_CLUEFLOW (self);
	next = HTML_CLUEFLOW (next_object);

	if (!is_item (flow)
	    || !is_item (next)
	    || flow->level != next->level
	    || next->item_type != flow->item_type)
		return FALSE;

	return TRUE;
}

static HTMLObject *
get_prev_relative_item (HTMLObject *self)
{
	HTMLObject *prev;

	prev = self->prev;
	while (prev && HTML_IS_CLUEFLOW (prev) && is_item (HTML_CLUEFLOW (prev))
	       && HTML_CLUEFLOW (prev)->level > HTML_CLUEFLOW (self)->level)
		prev = prev->prev;

	return prev;
}

static HTMLObject *
get_next_relative_item (HTMLObject *self)
{
	HTMLObject *next;

	next = self->next;
	while (next && HTML_IS_CLUEFLOW (next) && is_item (HTML_CLUEFLOW (next))
	       && HTML_CLUEFLOW (next)->level > HTML_CLUEFLOW (self)->level)
		next = next->next;

	return next;
}

static void
update_item_number (HTMLObject *self)
{
	HTMLObject *prev, *next;

	if (!self || !is_item (HTML_CLUEFLOW (self)))
		return;

	prev = get_prev_relative_item (self);
	if (items_are_relative (prev, self))
		HTML_CLUEFLOW (self)->item_number = HTML_CLUEFLOW (prev)->item_number + 1;
	else if (is_item (HTML_CLUEFLOW (self)))
		HTML_CLUEFLOW (self)->item_number = 1;

	next = self;
	while ((next = get_next_relative_item (next)) && items_are_relative (self, next)) {
		HTML_CLUEFLOW (next)->item_number = HTML_CLUEFLOW (self)->item_number + 1;
		self = next;
	}
}

static guint
get_recursive_length (HTMLObject *self)
{
	return (*HTML_OBJECT_CLASS (parent_class)->get_recursive_length) (self) + (self->next ? 1 : 0);
}

static HTMLObject *
op_helper (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right, guint *len, gboolean cut)
{
	HTMLObject *o;

	/* if (!from && to && HTML_IS_TABLE (to->data) && to->next && GPOINTER_TO_INT (to->next->data) == 0)
		return NULL;
	if (!to && from && HTML_IS_TABLE (from->data) && from->next && GPOINTER_TO_INT (from->next->data) == 1)
		return NULL; 
	if (!from && (*len || !(self->prev && HTML_IS_CLUEFLOW (self->prev) && HTML_IS_TABLE (HTML_CLUE (self->prev)->tail))))
	(*len) ++; */
	if (!from && self->prev) {
		(*len) ++;
		/* if (cut)
		   e->cursor->position --; */
	}
	if (cut)
		html_clueflow_remove_text_slaves (HTML_CLUEFLOW (self));
	o = cut
		? (*HTML_OBJECT_CLASS (parent_class)->op_cut) (self, e, from, to, left, right, len)
		: (*HTML_OBJECT_CLASS (parent_class)->op_copy) (self, e, from, to, len);

	if (!cut && o) {
		html_clueflow_remove_text_slaves (HTML_CLUEFLOW (o));
	}

	return o;
}

static HTMLObject *
op_copy (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, guint *len)
{
	return op_helper (self, e, from, to, NULL, NULL, len, FALSE);
}

static HTMLObject *
op_cut (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right, guint *len)
{
	HTMLObject *rv, *prev, *next;

	prev = self->prev;
	next = self->next;

	rv = op_helper (self, e, from, to, left, right, len, TRUE);

	if (prev) {
		update_item_number (prev);
		if (prev->next == self)
			update_item_number (self);
	}
	if (next) {
		if (next->prev == self)
			update_item_number (self);
		update_item_number (next);
	}

	return rv;
}

static void
set_head_size (HTMLObject *o)
{
	if (o && HTML_CLUE (o)->head)
		HTML_CLUE (o)->head->change |= HTML_CHANGE_SIZE;
}

static void
set_tail_size (HTMLObject *o)
{
	if (o && HTML_CLUE (o)->tail) {
		HTML_CLUE (o)->tail->change |= HTML_CHANGE_SIZE;
	}
}

static void
set_around_size (HTMLObject *o) {
	if (o) {
		o->change |= HTML_CHANGE_SIZE;
		if (o->next)
			o->next->change |= HTML_CHANGE_SIZE;
		if (o->prev)
			o->prev->change |= HTML_CHANGE_SIZE;
	}
}

static void
split (HTMLObject *self, HTMLEngine *e, HTMLObject *child, gint offset, gint level, GList **left, GList **right)
{
	set_around_size (child);
	html_clueflow_remove_text_slaves (HTML_CLUEFLOW (self));
	(*HTML_OBJECT_CLASS (parent_class)->split) (self, e, child, offset, level, left, right);

	update_item_number (self);
}

static gboolean
merge (HTMLObject *self, HTMLObject *with, HTMLEngine *e, GList **left, GList **right, HTMLCursor *cursor)
{
	HTMLClueFlow *cf1, *cf2;
	gboolean rv;

	cf1 = HTML_CLUEFLOW (self);
	cf2 = HTML_CLUEFLOW (with);

	html_clueflow_remove_text_slaves (cf1);
	html_clueflow_remove_text_slaves (cf2);

	set_tail_size (self);
	set_head_size (with);

	/* printf ("merge flows\n"); */

	if (html_clueflow_is_empty (cf1)) {
		cf1->style = cf2->style;
		cf1->level = cf2->level;
		cf1->item_type = cf2->item_type;
		cf1->item_number = cf2->item_number - 1;
		self->x = with->x;
		self->y = with->y;
		self->width = with->width;
		self->ascent = with->ascent;
		self->descent = with->descent;
		HTML_CLUE  (cf1)->halign = HTML_CLUE (cf2)->halign;
		HTML_CLUE  (cf1)->valign = HTML_CLUE (cf2)->valign;
		html_object_copy_data_from_object (self, with);
	}

	rv = (* HTML_OBJECT_CLASS (parent_class)->merge) (self, with, e, left, right, cursor);

	update_item_number (self);
	cf1->item_number --;
	update_item_number (with);
	cf1->item_number ++;

	return rv;
}

static guint
calc_padding (HTMLPainter *painter)
{
	guint ascent, descent;

	/* FIXME maybe this should depend on the style.  */
	/* FIXME move this into the painter.             */
	if (HTML_IS_PLAIN_PAINTER (painter)) {
		ascent = 0;
		descent = 0;
	} else {
		ascent = html_painter_calc_ascent (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);
		descent = html_painter_calc_descent (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);
	}
	return ascent + descent;
}

static guint
calc_indent_unit (HTMLPainter *painter)
{
	guint ascent, descent;

	ascent = html_painter_calc_ascent (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);
	descent = html_painter_calc_descent (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);

	return (ascent + descent) * 3;
}

static guint
calc_bullet_size (HTMLPainter *painter)
{
	guint ascent, descent;

	ascent = html_painter_calc_ascent (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);
	descent = html_painter_calc_descent (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);

	return (ascent + descent) / 3;
}



static gboolean
is_header (HTMLClueFlow *flow)
{
	switch (flow->style) {
	case HTML_CLUEFLOW_STYLE_H1:
	case HTML_CLUEFLOW_STYLE_H2:
	case HTML_CLUEFLOW_STYLE_H3:
	case HTML_CLUEFLOW_STYLE_H4:
	case HTML_CLUEFLOW_STYLE_H5:
	case HTML_CLUEFLOW_STYLE_H6:
		return TRUE;
	default:
		return FALSE;
	}
}

static guint get_post_padding (HTMLClueFlow *flow, guint pad);

static guint
get_pre_padding (HTMLClueFlow *flow, guint pad)
{
	HTMLObject *prev_object;

	prev_object = HTML_OBJECT (flow)->prev;
	if (prev_object == NULL)
		return 0;

	if (HTML_OBJECT_TYPE (prev_object) == HTML_TYPE_CLUEFLOW) {
		HTMLClueFlow *prev;

		if (get_post_padding (HTML_CLUEFLOW (prev_object), 1))
			return 0;

		if (is_item (HTML_CLUEFLOW (prev_object))) {
			if (is_item (flow)) {
				return 0;
			} else {
				return pad;
			}
		} else if (is_item (flow)) {
			return pad;
		}

		prev = HTML_CLUEFLOW (prev_object);
		if (prev->level > flow->level)
			return pad;

		if (flow->style == HTML_CLUEFLOW_STYLE_PRE
		    && prev->style != HTML_CLUEFLOW_STYLE_PRE
		    && ! is_header (prev))
			return pad;

		if (is_header (flow) && ! is_header (prev))
			return pad;

		return 0;
	}

	if (! is_header (flow) && flow->level == 0)
		return 0;

	return pad;
}

static guint
get_post_padding (HTMLClueFlow *flow,
		  guint pad)
{
	HTMLObject *next_object;

	next_object = HTML_OBJECT (flow)->next;
	if (next_object == NULL)
		return 0;

	if (HTML_OBJECT_TYPE (next_object) == HTML_TYPE_CLUEFLOW) {
		HTMLClueFlow *next;

		if (is_item (flow) && is_item (HTML_CLUEFLOW (next_object)))
			return 0;

		next = HTML_CLUEFLOW (next_object);
		if (next->level > flow->level)
			return pad;

		if (flow->style == HTML_CLUEFLOW_STYLE_PRE
		    && next->style != HTML_CLUEFLOW_STYLE_PRE
		    && ! is_header (next))
			return pad;

		if (is_header (flow))
			return pad;

		return 0;
	}

	if (! is_header (flow) && flow->level == 0)
		return 0;

	return pad;
}

static void
add_pre_padding (HTMLClueFlow *flow,
		 guint pad)
{
	guint real_pad;

	real_pad = get_pre_padding (flow, pad);

	HTML_OBJECT (flow)->ascent += real_pad;
	HTML_OBJECT (flow)->y += real_pad;
}

static void
add_post_padding (HTMLClueFlow *flow,
		  guint pad)
{
	guint real_pad;

	real_pad = get_post_padding (flow, pad);

	HTML_OBJECT (flow)->ascent += real_pad;
	HTML_OBJECT (flow)->y += real_pad;
}

static guint
get_indent (HTMLClueFlow *flow,
	    HTMLPainter *painter)
{
	guint level;
	guint indent;

	level = flow->level;

	if (level > 0 || ! is_item (flow))
		indent = level * calc_indent_unit (painter);
	else
		indent = 5 * html_painter_get_space_width (painter, html_clueflow_get_default_font_style (flow), NULL);

	return indent;
}


/* HTMLObject methods.  */
static void
set_max_width (HTMLObject *o,
	       HTMLPainter *painter,
	       gint max_width)
{
	HTMLObject *obj;
	guint indent;

	o->max_width = max_width;

	indent = get_indent (HTML_CLUEFLOW (o), painter);
	
	for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
		html_object_set_max_width (obj, painter, o->max_width - indent);
	}
}

static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
	HTMLObject *cur;
	gint min_width = 0;
	gint w = 0;
	gboolean add;

	add = HTML_CLUEFLOW (o)->style == HTML_CLUEFLOW_STYLE_PRE;

	cur = HTML_CLUE (o)->head;
	while (cur) {
		w += add ? html_object_calc_preferred_width (cur, painter) : html_object_calc_min_width (cur, painter);
		if (!add || cur->flags & HTML_OBJECT_FLAG_NEWLINE || !cur->next) {
			if (min_width < w) min_width = w;
			w = 0;
		}
		cur = cur->next;
	}

	return min_width + get_indent (HTML_CLUEFLOW (o), painter);
}

static gint
set_line_x (HTMLObject **obj, HTMLObject *run, gint x, gboolean *changed)
{
	while (*obj != run) {
		if ((*obj)->x != x) {
			(*obj)->x = x;   
			*changed = TRUE;
		}
		x   += (*obj)->width;
		*obj = (*obj)->next;
	}
	return x;
}

static gint
pref_right_margin (HTMLPainter *p, HTMLClueFlow *clueflow, HTMLObject *o, gint y) 
{
	gint fixed_margin = html_object_get_right_margin (o, p, y);

	/* FIXME: this hack lets us wrap the display at 72 characters when we are using
	   a plain painter */
	  
	if (clueflow->style == HTML_CLUEFLOW_STYLE_PRE || ! HTML_IS_PLAIN_PAINTER(p))
		return fixed_margin;
	
	return MIN (fixed_margin, 72 * html_painter_get_space_width (p, GTK_HTML_FONT_STYLE_SIZE_3, NULL));
}

static void
add_clear_area (GList **changed_objs, HTMLObject *o, gint x, gint w)
{
	HTMLObjectClearRectangle *cr;

	cr = g_new (HTMLObjectClearRectangle, 1);

	cr->object = o;
	cr->x = x;
	cr->y = 0;
	cr->width = w;
	cr->height = o->ascent + o->descent;

	*changed_objs = g_list_prepend (*changed_objs, cr);
	/* NULL meens: clear rectangle follows */
	*changed_objs = g_list_prepend (*changed_objs, NULL);
}

/* EP CHECK: should be mostly OK.  */
/* FIXME: But it's awful.  Too big and ugly.  */
static gboolean
calc_size (HTMLObject *o, HTMLPainter *painter, GList **changed_objs)
{
	HTMLVSpace *vspace;
	HTMLClue *clue;
	HTMLClueFlow *flow;
	HTMLObject *obj;
	HTMLObject *line;
	HTMLClearType clear;
	gboolean newLine;
	gboolean firstLine;
	gint lmargin, rmargin;
	gint indent;
	gint oldy;
	gint w, a, d;
	guint padding;
	gboolean changed, leaf_childs_changed_size;
	gint old_ascent, old_descent, old_width;
	gint runWidth = 0;
	gboolean have_valign_top;

	html_clueflow_remove_text_slaves (HTML_CLUEFLOW (o));

	changed = FALSE;
	leaf_childs_changed_size = FALSE;
	old_ascent = o->ascent;
	old_descent = o->descent;
	old_width = o->width;

	/* if (changed_objs)
		printf ("begin:   %d==%d %d==%d %d==%d\n",
		o->width, old_width, o->ascent, old_ascent, o->descent, old_descent); */

	clue = HTML_CLUE (o);
	flow = HTML_CLUEFLOW (o);

	obj = clue->head;
	line = clue->head;
	clear = HTML_CLEAR_NONE;
	indent = get_indent (HTML_CLUEFLOW (o), painter);

	o->ascent = 0;
	o->descent = 0;
	o->width = 0;

	padding = calc_padding (painter);
	add_pre_padding (HTML_CLUEFLOW (o), padding);

	lmargin = html_object_get_left_margin (o->parent, painter, o->y);
	if (indent > lmargin)
		lmargin = indent;
	/* rmargin = html_object_get_right_margin (o->parent, painter, o->y); */
	rmargin = pref_right_margin (painter, HTML_CLUEFLOW (o), o->parent, o->y);

	w = lmargin;
	a = 0;
	d = 0;
	newLine = FALSE;
	firstLine = TRUE;

	have_valign_top = FALSE;

	while (obj != NULL) {

		if (obj && obj->change & HTML_CHANGE_SIZE
		    && HTML_OBJECT_TYPE (obj) != HTML_TYPE_TEXTSLAVE && !html_object_is_container (obj))
			leaf_childs_changed_size = TRUE;

		if (obj->flags & HTML_OBJECT_FLAG_NEWLINE) {
			if (!a)
				a = obj->ascent;
			if (!a && (obj->descent > d))
				d = obj->descent;
			newLine = TRUE;
			vspace = HTML_VSPACE (obj);
			clear = vspace->clear;
			obj = obj->next;
		} else if (obj->flags & HTML_OBJECT_FLAG_SEPARATOR) {
			if (obj->x != w) {
				obj->x = w;
				changed = TRUE;
			}
			if (TRUE /* w != lmargin */) {
				w += obj->width;
				if (obj->ascent > a)
					a = obj->ascent;
				if (obj->descent > d)
					d = obj->descent;
			}
			obj = obj->next;
		} else if (obj->flags & HTML_OBJECT_FLAG_ALIGNED) {
			HTMLClueAligned *c = (HTMLClueAligned *)obj;
			
			if (! html_clue_appended (HTML_CLUE (o->parent), HTML_CLUE (c))) {
				html_object_calc_size (obj, painter, changed_objs);

				if (HTML_CLUE (c)->halign == HTML_HALIGN_LEFT) {
					if (obj->x != lmargin) {
						obj->x = lmargin;
						changed = TRUE;
					}
					if (obj->y != o->ascent + obj->ascent + a + d) {
						obj->y = o->ascent + obj->ascent + a + d;
						changed = TRUE;
					}
					html_clue_append_left_aligned (HTML_CLUE (o->parent), HTML_CLUE (c));

					lmargin = html_object_get_left_margin (o->parent, painter, o->y);

					if (indent > lmargin)
						lmargin = indent;

					if (a + d == 0)
						w = lmargin;
					else
						w = runWidth + lmargin;
				} else {
					if (obj->x != rmargin - obj->width) {
						obj->x = rmargin - obj->width;
						changed = TRUE;
					}
					if (obj->y != o->ascent + obj->ascent + a + d) {
						obj->y = o->ascent + obj->ascent + a + d;
						changed = TRUE;
					}
					
					html_clue_append_right_aligned (HTML_CLUE (o->parent), HTML_CLUE (c));

					/* rmargin = html_object_get_right_margin (o->parent, painter, o->y); */
					rmargin = pref_right_margin (painter, HTML_CLUEFLOW (o), o->parent, o->y);
				}
			}

			obj = obj->next;
		}
		/* This is a normal object.  We must add all objects upto the next
		   separator/newline/aligned object. */
		else {
			HTMLObject *run;

			/* By setting "newLine = true" we move the complete run
			   to a new line.  We shouldn't set newLine if we are
			   at the start of a line.  */
			runWidth = 0;
			run = obj;
			
			if (lmargin >= rmargin) {
				gint new_y;

				if (run && run->change & HTML_CHANGE_SIZE
				    && HTML_OBJECT_TYPE (run) != HTML_TYPE_TEXTSLAVE && !html_object_is_container (run))
					leaf_childs_changed_size = TRUE;
				html_object_calc_size (run, painter, changed_objs);
				html_clue_find_free_area (HTML_CLUE (o->parent), o->y, run->min_width,
							  run->ascent + run->descent, indent, &new_y,
							  &lmargin, &rmargin);
				o->ascent += new_y - o->y;
				o->y       = new_y;
				w          = lmargin;
			}

			while ( run
				&& ! (run->flags & HTML_OBJECT_FLAG_SEPARATOR)
				&& ! (run->flags & HTML_OBJECT_FLAG_NEWLINE)
				&& ! (run->flags & HTML_OBJECT_FLAG_ALIGNED)) {
				HTMLFitType fit;
				HTMLVAlignType valign;
				gboolean firstRun;
				gint width_left;

				if (run && run->change & HTML_CHANGE_SIZE
				    && HTML_OBJECT_TYPE (run) != HTML_TYPE_TEXTSLAVE && !html_object_is_container (run))
					leaf_childs_changed_size = TRUE;

				width_left = rmargin - runWidth - w;
				firstRun = run == line || (HTML_IS_TEXT_SLAVE (run)
							   && HTML_OBJECT (HTML_TEXT_SLAVE (run)->owner) == line
							   && HTML_TEXT_SLAVE (run)->posStart == 0);

				if (!firstRun && width_left < 0) {
					fit = HTML_FIT_NONE;
				} else {
					fit = html_object_fit_line (run,
								    painter,
								    w + runWidth == lmargin,
								    firstRun,
								    flow->style == HTML_CLUEFLOW_STYLE_PRE ? G_MAXINT : width_left);
				}

				if (fit == HTML_FIT_NONE) {
					w = set_line_x (&obj, run, w, &changed);
					newLine = TRUE;
					break;
				}

				html_object_calc_size (run, painter, changed_objs);
				runWidth += run->width;
				valign = html_object_get_valign (run);

				/* Algorithm for dealing vertical alignment.
				   Elements with `HTML_VALIGN_BOTTOM' and
				   `HTML_VALIGN_MIDDLE' can be handled
				   immediately.	 Objects with
				   `HTML_VALIGN_TOP', instead, need to know the
				   total height of the line, so need to be
				   handled last.  */

				switch (valign) {
				case HTML_VALIGN_MIDDLE: {
					gint height;
					gint half_height;

					height = run->ascent + run->descent;
					half_height = height / 2;
					if (half_height > a)
						a = half_height;
					if (height - half_height > d)
						d = height - half_height;
					break;
				}
				case HTML_VALIGN_BOTTOM:
					if (run->ascent > a)
						a = run->ascent;
					if (run->descent > d)
						d = run->descent;
					break;
				case HTML_VALIGN_TOP:
					have_valign_top = TRUE;
					/* Do nothing for now.	*/
					break;
				default:
					g_assert_not_reached ();
				}

				run = run->next;

				if (fit == HTML_FIT_PARTIAL) {
					/* Implicit separator */
					break;
				}
			}

			if (! newLine) {
				gint new_y, new_lmargin, new_rmargin;

				/* Check if the run fits in the current flow area 
				   especially with respect to its height.
				   If not, find a rectangle with height a+b. The size of
				   the rectangle will be rmargin-lmargin. */

				html_clue_find_free_area (HTML_CLUE (o->parent),
							  o->y,
							  rmargin - lmargin, a+d, indent, &new_y,
							  &new_lmargin, &new_rmargin);
				
				if (new_y != o->y
				    || new_lmargin > lmargin
				    || new_rmargin < rmargin) {
					
					/* We did not get the location we expected 
					   we start building our current line again */
					/* We got shifted downwards by "new_y - y"
					   add this to both "y" and "ascent" */

					new_y -= o->y;
					o->ascent += new_y;

					o->y += new_y;

					lmargin = new_lmargin;
					if (indent > lmargin)
						lmargin = indent;
					rmargin = new_rmargin;
					obj = line;
					
					/* Reset this line */
					w = lmargin;
					d = 0;
					a = 0;

					newLine = FALSE;
					clear = HTML_CLEAR_NONE;
				} else {
					w = set_line_x (&obj, run, w, &changed);
					/* we've used up this line so insert a newline */
					newLine = TRUE;

					lmargin = html_object_get_left_margin (o->parent, painter, o->y);
				
					if (indent > lmargin)
						lmargin = indent;

				        /* rmargin = html_object_get_right_margin (o->parent, painter, o->y); */
					rmargin = pref_right_margin (painter, HTML_CLUEFLOW (o), o->parent, o->y);
				}
			}
		}
		
		/* if we need a new line, or all objects have been processed
		   and need to be aligned. */
		if ( newLine || !obj) {
			HTMLHAlignType halign;
			int extra;

			extra = 0;
			halign = html_clueflow_get_halignment (flow);

			if (w > o->width)
				o->width = w;

			if (halign == HTML_HALIGN_CENTER) {
				extra = (rmargin - w) / 2;
				if (extra < 0)
					extra = 0;
			}
			else if (halign == HTML_HALIGN_RIGHT) {
				extra = rmargin - w;
				if (extra < 0)
					extra = 0;
			}

			o->ascent += a + d;
			o->y += a + d;

			/* Update the height for `HTML_VALIGN_TOP' objects.  */
			if (have_valign_top) {
				HTMLObject *p;

				for (p = line; p != obj; p = p->next) {
					gint height, rest;

					if (html_object_get_valign (p) != HTML_VALIGN_TOP)
						continue;

					height = p->ascent + p->descent;
					rest = height - a;

					if (rest > d) {
						o->ascent += rest - d;
						o->y += rest - d;
						d = rest;
					}
				}

				have_valign_top = FALSE;
			}

			for (; line != obj; line = line->next) {
				if (line->flags & HTML_OBJECT_FLAG_ALIGNED)
					continue;

				/* FIXME max_ascent/max_descent -- not quite
				   sure what they are for. */

				switch (html_object_get_valign (line)) {
				case HTML_VALIGN_BOTTOM:
					line->y = o->ascent - d;
					html_object_set_max_ascent (line, painter, a);
					html_object_set_max_descent (line, painter, d);
					break;
				case HTML_VALIGN_MIDDLE:
					line->y = o->ascent - a - d + line->ascent;
					break;
				case HTML_VALIGN_TOP:
					line->y = o->ascent - a - d + line->ascent;
					break;
				default:
					g_assert_not_reached ();
				}

				if (halign == HTML_HALIGN_CENTER
				    || halign == HTML_HALIGN_RIGHT)
					line->x += extra;
			}

			oldy = o->y;
			
			if (clear == HTML_CLEAR_ALL) {
				int new_lmargin, new_rmargin;
				
				html_clue_find_free_area (HTML_CLUE (o->parent),
							  oldy,
							  o->max_width,
							  1, 0,
							  &o->y,
							  &new_lmargin,
							  &new_rmargin);
			} else if (clear == HTML_CLEAR_LEFT) {
				o->y = html_clue_get_left_clear (HTML_CLUE (o->parent), oldy);
			} else if (clear == HTML_CLEAR_RIGHT) {
				o->y = html_clue_get_right_clear (HTML_CLUE (o->parent), oldy);
			}

			o->ascent += o->y - oldy;

			lmargin = html_object_get_left_margin (o->parent, painter, o->y);
			if (indent > lmargin)
				lmargin = indent;
			/* rmargin = html_object_get_right_margin (o->parent, painter, o->y); */
			rmargin = pref_right_margin (painter, HTML_CLUEFLOW (o), o->parent, o->y);

			w = lmargin;
			d = 0;
			a = 0;
			
			newLine = FALSE;
			clear = HTML_CLEAR_NONE;
		
		}
	}
	
       
	if (o->width < o->max_width)
		o->width = o->max_width;

	add_post_padding (HTML_CLUEFLOW (o), padding);

	if (o->ascent != old_ascent || o->descent != old_descent || o->width != old_width) {
		changed = TRUE;
	}
	if (o->ascent != old_ascent || o->descent != old_descent || o->width != old_width || leaf_childs_changed_size) {
		/* if (changed_objs)
			printf ("changed: %d==%d %d==%d %d==%d %d\n",
			o->width, old_width, o->ascent, old_ascent, o->descent, old_descent, leaf_childs_changed_size); */
		if (changed_objs) {
			if (old_width > o->max_width && o->width < old_width) {
				add_clear_area (changed_objs, o, o->width, old_width - o->width);
			}
			html_object_add_to_changed (changed_objs, o);
		}
	}

	return changed;
}

static gint
calc_preferred_width (HTMLObject *o,
		      HTMLPainter *painter)
{
	HTMLObject *obj, *next;
	gint maxw = 0, w = 0;

	next = NULL;

	for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
		if (!(obj->flags & HTML_OBJECT_FLAG_NEWLINE)) {
			w += html_object_calc_preferred_width (obj, painter);
		}

		if (obj->flags & HTML_OBJECT_FLAG_NEWLINE || !(next = html_object_next_not_slave (obj))) {
			HTMLObject *eol = (obj->flags & HTML_OBJECT_FLAG_NEWLINE) ? html_object_prev_not_slave (obj) : obj;

			/* remove trailing space width on the end of line which is not on end of paragraph */
			if (next && html_object_is_text (eol))
				w -= html_text_trail_space_width (HTML_TEXT (eol), painter);

			if (w > maxw)
				maxw = w;
			w = 0;
		}
	}

	return maxw + get_indent (HTML_CLUEFLOW (o), painter);
}

static gchar *
get_alpha_value (gint value, gboolean lower)
{
	GString *str;
	gchar *rv;
	gint add = lower ? 'a' : 'A';

	str = g_string_new (".");

	do {
		g_string_prepend_c (str, ((value - 1) % 26) + add);
		value = (value - 1) / 26;
	} while (value);

	rv = str->str;
	g_string_free (str, FALSE);

	return rv;
}

#define BASES 7

static gchar *
get_roman_value (gint value, gboolean lower)
{
	GString *str;
	gchar *rv, *base = "IVXLCDM";
	gint b, r, add = lower ? 'a' - 'A' : 0;

	if (value > 3999)
		g_strdup ("?.");

	str = g_string_new (".");

	for (b = 0; value > 0 && b < BASES - 1; b += 2, value /= 10) {
		r = value % 10;
		if (r != 0) {
			if (r < 4) {
				for (; r; r--)
					g_string_prepend_c (str, base [b] + add);
			} else if (r == 4) {
				g_string_prepend_c (str, base [b + 1] + add);
				g_string_prepend_c (str, base [b] + add);
			} else if (r == 5) {
				g_string_prepend_c (str, base [b + 1] + add);
			} else if (r < 9) {
				for (; r > 5; r--)
					g_string_prepend_c (str, base [b] + add);
				g_string_prepend_c (str, base [b + 1] + add);
			} else if (r == 9) {
				g_string_prepend_c (str, base [b + 2] + add);
				g_string_prepend_c (str, base [b] + add);
			}
		}
	}

	rv = str->str;
	g_string_free (str, FALSE);

	return rv;
}

static gchar *
get_item_number_str (HTMLClueFlow *flow)
{
	switch (flow->item_type) {
	case HTML_LIST_TYPE_ORDERED_ARABIC:
		return g_strdup_printf ("%d.", flow->item_number);
	case HTML_LIST_TYPE_ORDERED_LOWER_ALPHA:
	case HTML_LIST_TYPE_ORDERED_UPPER_ALPHA:
		return get_alpha_value (flow->item_number, flow->item_type == HTML_LIST_TYPE_ORDERED_LOWER_ALPHA);
	case HTML_LIST_TYPE_ORDERED_LOWER_ROMAN:
	case HTML_LIST_TYPE_ORDERED_UPPER_ROMAN:
		return get_roman_value (flow->item_number, flow->item_type == HTML_LIST_TYPE_ORDERED_LOWER_ROMAN);
	default:
		return NULL;
	}
}

static void
draw_item (HTMLObject *self, HTMLPainter *painter, gint x, gint y, gint width, gint height, gint tx, gint ty)
{
	HTMLClueFlow *flow;
	HTMLObject *first;

	first = HTML_CLUE (self)->head;

	flow = HTML_CLUEFLOW (self);
	html_painter_set_pen (painter, &html_colorset_get_color_allocated (painter, HTMLTextColor)->color);

	if (flow->item_type == HTML_LIST_TYPE_UNORDERED) {
		guint bullet_size;
		gint xp, yp;
		bullet_size = MAX (3, calc_bullet_size (painter));

		xp = self->x + first->x - 2 * bullet_size;
		yp = self->y - self->ascent 
			+ (first->y - first->ascent) 
			+ (first->ascent + first->descent)/2 
			- bullet_size/2;

		xp += tx, yp += ty;

		if (flow->level == 0 || (flow->level & 1) != 0)
			html_painter_fill_rect (painter, xp + 1, yp + 1, bullet_size - 2, bullet_size - 2);

		html_painter_draw_line (painter, xp + 1, yp, xp + bullet_size - 2, yp);
		html_painter_draw_line (painter, xp + 1, yp + bullet_size - 1,
					xp + bullet_size - 2, yp + bullet_size - 1);
		html_painter_draw_line (painter, xp, yp + 1, xp, yp + bullet_size - 2);
		html_painter_draw_line (painter, xp + bullet_size - 1, yp + 1,
					xp + bullet_size - 1, yp + bullet_size - 2);
	} else {
		gchar *number;

		number = get_item_number_str (flow);
		if (number) {
			gint width, len, line_offset = 0;

			len   = strlen (number);
			width = html_painter_calc_text_width (painter, number, len, &line_offset,
							      html_clueflow_get_default_font_style (flow), NULL)
				+ html_painter_get_space_width (painter, html_clueflow_get_default_font_style (flow), NULL);
			html_painter_set_font_style (painter, html_clueflow_get_default_font_style (flow));
			html_painter_set_font_face  (painter, NULL);
			html_painter_draw_text (painter, self->x + first->x - width + tx,
						self->y - self->ascent + first->y + ty,
						number, strlen (number), 0);
		}
		g_free (number);
	}
}

static void
draw (HTMLObject *self,
      HTMLPainter *painter,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	if (y > self->y + self->descent || y + height < self->y - self->ascent)
		return;

	if (HTML_CLUE (self)->head != NULL && is_item (HTML_CLUEFLOW (self)))
		draw_item (self, painter, x, y, width, height, tx, ty);

	(* HTML_OBJECT_CLASS (&html_clue_class)->draw) (self, painter, x, y, width, height, tx, ty);
}

static void
draw_background (HTMLObject *self,
		 HTMLPainter *p,
		 gint x, gint y,
		 gint width, gint height,
		 gint tx, gint ty)
{
	html_object_draw_background (self->parent, p,
				     x + self->parent->x,
				     y + self->parent->y - self->parent->ascent,
				     width, height,
				     tx - self->parent->x,
				     ty - self->parent->y + self->parent->ascent);
}


static HTMLObject*
check_point (HTMLObject *self,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	HTMLObject *obj, *p, *pnext, *eol, *cur;
	HTMLClue *clue;
	gint line_ly = 0;
	gint line_y;

	if (x < self->x || x >= self->x + self->width
	    || y < self->y - self->ascent || y >= self->y + self->descent)
		return NULL;

	clue = HTML_CLUE (self);

	x = x - self->x;
	y = y - self->y + self->ascent;

	for (p = clue->head; p; ) {

		if (html_object_is_text (p))
			p = p->next;
		if (!p)
			break;

		line_y  = line_ly;
		line_ly = p->y + p->descent;

		for (eol = p; eol && (eol->y - eol->ascent < line_ly || eol->ascent + eol->y == line_y); ) {
			line_ly = MAX (line_ly, eol->y + eol->descent);
			do
				eol = eol->next;
			while (eol && html_object_is_text (eol));
		}

		if (y >= line_y && y < line_ly)
			for (cur = p; cur && cur != eol; ) {
				obj = html_object_check_point (cur, painter, x, for_cursor
							       ? MIN (MAX (y, cur->y - cur->ascent),
								      cur->y + cur->descent - 1) : y,
							       offset_return, for_cursor);

				if (obj != NULL)
					return obj;
				do
					cur = cur->next;
				while (cur && cur != eol && html_object_is_text (cur));
			}
		p = eol;
	}

	if (!for_cursor)
		return NULL;

	/* before */
	p = clue->head;
	if (p && html_object_is_text (p))
		p = p->next;
	if (p && (x < p->x || y < p->y - p->ascent)) {
		obj = html_object_check_point (p, painter, p->x, p->y - p->ascent, offset_return, for_cursor);
		if (obj != NULL)
			return obj;
	}
	for (p = clue->head; p != NULL; p = pnext) {
		if (html_object_is_text (p))
			p = p->next;
		if (!p)
			break;
		pnext = p->next;
		while (pnext && html_object_is_text (pnext))
			pnext = pnext->next;

		/* new line */
		if (pnext == NULL || (pnext->y - pnext->ascent >= p->y + p->descent
				      && y >= p->y - p->ascent
				      && y <  p->y + p->descent)) {
			obj = html_object_check_point (p, painter, MAX (0, p->x + p->width - 1), p->y + p->descent - 1,
						       offset_return, for_cursor);
			if (obj != NULL)
				return obj;
		}
	}

	/* after */
	p = clue->tail;
	if (p && ((x >= p->x + p->width) || (y >= p->y + p->descent))) {
		obj = html_object_check_point (p, painter, MAX (0, p->x + p->width - 1), p->y + p->descent - 1,
					       offset_return, for_cursor);
		if (obj != NULL)
			return obj;
	}
	return NULL;
}

static void
append_selection_string (HTMLObject *self,
			 GString *buffer)
{
        (*HTML_OBJECT_CLASS (parent_class)->append_selection_string) (self, buffer);
	if (self->selected)
		g_string_append_c (buffer, '\n');
}


/* Saving support.  */

static gboolean
write_indent (HTMLEngineSaveState *state, gint level)
{
	while (level > 0) {
		if (!html_engine_save_output_string (state, "    "))
			return FALSE;
		level --;
	}

	return TRUE;
}

/* static const char *
get_item_tag 
(HTMLClueFlow *flow)
{
	switch (flow->item_type) {
	case HTML_LIST_TYPE_UNORDERED:
		return "ul";
	case HTML_LIST_TYPE_ORDERED_ARABIC:
		return "ol";
	case HTML_LIST_TYPE_ORDERED_LOWER_ROMAN:
		return "ol type=i";
	case HTML_LIST_TYPE_ORDERED_UPPER_ROMAN:
		return "ol type=I";
	case HTML_LIST_TYPE_ORDERED_LOWER_ALPHA:
		return "ol type=a";
	case HTML_LIST_TYPE_ORDERED_UPPER_ALPHA:
		return "ol type=A";
	default:
		return "ul";
	}
} */

#define INDENT(cf) \
	if (cf->style != HTML_CLUEFLOW_STYLE_PRE) { \
		if (! write_indent (state, cf->level)) { \
				return FALSE; \
		} \
	}
#define INDENT_T(cf) \
	if (cf->style != HTML_CLUEFLOW_STYLE_PRE) { \
		if (! write_indent (state, cf->level)) { \
                        g_free (tag); \
			return FALSE; \
		} \
	}

static const char *
get_tag (HTMLClueFlow *flow)
{
	if (!flow)
		return NULL;

	switch (flow->style) {
	case HTML_CLUEFLOW_STYLE_LIST_ITEM:
		return NULL;
	case HTML_CLUEFLOW_STYLE_NORMAL:
	case HTML_CLUEFLOW_STYLE_H1:
	case HTML_CLUEFLOW_STYLE_H2:
	case HTML_CLUEFLOW_STYLE_H3:
	case HTML_CLUEFLOW_STYLE_H4:
	case HTML_CLUEFLOW_STYLE_H5:
	case HTML_CLUEFLOW_STYLE_H6:
	case HTML_CLUEFLOW_STYLE_ADDRESS:
	case HTML_CLUEFLOW_STYLE_PRE:
	default:
		return "BLOCKQUOTE";
	}
}

static gboolean
write_indentation_tags (HTMLEngineSaveState *state, guint last_value, guint new_value, const gchar *tag)
{
	guint i;

	if (new_value == last_value)
		return TRUE;

	if (new_value > last_value) {
		for (i = last_value; i < new_value; i++) {
			if (! write_indent (state, i + 1) || ! html_engine_save_output_string (state, "<%s>\n", tag)) {
				return FALSE;
			}
		}
	} else {
		for (i = last_value; i > new_value; i--) {
			if (! write_indent (state, i) || ! html_engine_save_output_string (state, "</%s>\n", tag)) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

inline static gint
get_level (HTMLClueFlow *cf)
{
	return cf->level;
}

static gboolean
write_pre_tags (HTMLClueFlow *self,
		HTMLEngineSaveState *state)
{
	HTMLClueFlow *prev;
	const char *prev_tag, *curr_tag;

	prev = HTML_CLUEFLOW (HTML_OBJECT (self)->prev);
	if (prev != NULL && !HTML_IS_TABLE (HTML_CLUE (self)->head)
	    && prev->level == self->level && prev->style == self->style) {
		if (!is_item (self) && self->style != HTML_CLUEFLOW_STYLE_PRE) {
			if (! write_indent (state, self->level))
				return FALSE;
			return html_engine_save_output_string (state, "<BR>\n");
		} else
			return TRUE;
	}

	prev_tag = get_tag (prev);
	curr_tag = get_tag (self);

	if ((prev_tag != NULL) && (curr_tag != NULL) && (strcmp (prev_tag, curr_tag) == 0)) {
		write_indentation_tags (state, get_level (prev), get_level (self), prev_tag);
	} else {
		if (prev_tag != NULL) {
			if (is_item (self)) {
				write_indentation_tags (state, get_level (prev), self->level > 0 ? self->level - 1 : 0,
							prev_tag);
			} else {
				write_indentation_tags (state, get_level (prev), 0, prev_tag);
			}
		}
		if (curr_tag != NULL) {
			if (prev && is_item (prev)) {
				write_indentation_tags (state, prev->level > 0 ? prev->level - 1 : 0,
							get_level (self), curr_tag);
			} else {
				write_indentation_tags (state, 0, get_level (self), curr_tag);
			}
		}
		if (curr_tag == NULL && prev_tag == NULL && prev && is_item (prev) && is_item (self)
		    && abs (prev->level - self->level) > 1) {
			write_indentation_tags (state,
						prev->level < self->level ? prev->level : prev->level - 1,
						prev->level < self->level ? self->level - 1 : self->level,
						"BLOCKQUOTE");
		}
	}

	return TRUE;
}

static gboolean
write_post_tags (HTMLClueFlow *self,
		 HTMLEngineSaveState *state)
{
	const char *tag;

	if (HTML_OBJECT (self)->next != NULL)
		return TRUE;

	tag = get_tag (self);
	if (tag)
		write_indentation_tags (state, get_level (self), 0, tag);
	else if (is_item (self) && self->level > 0)
		write_indentation_tags (state, self->level - 1, 0, "BLOCKQUOTE");

	return TRUE;
}

static gboolean 
is_similar (HTMLObject *self, HTMLObject *friend)
{
	if (friend &&  HTML_OBJECT_TYPE (friend) == HTML_TYPE_CLUEFLOW) {
		if ((HTML_CLUEFLOW (friend)->style == HTML_CLUEFLOW (self)->style)
		    && (HTML_CLUEFLOW (friend)->level == HTML_CLUEFLOW (self)->level)) {
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
need_list_begin (HTMLObject *self)
{
	return !items_are_relative (self->prev, self)
		&& (!self->prev
		    || (HTML_IS_CLUEFLOW (self->prev)
			&& (HTML_CLUEFLOW (self->prev)->level < HTML_CLUEFLOW (self)->level
			    || !is_item (HTML_CLUEFLOW (self->prev)))))
		    && HTML_CLUEFLOW (self)->level > 0;
}

static gboolean
need_list_end (HTMLObject *self)
{
	return !items_are_relative (self, self->next)
		&& (!self->next
		    || (HTML_IS_CLUEFLOW (self->next)
			&& (HTML_CLUEFLOW (self->next)->level < HTML_CLUEFLOW (self)->level
			    || !is_item (HTML_CLUEFLOW (self->next)))))
		    && HTML_CLUEFLOW (self)->level > 0;
}

static gchar *
get_list_start_tag (HTMLObject *self)
{
	switch (HTML_CLUEFLOW (self)->item_type) {
	case HTML_LIST_TYPE_UNORDERED:
	case HTML_LIST_TYPE_MENU:
	case HTML_LIST_TYPE_DIR:
		return g_strdup ("LI");
	case HTML_LIST_TYPE_ORDERED_ARABIC:
		return g_strdup_printf ("LI TYPE=1 VALUE=%d", HTML_CLUEFLOW (self)->item_number);
	case HTML_LIST_TYPE_ORDERED_UPPER_ROMAN:
		return g_strdup_printf ("LI TYPE=I VALUE=%d", HTML_CLUEFLOW (self)->item_number);
	case HTML_LIST_TYPE_ORDERED_LOWER_ROMAN:
		return g_strdup_printf ("LI TYPE=i VALUE=%d", HTML_CLUEFLOW (self)->item_number);
	case HTML_LIST_TYPE_ORDERED_UPPER_ALPHA:
		return g_strdup_printf ("LI TYPE=A VALUE=%d", HTML_CLUEFLOW (self)->item_number);
	case HTML_LIST_TYPE_ORDERED_LOWER_ALPHA:
		return g_strdup_printf ("LI TYPE=a VALUE=%d", HTML_CLUEFLOW (self)->item_number);
	}

	return NULL;
}

static gchar *
get_start_tag (HTMLObject *self)
{
	switch (HTML_CLUEFLOW (self)->style) {
	case HTML_CLUEFLOW_STYLE_H1:
		return g_strdup ("H1");
	case HTML_CLUEFLOW_STYLE_H2:
		return g_strdup ("H2");
	case HTML_CLUEFLOW_STYLE_H3:
		return g_strdup ("H3");
	case HTML_CLUEFLOW_STYLE_H4:
		return g_strdup ("H4");
	case HTML_CLUEFLOW_STYLE_H5:
		return g_strdup ("H5");
	case HTML_CLUEFLOW_STYLE_H6:
		return g_strdup ("H6");
	case HTML_CLUEFLOW_STYLE_ADDRESS:
		return g_strdup ("ADDRESS");
	case HTML_CLUEFLOW_STYLE_PRE:
		return g_strdup ("PRE");
	case HTML_CLUEFLOW_STYLE_LIST_ITEM:
		switch (HTML_CLUEFLOW (self)->item_type) {
		case HTML_LIST_TYPE_UNORDERED:
		case HTML_LIST_TYPE_MENU:
		case HTML_LIST_TYPE_DIR:
			return g_strdup (need_list_begin (self) ? "UL" : NULL);
		case HTML_LIST_TYPE_ORDERED_LOWER_ALPHA:
			return g_strdup (need_list_begin (self) ? "OL TYPE=a" : NULL);
		case HTML_LIST_TYPE_ORDERED_UPPER_ALPHA:
			return g_strdup (need_list_begin (self) ? "OL TYPE=A" : NULL);
		case HTML_LIST_TYPE_ORDERED_LOWER_ROMAN:
			return g_strdup (need_list_begin (self) ? "OL TYPE=i" : NULL);
		case HTML_LIST_TYPE_ORDERED_UPPER_ROMAN:
			return g_strdup (need_list_begin (self) ? "OL TYPE=I" : NULL);
		case HTML_LIST_TYPE_ORDERED_ARABIC:
			return g_strdup (need_list_begin (self) ? "OL TYPE=1" : NULL);
		}
	case HTML_CLUEFLOW_STYLE_NORMAL:
	default:
		return NULL;
	}
}

static gchar *
get_end_tag (HTMLObject *self)
{
	switch (HTML_CLUEFLOW (self)->style) {
	case HTML_CLUEFLOW_STYLE_H1:
		return g_strdup ("H1");
	case HTML_CLUEFLOW_STYLE_H2:
		return g_strdup ("H2");
	case HTML_CLUEFLOW_STYLE_H3:
		return g_strdup ("H3");
	case HTML_CLUEFLOW_STYLE_H4:
		return g_strdup ("H4");
	case HTML_CLUEFLOW_STYLE_H5:
		return g_strdup ("H5");
	case HTML_CLUEFLOW_STYLE_H6:
		return g_strdup ("H6");
	case HTML_CLUEFLOW_STYLE_ADDRESS:
		return g_strdup ("ADDRESS");
	case HTML_CLUEFLOW_STYLE_PRE:
		return g_strdup ("PRE");
	case HTML_CLUEFLOW_STYLE_LIST_ITEM:
		switch (HTML_CLUEFLOW (self)->item_type) {
		case HTML_LIST_TYPE_UNORDERED:
		case HTML_LIST_TYPE_MENU:
		case HTML_LIST_TYPE_DIR:
			return need_list_end (self) ? g_strdup ("UL") : NULL;
		default:
			return need_list_end (self) ? g_strdup ("OL") : NULL;
		}
	default:
		return NULL;
	}
}

static gboolean
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	HTMLClueFlow *clueflow;
	HTMLHAlignType halign;
	gchar *tag;
	gboolean start = TRUE, end = TRUE;

	clueflow = HTML_CLUEFLOW (self);
	halign = HTML_CLUE (self)->halign;

	if (! write_pre_tags (clueflow, state))
		return FALSE;

	if (is_similar (self, self->prev))
		start = FALSE;

	if (is_similar (self, self->next))
		end = FALSE;
	
	INDENT (clueflow);

	/* Start tag.  */
	tag = get_start_tag (self);
	if (start || is_item (clueflow)) {
		if (tag && ! html_engine_save_output_string (state, "<%s>", tag)) {
			g_free (tag);
			return FALSE;
		}
		if (is_item (clueflow)) {
			if (tag) {
				g_free (tag);
				if (! html_engine_save_output_string (state, "\n")) {
					return FALSE;
				}
				INDENT (clueflow);
			}
			tag = get_list_start_tag (self);
			if (! html_engine_save_output_string (state, "<%s>", tag)) {
				g_free (tag);
				return FALSE;
			}
		}
	}
	g_free (tag);

	/* Alignment tag.  */
	if (halign != HTML_HALIGN_NONE && halign != HTML_HALIGN_LEFT) {
		if (! html_engine_save_output_string
		    (state, "<DIV ALIGN=%s>",
		     html_engine_save_get_paragraph_align (html_alignment_to_paragraph (halign))))
			return FALSE;
	}

	/* Paragraph's content.  */
	if (! HTML_OBJECT_CLASS (&html_clue_class)->save (self, state))
		return FALSE;

	/* Close alignment tag.  */
	if (halign != HTML_HALIGN_NONE && halign != HTML_HALIGN_LEFT) {
		if (! html_engine_save_output_string (state, "</DIV>"))
			return FALSE;
	}

	/* End tag.  */
	tag = get_end_tag (self);
	if (end || is_item (clueflow)) {
		if (is_item (clueflow)) {
			if (! html_engine_save_output_string (state, "</LI>")) {
				g_free (tag);
				return FALSE;
			}
			if (tag) {
				if (! html_engine_save_output_string (state, "\n")) {
					g_free (tag);
					return FALSE;
				}
				INDENT_T (clueflow);
			}
		}
		if (tag && ! html_engine_save_output_string (state, "</%s>", tag)) {
			g_free (tag);
			return FALSE;
		}
	}
	g_free (tag);

	if (! html_engine_save_output_string (state, "\n"))
		return FALSE;

	return write_post_tags (HTML_CLUEFLOW (self), state);
}

static gint
string_append_nonbsp (GString *out, guchar *s, gint length)
{
	gint len = length;

	while (len--) {
		if (IS_UTF8_NBSP (s)) {
			g_string_append_c (out, ' ');
			s += 2;
			len--;
		} else {
			g_string_append_c (out, *s);
			s++;
		}
	}
	return length;
}

#define CLUEFLOW_ITEM_MARKER        "    * "
#define CLUEFLOW_ITEM_MARKER_PAD    "      "
#define CLUEFLOW_INDENT             "    "

static gchar *
plain_get_marker (HTMLClueFlow *flow, gint *pad, gchar **pad_indent)
{
	gchar *number;

	number = get_item_number_str (flow);
	if (number) {
		GString *str;
		gint len;
		gint i;

		len = strlen (number);
		if (len < 6) {
			gint to_add;

			to_add = 6 - len;
			str = g_string_new (number);
			g_string_append_c (str, ' ');
			len ++;
			for (i = 0; i < to_add; i++, len++)
				g_string_prepend_c (str, ' ');
			g_free (number);
			number = str->str;
			g_string_free (str, FALSE);
		} else {
			gchar *old;

			old = number;
			number = g_strconcat (number, " ", NULL);
			g_free (old);
			len ++;
		}

		*pad = len;
		str = g_string_new (NULL);
		for (i = 0; i < len; i ++)
			g_string_append_c (str, ' ');
		*pad_indent = str->str;
		g_string_free (str, FALSE);
	} else {
		number = g_strdup (CLUEFLOW_ITEM_MARKER);
		*pad = strlen (CLUEFLOW_ITEM_MARKER_PAD);
		*pad_indent = g_strdup (CLUEFLOW_ITEM_MARKER_PAD);
	}

	return number;
}

static gint 
plain_padding (HTMLClueFlow *flow, GString *out, gboolean firstline)
{
	gchar *item_pad_str = NULL, *item_marker = NULL;
	gint pad, item_pad = 0;
	gint i;

	if (is_item (flow))
		item_marker = plain_get_marker (flow, &item_pad, &item_pad_str);

	pad = flow->level * strlen (CLUEFLOW_INDENT) 
		+ item_pad;

	if (out) {
		for (i = 0; i < (gint) flow->level; i++) {
			g_string_append (out, CLUEFLOW_INDENT);
		}

		if (is_item (flow)) {
			if (firstline) {
				g_string_append (out, item_marker);
			} else {
				g_string_append (out, item_pad_str);
			}
		}
	}

	g_free (item_marker);
	g_free (item_pad_str);

	return pad;
}

static gboolean
save_plain (HTMLObject *self,
	    HTMLEngineSaveState *state,
	    gint requested_width)
{
	HTMLClueFlow *flow;
	HTMLEngineSaveState *buffer_state;
	GString *out = g_string_new ("");
	size_t len;
	gint pad;
	gboolean firstline = TRUE;
	gint max_len;

	flow = HTML_CLUEFLOW (self);

	pad = plain_padding (flow, NULL, FALSE);
	buffer_state = html_engine_save_buffer_new (state->engine);

	max_len = MAX (requested_width - pad, 0);
	/* buffer the paragraph's content into the save buffer */
	if (HTML_OBJECT_CLASS (&html_clue_class)->save_plain (self, 
							      buffer_state, 
							      max_len)) {
		guchar *s, *space;
		
		if (get_pre_padding (flow, calc_padding (state->engine->painter)) > 0)
			g_string_append (out, "\n");

		s = html_engine_save_buffer_peek_text (buffer_state);

		if (*s == 0) {
		        plain_padding (flow, out, TRUE);
			g_string_append (out, "\n");
		} else while (*s) {
			len = strcspn (s, "\n");
			
			if (flow->style != HTML_CLUEFLOW_STYLE_PRE) {

				if (g_utf8_strlen (s, len) > max_len) {
					space = g_utf8_offset_to_pointer (s, max_len);
					while (space 
					       && (*space != ' '))
						/* || (IS_UTF8_NBSP ((guchar *)g_utf8_find_next_char (space, NULL)))
						   || (IS_UTF8_NBSP ((guchar *)g_utf8_find_prev_char (s, space))))) */
						space = g_utf8_find_prev_char (s, space);
					
					if (space != NULL)
						len = space - s;
				}
			}
			
		        plain_padding (flow, out, firstline);
			s += string_append_nonbsp (out, s, len);
			
			/* Trim the space at the end */
			while (*s == ' ' || IS_UTF8_NBSP (s)) 
				s = g_utf8_next_char (s);
			
			if (*s == '\n') 
				s++;
			
			g_string_append_c (out, '\n');
			firstline = FALSE;
		}
		
		if (get_post_padding (flow, calc_padding (state->engine->painter)) > 0)
			g_string_append (out, "\n");
				
	}
	html_engine_save_buffer_free (buffer_state);
		
	if (!html_engine_save_output_string (state, "%s", out->str)) {
		g_string_free (out, TRUE);
		return FALSE;
	}		
	
	g_string_free (out, TRUE);
	return TRUE;
}


static GtkHTMLFontStyle
get_default_font_style (const HTMLClueFlow *self)
{
	GtkHTMLFontStyle style = 0;

	if (HTML_OBJECT (self)->parent && HTML_IS_TABLE_CELL (HTML_OBJECT (self)->parent)
	    && HTML_TABLE_CELL (HTML_OBJECT (self)->parent)->heading)
		style = GTK_HTML_FONT_STYLE_BOLD;

	switch (self->style) {
	case HTML_CLUEFLOW_STYLE_NORMAL:
	case HTML_CLUEFLOW_STYLE_LIST_ITEM:
		return style | GTK_HTML_FONT_STYLE_SIZE_3;
	case HTML_CLUEFLOW_STYLE_ADDRESS:
		return style | GTK_HTML_FONT_STYLE_SIZE_3 | GTK_HTML_FONT_STYLE_ITALIC;
	case HTML_CLUEFLOW_STYLE_PRE:
		return style | GTK_HTML_FONT_STYLE_SIZE_3 | GTK_HTML_FONT_STYLE_FIXED;
	case HTML_CLUEFLOW_STYLE_H1:
		return style | GTK_HTML_FONT_STYLE_SIZE_6 | GTK_HTML_FONT_STYLE_BOLD;
	case HTML_CLUEFLOW_STYLE_H2:
		return style | GTK_HTML_FONT_STYLE_SIZE_5 | GTK_HTML_FONT_STYLE_BOLD;
	case HTML_CLUEFLOW_STYLE_H3:
		return style | GTK_HTML_FONT_STYLE_SIZE_4 | GTK_HTML_FONT_STYLE_BOLD;
	case HTML_CLUEFLOW_STYLE_H4:
		return style | GTK_HTML_FONT_STYLE_SIZE_3 | GTK_HTML_FONT_STYLE_BOLD;
	case HTML_CLUEFLOW_STYLE_H5:
		return style | GTK_HTML_FONT_STYLE_SIZE_2 | GTK_HTML_FONT_STYLE_BOLD;
	case HTML_CLUEFLOW_STYLE_H6:
		return style | GTK_HTML_FONT_STYLE_SIZE_1 | GTK_HTML_FONT_STYLE_BOLD;
	default:
		g_warning ("Unexpected HTMLClueFlow style %d", self->style);
		return style | GTK_HTML_FONT_STYLE_DEFAULT;
	}
}

static void
search_set_info (HTMLObject *cur, HTMLSearch *info, guchar *text, guint pos, guint len)
{
	guint text_len = 0;
	guint cur_len;

	info->found_len = len;

	if (info->found) {
		g_list_free (info->found);
		info->found = NULL;
	}

	while (cur) {
		if (html_object_is_text (cur)) {
			cur_len = strlen (HTML_TEXT (cur)->text);
			if (text_len + cur_len > pos) {
				if (!info->found) {
					info->start_pos = g_utf8_pointer_to_offset (text + text_len,
										    text + pos);
				}
				info->found = g_list_append (info->found, cur);
			}
			text_len += cur_len;
			if (text_len >= pos+info->found_len) {
				info->stop_pos = info->start_pos + info->found_len;
				info->last     = HTML_OBJECT (cur);
				return;
			}
		} else if (HTML_OBJECT_TYPE (cur) != HTML_TYPE_TEXTSLAVE) {
			break;
		}		
		cur = cur->next;
	}

	g_assert_not_reached ();
}

/* search text objects ([TextMaster, LinkTextMaster], TextSlave*) */
static gboolean
search_text (HTMLObject **beg, HTMLSearch *info)
{
	HTMLObject *cur = *beg;
	HTMLObject *end = cur;
	HTMLObject *head;
	guchar *par, *pp;
	guint text_len;
	guint eq_len;
	gint  pos;
	gboolean retval = FALSE;

	/* printf ("search flow look for \"text\" %s\n", info->text); */

	/* first get flow text_len */
	text_len = 0;
	while (cur) {
		if (html_object_is_text (cur)) {
			text_len += strlen (HTML_TEXT (cur)->text);
			end = cur;
		} else if (HTML_OBJECT_TYPE (cur) != HTML_TYPE_TEXTSLAVE) {
			break;
		}
		cur = (info->forward) ? cur->next : cur->prev;
	}

	if (text_len > 0) {
		par = g_new (gchar, text_len+1);
		par [text_len] = 0;

		pp = (info->forward) ? par : par+text_len;

		/* now fill par with text */
		head = cur = (info->forward) ? *beg : end;
		cur = *beg;
		while (cur) {
			if (html_object_is_text (cur)) {
				if (!info->forward) {
					pp -= strlen (HTML_TEXT (cur)->text);
				}
				strncpy (pp, HTML_TEXT (cur)->text, strlen (HTML_TEXT (cur)->text));
				if (info->forward) {
					pp += strlen (HTML_TEXT (cur)->text);
				}
			} else if (HTML_OBJECT_TYPE (cur) != HTML_TYPE_TEXTSLAVE) {
				break;
			}		
			cur = (info->forward) ? cur->next : cur->prev;
		}

		/* set eq_len and pos counters */
		eq_len = 0;
		if (info->found) {
			pos = info->start_pos + ((info->forward) ? 1 : -1);
		} else {
			pos = (info->forward) ? 0 : text_len - 1;
		}

		/* FIXME make shorter text instead */
		if (!info->forward)
			par [pos+1] = 0;

		if ((info->forward && pos < text_len)
		    || (!info->forward && pos>0)) {
			if (info->reb) {
				/* regex search */
				gint rv;
#ifndef HAVE_GNU_REGEX
				regmatch_t match;
				guchar *p=par+pos;

				/* FIXME UTF8
				   replace &nbsp;'s with spaces
				while (*p) {
					if (*p == ENTITY_NBSP) {
						*p = ' ';
					}
					p += (info->forward) ? 1 : -1;
					} */

				while ((info->forward && pos < text_len)
				       || (!info->forward && pos >= 0)) {
					rv = regexec (info->reb,
						      par + pos,
						      1, &match, 0);
					if (rv == 0) {
						search_set_info (head, info, par,
								 pos + match.rm_so, match.rm_eo - match.rm_so);
						retval = TRUE;
						break;
					}
					pos += (info->forward) ? 1 : -1;
				}
#else
				rv = re_search (info->reb, par, text_len, pos,
						(info->forward) ? text_len-pos : -pos, NULL);
				if (rv>=0) {
					guint found_pos = rv;
					rv = re_match (info->reb, par, text_len, found_pos, NULL);
					if (rv < 0) {
						g_warning ("re_match (...) error");
					}
					search_set_info (head, info, par, found_pos, rv);
					retval = TRUE;
				} else {
					if (rv < -1) {
						g_warning ("re_search (...) error");
					}
				}
#endif
			} else {
				/* substring search - simple one - could be improved
				   go thru par and look for info->text */
				while (par [pos]) {
					if (info->trans [(guchar) info->text
							[(info->forward) ? eq_len : info->text_len - eq_len - 1]]
					    == info->trans [par [pos]]) {
						eq_len++;
						if (eq_len == info->text_len) {
							search_set_info (head, info, par,
									 pos - (info->forward ? eq_len-1 : 0),
									 info->text_len);
							retval=TRUE;
							break;
						}
					} else {
						pos += (info->forward) ? -eq_len : eq_len;
						eq_len = 0;
					}
					pos += (info->forward) ? 1 : -1;
				}
			}
		}
		g_free (par);
	}

	*beg = cur;

	return retval;
}

static gboolean
search (HTMLObject *obj, HTMLSearch *info)
{
	HTMLClue *clue = HTML_CLUE (obj);
	HTMLObject *cur;
	gboolean next = FALSE;

	/* does last search end here? */
	if (info->found) {
		cur  = HTML_OBJECT (info->found->data);
		next = TRUE;
	} else {
		/* search_next? */
		if (html_search_child_on_stack (info, obj)) {
			cur  = html_search_pop (info);
			cur  = (info->forward) ? cur->next : cur->prev;
			next = TRUE;
		} else {
			/* normal search */
			cur  = (info->forward) ? clue->head : clue->tail;
		}
	}
	while (cur) {
		if (html_object_is_text (cur)) {
			if (search_text (&cur, info))
				return TRUE;
		} else {
			html_search_push (info, cur);
			if (html_object_search (cur, info))
				return TRUE;
			html_search_pop (info);
			cur = (info->forward) ? cur->next : cur->prev;
		}
		if (info->found) {
			g_list_free (info->found);
			info->found = NULL;
			info->start_pos = 0;
		}
	}

	if (next) {
		return html_search_next_parent (info);
	}

	return FALSE;
}

static gboolean
search_next (HTMLObject *obj, HTMLSearch *info)
{
	return FALSE;
}

static gboolean
relayout (HTMLObject *self,
	  HTMLEngine *engine,
	  HTMLObject *child)
{
	gint mw;

	mw = html_object_calc_min_width (self, engine->painter);
	if (mw <= self->max_width)
		return (*HTML_OBJECT_CLASS (parent_class)->relayout) (self, engine, child);
	html_engine_calc_size (engine, FALSE);
	html_engine_draw (engine, 0, 0, engine->width, engine->height);

	return TRUE;
}


void
html_clueflow_type_init (void)
{
	html_clueflow_class_init (&html_clueflow_class, HTML_TYPE_CLUEFLOW, sizeof (HTMLClueFlow));
}

void
html_clueflow_class_init (HTMLClueFlowClass *klass,
			  HTMLType type,
			  guint size)
{
	HTMLClueClass *clue_class;
	HTMLObjectClass *object_class;

	clue_class = HTML_CLUE_CLASS (klass);
	object_class = HTML_OBJECT_CLASS (klass);

	html_clue_class_init (clue_class, type, size);

	/* FIXME destroy */

	object_class->copy = copy;
	object_class->op_cut = op_cut;
	object_class->op_copy = op_copy;
	object_class->split = split;
	object_class->merge = merge;
	object_class->calc_size = calc_size;
	object_class->set_max_width = set_max_width;
	object_class->calc_min_width = calc_min_width;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->draw = draw;
	object_class->draw_background = draw_background;
	object_class->save = save;
	object_class->save_plain = save_plain;
	object_class->check_point = check_point;
	object_class->append_selection_string = append_selection_string;
	object_class->search = search;
	object_class->search_next = search_next;
	object_class->relayout = relayout;
	object_class->get_recursive_length = get_recursive_length;

	klass->get_default_font_style = get_default_font_style;

	parent_class = &html_clue_class;
}

void
html_clueflow_init (HTMLClueFlow *clueflow, HTMLClueFlowClass *klass,
		    HTMLClueFlowStyle style, guint8 level, HTMLListType item_type, gint item_number)
{
	HTMLObject *object;
	HTMLClue *clue;

	object = HTML_OBJECT (clueflow);
	clue = HTML_CLUE (clueflow);

	html_clue_init (clue, HTML_CLUE_CLASS (klass));

	object->flags &= ~HTML_OBJECT_FLAG_FIXEDWIDTH;

	clue->valign = HTML_VALIGN_BOTTOM;
	clue->halign = HTML_HALIGN_NONE;

	clueflow->style = style;
	clueflow->level = level; 

	clueflow->item_type   = item_type;
	clueflow->item_number = item_number;
}

HTMLObject *
html_clueflow_new (HTMLClueFlowStyle style, guint8 level, HTMLListType item_type, gint item_number)
{
	HTMLClueFlow *clueflow;

	clueflow = g_new (HTMLClueFlow, 1);
	html_clueflow_init (clueflow, &html_clueflow_class, style, level, item_type, item_number);

	return HTML_OBJECT (clueflow);
}

HTMLObject *
html_clueflow_new_from_flow (HTMLClueFlow *flow)
{
	HTMLObject *o;

	o = html_clueflow_new (flow->style, flow->level, flow->item_type, flow->item_number);
	html_object_copy_data_from_object (o, HTML_OBJECT (flow));

	return o;
}


/* Virtual methods.  */

GtkHTMLFontStyle
html_clueflow_get_default_font_style (const HTMLClueFlow *self)
{
	g_return_val_if_fail (self != NULL, GTK_HTML_FONT_STYLE_DEFAULT);

	return (* HCF_CLASS (self)->get_default_font_style) (self);
}


/* Clue splitting (for editing).  */

/**
 * html_clue_split:
 * @clue: 
 * @child: 
 * 
 * Remove @child and its successors from @clue, and create a new clue
 * containing them.  The new clue has the same properties as the original clue.
 * 
 * Return value: A pointer to the new clue.
 **/
HTMLClueFlow *
html_clueflow_split (HTMLClueFlow *clue,
		     HTMLObject *child)
{
	HTMLClueFlow *new;
	HTMLObject *prev;

	g_return_val_if_fail (clue != NULL, NULL);
	g_return_val_if_fail (child != NULL, NULL);

	/* Create the new clue.  */

	new = HTML_CLUEFLOW (html_clueflow_new_from_flow (clue));

	/* Remove the children from the original clue.  */

	prev = child->prev;
	if (prev != NULL) {
		prev->next = NULL;
		HTML_CLUE (clue)->tail = prev;
	} else {
		HTML_CLUE (clue)->head = NULL;
		HTML_CLUE (clue)->tail = NULL;
	}

	child->prev = NULL;
	html_object_change_set (HTML_OBJECT (clue), HTML_CHANGE_ALL_CALC);

	/* Put the children into the new clue.  */

	html_clue_append (HTML_CLUE (new), child);

	/* Return the new clue.  */

	return new;
}


static void
relayout_and_draw (HTMLObject *object,
		   HTMLEngine *engine)
{
	if (engine == NULL)
		return;

	html_object_relayout (object, engine, NULL);
	html_engine_queue_draw (engine, object);
}

/* This performs a relayout of the object when the indentation level
   has changed.  In this case, we need to relayout the previous
   paragraph and the following one, because their padding might change
   after the level change. */
static void
relayout_with_siblings (HTMLClueFlow *flow,
			HTMLEngine *engine)
{
	if (engine == NULL)
		return;

	/* FIXME this is ugly and inefficient.  */

	if (HTML_OBJECT (flow)->prev != NULL)
		relayout_and_draw (HTML_OBJECT (flow)->prev, engine);

	relayout_and_draw (HTML_OBJECT (flow), engine);

	if (HTML_OBJECT (flow)->next != NULL)
		relayout_and_draw (HTML_OBJECT (flow)->next, engine);
}


void
html_clueflow_set_style (HTMLClueFlow *flow,
			 HTMLEngine *engine,
			 HTMLClueFlowStyle style)
{
	g_return_if_fail (flow != NULL);
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	if ((flow->style & HTML_CLUEFLOW_STYLE_PRE) ^ (style & HTML_CLUEFLOW_STYLE_PRE))
		html_object_clear_word_width (HTML_OBJECT (flow));
	html_object_change_set (HTML_OBJECT (flow), HTML_CHANGE_ALL);
	flow->style = style;
	if (style != HTML_CLUEFLOW_STYLE_LIST_ITEM)
		flow->item_number = 0;

	html_engine_schedule_update (engine);
	/* FIXME - make it more effective: relayout_with_siblings (flow, engine); */
}

void
html_clueflow_set_item_type (HTMLClueFlow *flow,
			     HTMLEngine *engine,
			     HTMLListType item_type)
{
	g_return_if_fail (flow != NULL);
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	html_object_change_set (HTML_OBJECT (flow), HTML_CHANGE_ALL);
	flow->item_type = item_type;
	update_item_number (HTML_OBJECT (flow));
	if (!items_are_relative (HTML_OBJECT (flow), HTML_OBJECT (flow)->next) && HTML_OBJECT (flow)->next)
		update_item_number (HTML_OBJECT (flow)->next);

	html_engine_schedule_update (engine);
	/* FIXME - make it more effective: relayout_with_siblings (flow, engine); */
}

HTMLClueFlowStyle
html_clueflow_get_style (HTMLClueFlow *flow)
{
	g_return_val_if_fail (flow != NULL, HTML_CLUEFLOW_STYLE_NORMAL);

	return flow->style;
}

HTMLListType
html_clueflow_get_item_type (HTMLClueFlow *flow)
{
	g_return_val_if_fail (flow != NULL, HTML_CLUEFLOW_STYLE_NORMAL);

	return flow->item_type;
}

void
html_clueflow_set_halignment (HTMLClueFlow *flow,
			      HTMLEngine *engine,
			      HTMLHAlignType alignment)
{
	g_return_if_fail (flow != NULL);
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	HTML_CLUE (flow)->halign = alignment;

	relayout_and_draw (HTML_OBJECT (flow), engine);
}

inline HTMLHAlignType
html_clueflow_get_halignment (HTMLClueFlow *flow)
{
	g_return_val_if_fail (flow != NULL, HTML_HALIGN_NONE);

	if (HTML_CLUE (flow)->halign == HTML_HALIGN_NONE) {
		if (HTML_OBJECT (flow)->parent && HTML_IS_TABLE_CELL (HTML_OBJECT (flow)->parent))
			return HTML_CLUE (HTML_OBJECT (flow)->parent)->halign == HTML_HALIGN_NONE
				? HTML_TABLE_CELL (HTML_OBJECT (flow)->parent)->heading ? HTML_HALIGN_CENTER : HTML_HALIGN_LEFT
				: HTML_CLUE (HTML_OBJECT (flow)->parent)->halign;
		else
			return HTML_CLUE (HTML_OBJECT (flow)->parent)->halign == HTML_HALIGN_NONE
				? HTML_HALIGN_LEFT : HTML_CLUE (HTML_OBJECT (flow)->parent)->halign;
	} else
		return HTML_CLUE (flow)->halign;
}

static void
update_items_after_indentation_change (HTMLClueFlow *flow)
{
	if (is_item (flow)) {
		update_item_number (HTML_OBJECT (flow));
		if (HTML_OBJECT (flow)->next && is_item (HTML_CLUEFLOW (HTML_OBJECT (flow)->next)))
			update_item_number (HTML_OBJECT (flow)->next);
	}
}

void
html_clueflow_modify_indentation_by_delta (HTMLClueFlow *flow,
					   HTMLEngine *engine,
					   gint indentation_delta)
{
	g_return_if_fail (flow != NULL);
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	if (indentation_delta == 0)
		return;

	if (indentation_delta > 0) {
		flow->level += indentation_delta;
	} else if ((- indentation_delta) < flow->level) {
		flow->level += indentation_delta;
	} else if (flow->level != 0) {
		flow->level = 0;
	} else {
		/* No change.  */
		return;
	}

	update_items_after_indentation_change (flow);

	relayout_with_siblings (flow, engine);
}

void
html_clueflow_set_indentation (HTMLClueFlow *flow,
			       HTMLEngine *engine,
			       guint8 indentation)
{
	g_return_if_fail (flow != NULL);
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	if (flow->level == indentation)
		return;

	flow->level = indentation;
	update_items_after_indentation_change (flow);

	relayout_with_siblings (flow, engine);
}

guint8
html_clueflow_get_indentation (HTMLClueFlow *flow)
{
	g_return_val_if_fail (flow != NULL, 0);

	return flow->level;
}

void
html_clueflow_set_properties (HTMLClueFlow *flow,
			      HTMLEngine *engine,
			      HTMLClueFlowStyle style,
			      guint8 indentation,
			      HTMLHAlignType alignment)
{
	g_return_if_fail (flow != NULL);
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	HTML_CLUE (flow)->halign = alignment;

	flow->style = style;
	flow->level = indentation;

	relayout_and_draw (HTML_OBJECT (flow), engine);
}

void
html_clueflow_get_properties (HTMLClueFlow *flow,
			      HTMLClueFlowStyle *style_return,
			      guint8 *indentation_return,
			      HTMLHAlignType *alignment_return)
{
	g_return_if_fail (flow != NULL);

	if (style_return != NULL)
		*style_return = flow->style;
	if (indentation_return != NULL)
		*indentation_return = flow->level;
	if (alignment_return != NULL)
		*alignment_return = HTML_CLUE (flow)->halign;
}


void
html_clueflow_remove_text_slaves (HTMLClueFlow *flow)
{
	HTMLClue *clue;
	HTMLObject *p;
	HTMLObject *pnext;

	g_return_if_fail (flow != NULL);

	clue = HTML_CLUE (flow);
	for (p = clue->head; p != NULL; p = pnext) {
		pnext = p->next;

		if (HTML_OBJECT_TYPE (p) == HTML_TYPE_TEXTSLAVE) {
			html_clue_remove (clue, p);
			html_object_destroy (p);
		}
	}
}

/* spell checking */

#include "htmlinterval.h"

static guint
get_text_bytes (HTMLClue *clue, HTMLInterval *i)
{
	HTMLObject *obj;
	guint bytes;

	g_assert (i);
	g_assert (i->from.object);
	g_assert (i->to.object);

	bytes = 0;
	obj = html_interval_get_head (i, HTML_OBJECT (clue));
	while (obj) {
		bytes += html_interval_get_bytes (i, obj);
		if (obj == i->to.object)
			break;
		obj = html_object_next_not_slave (obj);
	}

	return bytes;
}

static gchar *
get_text (HTMLClue *clue, HTMLInterval *i)
{
	HTMLObject *obj;
	guint cb, bytes = 0;
	gchar *text, *ct;

	bytes        = get_text_bytes (clue, i);
	ct           = text = g_malloc (bytes + 1);
	text [bytes] = 0;

	obj = html_interval_get_head (i, HTML_OBJECT (clue));
	while (obj) {
		cb = html_interval_get_bytes (i, obj);
		if (html_object_is_text (obj))
			strncpy (ct, HTML_TEXT (obj)->text + html_interval_get_start_index (i, obj), cb);
		else
			if (cb == 1) *ct = ' ';
			else memset (ct, ' ', cb);
		ct += cb;
		if (obj == i->to.object)
			break;
		obj = html_object_next_not_slave (obj);
	}

	/* printf ("get_text: %d \"%s\"\n", bytes, text); */

	return text;
}

static HTMLObject *
next_obj_and_clear (HTMLObject *obj, guint *off, gboolean *is_text, HTMLInterval *i)
{
	*off += html_object_get_length (obj) - html_interval_get_start (i, obj);
	obj = obj->next;
	if (obj && (*is_text = html_object_is_text (obj)))
		html_text_spell_errors_clear_interval (HTML_TEXT (obj), i);

	return obj;
}

static HTMLObject *
spell_check_word_mark (HTMLObject *obj, const gchar *text, const gchar *word, guint *off, HTMLInterval *i)
{
	guint w_off, ioff;
	guint len = g_utf8_strlen (word, -1);
	gboolean is_text;

	/* printf ("[not in dictionary word off: %d off: %d]\n", word - text, *off); */
	is_text = html_object_is_text (obj);
	w_off   = g_utf8_pointer_to_offset (text, word);
	while (obj && (!is_text || (is_text && *off + html_interval_get_length (i, obj) <= w_off)))
		obj = next_obj_and_clear (obj, off, &is_text, i);

	/* printf ("is_text: %d len: %d obj: %p off: %d\n", is_text, len, obj, *off); */
	if (obj && is_text) {
		gchar *t;
		guint tlen;
		guint toff;

		while (len) {
			toff  = w_off - *off;
			ioff  = html_interval_get_start (i, obj);
			tlen  = MIN (HTML_TEXT (obj)->text_len - toff - ioff, len);
			t     = HTML_TEXT (obj)->text;
			g_assert (!strncmp (word, g_utf8_offset_to_pointer (t, toff + ioff),
					    g_utf8_offset_to_pointer (t, toff + ioff + tlen)
					    - g_utf8_offset_to_pointer (t, toff + ioff)));
			/* printf ("add spell error - word: %s off: %d beg: %s len: %d\n",
			   word, *off, HTML_TEXT (obj)->text + toff, tlen); */
			html_text_spell_errors_add (HTML_TEXT (obj),
						    ioff + toff, tlen);
			len     -= tlen;
			w_off   += tlen;
			word     = g_utf8_offset_to_pointer (word, tlen);
			if (len)
				do obj = next_obj_and_clear (obj, off, &is_text, i); while (obj && !is_text);
			/* printf ("off: %d\n", *off); */
			g_assert (!len || obj);
		}
	}

	return obj;
}

static gchar *
begin_of_word (gchar *text, gchar *ct, gboolean *cited)
{
	gunichar uc;

	*cited = FALSE;
	do
		uc = g_utf8_get_char (ct);
	while (!html_selection_spell_word (uc, cited) && (ct = g_utf8_next_char (ct)) && *ct);

	return ct;
}

static gchar *
end_of_word (gchar *ct, gboolean cited)
{
	gunichar uc;
	gchar *cn;
	gboolean cited2;

	cited2 = FALSE;
	while (*ct && (cn = e_unicode_get_utf8 (ct, &uc))
	       && (html_selection_spell_word (uc, &cited2) || (!cited && cited2))) {
		ct = cn;
		cited2 = FALSE;
	}

	return ct;
}

static void
queue_draw (HTMLObject *o, HTMLEngine *e, HTMLInterval *i)
{
	if (html_object_is_text (o))
		html_text_queue_draw (HTML_TEXT (o), e, html_interval_get_start (i, o), html_interval_get_length (i, o));
}

void
html_clueflow_spell_check (HTMLClueFlow *flow, HTMLEngine *e, HTMLInterval *interval)
{
	HTMLObject *obj;
	HTMLClue *clue;
	guint off;
	gchar *text, *ct, *word;
	HTMLInterval *new_interval = NULL;

	g_return_if_fail (flow != NULL);
	g_return_if_fail (HTML_OBJECT_TYPE (flow) == HTML_TYPE_CLUEFLOW);

	/* if (interval)
	   printf ("html_clueflow_spell_check %p %p %d %d\n", i->from, i->to, i->from_offset, i->to_offset); */

	clue = HTML_CLUE (flow);
	if (!e->widget->editor_api
	    || !GTK_HTML_CLASS (GTK_OBJECT (e->widget)->klass)->properties->live_spell_check
	    || !clue || !clue->tail)
		return;

	off  = 0;

	if (!interval) {
		new_interval = html_interval_new (clue->head, clue->tail, 0, html_object_get_length (clue->tail));
		interval = new_interval;
	}

	text = get_text (clue, interval);
	obj  = html_interval_get_head (interval, HTML_OBJECT (flow));
	if (obj && html_object_is_text (obj))
		html_text_spell_errors_clear_interval (HTML_TEXT (obj), interval);

	if (text) {
		ct = text;
		while (*ct) {
			gboolean cited;

			word = ct = begin_of_word (text, ct, &cited);
			ct        =   end_of_word (ct, cited);

			/* test if we have found word */
			if (word != ct) {
				gint result;
				gchar bak;

				bak = *ct;
				*ct = 0;
				/* printf ("off %d going to test word: \"%s\"\n", off, word); */
				result = (*e->widget->editor_api->check_word) (e->widget, word, e->widget->editor_data);

				if (result == 1) {
					gboolean is_text = (obj) ? html_object_is_text (obj) : FALSE;
					while (obj && (!is_text
						       || (off + html_interval_get_length (interval, obj)
							   < g_utf8_pointer_to_offset (text, ct))))
						obj = next_obj_and_clear (obj, &off, &is_text, interval);
				} else if (obj)
						obj = spell_check_word_mark (obj, text, word, &off, interval);

				*ct = bak;
				if (*ct)
					ct = g_utf8_next_char (ct);
			}
		}
		g_free (text);

		if (!html_engine_frozen (e)) {
			/* html_engine_queue_clear (e); */
			html_interval_forall (interval, e, (HTMLObjectForallFunc) queue_draw, interval);
			html_engine_flush_draw_queue (e);
		}
		html_interval_destroy (new_interval);
	}
}

gboolean
html_clueflow_is_empty (HTMLClueFlow *flow)
{
	HTMLClue *clue;
	g_return_val_if_fail (HTML_IS_CLUEFLOW (flow), TRUE);

	clue = HTML_CLUE (flow);

	if (clue->head && html_object_is_text (clue->head)
	    && HTML_TEXT (clue->head)->text_len == 0 && !html_object_next_not_slave (clue->head))
		return TRUE;
	return FALSE;
}

gint
html_clueflow_get_line_offset (HTMLClueFlow *flow, HTMLPainter *painter, HTMLObject *child)
{
	HTMLObject *o, *head;
	gint line_offset;

	g_assert (HTML_IS_CLUEFLOW (flow));

	if (!html_clueflow_tabs (flow, painter))
		return -1;

	line_offset = 0;

	/* find head */
	o = head = child;
	while (o) {
		o = head->prev;
		if (o && o->y + o->descent - 1 < child->y - child->ascent)
			break;
		else
			head = o;
	}

	if (HTML_IS_TEXT_SLAVE (head)) {
		HTMLTextSlave *bol = HTML_TEXT_SLAVE (head);
		line_offset = html_text_text_line_length (html_text_get_text (bol->owner, bol->posStart),
							  0, bol->owner->text_len - bol->posStart);
		head = html_object_next_not_slave (head);
	}

	while (head) {
		if (head == child)
			break;
		line_offset += html_object_get_line_length (head, painter, line_offset);
		head = html_object_prev_not_slave (head);
	}
	/* printf ("lo: %d\n", line_offset); */
	return line_offset;
}

gboolean
html_clueflow_tabs (HTMLClueFlow *flow, HTMLPainter *p)
{
	return (flow && HTML_IS_CLUEFLOW (flow) && flow->style == HTML_CLUEFLOW_STYLE_PRE) || HTML_IS_PLAIN_PAINTER (p)
		? TRUE : FALSE;
}
