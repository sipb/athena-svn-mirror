/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-rgb.hp: banded RGB bitmap backend
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
 *    Miguel de Icaza <miguel@gnu.org>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2001 Ximian Inc.
 */

#ifndef __GNOME_PRINT_RGBP_H__
#define __GNOME_PRINT_RGBP_H__

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_RGBP         (gnome_print_rgbp_get_type ())
#define GNOME_PRINT_RGBP(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_RGBP, GnomePrintRGBP))
#define GNOME_PRINT_RGBP_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k),    GNOME_TYPE_PRINT_RGBP, GnomePrintRGBPClass))
#define GNOME_IS_PRINT_RGBP(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_RGBP))
#define GNOME_IS_PRINT_RGBP_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k),    GNOME_TYPE_PRINT_RGBP))
#define GNOME_PRINT_RGBP_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  GNOME_TYPE_PRINT_RGBP, GnomePrintRGBPClass))

#include <libart_lgpl/art_rect.h>
#include <libgnomeprint/gnome-print-private.h>

typedef struct _GnomePrintRGBP GnomePrintRGBP;

struct _GnomePrintRGBP {
	GnomePrintContext pc;

	ArtDRect margins;
	gdouble dpix, dpiy;   /* Resolution */
	gint band_height;

	GnomePrintContext *meta;
};

typedef struct _GnomePrintRGBPClass GnomePrintRGBPClass;
struct _GnomePrintRGBPClass {
	GnomePrintContextClass parent_class;

	int (*print_init) (GnomePrintRGBP *rgbp, gdouble dpix, gdouble dpiy);
	int (*page_begin) (GnomePrintRGBP *rgbp);
	int (*page_end)   (GnomePrintRGBP *rgbp);
	int (*print_band) (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *rect);
};

GType gnome_print_rgbp_get_type (void);

GnomePrintContext * gnome_print_rgbp_new (ArtDRect *margins, gdouble dpix, gdouble dpiy, gint band_height);

gint gnome_print_rgbp_construct (GnomePrintRGBP *rgbp, ArtDRect *margins, gdouble dpix, gdouble dpiy, gint band_height);

G_END_DECLS

#endif /* __GNOME_PRINT_RGBP_H__ */

