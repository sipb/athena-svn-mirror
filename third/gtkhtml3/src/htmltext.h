/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the KDE libraries

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
   Copyright (C) 1999, 2000, 2001 Helix Code, Inc.
   
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
#ifndef _HTMLTEXT_H_
#define _HTMLTEXT_H_

#include "htmlobject.h"

#define HTML_TEXT(x) ((HTMLText *)(x))
#define HTML_TEXT_CLASS(x) ((HTMLTextClass *)(x))
#define HTML_IS_TEXT(x) (HTML_CHECK_TYPE ((x), HTML_TYPE_TEXT))

struct _SpellError {
	guint off;
	guint len;
};

struct _Link {
	guint start_index;
	guint end_index;
	gint start_offset;
	gint end_offset;
	gchar *url;
	gchar *target;
};

struct _HTMLTextPangoInfoEntry {
	PangoItem *item;
	PangoGlyphUnit *widths;
};

struct _HTMLTextPangoInfo {
	HTMLTextPangoInfoEntry *entries;
	PangoLogAttr *attrs;
	gint n;
};

struct _HTMLPangoAttrFontSize {
        PangoAttrInt attr_int;
	GtkHTMLFontStyle style;
};

struct _HTMLText {
	HTMLObject object;
	
	gchar   *text;
	guint    text_len;
	guint    text_bytes;

	PangoAttrList    *attr_list;
	PangoAttrList    *extra_attr_list;
	GtkHTMLFontStyle  font_style;
	HTMLFontFace     *face;
	HTMLColor        *color;

	guint select_start;
	guint select_length;

	GList *spell_errors;

	HTMLTextPangoInfo *pi;

	GSList *links;
	gint focused_link_offset;
};

struct _HTMLTextClass {
	HTMLObjectClass object_class;

        void         	   (* queue_draw)     (HTMLText *text, HTMLEngine *engine,
					       guint offset, guint len);

	GtkHTMLFontStyle   (* get_font_style) (const HTMLText *text);
	void               (* set_font_style) (HTMLText *text, HTMLEngine *engine, GtkHTMLFontStyle style);
};

extern HTMLTextClass html_text_class;

void              html_text_type_init                    (void);
void              html_text_class_init                   (HTMLTextClass      *klass,
							  HTMLType            type,
							  guint               object_size);
void              html_text_init                         (HTMLText           *text_object,
							  HTMLTextClass      *klass,
							  const gchar        *text,
							  gint                len,
							  GtkHTMLFontStyle    font_style,
							  HTMLColor          *color);
HTMLObject       *html_text_new                          (const gchar        *text,
							  GtkHTMLFontStyle    font_style,
							  HTMLColor          *color);
HTMLObject       *html_text_new_with_len                 (const gchar        *text,
							  gint                len,
							  GtkHTMLFontStyle    font_style,
							  HTMLColor          *color);
void              html_text_queue_draw                   (HTMLText           *text,
							  HTMLEngine         *engine,
							  guint               offset,
							  guint               len);
GtkHTMLFontStyle  html_text_get_font_style               (const HTMLText     *text);
void              html_text_set_font_style               (HTMLText           *text,
							  HTMLEngine         *engine,
							  GtkHTMLFontStyle    style);
void              html_text_append                       (HTMLText           *text,
							  const gchar        *str,
							  gint                len);
void              html_text_set_text                     (HTMLText           *text,
							  const gchar        *new_text);
void              html_text_set_font_face                (HTMLText           *text,
							  HTMLFontFace       *face);
gint              html_text_get_nb_width                 (HTMLText           *text,
							  HTMLPainter        *painter,
							  gboolean            begin);
guint             html_text_get_bytes                    (HTMLText           *text);
guint             html_text_get_index                    (HTMLText           *text,
							  guint               offset);
gunichar          html_text_get_char                     (HTMLText           *text,
							  guint               offset);
gchar            *html_text_get_text                     (HTMLText           *text,
							  guint               offset);
GList            *html_text_get_items                    (HTMLText           *text,
							  HTMLPainter        *painter);
void              html_text_spell_errors_clear           (HTMLText           *text);
void              html_text_spell_errors_clear_interval  (HTMLText           *text,
							  HTMLInterval       *i);
void              html_text_spell_errors_add             (HTMLText           *text,
							  guint               off,
							  guint               len);
gboolean          html_text_magic_link                   (HTMLText           *text,
							  HTMLEngine         *engine,
							  guint               offset);
gint              html_text_trail_space_width            (HTMLText           *text,
							  HTMLPainter        *painter);
gboolean          html_text_convert_nbsp                 (HTMLText           *text,
							  gboolean            free_text);
gint              html_text_get_line_offset              (HTMLText           *text,
							  HTMLPainter        *painter,
							  gint                offset);
gint              html_text_text_line_length             (const gchar        *text,
							  gint               *line_offset,
							  guint               len,
							  gint               *tabs);
gint              html_text_calc_part_width              (HTMLText           *text,
							  HTMLPainter        *painter,
							  char               *start,
							  gint                offset,
							  gint                len,
							  gint               *asc,
							  gint               *dsc);
gint              html_text_get_item_index               (HTMLText           *text,
							  HTMLPainter        *painter,
							  gint                offset,
							  gint               *item_offset);
gboolean          html_text_pi_backward                  (HTMLTextPangoInfo  *pi,
							  gint               *ii,
							  gint               *io);
gboolean          html_text_pi_forward                   (HTMLTextPangoInfo  *pi,
							  gint               *ii,
							  gint               *io);
void              html_text_free_attrs                   (GSList             *attrs);
gint              html_text_tail_white_space             (HTMLText           *text,
							  HTMLPainter        *painter,
							  gint                offset,
							  gint                ii,
							  gint                io,
							  gint               *white_len,
							  gint                line_offset,
							  gchar              *s);
void              html_text_append_link                  (HTMLText           *text,
							  gchar              *url,
							  gchar              *target,
							  gint                start_offset,
							  gint                end_offset);
void              html_text_append_link_full             (HTMLText           *text,
							  gchar              *url,
							  gchar              *target,
							  gint                start_index,
							  gint                end_index,
							  gint                start_offset,
							  gint                end_offset);
void              html_text_add_link                     (HTMLText           *text,
							  HTMLEngine         *e,
							  gchar              *url,
							  gchar              *target,
							  gint                start_offset,
							  gint                end_offset);
void              html_text_add_link_full                (HTMLText           *text,
							  HTMLEngine         *e,
							  gchar              *url,
							  gchar              *target,
							  gint                start_index,
							  gint                end_index,
							  gint                start_offset,
							  gint                end_offset);
void              html_text_remove_links                 (HTMLText           *text);
gboolean          html_text_get_link_rectangle           (HTMLText           *text,
							  HTMLPainter        *painter,
							  gint                offset,
							  gint               *x1,
							  gint               *y1,
							  gint               *x2,
							  gint               *y2);
Link             *html_text_get_link_at_offset           (HTMLText           *text,
							  gint                offset);
HTMLTextSlave    *html_text_get_slave_at_offset          (HTMLObject         *o,
							  gint                offset);
Link             *html_text_get_link_slaves_at_offset    (HTMLText           *text,
							  gint                offset,
							  HTMLTextSlave     **start,
							  HTMLTextSlave     **end);
gboolean          html_text_next_link_offset             (HTMLText           *text,
							  gint               *offset);
gboolean          html_text_prev_link_offset             (HTMLText           *text,
							  gint               *offset);
gboolean          html_text_first_link_offset            (HTMLText           *text,
							  gint               *offset);
gboolean          html_text_last_link_offset             (HTMLText           *text,
							  gint               *offset);
gchar            *html_text_get_link_text                (HTMLText           *text,
							  gint                offset);
void              html_text_calc_font_size               (HTMLText           *text,
							  HTMLEngine         *e);
GtkHTMLFontStyle  html_text_get_fontstyle_at_index       (HTMLText           *text,
							  gint                index);
GtkHTMLFontStyle  html_text_get_style_conflicts          (HTMLText           *text,
							  GtkHTMLFontStyle    style,
							  gint                start_index,
							  gint                end_index);
void              html_text_set_style_in_range           (HTMLText           *text,
							  GtkHTMLFontStyle    style,
							  HTMLEngine         *e,
							  gint                start_index,
							  gint                end_index);
void              html_text_set_style                    (HTMLText           *text,
							  GtkHTMLFontStyle    style,
							  HTMLEngine         *e);
void              html_text_unset_style                  (HTMLText           *text,
							  GtkHTMLFontStyle    style);
HTMLColor        *html_text_get_color_at_index           (HTMLText           *text,
							  HTMLEngine         *e,
							  gint                index);
HTMLColor        *html_text_get_color                    (HTMLText           *text,
							  HTMLEngine         *e,
							  gint                start_index);
void              html_text_set_color_in_range           (HTMLText           *text,
							  HTMLColor          *color,
							  gint                start_index,
							  gint                end_index);
void              html_text_set_color                    (HTMLText           *text,
							  HTMLColor          *color);

Link     *html_link_new                 (gchar *url,
					 gchar *target,
					 guint  start_index,
					 guint  end_index,
					 gint   start_offset,
					 gint   end_offset);
Link     *html_link_dup                 (Link  *link);
void      html_link_free                (Link  *link);
gboolean  html_link_equal               (Link  *l1,
					 Link  *l2);
void      html_link_set_url_and_target  (Link  *link,
					 gchar *url,
					 gchar *target);

/*
 * protected
 */
HTMLTextPangoInfo *html_text_pango_info_new        (gint                   n);
void               html_text_pango_info_destroy    (HTMLTextPangoInfo     *pi);
HTMLTextPangoInfo *html_text_get_pango_info        (HTMLText              *text,
						    HTMLPainter           *painter);
gint               html_text_pango_info_get_index  (HTMLTextPangoInfo     *pi,
						    gint                   byte_offset,
						    gint                   idx);
PangoAttribute    *html_pango_attr_font_size_new   (GtkHTMLFontStyle       style);
void               html_pango_attr_font_size_calc  (HTMLPangoAttrFontSize *attr,
						    HTMLEngine            *e);
PangoAttrList     *html_text_get_attr_list         (HTMLText              *text,
						    gint                   start_index,
						    gint                   end_index);
void               html_text_calc_text_size        (HTMLText              *t,
						    HTMLPainter           *painter,
						    gint                   start_byte_offset,
						    guint                  len,
						    HTMLTextPangoInfo     *pi,
						    GList                 *glyphs,
						    gint                  *line_offset,
						    GtkHTMLFontStyle       font_style,
						    HTMLFontFace          *face,
						    gint                  *width,
						    gint                  *asc,
						    gint                  *dsc);

gboolean  html_text_is_line_break                (PangoLogAttr  attr);
void      html_text_remove_unwanted_line_breaks  (char         *s,
						  int           len,
						  PangoLogAttr *attrs);

typedef HTMLObject * (* HTMLTextHelperFunc)       (HTMLText *, gint begin, gint end);
HTMLObject *html_text_op_copy_helper    (HTMLText           *text,
					 GList              *from,
					 GList              *to,
					 guint              *len);
HTMLObject *html_text_op_cut_helper     (HTMLText           *text,
					 HTMLEngine         *e,
					 GList              *from,
					 GList              *to,
					 GList              *left,
					 GList              *right,
					 guint              *len);
#endif /* _HTMLTEXT_H_ */
