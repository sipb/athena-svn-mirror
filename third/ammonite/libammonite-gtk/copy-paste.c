
/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* copy-paste.c -- functions originating in nautilus-gtk-extensions.c

   Copyright (C) 1999, 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: John Sullivan <sullivan@eazel.com>
*/

#include <config.h>

#include "copy-paste.h"
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>


/**
 * ammonite_gtk_label_make_bold.
 *
 * Switches the font of label to a bold equivalent.
 * @label: The label.
 **/

void
ammonite_gtk_label_make_bold (GtkLabel *label)
{
	GtkStyle *style;
	GdkFont *bold_font;

	g_return_if_fail (GTK_IS_LABEL (label));

	style = gtk_widget_get_style (GTK_WIDGET (label));

	bold_font = ammonite_gdk_font_get_bold (style->font);
	if (bold_font == NULL) {
		return;
	}
	ammonite_gtk_widget_set_font (GTK_WIDGET (label), bold_font);
	gdk_font_unref (bold_font);
}

/**
 * ammonite_gtk_widget_set_font
 *
 * Sets the font for a widget's style, managing the style objects.
 * @widget: The widget.
 * @font: The font.
 **/
void
ammonite_gtk_widget_set_font (GtkWidget *widget, GdkFont *font)
{
	GtkStyle *new_style;
	
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (font != NULL);
	
	new_style = gtk_style_copy (gtk_widget_get_style (widget));

	ammonite_gtk_style_set_font (new_style, font);
	
	gtk_widget_set_style (widget, new_style);
	gtk_style_unref (new_style);
}


/**
 * ammonite_gdk_font_get_bold
 * @plain_font: A font.
 * Returns: A bold variant of @plain_font or NULL.
 *
 * Tries to find a bold flavor of a given font. Returns NULL if none is available.
 */
GdkFont *
ammonite_gdk_font_get_bold (const GdkFont *plain_font)
{
	const char *plain_name;
	const char *scanner;
	char *bold_name;
	int count;
	GSList *p;
	GdkFont *result;
	GdkFontPrivate *private_plain;

	private_plain = (GdkFontPrivate *)plain_font;

	if (private_plain->names == NULL) {
		return NULL;
	}


	/* -foundry-family-weight-slant-sel_width-add-style-pixels-points-hor_res-ver_res-spacing-average_width-char_set_registry-char_set_encoding */

	bold_name = NULL;
	for (p = private_plain->names; p != NULL; p = p->next) {
		plain_name = (const char *)p->data;
		scanner = plain_name;

		/* skip past foundry and family to weight */
		for (count = 2; count > 0; count--) {
			scanner = strchr (scanner + 1, '-');
			if (!scanner) {
				break;
			}
		}

		if (!scanner) {
			/* try the other names in the list */
			continue;
		}
		g_assert (*scanner == '-');

		/* copy "-foundry-family-" over */
		scanner++;
		bold_name = g_strndup (plain_name, scanner - plain_name);

		/* skip weight */
		scanner = strchr (scanner, '-');
		g_assert (scanner != NULL);

		/* FIXME bugzilla.eazel.com 2558:
		 * some fonts have demibold, etc. instead. We should be able to figure out
		 * which they are and use them here.
		 */

		/* add "bold" and copy everything past weight over */
		bold_name = g_strconcat (bold_name, "bold", scanner, NULL);
		break;
	}
	
	if (bold_name == NULL) {
		return NULL;
	}
	
	result = gdk_fontset_load (bold_name);
	g_free (bold_name);

	return result;
}

/**
 * nautilus_gtk_style_set_font
 *
 * Sets the font in a style object, managing the ref. counts.
 * @style: The style to change.
 * @font: The new font.
 **/
void
ammonite_gtk_style_set_font (GtkStyle *style, GdkFont *font)
{
	g_return_if_fail (style != NULL);
	g_return_if_fail (font != NULL);
	
	gdk_font_ref (font);
	gdk_font_unref (style->font);
	style->font = font;
}
