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

#ifndef _HTMLCLUEFLOW_H_
#define _HTMLCLUEFLOW_H_

#include "htmltypes.h"
#include "htmlclue.h"

#define HTML_CLUEFLOW(x) ((HTMLClueFlow *)(x))
#define HTML_CLUEFLOW_CLASS(x) ((HTMLClueFlowClass *)(x))
#define HTML_IS_CLUEFLOW(x) (HTML_CHECK_TYPE ((x), HTML_TYPE_CLUEFLOW))

struct _HTMLClueFlow {
	HTMLClue clue;

	/* Paragraph style.  */
	HTMLClueFlowStyle style;

	/* Indentation level for blockquote and lists.  */
	guint8 level;

	/* list item attributes - this will be ideally moved to list item type
	   based on HTMLClueFlow once we have real types */
	HTMLListType item_type;
	gint         item_number;
};

struct _HTMLClueFlowClass {
	HTMLClueClass clue_class;

	GtkHTMLFontStyle (* get_default_font_style) (const HTMLClueFlow *self);
};


extern HTMLClueFlowClass html_clueflow_class;


void               html_clueflow_type_init                    (void);
void               html_clueflow_class_init                   (HTMLClueFlowClass  *klass,
							       HTMLType            type,
							       guint               object_size);
void               html_clueflow_init                         (HTMLClueFlow       *flow,
							       HTMLClueFlowClass  *klass,
							       HTMLClueFlowStyle   style,
							       guint8              indentation,
							       HTMLListType        item_type,
							       gint                item_number);
HTMLObject        *html_clueflow_new                          (HTMLClueFlowStyle   style,
							       guint8              indentation,
							       HTMLListType        item_type,
							       gint                item_number);
HTMLObject        *html_clueflow_new_from_flow                (HTMLClueFlow       *flow);
GtkHTMLFontStyle   html_clueflow_get_default_font_style       (const HTMLClueFlow *self);
HTMLClueFlow      *html_clueflow_split                        (HTMLClueFlow       *clue,
							       HTMLObject         *child);
void               html_clueflow_set_style                    (HTMLClueFlow       *flow,
							       HTMLEngine         *engine,
							       HTMLClueFlowStyle   style);
HTMLClueFlowStyle  html_clueflow_get_style                    (HTMLClueFlow       *flow);
void               html_clueflow_set_item_type                (HTMLClueFlow       *flow,
							       HTMLEngine         *engine,
							       HTMLListType        item_type);
HTMLListType       html_clueflow_get_item_type                (HTMLClueFlow       *flow);
void               html_clueflow_set_halignment               (HTMLClueFlow       *flow,
							       HTMLEngine         *engine,
							       HTMLHAlignType      alignment);
HTMLHAlignType     html_clueflow_get_halignment               (HTMLClueFlow       *flow);
void               html_clueflow_modify_indentation_by_delta  (HTMLClueFlow       *flow,
							       HTMLEngine         *engine,
							       gint                indentation);
void               html_clueflow_set_indentation              (HTMLClueFlow       *flow,
							       HTMLEngine         *engine,
							       guint8              indentation);
guint8             html_clueflow_get_indentation              (HTMLClueFlow       *flow);
void               html_clueflow_set_properties               (HTMLClueFlow       *flow,
							       HTMLEngine         *engine,
							       HTMLClueFlowStyle   style,
							       guint8              indentation,
							       HTMLHAlignType      alignment);
void               html_clueflow_get_properties               (HTMLClueFlow       *flow,
							       HTMLClueFlowStyle  *style_return,
							       guint8             *indentation_return,
							       HTMLHAlignType     *alignment_return);
void               html_clueflow_remove_text_slaves           (HTMLClueFlow       *flow);
void               html_clueflow_spell_check                  (HTMLClueFlow       *flow,
							       HTMLEngine         *e,
							       HTMLInterval       *i);
gboolean           html_clueflow_is_empty                     (HTMLClueFlow       *flow);
gint               html_clueflow_get_line_offset              (HTMLClueFlow       *flow,
							       HTMLPainter        *painter,
							       HTMLObject         *child);
gboolean           html_clueflow_tabs                         (HTMLClueFlow       *flow,
							       HTMLPainter        *p);

#define SPELL_CHECK(f, e) if (f && HTML_OBJECT_TYPE (f) == HTML_TYPE_CLUEFLOW) \
                                   html_clueflow_spell_check (HTML_CLUEFLOW (f), e, NULL)

#endif /* _HTMLCLUEFLOW_H_ */
