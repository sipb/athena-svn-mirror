/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-pdf-private.h: private structs of the PDF backend
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
 *    Chema Celorio <chema@celorio.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright 2000-2003 Ximian, Inc. and authors
 */

#ifndef __GNOME_PRINT_PDF_PRIVATE_H__
#define __GNOME_PRINT_PDF_PRIVATE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GnomePrintPdf GnomePrintPdf;
typedef struct _GnomePrintPdfFont   GnomePrintPdfFont;

#include <libgnomeprint/gnome-font-private.h>

struct _GnomePrintPdfFont {
	GnomeFontFace *face;
	GnomeFontPsObject *pso;
	guint is_basic_14 : 1;
	guint is_type_1 : 1; /* FALSE == TrueType */

	gint nglyphs;
	gint object_number;

	gint code_assigned; /* It is a counter of the last code assigned */
	GHashTable *glyph_to_code; /* Given a glyph, give me its code  */
	gint *code_to_glyph; /* Array of code->glyph conversion */
	
	gint object_number_encoding;
	gint object_number_widths;
	gint object_number_lastchar;
};

/* Write to stream & objects functions */
#define EOL "\r\n"

gint gnome_print_pdf_print_sized (GnomePrintPdf *pdf, const char *content, gint len);
gint gnome_print_pdf_fprintf     (GnomePrintPdf *pdf, const char *format, ...);
gint gnome_print_pdf_print_double (GnomePrintPdf *pdf, const gchar *format, gdouble x);

gint gnome_print_pdf_object_new   (GnomePrintPdf *pdf);
gint gnome_print_pdf_object_start (GnomePrintPdf *pdf, gint object_number, gboolean dont_print);
gint gnome_print_pdf_object_end   (GnomePrintPdf *pdf, gint object_number, gboolean dont_print);

/* Type 1 & TrueType embed */
void gnome_print_embed_pdf_font  (GnomePrintPdf *pdf, GnomePrintPdfFont *font);
gint gnome_print_pdf_t1_embed (GnomePrintPdf *pdf, const guchar *file_name, gint *object_number);
gint gnome_print_pdf_tt_embed (GnomePrintPdf *pdf, const guchar *file_name, gint *object_number);
gint gnome_print_pdf_tt_subset_embed (GnomePrintPdf *pdf, GnomePrintPdfFont *font, const guchar *file_name, gint *object_number);


G_END_DECLS

#endif /* __GNOME_PRINT_PDF_PRIVATE_H__ */
