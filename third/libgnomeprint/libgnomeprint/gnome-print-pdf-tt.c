/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-pdf-t1.c: Tyep1 Font embeding for gnome-print-pdf
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useoful,
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
 *
 *  Copyright 2000-2001 Ximian, Inc.
 *
 *  References:
 *    [1] Portable Document Format Referece Manual, Version 1.3 (March 11, 1999)
 */

#include <config.h>

#include <string.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-print-pdf-private.h>

gint
gnome_print_pdf_tt_embed (GnomePrintPdf *pdf,
			  const guchar *file_name,
			  gint *object_number_ret)
{
	GnomePrintReturnCode retval = GNOME_PRINT_ERROR_UNKNOWN;
	GnomePrintBuffer b = {NULL};
	gint object_number;

	g_return_val_if_fail (file_name != NULL, retval);

	if (gnome_print_buffer_mmap (&b, file_name))
		goto pdf_truetype_error;

	if (b.buf_size < 8)
		goto pdf_truetype_error;

	object_number = gnome_print_pdf_object_new (pdf);
	*object_number_ret = object_number;
	
	/* Write the object */
	gnome_print_pdf_object_start (pdf, object_number, FALSE);
	gnome_print_pdf_fprintf    (pdf,
				    "/Length %d" EOL
				    "/Length1 %d" EOL
				    ">>" EOL
				    "stream" EOL,
				    b.buf_size + strlen (EOL),
				    b.buf_size);
	gnome_print_pdf_print_sized (pdf, b.buf, b.buf_size);
	gnome_print_pdf_fprintf (pdf, EOL);
	gnome_print_pdf_fprintf (pdf,
				 "endstream" EOL
				 "endobj" EOL);
	gnome_print_pdf_object_end  (pdf, object_number, TRUE);

	retval = GNOME_PRINT_OK;

pdf_truetype_error:
	if (b.buf)
		gnome_print_buffer_munmap (&b);
	if (retval != GNOME_PRINT_OK)
		g_warning ("Could not parse Type1 font from %s\n", file_name);
	
	return retval;
}

