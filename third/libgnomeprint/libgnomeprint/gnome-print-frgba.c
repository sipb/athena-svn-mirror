/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-frgba: Wrapper context that renders semitransparent objects as bitmaps
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
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian Inc.
 */

#include <config.h>
#include <math.h>
#include <string.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>

#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gp-gc-private.h>
#include <libgnomeprint/gnome-print-rbuf.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <libgnomeprint/gnome-print-frgba.h>

#define GP_RENDER_DPI 72.0

struct _GnomePrintFRGBA {
	GnomePrintContext pc;

	GnomePrintContext *ctx;
	GnomePrintContext *meta;
};

struct _GnomePrintFRGBAClass {
	GnomePrintContextClass parent_class;
};

static void gnome_print_frgba_class_init (GnomePrintFRGBAClass *class);
static void gnome_print_frgba_init (GnomePrintFRGBA *frgba);
static void gnome_print_frgba_finalize (GObject *object);

static gint gpf_beginpage (GnomePrintContext * pc, const guchar * name);
static gint gpf_showpage (GnomePrintContext * pc);
static gint gpf_end_doc (GnomePrintContext * pc);

static gint gpf_gsave (GnomePrintContext * pc);
static gint gpf_grestore (GnomePrintContext * pc);

static gint gpf_clip (GnomePrintContext * pc, const ArtBpath *bpath, ArtWindRule rule);
static gint gpf_fill (GnomePrintContext * pc, const ArtBpath *bpath, ArtWindRule rule);
static gint gpf_stroke (GnomePrintContext * pc, const ArtBpath *bpath);

static gint gpf_image (GnomePrintContext * pc, const gdouble *affine, const guchar * px, gint w, gint h, gint rowstride, gint ch);

static gint gpf_glyphlist (GnomePrintContext * pc, const gdouble *affine, GnomeGlyphList *gl);

static gint gpf_close (GnomePrintContext * pc);

static ArtDRect * gpf_bpath_bbox (const ArtBpath * bpath, ArtDRect * box);
static void gpf_render_buf (GnomePrintFRGBA * frgba, ArtDRect * box);

static GnomePrintContextClass *parent_class;

/**
 * gnome_print_frgba_get_type:
 *
 * Gtype identification routine for #GnomePrintFRGBA
 *
 * Returns: The Gtype for the #GnomePrintFRGBA object
 */

GType
gnome_print_frgba_get_type (void)
{
	static GType frgba_type = 0;
	if (!frgba_type) {
		static const GTypeInfo frgba_info = {
			sizeof (GnomePrintFRGBAClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_frgba_class_init,
			NULL, NULL,
			sizeof (GnomePrintFRGBA),
			0,
			(GInstanceInitFunc) gnome_print_frgba_init
		};
		frgba_type = g_type_register_static (GNOME_TYPE_PRINT_CONTEXT, "GnomePrintFRGBA", &frgba_info, 0);
	}

	return frgba_type;
}

static void
gnome_print_frgba_class_init (GnomePrintFRGBAClass *klass)
{
	GObjectClass * object_class;
	GnomePrintContextClass * pc_class;

	object_class = (GObjectClass *) klass;
	pc_class = (GnomePrintContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_print_frgba_finalize;
	
	pc_class->beginpage = gpf_beginpage;
	pc_class->showpage = gpf_showpage;
	pc_class->end_doc = gpf_end_doc;

	pc_class->gsave = gpf_gsave;
	pc_class->grestore = gpf_grestore;

	pc_class->clip = gpf_clip;
	pc_class->fill = gpf_fill;
	pc_class->stroke = gpf_stroke;

	pc_class->image = gpf_image;

	pc_class->glyphlist = gpf_glyphlist;

	pc_class->close = gpf_close;
}

static void
gnome_print_frgba_init (GnomePrintFRGBA * frgba)
{
	frgba->ctx = NULL;
	frgba->meta = NULL;
}

static void
gnome_print_frgba_finalize (GObject *object)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (object);

	if (frgba->ctx) {
		g_object_unref (G_OBJECT (frgba->ctx));
		frgba->ctx = NULL;
	}
	if (frgba->meta) {
		g_object_unref (G_OBJECT (frgba->meta));
		frgba->meta = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gpf_beginpage (GnomePrintContext * pc, const guchar * name)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_beginpage (frgba->meta, name);
	return gnome_print_beginpage (frgba->ctx, name);
}

static gint
gpf_showpage (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	/* Release old meta */
	g_object_unref (G_OBJECT (frgba->meta));
	/* Create fresh meta */
	frgba->meta = (GnomePrintContext *) gnome_print_meta_new ();
	/* Showpage */
	return gnome_print_showpage (frgba->ctx);
}

static gint
gpf_gsave (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_gsave (frgba->meta);
	return gnome_print_gsave (frgba->ctx);
}

static gint
gpf_grestore (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_grestore (frgba->meta);
	return gnome_print_grestore (frgba->ctx);
}

static gint
gpf_end_doc (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_end_doc (frgba->meta);
	return gnome_print_end_doc (frgba->ctx);
}

static gint
gpf_clip (GnomePrintContext * pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_clip_bpath_rule (frgba->meta, bpath, rule);
	return gnome_print_clip_bpath_rule (frgba->ctx, bpath, rule);
}

static gint
gpf_fill (GnomePrintContext * pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setrgbcolor (frgba->meta, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
	gnome_print_setopacity (frgba->meta, gp_gc_get_opacity (pc->gc));
	gnome_print_fill_bpath_rule (frgba->meta, bpath, rule);

	if (gp_gc_get_opacity (pc->gc) <= (255.0 / 256.0)) {
		/* We have alpha! */
		ArtDRect bbox;
		/* fixme: We need clipping here */
		gpf_bpath_bbox (bpath, &bbox);
		gnome_print_gsave (frgba->ctx);
		gnome_print_clip_bpath_rule (frgba->ctx, bpath, rule);
		gpf_render_buf (frgba, &bbox);
		gnome_print_grestore (frgba->ctx);
	} else {
		gnome_print_setrgbcolor (frgba->ctx, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
		gnome_print_setopacity (frgba->ctx, gp_gc_get_opacity (pc->gc));
		return gnome_print_fill_bpath_rule (frgba->ctx, bpath, rule);
	}

	return GNOME_PRINT_OK;
}

/* fixme: do buffering - but we have to find the right bbox! */

static gint
gpf_stroke (GnomePrintContext * pc, const ArtBpath *bpath)
{
	GnomePrintFRGBA * frgba;
	const ArtVpathDash * dash;

	frgba = GNOME_PRINT_FRGBA (pc);

	dash = gp_gc_get_dash (pc->gc);

	gnome_print_setrgbcolor (frgba->meta, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
	gnome_print_setopacity (frgba->meta, gp_gc_get_opacity (pc->gc));
	gnome_print_setlinewidth (frgba->meta, gp_gc_get_linewidth (pc->gc));
	gnome_print_setmiterlimit (frgba->meta, gp_gc_get_miterlimit (pc->gc));
	gnome_print_setlinejoin (frgba->meta, gp_gc_get_linejoin (pc->gc));
	gnome_print_setlinecap (frgba->meta, gp_gc_get_linecap (pc->gc));
	gnome_print_setdash (frgba->meta, dash->n_dash, dash->dash, dash->offset);

	gnome_print_setrgbcolor (frgba->ctx, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
	gnome_print_setopacity (frgba->ctx, gp_gc_get_opacity (pc->gc));
	gnome_print_setlinewidth (frgba->ctx, gp_gc_get_linewidth (pc->gc));
	gnome_print_setmiterlimit (frgba->ctx, gp_gc_get_miterlimit (pc->gc));
	gnome_print_setlinejoin (frgba->ctx, gp_gc_get_linejoin (pc->gc));
	gnome_print_setlinecap (frgba->ctx, gp_gc_get_linecap (pc->gc));
	gnome_print_setdash (frgba->ctx, dash->n_dash, dash->dash, dash->offset);

	gnome_print_stroke_bpath (frgba->meta, bpath);
	return gnome_print_stroke_bpath (frgba->ctx, bpath);
}

static gint
gpf_image (GnomePrintContext * pc, const gdouble *affine, const guchar * px, gint w, gint h, gint rowstride, gint ch)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_image_transform (frgba->meta, affine, px, w, h, rowstride, ch);

	if ((ch == 1) || (ch == 3)) {
		return gnome_print_image_transform (frgba->ctx, affine, px, w, h, rowstride, ch);
	} else {
		ArtDRect bbox;
		ArtPoint p;

		gnome_print_gsave (frgba->ctx);
		gnome_print_newpath (frgba->ctx);
		p.x = 0.0;
		p.y = 0.0;
		art_affine_point (&p, &p, affine);
		gnome_print_moveto (frgba->ctx, p.x, p.y);
		bbox.x0 = bbox.x1 = p.x;
		bbox.y0 = bbox.y1 = p.y;
		p.x = 1.0;
		p.y = 0.0;
		art_affine_point (&p, &p, affine);
		gnome_print_lineto (frgba->ctx, p.x, p.y);
		bbox.x0 = MIN (bbox.x0, p.x);
		bbox.y0 = MIN (bbox.y0, p.y);
		bbox.x1 = MAX (bbox.x1, p.x);
		bbox.y1 = MAX (bbox.y1, p.y);
		p.x = 1.0;
		p.y = 1.0;
		art_affine_point (&p, &p, affine);
		gnome_print_lineto (frgba->ctx, p.x, p.y);
		bbox.x0 = MIN (bbox.x0, p.x);
		bbox.y0 = MIN (bbox.y0, p.y);
		bbox.x1 = MAX (bbox.x1, p.x);
		bbox.y1 = MAX (bbox.y1, p.y);
		p.x = 0.0;
		p.y = 1.0;
		art_affine_point (&p, &p, affine);
		gnome_print_lineto (frgba->ctx, p.x, p.y);
		bbox.x0 = MIN (bbox.x0, p.x);
		bbox.y0 = MIN (bbox.y0, p.y);
		bbox.x1 = MAX (bbox.x1, p.x);
		bbox.y1 = MAX (bbox.y1, p.y);

		gnome_print_closepath (frgba->ctx);
		gnome_print_clip (frgba->ctx);
		gpf_render_buf (frgba, &bbox);
		gnome_print_grestore (frgba->ctx);
	}

	return GNOME_PRINT_OK;
}

static gint
gpf_glyphlist (GnomePrintContext * pc, const gdouble *affine, GnomeGlyphList *gl)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_glyphlist_transform (frgba->meta, affine, gl);
	return gnome_print_glyphlist_transform (frgba->ctx, affine, gl);


}

static gint
gpf_close (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_context_close (frgba->meta);
	return gnome_print_context_close (frgba->ctx);
}


/**
 * gnome_print_frgba_new:
 * @context: 
 * 
 * Creates a new FRGBA wrapper context around
 * an existing one (usable mostly for PostScript)
 * 
 * Return Value: the new context, NULL on error
 **/
GnomePrintContext *
gnome_print_frgba_new (GnomePrintContext * context)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (context), NULL);

	frgba = g_object_new (GNOME_TYPE_PRINT_FRGBA, NULL);

	frgba->meta = (GnomePrintContext *) gnome_print_meta_new ();

	frgba->ctx = context;
	g_object_ref (G_OBJECT (context));

	return GNOME_PRINT_CONTEXT (frgba);
}

static ArtDRect *
gpf_bpath_bbox (const ArtBpath * bpath, ArtDRect * box)
{
	ArtVpath * vpath;

	vpath = art_bez_path_to_vec (bpath, 0.25);
	art_vpath_bbox_drect (vpath, box);
	art_free (vpath);

	return box;
}

static void
gpf_render_buf (GnomePrintFRGBA * frgba, ArtDRect * box)
{
	GnomePrintContext * gpr;
	guchar * pixels;
	gdouble page2buf[6], a[6];
	gdouble width, height;
	gint w, h;

#ifdef VERBOSE
	g_print ("box %g %g %g %g\n", box->x0, box->y0, box->x1, box->y1);
#endif

	width = ceil ((box->x1 - box->x0) * GP_RENDER_DPI / 72.0);
	height = ceil ((box->y1 - box->y0) * GP_RENDER_DPI / 72.0);
	w = (gint) width;
	h = (gint) height;

	if (width <= 0)
		return;
	if (height <= 0)
		return;

	pixels = g_new (guchar, w * h * 3);

	/* fixme: should be paper color */
	memset (pixels, 0xff, w * h * 3);
	art_affine_translate (page2buf, -box->x0, -box->y1);
	art_affine_scale (a, width / (box->x1 - box->x0), -height / (box->y1 - box->y0));
	art_affine_multiply (page2buf, page2buf, a);

	gpr = gnome_print_rbuf_new (pixels,
				    w,
				    h,
				    w * 3,
				    page2buf,
				    FALSE);

	gnome_print_meta_render_data (gpr,
				      gnome_print_meta_get_buffer (GNOME_PRINT_META (frgba->meta)),
				      gnome_print_meta_get_length (GNOME_PRINT_META (frgba->meta)));

	g_object_unref (G_OBJECT (gpr));

	gnome_print_gsave (frgba->ctx);
	gnome_print_translate (frgba->ctx, box->x0, box->y0);
	gnome_print_scale (frgba->ctx, (box->x1 - box->x0), (box->y1 - box->y0));

	gnome_print_rgbimage (frgba->ctx, pixels, w, h, w * 3);

	gnome_print_grestore (frgba->ctx);

	g_free (pixels);
}



