/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-rbuf.c: Driver that renders into transformed rectangular RGB(A) buffer
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
 *    Lauris Kaplinski <lauris@ariman.ee>
 *
 *  Copyright (C) 2000-2001 Ximian Inc. and authors
 *
 */

#define __GNOME_PRINT_RBUF_C__

/*
 * TODO
 *
 * - figure out, how is path & currentpoint handled during fill, clip, show...
 * - implement new libart rendering/clipping, when available
 * - glyph outline cache (should be in gnome-font)
 * - are dashes in device or current coordinates?
 *
 */

#include <math.h>
#include <string.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>
#include <libart_lgpl/art_svp_ops.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libart_lgpl/art_vpath_dash.h>
#include <libart_lgpl/art_vpath_svp.h>
#include <libart_lgpl/art_rgb_svp.h>
#include <libart_lgpl/art_rgb_rgba_affine.h>

#include <libgnomeprint/gp-gc.h>
#include "art_rgba_svp.h"
#include "art_rgba_rgba_affine.h"

#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-rbuf.h>
#include <libgnomeprint/gnome-print-rbuf-private.h>
#include <libgnomeprint/gnome-rfont.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-pgl.h>
#include <libgnomeprint/gnome-pgl-private.h>
#include <libgnomeprint/gp-gc-private.h>

/*
 * Private structures
 */

struct _GnomePrintRBufPrivate {
	guchar * pixels;
	gint width;
	gint height;
	gint rowstride;

	gdouble page2buf[6];
	guint32 alpha : 1;
};

static void gpb_class_init (GnomePrintRBufClass *class);
static void gpb_init (GnomePrintRBuf *rbuf);
static void gpb_finalize (GObject *object);

static gint gpb_beginpage (GnomePrintContext * pc, const guchar *name);
static gint gpb_showpage (GnomePrintContext * pc);

static gint gpb_clip (GnomePrintContext * pc, const ArtBpath *bpath, ArtWindRule rule);
static gint gpb_fill (GnomePrintContext * pc, const ArtBpath *bpath, ArtWindRule rule);
static gint gpb_stroke (GnomePrintContext * pc, const ArtBpath *bpath);

static gint gpb_image (GnomePrintContext * pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch);

static gint gpb_glyphlist (GnomePrintContext * pc, const gdouble *affine, GnomeGlyphList *gl);

static gint gpb_close (GnomePrintContext * pc);

static void gp_svp_uncross_to_render (GnomePrintContext * pc, const ArtSVP * svp, ArtWindRule rule);
static void gp_vpath_to_render (GnomePrintContext * pc, const ArtBpath * bpath, ArtWindRule rule);

static void gp_render_silly_rgba (GnomePrintContext * pc, const gdouble *affine, const guchar * px, gint w, gint h, gint rowstride);

static GnomePrintContextClass *parent_class;

/**
 * gnome_print_rbuf_get_type:
 *
 * GType identification routine for #GnomePrintRBuf
 *
 * Returns: The GType for the #GnomePrintRBuf object
 */

GType
gnome_print_rbuf_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintRBufClass),
			NULL, NULL,
			(GClassInitFunc) gpb_class_init,
			NULL, NULL,
			sizeof (GnomePrintRBuf),
			0,
			(GInstanceInitFunc) gpb_init
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_CONTEXT, "GnomePrintRBuf", &info, 0);
	}
	return type;
}

static void
gpb_class_init (GnomePrintRBufClass *klass)
{
	GObjectClass * object_class;
	GnomePrintContextClass * pc_class;

	object_class = (GObjectClass *) klass;
	pc_class = (GnomePrintContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpb_finalize;
	
	pc_class->beginpage = gpb_beginpage;
	pc_class->showpage = gpb_showpage;

	pc_class->clip = gpb_clip;
	pc_class->fill = gpb_fill;
	pc_class->stroke = gpb_stroke;

	pc_class->image = gpb_image;

	pc_class->glyphlist = gpb_glyphlist;

	pc_class->close = gpb_close;
}

static void
gpb_init (GnomePrintRBuf * rbuf)
{
	rbuf->private = g_new (GnomePrintRBufPrivate, 1);

	rbuf->private->pixels = NULL;
	art_affine_identity (rbuf->private->page2buf);
}

static void
gpb_finalize (GObject *object)
{
	GnomePrintRBuf * rbuf;

	rbuf = GNOME_PRINT_RBUF (object);

	if (rbuf->private) {
		g_free (rbuf->private);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*
 * Beginpage
 *
 * fixme: Currently we simply clear rbuffer
 *
 * showpage, close - do nothing
 */

static gint
gpb_beginpage (GnomePrintContext *pc, const guchar *name)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	return GNOME_PRINT_OK;
}

static gint
gpb_showpage (GnomePrintContext * pc)
{
	GnomePrintRBuf * rbuf;

	rbuf = GNOME_PRINT_RBUF (pc);

	return GNOME_PRINT_OK;
}

static gint
gpb_clip (GnomePrintContext * pc, const ArtBpath *bpath, ArtWindRule rule)
{
#ifdef __GNUC__
#warning Why is are we not clipping?
#endif
	
#if 0
	GnomePrintRBuf * rbuf;

	rbuf = GNOME_PRINT_RBUF (pc);

	if (rule == ART_WIND_RULE_NONZERO) {
		gp_gc_clip (pc->gc);
	} else {
		gp_gc_eoclip (pc->gc);
	}
#endif

	return GNOME_PRINT_OK;
}

static gint
gpb_fill (GnomePrintContext * pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;
	ArtBpath * abp;

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	abp = art_bpath_affine_transform (bpath, rbp->page2buf);
	gp_vpath_to_render (pc, abp, rule);
	art_free (abp);

	return 1;
}

static gint
gpb_stroke (GnomePrintContext * pc, const ArtBpath *bpath)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;
	ArtBpath * abp;
	ArtVpath * vpath, * pvpath;
	ArtSVP * svp;
	const ArtVpathDash * dash;
	gdouble linewidth;

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	abp = art_bpath_affine_transform (bpath, rbp->page2buf);
	vpath = art_bez_path_to_vec (abp, 0.25);
	art_free (abp);

	pvpath = art_vpath_perturb (vpath);
	art_free (vpath);

	dash = gp_gc_get_dash (pc->gc);

	if ((dash->n_dash > 0) && (dash->dash != NULL)) {
		ArtVpath * dvp;
		dvp = art_vpath_dash (pvpath, dash);
		g_assert (dvp != NULL);
		art_free (pvpath);
		pvpath = dvp;
	}

	/* fixme */

	linewidth = gp_gc_get_linewidth (pc->gc);

	svp = art_svp_vpath_stroke (pvpath,
		gp_gc_get_linejoin (pc->gc),
		gp_gc_get_linecap (pc->gc),
		linewidth,
		gp_gc_get_miterlimit (pc->gc),
		0.25);
	g_assert (svp != NULL);
	art_free (pvpath);

	gp_svp_uncross_to_render (pc, svp, ART_WIND_RULE_NONZERO);

	art_svp_free (svp);

	return 1;
}

static gint
gpb_image (GnomePrintContext * pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;
	art_u8 * ib, * ipd;
	const art_u8 * ips;
	gint x, y;

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	if (ch == 1) {
		ib = g_new (art_u8, w * h * 4);
		for (y = 0; y < h; y++) {
			ips = px + y * rowstride;
			ipd = ib + y * w * 4;
			for (x = 0; x < w; x++) {
				*ipd++ = *ips;
				*ipd++ = *ips;
				*ipd++ = *ips++;
				*ipd++ = 0xff;
			}
		}
		gp_render_silly_rgba (pc, affine, ib, w, h, w * 4);
		g_free (ib);
	} else if (ch == 3) {
		ib = g_new (art_u8, w * h * 4);
		for (y = 0; y < h; y++) {
			ips = px + y * rowstride;
			ipd = ib + y * w * 4;
			for (x = 0; x < w; x++) {
				* ipd++ = *ips++;
				* ipd++ = *ips++;
				* ipd++ = *ips++;
				* ipd++ = 0xff;
			}
		}
		gp_render_silly_rgba (pc, affine, ib, w, h, w * 4);
		g_free (ib);
	} else {
		gp_render_silly_rgba (pc, affine, px, w, h, rowstride);
	}

	return 1;
}

/* fixme: */

static gint
gpb_glyphlist (GnomePrintContext * pc, const gdouble *affine, GnomeGlyphList *gl)
{
	GnomePrintRBuf *rbuf;
	GnomePosGlyphList *pgl;
	gdouble t[6];

	rbuf = (GnomePrintRBuf *) pc;

	art_affine_multiply (t, affine, rbuf->private->page2buf);

	pgl = gnome_pgl_from_gl (gl, t, 0);

	if (rbuf->private->alpha) {
		gnome_rfont_render_pgl_rgba8 (pgl, 0, 0, rbuf->private->pixels,
					      rbuf->private->width, rbuf->private->height, rbuf->private->rowstride, 0);
	} else {
		gnome_rfont_render_pgl_rgb8 (pgl, 0, 0, rbuf->private->pixels,
					     rbuf->private->width, rbuf->private->height, rbuf->private->rowstride, 0);
	}

	return 1;
}

static gint
gpb_close (GnomePrintContext * pc)
{
	return 1;
}

static GnomePrintRBuf *
gnome_print_rbuf_construct (GnomePrintRBuf * rbuf,
	guchar * pixels,
	gint width,
	gint height,
	gint rowstride,
	gdouble page2buf[6],
	gboolean alpha)
{
	g_return_val_if_fail (rbuf != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (rbuf), NULL);
	g_return_val_if_fail (pixels != NULL, NULL);
	g_return_val_if_fail (width > 0, NULL);
	g_return_val_if_fail (height > 0, NULL);
	g_return_val_if_fail (rowstride >= 3 * width, NULL);
	g_return_val_if_fail (page2buf != NULL, NULL);

	g_assert (rbuf->private != NULL);

	rbuf->private->pixels = pixels;
	rbuf->private->width = width;
	rbuf->private->height = height;
	rbuf->private->rowstride = rowstride;
	rbuf->private->alpha = alpha;

	memcpy (rbuf->private->page2buf, page2buf, sizeof (gdouble) * 6);

	return rbuf;
}

/* Constructors */

GnomePrintContext *
gnome_print_rbuf_new (guchar * pixels,
	gint width,
	gint height,
	gint rowstride,
	gdouble page2buf[6],
	gboolean alpha)
	
{
	GnomePrintRBuf * rbuf;

	g_return_val_if_fail (pixels != NULL, NULL);
	g_return_val_if_fail (width > 0, NULL);
	g_return_val_if_fail (height > 0, NULL);
	g_return_val_if_fail (rowstride >= 3 * width, NULL);
	g_return_val_if_fail (page2buf != NULL, NULL);

	rbuf = g_object_new (GNOME_TYPE_PRINT_RBUF, NULL);

	if (!gnome_print_rbuf_construct (rbuf, pixels, width, height, rowstride, page2buf, alpha)) {
		g_object_unref (G_OBJECT (rbuf));
	}

	return GNOME_PRINT_CONTEXT (rbuf);
}

/* Private helpers */

static void
gp_svp_uncross_to_render (GnomePrintContext * pc, const ArtSVP * svp, ArtWindRule rule)
{
	GnomePrintRBufPrivate * rbp;
	ArtSVP * svp1, * svp2;

	g_assert (pc != NULL);
	g_assert (svp != NULL);

	rbp = GNOME_PRINT_RBUF (pc)->private;

	svp2 = art_svp_uncross ((ArtSVP *) svp);
	g_assert (svp2 != NULL);

	svp1 = art_svp_rewind_uncrossed (svp2, rule);
	g_assert (svp1 != NULL);
	art_svp_free (svp2);

	if (gp_gc_has_clipsvp (pc->gc)) {
		svp2 = art_svp_intersect (svp1, gp_gc_get_clipsvp (pc->gc));
		g_assert (svp2 != NULL);
		art_svp_free (svp1);
		svp1 = svp2;
	}

	if (rbp->alpha) {
		art_rgba_svp_alpha (svp1,
			0, 0, rbp->width, rbp->height,
			gp_gc_get_rgba (pc->gc),
			rbp->pixels, rbp->rowstride,
			NULL);
	} else {
		art_rgb_svp_alpha (svp1,
			0, 0, rbp->width, rbp->height,
			gp_gc_get_rgba (pc->gc),
			rbp->pixels, rbp->rowstride,
			NULL);
	}

	art_svp_free (svp1);
}

static void
gp_vpath_to_render (GnomePrintContext * pc, const ArtBpath * bpath, ArtWindRule rule)
{
	GnomePrintRBufPrivate * rbp;
	ArtVpath * vpath1, * vpath2;
	ArtSVP * svp;

	g_assert (pc != NULL);
	g_assert (bpath != NULL);

	rbp = GNOME_PRINT_RBUF (pc)->private;

	vpath1 = art_bez_path_to_vec (bpath, 0.25);
	g_assert (vpath1 != NULL);

	vpath2 = art_vpath_perturb (vpath1);
	g_assert (vpath2 != NULL);
	art_free (vpath1);

	svp = art_svp_from_vpath (vpath2);
	g_assert (svp != NULL);
	art_free (vpath2);

	gp_svp_uncross_to_render (pc, svp, rule);

	art_svp_free (svp);
}

static void
gp_render_silly_rgba (GnomePrintContext * pc, const gdouble *affine, const guchar * px, gint w, gint h, gint rowstride)
{
	static const ArtVpath vp[] = {{ART_MOVETO, 0.0, 0.0},
				      {ART_LINETO, 1.0, 0.0},
				      {ART_LINETO, 1.0, 1.0},
				      {ART_LINETO, 0.0, 1.0},
				      {ART_LINETO, 0.0, 0.0},
				      {ART_END, 0.0, 0.0}};
	gdouble imga[6];
	GnomePrintRBuf *rbuf;
	gdouble a[6];
	ArtVpath * vpath, * pvpath;
	ArtSVP * svp1, * svp2;
	ArtDRect bbox, pbox;
	ArtIRect ibox;
	gdouble ba[6];
	guchar * cbuf, * ibuf;
	guchar * p, * ip, * cp;
	gint bw, bh, x, y;

	rbuf = GNOME_PRINT_RBUF (pc);

	art_affine_multiply (a, affine, rbuf->private->page2buf);
	vpath = art_vpath_affine_transform (vp, a);

	art_affine_scale (imga, 1.0 / w, -1.0 / h);
	imga[5] = 1.0;
	art_affine_multiply (a, imga, affine);
	art_affine_multiply (a, a, rbuf->private->page2buf);

	pvpath = art_vpath_perturb (vpath);
	art_free (vpath);
	svp1 = art_svp_from_vpath (pvpath);
	art_free (pvpath);
	svp2 = art_svp_uncross (svp1);
	art_svp_free (svp1);
	svp1 = art_svp_rewind_uncrossed (svp2, ART_WIND_RULE_NONZERO);
	art_svp_free (svp2);

	/* fixme: */
	if (gp_gc_has_clipsvp (pc->gc)) {
		svp2 = art_svp_intersect (svp1, gp_gc_get_clipsvp (pc->gc));
		art_svp_free (svp1);
		svp1 = svp2;
	}

	art_drect_svp (&bbox, svp1);

	pbox.x0 = pbox.y0 = 0.0;
	pbox.x1 = rbuf->private->width;
	pbox.y1 = rbuf->private->height;

	art_drect_intersect (&bbox, &bbox, &pbox);

	if (art_drect_empty (&bbox)) {
		art_svp_free (svp1);
		return;
	}

	art_drect_to_irect (&ibox, &bbox);

	bw = ibox.x1 - ibox.x0;
	bh = ibox.y1 - ibox.y0;

	/* Create coverage */

	cbuf = g_new (guchar, bw * bh * 4);
	for (y = 0; y < bh; y++) {
		p = cbuf + y * bw * 4;
		for (x = 0; x < bw; x++) {
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
		}
	}

	art_rgba_svp_alpha (svp1,
		ibox.x0, ibox.y0, ibox.x1, ibox.y1,
		0xffffffff,
		cbuf, bw * 4,
		NULL);

	art_svp_free (svp1);

	/* Create image */

	ibuf = g_new (guchar, bw * bh * 4);
	for (y = 0; y < bh; y++) {
		p = ibuf + y * bw * 4;
		for (x = 0; x < bw; x++) {
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
		}
	}

	memcpy (ba, a, 6 * sizeof (gdouble));
	ba[4] -= ibox.x0;
	ba[5] -= ibox.y0;

	art_rgba_rgba_affine (ibuf,
		0, 0, bw, bh, bw * 4,
		px, w, h, rowstride,
		ba,
		ART_FILTER_NEAREST, NULL);

	/* Composite */

	for (y = 0; y < bh; y++) {
		ip = ibuf + y * bw * 4;
		cp = cbuf + y * bw * 4;
		for (x = 0; x < bw; x++) {
			ip += 3;
			cp += 3;
			*ip = (*ip) * (*cp) >> 8;
			ip++;
			cp++;
		}
	}

	art_affine_translate (ba, ibox.x0, ibox.y0);

	/* Render */

	if (rbuf->private->alpha) {
		art_rgba_rgba_affine (rbuf->private->pixels,
			0, 0, rbuf->private->width, rbuf->private->height, rbuf->private->rowstride,
			ibuf, bw, bh, bw * 4,
			ba,
			ART_FILTER_NEAREST, NULL);
	} else {
		art_rgb_rgba_affine (rbuf->private->pixels,
			0, 0, rbuf->private->width, rbuf->private->height, rbuf->private->rowstride,
			ibuf, bw, bh, bw * 4,
			ba,
			ART_FILTER_NEAREST, NULL);
	}

	g_free (ibuf);
	g_free (cbuf);
}


