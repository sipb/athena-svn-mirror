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
#include <gal/widgets/e-unicode.h>

#include "htmltext.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlclueflow.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-save.h"
#include "htmlentity.h"
#include "htmllinktext.h"
#include "htmlsettings.h"
#include "htmltextslave.h"
#include "htmlundo.h"

HTMLTextClass html_text_class;
static HTMLObjectClass *parent_class = NULL;

#define HT_CLASS(x) HTML_TEXT_CLASS (HTML_OBJECT (x)->klass)

static SpellError * spell_error_new         (guint off, guint len);
static void         spell_error_destroy     (SpellError *se);
static void         move_spell_errors       (GList *spell_errors, guint offset, gint delta);
static GList *      remove_spell_errors     (GList *spell_errors, guint offset, guint len);
static guint        get_words               (const gchar *s);
static void         remove_text_slaves      (HTMLObject *self);

/* static void
debug_spell_errors (GList *se)
{
	for (;se;se = se->next)
		printf ("SE: %4d, %4d\n", ((SpellError *) se->data)->off, ((SpellError *) se->data)->len);
} */

#ifndef GAL_NOT_SLOW
/* NOTE see htmltextslave.c for an explanation and the h_utf8 decls */
gint h_utf8_pointer_to_offset (const gchar *str, const gchar *pos);
gint h_utf8_strlen (const gchar *p, gint max);
gchar *h_utf8_offset_to_pointer  (const gchar *str, gint         offset);

#define g_utf8_strlen h_utf8_strlen
#define g_utf8_offset_to_pointer h_utf8_offset_to_pointer
#define g_utf8_pointer_to_offset h_utf8_pointer_to_offset
#endif

static inline gboolean
is_in_the_save_cluev (HTMLObject *text, HTMLObject *o)
{
	return html_object_nth_parent (o, 2) == html_object_nth_parent (text, 2);
}

static void
get_tags (const HTMLText *text,
	  const HTMLEngineSaveState *state,
	  gchar **opening_tags,
	  gchar **closing_tags)
{
	GtkHTMLFontStyle font_style;
	GString *ot, *ct;
	HTMLObject *prev, *next;
	HTMLText *pt = NULL, *nt = NULL;
	gboolean font_tag = FALSE;
	gboolean std_color, std_size;

	font_style = text->font_style;

	ot = g_string_new (NULL);
	ct = g_string_new (NULL);

	prev = html_object_prev_cursor_leaf (HTML_OBJECT (text), state->engine);
	while (prev && !html_object_is_text (prev))
		prev = html_object_prev_cursor_leaf (prev, state->engine);

	next = html_object_next_cursor_leaf (HTML_OBJECT (text), state->engine);
	while (next && !html_object_is_text (next))
		next = html_object_next_cursor_leaf (next, state->engine);

	if (prev && is_in_the_save_cluev (HTML_OBJECT (text), prev) && html_object_is_text (prev))
		pt = HTML_TEXT (prev);
	if (next && is_in_the_save_cluev (HTML_OBJECT (text), next) && html_object_is_text (next))
		nt = HTML_TEXT (next);

	/* font tag */
	std_color = html_color_equal (text->color, html_colorset_get_color (state->engine->settings->color_set,
									     HTMLTextColor))
		|| html_color_equal (text->color,
				      html_colorset_get_color (state->engine->settings->color_set, HTMLLinkColor));
	std_size = (font_style & GTK_HTML_FONT_STYLE_SIZE_MASK) == 0;


	if ((!std_color || !std_size)
	    && (!pt
		|| !html_color_equal (text->color, pt->color)
		|| (pt->font_style & GTK_HTML_FONT_STYLE_SIZE_MASK) != (font_style & GTK_HTML_FONT_STYLE_SIZE_MASK))) {
		if (!std_color) {
			g_string_sprintfa (ot, "<FONT COLOR=\"#%02x%02x%02x\"",
					   text->color->color.red   >> 8,
					   text->color->color.green >> 8,
					   text->color->color.blue  >> 8);
			font_tag = TRUE;
		}
		if (!std_size) {
			if (!font_tag)
				g_string_append (ot, "<FONT");
			g_string_sprintfa (ot, " SIZE=\"%d\"", font_style & GTK_HTML_FONT_STYLE_SIZE_MASK);
		}
		g_string_append_c (ot, '>');
	}

	if ((!std_color || !std_size)
	    && (!nt
		|| !html_color_equal (text->color, nt->color)
		|| (nt->font_style & GTK_HTML_FONT_STYLE_SIZE_MASK) != (font_style & GTK_HTML_FONT_STYLE_SIZE_MASK))) {
		g_string_append (ct, "</FONT>");
	}

	/* bold tag */
	if (font_style & GTK_HTML_FONT_STYLE_BOLD) {
		if (!pt || !(pt->font_style & GTK_HTML_FONT_STYLE_BOLD))
			g_string_append (ot, "<B>");
		if (!nt || !(nt->font_style & GTK_HTML_FONT_STYLE_BOLD))
			g_string_prepend (ct, "</B>");
	}

	/* italic tag */
	if (font_style & GTK_HTML_FONT_STYLE_ITALIC) {
		if (!pt || !(pt->font_style & GTK_HTML_FONT_STYLE_ITALIC))
			g_string_append (ot, "<I>");
		if (!nt || !(nt->font_style & GTK_HTML_FONT_STYLE_ITALIC))
			g_string_prepend (ct, "</I>");
	}

	/* underline tag */
	if (font_style & GTK_HTML_FONT_STYLE_UNDERLINE) {
		if (!pt || !(pt->font_style & GTK_HTML_FONT_STYLE_UNDERLINE))
			g_string_append (ot, "<U>");
		if (!nt || !(nt->font_style & GTK_HTML_FONT_STYLE_UNDERLINE))
			g_string_prepend (ct, "</U>");
	}

	/* strikeout tag */
	if (font_style & GTK_HTML_FONT_STYLE_STRIKEOUT) {
		if (!pt || !(pt->font_style & GTK_HTML_FONT_STYLE_STRIKEOUT))
			g_string_append (ot, "<S>");
		if (!nt || !(nt->font_style & GTK_HTML_FONT_STYLE_STRIKEOUT))
			g_string_prepend (ct, "</S>");
	}

	/* fixed tag */
	if (font_style & GTK_HTML_FONT_STYLE_FIXED) {
		if (!pt || !(pt->font_style & GTK_HTML_FONT_STYLE_FIXED))
			g_string_append (ot, "<TT>");
		if (!nt || !(nt->font_style & GTK_HTML_FONT_STYLE_FIXED))
			g_string_prepend (ct, "</TT>");
	}

	*opening_tags = ot->str;
	*closing_tags = ct->str;

	g_string_free (ot, FALSE);
	g_string_free (ct, FALSE);
}

/* HTMLObject methods.  */

static void
copy (HTMLObject *s,
      HTMLObject *d)
{
	HTMLText *src  = HTML_TEXT (s);
	HTMLText *dest = HTML_TEXT (d);
	GList *cur;

	(* HTML_OBJECT_CLASS (parent_class)->copy) (s, d);

	dest->text = g_strdup (src->text);
	dest->text_len      = src->text_len;
	dest->font_style    = src->font_style;
	dest->face          = g_strdup (src->face);
	dest->color         = src->color;
	dest->select_start  = src->select_start;
	dest->select_length = src->select_length;

	html_color_ref (dest->color);

	dest->spell_errors = g_list_copy (src->spell_errors);
	cur = dest->spell_errors;
	while (cur) {
		SpellError *se = (SpellError *) cur->data;
		cur->data = spell_error_new (se->off, se->len);
		cur = cur->next;
	}

	dest->words      = 0;
	dest->word_width = NULL;
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

void
html_text_clear_word_width (HTMLText *text)
{
	g_free (text->word_width);
	text->word_width = NULL;
	text->words = 0;
}

HTMLObject *
html_text_op_copy_helper (HTMLText *text, GList *from, GList *to, guint *len, HTMLTextHelperFunc f)
{
	gint begin, end;

	begin = (from) ? GPOINTER_TO_INT (from->data) : 0;
	end   = (to)   ? GPOINTER_TO_INT (to->data)   : text->text_len;

	*len += end - begin;

	return (*f) (text, begin, end);

	/* word_get_position (text, begin, &w1, &o1l, &o1r);
	word_get_position (text, end,   &w2, &o2l, &o2r);

	ct->words      = w2 - w1 + (o1r == 0 ? 1 : 0);
	ct->word_width = g_new (guint, ct->words);

	ct->word_width [0] = 0;
	for (i = 1; i < ct->words; i++)
	ct->word_width [i] = text->word_width [w1 + i]; */
}

HTMLObject *
html_text_op_cut_helper (HTMLText *text, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right,
			 guint *len, HTMLTextHelperFunc f)
{
	HTMLObject *rv;
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

		if (begin == end)
			return (*f) (text, 0, 0);

		rv = (*f) (text, begin, end);

		tail = html_text_get_text (text, end);
		text->text [html_text_get_index (text, begin)] = 0;
		nt = g_strconcat (text->text, tail, NULL);
		g_free (text->text);
		text->text = nt;
		text->text_len -= end - begin;
		*len           += end - begin;

		text->spell_errors = remove_spell_errors (text->spell_errors, begin, end - begin);
		move_spell_errors (text->spell_errors, end, - (end - begin));
		html_text_convert_nbsp (text, TRUE);
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

	html_text_clear_word_width (text);
	html_object_change_set (HTML_OBJECT (text), HTML_CHANGE_ALL_CALC);

	/* printf ("after cut '%s'\n", text->text);
	   debug_word_width (text); */

	return rv;
}

static HTMLObject *
new_text (HTMLText *t, gint begin, gint end)
{
	return HTML_OBJECT (html_text_new_with_len (html_text_get_text (t, begin),
						    end - begin, t->font_style, t->color));
}

static HTMLObject *
op_copy (HTMLObject *self, HTMLObject *parent, HTMLEngine *e, GList *from, GList *to, guint *len)
{
	return html_text_op_copy_helper (HTML_TEXT (self), from, to, len, new_text);
}

static HTMLObject *
op_cut (HTMLObject *self, HTMLEngine *e, GList *from, GList *to, GList *left, GList *right, guint *len)
{
	return html_text_op_cut_helper (HTML_TEXT (self), e, from, to, left, right, len, new_text);
}

static gboolean
object_merge (HTMLObject *self, HTMLObject *with, HTMLEngine *e, GList **left, GList **right, HTMLCursor *cursor)
{
	HTMLText *t1, *t2;
	gchar *to_free;

	t1 = HTML_TEXT (self);
	t2 = HTML_TEXT (with);

	if (t1->font_style != t2->font_style || t1->color != t2->color)
		return FALSE;

	/* printf ("merge '%s' '%s'\n", t1->text, t2->text); */

	/* merge_word_width (t1, t2, e->painter); */

	if (e->cursor->object == with) {
		e->cursor->object  = self;
		e->cursor->offset += t1->text_len;
	}

	move_spell_errors (t2->spell_errors, 0, t1->text_len);
	t1->spell_errors = g_list_concat (t1->spell_errors, t2->spell_errors);
	t2->spell_errors = NULL;

	to_free       = t1->text;
	t1->text      = g_strconcat (t1->text, t2->text, NULL);
	t1->text_len += t2->text_len;
	g_free (to_free);
	html_text_convert_nbsp (t1, TRUE);
	html_object_change_set (self, HTML_CHANGE_ALL_CALC);

	html_text_clear_word_width (t1);
	/* html_text_request_word_width (t1, e->painter); */
	/* printf ("merged '%s'\n", t1->text); */
	/* printf ("--- after merge\n");
	   debug_spell_errors (t1->spell_errors);
	   printf ("---\n"); */

	return TRUE;
}

static void
object_split (HTMLObject *self, HTMLEngine *e, HTMLObject *child, gint offset, gint level, GList **left, GList **right)
{
	HTMLObject *dup, *prev;
	HTMLText *t1, *t2;
	gchar *tt;

	g_assert (self->parent);

	html_clue_remove_text_slaves (HTML_CLUE (self->parent));

	t1              = HTML_TEXT (self);
	dup             = html_object_dup (self);
	tt              = t1->text;
	t1->text        = g_strndup (tt, html_text_get_index (t1, offset));
	t1->text_len    = offset;
	g_free (tt);
	html_text_convert_nbsp (t1, TRUE);

	t2              = HTML_TEXT (dup);
	tt              = t2->text;
	t2->text        = html_text_get_text (t2, offset);
	t2->text_len   -= offset;
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
	printf ("---\n"); */

	*left  = g_list_prepend (*left, self);
	*right = g_list_prepend (*right, dup);

	html_object_change_set (self, HTML_CHANGE_ALL_CALC);
	html_object_change_set (dup,  HTML_CHANGE_ALL_CALC);

	html_text_clear_word_width (HTML_TEXT (self));
	html_text_clear_word_width (HTML_TEXT (dup));

	level--;
	if (level)
		html_object_split (self->parent, e, dup, 0, level, left, right);
}

static gboolean
calc_size (HTMLObject *self, HTMLPainter *painter, GList **changed_objs)
{
	HTMLText *text = HTML_TEXT (self);
	GtkHTMLFontStyle style = html_text_get_font_style (text);

	self->width = 0;
	self->ascent = html_painter_calc_ascent (painter, style, text->face);
	self->descent = html_painter_calc_descent (painter, style, text->face);

	return FALSE;
}

gint
html_text_text_line_length (const gchar *text, gint line_offset, guint len)
{
	const gchar *tab, *found_tab;
	gint lo, cl, l, skip, sum_skip;

	/* printf ("lo: %d o: %d t: '%s'\n", line_offset, len, text); */
	l = 0;
	lo = line_offset;
	sum_skip = 0;
	tab = text;
	while (tab && (found_tab = strchr (tab, '\t')) && l < len) {
		cl   = g_utf8_pointer_to_offset (tab, found_tab);
		l   += cl;
		if (l >= len)
			break;
		lo  += cl;
		skip = 8 - (lo % 8);
		tab  = found_tab + 1;
		lo  += skip;
		sum_skip += skip - 1;
		l ++;
	}

	return len + sum_skip;
}

static guint
get_line_length (HTMLObject *self, HTMLPainter *p, gint line_offset)
{
	return html_clueflow_tabs (HTML_CLUEFLOW (self->parent), p) || line_offset == -1
		? html_text_text_line_length (HTML_TEXT (self)->text, line_offset, HTML_TEXT (self)->text_len)
		: HTML_TEXT (self)->text_len;
}

gint
html_text_get_line_offset (HTMLText *text, HTMLPainter *painter)
{
	return html_clueflow_tabs (HTML_CLUEFLOW (HTML_OBJECT (text)->parent), painter)
		? html_clueflow_get_line_offset (HTML_CLUEFLOW (HTML_OBJECT (text)->parent), 
						 painter, HTML_OBJECT (text))
		: -1;
}

static guint
get_words (const gchar *s)
{
	guint words = 1;

	while ((s = strchr (s, ' '))) {
		words ++;
		s ++;
	}

	return words;
}

static void
calc_word_width (HTMLText *text, HTMLPainter *painter, gint line_offset)
{
	GtkHTMLFontStyle style;
	HTMLFont *font;
	gchar *begin, *end;
	gint i;

	text->words      = get_words (text->text);
	if (text->word_width)
		g_free (text->word_width);
	text->word_width = g_new (guint, text->words);
	style = html_text_get_font_style (text);
	font = html_painter_get_html_font (painter, text->face, style);

	begin            = text->text;
	for (i = 0; i < text->words; i++) {
		end   = strchr (begin + (i ? 1 : 0), ' ');
		text->word_width [i] = (i ? text->word_width [i - 1] : 0)
			+ html_painter_calc_text_width_bytes (painter,
							      begin, end ? end - begin : strlen (begin),
							      &line_offset, font, style);
		begin = end;
	}

	HTML_OBJECT (text)->change &= ~HTML_CHANGE_WORD_WIDTH;
}

void
html_text_request_word_width (HTMLText *text, HTMLPainter *painter)
{
	if (!text->word_width || (HTML_OBJECT (text)->change & HTML_CHANGE_WORD_WIDTH)) {
		gint offset;

		offset = html_text_get_line_offset (text, painter);
		calc_word_width (text, painter, offset);
	}
}

static gint
calc_preferred_width (HTMLObject *self,
		      HTMLPainter *painter)
{
	HTMLText *text;

	text = HTML_TEXT (self);

	html_text_request_word_width (text, painter);

	return MAX (1, text->word_width [text->words - 1]);
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

	if (o->flags & HTML_OBJECT_FLAG_NEWLINE)
		return HTML_FIT_COMPLETE;

	remove_text_slaves (o);

	/* Turn all text over to our slaves */
	text_slave = html_text_slave_new (text, 0, HTML_TEXT (text)->text_len, 0);
	html_clue_append_after (HTML_CLUE (o->parent), text_slave, o);

	return HTML_FIT_COMPLETE;
}

static gint
forward_get_nb_width (HTMLText *text, HTMLPainter *painter, gboolean begin)
{
	HTMLObject *obj;

	g_assert (text);
	g_assert (html_object_is_text (HTML_OBJECT (text)));
	g_assert (text->text_len == 0);

	/* find prev/next object */
	obj = begin
		? html_object_prev_not_slave (HTML_OBJECT (text))
		: html_object_next_not_slave (HTML_OBJECT (text));

	/* if not found or not text return 0, otherwise forward get_nb_with there */
	if (!obj || !html_object_is_text (obj))
		return 0;
	else
		return html_text_get_nb_width (HTML_TEXT (obj), painter, begin);
}

static gint
get_next_nb_width (HTMLText *text, HTMLPainter *painter, gboolean begin)
{
	HTMLObject *obj;

	g_assert (text);
	g_assert (html_object_is_text (HTML_OBJECT (text)));
	g_assert (text->words == 1);

	/* find prev/next object */
	obj = begin
		? html_object_next_not_slave (HTML_OBJECT (text))
		: html_object_prev_not_slave (HTML_OBJECT (text));

	/* if not found or not text return 0, otherwise forward get_nb_with there */
	if (!obj || !html_object_is_text (obj))
		return 0;
	else
		return html_text_get_nb_width (HTML_TEXT (obj), painter, begin);
}

static guint
word_width (HTMLText *text, HTMLPainter *p, guint i)
{
	g_assert (i < text->words);

	return text->word_width [i]
		- (i > 0 ? text->word_width [i - 1]
		   + html_painter_get_space_width (p, html_text_get_font_style (text), text->face) : 0);
}

/* return non-breakable text width on begin/end of this text */
gint
html_text_get_nb_width (HTMLText *text, HTMLPainter *painter, gboolean begin)
{
	/* handle "" case */
	if (text->text_len == 0)
		return forward_get_nb_width (text, painter, begin);

	/* if begins/ends with ' ' the width is 0 */
	if ((begin && html_text_get_char (text, 0) == ' ')
	    || (!begin && html_text_get_char (text, text->text_len - 1) == ' '))
		return 0;

	html_text_request_word_width (text, painter);

	return word_width (text, painter, begin ? 0 : text->words - 1)
		+ (text->words == 1 ? get_next_nb_width (text, painter, begin) : 0);
}

static gint
calc_min_width (HTMLObject *self, HTMLPainter *painter)
{
	HTMLText *text = HTML_TEXT (self);
	HTMLObject *obj;
	guint i, w, mw;

	html_text_request_word_width (text, painter);
	mw = 0;

	for (i = 0; i < text->words; i++) {
		w = word_width (text, painter, i);
		if (i == 0) {
			obj = html_object_prev_not_slave (self);
			if (obj && html_object_is_text (obj))
				w += html_text_get_nb_width (HTML_TEXT (obj), painter, FALSE);
		} else if (i == text->words - 1) {
			obj = html_object_next_not_slave (self);
			if (obj && html_object_is_text (obj))
				w += html_text_get_nb_width (HTML_TEXT (obj), painter, TRUE);
		}
		if (w > mw)
			mw = w;
	}

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
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	gchar *opening_tags;
	gchar *closing_tags;
	HTMLText *text;

	text = HTML_TEXT (self);

	get_tags (text, state, &opening_tags, &closing_tags);

	if (! html_engine_save_output_string (state, "%s", opening_tags)) {
		g_free (opening_tags);
		g_free (closing_tags);
		return FALSE;
	}
	g_free (opening_tags);

	if (! html_engine_save_encode (state, text->text, text->text_len)) {
		g_free (closing_tags);
		return FALSE;
	}

	if (! html_engine_save_output_string (state, "%s", closing_tags)) {
		g_free (closing_tags);
		return FALSE;
	}

	g_free (closing_tags);

	return TRUE;
}

static gboolean
save_plain (HTMLObject *self,
	    HTMLEngineSaveState *state,
	    gint requested_width)
{
	HTMLText *text;
	gboolean rv;

	text = HTML_TEXT (self);

	rv  = html_engine_save_output_string (state, "%s", text->text);
	
	return rv;
}

static guint
get_length (HTMLObject *self)
{
	return HTML_TEXT (self)->text_len;
}

/* #define DEBUG_NBSP */

static gboolean
check_last_white (gboolean rv, gint white_space, gunichar last_white, gint *delta_out)
{
	if (white_space > 0 && last_white == ENTITY_NBSP) {
		(*delta_out) --;
		rv = TRUE;
	}

	return rv;
}

static gboolean
check_prev_white (gboolean rv, gint white_space, gunichar last_white, gint *delta_out)
{
	if (white_space > 0 && last_white == ' ') {
		(*delta_out) ++;
		rv = TRUE;
	}

	return rv;
}

static gboolean
is_convert_nbsp_needed (const gchar *s, gint *delta_out)
{
	gunichar uc, last_white = 0;
	gboolean rv = FALSE;
	gint white_space;
	const gchar *p, *op;

	*delta_out = 0;

	op = p = s;
	white_space = 0;
	while (*p && (p = e_unicode_get_utf8 (p, &uc))) {
		if (uc == ENTITY_NBSP || uc == ' ') {
			rv = check_prev_white (rv, white_space, last_white, delta_out);
			white_space ++;
			last_white = uc;
		} else {
			rv = check_last_white (rv, white_space, last_white, delta_out);
			white_space = 0;
		}
		op = p;
	}
	rv = check_last_white (rv, white_space, last_white, delta_out);

	return rv;
}

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

static void
convert_nbsp (gchar *fill, const gchar *p)
{
	gint white_space;
	gunichar uc;
	const gchar *op;

#ifdef DEBUG_NBSP
	printf ("convert_nbsp: %s --> \"", p);
#endif
	op = p;
	white_space = 0;

	while (*p && (p = e_unicode_get_utf8 (p, &uc))) {
		if (uc == ENTITY_NBSP || uc == ' ') {
			write_prev_white_space (white_space, &fill);
			white_space ++;
		} else {
			write_last_white_space (white_space, &fill);
			white_space = 0;
#ifdef DEBUG_NBSP
			printf ("*");
#endif
			strncpy (fill, op, p - op);
			fill += p - op;
		}
		op = p;
	}

	write_last_white_space (white_space, &fill);
	*fill = 0;

#ifdef DEBUG_NBSP
	printf ("\"\n");
#endif
}

gboolean
html_text_convert_nbsp (HTMLText *text, gboolean free_text)
{
	gint delta;
	gchar *to_free;

	if (is_convert_nbsp_needed (text->text, &delta)) {
		html_text_clear_word_width (text);
		to_free    = text->text;
		text->text = g_malloc (strlen (to_free) + delta + 1);
		convert_nbsp (text->text, to_free);
		if (free_text)
			g_free (to_free);
		return TRUE;
	}
	return FALSE;
}

static void 
move_spell_errors (GList *spell_errors, guint offset, gint delta) 
{ 
	SpellError *se; 

	if (!delta) return;

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

static HTMLColor *
get_color (HTMLText *text,
	   HTMLPainter *painter)
{
	return text->color;
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
set_color (HTMLText *text,
	   HTMLEngine *engine,
	   HTMLColor *color)
{
	if (html_color_equal (text->color, color))
		return;

	html_color_unref (text->color);
	html_color_ref (color);
	text->color = color;

	if (engine != NULL) {
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
	g_free (text->word_width);
	g_free (text->face);

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

	if (offset != text->select_start || length != text->select_length)
		changed = TRUE;
	else
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
	HTMLText *text = HTML_TEXT (self);

	return url ? html_link_text_new_with_len (text->text, text->text_len, text->font_style, color, url, target) : NULL;
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

	/* FIXME: we need a `g_string_append()' that takes the number of
           characters to append as an extra parameter.  */

	p    = html_text_get_text (text, text->select_start);
	last = g_utf8_offset_to_pointer (p, text->select_length);
	
	/* OPTIMIZED
	last = html_text_get_text (text,
				   text->select_start + text->select_length);
	*/
	for (; p < last;) {
		g_string_append_c (buffer, *p);
		p++;
	}
}

static void
get_cursor (HTMLObject *self,
	    HTMLPainter *painter,
	    guint offset,
	    gint *x1, gint *y1,
	    gint *x2, gint *y2)
{
	HTMLObject *slave, *next;
	guint ascent, descent;

	next = html_object_next_not_slave (self);
	if (offset == HTML_TEXT (self)->text_len && next && html_object_is_text (next) && HTML_TEXT (next)->text [0] != ' ') {
		html_object_get_cursor (next, painter, 0, x1, y1, x2, y2);
		return;
	}

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
	HTMLObject *obj, *next;

	next = html_object_next_not_slave (self);
	if (offset == HTML_TEXT (self)->text_len && next && html_object_is_text (next) && HTML_TEXT (next)->text [0] != ' ') {
		html_object_get_cursor_base (next, painter, 0, x, y);
		return;
	}

	for (obj = self->next; obj != NULL; obj = obj->next) {
		HTMLTextSlave *slave;

		if (HTML_OBJECT_TYPE (obj) != HTML_TYPE_TEXTSLAVE)
			break;

		slave = HTML_TEXT_SLAVE (obj);

		if (offset <= slave->posStart + slave->posLen
		    || obj->next == NULL
		    || HTML_OBJECT_TYPE (obj->next) != HTML_TYPE_TEXTSLAVE) {
			html_object_calc_abs_position (obj, x, y);
			if (offset > slave->posStart) {
				HTMLText *text;
				GtkHTMLFontStyle font_style;
				gint line_offset;

				text = HTML_TEXT (self);

				font_style = html_text_get_font_style (text);
				line_offset = html_text_slave_get_line_offset (slave,
									       html_text_get_line_offset (HTML_TEXT (self),
													  painter),
									       slave->posStart, painter);
				*x += html_painter_calc_text_width (painter,
								    html_text_get_text (text, slave->posStart),
								    offset - slave->posStart, &line_offset,
								    font_style, text->face);
			}

			return;
		}
	}

	g_warning ("Getting cursor base for an HTMLText with no slaves -- %p\n",
		   self);
	html_object_calc_abs_position (self, x, y);
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
	object_class->calc_size = calc_size;
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

	/* HTMLText methods.  */

	klass->queue_draw = queue_draw;
	klass->get_font_style = get_font_style;
	klass->get_color = get_color;
	klass->set_font_style = set_font_style;
	klass->set_color = set_color;

	parent_class = &html_object_class;
}

static gint
text_len (const gchar *str, gint len)
{
	if (len == -1) {
		return g_utf8_validate (str, -1, NULL) ? g_utf8_strlen (str, -1) : 0;
	}

	return len;
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

	text->text          = len == -1 ? g_strdup (str) : g_strndup (str, g_utf8_offset_to_pointer (str, len) - str);
	text->text_len      = text_len (str, len);
	text->font_style    = font_style;
	text->face          = NULL;
	text->color         = color;
	text->spell_errors  = NULL;
	text->select_start  = 0;
	text->select_length = 0;
	text->word_width    = NULL;
	text->words         = 0;

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

HTMLColor *
html_text_get_color (HTMLText *text,
		     HTMLPainter *painter)
{
	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (painter != NULL, NULL);

	return (* HT_CLASS (text)->get_color) (text, painter);
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
html_text_set_color (HTMLText *text,
		     HTMLEngine *engine,
		     HTMLColor *color)
{
	g_return_if_fail (text != NULL);
	g_return_if_fail (color != NULL);

	(* HT_CLASS (text)->set_color) (text, engine, color);
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
	text->text = g_strdup (new_text);
	text->text_len = text_len (new_text, -1);
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
	HTMLObject *new_obj;
	gchar *href;
	gchar *base;
	gint offset;
	guint position;

	base = g_strndup (html_text_get_text (text, so), html_text_get_index (text, eo) - html_text_get_index (text, so));
	href = (prefix) ? g_strconcat (prefix, base, NULL) : g_strdup (base);
	g_free (base);

	new_obj = html_link_text_new_with_len
		(html_text_get_text (text, so),
		 eo - so,
		 text->font_style,
		 html_colorset_get_color (engine->settings->color_set, HTMLLinkColor),
		 href, NULL);

	offset   = HTML_OBJECT (text) == engine->cursor->object ? engine->cursor->offset : 0;
	position = engine->cursor->position;

	html_cursor_jump_to_position (engine->cursor, engine, position + so - offset);
	html_engine_set_mark (engine);
	html_cursor_jump_to_position (engine->cursor, engine, position + eo - offset);

	html_engine_paste_object (engine, new_obj, eo - so);

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
	do {
		cur = g_utf8_next_char (cur);
		if (cur && *cur && g_utf8_get_char (cur) >= 0x80)
			exec = FALSE;
	} while (exec && cur && *cur && uc != ' ' && uc != ENTITY_NBSP);

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
		while (offset < text->text_len && !rv) {
			for (i=0; i<MIM_N; i++) {
				if (mim [i].preg && !regexec (mim [i].preg, str, 2, pmatch, 0)) {
					paste_link (engine, text,
						    g_utf8_pointer_to_offset (text->text, str + pmatch [0].rm_so),
						    g_utf8_pointer_to_offset (text->text, str + pmatch [0].rm_eo), mim [i].prefix);
					rv = TRUE;
					break;
				}
			}
			str = g_utf8_next_char (str);
			offset++;
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
	text->text      = g_strconcat (to_delete, str, NULL);
	text->text_len += text_len (str, len);

	g_free (to_delete);

	html_object_change_set (HTML_OBJECT (text), HTML_CHANGE_ALL);
}
