/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.

   Copyright (C) 2000 Helix Code, Inc.
   
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
#include <string.h>
#include <stdlib.h>
#include <gdk/gdkx.h>
#include <libart_lgpl/art_rect.h>

#include "htmlentity.h"
#include "htmlgdkpainter.h"
#include "htmlplainpainter.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlengine.h"
#include "htmltext.h"

static HTMLGdkPainterClass *parent_class = NULL;

static void
draw_panel (HTMLPainter *painter,
	    GdkColor *bg,
	    gint x, gint y,
	    gint width, gint height,
	    GtkHTMLEtchStyle inset,
	    gint bordersize)
{
}

static void
draw_background (HTMLPainter *painter,
		 GdkColor *color,
		 GdkPixbuf *pixbuf,
		 gint x, gint y, 
		 gint width, gint height,
		 gint tile_x, gint tile_y)
{
	HTMLGdkPainter *gdk_painter;
	GdkRectangle expose, paint, clip;

	gdk_painter = HTML_GDK_PAINTER (painter);

	expose.x = x;
	expose.y = y;
	expose.width  = width;
	expose.height = height;

	clip.x = gdk_painter->x1;
	clip.width = gdk_painter->x2 - gdk_painter->x1;
	clip.y = gdk_painter->y1;
	clip.height = gdk_painter->y2 - gdk_painter->y1;

	if (!gdk_rectangle_intersect (&clip, &expose, &paint))
		return;

	if (!color && !pixbuf)
		return;

	if (color) {
		gdk_gc_set_foreground (gdk_painter->gc, color);
		gdk_draw_rectangle (gdk_painter->pixmap, gdk_painter->gc,
				    TRUE, paint.x - clip.x, paint.y - clip.y,
				    paint.width, paint.height);	
		
	}

	return;
}

static void
draw_pixmap (HTMLPainter *painter,
	     GdkPixbuf *pixbuf,
	     gint x, gint y,
	     gint scale_width, gint scale_height,
	     const GdkColor *color)
{
}

static void
fill_rect (HTMLPainter *painter,
	   gint x, gint y,
	   gint width, gint height)
{
	HTMLGdkPainter *gdk_painter;

	gdk_painter = HTML_GDK_PAINTER (painter);

	gdk_draw_rectangle (gdk_painter->pixmap, gdk_painter->gc,
			    TRUE, x - gdk_painter->x1, y - gdk_painter->y1,
			    width, height);
}

static HTMLFont *
alloc_fixed_font (HTMLPainter *painter, gchar *face, gdouble size, gboolean points, GtkHTMLFontStyle style)
{
	return HTML_PAINTER_CLASS (parent_class)->alloc_font (painter, 
							      face ? painter->font_manager.fixed.face : NULL,
							      painter->font_manager.fix_size, painter->font_manager.fix_points,
							      GTK_HTML_FONT_STYLE_DEFAULT); 
}


static void
draw_shade_line (HTMLPainter *painter,
		 gint x, gint y,
		 gint width)
{
}

static void
html_plain_painter_init (GObject *object)
{
}

static void
draw_rect (HTMLPainter *painter,
	   gint x, gint y,
	   gint width, gint height)
{
}

static guint
get_page_width (HTMLPainter *painter, HTMLEngine *e)
{
	return MIN (72 * MAX (html_painter_get_space_width (painter, GTK_HTML_FONT_STYLE_SIZE_3 | GTK_HTML_FONT_STYLE_FIXED, NULL),
			      html_painter_get_e_width (painter, GTK_HTML_FONT_STYLE_SIZE_3 | GTK_HTML_FONT_STYLE_FIXED, NULL)),
		    html_engine_get_view_width (e)) + (html_engine_get_left_border (e) + html_engine_get_right_border (e));
}

static guint
get_page_height (HTMLPainter *painter, HTMLEngine *e)
{
	return html_engine_get_view_height (e) + (html_engine_get_top_border (e) + html_engine_get_bottom_border (e));
}

static void
html_plain_painter_class_init (GObjectClass *object_class)
{
	HTMLPainterClass *painter_class;

	painter_class = HTML_PAINTER_CLASS (object_class);
	parent_class = g_type_class_ref (HTML_TYPE_GDK_PAINTER);

	painter_class->alloc_font = alloc_fixed_font;
	painter_class->draw_rect = draw_rect;
	painter_class->fill_rect = fill_rect;
	painter_class->draw_pixmap = draw_pixmap;
	painter_class->draw_shade_line = draw_shade_line;
	painter_class->draw_panel = draw_panel;
	painter_class->draw_background = draw_background;
	painter_class->get_page_width = get_page_width;
	painter_class->get_page_height = get_page_height;
}

GType
html_plain_painter_get_type (void)
{
	static GType html_plain_painter_type = 0;

	if (html_plain_painter_type == 0) {
		static const GTypeInfo html_plain_painter_info = {
			sizeof (HTMLPlainPainterClass),
			NULL,
			NULL,
			(GClassInitFunc) html_plain_painter_class_init,
			NULL,
			NULL,
			sizeof (HTMLPlainPainter),
			1,
			(GInstanceInitFunc) html_plain_painter_init,
		};
		html_plain_painter_type = g_type_register_static (HTML_TYPE_GDK_PAINTER, "HTMLPlainPainter",
								  &html_plain_painter_info, 0);
	}

	return html_plain_painter_type;
}

HTMLPainter *
html_plain_painter_new (GtkWidget *widget, gboolean double_buffer)
{
	HTMLPlainPainter *new;

	new = g_object_new (HTML_TYPE_PLAIN_PAINTER, NULL);
	html_painter_set_widget (HTML_PAINTER (new), widget);
	HTML_GDK_PAINTER (new)->double_buffer = double_buffer;
	HTML_GDK_PAINTER (new)->pc = gtk_widget_get_pango_context (widget);
	g_object_ref (HTML_GDK_PAINTER (new)->pc);

	return HTML_PAINTER (new);
}
