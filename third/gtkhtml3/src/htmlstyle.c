/* "a -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2002, Ximian Inc.

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

#include <string.h>
#include "htmlstyle.h"

/* Color handling.  */
static gboolean
parse_color (const gchar *text,
	     GdkColor *color)
{
	gchar c [8];
	gint  len = strlen (text);

	if (gdk_color_parse (text, color))
		return TRUE;

	c [7] = 0;
	if (*text != '#') {
		c[0] = '#'; 
		strncpy (c + 1, text, 6);
		len++;
	} else {
		strncpy (c, text, 7);
	}
	
	if (len < 7)
		memset (c + len, '0', 7-len);

	return gdk_color_parse (c, color);
}

HTMLStyle *
html_style_new (void) 
{
	HTMLStyle *style = g_new0 (HTMLStyle, 1);

	style->color = NULL;
	style->mask = 0;
	style->settings = 0;

	/* BLOCK */
	style->text_align = HTML_HALIGN_NONE;

	style->text_valign = HTML_VALIGN_NONE;

	return style;
}

void
html_style_free (HTMLStyle *style)
{
	if (!style)
		return;

	g_free (style->face);
	g_free (style->bg_image);

	if (style->color)
		html_color_unref (style->color);

	if (style->bg_color)
		gdk_color_free (style->bg_color);

	g_free (style);
}

HTMLStyle *
html_style_add_color (HTMLStyle *style, HTMLColor *color)
{
	if (!style)
		style = html_style_new ();

	if (style->color)
		html_color_unref (style->color);

	style->color = color;

	if (color)
		html_color_ref (color);

	return style;
}      

HTMLStyle *
html_style_unset_decoration (HTMLStyle *style, GtkHTMLFontStyle font_style)
{
	if (!style)
		style = html_style_new ();

	font_style &= ~GTK_HTML_FONT_STYLE_SIZE_MASK;
	style->mask |= font_style;
	style->settings &= ~font_style;

	return style;
}

HTMLStyle *
html_style_set_decoration (HTMLStyle *style, GtkHTMLFontStyle font_style)
{
	if (!style)
		style = html_style_new ();

	font_style &= ~GTK_HTML_FONT_STYLE_SIZE_MASK;
	style->mask |= font_style;
	style->settings |= font_style;

	return style;
}

HTMLStyle *
html_style_set_font_size (HTMLStyle *style, GtkHTMLFontStyle font_style)
{
	if (!style)
		style = html_style_new ();

	font_style &= GTK_HTML_FONT_STYLE_SIZE_MASK;
	style->mask |= font_style;
	style->settings |= font_style;

	return style;
}

HTMLStyle *
html_style_add_font_face (HTMLStyle *style, const HTMLFontFace *face)
{
	if (!style)
		style = html_style_new ();

	g_free (style->face);
	style->face = g_strdup (face);

	return style;
}

HTMLStyle *
html_style_add_text_align (HTMLStyle *style, HTMLHAlignType type)
{
	if (!style)
		style = html_style_new ();

	style->text_align = type;

	return style;
}

HTMLStyle *
html_style_add_text_valign (HTMLStyle *style, HTMLVAlignType type)
{
	if (!style)
		style = html_style_new ();

	style->text_valign = type;

	return style;
}

HTMLStyle *
html_style_add_background_color (HTMLStyle *style, GdkColor *color)
{
	if (!style)
		style = html_style_new ();

	style->bg_color = gdk_color_copy (color);

	return style;
}

HTMLStyle *
html_style_add_background_image (HTMLStyle *style, const char *url)
{
	if (!style)
		style = html_style_new ();

	g_free (style->bg_image);
	style->bg_image = g_strdup (url);

	return style;
}

HTMLStyle *
html_style_add_attribute (HTMLStyle *style, const char *attr)
{
	gchar **prop;

	if (!style)
		style = html_style_new ();

	prop = g_strsplit (attr, ";", 100);

	if (prop) {
		gint i;
		for (i = 0; prop[i]; i++) {
			char *text;
			
			text = g_strstrip (prop[i]);
			if (!strncasecmp ("color: ", text, 7)) {
				GdkColor color;
				
				if (parse_color (g_strstrip (text + 7), &color)) {
					HTMLColor *hc = html_color_new_from_gdk_color (&color);
					html_style_add_color (style, hc);
				        html_color_unref (hc);
				}
			} else if (!strncasecmp ("text-decoration: none", text, 21)) {
				html_style_unset_decoration (style, ~GTK_HTML_FONT_STYLE_SIZE_MASK);
			}
		}
		g_strfreev (prop);
	}
	return style;
}

