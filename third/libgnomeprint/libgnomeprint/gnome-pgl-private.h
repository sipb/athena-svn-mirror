/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-pgl-private.h: private members of positioned glyphlists
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Lauris Kaplinski <lauris@helixcode.com>
 *
 *  Copyright 2000-2003 Ximian, Inc. and authors
 */

#ifndef __GNOME_PGL_PRIVATE_H__
#define __GNOME_PGL_PRIVATE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GnomePosGlyph  GnomePosGlyph;
typedef struct _GnomePosString GnomePosString;

#include <libgnomeprint/gnome-rfont.h>
#include <libgnomeprint/gnome-pgl.h>

struct _GnomePosGlyph {
	   gint glyph;
	   gdouble x, y;
};

struct _GnomePosString {
	   gint start, length;
	   GnomeRFont * rfont;
	   guint32 color;
};

struct _GnomePosGlyphList {
	   guint version;
	   GnomePosGlyph *glyphs;
	   GnomePosString *strings;
	   gint num_strings;
};

void gnome_pgl_render_rgb8  (const GnomePosGlyphList * pgl,
			     gdouble x, gdouble y,
			     guchar *buf, gint width, gint height, gint rowstride,
			     guint flags);
void gnome_pgl_render_rgba8 (const GnomePosGlyphList * pgl,
			     gdouble x, gdouble y,
			     guchar *buf, gint width, gint height, gint rowstride,
			     guint flags);

G_END_DECLS

#endif /* __GNOME_PGL_PRIVATE_H__ */
