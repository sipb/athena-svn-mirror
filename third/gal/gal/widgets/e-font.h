/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-font.h - Temporary wrappers around GdkFonts to get unicode displaying
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _E_FONT_H_
#define _E_FONT_H_

#include <glib.h>
#include <gdk/gdk.h>
#include <iconv.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

typedef struct _EFont EFont;

/*
 * We use very primitive styling here, enough for marking read/unread lines
 */

typedef enum {
	E_FONT_PLAIN = 0,
	E_FONT_BOLD = (1 << 0),
	E_FONT_ITALIC = (1 << 4)
} EFontStyle;

EFont * e_font_from_gdk_name (const gchar *name);
EFont * e_font_from_gdk_font (GdkFont *font);

GdkFont *e_font_to_gdk_font (EFont *font, EFontStyle style);

void e_font_ref (EFont *font);
void e_font_unref (EFont *font);

gint e_font_ascent (EFont * font);
gint e_font_descent (EFont * font);

gchar *e_font_get_name (EFont *font);

#define e_font_height(f) (e_font_ascent (f) + e_font_descent (f))

/*
 * NB! UTF-8 text widths are given in chars, not bytes
 */

void e_font_draw_utf8_text (GdkDrawable *drawable,
			    EFont *font, EFontStyle style,
			    GdkGC *gc,
			    gint x, gint y,
			    const gchar *text,
			    gint numbytes);

int e_font_utf8_text_width (EFont *font, EFontStyle style,
			    const char *text,
			    int numbytes);

int e_font_utf8_char_width (EFont *font, EFontStyle style,
			    char *text);

#if 0
iconv_t e_iconv_from_charset (const char *charset);
iconv_t e_iconv_to_charset (const char *charset);
#endif
const gchar *e_gdk_font_encoding (GdkFont *font);
iconv_t e_iconv_from_gdk_font (GdkFont *font);
iconv_t e_iconv_to_gdk_font (GdkFont *font);
#if 0
const gchar *e_locale_encoding (void);
iconv_t e_iconv_from_locale (void);
iconv_t e_iconv_to_locale (void);
#endif

END_GNOME_DECLS

#endif
