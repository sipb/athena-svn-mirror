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

#include "gtkhtml-properties.h"

#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlclue.h"
#include "htmlclueflow.h"
#include "htmlcluealigned.h"
#include "htmlentity.h"
#include "htmlengine-edit.h"
#include "htmlengine-save.h"
#include "htmlpainter.h"
#include "htmlplainpainter.h"
#include "htmlsearch.h"
#include "htmlselection.h"
#include "htmlsettings.h"
#include "htmltable.h"
#include "htmltablecell.h"
#include "htmltext.h"
#include "htmltextslave.h"	/* FIXME */

HTMLClueFlowClass html_clueflow_class;
static HTMLClueClass *parent_class = NULL;

#define HCF_CLASS(x) HTML_CLUEFLOW_CLASS (HTML_OBJECT (x)->klass)

inline HTMLHAlignType html_clueflow_get_halignment          (HTMLClueFlow *flow);
static gchar *        get_item_marker_str                   (HTMLClueFlow *flow, gboolean ascii_only);
static guint          get_post_padding                      (HTMLClueFlow *flow, 
							     guint pad);
static int            get_similar_depth                     (HTMLClueFlow *self, 
							     HTMLClueFlow *neighbor);

static void
copy_levels (GByteArray *dst, GByteArray *src)
{
	int i;

	g_byte_array_set_size (dst, src->len);

	for (i = 0; i < src->len; i++)
		dst->data[i] = src->data[i];
}

static gboolean
is_levels_equal (HTMLClueFlow *me, HTMLClueFlow *you)
{
	if (!you)
		return FALSE;

	if (me->levels->len != you->levels->len)
		return FALSE;

	if (me->levels->len == 0)
		return TRUE;

	return !memcmp (me->levels->data, you->levels->data, you->levels->len);
}

static inline gboolean
is_item (HTMLClueFlow *flow)
{
	return flow && flow->style == HTML_CLUEFLOW_STYLE_LIST_ITEM;
}

static void
destroy (HTMLObject *self)
{
	HTMLClueFlow *flow = HTML_CLUEFLOW (self);

	g_byte_array_free (flow->levels, TRUE);
	if (flow->item_color) {
		html_color_unref (flow->item_color);
		flow->item_color = NULL;
	}

	(* HTML_OBJECT_CLASS (parent_class)->destroy) (self);
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	HTML_CLUEFLOW (dest)->levels = html_clueflow_dup_levels (HTML_CLUEFLOW (self));
	HTML_CLUEFLOW (dest)->style = HTML_CLUEFLOW (self)->style;
	HTML_CLUEFLOW (dest)->item_type = HTML_CLUEFLOW (self)->item_type;
	HTML_CLUEFLOW (dest)->item_number = HTML_CLUEFLOW (self)->item_number;
	HTML_CLUEFLOW (dest)->clear = HTML_CLUEFLOW (self)->clear;
	HTML_CLUEFLOW (dest)->item_color = HTML_CLUEFLOW (self)->item_color;
	HTML_CLUEFLOW (dest)->indent_width = HTML_CLUEFLOW (self)->indent_width;
	
	if (HTML_CLUEFLOW (dest)->item_color)
		html_color_ref (HTML_CLUEFLOW (dest)->item_color);
}

static inline gboolean
is_blockquote (HTMLListType type)
{
	if ((type == HTML_LIST_TYPE_BLOCKQUOTE_CITE)
	    || (type == HTML_LIST_TYPE_BLOCKQUOTE))
		return TRUE;

	return FALSE;
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
	    || !is_levels_equal (flow, next)
	    || next->item_type != flow->item_type)
		return FALSE;

	return TRUE;
}

static HTMLObject *
get_prev_relative_item (HTMLObject *self)
{
	HTMLObject *prev;
	
	prev = self->prev;
	while (prev 
	       && HTML_IS_CLUEFLOW (prev) 
	       && (HTML_CLUEFLOW (prev)->levels->len > HTML_CLUEFLOW (self)->levels->len
		   || (HTML_CLUEFLOW (prev)->levels->len == HTML_CLUEFLOW (self)->levels->len
		       && !is_item (HTML_CLUEFLOW (prev))))
	       && !memcmp (HTML_CLUEFLOW (prev)->levels->data,
			   HTML_CLUEFLOW (self)->levels->data,
			   HTML_CLUEFLOW (self)->levels->len))
	       
		prev = prev->prev;

	return prev;
}

static HTMLObject *
get_next_relative_item (HTMLObject *self)
{
	HTMLObject *next;

	next = self->next;
	while (next 
	       && HTML_IS_CLUEFLOW (next)
	       && (HTML_CLUEFLOW (next)->levels->len > HTML_CLUEFLOW (self)->levels->len
		   || (HTML_CLUEFLOW (next)->levels->len == HTML_CLUEFLOW (self)->levels->len
		       && !is_item (HTML_CLUEFLOW (next))))
	       && !memcmp (HTML_CLUEFLOW (next)->levels->data,
			   HTML_CLUEFLOW (self)->levels->data, 
			   HTML_CLUEFLOW (self)->levels->len))
	       
		next = next->next;

	return next;
}

static void
update_item_number (HTMLObject *self, HTMLEngine *e)
{
	HTMLObject *prev, *next;

	if (!self || !is_item (HTML_CLUEFLOW (self)))
		return;

	/* printf ("update_item_number\n"); */

	prev = get_prev_relative_item (self);
	if (items_are_relative (prev, self))
		HTML_CLUEFLOW (self)->item_number = HTML_CLUEFLOW (prev)->item_number + 1;
	else if (is_item (HTML_CLUEFLOW (self)))
		HTML_CLUEFLOW (self)->item_number = 1;
	html_engine_queue_draw (e, self);

	next = self;
	while ((next = get_next_relative_item (next)) && items_are_relative (self, next)) {
		HTML_CLUEFLOW (next)->item_number = HTML_CLUEFLOW (self)->item_number + 1;
		html_engine_queue_draw (e, next);
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
		html_clue_remove_text_slaves (HTML_CLUE (self));
	o = cut
		? (*HTML_OBJECT_CLASS (parent_class)->op_cut) (self, e, from, to, left, right, len)
		: (*HTML_OBJECT_CLASS (parent_class)->op_copy) (self, NULL, e, from, to, len);

	if (!cut && o) {
		html_clue_remove_text_slaves (HTML_CLUE (o));
	}

	return o;
}

static HTMLObject *
op_copy (HTMLObject *self, HTMLObject *parent, HTMLEngine *e, GList *from, GList *to, guint *len)
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

	if (prev && from) {
		update_item_number (prev, e);
		if (prev->next == self)
			update_item_number (self, e);
	}
	if (next && to) {
		if (next->prev == self)
			update_item_number (self, e);
		update_item_number (next, e);
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
	html_clue_remove_text_slaves (HTML_CLUE (self));
	(*HTML_OBJECT_CLASS (parent_class)->split) (self, e, child, offset, level, left, right);

	update_item_number (self, e);
}

static gboolean
merge (HTMLObject *self, HTMLObject *with, HTMLEngine *e, GList **left, GList **right, HTMLCursor *cursor)
{
	HTMLClueFlow *cf1, *cf2;
	HTMLObject *cf2_next_relative;
	gboolean rv;

	cf1 = HTML_CLUEFLOW (self);
	cf2 = HTML_CLUEFLOW (with);

	html_clue_remove_text_slaves (HTML_CLUE (cf1));
	html_clue_remove_text_slaves (HTML_CLUE (cf2));

	cf2_next_relative = get_next_relative_item (with);

	set_tail_size (self);
	set_head_size (with);

	if (html_clueflow_is_empty (cf1)) {
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

	if (rv) {
		if (is_item (cf1)) {
			/* cf2 will be removed, update item numbers around
			   as if it was already removed - it has to have
			   the same item style as cf1 to not break item lists*/
			g_byte_array_free (cf2->levels, TRUE);
			cf2->levels = html_clueflow_dup_levels (cf1);
			cf2->style = cf1->style;
			cf2->item_type = cf1->item_type;

			update_item_number (self, e);
			cf1->item_number --;
			update_item_number (with, e);
			cf1->item_number ++;

			if (cf2_next_relative)
				update_item_number (cf2_next_relative, e);
		}
	
	}
		
	return rv;
}

static guint
calc_padding (HTMLPainter *painter)
{
	if (!HTML_IS_PLAIN_PAINTER (painter)) {
		return 2 * html_painter_get_space_width (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);
	}
	return 0;
}

static gboolean
is_cite (HTMLClueFlow *flow, gint level)
{
	if (flow->levels->data[level] == HTML_LIST_TYPE_BLOCKQUOTE_CITE)
		return TRUE;

	return FALSE;
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

static gboolean
need_blockquote_padding  (HTMLClueFlow *flow, HTMLClueFlow *prev)
{
	int i = get_similar_depth (flow, prev);
	
	/* 
	 * If the levels don't match up the the current flow
	 * level the padding should be handled the other way.
	 */
	if (i < flow->levels->len || flow->levels->len == 0) {
		if (i < prev->levels->len)
			return TRUE;
                else 
			return FALSE;
	}
	
	i = prev->levels->len - i;
	
	/*
	 * now we check each level greater than the current flow level
	 * and see if it is a blockquote and therefore needs padding
	 */
	while (i > 0) {
		HTMLListType type;
		
		type = prev->levels->data [prev->levels->len - i];

		if (is_blockquote (type)) {
			return TRUE;
		}
		i--;
	}

	/*
	 * If all the levels were items we don't need padding
	 */
	return FALSE;
}

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

		prev = HTML_CLUEFLOW (prev_object);

		if (!is_levels_equal (flow, prev)) {
			if (need_blockquote_padding (flow, prev))
				return pad;
			else 
				return 0;
  		}

		if (flow->style == HTML_CLUEFLOW_STYLE_PRE
		    && prev->style != HTML_CLUEFLOW_STYLE_PRE
		    && ! is_header (prev))
			return pad;

		if (is_header (flow) && ! is_header (prev))
			return pad;

		return 0;
	}

	if (! is_header (flow) && flow->levels->len == 0)
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

		next = HTML_CLUEFLOW (next_object);

		if (!is_levels_equal (next, flow)) {
			if (need_blockquote_padding (flow, next)) 
				return pad;
			else 
				return 0;
		}

		if (flow->style == HTML_CLUEFLOW_STYLE_PRE
		    && next->style != HTML_CLUEFLOW_STYLE_PRE
		    && ! is_header (next))
			return pad;

		if (is_header (flow))
			return pad;

		return 0;
	}

	if (! is_header (flow) && flow->levels->len == 0)
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
get_level_indent (HTMLClueFlow *flow,
		  gint level,
		  HTMLPainter *painter)
{
	guint indent = 0;
	gint i = 0;

	if (flow->levels->len > 0 || ! is_item (flow)) {
		guint cite_width, indent_width;

		cite_width   = html_painter_get_block_cite_width (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);
		indent_width = html_painter_get_block_indent_width (painter, GTK_HTML_FONT_STYLE_SIZE_3, NULL);
		
		while (i <= level) {
			switch (flow->levels->data[i]) {
			case HTML_LIST_TYPE_BLOCKQUOTE_CITE:
				indent += cite_width;
				break;
			case HTML_LIST_TYPE_GLOSSARY_DL:
				indent += 0;
				break;
			default:
				indent += indent_width;
				break;
			}
			i++;
		}
	} else {
		GtkHTMLFontStyle style;
		style = html_clueflow_get_default_font_style (flow);

		indent = 4 * html_painter_get_space_width (painter, style, NULL);
	}

	return indent;
}

static void
set_painter (HTMLObject *o, HTMLPainter *painter)
{
	HTML_CLUEFLOW (o)->indent_width = -1;
}

static guint
get_indent (HTMLClueFlow *flow,
	    HTMLPainter *painter)
{
	if (flow->indent_width < 0 )
		flow->indent_width = get_level_indent (flow, flow->levels->len -1, painter);

	return flow->indent_width;
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
	gint aligned_min_width = 0;
	gint w = 0;
	gboolean add;

	add = HTML_CLUEFLOW (o)->style == HTML_CLUEFLOW_STYLE_PRE;

	cur = HTML_CLUE (o)->head;
	while (cur) {
		if (cur->flags & HTML_OBJECT_FLAG_ALIGNED)
			aligned_min_width = MAX (aligned_min_width, html_object_calc_min_width (cur, painter));
		else {
			w += add
				? html_object_calc_preferred_width (cur, painter)
				: html_object_calc_min_width (cur, painter);
			if (!add || !cur->next) {
				if (min_width < w) min_width = w;
				w = 0;
			}
		}
		cur = cur->next;
	}

	return MAX (aligned_min_width, min_width) + get_indent (HTML_CLUEFLOW (o), painter);
}

static gint
pref_right_margin (HTMLPainter *p, HTMLClueFlow *clueflow, HTMLObject *o, gint y, gboolean with_aligned) 
{
	gint fixed_margin = html_object_get_right_margin (o, p, y, with_aligned);

	/* FIXME: this hack lets us wrap the display at 72 characters when we are using
	   a plain painter */
	  
	if (clueflow->style == HTML_CLUEFLOW_STYLE_PRE || ! HTML_IS_PLAIN_PAINTER(p))
		return fixed_margin;

	return MIN (fixed_margin, 72 * (MAX (html_painter_get_space_width (p, GTK_HTML_FONT_STYLE_SIZE_3 | GTK_HTML_FONT_STYLE_FIXED, NULL),
					     html_painter_get_e_width (p, GTK_HTML_FONT_STYLE_SIZE_3 | GTK_HTML_FONT_STYLE_FIXED, NULL))));
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
	/* NULL means: clear following rectangle */
	*changed_objs = g_list_prepend (*changed_objs, NULL);
}

static void
calc_margins (HTMLObject *o, HTMLPainter *painter, gint indent, gint *lmargin, gint *rmargin)
{
	*lmargin = html_object_get_left_margin (o->parent, painter, o->y, TRUE);
	if (indent > *lmargin)
		*lmargin = indent;
	*rmargin = pref_right_margin (painter, HTML_CLUEFLOW (o), o->parent, o->y, TRUE);
}

static inline gint
width_left (HTMLObject *o, gint x, gint rmargin)
{
	return HTML_CLUEFLOW (o)->style == HTML_CLUEFLOW_STYLE_PRE ? G_MAXINT : rmargin - x;
}

static gint
object_nb_width (HTMLObject *o, HTMLPainter *painter, gboolean lineBegin)
{
	if (HTML_IS_TEXT_SLAVE (o))
		return html_text_slave_get_nb_width (HTML_TEXT_SLAVE (o), painter, lineBegin);
		
	return html_object_calc_min_width (o, painter);
}

static inline gboolean
is_top_aligned (HTMLVAlignType valign)
{
	return valign == HTML_VALIGN_TOP;
}

static inline void
update_leafs_children_changed_size (HTMLObject *o, gboolean *leaf_children_changed_size)
{
	if (o && o->change & HTML_CHANGE_SIZE
	    && HTML_OBJECT_TYPE (o) != HTML_TYPE_TEXTSLAVE && !html_object_is_container (o))
		*leaf_children_changed_size = TRUE;
}

static inline void
update_height (HTMLObject *o, HTMLVAlignType valign, gint *a, gint *d, gint *height, gboolean *top)
{
	switch (valign) {
	case HTML_VALIGN_TOP:
		*top = TRUE;
		*height = MAX (*height, o->ascent + o->descent);
		break;
	case HTML_VALIGN_NONE:
	case HTML_VALIGN_BOTTOM:
		*a = MAX (*a, o->ascent);
		*d = MAX (*d, o->descent);
		*height = MAX (*height, *a + *d);
		break;
	case HTML_VALIGN_MIDDLE: {
		gint h, h2;

		h = o->ascent + o->descent;
		h2 = h / 2;
		*a = MAX (*a, h2);
		*d = MAX (*d, h - h2);
		*height = MAX (*height, *a + *d);
		break;
	}
	}
}

static inline void
update_top_height (HTMLObject *begin, HTMLObject *end, gint *a, gint *d, gint *height)
{
	while (begin && begin != end) {
		if (html_object_get_valign (begin) == HTML_VALIGN_TOP) {
			gint rest = begin->ascent + begin->descent - *a;

			if (rest > *d) {
				*d = rest;
				*height = MAX (*height, *a + *d);
			}
		}
		begin = begin->next;
	}
}

static inline void
update_line_positions (HTMLObject *clue, HTMLObject *begin, HTMLObject *end, gint left, gint a, gint d, gint height)
{
	gint xinc = 0;

	switch (html_clueflow_get_halignment (HTML_CLUEFLOW (clue))) {
	case HTML_HALIGN_NONE:
	case HTML_HALIGN_LEFT:
		xinc = 0;
		break;
	case HTML_HALIGN_CENTER:
		xinc = left / 2;
		break;
	case HTML_HALIGN_RIGHT:
		xinc = left;
		break;
	}

	while (begin && begin != end) {
		begin->x += xinc;

		switch (html_object_get_valign (begin)) {
		case HTML_VALIGN_NONE:
		case HTML_VALIGN_BOTTOM:
			begin->y = clue->ascent + a;
			break;
		case HTML_VALIGN_MIDDLE:
			begin->y = clue->ascent + (height - begin->ascent - begin->descent) / 2 + begin->ascent;
			break;
		case HTML_VALIGN_TOP:
			begin->y = clue->ascent + begin->ascent;
			break;
		}
		begin = begin->next;
	}
}


static HTMLObject *
layout_line (HTMLObject *o, HTMLPainter *painter, HTMLObject *begin,
	     GList **changed_objs, gboolean *leaf_children_changed_size,
	     gint *lmargin, gint *rmargin, gint indent)
{
	HTMLObject *cur;
	gboolean first = TRUE;
	gboolean top_align = FALSE;
	gboolean need_update_height = FALSE;
	gint old_y;
	gint x;
	gint start_lmargin;
	gint a, d, height;
	gint nb_width;

	if (html_object_is_text (begin)) {
		update_leafs_children_changed_size (begin, leaf_children_changed_size);
		/* this ever succeds and creates slaves */
		html_object_calc_size (begin, painter, changed_objs);
		html_object_fit_line (begin, painter, first, first, FALSE, 0);
		begin = begin->next;
	}
	cur = begin;

	old_y = o->y;
	if (!HTML_IS_TEXT_SLAVE (begin) || HTML_IS_TEXT (begin->prev))
		html_object_calc_size (begin, painter, changed_objs);

	a = d = height = 0;
	update_height (begin, html_object_get_valign (begin), &a, &d, &height, &top_align);

	nb_width = object_nb_width (begin, painter, first);
	if (*rmargin - *lmargin < nb_width)
		html_clue_find_free_area (HTML_CLUE (o->parent), painter, o->y,
					  nb_width, height,
					  indent, &o->y, lmargin, rmargin);

	x = start_lmargin = *lmargin;
	o->ascent += o->y - old_y;

	while (cur && !(cur->flags & HTML_OBJECT_FLAG_ALIGNED)) {
		HTMLFitType fit;
		HTMLVAlignType valign;

		update_leafs_children_changed_size (cur, leaf_children_changed_size);

		cur->x = x;
		if (cur != begin)
			html_object_calc_size (cur, painter, changed_objs);

		valign = html_object_get_valign (cur);
		if ((!is_top_aligned (valign) && (cur->ascent > a || cur->descent > d))
		    || cur->ascent + cur->descent > height) {
			nb_width = object_nb_width (cur, painter, first);
			old_y = o->y;
			html_clue_find_free_area (HTML_CLUE (o->parent), painter, o->y,
						  nb_width, height,
						  indent, &o->y, lmargin, rmargin);

			/* is there enough space for this object? */
			if (HTML_CLUEFLOW (o)->style != HTML_CLUEFLOW_STYLE_PRE && o->y != old_y && *rmargin - x < nb_width)
				break;
			need_update_height = TRUE;
		}

		cur->y = o->ascent + a;
		fit = html_object_fit_line (cur, painter, first, first, FALSE, width_left (o, x, *rmargin));
		first = FALSE;
		if (fit == HTML_FIT_NONE)
			break;

		if (need_update_height)
			update_height (cur, valign, &a, &d, &height, &top_align);
		need_update_height = FALSE;
		x += cur->width;
		cur = cur->next;

		if (fit == HTML_FIT_PARTIAL)
			break;
	}

	if (top_align)
		update_top_height (begin, cur, &a, &d, &height);
	update_line_positions (o, begin, cur, MAX (0, *rmargin - start_lmargin - x), a, d, height);

	o->y += height;
	o->ascent += height;

	calc_margins (o, painter, indent, lmargin, rmargin);

	return cur;
}

static HTMLObject *
layout_aligned (HTMLObject *o, HTMLPainter *painter, HTMLObject *cur,
		GList **changed_objs, gboolean *leaf_children_changed_size,
		gint *lmargin, gint *rmargin, gint indent, gboolean *changed)
{
	if (! html_clue_appended (HTML_CLUE (o->parent), HTML_CLUE (cur))) {
		html_object_calc_size (cur, painter, changed_objs);

		if (HTML_CLUE (cur)->halign == HTML_HALIGN_LEFT)
			html_clue_append_left_aligned (HTML_CLUE (o->parent), painter,
						       HTML_CLUE (cur), lmargin, rmargin, indent);
		else
			html_clue_append_right_aligned (HTML_CLUE (o->parent), painter,
							HTML_CLUE (cur), lmargin, rmargin, indent);
		*changed = TRUE;
	}

	return cur->next;
}

static gboolean
html_clue_flow_layout (HTMLObject *o, HTMLPainter *painter, GList **changed_objs, gboolean *leaf_children_changed_size)
{
	HTMLClueFlow *cf = HTML_CLUEFLOW (o);
	HTMLObject *cur = HTML_CLUE (o)->head;
	gint indent, lmargin, rmargin;
	gboolean changed = FALSE;

	/* prepare margins */
	indent = get_indent (cf, painter);
	calc_margins (o, painter, indent, &lmargin, &rmargin);

	while (cur) {
		if (cur->flags & HTML_OBJECT_FLAG_ALIGNED)
			cur = layout_aligned (o, painter, cur, changed_objs, leaf_children_changed_size,
					      &lmargin, &rmargin, indent, &changed);
		else
			cur = layout_line (o, painter, cur, changed_objs, leaf_children_changed_size,
					   &lmargin, &rmargin, indent);
	}

	return changed;
}

static gboolean
html_clue_flow_real_calc_size (HTMLObject *o, HTMLPainter *painter, GList **changed_objs)
{
	HTMLClueFlow *cf = HTML_CLUEFLOW (o);
	gint oa, od, ow, padding;
	gboolean leaf_children_changed_size = FALSE;
	gboolean changed, changed_size = FALSE;

	/* reset size */
	oa = o->ascent;
	od = o->descent;
	ow = o->width;

	cf->indent_width = -1;

	o->ascent = 0;
	o->descent = 0;
	o->width = MAX (o->max_width, html_object_calc_min_width (o, painter));

	/* calc size */
	padding = calc_padding (painter);
	add_pre_padding (cf, padding);
	changed = html_clue_flow_layout (o, painter, changed_objs, &leaf_children_changed_size);
	add_post_padding (cf, padding);

	/* take care about changes */
	if (o->ascent != oa || o->descent != od || o->width != ow)
		changed = changed_size = TRUE;

	if (changed_size || leaf_children_changed_size)
		if (changed_objs) {
			if (ow > o->max_width && o->width < ow)
				add_clear_area (changed_objs, o, o->width, ow - o->width);
			html_object_add_to_changed (changed_objs, o);
		}

	return changed;
}

static void
set_max_height (HTMLObject *o, HTMLPainter *painter, gint max_height)
{
}

static HTMLClearType
get_clear (HTMLObject *self)
{
	return HTML_CLUEFLOW (self)->clear;
}

static gint
calc_preferred_width (HTMLObject *o,
		      HTMLPainter *painter)
{
	HTMLObject *obj, *next;
	gint maxw = 0, w = 0;

	next = NULL;

	for (obj = HTML_CLUE (o)->head; obj != 0; obj = obj->next) {
		w += html_object_calc_preferred_width (obj, painter);

		if (!(next = html_object_next_not_slave (obj))) {

			/* remove trailing space width on the end of line which is not on end of paragraph */
			if (next && html_object_is_text (obj))
				w -= html_text_trail_space_width (HTML_TEXT (obj), painter);

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

	str = g_string_new (". ");

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
		return g_strdup ("?. ");

	str = g_string_new (". ");

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
get_item_marker_str (HTMLClueFlow *flow, gboolean ascii_only)
{
	HTMLListType type = flow->item_type;

	if (type == HTML_LIST_TYPE_BLOCKQUOTE && flow->levels->len > 0) {
		int i;

		for (i = flow->levels->len - 1; i >= 0; i --) {
			if (flow->levels->data [i] != HTML_LIST_TYPE_BLOCKQUOTE) {
				type = flow->levels->data [i];
				break;
			}
		}
	}

	switch (type) {
	case HTML_LIST_TYPE_ORDERED_ARABIC:
		return g_strdup_printf ("%d. ", flow->item_number);
	case HTML_LIST_TYPE_ORDERED_LOWER_ALPHA:
	case HTML_LIST_TYPE_ORDERED_UPPER_ALPHA:
		return get_alpha_value (flow->item_number, flow->item_type == HTML_LIST_TYPE_ORDERED_LOWER_ALPHA);
	case HTML_LIST_TYPE_ORDERED_LOWER_ROMAN:
	case HTML_LIST_TYPE_ORDERED_UPPER_ROMAN:
		return get_roman_value (flow->item_number, flow->item_type == HTML_LIST_TYPE_ORDERED_LOWER_ROMAN);
	case HTML_LIST_TYPE_UNORDERED:
		if (ascii_only)
			return g_strdup ("* ");
		else if (flow->levels->len == 0 || flow->levels->len & 1)
			return g_strdup ("\342\227\217 "); /* U+25CF BLACK CIRCLE */
		else
			return g_strdup ("\342\227\213 "); /* U+25CB WHITE CIRCLE */
	default:
		return NULL;
	}
}

static void
draw_gt_line (HTMLObject *cur, HTMLPainter *p, gint offset, gint x, gint y)
{
	gint cy, w, a, d, line_offset = 0;

	/* FIXME: cache items and glyphs? */
	html_painter_calc_text_size (p, HTML_BLOCK_CITE, 
				     strlen (HTML_BLOCK_CITE), NULL, NULL, NULL, 0, &line_offset,
				     GTK_HTML_FONT_STYLE_SIZE_3, NULL,
				     &w, &a, &d);

	cy = offset;
	while (cy + a <= cur->ascent) {
		/* FIXME: cache items and glyphs? */
		html_painter_draw_text (p, x, y + cur->y - cy,
					HTML_BLOCK_CITE, 1, NULL, NULL, NULL, 0, 0);
		cy += a + d;
	}

	cy = - offset + a + d;
	while (cy + d <= cur->descent) {
		/* FIXME: cache items and glyphs? */
		html_painter_draw_text (p, x, y + cur->y + cy,
					HTML_BLOCK_CITE, 1, NULL, NULL, NULL, 0, 0);
		cy += a + d;
	}
}
		
static void
draw_quotes (HTMLObject *self, HTMLPainter *painter, 
	     gint x, gint y, gint width, gint height,
	     gint tx, gint ty)
{
	HTMLClueFlow *flow;
	GdkRectangle paint, area, clip;
	int i;
	int indent = 0;
	int last_indent = 0;
	gint pixel_size = html_painter_get_pixel_size (painter);
	gboolean is_plain = HTML_IS_PLAIN_PAINTER (painter);
	HTMLEngine *e;

	if (painter->widget && GTK_IS_HTML (painter->widget))
		e = GTK_HTML (painter->widget)->engine;
	else
		return;
	
	flow = HTML_CLUEFLOW (self);

	for (i = 0; i < flow->levels->len; i++, last_indent = indent) {
		indent = get_level_indent (flow, i, painter);

		html_painter_set_pen (painter, &html_colorset_get_color_allocated (e->settings->color_set,
										   painter, HTMLLinkColor)->color);
		if (is_cite (flow, i)) {
			if (!is_plain) {
				area.x = self->x + indent - 5 * pixel_size;
				area.width = 2 * pixel_size;
				area.y = self->y - self->ascent;
				area.height = self->ascent + self->descent;
				
				clip.x = x;
				clip.width = width;
				clip.y = y;
				clip.height = height;
				
				if (!gdk_rectangle_intersect (&clip, &area, &paint))
					return;

				html_painter_fill_rect (painter, 
							paint.x + tx, paint.y + ty,
							paint.width, paint.height);
			} else {
				HTMLObject *cur = HTML_CLUE (self)->head;
				gint baseline = 0;
				while (cur) {
					if (cur->y != 0) {
						baseline = cur->y;
						break;
  					}
  					cur = cur->next;
  				}


				/* draw "> " quote characters in the plain case */ 
				html_painter_set_font_style (painter, 
							     html_clueflow_get_default_font_style (flow));
						
				html_painter_set_font_face  (painter, NULL);
				draw_gt_line (self, painter, self->ascent - baseline,
					      self->x + tx + last_indent, ty);
			}
		}
	}
}		

static void
draw_item (HTMLObject *self, HTMLPainter *painter, gint x, gint y, gint width, gint height, gint tx, gint ty)
{
	HTMLClueFlow *flow;
	HTMLObject *first;
	gchar *marker;
	HTMLEngine *e;

	if (painter->widget && GTK_IS_HTML (painter->widget))
		e = GTK_HTML (painter->widget)->engine;
	else
		return;

	first = HTML_CLUE (self)->head;
	if (html_object_is_text (first) && first->next)
		first = first->next;

	flow = HTML_CLUEFLOW (self);

	if (flow->item_color) {
		html_color_alloc (flow->item_color, painter);
		html_painter_set_pen (painter, &flow->item_color->color);
	} else
		html_painter_set_pen (painter, &html_colorset_get_color_allocated (e->settings->color_set,
										   painter, HTMLTextColor)->color);

	marker = get_item_marker_str (flow, HTML_IS_PLAIN_PAINTER (painter));
	if (marker) {
		gint width, len, line_offset = 0, asc, dsc;
		
		len   = g_utf8_strlen (marker, -1);
		/* FIXME: cache items and glyphs? */
		html_painter_calc_text_size (painter, marker, len, NULL, NULL, NULL, 0, &line_offset,
					     html_clueflow_get_default_font_style (flow), NULL, &width, &asc, &dsc);
		width += html_painter_get_space_width (painter, html_clueflow_get_default_font_style (flow), NULL);
		html_painter_set_font_style (painter, html_clueflow_get_default_font_style (flow));
		html_painter_set_font_face  (painter, NULL);
		/* FIXME: cache items and glyphs? */
		html_painter_draw_text (painter, self->x + first->x - width + tx,
					self->y - self->ascent + first->y + ty,
					marker, len, NULL, NULL, NULL, 0, 0);
	}
	g_free (marker);
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

	if (HTML_CLUE (self)->head != NULL)
		draw_quotes (self, painter, x, y, width, height, tx, ty);

	(* HTML_OBJECT_CLASS (&html_clue_class)->draw) (self, painter, x, y, width, height, tx, ty);
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
	
	/*
	 * shift any selection inside the indent block to the 
	 * left edge of the flow.
	 */
	if (for_cursor)
		x = MAX (x, get_indent (HTML_CLUEFLOW (self), painter));

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

inline static gint
get_level (HTMLClueFlow *cf)
{
	return cf->levels->len;
}

static gchar *
get_list_start_tag (HTMLClueFlow *self)
{
	switch (self->item_type) {
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
	case HTML_LIST_TYPE_GLOSSARY_DL:
		return g_strdup_printf ("DT");
	case HTML_LIST_TYPE_GLOSSARY_DD:
		return g_strdup_printf ("DD");
	default:
		return NULL;
	}

	return NULL;
}


static gchar *
get_start_tag (HTMLClueFlow *self)
{
	switch (self->style) {
	case HTML_CLUEFLOW_STYLE_H1:
		return "H1";
	case HTML_CLUEFLOW_STYLE_H2:
		return "H2";
	case HTML_CLUEFLOW_STYLE_H3:
		return "H3";
	case HTML_CLUEFLOW_STYLE_H4:
		return "H4";
	case HTML_CLUEFLOW_STYLE_H5:
		return "H5";
	case HTML_CLUEFLOW_STYLE_H6:
		return "H6";
	case HTML_CLUEFLOW_STYLE_ADDRESS:
		return "ADDRESS";
	case HTML_CLUEFLOW_STYLE_PRE:
		return "PRE";
	case HTML_CLUEFLOW_STYLE_LIST_ITEM:
		g_warning ("This should not be reached");
	case HTML_CLUEFLOW_STYLE_NORMAL:
	default:
		return NULL;
	}
}

static const char *
get_start_indent_item (HTMLListType type)
{
	switch (type) {
	case HTML_LIST_TYPE_UNORDERED:
	case HTML_LIST_TYPE_MENU:
	case HTML_LIST_TYPE_DIR:
		return "UL";
	case HTML_LIST_TYPE_ORDERED_LOWER_ALPHA:
		return "OL TYPE=a";
	case HTML_LIST_TYPE_ORDERED_UPPER_ALPHA:
		return "OL TYPE=A";
	case HTML_LIST_TYPE_ORDERED_LOWER_ROMAN:
		return "OL TYPE=i";
	case HTML_LIST_TYPE_ORDERED_UPPER_ROMAN:
		return "OL TYPE=I";
	case HTML_LIST_TYPE_ORDERED_ARABIC:
		return "OL TYPE=1";
	case HTML_LIST_TYPE_GLOSSARY_DD:
	case HTML_LIST_TYPE_GLOSSARY_DL:
		return "DL";
	case HTML_LIST_TYPE_BLOCKQUOTE_CITE:
		return "BLOCKQUOTE TYPE=CITE";
	case HTML_LIST_TYPE_BLOCKQUOTE:
		return "BLOCKQUOTE";
	}
	return "";		
}

static const char *
get_end_indent_item (HTMLListType type)
{
	switch (type) {
	case HTML_LIST_TYPE_BLOCKQUOTE:
	case HTML_LIST_TYPE_BLOCKQUOTE_CITE:
		return "BLOCKQUOTE";
	case HTML_LIST_TYPE_GLOSSARY_DD:
	case HTML_LIST_TYPE_GLOSSARY_DL:
		return "DL";
	case HTML_LIST_TYPE_UNORDERED:
	case HTML_LIST_TYPE_MENU:
	case HTML_LIST_TYPE_DIR:
		return "UL";
	default:
		return "OL";
	}
	return "";
}

static int
get_similar_depth (HTMLClueFlow *self, HTMLClueFlow *neighbor)
{
	int i;
	int max_depth;

	if (neighbor == NULL)
		return 0;

	max_depth = MIN (self->levels->len, neighbor->levels->len);

	for (i = 0; i < max_depth; i++) {
		if (self->levels->data[i] != neighbor->levels->data[i])
			break;
	}

	return i;
}

static gboolean
save_indent_string (HTMLClueFlow *self, HTMLEngineSaveState *state, const char *format, ...)
{
	va_list args;
	gboolean retval;

	if (self->style != HTML_CLUEFLOW_STYLE_PRE)
		if (!write_indent (state, self->levels->len))
			return FALSE;

	va_start (args, format);
	retval = html_engine_save_output_stringv (state, format, args);
	va_end (args);

	return retval;
}

static gboolean
write_flow_tag (HTMLClueFlow *self, HTMLEngineSaveState *state) 
{
	int d;
	HTMLClueFlow *next = NULL;
	HTMLClueFlow *prev = NULL;
	HTMLHAlignType halign;
	
	if (HTML_IS_CLUEFLOW (HTML_OBJECT (self)->next))
		next = HTML_CLUEFLOW (HTML_OBJECT (self)->next);
	    
	if (HTML_IS_CLUEFLOW (HTML_OBJECT (self)->prev))
		prev = HTML_CLUEFLOW (HTML_OBJECT (self)->prev);

	d = get_similar_depth (self, prev);
	if (is_item (self)) {
		char *li = get_list_start_tag (self);

		if (li && !save_indent_string (self, state, "<%s>", li)) {
			g_free (li);
			return FALSE;
		}
	} else if (is_levels_equal (self, prev) && prev->style == self->style) {
		if (!save_indent_string (self, state, ""))
			return FALSE;
	} else {
		char *start = get_start_tag (self);

		if (start) {
			if (!save_indent_string (self, state, "<%s>\n", start))
				return FALSE;
		} else {
			if (!save_indent_string (self, state, ""))
				return FALSE;
		}
	}

	halign = HTML_CLUE (self)->halign;
	/* Alignment tag.  */
	if (halign != HTML_HALIGN_NONE && halign != HTML_HALIGN_LEFT) {
		if (! html_engine_save_output_string
		    (state, "<DIV ALIGN=%s>",
		     html_engine_save_get_paragraph_align (html_alignment_to_paragraph (halign))))
			return FALSE;
	}

	if (!html_object_save_data (HTML_OBJECT (self), state))
		return FALSE;

	/* Paragraph's content.  */
	if (! HTML_OBJECT_CLASS (&html_clue_class)->save (HTML_OBJECT (self), state))
		return FALSE;

	/* Close alignment tag.  */
	if (halign != HTML_HALIGN_NONE && halign != HTML_HALIGN_LEFT) {
		if (! html_engine_save_output_string (state, "</DIV>"))
			return FALSE;
	}

	if (is_item (self)) {
		if (next && is_levels_equal (self, next) && !is_item (next) && !html_clueflow_contains_table (self)) {
			if (!html_engine_save_output_string (state, "<BR>\n"))
				return FALSE;
		} else if (!html_engine_save_output_string (state, "\n"))
			return FALSE;
	} else if (is_levels_equal (self, next) && self->style == next->style) {
		if (self->style != HTML_CLUEFLOW_STYLE_PRE && !html_clueflow_contains_table (self)) {
			if (!html_engine_save_output_string (state, "<BR>\n"))
				return FALSE;
		} else {
			if (!html_engine_save_output_string (state, "\n"))
				return FALSE;
		}
	} else {
		char *end = get_start_tag (self);

		if (self->style != HTML_CLUEFLOW_STYLE_PRE) {
			if ((!html_clueflow_contains_table (self) && !end && next && self->style == next->style) || html_clueflow_is_empty (self)) {
				if (!html_engine_save_output_string (state, "<BR>\n"))
					return FALSE;
			} else {
				if (!html_engine_save_output_string (state, "\n"))
					return FALSE;
			}
		} else {
			if (!html_engine_save_output_string (state, "\n"))
				return FALSE;
		}

		if (end) {
			if (!html_engine_save_output_string (state, "</%s>\n", end))
				return FALSE;
		}
	}
	
	return TRUE;
}

static gboolean
save (HTMLObject *s,
      HTMLEngineSaveState *state)
{
	HTMLClueFlow *self = HTML_CLUEFLOW (s);
	HTMLClueFlow *next = NULL;
	HTMLClueFlow *prev = NULL;
	int d;
	int i;
	
	if (HTML_IS_CLUEFLOW (HTML_OBJECT (self)->next))
		next = HTML_CLUEFLOW (HTML_OBJECT (self)->next);
	    
	if (HTML_IS_CLUEFLOW (HTML_OBJECT (self)->prev))
		prev = HTML_CLUEFLOW (HTML_OBJECT (self)->prev);

	d = i = get_similar_depth (self, prev);
	while (i < self->levels->len) {
		const char *stag = get_start_indent_item (self->levels->data[i]);
		
		if (!write_indent (state, i) 
		    || !html_engine_save_output_string (state, "<%s>\n", stag))
			return FALSE;
		
		i++;
	}

	if (!write_flow_tag (self, state))
	    return FALSE;

	i = self->levels->len - 1;
	d = get_similar_depth (self, next);
	while (i >= d) {
		const char *stag = get_end_indent_item (self->levels->data[i]);

		if (!write_indent (state, i)
		    || !html_engine_save_output_string (state, "</%s>\n", stag))
			return FALSE;

		i--;
	}

	return TRUE;
}

static void
write_item_marker (GString *pad_string, HTMLClueFlow *flow)
{
	char *marker;

	marker = get_item_marker_str (flow, TRUE);

	if (marker) {
		gint marker_len = strlen (marker);
		gint len = pad_string->len - 1;
		char *str = pad_string->str;

		while (len > 0) {
			if ((str[len - 1] != ' ') || (pad_string->len - len >= marker_len))
				break;
			else 
				len--;
		}

		if (len > 0)
			g_string_truncate (pad_string, len);

		g_string_append (pad_string, marker);
	}
}

static gint 
plain_padding (HTMLClueFlow *flow, GString *out, gboolean firstline)
{
	GString *pad_string = NULL;
	gint pad_len = 0;
	gint i;

	pad_string = g_string_new ("");

#define APPEND_PLAIN(w) \
        pad_len += strlen (w); \
        if (out) g_string_append (pad_string, w); 

	if (flow->levels->len) {
		for (i = 0; i < flow->levels->len; i++) {
			switch (flow->levels->data[i]) {
			case HTML_LIST_TYPE_BLOCKQUOTE_CITE:
				APPEND_PLAIN (HTML_BLOCK_CITE);
				break;
			case HTML_LIST_TYPE_GLOSSARY_DL:
				break;
			default:
				APPEND_PLAIN (HTML_BLOCK_INDENT);
				break;
			}
		}
	} else if (is_item (flow)) {
		/* item without a list block give it a little pading */
		APPEND_PLAIN ("    ");
	}

	if (is_item (flow) && firstline) {
		write_item_marker (pad_string, flow);
	}

	if (out) 
		g_string_append (out, pad_string->str);

	g_string_free (pad_string, TRUE);
	return pad_len;
}

static void
append_selection_string (HTMLObject *self,
			 GString *buffer)
{
        (*HTML_OBJECT_CLASS (parent_class)->append_selection_string) (self, buffer);

	if (self->selected) {
		g_string_append_c (buffer, '\n');
		plain_padding (HTML_CLUEFLOW (self), buffer, TRUE);
	}
}

static gboolean
save_plain (HTMLObject *self,
	    HTMLEngineSaveState *state,
	    gint requested_width)
{
	HTMLClueFlow *flow;
	HTMLEngineSaveState *buffer_state;
	GString *out = g_string_new ("");
	gint pad;
	gint align_pad;
	gboolean firstline = TRUE;
	gint max_len;

	flow = HTML_CLUEFLOW (self);

	pad = plain_padding (flow, NULL, FALSE);
	buffer_state = html_engine_save_buffer_new (state->engine, 
						    state->inline_frames);
	max_len = MAX (requested_width - pad, 0);
	/* buffer the paragraph's content into the save buffer */
	if (HTML_OBJECT_CLASS (&html_clue_class)->save_plain (self, 
							      buffer_state, 
							      max_len)) {
		guchar *s;
		int offset;
		
		if (get_pre_padding (flow, calc_padding (state->engine->painter)) > 0) {
		        plain_padding (flow, out, FALSE);
			g_string_append (out, "\n");
		}

		s = html_engine_save_buffer_peek_text (buffer_state);

		if (*s == 0) {
		        plain_padding (flow, out, TRUE);
			g_string_append (out, "\n");
		} else {
			PangoAttrList *attrs = pango_attr_list_new ();
			gint bytes = html_engine_save_buffer_peek_text_bytes (buffer_state), slen = g_utf8_strlen (s, -1), i, clen, n_items;
			GList *items_list, *cur;
			PangoContext *pc = gtk_widget_get_pango_context (GTK_WIDGET (state->engine->widget));
			PangoLogAttr *lattrs;
			PangoItem **items;
			gint len, skip;

			items_list = pango_itemize (pc, s, 0, bytes, attrs, NULL);
			lattrs = g_new (PangoLogAttr, slen + 1);
			n_items = g_list_length (items_list);
			items = g_new (PangoItem *, n_items);
			for (i = 0, cur = items_list; i < n_items; i ++, cur = cur->next)
				items [i] = (PangoItem *) cur->data;

			offset = 0;
			for (i = 0; i < n_items; i ++) {
				PangoItem tmp_item;
				int start_i, start_offset;

				start_i = i;
				start_offset = offset;
				offset += items [i]->num_chars;
				tmp_item = *items [i];
				while (i < n_items - 1) {
					if (tmp_item.analysis.lang_engine == items [i + 1]->analysis.lang_engine) {
						tmp_item.length += items [i + 1]->length;
						tmp_item.num_chars += items [i + 1]->num_chars;
						offset += items [i + 1]->num_chars;
						i ++;
					} else
						break;
				}

				pango_break (s + tmp_item.offset, tmp_item.length, &tmp_item.analysis, lattrs + start_offset, tmp_item.num_chars + 1);
			}

			html_text_remove_unwanted_line_breaks (s, slen, lattrs);

			g_list_free (items_list);
			for (i = 0; i < n_items; i ++)
				pango_item_free (items [i]);
			g_free (items);
			pango_attr_list_unref (attrs);

			clen = 0;
			while (*s) {
				len = strcspn (s, "\n");
				len = g_utf8_strlen (s, len);
				skip = 0;
			
				if ((flow->style != HTML_CLUEFLOW_STYLE_PRE) 
				    && !HTML_IS_TABLE (HTML_CLUE (flow)->head)) {
					if (len > max_len) {
						gboolean look_backward = TRUE;
						gint wi, wl;

						wl = clen + max_len;

						if (lattrs [wl].is_white) {

							while (lattrs [wl].is_white && wl < slen)
								wl ++;

							if (wl < slen && html_text_is_line_break (lattrs [wl]))
								look_backward = FALSE;
							else
								wl = clen + max_len;
						}

						if (look_backward) {
							while (wl > 0) {
								if (html_text_is_line_break (lattrs [wl]))
									break;
								wl --;
							}
						}

						if (wl > clen && wl < slen && html_text_is_line_break (lattrs [wl])) {
							wi = MIN (wl, clen + max_len);
							while (wi > 0 && lattrs [wi - 1].is_white)
								wi --;
							len = wi - clen;
							skip = wl - wi;
						}
					}
				}

				/* FIXME plain padding doesn't work properly with tables aligment
				 * at the moment.
				 */
				plain_padding (flow, out, firstline);

				switch (html_clueflow_get_halignment (flow)) {
				case HTML_HALIGN_RIGHT:
					align_pad = max_len - len;
					break;
				case HTML_HALIGN_CENTER:
					align_pad = (max_len - len) / 2;
					break;
				default:
					align_pad = 0;
					break;
				}
			
				while (align_pad > 0) {
					g_string_append_c (out, ' ');
					align_pad--;
				}

				bytes = ((guchar *) g_utf8_offset_to_pointer (s, len)) - s;
				html_engine_save_string_append_nonbsp (out, s, bytes);
				s += bytes;
				s = g_utf8_offset_to_pointer (s, skip);
				clen += len + skip;

				if (*s == '\n') {
					s++;
					clen ++;
				}
			
				g_string_append_c (out, '\n');
				firstline = FALSE;
			}
			g_free (lattrs);
		}

		if (get_post_padding (flow, calc_padding (state->engine->painter)) > 0) {
			plain_padding (flow, out, FALSE);
			g_string_append (out, "\n");
		}
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
search_set_info (HTMLObject *cur, HTMLSearch *info, guchar *text, guint index, guint bytes)
{
	guint text_bytes = 0;
	guint cur_bytes;

	info->found_bytes = bytes;

	if (info->found) {
		g_list_free (info->found);
		info->found = NULL;
	}

	while (cur) {
		if (html_object_is_text (cur)) {
			cur_bytes = HTML_TEXT (cur)->text_bytes;
			if (text_bytes + cur_bytes > index) {
				if (!info->found) {
					info->start_pos = g_utf8_pointer_to_offset (text + text_bytes,
										    text + index);
				}
				info->found = g_list_append (info->found, cur);
			}
			text_bytes += cur_bytes;
			if (text_bytes >= index + info->found_bytes) {
				info->stop_pos = info->start_pos + g_utf8_pointer_to_offset (text + index,
											     text + index + info->found_bytes);
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
	guint text_bytes;
	guint eq_bytes;
	gint index;
	gboolean retval = FALSE;

	/* printf ("search flow look for \"text\" %s\n", info->text); */

	/* first get flow text_bytes */
	text_bytes = 0;
	while (cur) {
		if (html_object_is_text (cur)) {
			text_bytes += HTML_TEXT (cur)->text_bytes;
			end = cur;
		} else if (HTML_OBJECT_TYPE (cur) != HTML_TYPE_TEXTSLAVE) {
			break;
		}
		cur = (info->forward) ? cur->next : cur->prev;
	}

	if (text_bytes > 0) {
		par = g_new (gchar, text_bytes + 1);
		par [text_bytes] = 0;

		pp = (info->forward) ? par : par + text_bytes;

		/* now fill par with text */
		head = cur = (info->forward) ? *beg : end;
		cur = *beg;
		while (cur) {
			if (html_object_is_text (cur)) {
				if (!info->forward) {
					pp -= HTML_TEXT (cur)->text_bytes;
				}
				strncpy (pp, HTML_TEXT (cur)->text, HTML_TEXT (cur)->text_bytes);
				if (info->forward) {
					pp += HTML_TEXT (cur)->text_bytes;
				}
			} else if (HTML_OBJECT_TYPE (cur) != HTML_TYPE_TEXTSLAVE) {
				break;
			}		
			cur = (info->forward) ? cur->next : cur->prev;
		}

		/* set eq_bytes and pos counters */
		eq_bytes = 0;
		if (info->found) {
			index = ((guchar *)g_utf8_offset_to_pointer (par, info->start_pos + ((info->forward) ? 1 : -1))) - par;
		} else {
			index = (info->forward) ? 0 : text_bytes - 1;
		}

		/* FIXME make shorter text instead */
		if (!info->forward)
			par [index+1] = 0;

		if ((info->forward && index < text_bytes)
		    || (!info->forward && index > 0)) {
			if (info->reb) {
				/* regex search */
				gint rv;
#ifndef HAVE_GNU_REGEX
				regmatch_t match;
				/* guchar *p=par+pos; */

				/* FIXME UTF8
				   replace &nbsp;'s with spaces
				while (*p) {
					if (*p == ENTITY_NBSP) {
						*p = ' ';
					}
					p += (info->forward) ? 1 : -1;
					} */

				while ((info->forward && index < text_bytes)
				       || (!info->forward && index >= 0)) {
					rv = regexec (info->reb,
						      par + index,
						      1, &match, 0);
					if (rv == 0) {
						search_set_info (head, info, par,
								 index + match.rm_so, match.rm_eo - match.rm_so);
						retval = TRUE;
						break;
					}
					index += (info->forward)
						? (((guchar *) g_utf8_next_char (par + index)) - par - index)
						: (((guchar *) g_utf8_prev_char (par + index)) - par - index);
				}
#else
				rv = re_search (info->reb, par, text_bytes, index,
						(info->forward) ? text_bytes - index : -index, NULL);
				if (rv >= 0) {
					guint found_index = rv;
					rv = re_match (info->reb, par, text_bytes, found_index, NULL);
					if (rv < 0) {
						g_warning ("re_match (...) error");
					}
					search_set_info (head, info, par, found_index, rv);
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
				while (par [index]) {
					if (info->trans [(guchar) info->text
							[(info->forward) ? eq_bytes : info->text_bytes - eq_bytes - 1]]
					    == info->trans [par [index]]) {
						eq_bytes ++;
						if (eq_bytes == info->text_bytes) {
							search_set_info (head, info, par,
									 index - (info->forward
										  ? -(((guchar *) g_utf8_next_char (par + index - eq_bytes)) - par - index)
										  : 0),
									 info->text_bytes);
							retval=TRUE;
							break;
						}
					} else {
						index += (info->forward) ? -eq_bytes : eq_bytes;
						eq_bytes = 0;
					}
					index += (info->forward)
						? (((guchar *) g_utf8_next_char (par + index)) - par - index)
						: (((guchar *) g_utf8_prev_char (par + index)) - par - index);
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
		gboolean found = FALSE;
		gboolean is_text;

		is_text = html_object_is_text (cur);

		if (is_text) {
			if (search_text (&cur, info))
				return TRUE;
		}

		if (info->found) {
			g_list_free (info->found);
			info->found = NULL;
			info->start_pos = 0;
			found = TRUE;
		}

		if (!is_text) {
			if (!found || (info->start_pos < 0 && info->forward) || (info->start_pos >= 0 && !info->forward)) {
				html_search_push (info, cur);
				if (html_object_search (cur, info))
					return TRUE;
				html_search_pop (info);
			}
			cur = (info->forward) ? cur->next : cur->prev;
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
	html_engine_draw (engine, engine->x_offset, engine->y_offset, engine->width, engine->height);

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

	object_class->destroy = destroy;
	object_class->copy = copy;
	object_class->op_cut = op_cut;
	object_class->op_copy = op_copy;
	object_class->split = split;
	object_class->merge = merge;
	object_class->calc_size = html_clue_flow_real_calc_size;
	object_class->set_max_width = set_max_width;
	object_class->set_max_height = set_max_height;
	object_class->calc_min_width = calc_min_width;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->draw = draw;
	object_class->save = save;
	object_class->save_plain = save_plain;
	object_class->check_point = check_point;
	object_class->append_selection_string = append_selection_string;
	object_class->search = search;
	object_class->search_next = search_next;
	object_class->relayout = relayout;
	object_class->get_recursive_length = get_recursive_length;
	object_class->get_clear = get_clear;
	object_class->set_painter = set_painter;

	klass->get_default_font_style = get_default_font_style;

	parent_class = &html_clue_class;
}

void
html_clueflow_init (HTMLClueFlow *clueflow, HTMLClueFlowClass *klass,
		    HTMLClueFlowStyle style, GByteArray *levels, HTMLListType item_type, gint item_number,
 		    HTMLClearType clear)
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
	clueflow->levels = levels; 
	clueflow->indent_width = -1;

	clueflow->item_type   = item_type;
	clueflow->item_number = item_number;
	clueflow->item_color = NULL;

	clueflow->clear = clear;
}

HTMLObject *
html_clueflow_new (HTMLClueFlowStyle style, GByteArray *levels, HTMLListType item_type, gint item_number, HTMLClearType clear)
{
	HTMLClueFlow *clueflow;

	clueflow = g_new (HTMLClueFlow, 1);
	html_clueflow_init (clueflow, &html_clueflow_class, style, levels, item_type, item_number, clear);

	return HTML_OBJECT (clueflow);
}

HTMLObject *
html_clueflow_new_from_flow (HTMLClueFlow *flow)
{
	HTMLObject *o;

	o = html_clueflow_new (flow->style, html_clueflow_dup_levels (flow), 
			       flow->item_type, flow->item_number, flow->clear);
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

	html_object_change_set_down (HTML_OBJECT (flow), HTML_CHANGE_ALL);
	flow->style = style;
	if (style != HTML_CLUEFLOW_STYLE_LIST_ITEM)
		flow->item_number = 0;

	html_engine_schedule_update (engine);
	/* FIXME - make it more effective: relayout_with_siblings (flow, engine); */
}

GByteArray *
html_clueflow_dup_levels (HTMLClueFlow *flow)
{
	GByteArray *levels;
	
	levels = g_byte_array_new ();
	copy_levels (levels, flow->levels);
	
	return levels;
}

void
html_clueflow_set_levels (HTMLClueFlow *flow,
			  HTMLEngine *engine,
			  GByteArray *levels)
{
	HTMLObject *next_relative;

	next_relative = get_next_relative_item (HTML_OBJECT (flow));
	copy_levels (flow->levels, levels);

	update_item_number (HTML_OBJECT (flow), engine);
	if (next_relative)
		update_item_number (next_relative, engine);

	relayout_with_siblings (flow, engine);
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

	if ((is_blockquote (item_type) != is_blockquote (flow->item_type)) && flow->levels->len)
		flow->levels->data[flow->levels->len - 1] = item_type;

	flow->item_type = item_type;

	update_item_number (HTML_OBJECT (flow), engine);
	if (!items_are_relative (HTML_OBJECT (flow), HTML_OBJECT (flow)->next) && HTML_OBJECT (flow)->next)
		update_item_number (HTML_OBJECT (flow)->next, engine);

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
	g_return_val_if_fail (flow != NULL, HTML_LIST_TYPE_BLOCKQUOTE);

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

void
html_clueflow_modify_indentation_by_delta (HTMLClueFlow *flow,
					   HTMLEngine *engine,
					   gint indentation_delta,
					   guint8 *indentation_levels)
{
	HTMLObject *next_relative;
	gint indentation;
	g_return_if_fail (flow != NULL);
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	next_relative = get_next_relative_item (HTML_OBJECT (flow));

	indentation = flow->levels->len + indentation_delta;
	indentation = indentation < 0 ? 0 : indentation;

	if (indentation_delta > 0)
		g_byte_array_append (flow->levels, indentation_levels, indentation_delta);
	else {
		g_byte_array_set_size (flow->levels, indentation);
		if (is_item (flow) && indentation < 1 && indentation_delta < 0) {
			html_clueflow_set_style (flow, engine, HTML_CLUEFLOW_STYLE_NORMAL);
			html_clueflow_set_item_type (flow, engine, HTML_LIST_TYPE_BLOCKQUOTE);
			html_object_change_set_down (HTML_OBJECT (flow), HTML_CHANGE_ALL);
		}
	}

	update_item_number (HTML_OBJECT (flow), engine);
	if (next_relative)
		update_item_number (next_relative, engine);
	relayout_with_siblings (flow, engine);
}

void
html_clueflow_set_indentation (HTMLClueFlow *flow,
			       HTMLEngine *engine,
			       gint indentation,
			       guint8 *indentation_levels)
{
	HTMLObject *next_relative;
	int i;
	g_return_if_fail (flow != NULL);
	g_return_if_fail (engine != NULL);
	g_return_if_fail (HTML_IS_ENGINE (engine));

	if (indentation < 0)
		indentation = 0;

	next_relative = get_next_relative_item (HTML_OBJECT (flow));

	g_byte_array_set_size (flow->levels, indentation);

	i = indentation;
	while (i--)
		flow->levels->data[i] = indentation_levels[i];

	update_item_number (HTML_OBJECT (flow), engine);
	if (next_relative)
		update_item_number (next_relative, engine);
	relayout_with_siblings (flow, engine);
}

guint8
html_clueflow_get_indentation (HTMLClueFlow *flow)
{
	g_return_val_if_fail (flow != NULL, 0);

	// FIXME levels
	return flow->levels->len;
}

#if 0
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
	html_clueflow_set_indentation (flow, engine, indentation);

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
		// FIXME levels
		*indentation_return = flow->levels->len;
	if (alignment_return != NULL)
		*alignment_return = HTML_CLUE (flow)->halign;
}
#endif
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
	gunichar uc, ucn;
	gchar *cn;
	gboolean cited2;

	cited2 = FALSE;
	while (*ct
	       && (uc = g_utf8_get_char (ct))
	       && (cn = g_utf8_next_char (ct))
	       && (html_selection_spell_word (uc, &cited2)
		   || (!cited && cited2)
		   || (cited && cited2 && (ucn = g_utf8_get_char (cn)) && g_unichar_isalpha (ucn)))) {
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
	if (!e->widget->editor_api || !gtk_html_get_inline_spelling (e->widget) || !clue || !clue->tail)
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

	if (!clue->head
	    || (clue->head && html_object_is_text (clue->head)
		&& HTML_TEXT (clue->head)->text_len == 0 && !html_object_next_not_slave (clue->head)))
		return TRUE;
	return FALSE;
}

gboolean
html_clueflow_contains_table (HTMLClueFlow *flow)
{
	HTMLClue *clue;
	g_return_val_if_fail (HTML_IS_CLUEFLOW (flow), FALSE);

	clue = HTML_CLUE (flow);

	if (clue->head && HTML_IS_TABLE (clue->head))
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
		if (o) {
			if (o->y + o->descent - 1 < child->y - child->ascent)
				break;
			else
				head = o;
		}
	}

	if (HTML_IS_TEXT_SLAVE (head)) {
		HTMLTextSlave *bol = HTML_TEXT_SLAVE (head);

		html_text_text_line_length (html_text_get_text (bol->owner, bol->posStart),
					    &line_offset, bol->owner->text_len - bol->posStart, NULL);
		head = html_object_next_not_slave (head);
	}

	while (head) {
		if (head == child)
			break;
		line_offset += html_object_get_line_length (head, painter, line_offset);
		head = html_object_next_not_slave (head);
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

void
html_clueflow_set_item_color (HTMLClueFlow *flow, HTMLColor *color)
{
	if (flow->item_color)
		html_color_unref (flow->item_color);
	if (color)
		html_color_ref (color);
	flow->item_color = color;
}

gboolean
html_clueflow_style_equals (HTMLClueFlow *cf1, HTMLClueFlow *cf2)
{
	if (!cf1 || !cf2
	    || !HTML_IS_CLUEFLOW (cf1) || !HTML_IS_CLUEFLOW (cf2)
	    || cf1->style != cf2->style
	    || (cf1->style == HTML_CLUEFLOW_STYLE_LIST_ITEM && cf1->item_type != cf2->item_type)
	    || !is_levels_equal (cf1, cf2))
		return FALSE;
	return TRUE;
}
