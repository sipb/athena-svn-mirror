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
#include "gtkhtml-compat.h"

#include <string.h>
#include <gnome.h>
#include <ctype.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-paper.h>

#include "htmlembedded.h"
#include "htmlentity.h"
#include "gtkhtml-embedded.h"
#include "htmlfontmanager.h"
#include "htmlprinter.h"
#include "htmltext.h"

/* #define PRINTER_DEBUG */


static HTMLPainterClass *parent_class = NULL;


/* The size of a pixel in the printed output, in points.  */
#define PIXEL_SIZE .5

static void
insure_config (HTMLPrinter *p)
{
	if (!p->config)
		p->config = p->master ? gnome_print_job_get_config (p->master) : gnome_print_config_default ();
}

static gdouble
printer_get_page_height (HTMLPrinter *printer)
{
	gdouble width, height = 0.0;

	insure_config (printer);
	gnome_print_config_get_page_size (printer->config, &width, &height);

	return height;
}

static gdouble
printer_get_page_width (HTMLPrinter *printer)
{
	gdouble width = 0.0, height;

	insure_config (printer);
	gnome_print_config_get_page_size (printer->config, &width, &height);

	return width;
}

/* .5" in PS units */
#define TEMP_MARGIN 72*.5

static gdouble
get_lmargin (HTMLPrinter *printer)
{
	/*gdouble lmargin = 0.0;

	insure_config (printer);
	gnome_print_config_get_double (printer->config, GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, &lmargin);

	printf ("lmargin %f\n", lmargin);
	return lmargin;*/

	return TEMP_MARGIN;
}

static gdouble
get_rmargin (HTMLPrinter *printer)
{
	/*gdouble rmargin = 0.0;

	insure_config (printer);
	gnome_print_config_get_double (printer->config, GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, &rmargin);

	return rmargin;*/

	return TEMP_MARGIN;
}

static gdouble
get_tmargin (HTMLPrinter *printer)
{
	/* gdouble tmargin = 0.0;

	insure_config (printer);
	gnome_print_config_get_double (printer->config, GNOME_PRINT_KEY_PAGE_MARGIN_TOP, &tmargin);

	return tmargin; */

	return TEMP_MARGIN;
}

static gdouble
get_bmargin (HTMLPrinter *printer)
{
	/* gdouble bmargin = 0.0;

	insure_config (printer);
	gnome_print_config_get_double (printer->config, GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, &bmargin);

	return bmargin; */

	return TEMP_MARGIN;
}

gdouble
html_printer_scale_to_gnome_print (HTMLPrinter *printer, gint x)
{
	return SCALE_ENGINE_TO_GNOME_PRINT (x);
}

void
html_printer_coordinates_to_gnome_print (HTMLPrinter *printer,
					 gint engine_x, gint engine_y,
					 gdouble *print_x_return, gdouble *print_y_return)
{
	gdouble print_x, print_y;

	print_x = SCALE_ENGINE_TO_GNOME_PRINT (engine_x);
	print_y = SCALE_ENGINE_TO_GNOME_PRINT (engine_y);

	print_x = print_x + get_lmargin (printer);
	print_y = (printer_get_page_height (printer) - print_y) - get_tmargin (printer);

	*print_x_return = print_x;
	*print_y_return = print_y;
}

#if 0
static void
gnome_print_coordinates_to_engine (HTMLPrinter *printer,
				   gdouble print_x, gdouble print_y,
				   gint *engine_x_return, gint *engine_y_return)
{
	print_x -= get_lmargin (printer);
	print_y -= get_bmargin (printer);

	*engine_x_return = SCALE_ENGINE_TO_GNOME_PRINT (print_x);
	*engine_y_return = SCALE_GNOME_PRINT_TO_ENGINE (get_page_height (printer) - print_y);
}
#endif


/* GtkObject methods.  */

static void
finalize (GObject *object)
{
	HTMLPrinter *printer;

	printer = HTML_PRINTER (object);

	if (printer->config != NULL) {
		/* FIXME g_object_unref (printer->config); */
		printer->config = NULL;
	}
	if (printer->context != NULL) {
		gnome_print_context_close (printer->context);
		g_object_unref (printer->context);
		printer->context = NULL;
	}

	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}


static void
begin (HTMLPainter *painter,
       int x1, int y1,
       int x2, int y2)
{
	HTMLPrinter *printer;
	GnomePrintContext *pc;
	gdouble printer_x1, printer_y1;
	gdouble printer_x2, printer_y2;
#ifdef PRINTER_DEBUG
	gdouble dash [2];
#endif
	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer);
	pc      = printer->context;
	g_return_if_fail (pc);

	gnome_print_beginpage (pc, "page");
	gnome_print_gsave (pc);

	html_printer_coordinates_to_gnome_print (printer, x1, y1, &printer_x1, &printer_y1);
	printer_x2 = printer_x1 + SCALE_ENGINE_TO_GNOME_PRINT (x2);
	printer_y2 = printer_y1 - SCALE_ENGINE_TO_GNOME_PRINT (y2);

	gnome_print_newpath (pc);

	gnome_print_moveto (pc, printer_x1, printer_y1);
	gnome_print_lineto (pc, printer_x1, printer_y2);
	gnome_print_lineto (pc, printer_x2, printer_y2);
	gnome_print_lineto (pc, printer_x2, printer_y1);
	gnome_print_lineto (pc, printer_x1, printer_y1);
	gnome_print_closepath (pc);

#ifdef PRINTER_DEBUG
	gnome_print_gsave (pc);
	dash [0] = 10.0;
	dash [1] = 10.0;
	gnome_print_setrgbcolor (pc, .5, .5, .5);
	gnome_print_setlinewidth (pc, .3);
	gnome_print_setdash (pc, 2, dash, .0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);
#endif
	gnome_print_clip (pc);
}

static void
end (HTMLPainter *painter)
{
	HTMLPrinter *printer;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->context != NULL);

	gnome_print_grestore (printer->context);
	gnome_print_showpage (printer->context);
}

static void
clear (HTMLPainter *painter)
{
}


static void
alloc_color (HTMLPainter *painter, GdkColor *color)
{
}

static void
free_color (HTMLPainter *painter, GdkColor *color)
{
}


static void
set_pen (HTMLPainter *painter,
	 const GdkColor *color)
{
	HTMLPrinter *printer;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->context != NULL);

	gnome_print_setrgbcolor (printer->context,
				 color->red / 65535.0, color->green / 65535.0, color->blue / 65535.0);
}

static const GdkColor *
get_black (const HTMLPainter *painter)
{
	static GdkColor black = { 0, 0, 0, 0 };

	return &black;
}

static void
prepare_rectangle (HTMLPainter *painter, gint _x, gint _y, gint w, gint h)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);
	GnomePrintContext *context = printer->context;
	gdouble x;
	gdouble y;
	gdouble width;
	gdouble height;

	width = SCALE_ENGINE_TO_GNOME_PRINT (w);
	height = SCALE_ENGINE_TO_GNOME_PRINT (h);

	html_printer_coordinates_to_gnome_print (HTML_PRINTER (painter), _x, _y, &x, &y);

	gnome_print_newpath (context);
	gnome_print_moveto  (context, x, y);
	gnome_print_lineto  (context, x + width, y);
	gnome_print_lineto  (context, x + width, y - height);
	gnome_print_lineto  (context, x, y - height);
	gnome_print_lineto  (context, x, y);
	gnome_print_closepath (context);
}

static void
do_rectangle (HTMLPainter *painter, gint x, gint y, gint w, gint h, gint lw)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);
	GnomePrintContext *context = printer->context;

	gnome_print_setlinewidth (context, SCALE_ENGINE_TO_GNOME_PRINT (lw) * PIXEL_SIZE);
	prepare_rectangle (painter, x, y, w, h);
	gnome_print_stroke (context);
}

static void
set_clip_rectangle (HTMLPainter *painter,
		    gint x, gint y,
		    gint width, gint height)
{
	prepare_rectangle (painter, x, y, width, height);
	gnome_print_clip (HTML_PRINTER (painter)->context);
}

/* HTMLPainter drawing functions.  */

static void
draw_line (HTMLPainter *painter,
	   gint x1, gint y1,
	   gint x2, gint y2)
{
	HTMLPrinter *printer;
	double printer_x1, printer_y1;
	double printer_x2, printer_y2;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->context != NULL);

	html_printer_coordinates_to_gnome_print (printer, x1, y1, &printer_x1, &printer_y1);
	html_printer_coordinates_to_gnome_print (printer, x2, y2, &printer_x2, &printer_y2);

	gnome_print_setlinewidth (printer->context, PIXEL_SIZE);

	gnome_print_newpath (printer->context);
	gnome_print_moveto (printer->context, printer_x1, printer_y1);
	gnome_print_lineto (printer->context, printer_x2, printer_y2);

	gnome_print_stroke (printer->context);
}

static void
draw_rect (HTMLPainter *painter,
	   gint x, gint y,
	   gint width, gint height)
{
	do_rectangle (painter, x, y, width, height, 1);
}

static void
draw_panel (HTMLPainter *painter,
	    GdkColor *bg,
	    gint _x, gint _y,
	    gint w, gint h,
	    GtkHTMLEtchStyle inset,
	    gint bordersize)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);
	GnomePrintContext *pc = printer->context;
	GdkColor *col1 = NULL, *col2 = NULL;
	GdkColor dark, light;
	gdouble x;
	gdouble y;
	gdouble width, bs;
	gdouble height;

	#define INC 0x8000
	#define DARK(c)  dark.c = MAX (((gint) bg->c) - INC, 0)
	#define LIGHT(c) light.c = MIN (((gint) bg->c) + INC, 0xffff)

	DARK(red);
	DARK(green);
	DARK(blue);
	LIGHT(red);
	LIGHT(green);
	LIGHT(blue);

	switch (inset) {
	case GTK_HTML_ETCH_NONE:
		/* use the current pen color */
		col1 = NULL;
		col2 = NULL;
		break;
	case GTK_HTML_ETCH_OUT:
		col1 = &light;
		col2 = &dark;
		break;
	default:
	case GTK_HTML_ETCH_IN:
		col1 = &dark;
		col2 = &light;
		break;
	}

	width  = SCALE_ENGINE_TO_GNOME_PRINT (w);
	height = SCALE_ENGINE_TO_GNOME_PRINT (h);
	bs     = SCALE_ENGINE_TO_GNOME_PRINT (bordersize);

	html_printer_coordinates_to_gnome_print (HTML_PRINTER (painter), _x, _y, &x, &y);

	if (col2)
		gnome_print_setrgbcolor (pc, col1->red / 65535.0, col1->green / 65535.0, col1->blue / 65535.0);

	gnome_print_newpath (pc);
	gnome_print_moveto  (pc, x, y);
	gnome_print_lineto  (pc, x + width, y);
	gnome_print_lineto  (pc, x + width - bs, y - bs);
	gnome_print_lineto  (pc, x + bs, y - bs );
	gnome_print_lineto  (pc, x + bs, y - height + bs);
	gnome_print_lineto  (pc, x, y - height);
	gnome_print_closepath (pc);
	gnome_print_fill    (pc);

	if (col1)
		gnome_print_setrgbcolor (pc, col2->red / 65535.0, col2->green / 65535.0, col2->blue / 65535.0);

	gnome_print_newpath (pc);
	gnome_print_moveto  (pc, x, y - height);
	gnome_print_lineto  (pc, x + width, y - height);
	gnome_print_lineto  (pc, x + width, y);
	gnome_print_lineto  (pc, x + width - bs, y - bs);
	gnome_print_lineto  (pc, x + width - bs, y - height + bs);
	gnome_print_lineto  (pc, x + bs, y - height + bs);
	gnome_print_closepath (pc);
	gnome_print_fill    (pc);
}

static void
draw_background (HTMLPainter *painter,
		 GdkColor *color,
		 GdkPixbuf *pixbuf,
		 gint ix, gint iy, 
		 gint pix_width, gint pix_height,
		 gint tile_x, gint tile_y)
{
	GnomePrintContext *pc;
	HTMLPrinter *printer;
	gdouble x, y, width, height;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer);
	pc = printer->context;
	g_return_if_fail (printer->context);

	width = SCALE_ENGINE_TO_GNOME_PRINT  (pix_width);
	height = SCALE_ENGINE_TO_GNOME_PRINT (pix_height);
	html_printer_coordinates_to_gnome_print (printer, ix, iy, &x, &y);

	if (color) {
		gnome_print_setrgbcolor (pc, color->red / 65535.0, color->green / 65535.0, color->blue / 65535.0);

		gnome_print_newpath (pc);
		gnome_print_moveto (pc, x, y);
		gnome_print_lineto (pc, x + width, y);
		gnome_print_lineto (pc, x + width, y - height);
		gnome_print_lineto (pc, x, y - height);
		gnome_print_lineto (pc, x, y);
		gnome_print_closepath (pc);

		gnome_print_fill (pc);
	}
}

static void
print_pixbuf (GnomePrintContext *pc, GdkPixbuf *pixbuf)
{
	if (!pixbuf || (gdk_pixbuf_get_colorspace (pixbuf) != GDK_COLORSPACE_RGB))
		return;
	
	if (gdk_pixbuf_get_has_alpha (pixbuf)) {
		gnome_print_rgbaimage (pc, 
				       gdk_pixbuf_get_pixels (pixbuf),
				       gdk_pixbuf_get_width (pixbuf),
				       gdk_pixbuf_get_height (pixbuf),
				       gdk_pixbuf_get_rowstride (pixbuf));
	} else {
		gnome_print_rgbimage (pc, 
				      gdk_pixbuf_get_pixels (pixbuf),
				      gdk_pixbuf_get_width (pixbuf),
				      gdk_pixbuf_get_height (pixbuf),
				      gdk_pixbuf_get_rowstride (pixbuf));
	}
}

static void
draw_pixmap (HTMLPainter *painter, GdkPixbuf *pixbuf, gint x, gint y, gint scale_width, gint scale_height, const GdkColor *color)
{
	HTMLPrinter *printer;
	gint width, height;
	double print_x, print_y;
	double print_scale_width, print_scale_height;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->context != NULL);

	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);

	html_printer_coordinates_to_gnome_print (printer, x, y, &print_x, &print_y);

	print_scale_width  = SCALE_ENGINE_TO_GNOME_PRINT (scale_width);
	print_scale_height = SCALE_ENGINE_TO_GNOME_PRINT (scale_height);

	gnome_print_gsave (printer->context);
	gnome_print_translate (printer->context, print_x, print_y - print_scale_height);
	gnome_print_scale (printer->context, print_scale_width, print_scale_height);
	print_pixbuf (printer->context, pixbuf);
	gnome_print_grestore (printer->context);
}

static void
fill_rect (HTMLPainter *painter, gint x, gint y, gint width, gint height)
{
	HTMLPrinter *printer;
	double printer_x, printer_y;
	double printer_width, printer_height;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->context != NULL);

	printer_width = SCALE_ENGINE_TO_GNOME_PRINT (width);
	printer_height = SCALE_ENGINE_TO_GNOME_PRINT (height);

	html_printer_coordinates_to_gnome_print (printer, x, y, &printer_x, &printer_y);

	gnome_print_newpath (printer->context);
	gnome_print_moveto (printer->context, printer_x, printer_y);
	gnome_print_lineto (printer->context, printer_x + printer_width, printer_y);
	gnome_print_lineto (printer->context, printer_x + printer_width, printer_y - printer_height);
	gnome_print_lineto (printer->context, printer_x, printer_y - printer_height);
	gnome_print_lineto (printer->context, printer_x, printer_y);
	gnome_print_closepath (printer->context);

	gnome_print_fill (printer->context);
}

static void
process_attrs (HTMLPrinter *printer, GSList *attrs, GtkHTMLFontStyle *style, gboolean *bgcolor, gboolean *underline, gboolean *strikethrough, gboolean set_context)
{
	gboolean set_color = FALSE;

	while (attrs) {
		PangoAttribute *attr = attrs->data;

		switch (attr->klass->type) {
		case PANGO_ATTR_FOREGROUND: {
			PangoColor pc;
      
			if (set_context) {
				pc = ((PangoAttrColor *) attr)->color;
				gnome_print_setrgbcolor (printer->context,
							 pc.red / 65535.0, pc.green / 65535.0, pc.blue / 65535.0);
				set_color = TRUE;
			}
		}
		break;
		case PANGO_ATTR_BACKGROUND:
			if (bgcolor)
				*bgcolor = TRUE;
		case PANGO_ATTR_WEIGHT:
			if (style)
				*style |= GTK_HTML_FONT_STYLE_BOLD;
			break;
		case PANGO_ATTR_STYLE:
			if (style)
				*style |= GTK_HTML_FONT_STYLE_ITALIC;
			break;
		case PANGO_ATTR_UNDERLINE:
			if (underline)
				*underline = TRUE;
			break;
		case PANGO_ATTR_STRIKETHROUGH:
			if (strikethrough)
				*strikethrough = TRUE;
			break;
		case PANGO_ATTR_SIZE:
			if (style)
				*style |= ((HTMLPangoAttrFontSize *) attr)->style;
			break;
		case PANGO_ATTR_FAMILY: {
			PangoAttrString *as = (PangoAttrString *) attr;

			if (!strcmp (as->value, "Monospace")
			    || (HTML_PAINTER (printer)->font_manager.fixed.face && !strcmp (as->value, HTML_PAINTER (printer)->font_manager.fixed.face)))
				*style |= GTK_HTML_FONT_STYLE_FIXED;
		}
		break;
		default:
			break;
		}
		attrs = attrs->next;
	}

	if (set_context && !set_color)
		gnome_print_setrgbcolor (printer->context, 0.0, 0.0, 0.0);
}

static void
show_sized_with_nbsps_replaced (GnomePrintContext *pc, const char *text, int bytes)
{
	const char *cur_text;
	char *cur_replaced, *match, *replaced, *to_free = NULL;
	int replaced_bytes;

	if (bytes < 8192)
		replaced = alloca (bytes);
	else
		to_free = replaced = g_malloc (bytes);

	cur_text = text;
	cur_replaced = replaced;
	replaced_bytes = bytes;
	while (cur_text < text + bytes) {
		/* look for begining of 0xc2 0xa0 sequence - utf8 nbsp; */
		match = memchr (cur_text, 0xc2, bytes - (cur_text - text));
		if (match != NULL && match - text < bytes && IS_UTF8_NBSP ((guchar *)match)) {
			strncpy (cur_replaced, cur_text, match - cur_text);
			cur_replaced += match - cur_text;
			*cur_replaced = ' ';
			cur_replaced ++;
			cur_text = match + 2;
			replaced_bytes --;
		} else {
			strncpy (cur_replaced, cur_text, bytes - (cur_text - text));
			break;
		}
	}

	replaced [replaced_bytes] = 0;
	gnome_print_show_sized (pc, replaced, replaced_bytes);

	g_free (to_free);
}

static gint
draw_text (HTMLPainter *painter, gint x, gint y, const gchar *text, gint len, HTMLTextPangoInfo *pi, PangoAttrList *attrs, GList *glyphs, gint start_byte_offset)
{
	GnomeFont *font;
	HTMLPrinter *printer;
	gdouble print_x, print_y;
	double width, asc, dsc;
	gboolean temp_pi = FALSE;

	width = 0.0;

	printer = HTML_PRINTER (painter);
	g_return_val_if_fail (printer->context != NULL, 0);

	if (!pi) {
		pi = html_painter_text_itemize_and_prepare_glyphs (painter, html_painter_get_font (painter, painter->font_face, painter->font_style),
								   text, g_utf8_offset_to_pointer (text, len) - text, &glyphs, attrs);
		start_byte_offset = 0;
		temp_pi = TRUE;
	}

	font = html_painter_get_font (painter, painter->font_face, painter->font_style);
	dsc = -gnome_font_get_descender (font);
	asc = gnome_font_get_ascender (font);

	if (pi && pi->n) {
		GList *gl;
		PangoGlyphString *str;
		GtkHTMLFontStyle style;
		guint char_offset = 0;
		gint ii;
		const gchar *c_text = text;
		PangoAttrIterator *iter = NULL;

		html_printer_coordinates_to_gnome_print (printer, x, y, &print_x, &print_y);
		c_text = text;
		for (gl = glyphs; gl && char_offset < len; gl = gl->next) {
			GnomeFont *c_font;
			GdkGC *bg_gc;
			gboolean underline, strikethrough, bgcolor;
			gint c_bytes, begin, end;
			double cw = 0.0;

			str = (PangoGlyphString *) gl->data;
			gl = gl->next;
			ii = GPOINTER_TO_INT (gl->data);
			bg_gc = NULL;
			c_font = font;
			style = GTK_HTML_FONT_STYLE_DEFAULT;
			c_bytes = g_utf8_offset_to_pointer (c_text, str->num_glyphs) - c_text;
			gnome_print_gsave (printer->context);
			bgcolor = underline = strikethrough = FALSE;

			if (attrs)
				iter = pango_attr_list_get_iterator (attrs);

			if (iter) {
				do {
					pango_attr_iterator_range (iter, &begin, &end);
					if (iter && MAX (begin, c_text - text) < MIN (end, (c_text - text) + c_bytes)) {
						GSList *attr_list;
						attr_list = pango_attr_iterator_get_attrs (iter);
						process_attrs (printer, attr_list, &style, &bgcolor, &underline, &strikethrough, TRUE);
						html_text_free_attrs (attr_list);
					}
				} while (pango_attr_iterator_next (iter));
				pango_attr_iterator_destroy (iter);
			}
			process_attrs (printer, pi->entries [ii].item->analysis.extra_attrs, &style, &bgcolor, &underline, &strikethrough, TRUE);
			if (style != GTK_HTML_FONT_STYLE_DEFAULT)
				c_font = html_painter_get_font (HTML_PAINTER (printer), HTML_PAINTER (printer)->font_face, style);

			/* FIXME if (bgcolor) {
				PangoRectangle log_rect;

				pango_glyph_string_extents (str, pi->entries [ii].item->analysis.font, NULL, &log_rect);
				gdk_draw_rectangle (gdk_painter->pixmap, bg_gc, TRUE, x + width, y - PANGO_PIXELS (PANGO_ASCENT (log_rect)),
						    PANGO_PIXELS (log_rect.width), PANGO_PIXELS (log_rect.height));
				gdk_gc_unref (bg_gc);
				} */

			gnome_print_newpath (printer->context);
			gnome_print_moveto (printer->context, print_x, print_y);
			gnome_print_setfont (printer->context, c_font);
			show_sized_with_nbsps_replaced (printer->context, c_text, c_bytes);

			cw = gnome_font_get_width_utf8_sized (c_font, c_text, c_bytes);
			if (strikethrough || underline) {
				gint ly;

				gnome_print_setlinewidth (printer->context, 1.0);
				gnome_print_setlinecap (printer->context, GDK_CAP_BUTT);

				if (underline) {
					ly = print_y + gnome_font_get_underline_position (c_font);
					gnome_print_newpath (printer->context);
					gnome_print_moveto (printer->context, print_x, ly);
					gnome_print_lineto (printer->context, print_x + cw, ly);
					gnome_print_setlinewidth (printer->context, gnome_font_get_underline_thickness (c_font));
					gnome_print_stroke (printer->context);
				}

				if (strikethrough) {
					ly = print_y + gnome_font_get_ascender (c_font) / 2.0;
					gnome_print_newpath (printer->context);
					gnome_print_moveto (printer->context, print_x, ly);
					gnome_print_lineto (printer->context, print_x + cw, ly);
					gnome_print_setlinewidth (printer->context, gnome_font_get_underline_thickness (c_font));
					gnome_print_stroke (printer->context);
				}
			}

			gnome_print_grestore (printer->context);

			width += cw;
			print_x += cw;
			c_text += c_bytes;
			char_offset += str->num_glyphs;
		}
	}

	if (temp_pi) {
		if (glyphs)
			html_painter_glyphs_destroy (glyphs);
		if (pi)
			html_text_pango_info_destroy (pi);
	}

	return SCALE_GNOME_PRINT_TO_ENGINE (width);
}

static void
draw_embedded (HTMLPainter *p, HTMLEmbedded *o, gint x, gint y) 
{
	gdouble print_x, print_y;	
	HTMLPrinter *printer = HTML_PRINTER(p);
	GtkWidget *embedded_widget;

	html_printer_coordinates_to_gnome_print (printer, x, y, &print_x, &print_y);
	gnome_print_gsave(printer->context); 

	gnome_print_translate(printer->context, 
			      print_x, print_y - o->height * PIXEL_SIZE);
 
	embedded_widget = html_embedded_get_widget(o);
	if (embedded_widget && GTK_IS_HTML_EMBEDDED (embedded_widget)) {
		g_signal_emit_by_name(GTK_OBJECT (embedded_widget), 0,
				      "draw_print", 
				      printer->context);
	}

	gnome_print_grestore(printer->context); 
}

static void
draw_shade_line (HTMLPainter *painter,
		 gint x, gint y,
		 gint width)
{
	HTMLPrinter *printer;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->context != NULL);

	/* FIXME */
}

static void
text_size (HTMLPainter *painter, const gchar *text, guint blen, HTMLTextPangoInfo *pi, PangoAttrList *attrs, GList *glyphs, gint start_byte_offset,
	   GnomeFont *font, GtkHTMLFontStyle style, gint *width, gint *asc, gint *dsc)
{
	HTMLPrinter *printer;
	gboolean temp_pi = FALSE;
	double real_width = 0.0;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->context != NULL);
	g_return_if_fail (font != NULL);

	if (asc)
		*asc = SCALE_GNOME_PRINT_TO_ENGINE (gnome_font_get_ascender (font));
	if (dsc)
		*dsc = SCALE_GNOME_PRINT_TO_ENGINE (-gnome_font_get_descender (font));

	if (!pi) {
		pi = html_painter_text_itemize_and_prepare_glyphs (painter, html_painter_get_font (painter, painter->font_face, painter->font_style),
								   text, blen, &glyphs, attrs);
		start_byte_offset = 0;
		temp_pi = TRUE;
	}

	if (pi && pi->n) {
		GList *gl;
		guint char_offset = 0;
		gint ii;
		const gchar *c_text = text;
		PangoGlyphString *str;
		PangoAttrIterator *iter = NULL;

		c_text = text;
		for (gl = glyphs; gl && char_offset < blen; gl = gl->next) {
			GnomeFont *c_font;
			gint c_bytes, begin, end;

			str = (PangoGlyphString *) gl->data;
			gl = gl->next;
			ii = GPOINTER_TO_INT (gl->data);
			style = GTK_HTML_FONT_STYLE_DEFAULT;
			c_font = font;
			c_bytes = g_utf8_offset_to_pointer (c_text, str->num_glyphs) - c_text;

			if (attrs)
				iter = pango_attr_list_get_iterator (attrs);

			if (iter) {
				do {
					pango_attr_iterator_range (iter, &begin, &end);
					if (iter && MAX (begin, c_text - text) < MIN (end, (c_text - text) + c_bytes)) {
						GSList *attr_list;
						attr_list = pango_attr_iterator_get_attrs (iter);
						process_attrs (printer, attr_list, &style, NULL, NULL, NULL, FALSE);
						html_text_free_attrs (attr_list);
					}
				} while (pango_attr_iterator_next (iter));
				pango_attr_iterator_destroy (iter);
			}

			process_attrs (printer, pi->entries [ii].item->analysis.extra_attrs, &style, NULL, NULL, NULL, FALSE);
			if (style != GTK_HTML_FONT_STYLE_DEFAULT)
				c_font = html_painter_get_font (HTML_PAINTER (printer), HTML_PAINTER (printer)->font_face, style);

			if (asc)
				*asc = MAX (*asc, SCALE_GNOME_PRINT_TO_ENGINE (gnome_font_get_ascender (c_font)));
			if (dsc)
				*dsc = MAX (*dsc, SCALE_GNOME_PRINT_TO_ENGINE (-gnome_font_get_descender (c_font)));

			if (width)
				real_width += gnome_font_get_width_utf8_sized (c_font, c_text, c_bytes);
			c_text += c_bytes;
			char_offset += str->num_glyphs;
		}
	}

	if (temp_pi) {
		if (glyphs)
			html_painter_glyphs_destroy (glyphs);
		if (pi)
			html_text_pango_info_destroy (pi);
	}

	if (width)
		*width = SCALE_GNOME_PRINT_TO_ENGINE (real_width);
}

static void
calc_text_size_bytes (HTMLPainter *painter, const gchar *text, guint len, HTMLTextPangoInfo *pi, PangoAttrList *attrs, GList *glyphs, gint start_byte_offset,
		      HTMLFont *font, GtkHTMLFontStyle style, gint *width, gint *asc, gint *dsc)
{
	text_size (painter, text, len, pi, attrs, glyphs, start_byte_offset, font->data, style, width, asc, dsc);
}

static void
calc_text_size (HTMLPainter *painter, const gchar *text, guint len, HTMLTextPangoInfo *pi, PangoAttrList *attrs, GList *glyphs, gint start_byte_offset,
		GtkHTMLFontStyle style, HTMLFontFace *face, gint *width, gint *asc, gint *dsc)
{
	HTMLPrinter *printer;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->context != NULL);

	text_size (painter, text, g_utf8_offset_to_pointer (text, len) - text, pi, attrs, glyphs,
		   start_byte_offset, html_painter_get_font (painter, face, style), style, width, asc, dsc);
}


static guint
get_pixel_size (HTMLPainter *painter)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);

	return SCALE_GNOME_PRINT_TO_ENGINE (PIXEL_SIZE);
}

static inline gdouble
get_font_size (HTMLPrinter *printer, gboolean points, gdouble size)
{
	return printer->scale * (points ? PANGO_PIXELS (size) / 10 : PANGO_PIXELS (size));
}

static HTMLFont *
alloc_font (HTMLPainter *painter, gchar *face, gdouble size, gboolean points, GtkHTMLFontStyle style)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);
	GnomeFontWeight weight;
	GnomeFont *font;
	gboolean italic;

	weight = (style & GTK_HTML_FONT_STYLE_BOLD) ? GNOME_FONT_BOLD : GNOME_FONT_BOOK;
	italic = (style & GTK_HTML_FONT_STYLE_ITALIC) ? TRUE : FALSE;

	font = gnome_font_find_closest_from_weight_slant (face ? face : (style & GTK_HTML_FONT_STYLE_FIXED ? "Monospace" : "Sans"),
							  weight, italic, get_font_size (printer, points, size));

	if (font == NULL && face == NULL) {
		GList *family_list;

		family_list = gnome_font_family_list ();
		if (family_list && family_list->data) {
			font = gnome_font_find_closest_from_weight_slant (family_list->data, weight, italic, get_font_size (printer, points, size));
			gnome_font_family_list_free (family_list);
		}
	}

	return font ? html_font_new (font,
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, " ", 1)/HTML_PRINTER (printer)->scale),
				     SCALE_GNOME_PRINT_TO_ENGINE (gnome_font_get_ascender (font)),
				     SCALE_GNOME_PRINT_TO_ENGINE (-gnome_font_get_descender (font)),
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, "\xc2\xa0", 2)/HTML_PRINTER (printer)->scale),
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, "\t", 1)/HTML_PRINTER (printer)->scale),
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, "e", 1)/HTML_PRINTER (printer)->scale),
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, HTML_BLOCK_CITE, strlen (HTML_BLOCK_CITE))
								       /HTML_PRINTER (printer)->scale),
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, HTML_BLOCK_INDENT, strlen (HTML_BLOCK_INDENT))
								       /HTML_PRINTER (printer)->scale))
		: NULL;
}

static void
ref_font (HTMLPainter *painter, HTMLFont *font)
{
	g_object_ref (font->data);
}

static void
unref_font (HTMLPainter *painter, HTMLFont *font)
{
	g_object_unref (font->data);
}

static guint
get_page_width (HTMLPainter *painter, HTMLEngine *e)
{
	return html_printer_get_page_width (HTML_PRINTER (painter));
}

static guint
get_page_height (HTMLPainter *painter, HTMLEngine *e)
{
	return html_printer_get_page_height (HTML_PRINTER (painter));
}

static void
html_printer_init (GObject *object)
{
	HTMLPrinter *printer;

	printer                = HTML_PRINTER (object);
	printer->context       = NULL;
	printer->config        = NULL;
	printer->scale         = 1.0;
}

static void
html_printer_class_init (GObjectClass *object_class)
{
	HTMLPainterClass *painter_class;

	painter_class = HTML_PAINTER_CLASS (object_class);

	parent_class = g_type_class_ref (HTML_TYPE_PAINTER);

	object_class->finalize = finalize;

	painter_class->begin = begin;
	painter_class->end = end;
	painter_class->alloc_font = alloc_font;
	painter_class->ref_font   = ref_font;
	painter_class->unref_font = unref_font;
	painter_class->alloc_color = alloc_color;
	painter_class->free_color = free_color;
	painter_class->calc_text_size = calc_text_size;
	painter_class->calc_text_size_bytes = calc_text_size_bytes;
	painter_class->set_pen = set_pen;
	painter_class->get_black = get_black;
	painter_class->draw_line = draw_line;
	painter_class->draw_rect = draw_rect;
	painter_class->draw_panel = draw_panel;
	painter_class->draw_text = draw_text;
	painter_class->fill_rect = fill_rect;
	painter_class->draw_pixmap = draw_pixmap;
	painter_class->clear = clear;
	painter_class->draw_shade_line = draw_shade_line;
	painter_class->draw_background = draw_background;
	painter_class->get_pixel_size = get_pixel_size;
	painter_class->set_clip_rectangle = set_clip_rectangle;
	painter_class->draw_embedded = draw_embedded;
	painter_class->get_page_width = get_page_width;
	painter_class->get_page_height = get_page_height;
}

GType
html_printer_get_type (void)
{
	static GType html_printer_type = 0;

	if (html_printer_type == 0) {
		static const GTypeInfo html_printer_info = {
			sizeof (HTMLPrinterClass),
			NULL,
			NULL,
			(GClassInitFunc) html_printer_class_init,
			NULL,
			NULL,
			sizeof (HTMLPrinter),
			1,
			(GInstanceInitFunc) html_printer_init,
		};
		html_printer_type = g_type_register_static (HTML_TYPE_PAINTER, "HTMLPrinter", &html_printer_info, 0);
	}

	return html_printer_type;
}

HTMLPainter *
html_printer_new (GtkWidget *widget, GnomePrintContext *context, GnomePrintJob *master)
{
	HTMLPrinter *new;

	new = g_object_new (HTML_TYPE_PRINTER, NULL);

	g_object_ref (context);
	html_painter_set_widget (HTML_PAINTER (new), widget);
	new->context = context;
	new->master = master;

	return HTML_PAINTER (new);
}


guint
html_printer_get_page_width (HTMLPrinter *printer)
{
	double printer_width;
	guint engine_width;

	g_return_val_if_fail (printer != NULL, 0);
	g_return_val_if_fail (HTML_IS_PRINTER (printer), 0);

	printer_width = printer_get_page_width (printer) - get_lmargin (printer) - get_rmargin (printer);
	engine_width = SCALE_GNOME_PRINT_TO_ENGINE (printer_width);

	return engine_width;
}

guint
html_printer_get_page_height (HTMLPrinter *printer)
{
	double printer_height;
	guint engine_height;

	g_return_val_if_fail (printer != NULL, 0);
	g_return_val_if_fail (HTML_IS_PRINTER (printer), 0);

	printer_height = printer_get_page_height (printer) - get_tmargin (printer) - get_bmargin (printer);
	engine_height = SCALE_GNOME_PRINT_TO_ENGINE (printer_height);

	return engine_height;
}
