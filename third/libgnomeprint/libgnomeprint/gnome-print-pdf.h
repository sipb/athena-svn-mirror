/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-pdf-private.h: the PDF backend
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

#ifndef __GNOME_PRINT_PDF_H__
#define __GNOME_PRINT_PDF_H__

#include <glib.h>
#include <libgnomeprint/gnome-print.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_PDF  (gnome_print_pdf_get_type ())
#define GNOME_PRINT_PDF(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_PDF, GnomePrintPdf))
#define GNOME_IS_PRINT_PDF(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_PDF))

GType               gnome_print_pdf_get_type (void);
GnomePrintContext * gnome_print_pdf_new (GnomePrintConfig *config);

G_END_DECLS

#endif /* __GNOME_PRINT_PDF_H__ */
