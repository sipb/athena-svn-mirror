/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * test-font-loading.c
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
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

#define _TEST_E_FONT_C_

#include <gdk/gdkx.h>
#include <gnome.h>
#include "e-font.h"
#include "gal/util/e-iconv.h"

static void print_gdk_font_name (const GdkFont * font, const gchar * text);

int main (int argc, char ** argv)
{
	char **namelist;
	int numfonts;
	int i;

	gdk_init (&argc, &argv);

	namelist = XListFonts (GDK_DISPLAY (),
			       "*",
			       16384, &numfonts);

	g_print ("Loaded %d font names\n", numfonts);

	for (i = 0; i < numfonts; i++) {
		GdkFont * gdkfont;
		iconv_t iconv;
		gdkfont = gdk_font_load (namelist[i]);
		iconv = e_iconv_from_gdk_font (gdkfont);
		if (iconv == (iconv_t) -1) {
			print_gdk_font_name (gdkfont, "Missing from -");
		} else
			e_iconv_close(iconv);
		iconv = e_iconv_to_gdk_font (gdkfont);
		if (iconv == (iconv_t) -1) {
			print_gdk_font_name (gdkfont, "Missing to -");
		} else
			e_iconv_close(iconv);
		gdk_font_unref (gdkfont);
	}

	XFreeFontNames (namelist);

	return 0;
}

static void
print_gdk_font_name (const GdkFont * font, const gchar * text)
{
	Atom font_atom, atom;
	Bool status;

	font_atom = gdk_atom_intern ("FONT", FALSE);

	g_print (text);

	if (font == NULL) {
		g_print (" font is NULL\n");
	} else if (font->type == GDK_FONT_FONTSET) {
		XFontStruct **font_structs;
		gint num_fonts;
		gchar **font_names;

		num_fonts = XFontsOfFontSet (GDK_FONT_XFONT (font), &font_structs, &font_names);

		g_print (" fontset: %s\n",font_names[0]);
	} else {
		gchar * name;
		status = XGetFontProperty (GDK_FONT_XFONT (font), font_atom, &atom);
		name = gdk_atom_name (atom);
		g_print (" font: %s\n", name);
		if (name) g_free (name);
	}
}
