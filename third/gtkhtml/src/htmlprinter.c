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

#include <gnome.h>
#include <ctype.h>
#include <gal/unicode/gunicode.h>

#include "htmlembedded.h"
#include "gtkhtml-embedded.h"
#include "htmlfontmanager.h"
#include "htmlprinter.h"

/* #define PRINTER_DEBUG */


static HTMLPainterClass *parent_class = NULL;


/* The size of a pixel in the printed output, in points.  */
#define PIXEL_SIZE .5

/* Hm, this might need fixing.  */
#define SPACING_FACTOR 1.2


/* This is just a quick hack until GnomePrintContext is fixed to hold the paper
   size.  */

static const GnomePaper *paper = NULL;

static void
insure_paper (HTMLPrinter *printer)
{
	if (paper != NULL)
		return;

	if (printer->print_master) {
		paper = gnome_print_master_get_paper (printer->print_master);
	}
	if (!paper)
		paper = gnome_paper_with_name (_("US-Letter"));
	if (!paper)
		paper = gnome_paper_with_name (gnome_paper_name_default ()); 
	g_assert (paper != NULL);
}

static gdouble
printer_get_page_height (HTMLPrinter *printer)
{
	insure_paper (printer);
	return gnome_paper_psheight (paper) / printer->scale;
}

static gdouble
printer_get_page_width (HTMLPrinter *printer)
{
	insure_paper (printer);
	return gnome_paper_pswidth (paper) / printer->scale;
}

static gdouble
get_lmargin (HTMLPrinter *printer)
{
	insure_paper (printer);
	return gnome_paper_lmargin (paper) / 2;
}

static gdouble
get_rmargin (HTMLPrinter *printer)
{
	insure_paper (printer);
	return gnome_paper_rmargin (paper) / 2;
}

static gdouble
get_tmargin (HTMLPrinter *printer)
{
	insure_paper (printer);
	return gnome_paper_tmargin (paper) / 2;
}

static gdouble
get_bmargin (HTMLPrinter *printer)
{
	insure_paper (printer);
	return gnome_paper_bmargin (paper) / 2;
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

	print_x = print_x + get_lmargin (printer)/printer->scale;
	print_y = (printer_get_page_height (printer) - print_y) - get_tmargin (printer)/printer->scale;

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
finalize (GtkObject *object)
{
	HTMLPrinter *printer;

	printer = HTML_PRINTER (object);

	if (printer->print_context != NULL) {
		gnome_print_context_close (printer->print_context);
		gtk_object_unref (GTK_OBJECT (printer->print_context));
	}

	(* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
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
	pc      = printer->print_context;
	g_return_if_fail (pc);

	gnome_print_beginpage (pc, "page");
	gnome_print_gsave (pc);
	gnome_print_scale (pc, printer->scale, printer->scale);

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
	g_return_if_fail (printer->print_context != NULL);

	gnome_print_grestore (printer->print_context);
	gnome_print_showpage (printer->print_context);
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
	g_return_if_fail (printer->print_context != NULL);

	gnome_print_setrgbcolor (printer->print_context,
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
	GnomePrintContext *context = printer->print_context;
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
	GnomePrintContext *context = printer->print_context;

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
	gnome_print_clip (HTML_PRINTER (painter)->print_context);
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
	g_return_if_fail (printer->print_context != NULL);

	html_printer_coordinates_to_gnome_print (printer, x1, y1, &printer_x1, &printer_y1);
	html_printer_coordinates_to_gnome_print (printer, x2, y2, &printer_x2, &printer_y2);

	gnome_print_setlinewidth (printer->print_context, PIXEL_SIZE);

	gnome_print_newpath (printer->print_context);
	gnome_print_moveto (printer->print_context, printer_x1, printer_y1);
	gnome_print_lineto (printer->print_context, printer_x2, printer_y2);

	gnome_print_stroke (printer->print_context);
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
	GnomePrintContext *pc = printer->print_context;
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
	pc = printer->print_context;
	g_return_if_fail (printer->print_context);

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
draw_pixmap (HTMLPainter *painter,
	     GdkPixbuf *pixbuf,
	     gint x, gint y,
	     gint scale_width, gint scale_height,
	     const GdkColor *color)
{
	HTMLPrinter *printer;
	gint width, height;
	double print_x, print_y;
	double print_scale_width, print_scale_height;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->print_context != NULL);

	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);

	html_printer_coordinates_to_gnome_print (printer, x, y, &print_x, &print_y);

	print_scale_width  = SCALE_ENGINE_TO_GNOME_PRINT (scale_width);
	print_scale_height = SCALE_ENGINE_TO_GNOME_PRINT (scale_height);

	gnome_print_gsave (printer->print_context);
	gnome_print_translate (printer->print_context, print_x, print_y - print_scale_height);
	gnome_print_scale (printer->print_context, print_scale_width, print_scale_height);
	gnome_print_pixbuf (printer->print_context, pixbuf);
	gnome_print_grestore (printer->print_context);
}

static void
fill_rect (HTMLPainter *painter,
	   gint x, gint y,
	   gint width, gint height)
{
	HTMLPrinter *printer;
	double printer_x, printer_y;
	double printer_width, printer_height;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->print_context != NULL);

	printer_width = SCALE_ENGINE_TO_GNOME_PRINT (width);
	printer_height = SCALE_ENGINE_TO_GNOME_PRINT (height);

	html_printer_coordinates_to_gnome_print (printer, x, y, &printer_x, &printer_y);

	gnome_print_newpath (printer->print_context);
	gnome_print_moveto (printer->print_context, printer_x, printer_y);
	gnome_print_lineto (printer->print_context, printer_x + printer_width, printer_y);
	gnome_print_lineto (printer->print_context, printer_x + printer_width, printer_y - printer_height);
	gnome_print_lineto (printer->print_context, printer_x, printer_y - printer_height);
	gnome_print_lineto (printer->print_context, printer_x, printer_y);
	gnome_print_closepath (printer->print_context);

	gnome_print_fill (printer->print_context);
}

static void
draw_text (HTMLPainter *painter,
	   gint x, gint y,
	   const gchar *text,
	   gint len)
{
	GnomeFont *font;
	HTMLPrinter *printer;
	gdouble print_x, print_y;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->print_context != NULL);

	html_printer_coordinates_to_gnome_print (printer, x, y, &print_x, &print_y);

	gnome_print_newpath (printer->print_context);
	gnome_print_moveto (printer->print_context, print_x, print_y);

	font = html_painter_get_font (painter, painter->font_face, painter->font_style);
	gnome_print_setfont (printer->print_context, font);
	gnome_print_show_sized (printer->print_context, text, g_utf8_offset_to_pointer (text, len) - text);

	if (painter->font_style & (GTK_HTML_FONT_STYLE_UNDERLINE | GTK_HTML_FONT_STYLE_STRIKEOUT)) {
		double text_width;
		double ascender, descender;
		double y;

		gnome_print_gsave (printer->print_context);

		/* FIXME: We need something in GnomeFont to do this right.  */
		gnome_print_setlinewidth (printer->print_context, 1.0);
		gnome_print_setlinecap (printer->print_context, GDK_CAP_BUTT);

		text_width = gnome_font_get_width_utf8_sized (font, text, len);
		if (painter->font_style & GTK_HTML_FONT_STYLE_UNDERLINE) {
			descender = gnome_font_get_descender (font);
			y = print_y + gnome_font_get_underline_position (font);

			gnome_print_newpath (printer->print_context);
			gnome_print_moveto (printer->print_context, print_x, y);
			gnome_print_lineto (printer->print_context, print_x + text_width, y);
			gnome_print_setlinewidth (printer->print_context,
						  gnome_font_get_underline_thickness (font));
			gnome_print_stroke (printer->print_context);
		}

		if (painter->font_style & GTK_HTML_FONT_STYLE_STRIKEOUT) {
			ascender = gnome_font_get_ascender (font);
			y = print_y + ascender / 2.0;
			gnome_print_newpath (printer->print_context);
			gnome_print_moveto (printer->print_context, print_x, y);
			gnome_print_lineto (printer->print_context, print_x + text_width, y);
			gnome_print_setlinewidth (printer->print_context,
						  gnome_font_get_underline_thickness (font));
			gnome_print_stroke (printer->print_context);
		}

		gnome_print_grestore (printer->print_context);
	}
}

static void
draw_embedded (HTMLPainter *p, HTMLEmbedded *o, gint x, gint y) 
{
	gdouble print_x, print_y;	
	HTMLPrinter *printer = HTML_PRINTER(p);
	GtkWidget *embedded_widget;

	html_printer_coordinates_to_gnome_print (printer, x, y, &print_x, &print_y);
	gnome_print_gsave(printer->print_context); 

	gnome_print_translate(printer->print_context, 
			      print_x, print_y - o->height * PIXEL_SIZE);
 
	embedded_widget = html_embedded_get_widget(o);
	if (embedded_widget && GTK_IS_HTML_EMBEDDED (embedded_widget)) {
		gtk_signal_emit_by_name(GTK_OBJECT (embedded_widget), 
					"draw_print", 
					printer->print_context);
	}

	gnome_print_grestore(printer->print_context); 
}

static void
draw_shade_line (HTMLPainter *painter,
		 gint x, gint y,
		 gint width)
{
	HTMLPrinter *printer;

	printer = HTML_PRINTER (painter);
	g_return_if_fail (printer->print_context != NULL);

	/* FIXME */
}

static guint
calc_ascent (HTMLPainter *painter,
	     GtkHTMLFontStyle style,
	     HTMLFontFace *face)
{
	HTMLPrinter *printer;
	GnomeFont *font;
	double ascender;

	printer = HTML_PRINTER (painter);
	g_return_val_if_fail (printer->print_context != NULL, 0);

	font = html_painter_get_font (painter, face, style);
	g_return_val_if_fail (font != NULL, 0);

	ascender = gnome_font_get_ascender (font) * SPACING_FACTOR;
	return SCALE_GNOME_PRINT_TO_ENGINE (ascender);
}

static guint
calc_descent (HTMLPainter *painter,
	      GtkHTMLFontStyle style,
	      HTMLFontFace *face)
{
	HTMLPrinter *printer;
	GnomeFont *font;
	double descender;

	printer = HTML_PRINTER (painter);
	g_return_val_if_fail (printer->print_context != NULL, 0);

	font = html_painter_get_font (painter, face, style);
	g_return_val_if_fail (font != NULL, 0);

	descender = gnome_font_get_descender (font) * SPACING_FACTOR;
	return SCALE_GNOME_PRINT_TO_ENGINE (descender);
}

static guint
calc_text_width (HTMLPainter *painter,
		 const gchar *text,
		 guint len,
		 GtkHTMLFontStyle style,
		 HTMLFontFace *face)
{
	HTMLPrinter *printer;
	GnomeFont *font;
	double width;

	printer = HTML_PRINTER (painter);
	g_return_val_if_fail (printer->print_context != NULL, 0);

	font = html_painter_get_font (painter, face, style);
	g_return_val_if_fail (font != NULL, 0);

	width = gnome_font_get_width_utf8_sized (font, text, g_utf8_offset_to_pointer (text, len) - text);

	return SCALE_GNOME_PRINT_TO_ENGINE (width);
}

static guint
calc_text_width_bytes (HTMLPainter *painter,
		       const gchar *text,
		       guint len,
		       HTMLFont *font,
		       GtkHTMLFontStyle style)
{
	HTMLPrinter *printer;
	double width;

	printer = HTML_PRINTER (painter);
	g_return_val_if_fail (printer->print_context != NULL, 0);
	g_return_val_if_fail (font != NULL, 0);

	width = gnome_font_get_width_utf8_sized (font->data, text, len);

	return SCALE_GNOME_PRINT_TO_ENGINE (width);
}

static guint
get_pixel_size (HTMLPainter *painter)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);

	return SCALE_GNOME_PRINT_TO_ENGINE (PIXEL_SIZE);
}

static inline gdouble
get_font_size (gboolean points, gdouble size)
{
	return points ? size / 10 : size;
}

static HTMLFont *
alloc_font (gchar *face, gdouble size, gboolean points, GtkHTMLFontStyle style)
{
	GnomeFontWeight weight;
	GnomeFont *font;
	gboolean italic;
	gchar *family = NULL;

	weight = (style & GTK_HTML_FONT_STYLE_BOLD) ? GNOME_FONT_BOLD : GNOME_FONT_BOOK;
	italic = (style & GTK_HTML_FONT_STYLE_ITALIC);

	/* gnome-print is case sensitive - need to be fixed */
	if (face && *face) {
		gchar *s;

		s = family = html_font_manager_get_attr (face, 2);

		/* capitalize */
		*s = toupper (*s);
		s++;
		while (*s) {
			*s = tolower (*s);
			s++;
		}
	}

	font = gnome_font_new_closest (family ? family : (style & GTK_HTML_FONT_STYLE_FIXED ? "Courier" : "Helvetica"),
				       weight, italic, get_font_size (points, size));
	g_free (family);

	if (font == NULL) {
		GList *family_list;

		family_list = gnome_font_family_list ();
		if (family_list && family_list->data) {
			font = gnome_font_new_closest (family_list->data,
						       weight, italic, get_font_size (points, size));
			gnome_font_family_list_free (family_list);
		}
	}

	return font ? html_font_new (font,
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, " ", 1)),
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, "\xc2\xa0", 2)),
				     SCALE_GNOME_PRINT_FONT_TO_ENGINE (gnome_font_get_width_utf8_sized (font, "\t", 1)))
		: NULL;
}

static void
ref_font (HTMLFont *font)
{
	gtk_object_ref (GTK_OBJECT (font->data));
}

static void
unref_font (HTMLFont *font)
{
	gtk_object_unref (GTK_OBJECT (font->data));
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

static HTMLFontManagerId
get_font_manager_id ()
{
	return HTML_FONT_MANAGER_ID_PRINTER;
}

static void
init (GtkObject *object)
{
	HTMLPrinter *printer;

	printer                = HTML_PRINTER (object);
	printer->print_context = NULL;
	printer->scale         = 1.0;
}

static void
class_init (GtkObjectClass *object_class)
{
	HTMLPainterClass *painter_class;

	painter_class = HTML_PAINTER_CLASS (object_class);

	object_class->finalize = finalize;

	painter_class->begin = begin;
	painter_class->end = end;
	painter_class->alloc_font = alloc_font;
	painter_class->ref_font   = ref_font;
	painter_class->unref_font = unref_font;
	painter_class->alloc_color = alloc_color;
	painter_class->free_color = free_color;
	painter_class->calc_ascent = calc_ascent;
	painter_class->calc_descent = calc_descent;
	painter_class->calc_text_width = calc_text_width;
	painter_class->calc_text_width_bytes = calc_text_width_bytes;
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
	painter_class->get_font_manager_id = get_font_manager_id;

	parent_class = gtk_type_class (html_painter_get_type ());
}

GtkType
html_printer_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"HTMLPrinter",
			sizeof (HTMLPrinter),
			sizeof (HTMLPrinterClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		type = gtk_type_unique (HTML_TYPE_PAINTER, &info);
	}

	return type;
}


HTMLPainter *
html_printer_new (GnomePrintContext *print_context, GnomePrintMaster *print_master)
{
	HTMLPrinter *new;

	new = gtk_type_new (html_printer_get_type ());

	gtk_object_ref (GTK_OBJECT (print_context));
	new->print_context = print_context;
	new->print_master = print_master;

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
