/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-rgbp: driver that does banded RGB bitmap
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
 *  Copyright (C) 2000-2003 Ximian Inc.
 */

#include <config.h>
#include <string.h>
#include <math.h>

#include <libart_lgpl/art_rect.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gp-gc-private.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <libgnomeprint/gnome-print-rbuf.h>
#include <libgnomeprint/gnome-print-rgbp.h>

static void rgbp_init (GnomePrintRGBP *rgbp);
static void rgbp_class_init (GnomePrintRGBPClass *klass);

static void rgbp_finalize (GObject *object);

static int rgbp_beginpage (GnomePrintContext *pc, const guchar *name);
static int rgbp_showpage (GnomePrintContext *pc);

static int rgbp_gsave (GnomePrintContext *pc);
static int rgbp_grestore (GnomePrintContext *pc);

static int rgbp_clip (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule);
static int rgbp_fill (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule);
static int rgbp_stroke (GnomePrintContext *pc, const ArtBpath *bpath);

static int rgbp_image (GnomePrintContext *pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch);

static int rgbp_glyphlist (GnomePrintContext *pc, const gdouble *affine, GnomeGlyphList *gl);

static int rgbp_close (GnomePrintContext *pc);

static GnomePrintContextClass *parent_class;

GType
gnome_print_rgbp_get_type (void)
{
	static GType rgbp_type = 0;
	if (!rgbp_type) {
		static const GTypeInfo rgbp_info = {
			sizeof (GnomePrintRGBPClass),
			NULL, NULL,
			(GClassInitFunc) rgbp_class_init,
			NULL, NULL,
			sizeof (GnomePrintRGBP),
			0,
			(GInstanceInitFunc) rgbp_init
		};
		rgbp_type = g_type_register_static (GNOME_TYPE_PRINT_CONTEXT, "GnomePrintRGBP", &rgbp_info, 0);
	}
	return rgbp_type;
}

static void
rgbp_init (GnomePrintRGBP *rgbp)
{
	rgbp->meta = NULL;
}

static void
rgbp_class_init (GnomePrintRGBPClass *klass)
{
	GObjectClass *object_class;
	GnomePrintContextClass *pc_class;

	object_class= (GObjectClass *) klass;
	pc_class = (GnomePrintContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = rgbp_finalize;

	pc_class->showpage = rgbp_showpage;
	pc_class->beginpage = rgbp_beginpage;
	pc_class->gsave = rgbp_gsave;
	pc_class->grestore = rgbp_grestore;
	pc_class->clip = rgbp_clip;
	pc_class->fill = rgbp_fill;
	pc_class->stroke = rgbp_stroke;
	pc_class->image = rgbp_image;
	pc_class->glyphlist = rgbp_glyphlist;
	pc_class->close = rgbp_close;
}

static void
rgbp_finalize (GObject *object)
{
	GnomePrintRGBP *rgbp;

	rgbp = GNOME_PRINT_RGBP (object);

	if (rgbp->meta) {
		g_object_unref (G_OBJECT (rgbp->meta));
		rgbp->meta = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static int
rgbp_beginpage (GnomePrintContext *pc, const guchar *name)
{
	GnomePrintRGBP *rgbp;
	gint ret;

	if (((GnomePrintContextClass *) parent_class)->beginpage) {
		ret = (* ((GnomePrintContextClass *) parent_class)->beginpage) (pc, name);
		g_return_val_if_fail (ret != GNOME_PRINT_OK, ret);
	}

	rgbp = GNOME_PRINT_RGBP (pc);
	g_return_val_if_fail (rgbp->meta == NULL, GNOME_PRINT_ERROR_UNKNOWN);

	rgbp->meta = (GnomePrintContext *) gnome_print_meta_new ();
	gnome_print_beginpage (rgbp->meta, name);

	return GNOME_PRINT_OK;
}

static int
rgbp_showpage (GnomePrintContext *pc)
{
	GnomePrintRGBP *rgbp;
	gint width, height, bh;
	gint y;
	guchar *b;
	ArtIRect rect;
	gint ret;

	if (((GnomePrintContextClass *) parent_class)->showpage) {
		ret = (* ((GnomePrintContextClass *) parent_class)->showpage) (pc);
		g_return_val_if_fail (ret != GNOME_PRINT_OK, ret);
	}

	rgbp = GNOME_PRINT_RGBP (pc);
	g_return_val_if_fail (rgbp->meta != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	gnome_print_showpage (rgbp->meta);

	if (GNOME_PRINT_RGBP_GET_CLASS (rgbp)->page_begin)
		GNOME_PRINT_RGBP_GET_CLASS (rgbp)->page_begin (rgbp);

	width = ceil ((rgbp->margins.x1 - rgbp->margins.x0) * rgbp->dpix / 72.0);
	height = ceil ((rgbp->margins.y1 - rgbp->margins.y0) * rgbp->dpiy / 72.0);
	bh = rgbp->band_height;
	b = g_new (guchar, width * bh * 3);

	for (y = height; y > 0; y -= bh) {
		GnomePrintContext *rbuf;
		gdouble t[6];
		gint y1local;

		y1local = bh;

		rect.x0 = 0;
		rect.y0 = y - y1local;
		rect.x1 = width;
		rect.y1 = y;

		t[0] = rgbp->dpix / 72.0;
		t[1] = 0.0;
		t[2] = 0.0;
		t[3] = rgbp->dpiy / 72.0;
		t[4] = -rgbp->margins.x0 * rgbp->dpix / 72.0 - rect.x0;
		t[5] = -rgbp->margins.y0 * rgbp->dpiy / 72.0 - rect.y0;

		memset (b, 0xff, width * bh * 3);
		rbuf = gnome_print_rbuf_new (b, width, rect.y1 - rect.y0, width * 3, t, FALSE);
#ifdef VERBOSE
		g_print ("\nrgbp: %g %g %g %g %g %g\n", t[0], t[1], t[2], t[3], t[4], t[5]);
		g_print ("rgbp: %d %d %d %d\n\n", rect.x0, rect.y0, rect.x1, rect.y1);
#endif
		gnome_print_meta_render_data (rbuf,
					      gnome_print_meta_get_buffer (GNOME_PRINT_META (rgbp->meta)),
					      gnome_print_meta_get_length (GNOME_PRINT_META (rgbp->meta)));

		if (GNOME_PRINT_RGBP_GET_CLASS (rgbp)->print_band)
			GNOME_PRINT_RGBP_GET_CLASS (rgbp)->print_band (rgbp, b, &rect);
	}

	g_free (b);
	g_object_unref (G_OBJECT (rgbp->meta));
	rgbp->meta = NULL;

	if (GNOME_PRINT_RGBP_GET_CLASS (rgbp)->page_end)
		GNOME_PRINT_RGBP_GET_CLASS (rgbp)->page_end (rgbp);

	return GNOME_PRINT_OK;
}

static int
rgbp_gsave (GnomePrintContext *pc)
{
	return gnome_print_gsave (((GnomePrintRGBP *) pc)->meta);
}

static int
rgbp_grestore (GnomePrintContext *pc)
{
	return gnome_print_grestore (((GnomePrintRGBP *) pc)->meta);
}

static int
rgbp_clip (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	return gnome_print_clip_bpath_rule (((GnomePrintRGBP *) pc)->meta, bpath, rule);
}

static int
rgbp_fill (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintRGBP *rgbp;

	rgbp = GNOME_PRINT_RGBP (pc);

	gnome_print_setrgbcolor (rgbp->meta, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
	gnome_print_setopacity (rgbp->meta, gp_gc_get_opacity (pc->gc));
	return gnome_print_fill_bpath_rule (rgbp->meta, bpath, rule);
}

static int
rgbp_stroke (GnomePrintContext *pc, const ArtBpath *bpath)
{
	GnomePrintRGBP *rgbp;
	const ArtVpathDash *dash;

	rgbp = GNOME_PRINT_RGBP (pc);
	dash = gp_gc_get_dash (pc->gc);

	gnome_print_setrgbcolor (rgbp->meta, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
	gnome_print_setopacity (rgbp->meta, gp_gc_get_opacity (pc->gc));
	gnome_print_setlinewidth (rgbp->meta, gp_gc_get_linewidth (pc->gc));
	gnome_print_setmiterlimit (rgbp->meta, gp_gc_get_miterlimit (pc->gc));
	gnome_print_setlinejoin (rgbp->meta, gp_gc_get_linejoin (pc->gc));
	gnome_print_setlinecap (rgbp->meta, gp_gc_get_linecap (pc->gc));
	dash = gp_gc_get_dash (pc->gc);
	gnome_print_setdash (rgbp->meta, dash->n_dash, dash->dash, dash->offset);
	return gnome_print_stroke_bpath (rgbp->meta, bpath);
}

static int
rgbp_image (GnomePrintContext *pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch)
{
	return gnome_print_image_transform (((GnomePrintRGBP *) pc)->meta, affine, px, w, h, rowstride, ch);
}

static int
rgbp_glyphlist (GnomePrintContext *pc, const gdouble *affine, GnomeGlyphList *gl)
{
	return gnome_print_glyphlist_transform (((GnomePrintRGBP *) pc)->meta, affine, gl);
}

static int
rgbp_close (GnomePrintContext *pc)
{
	GnomePrintRGBP *rgbp;

	rgbp = GNOME_PRINT_RGBP (pc);

	if (rgbp->meta) {
		g_object_unref (G_OBJECT (rgbp->meta));
		rgbp->meta = NULL;
	}

	return GNOME_PRINT_OK;
}

gint
gnome_print_rgbp_construct (GnomePrintRGBP *rgbp, ArtDRect *margins, gdouble dpix, gdouble dpiy, gint band_height)
{
	g_return_val_if_fail (rgbp != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (GNOME_IS_PRINT_RGBP (rgbp), GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (margins != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (margins->x1 - margins->x0 >= 1.0, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (margins->y1 - margins->y0 >= 1.0, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (dpix >= 1.0, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (dpiy >= 1.0, GNOME_PRINT_ERROR_UNKNOWN);
	g_return_val_if_fail (band_height > 0, GNOME_PRINT_ERROR_UNKNOWN);

	rgbp->margins = *margins;
	rgbp->dpix = dpix;
	rgbp->dpiy = dpiy;
	rgbp->band_height = band_height;

	return GNOME_PRINT_OK;
}

GnomePrintContext *
gnome_print_rgbp_new (ArtDRect *margins, gdouble dpix, gdouble dpiy, gint band_height)
{
	GnomePrintRGBP *rgbp;
	gint ret;
	
	g_return_val_if_fail (margins != NULL, NULL);
	g_return_val_if_fail (margins->x1 - margins->x0 >= 1.0, NULL);
	g_return_val_if_fail (margins->y1 - margins->y0 >= 1.0, NULL);
	g_return_val_if_fail (dpix >= 1.0, NULL);
	g_return_val_if_fail (dpiy >= 1.0, NULL);
	g_return_val_if_fail (band_height > 0, NULL);

	rgbp = g_object_new (GNOME_TYPE_PRINT_RGBP, NULL);
	g_return_val_if_fail (rgbp != NULL, NULL);

	ret = gnome_print_rgbp_construct (rgbp, margins, dpix, dpiy, band_height);

	if (ret != GNOME_PRINT_OK) {
		g_object_unref (G_OBJECT (rgbp));
		return NULL;
	}
	
	return GNOME_PRINT_CONTEXT (rgbp);
}


