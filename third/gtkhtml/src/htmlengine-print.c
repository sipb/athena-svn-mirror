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
#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-dialog-util.h>
#include "gtkhtml.h"
#include "gtkhtml-private.h"
#include "gtkhtml-properties.h"
#include "htmlprinter.h"
#include "htmlengine-print.h"
#include "htmlobject.h"



/* #define CLIP_DEBUG */

static void
print_header_footer (HTMLPainter *painter, HTMLEngine *engine, gint width, gint y, gdouble height,
		     GtkHTMLPrintCallback cb, gpointer user_data)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);
	GnomePrintContext *context = printer->print_context;
	gdouble gx, gy;

	gnome_print_gsave (context);
	html_painter_set_clip_rectangle (painter, 0, y,
					 width, SCALE_GNOME_PRINT_TO_ENGINE (height));
#ifdef CLIP_DEBUG
	html_painter_draw_rect (painter, 0, y,
				SCALE_GNOME_PRINT_TO_ENGINE (width), SCALE_GNOME_PRINT_TO_ENGINE (height));
#endif
	html_printer_coordinates_to_gnome_print (printer, 0, y, &gx, &gy);
	(*cb) (GTK_HTML (engine->widget), context, gx, gy, SCALE_ENGINE_TO_GNOME_PRINT (width), height, user_data);
	gnome_print_grestore (context);
}

static void
print_page (HTMLPainter *painter,
	    HTMLEngine *engine,
	    gint start_y,
	    gint page_width, gint page_height, gint body_height,
	    gdouble header_height, gdouble footer_height,
	    GtkHTMLPrintCallback header_print, GtkHTMLPrintCallback footer_print, gpointer user_data)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);
	GnomePrintContext *context = printer->print_context;

	html_painter_begin (painter, 0, 0, page_width, page_height);
	if (header_print)
		print_header_footer (painter, engine, page_width, 0, header_height/printer->scale, header_print, user_data);
	gnome_print_gsave (context);
	html_painter_set_clip_rectangle (painter, 0, header_height, page_width, body_height);
#ifdef CLIP_DEBUG
	html_painter_draw_rect (painter, 0, header_height, page_width, body_height);
#endif
	html_object_draw (engine->clue, painter,
			  0, start_y,
			  page_width, body_height,
			  0, -start_y + header_height / printer->scale);
	gnome_print_grestore (context);
	if (footer_print)
		print_header_footer (painter, engine, page_width*printer->scale,
				     page_height - SCALE_GNOME_PRINT_TO_ENGINE (footer_height/printer->scale),
				     footer_height/printer->scale, footer_print, user_data);
	html_painter_end (painter);
}

static gint
print_all_pages (HTMLPainter *painter,
		 HTMLEngine *engine,
		 gdouble header_height, gdouble footer_height,
		 GtkHTMLPrintCallback header_print, GtkHTMLPrintCallback footer_print, gpointer user_data, gboolean do_print)
{
	HTMLPrinter *printer = HTML_PRINTER (painter);
	gint new_split_offset, split_offset;
	gint page_width, page_height, body_height;
	gint document_height;
	gint pages = 0;

	page_height = html_printer_get_page_height (printer);
	page_width  = html_printer_get_page_width (printer);

	if (header_height + footer_height >= page_height*printer->scale) {
		header_print = footer_print = NULL;
		g_warning ("Page header height + footer height >= page height, disabling header/footer printing");
	}

	body_height = page_height
		- SCALE_GNOME_PRINT_TO_ENGINE (header_height/printer->scale + footer_height/printer->scale);
	split_offset = 0;

	document_height = html_engine_get_doc_height (engine);

	do {
		pages ++;
		new_split_offset = html_object_check_page_split (engine->clue,
								 split_offset + body_height);

		if (new_split_offset <= split_offset
		    || new_split_offset - split_offset < engine->min_split_index * body_height)
			new_split_offset = split_offset + body_height;

		if (do_print)
			print_page   (painter, engine, split_offset,
				      page_width, page_height,
				      new_split_offset - split_offset,
				      header_height, footer_height, header_print, footer_print, user_data);

		split_offset = new_split_offset;
	}  while (split_offset < document_height);

	return pages;
}

static gboolean
do_we_have_default_font (HTMLPainter *painter)
{
	HTMLFont *font;
	gboolean rv = FALSE;

	font = html_painter_get_font (painter, NULL, GTK_HTML_FONT_STYLE_DEFAULT);
	if (font) {
		rv = TRUE;
	}

	return rv;
}

static gint
print_with_header_footer (HTMLEngine *engine, 
				      GnomePrintContext *print_context,
				      gdouble header_height, 
				      gdouble footer_height,
				      GtkHTMLPrintCallback header_print, 
				      GtkHTMLPrintCallback footer_print, 
				      gpointer user_data, gboolean do_print)
{
	HTMLPainter *printer;
	HTMLPainter *old_painter;
	GtkHTMLClassProperties *prop = GTK_HTML_CLASS (GTK_OBJECT (engine->widget)->klass)->properties;
	gint pages = 0;

	g_return_val_if_fail (engine->clue != NULL, 0);

	printer = html_printer_new (print_context, GTK_HTML (engine->widget)->priv->print_master);
	html_font_manager_set_default (html_engine_font_manager_with_painter (engine, printer),
				       prop->font_var_print,      prop->font_fix_print,
				       prop->font_var_size_print, prop->font_var_print_points,
				       prop->font_fix_size_print, prop->font_fix_print_points);

	if (do_we_have_default_font (printer)) {
		gint min_width, page_width;

		old_painter = engine->painter;

		gtk_object_ref (GTK_OBJECT (old_painter));
		html_engine_set_painter (engine, printer);

		min_width = html_engine_calc_min_width (engine);
		page_width = html_painter_get_page_width (engine->painter, engine);
		/* printf ("min_width %d page %d\n", min_width, page_width); */
		if (min_width > page_width) {
			HTML_PRINTER (printer)->scale = MAX (0.5, ((gdouble) page_width) / min_width);
			html_object_change_set_down (engine->clue, HTML_CHANGE_ALL);
			html_engine_calc_size (engine, NULL);
			/* printf ("scale %lf\n", HTML_PRINTER (printer)->scale);
			   gtk_html_debug_dump_tree (engine->clue, 0); */
		}

		pages = print_all_pages (HTML_PAINTER (printer), engine,
					 header_height, footer_height,
					 header_print, footer_print,
					 user_data, do_print);

		html_engine_set_painter (engine, old_painter);
		gtk_object_unref (GTK_OBJECT (old_painter));
	} else {
		gnome_ok_dialog (_("Cannot allocate default font for printing\n"));
	}

	gtk_object_unref (GTK_OBJECT (printer));

	return pages;
}

void
html_engine_print (HTMLEngine *engine, GnomePrintContext *print_context)
{
	html_engine_print_with_header_footer (engine, print_context, .0, .0, NULL, NULL, NULL);
}

void
html_engine_print_with_header_footer (HTMLEngine *engine, 
				      GnomePrintContext *print_context,
				      gdouble header_height, 
				      gdouble footer_height,
				      GtkHTMLPrintCallback header_print, 
				      GtkHTMLPrintCallback footer_print, 
				      gpointer user_data)
{
	print_with_header_footer (engine, print_context,
				  header_height, footer_height, header_print, footer_print, user_data, TRUE);
}

gint
html_engine_print_get_pages_num (HTMLEngine *e, GnomePrintContext *print_context,
				 gdouble header_height, gdouble footer_height)
{
	return print_with_header_footer (e, print_context, header_height, footer_height, NULL, NULL, NULL, FALSE);
}

void
html_engine_print_set_min_split_index (HTMLEngine *e, gdouble idx)
{
	e->min_split_index = idx;
}
