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

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

#include <pango/pango.h>

#include "htmltext.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlclueflow.h"
#include "htmlcursor.h"
#include "htmlgdkpainter.h"
#include "htmlplainpainter.h"
#include "htmlprinter.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-save.h"
#include "htmlentity.h"
#include "htmlsettings.h"
#include "htmltextslave.h"
#include "htmlundo.h"

HTMLTextClass html_text_class;
static HTMLObjectClass *parent_class = NULL;
static const PangoAttrClass html_pango_attr_font_size_klass;

#define HT_CLASS(x) HTML_TEXT_CLASS (HTML_OBJECT (x)->klass)

static SpellError * spell_error_new         (guint off, guint len);
static void         spell_error_destroy     (SpellError *se);
static void         move_spell_errors       (GList *spell_errors, guint offset, gint delta);
static GList *      remove_spell_errors     (GList *spell_errors, guint offset, guint len);
static void         remove_text_slaves      (HTMLObject *self);

/* void
debug_spell_errors (GList *se)
{
	for (;se;se = se->next)
		printf ("SE: %4d, %4d\n", ((SpellError *) se->data)->off, ((SpellError *) se->data)->len);
} */

static inline gboolean
is_in_the_save_cluev (HTMLObject *text, HTMLObject *o)
{
	return html_object_nth_parent (o, 2) == html_object_nth_parent (text, 2);
}

/* HTMLObject methods.  */

HTMLTextPangoInfo *
html_text_pango_info_new (gint n)
{
	HTMLTextPangoInfo *pi;

	pi = g_new (HTMLTextPangoInfo, 1);
	pi->n = n;
	pi->entries = g_new0 (HTMLTextPangoInfoEntry, n);
	pi->attrs = NULL;

	return pi;
}

void
html_text_pango_info_destroy (HTMLTextPangoInfo *pi)
{
	gint i;

	for (i = 0; i < pi->n; i ++) {
		pango_item_free (pi->entries [i].item);
		g_free (pi->entries [i].widths);
	}
	g_free (pi->entries);
	g_free (pi->attrs);
	g_free (pi);
}

static void
pango_info_destroy (HTMLText *text)
{
	if (text->pi) {
		html_text_pango_info_destroy (text->pi);
		text->pi = NULL;
	}
}

static void
free_links (GSList *list)
{
	if (list) {
		GSList *l;

		for (l = list; l; l = l->next)
			html_link_free ((Link *) l->data);
		g_slist_free (list);
	}
}

void
html_text_free_attrs (GSList *attrs)
{
	if (attrs) {
		GSList *l;

		for (l = attrs; l; l = l->next)
			pango_attribute_destroy ((PangoAttribute *) l->data);
		g_slist_free (attrs);
	}
}

static void
copy (HTMLObject *s,
      HTMLObject *d)
{
	HTMLText *src  = HTML_TEXT (s);
	HTMLText *dest = HTML_TEXT (d);
	GList *cur;
	GSList *csl;

	(* HTML_OBJECT_CLASS (parent_class)->copy) (s, d);

	dest->text = g_strdup (src->text);
	dest->text_len      = src->text_len;
	dest->text_bytes    = src->text_bytes;
	dest->font_style    = src->font_style;
	dest->face          = g_strdup (src->face);
	dest->color         = src->color;
	dest->select_start  = 0;
	dest->select_length = 0;
	dest->attr_list     = pango_attr_list_copy (src->attr_list);
	dest->extra_attr_list = src->extra_attr_list ? pango_attr_list_copy (src->extra_attr_list) : NULL;

	html_color_ref (dest->color);

	dest->spell_errors = g_list_copy (src->spell_errors);
	cur = dest->spell_errors;
	while (cur) {
		SpellError *se = (SpellError *) cur->data;
		cur->data = spell_error_new (se->off, se->len);
		cur = cur->next;
	}

	dest->links = g_slist_copy (src->links);

	for (csl = dest->links; csl; csl = csl->next)
		csl->data = html_link_dup ((Link *) csl->data);

	dest->pi = NULL;
}

/* static void
debug_word_width (HTMLText *t)
{
	guint i;

	printf ("words: %d | ", t->words);
	for (i = 0; i < t->words; i ++)
		printf ("%d ", t->word_width [i]);
	printf ("\n");
}

static void
word_get_position (HTMLText *text, guint off, guint *word_out, guint *left_out, guint *right_out)
{
	const gchar *s, *ls;
	guint coff, loff;

	coff      = 0;
	*word_out = 0;
	s         = text->text;
	do {
		ls    = s;
		loff  = coff;
		s     = strchr (s, ' ');
		coff += s ? g_utf8_pointer_to_offset (ls, s) : g_utf8_strlen (ls, -1);
		(*word_out) ++;
		if (s)
			s ++;
	} while (s && coff < off);

	*left_out  = off - loff;
	*right_out = coff - off;

	printf ("get position w: %d l: %d r: %d\n", *word_out, *left_out, *right_out);
} */

static gboolean
cut_attr_list_filter (PangoAttribute *attr, gpointer data)
{
	PangoAttribute *range = (PangoAttribute *) data;
	gint delta;

	if (attr->start_index >= range->start_index && attr->end_index <= range->end_index)
		return TRUE;

	delta = range->end_index - range->start_index;
	if (attr->start_index > range->end_index) {
		attr->start_index -= delta;
		attr->end_index -= delta;
	} else if (attr->start_index > range->start_index) {
		attr->start_index = range->start_index;
		attr->end_index -= delta;
		if (attr->end_index <= attr->start_index)
			return TRUE;
	} else if (attr->end_index >= range->end_index)
		attr->end_index -= delta;
	else if (attr->end_index >= range->start_index)
		attr->end_index = range->start_index;

	return FALSE;
}

static void
cut_attr_list_list (PangoAttrList *attr_list, gint begin_index, gint end_index)
{
	PangoAttrList *removed;
	PangoAttribute range;

	range.start_index = begin_index;
	range.end_index = end_index;

	removed = pango_attr_list_filter (attr_list, cut_attr_list_filter, &range);
	if (removed)
		pango_attr_list_unref (removed);
}

static void
cut_attr_list (HTMLText *text, gint begin_index, gint end_index)
{
	cut_attr_list_list (text->attr_list, begin_index, end_index);
	if (text->extra_attr_list)
		cut_attr_list_list (text->extra_attr_list, begin_index, end_index);
}

static void
cut_links_full (HTMLText *text, int start_offset, int end_offset, int start_index, int end_index, int shift_offset, int shift_index)
{
	GSList *l, *next;
	Link *link;

	for (l = text->links; l; l = next) {
		next = l->next;
		link = (Link *) l->data;

		if (start_offset <= link->start_offset && link->end_offset <= end_offset) {
			html_link_free (link);
			text->links = g_slist_delete_link (text->links, l);
		} else if (end_offset <= link->start_offset) {
			link->start_offset -= shift_offset;
			link->start_index -= shift_index;
			link->end_offset -= shift_offset;
			link->end_index -= shift_index;
		} else if (start_offset <= link->start_offset)  {
			link->start_offset = end_offset - shift_offset;
			link->end_offset -= shift_offset;
			link->start_index = end_index - shift_index;
			link->end_index -= shift_index;
		} else if (end_offset <= link->end_offset) {
			if (shift_offset > 0) {
				link->end_offset -= shift_offset;
				link->end_index -= shift_index;
			} else {
				if (link->end_offset == end_offset) {
					link->end_offset = start_offset;
					link->end_index = start_index;
				} else if (link->start_offset == start_offset) {
					link->start_offset = end_offset;
					link->start_index = end_index;
				} else {
					Link *dup = html_link_dup (link);

					link->start_offset = end_offset;
					link->start_index = end_index;
					dup->end_offset = start_offset;
					dup->end_index = start_index;

					l = g_slist_prepend (l, dup);
					next = l->next->next;
				}
			}
		} else if (start_offset < link->end_offset) {
			link->end_offset = start_offset;
			link->end_index = start_index;
		}
	}
}

static void
cut_links (HTMLText *text, int start_offset, int end_offset, int start_index, int end_index)
{
	cut_links_full (text, start_offset, end_offset, start_index, end_index, end_offset - start_offset, end_index - start_index);
}

HTMLObject *
html_text_op_copy_helper (HTMLText *text, GList *from, GList *to, guint *len)
{
	HTMLObject *rv;
	HTMLText *rvt;
	gchar *tail, *nt;
	gint begin, end, begin_index, end_index;

	begin = (from) ? GPOINTER_TO_INT (from->data) : 0;
	end   = (to)   ? GPOINTER_TO_INT (to->data)   : text->text_len;

	tail = html_text_get_text (text, end);
	begin_index = html_text_get_index (text, begin);
	end_index = tail - text->text;

	*len += end - begin;

	rv = html_object_dup (HTML_OBJECT (text));
	rvt = HTML_TEXT (rv);
	rvt->text_len = end - begin;
	rvt->text_bytes = end_index - begin_index;
	nt = g_strndup (rvt->text + begin_index, rvt->text_bytes);
	g_free (rvt->text);
	rvt->text = nt;

	rvt->spell_errors = remove_spell_errors (rvt->spell_errors, 0, begin);
	rvt->spell_errors = remove_spell_errors (rvt->spell_errors, end, text->text_len - end);

	if (end_index < text->text_bytes)
		cut_attr_list (rvt, end_index, text->text_bytes);
	if (begin_index > 0)
		cut_attr_list (rvt, 0, begin_index);
	if (end < text->text_len)
		cut_links (rvt, end, text->text_len, end_index, text->text_bytes);
	if (begin > 0)
		cut_links (rvt, 0, begin, 0, begin_index);

	return rv;
}

HTMLObject *
html_text_op_cut_helper (HTMLText *text, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right, guint *len)
{
	HTMLObject *rv;
	HTMLText *rvt; 
	gint begin, end;

	begin = (from) ? GPOINTER_TO_INT (from->data) : 0;
	end   = (to)   ? GPOINTER_TO_INT (to->data)   : text->text_len;

	g_assert (begin <= end);
	g_assert (end <= text->text_len);

	/* printf ("before cut '%s'\n", text->text);
	   debug_word_width (text); */

	remove_text_slaves (HTML_OBJECT (text));
	if (!html_object_could_remove_whole (HTML_OBJECT (text), from, to, left, right) || begin || end < text->text_len) {
		gchar *nt, *tail;
		gint begin_index, end_index;

		if (begin == end)
			return HTML_OBJECT (html_text_new_with_len ("", 0, text->font_style, text->color));

		rv = html_object_dup (HTML_OBJECT (text));
		rvt = HTML_TEXT (rv);

		tail = html_text_get_text (text, end);
		begin_index = html_text_get_index (text, begin);
		end_index = tail - text->text;
		text->text_bytes -= tail - (text->text + begin_index);
		text->text [begin_index] = 0;
		cut_attr_list (text, begin_index, end_index);
		if (end_index < rvt->text_bytes)
			cut_attr_list (rvt, end_index, rvt->text_bytes);
		if (begin_index > 0)
			cut_attr_list (rvt, 0, begin_index);
		cut_links (text, begin, end, begin_index, end_index);
		if (end < rvt->text_len)
			cut_links (rvt, end, rvt->text_len, end_index, rvt->text_bytes);
		if (begin > 0)
			cut_links (rvt, 0, begin, 0, begin_index);
		nt = g_strconcat (text->text, tail, NULL);
		g_free (text->text);

		rvt->spell_errors = remove_spell_errors (rvt->spell_errors, 0, begin);
		rvt->spell_errors = remove_spell_errors (rvt->spell_errors, end, text->text_len - end);
		move_spell_errors (rvt->spell_errors, begin, -begin);

		text->text = nt;
		text->text_len -= end - begin;
		*len           += end - begin;

		nt = g_strndup (rvt->text + begin_index, end_index - begin_index);
		g_free (rvt->text);
		rvt->text = nt;
		rvt->text_len = end - begin;
		rvt->text_bytes = end_index - begin_index;

		text->spell_errors = remove_spell_errors (text->spell_errors, begin, end - begin);
		move_spell_errors (text->spell_errors, end, - (end - begin));

		html_text_convert_nbsp (text, TRUE);
		html_text_convert_nbsp (rvt, TRUE);
		pango_info_destroy (text);
	} else {
		text->spell_errors = remove_spell_errors (text->spell_errors, 0, text->text_len);
		html_object_move_cursor_before_remove (HTML_OBJECT (text), e);
		html_object_change_set (HTML_OBJECT (text)->parent, HTML_CHANGE_ALL_CALC);
		/* force parent redraw */
		HTML_OBJECT (text)->parent->width = 0;
		html_object_remove_child (HTML_OBJECT (text)->parent, HTML_OBJECT (text));

		rv    = HTML_OBJECT (text);
		*len += text->text_len;
	}

	html_object_change_set (HTML_OBJECT (text), HTML_CHANGE_ALL_CALC);

	/* printf ("after cut '%s'\n", text->text);
	   debug_word_width (text); */

	return rv;
}

static HTMLObject *
op_copy (HTMLObject *self, HTMLObject *parent, HTMLEngine *e, GList *from, GList *to, guint *len)
{
	return html_text_op_copy_helper (HTML_TEXT (self), from, to, len);
}

static HTMLObject *
op_cut (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right, guint *len)
{
	return html_text_op_cut_helper (HTML_TEXT (self), e, from, to, left, right, len);
}

static void
merge_links (HTMLText *t1, HTMLText *t2)
{
	Link *tail, *head;
	GSList *l;

	if (t2->links) {
		for (l = t2->links; l; l = l->next) {
			Link *link = (Link *) l->data;

			link->start_offset += t1->text_len;
			link->start_index += t1->text_bytes;
			link->end_offset += t1->text_len;
			link->end_index += t1->text_bytes;
		}

		if (t1->links) {
			head = (Link *) t1->links->data;
			tail = (Link *) g_slist_last (t2->links)->data;

			if (tail->start_offset == head->end_offset && html_link_equal (head, tail)) {
				tail->start_offset = head->start_offset;
				tail->start_index = head->start_index;
				html_link_free (head);
				t1->links = g_slist_delete_link (t1->links, t1->links);
			}
		}

		t1->links = g_slist_concat (t2->links, t1->links);
		t2->links = NULL;
	}
}

static gboolean
object_merge (HTMLObject *self, HTMLObject *with, HTMLEngine *e, GList **left, GList **right, HTMLCursor *cursor)
{
	HTMLText *t1, *t2;
	gchar *to_free;

	t1 = HTML_TEXT (self);
	t2 = HTML_TEXT (with);

	/* printf ("merge '%s' '%s'\n", t1->text, t2->text); */

	/* merge_word_width (t1, t2, e->painter); */

	if (e->cursor->object == with) {
		e->cursor->object  = self;
		e->cursor->offset += t1->text_len;
	}

	/* printf ("--- before merge\n");
	   debug_spell_errors (t1->spell_errors);
	   printf ("---\n");
	   debug_spell_errors (t2->spell_errors);
	   printf ("---\n");
	*/
	move_spell_errors (t2->spell_errors, 0, t1->text_len);
	t1->spell_errors = g_list_concat (t1->spell_errors, t2->spell_errors);
	t2->spell_errors = NULL;

	pango_attr_list_splice (t1->attr_list, t2->attr_list, t1->text_bytes, t2->text_bytes);
	if (t2->extra_attr_list) {
		if (!t1->extra_attr_list)
			t1->extra_attr_list = pango_attr_list_new ();
		pango_attr_list_splice (t1->extra_attr_list, t2->extra_attr_list, t1->text_bytes, t2->text_bytes);
	}
	merge_links (t1, t2);

	to_free       = t1->text;
	t1->text      = g_strconcat (t1->text, t2->text, NULL);
	t1->text_len += t2->text_len;
	t1->text_bytes += t2->text_bytes;
	g_free (to_free);
	html_text_convert_nbsp (t1, TRUE);
	html_object_change_set (self, HTML_CHANGE_ALL_CALC);
	pango_info_destroy (t1);
	pango_info_destroy (t2);

	/* html_text_request_word_width (t1, e->painter); */
	/* printf ("merged '%s'\n", t1->text);
	   printf ("--- after merge\n");
	   debug_spell_errors (t1->spell_errors);
	   printf ("---\n"); */

	return TRUE;
}

static gboolean
split_attrs_filter_head (PangoAttribute *attr, gpointer data)
{
	gint index = GPOINTER_TO_INT (data);

	if (attr->start_index >= index)
		return TRUE;
	else if (attr->end_index > index)
		attr->end_index = index;

	return FALSE;
}

static gboolean
split_attrs_filter_tail (PangoAttribute *attr, gpointer data)
{
	gint index = GPOINTER_TO_INT (data);
	
	if (attr->end_index <= index)
		return TRUE;

	if (attr->start_index > index)
		attr->start_index -= index;
	else
		attr->start_index = 0;
	attr->end_index -= index;

	return FALSE;
}

static void
split_attrs (HTMLText *t1, HTMLText *t2, gint index)
{
	PangoAttrList *delete;

	delete = pango_attr_list_filter (t1->attr_list, split_attrs_filter_head, GINT_TO_POINTER (index));
	if (delete)
		pango_attr_list_unref (delete);
	if (t1->extra_attr_list) {
		delete = pango_attr_list_filter (t1->extra_attr_list, split_attrs_filter_head, GINT_TO_POINTER (index));
		if (delete)
			pango_attr_list_unref (delete);
	}
	delete = pango_attr_list_filter (t2->attr_list, split_attrs_filter_tail, GINT_TO_POINTER (index));
	if (delete)
		pango_attr_list_unref (delete);
	if (t2->extra_attr_list) {
		delete = pango_attr_list_filter (t2->extra_attr_list, split_attrs_filter_tail, GINT_TO_POINTER (index));
		if (delete)
			pango_attr_list_unref (delete);
	}
}

static void
split_links (HTMLText *t1, HTMLText *t2, gint offset, gint index)
{
	GSList *l, *prev = NULL;

	for (l = t1->links; l; l = l->next) {
		Link *link = (Link *) l->data;

		if (link->start_offset < offset) {
			if (link->end_offset > offset) {
				link->end_offset = offset;
				link->end_index = index;
			}

			if (prev) {
				prev->next = NULL;
				free_links (t1->links);
			}
			t1->links = l;
			break;
		}
		prev = l;

		if (!l->next) {
			free_links (t1->links);
			t1->links = NULL;
			break;
		}
	}

	prev = NULL;
	for (l = t2->links; l; l = l->next) {
		Link *link = (Link *) l->data;

		if (link->start_offset < offset) {
			if (link->end_offset > offset) {
				link->start_offset = offset;
				link->start_index = index;
				prev = l;
				l = l->next;
			}
			if (prev) {
				prev->next = NULL;
				free_links (l);
			} else {
				free_links (t2->links);
				t2->links = NULL;
			}
			break;
		}
		prev = l;
	}

	for (l = t2->links; l; l = l->next) {
		Link *link = (Link *) l->data;

		link->start_offset -= offset;
		link->start_index -= index;
		link->end_offset -= offset;
		link->end_index -= index;
	}
}

static void
object_split (HTMLObject *self, HTMLEngine *e, HTMLObject *child, gint offset, gint level, GList **left, GList **right)
{
	HTMLObject *dup, *prev;
	HTMLText *t1, *t2;
	gchar *tt;
	gint split_index;

	g_assert (self->parent);

	html_clue_remove_text_slaves (HTML_CLUE (self->parent));

	t1              = HTML_TEXT (self);
	dup             = html_object_dup (self);
	tt              = t1->text;
	split_index     = html_text_get_index (t1, offset);
	t1->text        = g_strndup (tt, split_index);
	t1->text_len    = offset;
	t1->text_bytes  = split_index;
	g_free (tt);
	html_text_convert_nbsp (t1, TRUE);

	t2              = HTML_TEXT (dup);
	tt              = t2->text;
	t2->text        = html_text_get_text (t2, offset);
	t2->text_len   -= offset;
	t2->text_bytes -= split_index;
	split_attrs (t1, t2, split_index);
	split_links (t1, t2, offset, split_index);
	if (!html_text_convert_nbsp (t2, FALSE))
		t2->text = g_strdup (t2->text);
	g_free (tt);

	html_clue_append_after (HTML_CLUE (self->parent), dup, self);

	prev = self->prev;
	if (t1->text_len == 0 && prev && html_object_merge (prev, self, e, NULL, NULL, NULL))
		self = prev;

	if (t2->text_len == 0 && dup->next)
		html_object_merge (dup, dup->next, e, NULL, NULL, NULL);

	/* printf ("--- before split offset %d dup len %d\n", offset, HTML_TEXT (dup)->text_len);
	   debug_spell_errors (HTML_TEXT (self)->spell_errors); */

	HTML_TEXT (self)->spell_errors = remove_spell_errors (HTML_TEXT (self)->spell_errors,
							      offset, HTML_TEXT (dup)->text_len);
	HTML_TEXT (dup)->spell_errors  = remove_spell_errors (HTML_TEXT (dup)->spell_errors,
							      0, HTML_TEXT (self)->text_len);
	move_spell_errors   (HTML_TEXT (dup)->spell_errors, 0, - HTML_TEXT (self)->text_len);

	/* printf ("--- after split\n");
	   printf ("left\n");
	   debug_spell_errors (HTML_TEXT (self)->spell_errors);
	   printf ("right\n");
	   debug_spell_errors (HTML_TEXT (dup)->spell_errors);
	   printf ("---\n");
	*/

	*left  = g_list_prepend (*left, self);
	*right = g_list_prepend (*right, dup);

	html_object_change_set (self, HTML_CHANGE_ALL_CALC);
	html_object_change_set (dup,  HTML_CHANGE_ALL_CALC);

	pango_info_destroy (HTML_TEXT (self));

	level--;
	if (level)
		html_object_split (self->parent, e, dup, 0, level, left, right);
}

static gboolean
html_text_real_calc_size (HTMLObject *self, HTMLPainter *painter, GList **changed_objs)
{
	self->width = 0;
	html_object_calc_preferred_width (self, painter);

	return FALSE;
}

static const gchar *
html_utf8_strnchr (const gchar *s, gchar c, gint len, gint *offset)
{
	const gchar *res = NULL;

	*offset = 0;
	while (s && *s && *offset < len) {
		if (*s == c) {
			res = s;
			break;
		}
		s = g_utf8_next_char (s);
		(*offset) ++;
	}

	return res;
}

gint
html_text_text_line_length (const gchar *text, gint *line_offset, guint len, gint *tabs)
{
	const gchar *tab, *found_tab;
	gint cl, l, skip, sum_skip;

	/* printf ("lo: %d len: %d t: '%s'\n", line_offset, len, text); */
	if (tabs)
		*tabs = 0;
	l = 0;
	sum_skip = skip = 0;
	tab = text;
	while (tab && (found_tab = html_utf8_strnchr (tab, '\t', len - l, &cl)) && l < len) {
		l   += cl;
		if (l >= len)
			break;
		if (*line_offset != -1) {
			*line_offset  += cl;
			skip = 8 - (*line_offset % 8);
		}
		tab  = found_tab + 1;

		*line_offset  += skip;
		if (*line_offset != -1)
			sum_skip += skip - 1;
		l ++;
		if (tabs)
			(*tabs) ++;
	}

	if (*line_offset != -1)
		(*line_offset) += len - l;
	/* printf ("ll: %d\n", len + sum_skip); */

	return len + sum_skip;
}

static guint
get_line_length (HTMLObject *self, HTMLPainter *p, gint line_offset)
{
	return html_clueflow_tabs (HTML_CLUEFLOW (self->parent), p)
		? html_text_text_line_length (HTML_TEXT (self)->text, &line_offset, HTML_TEXT (self)->text_len, NULL)
		: HTML_TEXT (self)->text_len;
}

gint
html_text_get_line_offset (HTMLText *text, HTMLPainter *painter, gint offset)
{
	gint line_offset = -1;

	if (html_clueflow_tabs (HTML_CLUEFLOW (HTML_OBJECT (text)->parent), painter)) {
		line_offset = html_clueflow_get_line_offset (HTML_CLUEFLOW (HTML_OBJECT (text)->parent), 
							     painter, HTML_OBJECT (text));
		if (offset) {
			gchar *s = text->text;

			while (offset > 0 && s && *s) {
				if (*s == '\t')
					line_offset += 8 - (line_offset % 8);
				else
					line_offset ++;
				s = g_utf8_next_char (s);
				offset --;
			}
		}
	}

	return line_offset;
}

gint
html_text_get_item_index (HTMLText *text, HTMLPainter *painter, gint offset, gint *item_offset)
{
	HTMLTextPangoInfo *pi = html_text_get_pango_info (text, painter);
	gint idx = 0;

	while (idx < pi->n - 1 && offset >= pi->entries [idx].item->num_chars) {
		offset -= pi->entries [idx].item->num_chars;
		idx ++;
	}

	*item_offset = offset;

	return idx;
}

static void
update_asc_dsc (HTMLPainter *painter, PangoItem *item, gint *asc, gint *dsc)
{
	PangoFontMetrics *pfm;

	if (!HTML_IS_GDK_PAINTER (painter) && !HTML_IS_PLAIN_PAINTER (painter))
		return;

	pfm = pango_font_get_metrics (item->analysis.font, item->analysis.language);
	if (asc)
		*asc = MAX (*asc, PANGO_PIXELS (pango_font_metrics_get_ascent (pfm)));
	if (dsc)
		*dsc = MAX (*dsc, PANGO_PIXELS (pango_font_metrics_get_descent (pfm)));
	pango_font_metrics_unref (pfm);
}

static void
html_text_get_attr_list_list (PangoAttrList *get_attrs, PangoAttrList *attr_list, gint start_index, gint end_index)
{
	PangoAttrIterator *iter = pango_attr_list_get_iterator (attr_list);

	if (iter) {
		do {
			gint begin, end;

			pango_attr_iterator_range (iter, &begin, &end);

			if (MAX (begin, start_index) < MIN (end, end_index)) {
				GSList *c, *l = pango_attr_iterator_get_attrs (iter);

				for (c = l; c; c = c->next) {
					PangoAttribute *attr = (PangoAttribute *) c->data;

					if (attr->start_index < start_index)
						attr->start_index = 0;
					else
						attr->start_index -= start_index;

					if (attr->end_index > end_index)
						attr->end_index = end_index - start_index;
					else
						attr->end_index -= start_index;

					c->data = NULL;
					pango_attr_list_insert (get_attrs, attr);
				}
				g_slist_free (l);
			}
		} while (pango_attr_iterator_next (iter));
	}
}

PangoAttrList *
html_text_get_attr_list (HTMLText *text, gint start_index, gint end_index)
{
	PangoAttrList *attrs = pango_attr_list_new ();

	html_text_get_attr_list_list (attrs, text->attr_list, start_index, end_index);
	if (text->extra_attr_list)
		html_text_get_attr_list_list (attrs, text->extra_attr_list, start_index, end_index);

	return attrs;
}

void
html_text_calc_text_size (HTMLText *t, HTMLPainter *painter,
			  gint start_byte_offset,
			  guint len, HTMLTextPangoInfo *pi, GList *glyphs, gint *line_offset,
			  GtkHTMLFontStyle font_style,
			  HTMLFontFace *face,
			  gint *width, gint *asc, gint *dsc)
{
		PangoAttrList *attrs = NULL;
		char *text = t->text + start_byte_offset;

		if (HTML_IS_PRINTER (painter)) {
			HTMLClueFlow *flow = NULL;
			HTMLEngine *e = NULL;

			attrs = html_text_get_attr_list (t, start_byte_offset, start_byte_offset + (g_utf8_offset_to_pointer (text, len) - text));

			if (painter->widget && GTK_IS_HTML (painter->widget))
				e = GTK_HTML (painter->widget)->engine;

			if (HTML_OBJECT (t)->parent && HTML_IS_CLUEFLOW (HTML_OBJECT (t)->parent))
				flow = HTML_CLUEFLOW (HTML_OBJECT (t)->parent);

			if (flow && e)
				html_text_change_attrs (attrs, html_clueflow_get_default_font_style (flow), GTK_HTML (painter->widget)->engine, 0, t->text_bytes, TRUE);
		}
		
		html_painter_calc_text_size (painter, text, len, pi, attrs, glyphs,
					     start_byte_offset, line_offset, font_style, face, width, asc, dsc);

		if (attrs)
			pango_attr_list_unref (attrs);
}

gint
html_text_calc_part_width (HTMLText *text, HTMLPainter *painter, char *start, gint offset, gint len, gint *asc, gint *dsc)
{
	gint idx, width = 0, line_offset;

	g_return_val_if_fail (offset >= 0, 0);
	g_return_val_if_fail (offset + len <= text->text_len, 0);

	if (asc)
		*asc = html_painter_get_space_asc (painter, html_text_get_font_style (text), text->face);
	if (dsc)
		*dsc = html_painter_get_space_dsc (painter, html_text_get_font_style (text), text->face);

	if (text->text_len == 0 || len == 0)
		return 0;

	line_offset = html_text_get_line_offset (text, painter, offset);

	if (start == NULL)
		start = html_text_get_text (text, offset);

	if (HTML_IS_GDK_PAINTER (painter) || HTML_IS_PLAIN_PAINTER (painter)) {
		HTMLTextPangoInfo *pi;
		PangoLanguage *language = NULL;
		PangoFont *font = NULL;
		gchar *s = start;

		pi = html_text_get_pango_info (text, painter);

		idx = html_text_get_item_index (text, painter, offset, &offset);
		if (asc || dsc) {
			update_asc_dsc (painter, pi->entries [idx].item, asc, dsc);
			font = pi->entries [idx].item->analysis.font;
			language = pi->entries [idx].item->analysis.language;
		}
		while (len > 0) {
			if (*s == '\t') {
				gint skip = 8 - (line_offset % 8);
				width += skip*pi->entries [idx].widths [offset];
				line_offset += skip;
			} else {
				width += pi->entries [idx].widths [offset];
				line_offset ++;
			}
			len --;
			if (offset >= pi->entries [idx].item->num_chars - 1) {
				idx ++;
				offset = 0;
				if (len > 0 && (asc || dsc) && (pi->entries [idx].item->analysis.font != font || pi->entries [idx].item->analysis.language != language)) {
					update_asc_dsc (painter, pi->entries [idx].item, asc, dsc);
				}
			} else
				offset ++;
			s = g_utf8_next_char (s);
		}
		width = PANGO_PIXELS (width);
	} else {
		html_text_calc_text_size (text, painter, start - text->text, len, NULL, NULL, &line_offset,
					  html_text_get_font_style (text), text->face, &width, asc, dsc);
	}

	return width;
}

static gint
calc_preferred_width (HTMLObject *self,
		      HTMLPainter *painter)
{
	HTMLText *text;
	gint width;

	text = HTML_TEXT (self);

	width = html_text_calc_part_width (text, painter, text->text, 0, text->text_len, &self->ascent, &self->descent);
	self->y = self->ascent;
	if (html_clueflow_tabs (HTML_CLUEFLOW (self->parent), painter)) {
		gint line_offset;
		gint tabs;

		line_offset = html_text_get_line_offset (text, painter, 0);
		width += (html_text_text_line_length (text->text, &line_offset, text->text_len, &tabs) - text->text_len)*
			html_painter_get_space_width (painter, html_text_get_font_style (text), text->face);
	}

	return MAX (1, width);
}

static void
remove_text_slaves (HTMLObject *self)
{
	HTMLObject *next_obj;

	/* Remove existing slaves */
	next_obj = self->next;
	while (next_obj != NULL
	       && (HTML_OBJECT_TYPE (next_obj) == HTML_TYPE_TEXTSLAVE)) {
		self->next = next_obj->next;
		html_clue_remove (HTML_CLUE (next_obj->parent), next_obj);
		html_object_destroy (next_obj);
		next_obj = self->next;
	}
}

static HTMLFitType
ht_fit_line (HTMLObject *o,
	     HTMLPainter *painter,
	     gboolean startOfLine,
	     gboolean firstRun,
	     gboolean next_to_floating,
	     gint widthLeft) 
{
	HTMLText *text; 
	HTMLObject *text_slave;

	text = HTML_TEXT (o);

	remove_text_slaves (o);

	/* Turn all text over to our slaves */
	text_slave = html_text_slave_new (text, 0, HTML_TEXT (text)->text_len);
	html_clue_append_after (HTML_CLUE (o->parent), text_slave, o);

	return HTML_FIT_COMPLETE;
}

static gint
min_word_width_calc_tabs (HTMLText *text, HTMLPainter *p, gint idx, gint *len)
{
	gchar *str, *end;
	gint rv = 0, line_offset, wt, wl, i;
	gint epos;
	gboolean tab = FALSE;
	
	if (!html_clueflow_tabs (HTML_CLUEFLOW (HTML_OBJECT (text)->parent), p))
		return 0;

	/* printf ("tabs %d\n", idx); */

	str = text->text;
	i = idx;
	while (i > 0 && *str) {
		if (*str == ' ')
			i--;

		str = g_utf8_next_char (str);
	}

	if (!*str)
		return 0;

	epos = 0;
	end = str;
	while (*end && *end != ' ') {
		tab |= *end == '\t';

		end = g_utf8_next_char (end);
		epos++;
	}
	

	if (tab) {
		line_offset = 0;
		
		if (idx == 0) {
			HTMLObject *prev;
			
			prev = html_object_prev_not_slave (HTML_OBJECT (text));
			if (prev && html_object_is_text (prev) /* FIXME-words && HTML_TEXT (prev)->words > 0 */) {
				min_word_width_calc_tabs (HTML_TEXT (prev), p, /* FIXME-words HTML_TEXT (prev)->words - 1 */ HTML_TEXT (prev)->text_len - 1, &line_offset);
				/* printf ("lo: %d\n", line_offset); */
			}
		}

		wl = html_text_text_line_length (str, &line_offset, epos, &wt);
	} else {
		wl = epos;
	}
	
	rv = wl - epos;
		
	if (len)
		*len = wl;

	/* printf ("tabs delta %d\n", rv); */
	return rv;
}

gint
html_text_pango_info_get_index (HTMLTextPangoInfo *pi, gint byte_offset, gint idx)
{
	while (idx < pi->n && pi->entries [idx].item->offset + pi->entries [idx].item->length <= byte_offset)
		idx ++;

	return idx;
}


static void
html_text_add_cite_color (PangoAttrList *attrs, HTMLText *text, HTMLClueFlow *flow, HTMLEngine *e)
{
	HTMLColor *cite_color = html_colorset_get_color (e->settings->color_set, HTMLCiteColor);

	if (cite_color && flow->levels->len > 0 && flow->levels->data[0] == HTML_LIST_TYPE_BLOCKQUOTE_CITE) {
		PangoAttribute *attr;

		attr = pango_attr_foreground_new (cite_color->color.red, cite_color->color.green, cite_color->color.blue);
		attr->start_index = 0;
		attr->end_index = text->text_bytes;
		pango_attr_list_change (attrs, attr);
	}
}

void
html_text_remove_unwanted_line_breaks (char *s, int len, PangoLogAttr *attrs)
{
	int i;
	gunichar last_uc = 0;

	for (i = 0; i < len; i ++) {
		gunichar uc = g_utf8_get_char (s);

		if (attrs [i].is_line_break) {
			if (last_uc == '.' || last_uc == '/' ||
			    last_uc == '-' || last_uc == '$' ||
			    last_uc == '+' || last_uc == '?' ||
			    last_uc == ')' ||
			    last_uc == '}' ||
			    last_uc == ']' ||
			    last_uc == '>')
				attrs [i].is_line_break = 0;
			else if ((uc == '(' ||
				  uc == '{' ||
				  uc == '[' ||
				  uc == '<'
				  )
				 && i > 0 && !attrs [i - 1].is_white)
				attrs [i].is_line_break = 0;
		}
		s = g_utf8_next_char (s);
		last_uc = uc;
	}
}

HTMLTextPangoInfo *
html_text_get_pango_info (HTMLText *text, HTMLPainter *painter)
{
	/*if (!HTML_IS_GDK_PAINTER (painter) && !HTML_IS_PLAIN_PAINTER (painter))
	  return NULL; */
	if (HTML_OBJECT (text)->change & HTML_CHANGE_RECALC_PI) {
		pango_info_destroy (text);
		HTML_OBJECT (text)->change &= ~HTML_CHANGE_RECALC_PI;
	}
	if (!text->pi) {
		PangoContext *pc = gtk_widget_get_pango_context (painter->widget);
		GList *items, *cur;
		PangoAttrList *attrs;
		PangoAttribute *attr;
		HTMLClueFlow *flow = NULL;
		HTMLEngine *e = NULL;
		gchar *translated, *heap = NULL;
		int i, offset;

		if (text->text_bytes > HTML_ALLOCA_MAX)
			heap = translated = g_malloc (text->text_bytes);
		else 
			translated = alloca (text->text_bytes);

		html_replace_tabs (text->text, translated, text->text_bytes);
		attrs = pango_attr_list_new ();

		if (HTML_OBJECT (text)->parent && HTML_IS_CLUEFLOW (HTML_OBJECT (text)->parent))
			flow = HTML_CLUEFLOW (HTML_OBJECT (text)->parent);
		
		if (painter->widget && GTK_IS_HTML (painter->widget))
			e = GTK_HTML (painter->widget)->engine;

		if (flow && e)
			html_text_add_cite_color (attrs, text, flow, e);

		if (HTML_IS_PLAIN_PAINTER (painter)) {
			attr = pango_attr_family_new (painter->font_manager.fixed.face);
			attr->start_index = 0;
			attr->end_index = text->text_bytes;
			pango_attr_list_insert (attrs, attr);
			if (painter->font_manager.fix_size != painter->font_manager.var_size) {
				attr = pango_attr_size_new (painter->font_manager.fix_size);
				attr->start_index = 0;
				attr->end_index = text->text_bytes;
				pango_attr_list_insert (attrs, attr);
			}
		} else
			pango_attr_list_splice (attrs, text->attr_list, 0, 0);

		if (text->extra_attr_list)
			pango_attr_list_splice (attrs, text->extra_attr_list, 0, 0);
		if (!HTML_IS_PLAIN_PAINTER (painter)) {
			if (flow && e)
				html_text_change_attrs (attrs, html_clueflow_get_default_font_style (flow), GTK_HTML (painter->widget)->engine, 0, text->text_bytes, TRUE);
		}

		if (text->links && e) {
			HTMLColor *link_color = html_colorset_get_color (e->settings->color_set, HTMLLinkColor);
			GSList *l;

			for (l = text->links; l; l = l->next) {
				PangoAttribute *attr;
				Link *link;

				link = (Link *) l->data;
				attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
				attr->start_index = link->start_index;
				attr->end_index = link->end_index;
				pango_attr_list_change (attrs, attr);

				attr = pango_attr_foreground_new (link_color->color.red, link_color->color.green, link_color->color.blue);
				attr->start_index = link->start_index;
				attr->end_index = link->end_index;
				pango_attr_list_change (attrs, attr);
			}
		}

		if (e && text->select_length) {
			gchar *end;
			gchar *start;
			GdkColor fg = html_colorset_get_color_allocated
				(e->settings->color_set, painter,
				 painter->focus ? HTMLHighlightTextColor : HTMLHighlightTextNFColor)->color;
			GdkColor bg = html_colorset_get_color_allocated
				(e->settings->color_set, painter,
				 painter->focus ? HTMLHighlightColor : HTMLHighlightNFColor)->color;
			
			start = html_text_get_text (text,  text->select_start);
			end = g_utf8_offset_to_pointer (start, text->select_length);
			
			attr = pango_attr_background_new (bg.red, bg.green, bg.blue);
			attr->start_index = start - text->text;
			attr->end_index = end - text->text;
			pango_attr_list_change (attrs, attr);
			
			attr = pango_attr_foreground_new (fg.red, fg.green, fg.blue);
			attr->start_index = start - text->text;
			attr->end_index = end - text->text;
			pango_attr_list_change (attrs, attr);
		}

		items = pango_itemize (pc, translated, 0, text->text_bytes, attrs, NULL);
		pango_attr_list_unref (attrs);

		text->pi = html_text_pango_info_new (g_list_length (items));

		for (i = 0, cur = items; i < text->pi->n; i ++, cur = cur->next)
			text->pi->entries [i].item = (PangoItem *) cur->data;

		offset = 0;
		text->pi->attrs = g_new (PangoLogAttr, text->text_len + 1);
		for (i = 0; i < text->pi->n; i ++) {
			PangoItem tmp_item;
			int start_i, start_offset;

			start_i = i;
			start_offset = offset;
			offset += text->pi->entries [i].item->num_chars;
			tmp_item = *text->pi->entries [i].item;
			while (i < text->pi->n - 1) {
				if (tmp_item.analysis.lang_engine == text->pi->entries [i + 1].item->analysis.lang_engine) {
					tmp_item.length += text->pi->entries [i + 1].item->length;
					tmp_item.num_chars += text->pi->entries [i + 1].item->num_chars;
					offset += text->pi->entries [i + 1].item->num_chars;
					i ++;
				} else
					break;
			}

			pango_break (translated + tmp_item.offset, tmp_item.length, &tmp_item.analysis, text->pi->attrs + start_offset, tmp_item.num_chars + 1);
		}

		if (text->pi && text->pi->attrs)
			html_text_remove_unwanted_line_breaks (text->text, text->text_len, text->pi->attrs);

		for (i = 0; i < text->pi->n; i ++) {
			PangoGlyphString *glyphs;
			PangoItem *item;

			item = text->pi->entries [i].item;

			/* printf ("item pos %d len %d\n", item->offset, item->length); */

			glyphs = pango_glyph_string_new ();
			text->pi->entries [i].widths = g_new (PangoGlyphUnit, item->num_chars);
			pango_shape (translated + item->offset, item->length, &item->analysis, glyphs);
			pango_glyph_string_get_logical_widths (glyphs, translated + item->offset, item->length, item->analysis.level, text->pi->entries [i].widths);
			pango_glyph_string_free (glyphs);
		}
		g_free (heap);
		g_list_free (items);
	}
	return text->pi;
}

gboolean
html_text_pi_backward (HTMLTextPangoInfo *pi, gint *ii, gint *io)
{
	if (*io <= 0) {
		if (*ii <= 0)
			return FALSE;
		(*ii) --;
		*io = pi->entries [*ii].item->num_chars - 1;
	} else
		(*io) --;

	return TRUE;
}

gboolean
html_text_pi_forward (HTMLTextPangoInfo *pi, gint *ii, gint *io)
{
	if (*io >= pi->entries [*ii].item->num_chars - 1) {
		if (*ii >= pi->n -1)
			return FALSE;
		(*ii) ++;
		*io = 0;
	} else
		(*io) ++;

	return TRUE;
}

gint
html_text_tail_white_space (HTMLText *text, HTMLPainter *painter, gint offset, gint ii, gint io, gint *white_len, gint line_offset, gchar *s)
{
	HTMLTextPangoInfo *pi = html_text_get_pango_info (text, painter);
	int wl = 0;
	int ww = 0;
	int current_offset = offset;

	if (html_text_pi_backward (pi, &ii, &io)) {
		s = g_utf8_prev_char (s);
		current_offset --;
		if (pi->attrs [current_offset].is_white) {
			if (HTML_IS_GDK_PAINTER (painter) || HTML_IS_PLAIN_PAINTER (painter)) {
				if (*s == '\t' && offset > 1) {
					gint skip = 8, co = offset - 1;

					do {
						s = g_utf8_prev_char (s);
						co --;
						if (*s != '\t')
							skip --;
					} while (s && co > 0 && *s != '\t');

					ww += skip*(PANGO_PIXELS (pi->entries [ii].widths [io]));
				} else {
					ww += PANGO_PIXELS (pi->entries [ii].widths [io]);
				}
			}
			wl ++;
		}
	}

	if (!HTML_IS_GDK_PAINTER (painter) && !HTML_IS_PLAIN_PAINTER (painter) && wl) {
		html_text_calc_text_size (text, painter, html_text_get_text (text, offset - wl) - text->text,
					  wl, NULL, NULL, &line_offset, html_text_get_font_style (text), text->face,
					  &ww, NULL, NULL);
	}

	if (white_len)
		*white_len = wl;

	return ww;
}

static void
update_mw (HTMLText *text, HTMLPainter *painter, gint offset, gint *last_offset, gint *ww, gint *mw, gint ii, gint io, gchar *s, gint line_offset) {
	if (!HTML_IS_GDK_PAINTER (painter) && !HTML_IS_PLAIN_PAINTER (painter)) {
		gint w;
		html_text_calc_text_size (text, painter, html_text_get_text (text, *last_offset) - text->text,
					  offset - *last_offset, NULL, NULL, NULL, html_text_get_font_style (text), text->face,
					  &w, NULL, NULL);
		*ww += w;
	}
	*ww -= html_text_tail_white_space (text, painter, offset, ii, io, NULL, line_offset, s);
	if (*ww > *mw)
		*mw = *ww;
	*ww = 0;

	*last_offset = offset;
}

gboolean
html_text_is_line_break (PangoLogAttr attr)
{
	return attr.is_line_break;
}

static gint
calc_min_width (HTMLObject *self, HTMLPainter *painter)
{
	HTMLText *text = HTML_TEXT (self);
	HTMLTextPangoInfo *pi = html_text_get_pango_info (text, painter);
	gint mw = 0, ww;
	gint ii, io, offset, last_offset, line_offset;
	gchar *s;

	ww = 0;

	last_offset = offset = 0;
	ii = io = 0;
	line_offset = html_text_get_line_offset (text, painter, 0);
	s = text->text;
	while (offset < text->text_len) {
		if (offset > 0 && html_text_is_line_break (pi->attrs [offset]))
			update_mw (text, painter, offset, &last_offset, &ww, &mw, ii, io, s, line_offset);

		if (*s == '\t') {
			gint skip = 8 - (line_offset % 8);
			if (HTML_IS_GDK_PAINTER (painter) || HTML_IS_PLAIN_PAINTER (painter))
				ww += skip*(PANGO_PIXELS (pi->entries [ii].widths [io]));
			line_offset += skip;
		} else {
			if (HTML_IS_GDK_PAINTER (painter) || HTML_IS_PLAIN_PAINTER (painter))
				ww += PANGO_PIXELS (pi->entries [ii].widths [io]);
			line_offset ++;
		}

		s = g_utf8_next_char (s);
		offset ++;

		html_text_pi_forward (pi, &ii, &io);
	}

	
	if (!HTML_IS_GDK_PAINTER (painter) && !HTML_IS_PLAIN_PAINTER (painter))
		html_text_calc_text_size (text, painter, html_text_get_text (text, last_offset) - text->text,
					  offset - last_offset, NULL, NULL, NULL, html_text_get_font_style (text), text->face,
					  &ww, NULL, NULL);

	if (ww > mw)
		mw = ww;

	return MAX (1, mw);
}

static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
}

static gboolean
accepts_cursor (HTMLObject *object)
{
	return TRUE;
}

static gboolean
save_open_attrs (HTMLEngineSaveState *state, GSList *attrs)
{
	gboolean rv = TRUE;

	for (; attrs; attrs = attrs->next) {
		PangoAttribute *attr = (PangoAttribute *) attrs->data;
		HTMLEngine *e = state->engine;
		gchar *tag = NULL;
		gboolean free_tag = FALSE;

		switch (attr->klass->type) {
		case PANGO_ATTR_WEIGHT:
			tag = "<B>";
			break;
		case PANGO_ATTR_STYLE:
			tag = "<I>";
			break;
		case PANGO_ATTR_UNDERLINE:
			tag = "<U>";
			break;
		case PANGO_ATTR_STRIKETHROUGH:
			tag = "<S>";
			break;
		case PANGO_ATTR_SIZE:
			if (attr->klass == &html_pango_attr_font_size_klass) {
				HTMLPangoAttrFontSize *size = (HTMLPangoAttrFontSize *) attr;
				if ((size->style & GTK_HTML_FONT_STYLE_SIZE_MASK) != GTK_HTML_FONT_STYLE_SIZE_3 && (size->style & GTK_HTML_FONT_STYLE_SIZE_MASK) != 0) {
					tag = g_strdup_printf ("<FONT SIZE=\"%d\">", size->style & GTK_HTML_FONT_STYLE_SIZE_MASK);
					free_tag = TRUE;
				}
			}
			break;
		case PANGO_ATTR_FAMILY: {
			PangoAttrString *family_attr = (PangoAttrString *) attr;

			if (!strcasecmp (e->painter->font_manager.fixed.face
					? e->painter->font_manager.fixed.face : "Monospace",
					family_attr->value))
				tag = "<TT>";
		}
		break;
		case PANGO_ATTR_FOREGROUND: {
			PangoAttrColor *color = (PangoAttrColor *) attr;
			tag = g_strdup_printf ("<FONT COLOR=\"#%02x%02x%02x\">",
					       (color->color.red >> 8) & 0xff, (color->color.green >> 8) & 0xff, (color->color.blue >> 8) & 0xff);
			free_tag = TRUE;
		}
			break;
		default:
			break;
		}

		if (tag) {
			if (!html_engine_save_output_string (state, "%s", tag))
				rv = FALSE;
			if (free_tag)
				g_free (tag);
			if (!rv)
				break;
		}
	}

	return TRUE;
}

static gboolean
save_close_attrs (HTMLEngineSaveState *state, GSList *attrs)
{
	for (; attrs; attrs = attrs->next) {
		PangoAttribute *attr = (PangoAttribute *) attrs->data;
		HTMLEngine *e = state->engine;
		gchar *tag = NULL;

		switch (attr->klass->type) {
		case PANGO_ATTR_WEIGHT:
			tag = "</B>";
			break;
		case PANGO_ATTR_STYLE:
			tag = "</I>";
			break;
		case PANGO_ATTR_UNDERLINE:
			tag = "</U>";
			break;
		case PANGO_ATTR_STRIKETHROUGH:
			tag = "</S>";
			break;
		case PANGO_ATTR_SIZE:
			if (attr->klass == &html_pango_attr_font_size_klass) {
				HTMLPangoAttrFontSize *size = (HTMLPangoAttrFontSize *) attr;
				if ((size->style & GTK_HTML_FONT_STYLE_SIZE_MASK) != GTK_HTML_FONT_STYLE_SIZE_3 && (size->style & GTK_HTML_FONT_STYLE_SIZE_MASK) != 0)
					tag = "</FONT>";
			}
			break;
		case PANGO_ATTR_FOREGROUND:
			tag = "</FONT>";
			break;
		case PANGO_ATTR_FAMILY: {
			PangoAttrString *family_attr = (PangoAttrString *) attr;

			if (!strcasecmp (e->painter->font_manager.fixed.face
					? e->painter->font_manager.fixed.face : "Monospace",
					family_attr->value))
				tag = "</TT>";
		}
		break;
		default:
			break;
		}

		if (tag)
			if (!html_engine_save_output_string (state, "%s", tag))
				return FALSE;
	}

	return TRUE;
}

static gboolean
save_text_part (HTMLText *text, HTMLEngineSaveState *state, guint start_index, guint end_index)
{
	gchar *str;
	gint len;
	gboolean rv;

	str = g_strndup (text->text + start_index, end_index - start_index);
	len = g_utf8_pointer_to_offset (text->text + start_index, text->text + end_index);

	rv = html_engine_save_encode (state, str, len);
	g_free (str);
	return rv;
}

static gboolean
save_link_open (Link *link, HTMLEngineSaveState *state)
{
	return html_engine_save_output_string (state, "<A HREF=\"%s\">", link->url);
}

static gboolean
save_link_close (Link *link, HTMLEngineSaveState *state)
{
	return html_engine_save_output_string (state, "%s", "</A>");
}

static gboolean
save_text (HTMLText *text, HTMLEngineSaveState *state, guint start_index, guint end_index, GSList **l, gboolean *link_started)
{
	if (*l) {
		Link *link;

		link = (Link *) (*l)->data;

		while (*l && ((!*link_started && start_index <= link->start_index && link->start_index < end_index)
			      || (*link_started && link->end_index <= end_index))) {
			if (!*link_started && start_index <= link->start_index && link->start_index < end_index) {
				if (!save_text_part (text, state, start_index, link->start_index))
					return FALSE;
				*link_started = TRUE;
				save_link_open (link, state);
				start_index = link->start_index;
			}
			if (*link_started && link->end_index <= end_index) {
				if (!save_text_part (text, state, start_index, link->end_index))
					return FALSE;
				save_link_close (link, state);
				*link_started = FALSE;
				(*l) = (*l)->next;
				start_index = link->end_index;
				if (*l)
					link = (Link *) (*l)->data;
			}
		}

	}

	if (start_index < end_index)
		return save_text_part (text, state, start_index, end_index);

	return TRUE;
}

static gboolean
save (HTMLObject *self, HTMLEngineSaveState *state)
{
	HTMLText *text = HTML_TEXT (self);
	PangoAttrIterator *iter = pango_attr_list_get_iterator (text->attr_list);

	if (iter) {
		GSList *l, *links = g_slist_reverse (g_slist_copy (text->links));
		gboolean link_started = FALSE;

		l = links;

		do {
			GSList *attrs;
			guint start_index, end_index;

			attrs = pango_attr_iterator_get_attrs (iter);
			pango_attr_iterator_range (iter, &start_index, &end_index);
			if (end_index > text->text_bytes)
				end_index = text->text_bytes;

			if (attrs)
				save_open_attrs (state, attrs);
			save_text (text, state, start_index, end_index, &l, &link_started);
			if (attrs) {
				attrs = g_slist_reverse (attrs);
				save_close_attrs (state, attrs);
				html_text_free_attrs (attrs);
			}
		} while (pango_attr_iterator_next (iter));
		g_slist_free (links);
	}

	return TRUE;
}

static gboolean
save_plain (HTMLObject *self,
	    HTMLEngineSaveState *state,
	    gint requested_width)
{
	HTMLText *text;

	text = HTML_TEXT (self);

	return html_engine_save_output_string (state, "%s", text->text);
}

static guint
get_length (HTMLObject *self)
{
	return HTML_TEXT (self)->text_len;
}

/* #define DEBUG_NBSP */

struct TmpDeltaRecord
{
	int index;		/* Byte index within original string  */
	int delta;		/* New delta (character at index was modified,
				 * new delta applies to characters afterwards)
				 */
};

/* Called when current character is not white space or at end of string */
static gboolean
check_last_white (gint white_space, gunichar last_white, gint *delta_out)
{
	if (white_space > 0 && last_white == ENTITY_NBSP) {
		(*delta_out) --; /* &nbsp; => &sp; is one byte shorter in UTF-8 */
		return TRUE;
	}

	return FALSE;
}

/* Called when current character is white space */
static gboolean
check_prev_white (gint white_space, gunichar last_white, gint *delta_out)
{
	if (white_space > 0 && last_white == ' ') {
		(*delta_out) ++; /* &sp; => &nbsp; is one byte longer in UTF-8 */
		return TRUE;
	}

	return FALSE;
}

static GSList *
add_change (GSList *list, int index, int delta)
{
	struct TmpDeltaRecord *rec = g_new (struct TmpDeltaRecord, 1);

	rec->index = index;
	rec->delta = delta;

	return g_slist_prepend (list, rec);
}

/* This function does a pre-scan for the transformation in convert_nbsp,
 * which converts a sequence of N white space characters (&sp; or &nbsp;)
 * into N-1 &nbsp and 1 &sp;.
 *
 * delta_out: total change in byte length of string
 * changes_out: location to store series of records for each change in offset
 *              between the original string and the new string.
 * returns: %TRUE if any records were stored in changes_out
 */
static gboolean
is_convert_nbsp_needed (const gchar *s, gint *delta_out, GSList **changes_out)
{
	gunichar uc, last_white = 0;
	gboolean change;
	gint white_space;
	const gchar *p, *last_p;

	*delta_out = 0;

	last_p = NULL;		/* Quiet GCC */
	white_space = 0;
	for (p = s; *p; p = g_utf8_next_char (p)) {
		uc = g_utf8_get_char (p);
		
		if (uc == ENTITY_NBSP || uc == ' ') {
			change = check_prev_white (white_space, last_white, delta_out);
			white_space ++;
			last_white = uc;
		} else {
			change = check_last_white (white_space, last_white, delta_out);
			white_space = 0;
		}
		if (change)
			*changes_out = add_change (*changes_out, last_p - s, *delta_out);
		last_p = p;
	}

	if (check_last_white (white_space, last_white, delta_out))
		*changes_out = add_change (*changes_out, last_p - s, *delta_out);


	*changes_out = g_slist_reverse (*changes_out);

	return *changes_out != NULL;
}

/* Called when current character is white space */
static void
write_prev_white_space (gint white_space, gchar **fill)
{
	if (white_space > 0) {
#ifdef DEBUG_NBSP
		printf ("&nbsp;");
#endif
		**fill = 0xc2; (*fill) ++;
		**fill = 0xa0; (*fill) ++;
	}
}

/* Called when current character is not white space or at end of string */
static void
write_last_white_space (gint white_space, gchar **fill)
{
	if (white_space > 0) {
#ifdef DEBUG_NBSP
		printf (" ");
#endif
		**fill = ' '; (*fill) ++;
	}
}

/* converts a sequence of N white space characters (&sp; or &nbsp;)
 * into N-1 &nbsp and 1 &sp;.
 */
static void
convert_nbsp (gchar *fill, const gchar *text)
{
	gint white_space;
	gunichar uc;
	const gchar *this_p, *p;

#ifdef DEBUG_NBSP
	printf ("convert_nbsp: %s --> \"", p);
#endif
	p = text;
	white_space = 0;

	while (*p) {
		this_p = p;
		uc = g_utf8_get_char (p);
		p = g_utf8_next_char (p);

		if (uc == ENTITY_NBSP || uc == ' ') {
			write_prev_white_space (white_space, &fill);
			white_space ++;
		} else {
			write_last_white_space (white_space, &fill);
			white_space = 0;
#ifdef DEBUG_NBSP
			printf ("*");
#endif
			strncpy (fill, this_p, p - this_p);
			fill += p - this_p;
		}
	}

	write_last_white_space (white_space, &fill);
	*fill = 0;

#ifdef DEBUG_NBSP
	printf ("\"\n");
#endif
}

static void
update_index_interval (int *start_index, int *end_index, GSList *changes)
{
	GSList *c;
	int index, delta;

	index = delta = 0;

	for (c = changes; c && *start_index > index; c = c->next) {
		struct TmpDeltaRecord *rec = c->data;

		if (*start_index > index && *start_index <= rec->index) {
			(*start_index) += delta;
			break;
		}
		index = rec->index;
		delta = rec->delta;
	}

	if (c == NULL && *start_index > index) {
		(*start_index) += delta;
		(*end_index) += delta;
		return;
	}

	for (; c && *end_index > index; c = c->next) {
		struct TmpDeltaRecord *rec = c->data;

		if (*end_index > index && *end_index <= rec->index) {
			(*start_index) += delta;
			break;
		}
		index = rec->index;
		delta = rec->delta;
	}

	if (c == NULL && *end_index > index)
		(*end_index) += delta;
}

static gboolean
update_attributes_filter (PangoAttribute *attr, gpointer data)
{
	update_index_interval (&attr->start_index, &attr->end_index, (GSList *) data);

	return FALSE;
}

static void
update_attributes (PangoAttrList *attrs, GSList *changes)
{
	pango_attr_list_filter (attrs, update_attributes_filter, changes);
}

static void
update_links (GSList *links, GSList *changes)
{
	GSList *cl;

	for (cl = links; cl; cl = cl->next) {
		Link *link = (Link *) cl->data;
		update_index_interval (&link->start_index, &link->end_index, changes);
	}
}

static void
free_changes (GSList *changes)
{
	GSList *c;

	for (c = changes; c; c = c->next)
		g_free (c->data);
	g_slist_free (changes);
}

gboolean
html_text_convert_nbsp (HTMLText *text, gboolean free_text)
{
	GSList *changes = NULL;
	gint delta;

	if (is_convert_nbsp_needed (text->text, &delta, &changes)) {
		gchar *to_free;

		to_free = text->text;
		text->text = g_malloc (strlen (to_free) + delta + 1);
		text->text_bytes += delta;
		convert_nbsp (text->text, to_free);
		if (free_text)
			g_free (to_free);
		if (changes) {
			if (text->attr_list)
				update_attributes (text->attr_list, changes);
			if (text->extra_attr_list)
				update_attributes (text->extra_attr_list, changes);
			if (text->links)
				update_links (text->links, changes);
			free_changes (changes);
		}
		html_object_change_set (HTML_OBJECT (text), HTML_CHANGE_ALL);
		return TRUE;
	}
	return FALSE;
}

static void 
move_spell_errors (GList *spell_errors, guint offset, gint delta) 
{ 
	SpellError *se; 

	if (!delta)
		return;

	while (spell_errors) { 
		se = (SpellError *) spell_errors->data; 
		if (se->off >= offset) 
			se->off += delta; 
 		spell_errors = spell_errors->next; 
  	} 
} 

static GList *
remove_one (GList *list, GList *link)
{
	spell_error_destroy ((SpellError *) link->data);
	return g_list_remove_link (list, link);
}

static GList *
remove_spell_errors (GList *spell_errors, guint offset, guint len)
{
	SpellError *se; 
	GList *cur, *cnext;

	cur = spell_errors;
	while (cur) { 
		cnext = cur->next;
		se = (SpellError *) cur->data;
		if (se->off < offset) {
			if (se->off + se->len > offset) {
				if (se->off + se->len <= offset + len)
					se->len = offset - se->off;
				else
					se->len -= len;
				if (se->len < 2)
					spell_errors = remove_one (spell_errors, cur);
			}
		} else if (se->off < offset + len) {
			if (se->off + se->len <= offset + len)
				spell_errors = remove_one (spell_errors, cur);
			else {
				se->len -= offset + len - se->off;
				se->off  = offset + len;
				if (se->len < 2)
					spell_errors = remove_one (spell_errors, cur);
			}
		}
 		cur = cnext;
  	} 
	return spell_errors;
}

static HTMLObject *
check_point (HTMLObject *self,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	return NULL;
}

static void
queue_draw (HTMLText *text,
	    HTMLEngine *engine,
	    guint offset,
	    guint len)
{
	HTMLObject *obj;

	for (obj = HTML_OBJECT (text)->next; obj != NULL; obj = obj->next) {
		HTMLTextSlave *slave;

		if (HTML_OBJECT_TYPE (obj) != HTML_TYPE_TEXTSLAVE)
			continue;

		slave = HTML_TEXT_SLAVE (obj);

		if (offset < slave->posStart + slave->posLen
		    && (len == 0 || offset + len >= slave->posStart)) {
			html_engine_queue_draw (engine, obj);
			if (len != 0 && slave->posStart + slave->posLen > offset + len)
				break;
		}
	}
}

/* This is necessary to merge the text-specified font style with that of the
   HTMLClueFlow parent.  */
static GtkHTMLFontStyle
get_font_style (const HTMLText *text)
{
	HTMLObject *parent;
	GtkHTMLFontStyle font_style;

	parent = HTML_OBJECT (text)->parent;

	if (HTML_OBJECT_TYPE (parent) == HTML_TYPE_CLUEFLOW) {
		GtkHTMLFontStyle parent_style;

		parent_style = html_clueflow_get_default_font_style (HTML_CLUEFLOW (parent));
		font_style = gtk_html_font_style_merge (parent_style, text->font_style);
	} else {
		font_style = gtk_html_font_style_merge (GTK_HTML_FONT_STYLE_SIZE_3, text->font_style);
	}

	return font_style;
}

static void
set_font_style (HTMLText *text,
		HTMLEngine *engine,
		GtkHTMLFontStyle style)
{
	if (text->font_style == style)
		return;

	text->font_style = style;

	html_object_change_set (HTML_OBJECT (text), HTML_CHANGE_ALL_CALC);

	if (engine != NULL) {
		html_object_relayout (HTML_OBJECT (text)->parent, engine, HTML_OBJECT (text));
		html_engine_queue_draw (engine, HTML_OBJECT (text));
	}
}

static void
destroy (HTMLObject *obj)
{
	HTMLText *text = HTML_TEXT (obj);
	html_color_unref (text->color);
	html_text_spell_errors_clear (text);
	g_free (text->text);
	g_free (text->face);
	pango_info_destroy (text);
	pango_attr_list_unref (text->attr_list);
	text->attr_list = NULL;
	if (text->extra_attr_list) {
		pango_attr_list_unref (text->extra_attr_list);
		text->extra_attr_list = NULL;
	}
	free_links (text->links);
	text->links = NULL;

	HTML_OBJECT_CLASS (parent_class)->destroy (obj);
}


static gboolean
select_range (HTMLObject *self,
	      HTMLEngine *engine,
	      guint offset,
	      gint length,
	      gboolean queue_draw)
{
	HTMLText *text;
	HTMLObject *p;
	gboolean changed;

	text = HTML_TEXT (self);

	if (length < 0 || length + offset > HTML_TEXT (self)->text_len)
		length = HTML_TEXT (self)->text_len - offset;

	if (offset != text->select_start || length != text->select_length) {
		HTMLObject *slave;
		changed = TRUE;
		html_object_change_set (self, HTML_CHANGE_RECALC_PI);
		slave = self->next;
		while (slave && HTML_IS_TEXT_SLAVE (slave)) {
			html_object_change_set (slave, HTML_CHANGE_RECALC_PI);
			slave = slave->next;
		}
	} else
		changed = FALSE;

	/* printf ("select range %d, %d\n", offset, length); */
	if (queue_draw) {
		for (p = self->next;
		     p != NULL && HTML_OBJECT_TYPE (p) == HTML_TYPE_TEXTSLAVE;
		     p = p->next) {
			HTMLTextSlave *slave;
			gboolean was_selected, is_selected;
			guint max;

			slave = HTML_TEXT_SLAVE (p);

			max = slave->posStart + slave->posLen;

			if (text->select_start + text->select_length > slave->posStart
			    && text->select_start < max)
				was_selected = TRUE;
			else
				was_selected = FALSE;

			if (offset + length > slave->posStart && offset < max)
				is_selected = TRUE;
			else
				is_selected = FALSE;

			if (was_selected && is_selected) {
				gint diff1, diff2;

				diff1 = offset - slave->posStart;
				diff2 = text->select_start - slave->posStart;

				/* printf ("offsets diff 1: %d 2: %d\n", diff1, diff2); */
				if (diff1 != diff2) {
					html_engine_queue_draw (engine, p);
				} else {
					diff1 = offset + length - slave->posStart;
					diff2 = (text->select_start + text->select_length
						 - slave->posStart);

					/* printf ("lens diff 1: %d 2: %d\n", diff1, diff2); */
					if (diff1 != diff2)
						html_engine_queue_draw (engine, p);
				}
			} else {
				if ((! was_selected && is_selected) || (was_selected && ! is_selected))
					html_engine_queue_draw (engine, p);
			}
		}
	}

	text->select_start = offset;
	text->select_length = length;

	if (length == 0)
		self->selected = FALSE;
	else
		self->selected = TRUE;

	return changed;
}

static HTMLObject *
set_link (HTMLObject *self, HTMLColor *color, const gchar *url, const gchar *target)
{
	/* HTMLText *text = HTML_TEXT (self); */

	/* FIXME-link return url ? html_link_text_new_with_len (text->text, text->text_len, text->font_style, color, url, target) : NULL; */
	return NULL;
}

static void
append_selection_string (HTMLObject *self,
			 GString *buffer)
{
	HTMLText *text;
	const gchar *p, *last;

	text = HTML_TEXT (self);
	if (text->select_length == 0)
		return;

	p    = html_text_get_text (text, text->select_start);
	last = g_utf8_offset_to_pointer (p, text->select_length);
	
	/* OPTIMIZED
	last = html_text_get_text (text,
				   text->select_start + text->select_length);
	*/
	html_engine_save_string_append_nonbsp (buffer, p, last - p);

}

static void
get_cursor (HTMLObject *self,
	    HTMLPainter *painter,
	    guint offset,
	    gint *x1, gint *y1,
	    gint *x2, gint *y2)
{
	HTMLObject *slave;
	guint ascent, descent;

	html_object_get_cursor_base (self, painter, offset, x2, y2);

	slave = self->next;
	if (slave == NULL || HTML_OBJECT_TYPE (slave) != HTML_TYPE_TEXTSLAVE) {
		ascent = self->ascent;
		descent = self->descent;
	} else {
		ascent = slave->ascent;
		descent = slave->descent;
	}

	*x1 = *x2;
	*y1 = *y2 - ascent;
	*y2 += descent - 1;
}

static void
get_cursor_base (HTMLObject *self,
		 HTMLPainter *painter,
		 guint offset,
		 gint *x, gint *y)
{
	HTMLObject *obj;

	for (obj = self->next; obj != NULL; obj = obj->next) {
		HTMLTextSlave *slave;

		if (HTML_OBJECT_TYPE (obj) != HTML_TYPE_TEXTSLAVE)
			break;

		slave = HTML_TEXT_SLAVE (obj);

		if (offset <= slave->posStart + slave->posLen
		    || obj->next == NULL
		    || HTML_OBJECT_TYPE (obj->next) != HTML_TYPE_TEXTSLAVE) {
			html_object_calc_abs_position (obj, x, y);
			if (offset > slave->posStart)
				*x += html_text_calc_part_width (HTML_TEXT (self), painter, html_text_slave_get_text (slave),
								 slave->posStart, offset - slave->posStart, NULL, NULL);

			return;
		}
	}

	g_warning ("Getting cursor base for an HTMLText with no slaves -- %p\n",
		   self);
	html_object_calc_abs_position (self, x, y);
}

Link *
html_text_get_link_at_offset (HTMLText *text, gint offset)
{
	GSList *l;

	for (l = text->links; l; l = l->next) {
		Link *link = (Link *) l->data;

		if (link->start_offset <= offset && offset <= link->end_offset)
			return link;
	}

	return NULL;
}

static const gchar *
get_url (HTMLObject *object, gint offset)
{
	Link *link = html_text_get_link_at_offset (HTML_TEXT (object), offset);

	return link ? link->url : NULL;
}

static const gchar *
get_target (HTMLObject *object, gint offset)
{
	Link *link = html_text_get_link_at_offset (HTML_TEXT (object), offset);

	return link ? link->target : NULL;
}

void
html_text_type_init (void)
{
	html_text_class_init (&html_text_class, HTML_TYPE_TEXT, sizeof (HTMLText));
}

void
html_text_class_init (HTMLTextClass *klass,
		      HTMLType type,
		      guint object_size)
{
	HTMLObjectClass *object_class;

	object_class = HTML_OBJECT_CLASS (klass);

	html_object_class_init (object_class, type, object_size);

	object_class->destroy = destroy;
	object_class->copy = copy;
	object_class->op_copy = op_copy;
	object_class->op_cut = op_cut;
	object_class->merge = object_merge;
	object_class->split = object_split;
	object_class->draw = draw;
	object_class->accepts_cursor = accepts_cursor;
	object_class->calc_size = html_text_real_calc_size;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->calc_min_width = calc_min_width;
	object_class->fit_line = ht_fit_line;
	object_class->get_cursor = get_cursor;
	object_class->get_cursor_base = get_cursor_base;
	object_class->save = save;
	object_class->save_plain = save_plain;
	object_class->check_point = check_point;
	object_class->select_range = select_range;
	object_class->get_length = get_length;
	object_class->get_line_length = get_line_length;
	object_class->set_link = set_link;
	object_class->append_selection_string = append_selection_string;
	object_class->get_url = get_url;
	object_class->get_target = get_target;

	/* HTMLText methods.  */

	klass->queue_draw = queue_draw;
	klass->get_font_style = get_font_style;
	klass->set_font_style = set_font_style;

	parent_class = &html_object_class;
}

static gint
text_len (const gchar **str, gint len)
{
	if (g_utf8_validate (*str, -1, NULL))
		return len != -1 ? len : g_utf8_strlen (*str, -1);
	else {
		*str = "[?]";
		return 3;
	}
}

void
html_text_init (HTMLText *text,
		HTMLTextClass *klass,
		const gchar *str,
		gint len,
		GtkHTMLFontStyle font_style,
		HTMLColor *color)
{
	g_assert (color);

	html_object_init (HTML_OBJECT (text), HTML_OBJECT_CLASS (klass));

	text->text_len      = text_len (&str, len);
	text->text_bytes    = g_utf8_offset_to_pointer (str, text->text_len) - str;
	text->text          = g_strndup (str, text->text_bytes);
	text->font_style    = font_style;
	text->face          = NULL;
	text->color         = color;
	text->spell_errors  = NULL;
	text->select_start  = 0;
	text->select_length = 0;
	text->pi            = NULL;
	text->attr_list     = pango_attr_list_new ();
	text->extra_attr_list = NULL;
	text->links         = NULL;

	html_color_ref (color);
}

HTMLObject *
html_text_new_with_len (const gchar *str, gint len, GtkHTMLFontStyle font, HTMLColor *color)
{
	HTMLText *text;

	text = g_new (HTMLText, 1);

	html_text_init (text, &html_text_class, str, len, font, color);

	return HTML_OBJECT (text);
}

HTMLObject *
html_text_new (const gchar *text,
	       GtkHTMLFontStyle font,
	       HTMLColor *color)
{
	return html_text_new_with_len (text, -1, font, color);
}

void
html_text_queue_draw (HTMLText *text,
		      HTMLEngine *engine,
		      guint offset,
		      guint len)
{
	g_return_if_fail (text != NULL);
	g_return_if_fail (engine != NULL);

	(* HT_CLASS (text)->queue_draw) (text, engine, offset, len);
}


GtkHTMLFontStyle
html_text_get_font_style (const HTMLText *text)
{
	g_return_val_if_fail (text != NULL, GTK_HTML_FONT_STYLE_DEFAULT);

	return (* HT_CLASS (text)->get_font_style) (text);
}

void
html_text_set_font_style (HTMLText *text,
			  HTMLEngine *engine,
			  GtkHTMLFontStyle style)
{
	g_return_if_fail (text != NULL);

	(* HT_CLASS (text)->set_font_style) (text, engine, style);
}

void
html_text_set_font_face (HTMLText *text, HTMLFontFace *face)
{
	if (text->face)
		g_free (text->face);
	text->face = g_strdup (face);
}

void
html_text_set_text (HTMLText *text, const gchar *new_text)
{
	g_free (text->text);
	text->text_len = text_len (&new_text, -1);
	text->text = g_strdup (new_text);
	text->text_bytes = strlen (text->text);
	html_object_change_set (HTML_OBJECT (text), HTML_CHANGE_ALL);
}

/* spell checking */

#include "htmlinterval.h"

static SpellError *
spell_error_new (guint off, guint len)
{
	SpellError *se = g_new (SpellError, 1);

	se->off = off;
	se->len = len;

	return se;
}

static void
spell_error_destroy (SpellError *se)
{
	g_free (se);
}

void
html_text_spell_errors_clear (HTMLText *text)
{
	g_list_foreach (text->spell_errors, (GFunc) spell_error_destroy, NULL);
	g_list_free    (text->spell_errors);
	text->spell_errors = NULL;
}

void
html_text_spell_errors_clear_interval (HTMLText *text, HTMLInterval *i)
{
	GList *cur, *cnext;
	SpellError *se;
	guint offset, len;

	offset = html_interval_get_start  (i, HTML_OBJECT (text));
	len    = html_interval_get_length (i, HTML_OBJECT (text));
	cur    = text->spell_errors;

	/* printf ("html_text_spell_errors_clear_interval %s %d %d\n", text->text, offset, len); */

	while (cur) {
		cnext = cur->next;
		se    = (SpellError *) cur->data;
		/* test overlap */
		if (MAX (offset, se->off) <= MIN (se->off + se->len, offset + len)) {
			text->spell_errors = g_list_remove_link (text->spell_errors, cur);
			spell_error_destroy (se);
			g_list_free (cur);
		}
		cur = cnext;
	}
}

static gint
se_cmp (gconstpointer a, gconstpointer b)
{
	guint o1, o2;

	o1 = ((SpellError *) a)->off;
	o2 = ((SpellError *) b)->off;

	if (o1 < o2)  return -1;
	if (o1 == o2) return 0;
	return 1;
}

void
html_text_spell_errors_add (HTMLText *text, guint off, guint len)
{
	/* GList *cur;
	SpellError *se;
	cur = */

	text->spell_errors = g_list_insert_sorted (text->spell_errors, spell_error_new (off, len), se_cmp);

	/* printf ("---------------------------------------\n");
	while (cur) {
		se = (SpellError *) cur->data;
		printf ("off: %d len: %d\n", se->off, se->len);
		cur = cur->next;
	}
	printf ("---------------------------------------\n"); */
}

guint
html_text_get_bytes (HTMLText *text)
{
	return strlen (text->text);
}

gchar *
html_text_get_text (HTMLText *text, guint offset)
{
	gchar *s = text->text;

	while (offset--)
		s = g_utf8_next_char (s);

	return s;
}

guint
html_text_get_index (HTMLText *text, guint offset)
{
	return html_text_get_text (text, offset) - text->text;
}

gunichar
html_text_get_char (HTMLText *text, guint offset)
{
	gunichar uc;

	uc = g_utf8_get_char (html_text_get_text (text, offset));
	return uc;
}

/* magic links */

struct _HTMLMagicInsertMatch
{
	gchar *regex;
	regex_t *preg;
	gchar *prefix;
};

typedef struct _HTMLMagicInsertMatch HTMLMagicInsertMatch;

static HTMLMagicInsertMatch mim [] = {
	{ "(news|telnet|nttp|file|http|ftp|https)://([-a-z0-9]+(:[-a-z0-9]+)?@)?[-a-z0-9.]+[-a-z0-9](:[0-9]*)?(/[-a-z0-9_$.+!*(),;:@%&=?/~#]*[^]'.}>\\) ,?!;:\"]?)?", NULL, NULL },
	{ "www\\.[-a-z0-9.]+[-a-z0-9](:[0-9]*)?(/[-A-Za-z0-9_$.+!*(),;:@%&=?/~#]*[^]'.}>\\) ,?!;:\"]?)?", NULL, "http://" },
	{ "ftp\\.[-a-z0-9.]+[-a-z0-9](:[0-9]*)?(/[-A-Za-z0-9_$.+!*(),;:@%&=?/~#]*[^]'.}>\\) ,?!;:\"]?)?", NULL, "ftp://" },
	{ "[-_a-z0-9.\\+]+@[-_a-z0-9.]+", NULL, "mailto:" }
};

#define MIM_N (sizeof (mim) / sizeof (mim [0]))

void
html_engine_init_magic_links (void)
{
	gint i;

	for (i=0; i<MIM_N; i++) {
		mim [i].preg = g_new0 (regex_t, 1);
		if (regcomp (mim [i].preg, mim [i].regex, REG_EXTENDED | REG_ICASE)) {
			/* error */
			g_free (mim [i].preg);
			mim [i].preg = 0;
		}
	}
}

static void
paste_link (HTMLEngine *engine, HTMLText *text, gint so, gint eo, gchar *prefix)
{
	gchar *href;
	gchar *base;

	base = g_strndup (html_text_get_text (text, so), html_text_get_index (text, eo) - html_text_get_index (text, so));
	href = (prefix) ? g_strconcat (prefix, base, NULL) : g_strdup (base);
	g_free (base);

	html_text_add_link (text, engine, href, NULL, so, eo);
	g_free (href);
}

gboolean
html_text_magic_link (HTMLText *text, HTMLEngine *engine, guint offset)
{
	regmatch_t pmatch [2];
	gint i;
	gboolean rv = FALSE, exec = TRUE;
	gint saved_position;
	gunichar uc;
	char *str, *cur;

	if (!offset)
		return FALSE;
	offset--;

	/* printf ("html_text_magic_link\n"); */

	html_undo_level_begin (engine->undo, "Magic link", "Remove magic link");
	saved_position = engine->cursor->position;

	cur = str = html_text_get_text (text, offset);

	/* check forward to ensure chars are < 0x80, could be removed once we have utf8 regex */
	while (TRUE) {
		cur = g_utf8_next_char (cur);
		if (!*cur)
			break;
		uc = g_utf8_get_char (cur);
		if (uc >= 0x80) {
			exec = FALSE;
			break;
		} else if (uc == ' ' || uc == ENTITY_NBSP) {
			break;
		}
	}

	uc = g_utf8_get_char (str);
	if (uc >= 0x80)
		exec = FALSE;
	while (exec && uc != ' ' && uc != ENTITY_NBSP && offset) {
		str = g_utf8_prev_char (str);
		uc = g_utf8_get_char (str);
		if (uc >= 0x80)
			exec = FALSE;
		offset--;
	}

	if (uc == ' ' || uc == ENTITY_NBSP) {
		str = g_utf8_next_char (str);
		offset++;
	}

	if (exec) {
		for (i=0; i<MIM_N; i++) {
			if (mim [i].preg && !regexec (mim [i].preg, str, 2, pmatch, 0)) {
				paste_link (engine, text,
					    g_utf8_pointer_to_offset (text->text, str + pmatch [0].rm_so),
					    g_utf8_pointer_to_offset (text->text, str + pmatch [0].rm_eo), mim [i].prefix);
					rv = TRUE;
					break;
			}
		}
	}

	html_undo_level_end (engine->undo);
	html_cursor_jump_to_position_no_spell (engine->cursor, engine, saved_position);

	return rv;
}

/*
 * magic links end
 */

gint
html_text_trail_space_width (HTMLText *text, HTMLPainter *painter)
{
	return text->text_len > 0 && html_text_get_char (text, text->text_len - 1) == ' '
		? html_painter_get_space_width (painter, html_text_get_font_style (text), text->face) : 0;
}

void
html_text_append (HTMLText *text, const gchar *str, gint len)
{
	gchar *to_delete;

	to_delete       = text->text;
	text->text_len += text_len (&str, len);
	text->text_bytes += strlen (str);
	text->text      = g_strconcat (to_delete, str, NULL);

	g_free (to_delete);

	html_object_change_set (HTML_OBJECT (text), HTML_CHANGE_ALL);
}

void
html_text_append_link_full (HTMLText *text, gchar *url, gchar *target, gint start_index, gint end_index, gint start_offset, gint end_offset)
{
	text->links = g_slist_prepend (text->links, html_link_new (url, target, start_index, end_index, start_offset, end_offset));
}

static void
html_text_offsets_to_indexes (HTMLText *text, gint so, gint eo, gint *si, gint *ei)
{
	*si = html_text_get_index (text, so);
	*ei = g_utf8_offset_to_pointer (text->text + *si, eo - so) - text->text;
}

void
html_text_append_link (HTMLText *text, gchar *url, gchar *target, gint start_offset, gint end_offset)
{
	gint start_index, end_index;

	html_text_offsets_to_indexes (text, start_offset, end_offset, &start_index, &end_index);
	html_text_append_link_full (text, url, target, start_index, end_index, start_offset, end_offset);
}

void
html_text_add_link_full (HTMLText *text, HTMLEngine *e, gchar *url, gchar *target, gint start_index, gint end_index, gint start_offset, gint end_offset)
{
	GSList *l, *prev = NULL;
	Link *link;

	cut_links_full (text, start_offset, end_offset, start_index, end_index, 0, 0);

	if (text->links == NULL)
		html_text_append_link_full (text, url, target, start_index, end_index, start_offset, end_offset);
	else {
		Link *plink = NULL, *new_link = html_link_new (url, target, start_index, end_index, start_offset, end_offset);

		for (l = text->links; new_link && l; l = l->next) {
			link = (Link *) l->data;
			if (new_link->start_offset >= link->end_offset) {
				if (new_link->start_offset == link->end_offset && html_link_equal (link, new_link)) {
					link->end_offset = end_offset;
					link->end_index = end_index;
					html_link_free (new_link);
					new_link = NULL;
				} else {
					l = g_slist_prepend (l, new_link);
					if (prev)
						prev->next = l;
					else
						text->links = l;
					link = new_link;
					new_link = NULL;
				}
				if (plink && html_link_equal (plink, link) && plink->start_offset == link->end_offset) {
					plink->start_offset = link->start_offset;
					plink->start_index = link->start_index;
					prev->next = g_slist_remove (prev->next, link);
					html_link_free (link);
					link = plink;
				}
				plink = link;
				prev = l;
			}
		}

		if (new_link && prev)
			prev->next = g_slist_prepend (NULL, new_link);
	}
}

void
html_text_add_link (HTMLText *text, HTMLEngine *e, gchar *url, gchar *target, gint start_offset, gint end_offset)
{
	gint start_index, end_index;

	html_text_offsets_to_indexes (text, start_offset, end_offset, &start_index, &end_index);
	html_text_add_link_full (text, e, url, target, start_index, end_index, start_offset, end_offset);
}

void
html_text_remove_links (HTMLText *text)
{
	if (text->links) {
		free_links (text->links);
		text->links = NULL;
		html_object_change_set (HTML_OBJECT (text), HTML_CHANGE_RECALC_PI);
	}
}

HTMLTextSlave *
html_text_get_slave_at_offset (HTMLObject *o, gint offset)
{
	if (!o || (!HTML_IS_TEXT (o) && !HTML_IS_TEXT_SLAVE (o)))
		return NULL;

	if (HTML_IS_TEXT (o))
		o = o->next;

	while (o && HTML_IS_TEXT_SLAVE (o)) {
		if (HTML_IS_TEXT_SLAVE (o) && HTML_TEXT_SLAVE (o)->posStart <= offset
		    && (offset < HTML_TEXT_SLAVE (o)->posStart + HTML_TEXT_SLAVE (o)->posLen
			|| (offset == HTML_TEXT_SLAVE (o)->posStart + HTML_TEXT_SLAVE (o)->posLen && HTML_TEXT_SLAVE (o)->owner->text_len == offset)))
			return HTML_TEXT_SLAVE (o);
		o = o->next;
	}

	return NULL;
}

Link *
html_text_get_link_slaves_at_offset (HTMLText *text, gint offset, HTMLTextSlave **start, HTMLTextSlave **end)
{
	Link *link = html_text_get_link_at_offset (text, offset);

	if (link) {
		*start = html_text_get_slave_at_offset (HTML_OBJECT (text), link->start_offset);
		*end = html_text_get_slave_at_offset (HTML_OBJECT (*start), link->end_offset);

		if (*start && *end)
			return link;
	}

	return NULL;
}

gboolean
html_text_get_link_rectangle (HTMLText *text, HTMLPainter *painter, gint offset, gint *x1, gint *y1, gint *x2, gint *y2)
{
	HTMLTextSlave *start;
	HTMLTextSlave *end;
	Link *link;

	link = html_text_get_link_slaves_at_offset (text, offset, &start, &end);
	if (link) {
		gint xs, ys, xe, ye;

		html_object_calc_abs_position (HTML_OBJECT (start), &xs, &ys);
		xs += html_text_calc_part_width (text, painter, html_text_slave_get_text (start), start->posStart, link->start_offset - start->posStart, NULL, NULL);
		ys -= HTML_OBJECT (start)->ascent;

		html_object_calc_abs_position (HTML_OBJECT (end), &xe, &ye);
		xe += HTML_OBJECT (end)->width;
		xe -= html_text_calc_part_width (text, painter, text->text + link->end_index, link->end_offset, end->posStart + start->posLen - link->end_offset, NULL, NULL);
		ye += HTML_OBJECT (end)->descent;

		*x1 = MIN (xs, xe);
		*y1 = MIN (ys, ye);
		*x2 = MAX (xs, xe);
		*y2 = MAX (ys, ye);

		return TRUE;
	}

	return FALSE;
}

gboolean
html_text_prev_link_offset (HTMLText *text, gint *offset)
{
	GSList *l;

	for (l = text->links; l; l = l->next) {
		Link *link = (Link *) l->data;

		if (link->start_offset <= *offset && *offset <= link->end_offset) {
			if (l->next) {
				*offset = ((Link *) l->next->data)->end_offset - 1;
				return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

gboolean
html_text_next_link_offset (HTMLText *text, gint *offset)
{
	GSList *l, *prev = NULL;

	for (l = text->links; l; l = l->next) {
		Link *link = (Link *) l->data;

		if (link->start_offset <= *offset && *offset <= link->end_offset) {
			if (prev) {
				*offset = ((Link *) prev->data)->start_offset + 1;
				return TRUE;
			}
			break;
		}
		prev = l;
	}

	return FALSE;
}

gboolean
html_text_first_link_offset (HTMLText *text, gint *offset)
{
	if (text->links)
		*offset = ((Link *) g_slist_last (text->links)->data)->start_offset + 1;

	return text->links != NULL;
}

gboolean
html_text_last_link_offset (HTMLText *text, gint *offset)
{
	if (text->links)
		*offset = ((Link *) text->links->data)->end_offset - 1;

	return text->links != NULL;
}

gchar *
html_text_get_link_text (HTMLText *text, gint offset)
{
	Link *link = html_text_get_link_at_offset (text, offset);
	gchar *start;

	start = html_text_get_text (text, link->start_offset);

	return g_strndup (start, g_utf8_offset_to_pointer (start, link->end_offset - link->start_offset) - start);
}

void
html_link_set_url_and_target (Link *link, gchar *url, gchar *target)
{
	if (!link)
		return;

	g_free (link->url);
	g_free (link->target);

	link->url = g_strdup (url);
	link->target = g_strdup (target);
}

Link *
html_link_dup (Link *l)
{
	Link *nl = g_new (Link, 1);

	nl->url = g_strdup (l->url);
	nl->target = g_strdup (l->target);
	nl->start_offset = l->start_offset;
	nl->end_offset = l->end_offset;
	nl->start_index = l->start_index;
	nl->end_index = l->end_index;

	return nl;
}

void
html_link_free (Link *link)
{
	g_return_if_fail (link != NULL);

	g_free (link->url);
	g_free (link->target);
	g_free (link);
}

gboolean
html_link_equal (Link *l1, Link *l2)
{
	return l1->url && l2->url && !strcasecmp (l1->url, l2->url)
		&& (l1->target == l2->target || (l1->target && l2->target && !strcasecmp (l1->target, l2->target)));
}

Link *
html_link_new (gchar *url, gchar *target, guint start_index, guint end_index, gint start_offset, gint end_offset)
{
	Link *link = g_new0 (Link, 1);

	link->url = g_strdup (url);
	link->target = g_strdup (target);
	link->start_offset = start_offset;
	link->end_offset = end_offset;
	link->start_index = start_index;
	link->end_index = end_index;

	return link;
}

/* extended pango attributes */

static PangoAttribute *
html_pango_attr_font_size_copy (const PangoAttribute *attr)
{
	HTMLPangoAttrFontSize *font_size_attr = (HTMLPangoAttrFontSize *) attr, *new_attr;

	new_attr = (HTMLPangoAttrFontSize *) html_pango_attr_font_size_new (font_size_attr->style);
	new_attr->attr_int.value = font_size_attr->attr_int.value;

	return (PangoAttribute *) new_attr;
}

static void
html_pango_attr_font_size_destroy (PangoAttribute *attr)
{
	g_free (attr);
}

static gboolean
html_pango_attr_font_size_equal (const PangoAttribute *attr1, const PangoAttribute *attr2)
{
	const HTMLPangoAttrFontSize *font_size_attr1 = (const HTMLPangoAttrFontSize *) attr1;
	const HTMLPangoAttrFontSize *font_size_attr2 = (const HTMLPangoAttrFontSize *) attr2;
  
	return (font_size_attr1->style == font_size_attr2->style);
}

void
html_pango_attr_font_size_calc (HTMLPangoAttrFontSize *attr, HTMLEngine *e)
{
	gint size, base_size, real_size;

	base_size = (attr->style & GTK_HTML_FONT_STYLE_FIXED) ? e->painter->font_manager.fix_size : e->painter->font_manager.var_size;
	if ((attr->style & GTK_HTML_FONT_STYLE_SIZE_MASK) != 0)
		size = (attr->style & GTK_HTML_FONT_STYLE_SIZE_MASK) - GTK_HTML_FONT_STYLE_SIZE_3;
	else
		size = 0;
	real_size = e->painter->font_manager.magnification * ((gdouble) base_size + (size > 0 ? (1 << size) : size) * base_size/8.0);

	attr->attr_int.value = real_size;
}

static const PangoAttrClass html_pango_attr_font_size_klass = {
	PANGO_ATTR_SIZE,
	html_pango_attr_font_size_copy,
	html_pango_attr_font_size_destroy,
	html_pango_attr_font_size_equal
};

PangoAttribute *
html_pango_attr_font_size_new (GtkHTMLFontStyle style)
{
	HTMLPangoAttrFontSize *result = g_new (HTMLPangoAttrFontSize, 1);
	result->attr_int.attr.klass = &html_pango_attr_font_size_klass;
	result->style = style;

	return (PangoAttribute *) result;
}

static gboolean
calc_font_size_filter (PangoAttribute *attr, gpointer data)
{
	HTMLEngine *e = HTML_ENGINE (data);

	if (attr->klass->type == PANGO_ATTR_SIZE)
		html_pango_attr_font_size_calc ((HTMLPangoAttrFontSize *) attr, e);
	else if (attr->klass->type == PANGO_ATTR_FAMILY) {
		/* FIXME: this is not very nice. we set it here as it's only used when fonts changed.
		   once family in style is used again, that code must be updated */
		PangoAttrString *sa = (PangoAttrString *) attr;
		g_free (sa->value);
		sa->value = g_strdup (e->painter->font_manager.fixed.face);
	}

	return FALSE;
}

void
html_text_calc_font_size (HTMLText *text, HTMLEngine *e)
{
	pango_attr_list_filter (text->attr_list, calc_font_size_filter, e);
}

static GtkHTMLFontStyle
style_from_attrs (PangoAttrIterator *iter)
{
	GtkHTMLFontStyle style = GTK_HTML_FONT_STYLE_DEFAULT;
	GSList *list, *l;

	list = pango_attr_iterator_get_attrs (iter);
	for (l = list; l; l = l->next) {
		PangoAttribute *attr = (PangoAttribute *) l->data;

		switch (attr->klass->type) {
		case PANGO_ATTR_WEIGHT:
			style |= GTK_HTML_FONT_STYLE_BOLD;
			break;
		case PANGO_ATTR_UNDERLINE:
			style |= GTK_HTML_FONT_STYLE_UNDERLINE;
			break;
		case PANGO_ATTR_STRIKETHROUGH:
			style |= GTK_HTML_FONT_STYLE_STRIKEOUT;
			break;
		case PANGO_ATTR_STYLE:
			style |= GTK_HTML_FONT_STYLE_ITALIC;
			break;
		case PANGO_ATTR_SIZE:
			style |= ((HTMLPangoAttrFontSize *) attr)->style;
			break;
		case PANGO_ATTR_FAMILY:
			style |= GTK_HTML_FONT_STYLE_FIXED;
			break;
		default:
			break;
		}
	}

	html_text_free_attrs (list);

	return style;
}

GtkHTMLFontStyle
html_text_get_fontstyle_at_index (HTMLText *text, gint index)
{
	GtkHTMLFontStyle style = GTK_HTML_FONT_STYLE_DEFAULT;
	PangoAttrIterator *iter = pango_attr_list_get_iterator (text->attr_list);

	if (iter) {
		do {
			gint start_index, end_index;

			pango_attr_iterator_range (iter, &start_index, &end_index);
			if (start_index <= index && index <= end_index) {
				style |= style_from_attrs (iter);
				break;
			}
		} while (pango_attr_iterator_next (iter));

		pango_attr_iterator_destroy (iter);
	}

	return style;
}

GtkHTMLFontStyle
html_text_get_style_conflicts (HTMLText *text, GtkHTMLFontStyle style, gint start_index, gint end_index)
{
	GtkHTMLFontStyle conflicts = GTK_HTML_FONT_STYLE_DEFAULT;
	PangoAttrIterator *iter = pango_attr_list_get_iterator (text->attr_list);

	if (iter) {
		do {
			gint iter_start_index, iter_end_index;

			pango_attr_iterator_range (iter, &iter_start_index, &iter_end_index);
			if (MAX (start_index, iter_start_index)  < MIN (end_index, iter_end_index))
				conflicts |= style_from_attrs (iter) ^ style;
			if (iter_start_index > end_index)
				break;
		} while (pango_attr_iterator_next (iter));

		pango_attr_iterator_destroy (iter);
	}

	return conflicts;
}

void
html_text_change_attrs (PangoAttrList *attr_list, GtkHTMLFontStyle style, HTMLEngine *e, gint start_index, gint end_index, gboolean avoid_default_size)
{
	PangoAttribute *attr;

	/* style */
	if (style & GTK_HTML_FONT_STYLE_BOLD) {
		attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change (attr_list, attr);
	}

	if (style & GTK_HTML_FONT_STYLE_ITALIC) {
		attr = pango_attr_style_new (PANGO_STYLE_ITALIC);
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change (attr_list, attr);
	}

	if (style & GTK_HTML_FONT_STYLE_UNDERLINE) {
		attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change (attr_list, attr);
	}

	if (style & GTK_HTML_FONT_STYLE_STRIKEOUT) {
		attr = pango_attr_strikethrough_new (TRUE);
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change (attr_list, attr);
	}

	if (style & GTK_HTML_FONT_STYLE_FIXED) {
		attr = pango_attr_family_new (e->painter->font_manager.fixed.face ? e->painter->font_manager.fixed.face : "Monospace");
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change (attr_list, attr);
	}

	if (!avoid_default_size
	    || (((style & GTK_HTML_FONT_STYLE_SIZE_MASK) != GTK_HTML_FONT_STYLE_DEFAULT)
		&& ((style & GTK_HTML_FONT_STYLE_SIZE_MASK) != GTK_HTML_FONT_STYLE_SIZE_3))) {
		attr = html_pango_attr_font_size_new (style);
		html_pango_attr_font_size_calc ((HTMLPangoAttrFontSize *) attr, e);
		attr->start_index = start_index;
		attr->end_index = end_index;
		pango_attr_list_change (attr_list, attr);
	}
}

void
html_text_set_style_in_range (HTMLText *text, GtkHTMLFontStyle style, HTMLEngine *e, gint start_index, gint end_index)
{
	html_text_change_attrs (text->attr_list, style, e, start_index, end_index, TRUE);
}

void
html_text_set_style (HTMLText *text, GtkHTMLFontStyle style, HTMLEngine *e)
{
	html_text_set_style_in_range (text, style, e, 0, text->text_bytes);
}

static gboolean
unset_style_filter (PangoAttribute *attr, gpointer data)
{
	GtkHTMLFontStyle style = GPOINTER_TO_INT (data);

	switch (attr->klass->type) {
	case PANGO_ATTR_WEIGHT:
		if (style & GTK_HTML_FONT_STYLE_BOLD)
			return TRUE;
		break;
	case PANGO_ATTR_STYLE:
		if (style & GTK_HTML_FONT_STYLE_ITALIC)
			return TRUE;
		break;
	case PANGO_ATTR_UNDERLINE:
		if (style & GTK_HTML_FONT_STYLE_UNDERLINE)
			return TRUE;
		break;
	case PANGO_ATTR_STRIKETHROUGH:
		if (style & GTK_HTML_FONT_STYLE_STRIKEOUT)
			return TRUE;
		break;
	case PANGO_ATTR_SIZE:
		if (((HTMLPangoAttrFontSize *) attr)->style & style)
			return TRUE;
		break;
	case PANGO_ATTR_FAMILY:
		if (style & GTK_HTML_FONT_STYLE_FIXED)
			return TRUE;
		break;
	default:
		break;
	}

	return FALSE;
}

void
html_text_unset_style (HTMLText *text, GtkHTMLFontStyle style)
{
	pango_attr_list_filter (text->attr_list, unset_style_filter, GINT_TO_POINTER (style));
}

static HTMLColor *
color_from_attrs (PangoAttrIterator *iter)
{
	HTMLColor *color = NULL;
	GSList *list, *l;

	list = pango_attr_iterator_get_attrs (iter);
	for (l = list; l; l = l->next) {
		PangoAttribute *attr = (PangoAttribute *) l->data;
		PangoAttrColor *ca;

		switch (attr->klass->type) {
		case PANGO_ATTR_FOREGROUND:
			ca = (PangoAttrColor *) attr;
			color = html_color_new_from_rgb (ca->color.red, ca->color.green, ca->color.blue);
			break;
		default:
			break;
		}
	}

	html_text_free_attrs (list);

	return color;
}

static HTMLColor *
html_text_get_first_color_in_range (HTMLText *text, HTMLEngine *e, gint start_index, gint end_index)
{
	HTMLColor *color = NULL;
	PangoAttrIterator *iter = pango_attr_list_get_iterator (text->attr_list);

	if (iter) {
		do {
			gint iter_start_index, iter_end_index;

			pango_attr_iterator_range (iter, &iter_start_index, &iter_end_index);
			if (MAX (iter_start_index, start_index) <= MIN (iter_end_index, end_index)) {
				color = color_from_attrs (iter);
				break;
			}
		} while (pango_attr_iterator_next (iter));

		pango_attr_iterator_destroy (iter);
	}

	if (!color) {
		color = html_colorset_get_color (e->settings->color_set, HTMLTextColor);
		html_color_ref (color);
	}

	return color;
}

HTMLColor *
html_text_get_color_at_index (HTMLText *text, HTMLEngine *e, gint index)
{
	return html_text_get_first_color_in_range (text, e, index, index);
}

HTMLColor *
html_text_get_color (HTMLText *text, HTMLEngine *e, gint start_index)
{
	return html_text_get_first_color_in_range (text, e, start_index, text->text_bytes);
}

void
html_text_set_color_in_range (HTMLText *text, HTMLColor *color, gint start_index, gint end_index)
{
	PangoAttribute *attr = pango_attr_foreground_new (color->color.red, color->color.green, color->color.blue);

	attr->start_index = start_index;
	attr->end_index = end_index;
	pango_attr_list_change (text->attr_list, attr);
}

void
html_text_set_color (HTMLText *text, HTMLColor *color)
{
	html_text_set_color_in_range (text, color, 0, text->text_bytes);
}
