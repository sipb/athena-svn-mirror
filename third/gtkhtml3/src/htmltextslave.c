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
#include "htmlobject.h"
#include "htmlprinter.h"
#include "htmlplainpainter.h"
#include "htmlgdkpainter.h"
#include "htmlsettings.h"
#include "gtkhtml.h"


/* #define HTML_TEXT_SLAVE_DEBUG */

HTMLTextSlaveClass html_text_slave_class;
static HTMLObjectClass *parent_class = NULL;

static GList * get_glyphs (HTMLTextSlave *slave, HTMLPainter *painter);
static GList * get_glyphs_part (HTMLTextSlave *slave, HTMLPainter *painter, guint offset, guint len);
static void    clear_glyphs (HTMLTextSlave *slave);

char *
html_text_slave_get_text (HTMLTextSlave *slave)
{
	if (!slave->charStart)
		slave->charStart = html_text_get_text (slave->owner, slave->posStart);

	return slave->charStart;
}

/* Split this TextSlave at the specified offset.  */
static void
split (HTMLTextSlave *slave, guint offset, char *start_pointer)
{
	HTMLObject *obj;
	HTMLObject *new;

	g_return_if_fail (offset >= 0);
	g_return_if_fail (offset < slave->posLen);

	obj = HTML_OBJECT (slave);

	new = html_text_slave_new (slave->owner,
				   slave->posStart + offset,
				   slave->posLen - offset);

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

static inline gint
html_text_slave_get_start_byte_offset (HTMLTextSlave *slave)
{
	return html_text_slave_get_text (slave) - slave->owner->text;
}

static guint
hts_calc_width (HTMLTextSlave *slave, HTMLPainter *painter, gint *asc, gint *dsc)
{
	HTMLText *text = slave->owner;
	gint line_offset, tabs = 0, width = 0;

	line_offset = html_text_slave_get_line_offset (slave, 0, painter);
	if (line_offset != -1)
		width += (html_text_text_line_length (html_text_slave_get_text (slave), &line_offset, slave->posLen, &tabs) - slave->posLen)*
			html_painter_get_space_width (painter, html_text_get_font_style (text), text->face);

	width += html_text_calc_part_width (text, painter, html_text_slave_get_text (slave), slave->posStart, slave->posLen, asc, dsc);

	return width;
}

inline static void
glyphs_destroy (GList *glyphs)
{
	GList *l;

	for (l = glyphs; l; l = l->next->next)
		pango_glyph_string_free ((PangoGlyphString *) l->data);
	g_list_free (glyphs);
}

static gboolean
html_text_slave_real_calc_size (HTMLObject *self, HTMLPainter *painter, GList **changed_objs)
{
	HTMLText *owner;
	HTMLTextSlave *slave;
	GtkHTMLFontStyle font_style;
	gint new_ascent, new_descent, new_width;
	gboolean changed;

	slave = HTML_TEXT_SLAVE (self);
	owner = HTML_TEXT (slave->owner);
	font_style = html_text_get_font_style (owner);

	new_width = MAX (1, hts_calc_width (slave, painter, &new_ascent, &new_descent));

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
	} else
		return html_text_get_line_offset (slave->owner, p, slave->posStart + offset);
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

gchar *
html_text_slave_remove_leading_space (HTMLTextSlave *slave, HTMLPainter *painter, gboolean lineBegin)
{
	gchar *begin;

	begin = html_text_slave_get_text (slave);
	if (*begin == ' ' && could_remove_leading_space (slave, lineBegin)) {
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

	return html_object_calc_min_width (HTML_OBJECT (slave), painter);
}

static gboolean
update_lb (HTMLTextSlave *slave, HTMLPainter *painter, gint widthLeft, gint offset, gchar *s, gint ii, gint io, gint line_offset,
	   gint *w, gint *ltw, gint *lwl, gint *lbw, gint *lbo, gchar **lbsp, gboolean *force_fit)
{
	gint new_ltw, new_lwl, aw;

	new_ltw = html_text_tail_white_space (slave->owner, painter, offset, ii, io, &new_lwl, line_offset, s);
	if (HTML_IS_GDK_PAINTER (painter) || HTML_IS_PLAIN_PAINTER (painter)) {
		aw = *w - new_ltw;
	} else {
		gint lo = html_text_get_line_offset (slave->owner, painter, *lbo);
		gint width;
				/* printf ("s: %s l: %d\n", html_text_get_text (slave->owner, lbo - lwl), offset - new_lwl - lbo + lwl); */
		html_text_calc_text_size (slave->owner, painter, html_text_get_text (slave->owner, *lbo) - slave->owner->text,
					  offset - *lbo, NULL, NULL, &lo,
					  html_text_get_font_style (slave->owner), slave->owner->face,
					  &width, NULL, NULL);
		*w += width;
		aw = *w - new_ltw;
	}
	if (aw <= widthLeft || *force_fit) {
		*ltw = new_ltw;
		*lwl = new_lwl;
		*lbw = aw;
		*lbo = offset;
		*lbsp = s;
		if (*force_fit && *lbw >= widthLeft)
			return TRUE;
		*force_fit = FALSE;
	} else
		return TRUE;

	return FALSE;
}

static HTMLFitType
hts_fit_line (HTMLObject *o, HTMLPainter *painter,
	      gboolean lineBegin, gboolean firstRun, gboolean next_to_floating, gint widthLeft)
{
	HTMLTextSlave *slave = HTML_TEXT_SLAVE (o);
	gint lbw, w, lbo, ltw, lwl, offset;
	gint ii, io, line_offset;
	gchar *s, *lbsp;
	HTMLFitType rv = HTML_FIT_NONE;
	HTMLTextPangoInfo *pi = html_text_get_pango_info (slave->owner, painter);
	gboolean force_fit = lineBegin;

	if (rv == HTML_FIT_COMPLETE)
		return rv;

	lbw = ltw = lwl = w = 0;
	offset = lbo = slave->posStart;
	ii = html_text_get_item_index (slave->owner, painter, offset, &io);

	line_offset = html_text_get_line_offset (slave->owner, painter, offset);
	lbsp = s = html_text_slave_get_text (slave);

	while ((force_fit || widthLeft > lbw) && offset < slave->posStart + slave->posLen) {
		if (offset > slave->posStart && offset > lbo && html_text_is_line_break (pi->attrs [offset]))
			if (update_lb (slave, painter, widthLeft, offset, s, ii, io, line_offset, &w, &ltw, &lwl, &lbw, &lbo, &lbsp, &force_fit))
				break;

		if (*s == '\t') {
			gint skip = 8 - (line_offset % 8);
			if (HTML_IS_GDK_PAINTER (painter) || HTML_IS_PLAIN_PAINTER (painter))
				w += skip*PANGO_PIXELS (pi->entries [ii].widths [io]);
			line_offset += skip;
		} else {
			if (HTML_IS_GDK_PAINTER (painter) || HTML_IS_PLAIN_PAINTER (painter))
				w += PANGO_PIXELS (pi->entries [ii].widths [io]);
			line_offset ++;
		}

		s = g_utf8_next_char (s);
		offset ++;

		html_text_pi_forward (pi, &ii, &io);
	}

	if (!HTML_IS_GDK_PAINTER (painter) && !HTML_IS_PLAIN_PAINTER (painter)) {
		gint aw;
		gint lo = html_text_get_line_offset (slave->owner, painter, lbo);

		/* printf ("s: %s l: %d\n", html_text_get_text (slave->owner, lbo - lwl), offset - lbo + lwl); */
		html_text_calc_text_size (slave->owner, painter, html_text_get_text (slave->owner, lbo) - slave->owner->text,
					  offset - lbo, NULL, NULL, &lo,
					  html_text_get_font_style (slave->owner), slave->owner->face,
					  &aw, NULL, NULL);
		w += aw;
	}

	if (offset == slave->posStart + slave->posLen && (widthLeft >= w || force_fit)) {
		rv = HTML_FIT_COMPLETE;
		if (slave->posLen)
			o->width = w;
	} else if (lbo > slave->posStart) {
		split (slave, lbo - slave->posStart, lbsp);
		rv = HTML_FIT_PARTIAL;
		o->width = lbw;
		slave->posLen -= lwl;
	}

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
	HTMLEngine *e;

	if (p->widget && GTK_IS_HTML (p->widget))
		e = GTK_HTML (p->widget)->engine;
	else
		return;

	while (cur) {

		se = (SpellError *) cur->data;
		ma = MAX (se->off, slave->posStart);
		mi = MIN (se->off + se->len, slave->posStart + slave->posLen);
		if (ma < mi) {
			GList *glyphs;
			guint off = ma - slave->posStart;
			guint len = mi - ma;
			gint lo, width, asc, dsc;

			html_painter_set_pen (p, &html_colorset_get_color_allocated (e->settings->color_set,
										     p, HTMLSpellErrorColor)->color);
			/* printf ("spell error: %s\n", html_text_get_text (slave->owner, off)); */
			lo = line_offset;
			
			glyphs = get_glyphs_part (slave, p, last_off, off - last_off);
			html_text_calc_text_size (slave->owner, p, text - slave->owner->text,
						  off - last_off, html_text_get_pango_info (slave->owner, p), glyphs,
						  &line_offset,
						  p->font_style,
						  p->font_face, &width, &asc, &dsc);
			glyphs_destroy (glyphs);
			x_off += width;
			text = g_utf8_offset_to_pointer (text, off - last_off);
			glyphs = get_glyphs_part (slave, p, off, len);
			x_off += html_painter_draw_spell_error (p, obj->x + tx + x_off,
								obj->y + ty + get_ys (slave->owner, p),
								text, len, html_text_get_pango_info (slave->owner, p), glyphs, text - slave->owner->text);
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

static inline GList *
get_glyphs_base_text (GList *glyphs, PangoItem *item, gint ii, const gchar *text, gint bytes)
{
	PangoGlyphString *str;

	str = pango_glyph_string_new ();
	pango_shape (text, bytes, &item->analysis, str);
	glyphs = g_list_prepend (glyphs, str);
	glyphs = g_list_prepend (glyphs, GINT_TO_POINTER (ii));

	return glyphs;
}

GList *
html_get_glyphs_non_tab (GList *glyphs, PangoItem *item, gint ii, const gchar *text, gint bytes, gint len)
{
	gchar *tab;

	while ((tab = memchr (text, (unsigned char) '\t', bytes))) {
		gint c_bytes = tab - text;
		if (c_bytes > 0)
			glyphs = get_glyphs_base_text (glyphs, item, ii, text, c_bytes);
		text += c_bytes + 1;
		bytes -= c_bytes + 1;
	}

	if (bytes > 0)
		glyphs = get_glyphs_base_text (glyphs, item, ii, text, bytes);

	return glyphs;
}

static GList *
get_glyphs_part (HTMLTextSlave *slave, HTMLPainter *painter, guint offset, guint len)
{
	GList *glyphs = NULL;
	HTMLTextPangoInfo *pi;

	pi = html_text_get_pango_info (slave->owner, painter);
	if (pi) {
		PangoItem *item;
		gint index, c_len;
		gint byte_offset, ii;
		const gchar *text, *owner_text;
		gchar *end;

		owner_text = slave->owner->text;
		text = g_utf8_offset_to_pointer (html_text_slave_get_text (slave), offset);
		byte_offset = text - owner_text;

		ii = html_text_pango_info_get_index (pi, byte_offset, 0);
		index = 0;
		while (index < len) {
			item = pi->entries [ii].item;
			c_len = MIN (item->num_chars - g_utf8_pointer_to_offset (owner_text + item->offset, text), len - index);

			end = g_utf8_offset_to_pointer (text, c_len);
			glyphs = html_get_glyphs_non_tab (glyphs, item, ii, text, end - text, c_len);
			text = end;
			index += c_len;
			ii ++;
		}
		glyphs = g_list_reverse (glyphs);
	}

	return glyphs;
}

static GList *
get_glyphs (HTMLTextSlave *slave, HTMLPainter *painter)
{
	if (!slave->glyphs || (HTML_OBJECT (slave)->change & HTML_CHANGE_RECALC_PI)) {
		clear_glyphs (slave);

		HTML_OBJECT (slave)->change &= ~HTML_CHANGE_RECALC_PI;
		slave->glyphs = get_glyphs_part (slave, painter, 0, slave->posLen);
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
		GList *glyphs;
		PangoAttrList *attrs = NULL;

		if (self->posStart > 0)
			glyphs = get_glyphs_part (self, p, 0, self->posLen);
		else
			glyphs = get_glyphs (self, p);

		if (HTML_IS_PRINTER (p)) {
			gchar *text = html_text_slave_get_text (self);
			gint start_index, end_index;

			start_index = text - self->owner->text;
			end_index = g_utf8_offset_to_pointer (text, self->posLen) - self->owner->text;
			attrs = html_text_get_attr_list (self->owner, start_index, end_index);
		}

		html_painter_draw_text (p, obj->x + tx, obj->y + ty + get_ys (text, p),
					str, self->posLen, html_text_get_pango_info (text, p), attrs, glyphs,
					str - self->owner->text, html_text_slave_get_line_offset (self, 0, p));
		if (attrs)
			pango_attr_list_unref (attrs);

		if (self->posStart > 0)
			glyphs_destroy (glyphs);
	}
}

static void
draw_focus  (HTMLPainter *painter, GdkRectangle *box)
{
	HTMLGdkPainter *p;
	GdkGCValues values;
	gchar dash [2];
	HTMLEngine *e;

	if (painter->widget && GTK_IS_HTML (painter->widget))
		e = GTK_HTML (painter->widget)->engine;
	else
		return;

	if (HTML_IS_PRINTER (painter))
		return;
	
	p = HTML_GDK_PAINTER (painter);
	/* printf ("draw_text_focus\n"); */

	gdk_gc_set_foreground (p->gc, &html_colorset_get_color_allocated (e->settings->color_set,
									  painter, HTMLTextColor)->color);
	gdk_gc_get_values (p->gc, &values);

	dash [0] = 1;
	dash [1] = 1;
	gdk_gc_set_line_attributes (p->gc, 1, GDK_LINE_ON_OFF_DASH, values.cap_style, values.join_style);
	gdk_gc_set_dashes (p->gc, 2, dash, 2);
	gdk_draw_rectangle (p->pixmap, p->gc, 0, box->x - p->x1, box->y - p->y1, box->width - 1, box->height - 1);
	gdk_gc_set_line_attributes (p->gc, 1, values.line_style, values.cap_style, values.join_style);
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
	draw_normal (textslave, p, font_style, x, y, width, height, tx, ty);
	
	if (owner->spell_errors)
		draw_spell_errors (textslave, p, tx ,ty);
	
	if (HTML_OBJECT (owner)->draw_focused) {
		GdkRectangle rect;
		Link *link = html_text_get_link_at_offset (owner, owner->focused_link_offset);

		if (link && MAX (link->start_offset, textslave->posStart) < MIN (link->end_offset, textslave->posStart + textslave->posLen)) {
			gint bw = 0;
			html_object_get_bounds (o, &rect);
			if (textslave->posStart < link->start_offset)
				bw = html_text_calc_part_width (owner, p, html_text_slave_get_text (textslave), textslave->posStart, link->start_offset - textslave->posStart, NULL, NULL);
			rect.x += tx + bw;
			rect.width -= bw;
			if (textslave->posStart + textslave->posLen > link->end_offset)
				rect.width -= html_text_calc_part_width (owner, p, owner->text + link->end_index, link->end_offset,  textslave->posStart + textslave->posLen - link->end_offset, NULL, NULL);
			rect.y += ty;
			draw_focus (p, &rect);
		}
	}
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
get_url (HTMLObject *o, gint offset)
{
	HTMLTextSlave *slave;

	slave = HTML_TEXT_SLAVE (o);
	return html_object_get_url (HTML_OBJECT (slave->owner), offset);
}

static gint
calc_offset (HTMLTextSlave *slave, HTMLPainter *painter, gint x, gint y)
{
	HTMLText *owner;
	GtkHTMLFontStyle font_style;
	gint line_offset;
	guint width, prev_width;
	gchar *text;
	guint upper;
	guint len;
	guint lower;
	GList *glyphs;
	gint lo;
	gint asc, dsc;

	g_assert (slave->posLen > 1);

	width = 0;
	prev_width  = 0;
	lower = 0;
	upper = slave->posLen;
	len = 0;

	text = html_text_slave_get_text (slave);
	line_offset = html_text_slave_get_line_offset (slave, 0, painter);	
	owner = HTML_TEXT (slave->owner);
	font_style = html_text_get_font_style (owner);

	while (upper - lower > 1) {
		lo = line_offset;
		prev_width = width;

		if (width > x)
			upper = len;
		else 
			lower = len;
	
		len = (lower + upper + 1) / 2;

		if (len) {
			glyphs = get_glyphs_part (slave, painter, 0, len);
			html_text_calc_text_size (slave->owner, painter, text - slave->owner->text, len, html_text_get_pango_info (owner, painter), glyphs,
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

static guint
get_offset_for_pointer (HTMLTextSlave *slave, HTMLPainter *painter, gint x, gint y)
{
	g_return_val_if_fail (slave != NULL, 0);

	x -= HTML_OBJECT (slave)->x;

	if (x <= 0)
		return 0;

	if (x >= HTML_OBJECT (slave)->width - 1)
		return slave->posLen;

	if (slave->posLen > 1)
		return calc_offset (slave, painter, x, y);
	else
		return x > HTML_OBJECT (slave)->width / 2 ? 1 : 0;
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
clear_glyphs (HTMLTextSlave *slave)
{
	if (slave->glyphs) {
		glyphs_destroy (slave->glyphs);
		slave->glyphs = NULL;
	}
}

static void
destroy (HTMLObject *obj)
{
	HTMLTextSlave *slave = HTML_TEXT_SLAVE (obj);

	clear_glyphs (slave);

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
	object_class->calc_size = html_text_slave_real_calc_size;
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
		      guint posLen)
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
	slave->owner      = owner;
	slave->charStart  = NULL;
	slave->pi         = NULL;
	slave->glyphs     = NULL;

	/* text slaves have always min_width 0 */
	object->min_width = 0;
	object->change   &= ~HTML_CHANGE_MIN_WIDTH;
}

HTMLObject *
html_text_slave_new (HTMLText *owner, guint posStart, guint posLen)
{
	HTMLTextSlave *slave;

	slave = g_new (HTMLTextSlave, 1);
	html_text_slave_init (slave, &html_text_slave_class, owner, posStart, posLen);

	return HTML_OBJECT (slave);
}
