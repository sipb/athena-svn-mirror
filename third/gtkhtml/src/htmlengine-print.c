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
#include "gtkhtml.h"
#include "gtkhtml-properties.h"
#include "htmlprinter.h"
#include "htmlengine-print.h"
#include "htmlobject.h"



/* #define CLIP_DEBUG */

static void
print_header_footer (HTMLPainter *painter, HTMLEngine *engine, gint width, gint y, gint height,
		     GtkHTMLPrintCallback cb, gpointer user_data)
{
	GnomePrintContext *context = HTML_PRINTER (painter)->print_context;
	gdouble gx, gy;

	gnome_print_gsave (context);
	html_painter_set_clip_rectangle (painter, 0, y, width, height);
#ifdef CLIP_DEBUG
	html_painter_draw_rect (painter, 0, y, width, height);
#endif
	html_printer_coordinates_to_gnome_print (HTML_PRINTER (painter), 0, y, &gx, &gy);
	(*cb) (GTK_HTML (engine->widget), context, gx, gy,
	       html_printer_scale_to_gnome_print (width), html_printer_scale_to_gnome_print (height), user_data);
	gnome_print_grestore (context);
}

static void
print_page (HTMLPainter *painter,
	    HTMLEngine *engine,
	    gint start_y,
	    gint page_width, gint page_height, gint body_height,
	    gint header_height, gint footer_height,
	    GtkHTMLPrintCallback header_print, GtkHTMLPrintCallback footer_print, gpointer user_data)
{
	GnomePrintContext *context = HTML_PRINTER (painter)->print_context;

	html_painter_begin (painter, 0, 0, page_width, page_height);
	if (header_print)
		print_header_footer (painter, engine, page_width, 0, header_height, header_print, user_data);
	gnome_print_gsave (context);
	html_painter_set_clip_rectangle (painter, 0, header_height, page_width, body_height);
#ifdef CLIP_DEBUG
	html_painter_draw_rect (painter, 0, header_height, page_width, body_height);
#endif
	html_object_draw (engine->clue, painter,
			  0, start_y + header_height,
			  page_width, body_height,
			  0, -start_y);
	gnome_print_grestore (context);
	if (footer_print)
		print_header_footer (painter, engine, page_width, page_height - footer_height, footer_height,
				     footer_print, user_data);
	html_painter_end (painter);
}

static void
print_all_pages (HTMLPainter *printer,
		 HTMLEngine *engine,
		 gdouble header_height_ratio, gdouble footer_height_ratio,
		 GtkHTMLPrintCallback header_print, GtkHTMLPrintCallback footer_print, gpointer user_data)
{
	gint new_split_offset, split_offset;
	gint page_width, page_height, header_height, body_height, footer_height;
	gint document_height;

	if (header_height_ratio + footer_height_ratio >= 1.0) {
		header_print = footer_print = NULL;
		g_warning ("Page header height + footer height >= 1, disabling header/footer printing");
	}

	page_height = html_printer_get_page_height (HTML_PRINTER (printer));
	page_width  = html_printer_get_page_width (HTML_PRINTER (printer));

	/* calc header/footer heights */
	header_height = (header_print) ? header_height_ratio * page_height : 0;
	footer_height = (footer_print) ? footer_height_ratio * page_height : 0;
	body_height   = page_height - header_height - footer_height;

	split_offset = 0;

	document_height = html_engine_get_doc_height (engine);

	do {
		new_split_offset = html_object_check_page_split (engine->clue,
								 split_offset + body_height);

		if (new_split_offset <= split_offset
		    || new_split_offset - split_offset < engine->min_split_index * body_height)
			new_split_offset = split_offset + body_height;

		print_page   (printer, engine, split_offset,
			      page_width, page_height,
			      new_split_offset - split_offset,
			      header_height, footer_height, header_print, footer_print, user_data);

		split_offset = new_split_offset;
	}  while (split_offset < document_height);
}


void
html_engine_print (HTMLEngine *engine,
		   GnomePrintContext *print_context)
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
	HTMLPainter *printer;
	HTMLPainter *old_painter;
	GtkHTMLClassProperties *prop = GTK_HTML_CLASS (GTK_OBJECT (engine->widget)->klass)->properties;

	g_return_if_fail (engine->clue != NULL);

	old_painter = engine->painter;
	printer     = html_printer_new (print_context);
	html_font_manager_set_default (&printer->font_manager,
				       prop->font_var_print,      prop->font_fix_print,
				       prop->font_var_size_print, prop->font_fix_size_print);

	gtk_object_ref (GTK_OBJECT (old_painter));
	html_engine_set_painter (engine, printer);

	print_all_pages (HTML_PAINTER (printer), 
			 engine, 
			 header_height,
			 footer_height, 
			 header_print, 
			 footer_print, 
			 user_data);

	html_engine_set_painter (engine, old_painter);
	gtk_object_unref (GTK_OBJECT (old_painter));
	gtk_object_unref (GTK_OBJECT (printer));	
}

void
html_engine_print_set_min_split_index (HTMLEngine *e, gdouble idx)
{
	e->min_split_index = idx;
}
