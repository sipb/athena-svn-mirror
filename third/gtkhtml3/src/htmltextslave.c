/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
   Copyright (C) 1999 Helix Code, Inc.
   
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

#include "htmltextslave.h"
#include "htmlclue.h"
#include "htmlclueflow.h"
#include "htmlcursor.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlpainter.h"
#include "htmlplainpainter.h"
#include "htmlgdkpainter.h"


/* #define HTML_TEXT_SLAVE_DEBUG */

HTMLTextSlaveClass html_text_slave_class;
static HTMLObjectClass *parent_class = NULL;

static GList * get_items (HTMLTextSlave *slave, HTMLPainter *painter);
static GList * get_glyphs (HTMLTextSlave *slave, HTMLPainter *painter);
static GList * get_glyphs_part (HTMLTextSlave *slave, HTMLPainter *painter, guint offset, guint len, GList **items);

char *
html_text_slave_get_text (HTMLTextSlave *slave)
{
	if (!slave->charStart)
		slave->charStart = html_text_get_text (slave->owner, slave->posStart);

	return slave->charStart;
}

/* Split this TextSlave at the specified offset.  */
static void
split (HTMLTextSlave *slave, guint offset, guint start_word, char *start_pointer)
{
	HTMLObject *obj;
	HTMLObject *new;

	g_return_if_fail (offset >= 0);
	g_return_if_fail (offset < slave->posLen);

	obj = HTML_OBJECT (slave);

	new = html_text_slave_new (slave->owner,
				   slave->posStart + offset,
				   slave->posLen - offset, start_word);

	HTML_TEXT_SLAVE (new)->charStart = start_pointer;

	html_clue_append_after (HTML_CLUE (obj->parent), new, obj);

	slave->posLen = offset;
}


/* HTMLObject methods.  */

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	/* FIXME it does not make much sense to me to share the owner.  */
	HTML_TEXT_SLAVE (dest)->owner = HTML_TEXT_SLAVE (self)->owner;
	HTML_TEXT_SLAVE (dest)->posStart = HTML_TEXT_SLAVE (self)->posStart;
	HTML_TEXT_SLAVE (dest)->posLen = HTML_TEXT_SLAVE (self)->posLen;
}

static guint
get_words_width (HTMLTextSlave *slave, HTMLPainter *p, guint words)
{
	HTMLText *text = slave->owner;
	gint width;

	if (words <= 0)
		return 0;

	width =  text->word_width [slave->start_word + words - 1]
		- (slave->start_word ? text->word_width [slave->start_word - 1]
		   + html_painter_get_space_width (p, html_text_get_font_style (text), text->face) : 0);

	if (html_clueflow_tabs (HTML_CLUEFLOW (HTML_OBJECT (slave)->parent), p)) {
		gchar *space, *str = html_text_slave_get_text (slave);
		gint line_offset = html_text_slave_get_line_offset (slave, 0, p);
		gint tabs, len = 0; 

		space = str;
		while (words && *space && len < slave->posLen) {
			if (*space == ' ')
				words --;

			if (words) {
				space = g_utf8_next_char (space);
				len++;
			}
		}

		/* printf ("width %d --> ", width); */
		width += html_painter_get_space_width (p, html_text_get_font_style (text), text->face)*(html_text_text_line_length (str, &line_offset, len, &tabs) - len);
		/* printf ("%d\n", width); */
	}

	return width;
}

static inline gint
html_text_slave_get_start_byte_offset (HTMLTextSlave *slave)
{
	return html_text_slave_get_text (slave) - slave->owner->text;
}

static guint
calc_width (HTMLTextSlave *slave, HTMLPainter *painter, gint *asc, gint *dsc)
{
	HTMLText *text = slave->owner;
	HTMLObject *next, *prev;
	gint line_offset, tabs = 0, width = 0;

	line_offset = html_text_slave_get_line_offset (slave, 0, painter);
	if (line_offset != -1)
		width += (html_text_text_line_length (html_text_slave_get_text (slave), &line_offset, slave->posLen, &tabs) - slave->posLen)*
			html_painter_get_space_width (painter, html_text_get_font_style (text), text->face);

	html_text_request_word_width (text, painter);
	if (slave->posStart == 0 && slave->posLen == text->text_len) {
		*asc = HTML_OBJECT (text)->ascent;
		*dsc = HTML_OBJECT (text)->descent;

		width += text->word_width [text->words - 1];
	} else {
		next = HTML_OBJECT (slave)->next;
		prev = HTML_OBJECT (slave)->prev;
		if ((prev && HTML_OBJECT_TYPE (prev) == HTML_TYPE_TEXTSLAVE
		     && slave->start_word == HTML_TEXT_SLAVE (prev)->start_word)
		    || (next && HTML_OBJECT_TYPE (next) == HTML_TYPE_TEXTSLAVE
			&& slave->start_word == HTML_TEXT_SLAVE (next)->start_word)) {
			gint line_offset = -1;
			gint w;

			html_painter_calc_text_size (painter, html_text_slave_get_text (slave), slave->posLen,
						     get_items (slave, painter), get_glyphs (slave, painter),
						     html_text_slave_get_start_byte_offset (slave),
						     &line_offset, html_text_get_font_style (text),
						     text->face, &w, asc, dsc);
			width += w + tabs*html_painter_get_space_width (painter, html_text_get_font_style (text), text->face);
		} else {
			width += get_words_width (slave, painter,
						  (next && HTML_OBJECT_TYPE (next) == HTML_TYPE_TEXTSLAVE
						   ? HTML_TEXT_SLAVE (next)->start_word : text->words) - slave->start_word);
			*asc = HTML_OBJECT (text)->ascent;
			*dsc = HTML_OBJECT (text)->descent;
		}
	}

	return width;
}

inline static void
glyphs_destroy (GList *glyphs)
{
	GList *l;

	for (l = glyphs; l; l = l->next)
		pango_glyph_string_free ((PangoGlyphString *) l->data);
	g_list_free (glyphs);
}

static gint
get_offset_for_bounded_width (HTMLTextSlave *slave, HTMLPainter *painter, gint *words, gint max_width)
{
	HTMLText *text = slave->owner;
	gint upper = slave->posLen;
	gint lower = 0;
	gint off = 0;
	gint len, width, asc, dsc;
	gint line_offset = -1;
	gchar *sep, *str;
	char *buffer = html_text_slave_get_text (slave);

	len = (lower + upper) / 2;
	html_painter_calc_text_size (painter, buffer, len, get_items (slave, painter), get_glyphs (slave, painter), html_text_slave_get_start_byte_offset (slave),
				     &line_offset, html_text_get_font_style (text), text->face, &width, &asc, &dsc);
	while (lower < upper) {
		if (width > max_width)
			upper = len - 1;
		else
			lower = len + 1;
		len = (lower + upper) / 2;
		line_offset = -1;
		html_painter_calc_text_size (painter, buffer, len, get_items (slave, painter), get_glyphs (slave, painter), html_text_slave_get_start_byte_offset (slave),
					     &line_offset, html_text_get_font_style (text), text->face, &width, &asc, &dsc);
	}

	if (width > max_width && len > 1)
		len --;

	*words = 0;
	str = sep = buffer;

	while (off < len && *sep) {
		if (*sep == ' ') 
			(*words) ++;

		sep = g_utf8_next_char (sep);
		off++;
	}

	return len;
}

static void
slave_split_if_too_long (HTMLTextSlave *slave, HTMLPainter *painter, gint *width, gint *asc, gint *dsc)
{
	gint x, y;

	html_object_calc_abs_position (HTML_OBJECT (slave), &x, &y);

	if (*width + x > MAX_WIDGET_WIDTH && slave->posLen > 1) {
		gint words, pos;

		pos = get_offset_for_bounded_width (slave, painter, &words, MAX_WIDGET_WIDTH - x);
		if (pos > 0 && pos < slave->posLen) {
			split (slave, pos, slave->start_word + words, NULL);
			*width = MAX (1, calc_width (slave, painter, asc, dsc));
		}
	}
}

static gboolean
calc_size (HTMLObject *self, HTMLPainter *painter, GList **changed_objs)
{
	HTMLText *owner;
	HTMLTextSlave *slave;
	HTMLObject *next;
	GtkHTMLFontStyle font_style;
	gint new_ascent, new_descent, new_width;
	gboolean changed;

	slave = HTML_TEXT_SLAVE (self);
	owner = HTML_TEXT (slave->owner);
	font_style = html_text_get_font_style (owner);

	new_width = MAX (1, calc_width (slave, painter, &new_ascent, &new_descent));

	next = HTML_OBJECT (slave)->next;
	if ((slave->start_word == owner->words - 1
	     && (!next || (HTML_IS_TEXT_SLAVE (next) && HTML_TEXT_SLAVE (next)->start_word == slave->start_word + 1)))
	    && (HTML_IS_PLAIN_PAINTER (painter) || HTML_IS_GDK_PAINTER (painter))
	    && new_width > HTML_OBJECT (owner)->max_width)
		slave_split_if_too_long (slave, painter, &new_width, &new_ascent, &new_descent);

	/* handle sub & super script */
	if (font_style & GTK_HTML_FONT_STYLE_SUBSCRIPT || font_style & GTK_HTML_FONT_STYLE_SUPERSCRIPT) {
		gint shift = (new_ascent + new_descent) >> 1;

		if (font_style & GTK_HTML_FONT_STYLE_SUBSCRIPT) {
			new_descent += shift;
			new_ascent  -= shift;
		} else {
			new_descent -= shift;
			new_ascent  += shift;
		}
	}

	changed = FALSE;

	if (new_ascent != self->ascent) {
		self->ascent = new_ascent;
		changed = TRUE;
	}

	if (new_descent != self->descent) {
		self->descent = new_descent;
		changed = TRUE;
	}

	if (new_width != self->width) {
		self->width = new_width;
		changed = TRUE;
	}

	return changed;
}

#ifdef HTML_TEXT_SLAVE_DEBUG
static void
debug_print (HTMLFitType rv, gchar *text, gint len)
{
	gint i;

	printf ("Split text");
	switch (rv) {
	case HTML_FIT_PARTIAL:
		printf (" (Partial): `");
		break;
	case HTML_FIT_NONE:
		printf (" (NoFit): `");
		break;
	case HTML_FIT_COMPLETE:
		printf (" (Complete): `");
		break;
	}

	for (i = 0; i < len; i++)
		putchar (text [i]);

	printf ("'\n");
}
#endif

gint
html_text_slave_get_line_offset (HTMLTextSlave *slave, gint offset, HTMLPainter *p)
{
	HTMLObject *head = HTML_OBJECT (slave->owner)->next;

	g_assert (HTML_IS_TEXT_SLAVE (head));

	if (!html_clueflow_tabs (HTML_CLUEFLOW (HTML_OBJECT (slave)->parent), p))
		return -1;

	if (head->y + head->descent - 1 < HTML_OBJECT (slave)->y - HTML_OBJECT (slave)->ascent) {
		HTMLObject *prev;
		HTMLTextSlave *bol;
		gint line_offset = 0;

		prev = html_object_prev (HTML_OBJECT (slave)->parent, HTML_OBJECT (slave));
		while (prev->y + prev->descent - 1 >= HTML_OBJECT (slave)->y - HTML_OBJECT (slave)->ascent)
			prev = html_object_prev (HTML_OBJECT (slave)->parent, HTML_OBJECT (prev));

		bol = HTML_TEXT_SLAVE (prev->next);
		return html_text_text_line_length (html_text_slave_get_text (bol),
						   &line_offset, slave->posStart + offset - bol->posStart, NULL);
	} else {
		gint line_offset = html_text_get_line_offset (slave->owner, p);
		/* printf ("owner lo %d\n", line_offset); */
		html_text_text_line_length (slave->owner->text, &line_offset, slave->posStart + offset, NULL);
		return line_offset;
	}
}

static gint
get_next_nb_width (HTMLTextSlave *slave, HTMLPainter *painter)
{
	
	gint width = 0;

	if (HTML_TEXT (slave->owner)->text_len == 0
	    || html_text_get_char (HTML_TEXT (slave->owner), slave->posStart + slave->posLen - 1) != ' ') {
		HTMLObject *obj;
		obj = html_object_next_not_slave (HTML_OBJECT (slave));
		if (obj && html_object_is_text (obj)
		    && html_text_get_char (HTML_TEXT (obj), 0) != ' ')
			width = html_text_get_nb_width (HTML_TEXT (obj), painter, TRUE);
	}

	return width;
}

static gboolean
could_remove_leading_space (HTMLTextSlave *slave, gboolean lineBegin)
{
	HTMLObject *o = HTML_OBJECT (slave->owner);

	if (lineBegin && (HTML_OBJECT (slave)->prev != o || o->prev))
		return TRUE;

	if (!o->prev)
		return FALSE;

	while (o->prev && HTML_OBJECT_TYPE (o->prev) == HTML_TYPE_CLUEALIGNED)
		o = o->prev;

	return o->prev ? FALSE : TRUE;
}

gint
html_text_slave_nb_width (HTMLTextSlave *slave, HTMLPainter *painter, gint words)
{
	return get_words_width (slave, painter, words)
		+ (slave->start_word + words == slave->owner->words ? get_next_nb_width (slave, painter) : 0);
}

gchar *
html_text_slave_remove_leading_space (HTMLTextSlave *slave, HTMLPainter *painter, gboolean lineBegin)
{
	gchar *begin;

	html_text_request_word_width (slave->owner, painter);

	begin = html_text_slave_get_text (slave);
	if (*begin == ' ' && could_remove_leading_space (slave, lineBegin)) {
		if (slave->posStart == 0)
			slave->start_word ++;
		begin = g_utf8_next_char (begin);
		slave->charStart = begin;
		slave->posStart ++;
		slave->posLen --;
	}

	return begin;
}

gint
html_text_slave_get_nb_width (HTMLTextSlave *slave, HTMLPainter *painter, gboolean lineBegin)
{
	html_text_slave_remove_leading_space (slave, painter, lineBegin);
	if (slave->owner->words - slave->start_word > 1)
		return html_text_slave_nb_width (slave, painter, 1);

	return html_object_calc_min_width (HTML_OBJECT (slave), painter);
}

static HTMLFitType
hts_fit_line (HTMLObject *o, HTMLPainter *painter,
	      gboolean lineBegin, gboolean firstRun, gboolean next_to_floating, gint widthLeft)
{
	HTMLFitType rv = HTML_FIT_PARTIAL;
	HTMLTextSlave *slave;
	HTMLText *text;
	HTMLObject *prev;
	guint  pos = 0;
	gchar *sep = NULL;
	gchar *begin;
	guint words = 0;
	gint orig_start_word;
	gboolean forceFit;

	slave = HTML_TEXT_SLAVE (o);
	text  = HTML_TEXT (slave->owner);
	orig_start_word = slave->start_word;

	begin = html_text_slave_remove_leading_space (slave, painter, lineBegin);

	/* printf ("fit_line %d left: %d lspacetext: \"%s\"\n", firstRun, widthLeft, begin); */

	prev = html_object_prev_not_slave (HTML_OBJECT (text));
	forceFit = orig_start_word == slave->start_word
		&& prev && html_object_is_text (prev) && HTML_TEXT (prev)->text_len && HTML_TEXT (prev)->text [strlen (HTML_TEXT (prev)->text) - 1] != ' ';

	sep = begin;
	while ((sep && *sep
		&& widthLeft >= html_text_slave_nb_width (slave, painter, words + 1))
	       || (words == 0 && text->words - slave->start_word > 0 && forceFit)) {
		if (words) {
			sep = g_utf8_next_char (sep);
			pos++;
		}
		
		words ++;
		while (*sep && *sep != ' ') {
			sep = g_utf8_next_char (sep);
			pos++;
		}

		if (words + slave->start_word >= text->words)
			break;
	}

	if (words + slave->start_word == text->words)
		rv = HTML_FIT_COMPLETE;
	else if (words == 0 || get_words_width (slave, painter, words) == 0) {
		if (!firstRun) {
			if (slave->posStart == 0 && text->text [0] != ' ' && HTML_OBJECT (text)->prev) {
				HTMLObject *prev = HTML_OBJECT (text)->prev;
				if (HTML_IS_TEXT_SLAVE (prev) && HTML_TEXT_SLAVE (prev)->posLen
				    && HTML_TEXT_SLAVE (prev)->owner->text [strlen (HTML_TEXT_SLAVE (prev)->owner->text) - 1]
				    != ' ')
					rv = slave->start_word + 1 == text->words ? HTML_FIT_COMPLETE : HTML_FIT_PARTIAL;
				else
					rv = HTML_FIT_NONE;
			} else
				rv = HTML_FIT_NONE;
		} else if (slave->start_word + 1 == text->words)
			rv = next_to_floating ? HTML_FIT_NONE : HTML_FIT_COMPLETE;
		else {
			if (words && *sep) {
				sep = g_utf8_next_char (sep);
				pos++;
			}

			words ++;

			while (*sep && *sep != ' ') {
				sep = g_utf8_next_char (sep);
				pos ++;
			}
		}
	}

	if (rv == HTML_FIT_PARTIAL)
		if (pos < slave->posLen) {
			split (slave, pos, slave->start_word + words, *sep ? sep : NULL);
		o->width = get_words_width (slave, painter, words);
	}

#ifdef HTML_TEXT_SLAVE_DEBUG
	debug_print (rv, html_text_slave_get_text (slave), slave->posLen);
#endif

	return rv;	
}

static gboolean
select_range (HTMLObject *self,
	      HTMLEngine *engine,
	      guint start, gint length,
	      gboolean queue_draw)
{
	return FALSE;
}

static guint
get_length (HTMLObject *self)
{
	return 0;
}


/* HTMLObject::draw() implementation.  */

static gint
get_ys (HTMLText *text, HTMLPainter *p)
{
	if (text->font_style & GTK_HTML_FONT_STYLE_SUBSCRIPT || text->font_style & GTK_HTML_FONT_STYLE_SUPERSCRIPT) {
		gint height2;

		height2 = (HTML_OBJECT (text)->ascent + HTML_OBJECT (text)->descent) / 2;
		/* FIX2? (html_painter_calc_ascent (p, text->font_style, text->face)
		   + html_painter_calc_descent (p, text->font_style, text->face)) >> 1; */
		return (text->font_style & GTK_HTML_FONT_STYLE_SUBSCRIPT) ? height2 : -height2;
			
	} else 
		return 0;
}

static void
draw_spell_errors (HTMLTextSlave *slave, HTMLPainter *p, gint tx, gint ty)
{
	GList *cur = HTML_TEXT (slave->owner)->spell_errors;
	HTMLObject *obj = HTML_OBJECT (slave);
	SpellError *se;
	guint ma, mi;
	gint x_off = 0;
	gint last_off = 0;
	gint line_offset = html_text_slave_get_line_offset (slave, 0, p);
	gchar *text = html_text_slave_get_text (slave);

	while (cur) {

		se = (SpellError *) cur->data;
		ma = MAX (se->off, slave->posStart);
		mi = MIN (se->off + se->len, slave->posStart + slave->posLen);
		if (ma < mi) {
			GList *items, *glyphs;
			guint off = ma - slave->posStart;
			guint len = mi - ma;
			gint lo, width, asc, dsc;

			html_painter_set_pen (p, &html_colorset_get_color_allocated (p, HTMLSpellErrorColor)->color);
			/* printf ("spell error: %s\n", html_text_get_text (slave->owner, off)); */
			lo = line_offset;
			
			glyphs = get_glyphs_part (slave, p, last_off, off - last_off, &items);
			html_painter_calc_text_size (p, text,
						     off - last_off, items, glyphs, text - slave->owner->text,
						     &line_offset,
						     p->font_style,
						     p->font_face, &width, &asc, &dsc);
			glyphs_destroy (glyphs);
			x_off += width;
			text = g_utf8_offset_to_pointer (text, off - last_off);
			glyphs = get_glyphs_part (slave, p, off, len, &items);
			x_off += html_painter_draw_spell_error (p, obj->x + tx + x_off,
								obj->y + ty + get_ys (slave->owner, p),
								text, len, items, glyphs, text - slave->owner->text);
			glyphs_destroy (glyphs);
			last_off = off + len;
			if (line_offset != -1)
				line_offset += len;
			text = g_utf8_offset_to_pointer (text, len);
		}
		if (se->off > slave->posStart + slave->posLen)
			break;
		cur = cur->next;
	}
}

static GList *
get_items (HTMLTextSlave *slave, HTMLPainter *painter)
{
	if (!slave->items) {
		PangoItem *item;
		gint start_offset = html_text_slave_get_text (slave) - slave->owner->text;

		slave->items = html_text_get_items (slave->owner, painter);
		if (slave->items) {
			item = (PangoItem *) slave->items->data;

			while (slave->items && start_offset >= item->offset + item->length) {
				slave->items = slave->items->next;
				item = (PangoItem *) slave->items->data;
			}
		}
	}

	return slave->items;
}

static inline GList *
get_glyphs_base_text (GList *glyphs, PangoItem *item, const gchar *text, gint bytes)
{
	PangoGlyphString *str;

	str = pango_glyph_string_new ();
	pango_shape (text, bytes, &item->analysis, str);
	glyphs = g_list_prepend (glyphs, str);

	return glyphs;
}

GList *
html_get_glyphs_non_tab (GList *glyphs, PangoItem *item, const gchar *text, gint bytes, gint len)
{
	gchar *tab;

	while ((tab = memchr (text, (unsigned char) '\t', bytes))) {
		gint c_bytes = tab - text;
		if (c_bytes > 0)
			glyphs = get_glyphs_base_text (glyphs, item, text, c_bytes);
		text += c_bytes + 1;
		bytes -= c_bytes + 1;
	}

	if (bytes > 0)
		glyphs = get_glyphs_base_text (glyphs, item, text, bytes);

	return glyphs;
}

static GList *
get_glyphs_part (HTMLTextSlave *slave, HTMLPainter *painter, guint offset, guint len, GList **items)
{
	GList *glyphs = NULL;

	*items = get_items (slave, painter);
	if (*items) {
		PangoItem *item;
		GList *il = *items;
		gint index, c_len;
		gint byte_offset;
		const gchar *text, *owner_text;
		gchar *end;

		owner_text = slave->owner->text;
		text = g_utf8_offset_to_pointer (html_text_slave_get_text (slave), offset);
		byte_offset = text - owner_text;

		if (offset) {
			while (il && (item = (PangoItem *) il->data) && item->offset + item->length <= byte_offset)
				il = il->next;
			*items = il;
		}

		index = 0;
		while (il && index < len) {
			item = (PangoItem *) il->data;
			c_len = MIN (item->num_chars - g_utf8_pointer_to_offset (owner_text + item->offset, text), len - index);

			end = g_utf8_offset_to_pointer (text, c_len);
			glyphs = html_get_glyphs_non_tab (glyphs, item, text, end - text, c_len);
			text = end;
			index += c_len;
			il = il->next;
		}
		glyphs = g_list_reverse (glyphs);
	}

	return glyphs;
}

static GList *
get_glyphs (HTMLTextSlave *slave, HTMLPainter *painter)
{
	if (!slave->glyphs) {
		GList *items;
		slave->glyphs = get_glyphs_part (slave, painter, 0, slave->posLen, &items);
	}

	return slave->glyphs;
}

static void
draw_normal (HTMLTextSlave *self,
	     HTMLPainter *p,
	     GtkHTMLFontStyle font_style,
	     gint x, gint y,
	     gint width, gint height,
	     gint tx, gint ty)
{
	HTMLObject *obj;
	HTMLText *text = self->owner;
	gchar *str;

	obj = HTML_OBJECT (self);

	str = html_text_slave_get_text (self);
	if (*str) {
		GList *glyphs, *items;

		html_painter_set_font_style (p, font_style);
		html_painter_set_font_face  (p, HTML_TEXT (self->owner)->face);
		html_color_alloc (HTML_TEXT (self->owner)->color, p);
		html_painter_set_pen (p, &HTML_TEXT (self->owner)->color->color);

		if (self->posStart > 0)
			glyphs = get_glyphs_part (self, p, 0, self->posLen, &items);
		else {
			items = get_items (self, p);
			glyphs = get_glyphs (self, p);
		}

		html_painter_draw_text (p, obj->x + tx, obj->y + ty + get_ys (text, p),
					str, self->posLen, items, glyphs, str - self->owner->text, html_text_slave_get_line_offset (self, 0, p));

		if (self->posStart > 0)
			glyphs_destroy (glyphs);
	}
}

static void
draw_highlighted (HTMLTextSlave *slave,
		  HTMLPainter *p,
		  GtkHTMLFontStyle font_style,
		  gint x, gint y,
		  gint width, gint height,
		  gint tx, gint ty)
{
	HTMLText *owner;
	HTMLObject *obj;
	GList *items1, *items2, *items3;
	guint start, end, len;
	gint offset_width, text_width, lo, lo_start, lo_sel, asc, dsc;
	const gchar *text;
	
	char *slave_begin;
	char *highlight_begin;

	obj = HTML_OBJECT (slave);
	owner = HTML_TEXT (slave->owner);
	start = owner->select_start;
	end = start + owner->select_length;

	text = HTML_TEXT (owner)->text;

	if (start < slave->posStart)
		start = slave->posStart;
	if (end > slave->posStart + slave->posLen)
		end = slave->posStart + slave->posLen;
	len = end - start;

	slave_begin = html_text_slave_get_text (slave);
	highlight_begin = g_utf8_offset_to_pointer (slave_begin, start - slave->posStart);

	lo_start = lo = html_text_slave_get_line_offset (slave, 0, p);

	html_painter_set_font_style (p, font_style);
	html_painter_set_font_face  (p, HTML_TEXT (owner)->face);
	html_color_alloc (HTML_TEXT (owner)->color, p);

	/* 1. Draw the leftmost non-highlighted part, if any.  */
	if (start > slave->posStart) {
		GList *glyphs1;

		glyphs1 = get_glyphs_part (slave, p, 0, start - slave->posStart, &items1);
		html_painter_calc_text_size (p, slave_begin, start - slave->posStart, items1, glyphs1, slave_begin - text, &lo,
					     font_style, HTML_TEXT (owner)->face, &offset_width, &asc, &dsc);

		html_painter_set_pen (p, &HTML_TEXT (owner)->color->color);

		if (obj->x + offset_width >= x)
			html_painter_draw_text (p,
						obj->x + tx, obj->y + ty + get_ys (HTML_TEXT (slave->owner), p),
						slave_begin,
						start - slave->posStart, items1, glyphs1, slave_begin - slave->owner->text,
						lo_start);

		if (glyphs1)
			glyphs_destroy (glyphs1);

	} else
		offset_width = 0;

	lo_sel = lo;
	
	/* Check bounds again */
	if (obj->x + offset_width > x + width)
		return;

	/* Draw the highlighted part with a highlight background.  */
	if (len) {
		GList *glyphs2;

		glyphs2 = get_glyphs_part (slave, p, start - slave->posStart, len, &items2);

		html_painter_calc_text_size (p, highlight_begin, len, items2, glyphs2, highlight_begin - text, &lo,
					     font_style, HTML_TEXT (owner)->face, &text_width, &asc, &dsc);
		/* printf ("s: %d l: %d - %d %d\n", start, len, offset_width, text_width); */

		html_painter_set_pen (p, &html_colorset_get_color_allocated
				      (p, p->focus ? HTMLHighlightColor : HTMLHighlightNFColor)->color);
		html_painter_fill_rect (p, obj->x + tx + offset_width, obj->y + ty - obj->ascent,
					text_width, obj->ascent + obj->descent);
		html_painter_set_pen (p, &html_colorset_get_color_allocated
				      (p, p->focus ? HTMLHighlightTextColor : HTMLHighlightTextNFColor)->color);
		
		if (obj->x + offset_width + text_width >= x)
			html_painter_draw_text (p, obj->x + tx + offset_width, 
						obj->y + ty + get_ys (HTML_TEXT (slave->owner), p),
						highlight_begin, len, items2, glyphs2, highlight_begin - slave->owner->text,
						lo_sel);
		if (glyphs2)
			glyphs_destroy (glyphs2);
	} else
		text_width = 0;

	/* Check bounds one last time */
	if (obj->x + offset_width + text_width > x + width)
		return;

	/* 2. Draw the rightmost non-highlighted part, if any.  */
	if (end < slave->posStart + slave->posLen) {
		gchar *end_text;
		GList *glyphs3;

		glyphs3 = get_glyphs_part (slave, p, start + len - slave->posStart, slave->posLen - start - len + slave->posStart, &items3);
		html_painter_set_pen (p, &HTML_TEXT (owner)->color->color);
		end_text = g_utf8_offset_to_pointer (highlight_begin, end - start);
		html_painter_draw_text (p,
					obj->x + tx + offset_width + text_width,
					obj->y + ty + get_ys (HTML_TEXT (slave->owner), p),
					end_text,
					slave->posStart + slave->posLen - end, items3, glyphs3, end_text - slave->owner->text, lo);
		if (glyphs3)
			glyphs_destroy (glyphs3);
	}
}

static void
draw (HTMLObject *o,
      HTMLPainter *p,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLTextSlave *textslave;
	HTMLText *owner;
	HTMLText *ownertext;
	GtkHTMLFontStyle font_style;
	guint end;
	GdkRectangle paint;

	/* printf ("slave draw %p\n", o); */

	textslave = HTML_TEXT_SLAVE (o);
	if (!html_object_intersect (o, &paint, x, y, width, height) || textslave->posLen == 0)
		return;
	
	owner = textslave->owner;
	ownertext = HTML_TEXT (owner);
	font_style = html_text_get_font_style (ownertext);

	end = textslave->posStart + textslave->posLen;
	if (owner->select_start + owner->select_length <= textslave->posStart
	    || owner->select_start >= end) {
		draw_normal (textslave, p, font_style, x, y, width, height, tx, ty);
	} else {
		draw_highlighted (textslave, p, font_style, x, y, width, height, tx, ty);
	}

	if (HTML_TEXT (textslave->owner)->spell_errors)
		draw_spell_errors (textslave, p, tx ,ty);
}

static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
	return 0;
}

static gint
calc_preferred_width (HTMLObject *o,
		      HTMLPainter *painter)
{
	return 0;
}

static const gchar *
get_url (HTMLObject *o)
{
	HTMLTextSlave *slave;

	slave = HTML_TEXT_SLAVE (o);
	return html_object_get_url (HTML_OBJECT (slave->owner));
}

static guint
get_offset_for_pointer (HTMLTextSlave *slave, HTMLPainter *painter, gint x, gint y)
{
	HTMLText *owner;
	GtkHTMLFontStyle font_style;
	guint width, prev_width;
	gint line_offset;
	gchar *text;
	guint upper;
	guint len;
	guint lower;
	GList *items;
	GList *glyphs;
	gint lo;
	gint asc, dsc;

	g_return_val_if_fail (slave != NULL, 0);

	owner = HTML_TEXT (slave->owner);
	font_style = html_text_get_font_style (owner);

	x -= HTML_OBJECT (slave)->x;

	if (x <= 0)
		return 0;

	if (slave->posLen == 1 && x > HTML_OBJECT (slave)->width / 2)
		return 1;

	if (x >= HTML_OBJECT (slave)->width)
		return slave->posLen;
				      
	len = 0;
	width = 0;
	prev_width  = 0;
	lower = 0;
	upper = slave->posLen;

	text = html_text_slave_get_text (slave);
	line_offset = html_text_slave_get_line_offset (slave, 0, painter);	


	while (upper - lower > 1) {
		lo = line_offset;
		prev_width = width;

		if (width > x)
			upper = len;
		else 
			lower = len;
	
		len = (lower + upper + 1) / 2;

		if (len) {
			glyphs = get_glyphs_part (slave, painter, 0, len, &items);
			html_painter_calc_text_size (painter, text, len, items, glyphs, html_text_slave_get_start_byte_offset (slave), 
						     &lo, font_style, owner->face, &width, &asc, &dsc);
			glyphs_destroy (glyphs);
		} else {
			width = 0;
		}
	}

	if ((width + prev_width) / 2 >= x)
		len--;

	return len;
}

static HTMLObject *
check_point (HTMLObject *self,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	if (x >= self->x
	    && x < self->x + MAX (1, self->width)
	    && y >= self->y - self->ascent
	    && y < self->y + self->descent) {
		HTMLTextSlave *slave = HTML_TEXT_SLAVE (self);

		if (offset_return != NULL)
			*offset_return = slave->posStart
				+ get_offset_for_pointer (slave, painter, x, y);

		return HTML_OBJECT (slave->owner);
	}

	return NULL;
}

static void
destroy (HTMLObject *obj)
{
	HTMLTextSlave *slave = HTML_TEXT_SLAVE (obj);

	if (slave->glyphs) {
		glyphs_destroy (slave->glyphs);
		slave->glyphs = NULL;
	}

	HTML_OBJECT_CLASS (parent_class)->destroy (obj);
}

void
html_text_slave_type_init (void)
{
	html_text_slave_class_init (&html_text_slave_class, HTML_TYPE_TEXTSLAVE, sizeof (HTMLTextSlave));
}

void
html_text_slave_class_init (HTMLTextSlaveClass *klass,
			    HTMLType type,
			    guint object_size)
{
	HTMLObjectClass *object_class;

	object_class = HTML_OBJECT_CLASS (klass);

	html_object_class_init (object_class, type, object_size);

	object_class->select_range = select_range;
	object_class->copy = copy;
	object_class->destroy = destroy;
	object_class->draw = draw;
	object_class->calc_size = calc_size;
	object_class->fit_line = hts_fit_line;
	object_class->calc_min_width = calc_min_width;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->get_url = get_url;
	object_class->get_length = get_length;
	object_class->check_point = check_point;

	parent_class = &html_object_class;
}

void
html_text_slave_init (HTMLTextSlave *slave,
		      HTMLTextSlaveClass *klass,
		      HTMLText *owner,
		      guint posStart,
		      guint posLen,
		      guint start_word)
{
	HTMLText *owner_text;
	HTMLObject *object;

	object = HTML_OBJECT (slave);
	owner_text = HTML_TEXT (owner);

	html_object_init (object, HTML_OBJECT_CLASS (klass));

	object->ascent = HTML_OBJECT (owner)->ascent;
	object->descent = HTML_OBJECT (owner)->descent;

	slave->posStart   = posStart;
	slave->posLen     = posLen;
	slave->start_word = start_word;
	slave->owner      = owner;
	slave->charStart  = NULL;
	slave->items      = NULL;
	slave->glyphs     = NULL;

	/* text slaves have always min_width 0 */
	object->min_width = 0;
	object->change   &= ~HTML_CHANGE_MIN_WIDTH;
}

HTMLObject *
html_text_slave_new (HTMLText *owner, guint posStart, guint posLen, guint start_word)
{
	HTMLTextSlave *slave;

	slave = g_new (HTMLTextSlave, 1);
	html_text_slave_init (slave, &html_text_slave_class, owner, posStart, posLen, start_word);

	return HTML_OBJECT (slave);
}
