/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.
    Authors:           Radek Doulik (rodo@helixcode.com)

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
#include <gdk/gdk.h>

#include "htmlcolor.h"
#include "htmlpainter.h"

HTMLColor *
html_color_new (void)
{
	HTMLColor *nc = g_new0 (HTMLColor, 1);

	nc->refcount = 1;

	return nc;
}

HTMLColor *
html_color_new_from_gdk_color (const GdkColor *color)
{
	HTMLColor *nc = html_color_new ();

	nc->color = *color;

	return nc;
}

HTMLColor *
html_color_new_from_rgb (gushort red, gushort green, gushort blue)
{
	HTMLColor *nc = html_color_new ();

	nc->color.red   = red;
	nc->color.green = green;
	nc->color.blue  = blue;

	return nc;
}

void
html_color_ref (HTMLColor *color)
{
	g_assert (color);

	color->refcount ++;
}

void
html_color_unref (HTMLColor *color)
{
	g_assert (color);
	g_assert (color->refcount > 0);

	color->refcount --;

	if (!color->refcount) {
		/* if (color->allocated)
		   g_warning ("FIXME, color free\n"); */
		/* FIXME commented out to catch g_asserts on refcount so we could hunt "too much unrefs" bugs */
		g_free (color);
	}
}

void
html_color_alloc (HTMLColor *color, HTMLPainter *painter)
{
	g_assert (color);

	if (!color->allocated) {
		html_painter_alloc_color (painter, &color->color);
		color->allocated = TRUE;
	}
}

gboolean
html_color_equal (HTMLColor *color1, HTMLColor *color2)
{
	if (color1 == color2)
		return TRUE;
	if (!color1 || !color2)
		return FALSE;

	return gdk_color_equal (&color1->color, &color2->color);
}

void
html_color_set (HTMLColor *color, GdkColor *c)
{
	/* if (color->allocated)
	   g_warning ("FIXME, color free\n"); */
	color->allocated = FALSE;
	color->color = *c;
}
