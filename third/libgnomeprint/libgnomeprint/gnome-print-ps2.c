/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-ps2.c: A Postscript driver for GnomePrint
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
 *    Lauris Kaplinski <lauris@helixcode.com>
 *
 *  Copyright 2000-2003 Ximian, Inc.
 *
 *  References:
 *    [1] Postscript Language Reference, 3rd Edition. Adobe. [http://partners.adobe.com/asn/developer/pdfs/tn/PLRM.pdf]
 *    [2] Document Structuring Conventions, Adobe. [http://partners.adobe.com/asn/developer/pdfs/tn/5001.DSC_Spec.pdf]
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>

#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_misc.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gp-gc-private.h>
#include <libgnomeprint/gnome-print-transport.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/gnome-pgl-private.h>
#include <libgnomeprint/gnome-print-ps2.h>

#define EOL "\n"

typedef struct _GnomePrintPs2Font GnomePrintPs2Font;
typedef struct _GnomePrintPs2Page GnomePrintPs2Page;

struct _GnomePrintPs2Class {
	GnomePrintContextClass parent_class;
};

struct _GnomePrintPs2 {
	GnomePrintContext pc;

	/* Bounding box */
	ArtDRect bbox;

	/* lists */
	GnomePrintPs2Font *fonts;
	GnomePrintPs2Page *pages;

	/* State */
	GnomePrintPs2Font *selected_font;
	gdouble r, g, b;
	gint private_color_flag;
	gint gsave_level;
	
	/* Buffer */
	FILE *buf;
	gchar *bufname;
};

struct _GnomePrintPs2Font {
	GnomePrintPs2Font *next;
	GnomeFontFace *face;
	GnomeFontPsObject *pso;
	gdouble currentsize;
};

struct _GnomePrintPs2Page {
	GnomePrintPs2Page *next;
	gchar *name;
	gint number;
	gboolean shown;
	GSList *usedfonts;
};

static void gnome_print_ps2_class_init (GnomePrintPs2Class *klass);
static void gnome_print_ps2_init (GnomePrintPs2 *ps2);
static void gnome_print_ps2_finalize (GObject *object);

static gint gnome_print_ps2_construct (GnomePrintContext *ctx);
static gint gnome_print_ps2_gsave (GnomePrintContext *pc);
static gint gnome_print_ps2_grestore (GnomePrintContext *pc);
static gint gnome_print_ps2_fill (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule);
static gint gnome_print_ps2_clip (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule);
static gint gnome_print_ps2_stroke (GnomePrintContext *pc, const ArtBpath *bpath);
static gint gnome_print_ps2_image (GnomePrintContext *pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch);
static gint gnome_print_ps2_glyphlist (GnomePrintContext *pc, const gdouble *affine, GnomeGlyphList *gl);
static gint gnome_print_ps2_beginpage (GnomePrintContext *pc, const guchar *name);
static gint gnome_print_ps2_showpage (GnomePrintContext *pc);
static gint gnome_print_ps2_close (GnomePrintContext *pc);

static gint gnome_print_ps2_set_color (GnomePrintPs2 *ps2);
static gint gnome_print_ps2_set_line (GnomePrintPs2 *ps2);
static gint gnome_print_ps2_set_dash (GnomePrintPs2 *ps2);

static gint gnome_print_ps2_set_color_real (GnomePrintPs2 *ps2, gdouble r, gdouble g, gdouble b);
static gint gnome_print_ps2_set_font_real (GnomePrintPs2 *ps2, const GnomeFont *font);

static gint gnome_print_ps2_print_bpath (GnomePrintPs2 *ps2, const ArtBpath *bpath);

static gchar *gnome_print_ps2_get_date (void);
static gint   gnome_print_ps2_fprintf (GnomePrintPs2 *ps2, const char *format, ...);

static GnomePrintContextClass *parent_class;

GType
gnome_print_ps2_get_type (void)
{
	static GType ps2_type = 0;
	if (!ps2_type) {
		static const GTypeInfo ps2_info = {
			sizeof (GnomePrintPs2Class),
			NULL, NULL,
			(GClassInitFunc) gnome_print_ps2_class_init,
			NULL, NULL,
			sizeof (GnomePrintPs2),
			0,
			(GInstanceInitFunc) gnome_print_ps2_init
		};
		ps2_type = g_type_register_static (GNOME_TYPE_PRINT_CONTEXT, "GnomePrintPs2", &ps2_info, 0);
	}
	return ps2_type;
}

static void
gnome_print_ps2_class_init (GnomePrintPs2Class *klass)
{
	GnomePrintContextClass *pc_class;
	GObjectClass *object_class;

	object_class = (GObjectClass *) klass;
	pc_class = (GnomePrintContextClass *)klass;

	parent_class = g_type_class_peek_parent (klass);
	
	object_class->finalize = gnome_print_ps2_finalize;

	pc_class->construct = gnome_print_ps2_construct;
	pc_class->beginpage = gnome_print_ps2_beginpage;
	pc_class->showpage = gnome_print_ps2_showpage;
	pc_class->gsave = gnome_print_ps2_gsave;
	pc_class->grestore = gnome_print_ps2_grestore;
	pc_class->clip = gnome_print_ps2_clip;
	pc_class->fill = gnome_print_ps2_fill;
	pc_class->stroke = gnome_print_ps2_stroke;
	pc_class->image = gnome_print_ps2_image;
	pc_class->glyphlist = gnome_print_ps2_glyphlist;
	pc_class->close = gnome_print_ps2_close;
}

static void
gnome_print_ps2_init (GnomePrintPs2 *ps2)
{
	ps2->gsave_level = 0;
	ps2->fonts = NULL;
	ps2->selected_font = NULL;
	ps2->private_color_flag = GP_GC_FLAG_UNSET;
	ps2->pages = NULL;
	ps2->buf = NULL;
}

static void
gnome_print_ps2_finalize (GObject *object)
{
	GnomePrintPs2 *ps2;

	ps2 = GNOME_PRINT_PS2 (object);

	if (ps2->buf) {
		g_warning ("file %s: line %d: Destroying PS2 context with open buffer", __FILE__, __LINE__);
		if (fclose (ps2->buf)) {
			g_warning ("Error closing buffer");
		}
		ps2->buf = NULL;
		if (unlink (ps2->bufname)) {
			g_warning ("Error unlinking buffer");
		}
		g_free (ps2->bufname);
		ps2->bufname = NULL;
	}

	while (ps2->pages) {
		GnomePrintPs2Page *p;
		p = ps2->pages;
		if (!p->shown)
			g_warning ("Page %d %s was not shown", p->number, p->name);
		if (p->name)
			g_free (p->name);
		while (p->usedfonts) {
			p->usedfonts = g_slist_remove (p->usedfonts, p->usedfonts->data);
		}
		ps2->pages = p->next;
		g_free (p);
	}

	while (ps2->fonts) {
		GnomePrintPs2Font *f;
		f = ps2->fonts;
		if (f->face)
			g_object_unref (G_OBJECT (f->face));
		if (f->pso)
			gnome_font_face_pso_free (f->pso);
		ps2->fonts = f->next;
		g_free (f);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gnome_print_ps2_construct (GnomePrintContext *ctx)
{
	GnomePrintPs2 *ps2;
	gchar *tmp;
	gint ret, fd;

	ps2 = GNOME_PRINT_PS2 (ctx);

	ret = gnome_print_context_create_transport (ctx);
	g_return_val_if_fail (ret >= 0, ret);
	ret = gnome_print_transport_open (ctx->transport);
	g_return_val_if_fail (ret >= 0, ret);

	tmp = g_strdup ("/tmp/gnome-print-XXXXXX");
	fd = mkstemp (tmp);
	if (fd < 0) {
		g_warning ("file %s: line %d: Cannot create temporary file", __FILE__, __LINE__);
		g_free (tmp);
		return GNOME_PRINT_ERROR_UNKNOWN;
	}
	ps2->buf = fdopen (fd, "r+");
	ps2->bufname = tmp;

	/* Set bbox to emty practical infinity */
	ps2->bbox.x0 = 0.0;
	ps2->bbox.y0 = 0.0;
	ps2->bbox.x1 = 210 * 72.0 / 25.4;
	ps2->bbox.y1 = 297 * 72.0 / 25.4;
	
	gnome_print_config_get_length (ctx->config, GNOME_PRINT_KEY_PAPER_WIDTH,  &ps2->bbox.x1, NULL);
	gnome_print_config_get_length (ctx->config, GNOME_PRINT_KEY_PAPER_HEIGHT, &ps2->bbox.y1, NULL);
	
	if (ctx->config) {
		gdouble pp2pa[6];
		art_affine_identity (pp2pa);
		if (gnome_print_config_get_transform (ctx->config, GNOME_PRINT_KEY_PAPER_ORIENTATION_MATRIX, pp2pa)) {
			art_drect_affine_transform (&ps2->bbox, &ps2->bbox, pp2pa);
			ps2->bbox.x1 -= ps2->bbox.x0;
			ps2->bbox.y1 -= ps2->bbox.y0;
			ps2->bbox.x0 = 0.0;
			ps2->bbox.y0 = 0.0;
		}
	}

	return GNOME_PRINT_OK;
}

static gint
gnome_print_ps2_gsave (GnomePrintContext *ctx)
{
	GnomePrintPs2 *ps2;

	ps2 = GNOME_PRINT_PS2 (ctx);

	ps2->gsave_level += 1;

	return gnome_print_ps2_fprintf (ps2, "q" EOL);
}

static gint
gnome_print_ps2_grestore (GnomePrintContext *ctx)
{
	GnomePrintPs2 *ps2;

	ps2 = GNOME_PRINT_PS2 (ctx);

	ps2->gsave_level -= 1;
	ps2->selected_font = NULL;
	ps2->private_color_flag = GP_GC_FLAG_UNSET;

	return gnome_print_ps2_fprintf (ps2, "Q" EOL);
}

static gint
gnome_print_ps2_clip (GnomePrintContext *ctx, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintPs2 *ps2;
	gint ret;

	ps2 = GNOME_PRINT_PS2 (ctx);

	ret = gnome_print_ps2_print_bpath (ps2, bpath);
	g_return_val_if_fail (ret >= 0, ret);

	if (rule == ART_WIND_RULE_NONZERO) {
		ret = gnome_print_ps2_fprintf (ps2, "W" EOL);
	} else {
		ret = gnome_print_ps2_fprintf (ps2, "W*" EOL);
	}

	return ret;
}

static gint
gnome_print_ps2_fill (GnomePrintContext *ctx, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintPs2 *ps2;
	gint ret = 0;

	ps2 = GNOME_PRINT_PS2 (ctx);

	ret += gnome_print_ps2_set_color (ps2);
	ret += gnome_print_ps2_print_bpath (ps2, bpath);

	if (rule == ART_WIND_RULE_NONZERO) {
		ret += gnome_print_ps2_fprintf (ps2, "f" EOL);
	} else {
		ret += gnome_print_ps2_fprintf (ps2, "f*" EOL);
	}

	g_return_val_if_fail (ret >= 0, ret);
	/* Update bbox */


	return ret;
}

static gint
gnome_print_ps2_stroke (GnomePrintContext *ctx, const ArtBpath *bpath)
{
	GnomePrintPs2 *ps2;
	gint ret = 0;

	ps2 = GNOME_PRINT_PS2 (ctx);

	ret += gnome_print_ps2_set_color (ps2);
	ret += gnome_print_ps2_set_line (ps2);
	ret += gnome_print_ps2_set_dash (ps2);
	ret += gnome_print_ps2_print_bpath (ps2, bpath);
	
	g_return_val_if_fail (ret >= 0, ret);

	ret = gnome_print_ps2_fprintf (ps2, "S" EOL);

	return ret;
}

static int
gnome_print_ps2_image (GnomePrintContext *pc, const gdouble *ctm, const guchar *px, gint w, gint h, gint rowstride, gint ch)
{
	GnomePrintPs2 *ps2;
	gchar *hex;
	gint hex_size;
	gint ret = 0;
	gint r;

	ps2 = GNOME_PRINT_PS2 (pc);

	ret += gnome_print_ps2_fprintf (ps2, "q" EOL);
	ret += gnome_print_ps2_fprintf (ps2, "[%g %g %g %g %g %g]cm" EOL, ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);

	/* Image commands */
	ret += gnome_print_ps2_fprintf (ps2, "/buf %d string def" EOL "%d %d 8" EOL, w * ch, w, h);
	ret += gnome_print_ps2_fprintf (ps2, "[%d 0 0 %d 0 %d]" EOL, w, -h, h);
	ret += gnome_print_ps2_fprintf (ps2, "{ currentfile buf readhexstring pop }\n");

	if (ch == 1) {
		ret += gnome_print_ps2_fprintf (ps2, "image" EOL);
	} else {
		ret += gnome_print_ps2_fprintf (ps2, "false %d colorimage" EOL, ch);
	}
	g_return_val_if_fail (ret >= 0, ret);

	hex = g_new (gchar, gnome_print_encode_hex_wcs (w * ch));

	for (r = 0; r < h; r++) {
		hex_size = gnome_print_encode_hex (px + r * rowstride, hex, w * ch);
		ret += fwrite (hex, sizeof (gchar), hex_size, ps2->buf);
		ret += gnome_print_ps2_fprintf (ps2, EOL);
	}

	g_free (hex);
	
	ret = gnome_print_ps2_fprintf (ps2, "Q" EOL);
	
	g_return_val_if_fail (ret >= 0, ret);

	return GNOME_PRINT_OK;
}

#define EPSILON 1e-9

static gint
gnome_print_ps2_glyphlist (GnomePrintContext *pc, const gdouble *a, GnomeGlyphList *gl)
{
	static gdouble id[] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
	GnomePrintPs2 *ps2;
	GnomePosGlyphList *pgl;
	gboolean identity;
	gdouble dx, dy;
	gint ret, s;

	ps2 = (GnomePrintPs2 *) pc;

	identity = ((fabs (a[0] - 1.0) < EPSILON) && (fabs (a[1]) < EPSILON) && (fabs (a[2]) < EPSILON) && (fabs (a[3] - 1.0) < EPSILON));

	if (!identity) {
		ret = gnome_print_ps2_fprintf (ps2, "q" EOL);
		g_return_val_if_fail (ret >= 0, ret);
		ret = gnome_print_ps2_fprintf (ps2, "[%g %g %g %g %g %g]cm" EOL, a[0], a[1], a[2], a[3], a[4], a[5]);
		g_return_val_if_fail (ret >= 0, ret);
		dx = dy = 0.0;
	} else {
		dx = a[4];
		dy = a[5];
	}

	pgl = gnome_pgl_from_gl (gl, id, GNOME_PGL_RENDER_DEFAULT);

	for (s = 0; s < pgl->num_strings; s++) {
		GnomePosString * ps;
		gint i;

		ps = pgl->strings + s;

		ret = gnome_print_ps2_set_font_real (ps2, gnome_rfont_get_font (ps->rfont));
		g_return_val_if_fail (ret >= 0, ret);
		ret = gnome_print_ps2_set_color_real (ps2,
						((ps->color >> 24) & 0xff) / 255.0,
						((ps->color >> 16) & 0xff) / 255.0,
						((ps->color >>  8) & 0xff) / 255.0);
		g_return_val_if_fail (ret >= 0, ret);
		ret = gnome_print_ps2_fprintf (ps2, "%g %g m" EOL, pgl->glyphs[ps->start].x + dx, pgl->glyphs[ps->start].y + dy);
		g_return_val_if_fail (ret >= 0, ret);
		/* Build string */



		ret = gnome_print_ps2_fprintf (ps2, "(");

		if (ps2->selected_font->pso->encodedbytes == 1) {
			/* 8-bit encoding */
			for (i = ps->start; i < ps->start + ps->length; i++) {
				gint glyph;
				glyph = pgl->glyphs[i].glyph & 0xff;
				gnome_font_face_pso_mark_glyph (ps2->selected_font->pso, glyph);
				ret = gnome_print_ps2_fprintf (ps2, "\\%o", glyph);
				g_return_val_if_fail (ret >= 0, ret);
			}
		} else {
			/* 16-bit encoding */
			for (i = ps->start; i < ps->start + ps->length; i++) {
				gint glyph, page;
				gnome_font_face_pso_mark_glyph (ps2->selected_font->pso, pgl->glyphs[i].glyph);
				glyph = pgl->glyphs[i].glyph & 0xff;
				page = (pgl->glyphs[i].glyph >> 8) & 0xff;
				ret = gnome_print_ps2_fprintf (ps2, "\\%o\\%o", page, glyph);
				g_return_val_if_fail (ret >= 0, ret);
			}
		}
		ret = gnome_print_ps2_fprintf (ps2, ")" EOL);
		g_return_val_if_fail (ret >= 0, ret);
		/* Build array */
		ret = gnome_print_ps2_fprintf (ps2, "[");
		g_return_val_if_fail (ret >= 0, ret);
		for (i = ps->start + 1; i < ps->start + ps->length; i++) {
			ret = gnome_print_ps2_fprintf (ps2, "%g %g ",
					      pgl->glyphs[i].x - pgl->glyphs[i-1].x,
					      pgl->glyphs[i].y - pgl->glyphs[i-1].y);
			g_return_val_if_fail (ret >= 0, ret);
		}
		ret = gnome_print_ps2_fprintf (ps2, "0 0] ");
		g_return_val_if_fail (ret >= 0, ret);
		/* xyshow */
		ret = gnome_print_ps2_fprintf (ps2, "xyshow" EOL);
		g_return_val_if_fail (ret >= 0, ret);
	}

	if (!identity) {
		ret = gnome_print_ps2_fprintf (ps2, "Q" EOL);
		g_return_val_if_fail (ret >= 0, ret);
		ps2->selected_font = NULL;
		ps2->private_color_flag = GP_GC_FLAG_UNSET;
	}

	gnome_pgl_destroy (pgl);

	return GNOME_PRINT_OK;
}

static gint
gnome_print_ps2_beginpage (GnomePrintContext *pc, const guchar *name)
{
	GnomePrintPs2Page *page;
	GnomePrintPs2 *ps2;
	gint number;
	gint ret = 0;

	ps2 = GNOME_PRINT_PS2 (pc);

	number = ps2->pages ? ps2->pages->number : 0;

	page = g_new (GnomePrintPs2Page, 1);
	page->next = ps2->pages;
	page->name = g_strdup (name);
	page->number = number + 1;
	page->shown = FALSE;
	page->usedfonts = NULL;

	ps2->pages = page;

	ps2->selected_font = NULL;
	ps2->private_color_flag = GP_GC_FLAG_UNSET;

	ret += gnome_print_ps2_fprintf (ps2, "%%%%Page: %s %d" EOL, name, page->number);
	ret += gnome_print_ps2_fprintf (ps2, "%%%%PageResources: (atend)" EOL);

	ret += gnome_print_newpath (pc);
	ret += gnome_print_moveto (pc, 0.0, 0.0);
	ret += gnome_print_lineto (pc, ps2->bbox.x1, 0.0);
	ret += gnome_print_lineto (pc, ps2->bbox.x1, ps2->bbox.y1);
	ret += gnome_print_lineto (pc, 0.0, ps2->bbox.y1);
	ret += gnome_print_lineto (pc, 0.0, 0.0);
	ret += gnome_print_closepath (pc);
	ret += gnome_print_clip (pc);
	ret += gnome_print_newpath (pc);
	
	g_return_val_if_fail (ret >= 0, ret);

	return GNOME_PRINT_OK;
}


static gint
gnome_print_ps2_showpage (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	gint ret = 0;

	ps2 = GNOME_PRINT_PS2 (pc);

	if (ps2->pages)
		ps2->pages->shown = TRUE;
	
	ps2->selected_font = NULL;
	ps2->private_color_flag = GP_GC_FLAG_UNSET;

	ret += gnome_print_ps2_fprintf (ps2, "SP" EOL);
	ret += gnome_print_ps2_fprintf (ps2, "%%%%PageTrailer" EOL);
	ret += gnome_print_ps2_fprintf (ps2, "%%%%PageResources: procset gnome-print-procs-%s" EOL, VERSION);

	while (ps2->pages->usedfonts) {
		GnomePrintPs2Font *font;
		font = ps2->pages->usedfonts->data;
		ret += gnome_print_ps2_fprintf (ps2, "%%%%+ font %s" EOL, font->pso->encodedname);
		ps2->pages->usedfonts = g_slist_remove (ps2->pages->usedfonts, ps2->pages->usedfonts->data);
	}

	g_return_val_if_fail (ret >= 0, ret);
	
	return GNOME_PRINT_OK;
}

static gint
gnome_print_ps2_close (GnomePrintContext *pc)
{
	GnomePrintPs2Font *font;
	GnomePrintPs2 *ps2;
	gchar *date;
	gchar *orientation;
	gchar *name;
	gint len, ret;

	ps2 = GNOME_PRINT_PS2 (pc);

	g_return_val_if_fail (pc->transport != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	if (!ps2->pages || !ps2->pages->shown) {
		g_warning ("file %s: line %d: Closing PS2 context without final showpage", __FILE__, __LINE__);
		ret = gnome_print_showpage (pc);
		g_return_val_if_fail (ret >= 0, ret);
	}

	/* Do header */
	date = gnome_print_ps2_get_date ();
	gnome_print_transport_printf (pc->transport, "%%!PS-Adobe-3.0" EOL);	
	/* DSC Comments */
	gnome_print_transport_printf (pc->transport, "%%%%Creator: Gnome Print Version %s" EOL, VERSION);
	gnome_print_transport_printf (pc->transport, "%%%%CreationDate: %s" EOL, date);
	gnome_print_transport_printf (pc->transport, "%%%%LanguageLevel: 2" EOL);
	gnome_print_transport_printf (pc->transport, "%%%%DocumentMedia: Regular %d %d 0 () ()" EOL,
				      (gint) (ps2->bbox.x1 - ps2->bbox.x0), (gint) (ps2->bbox.y1 - ps2->bbox.y0));

	/* Orientation ([2] Page 43) */
	gnome_print_transport_printf (pc->transport, "%%%%Orientation: ");
	orientation = gnome_print_config_get (pc->config, GNOME_PRINT_KEY_ORIENTATION);
	if (!orientation || strcmp ("R0", orientation) == 0) {
		gnome_print_transport_printf (pc->transport, "Portrait");
	} else if (strcmp ("R90", orientation) == 0) {
		gnome_print_transport_printf (pc->transport, "Landscape");
	} else if (strcmp ("R180", orientation) == 0) {
		gnome_print_transport_printf (pc->transport, "Portrait");
	} else if (strcmp ("R270", orientation) == 0) {
		gnome_print_transport_printf (pc->transport, "Landscape");
	} else {
		g_warning ("Could not interpret Orientation from GnomePrintConfig [%s]\n", orientation);
	}
	g_free (orientation);
	gnome_print_transport_printf (pc->transport, EOL);
	/* End: Orientation */

	gnome_print_transport_printf (pc->transport, "%%%%BoundingBox: %d %d %d %d" EOL,
				      (gint) floor (ps2->bbox.x0),
				      (gint) floor (ps2->bbox.y0),
				      (gint) ceil (ps2->bbox.x1),
				      (gint) ceil (ps2->bbox.y1));
	gnome_print_transport_printf (pc->transport, "%%%%Pages: %d" EOL, ps2->pages->number);
	gnome_print_transport_printf (pc->transport, "%%%%PageOrder: Ascend" EOL);

	name = gnome_print_config_get (pc->config,  GNOME_PRINT_KEY_DOCUMENT_NAME);
	if (name) {
		gnome_print_transport_printf (pc->transport, "%%%%Title: %s" EOL, name);
		g_free (name);
	}
	
	gnome_print_transport_printf (pc->transport, "%%%%DocumentSuppliedResources: procset pnome-print-procs-%s" EOL, VERSION);
	/* %%DocumentSuppliedResources: */
	for (font = ps2->fonts; font != NULL; font = font->next) {
		gnome_print_transport_printf (pc->transport, "%%%%+ font %s" EOL, font->pso->encodedname);
	}
	gnome_print_transport_printf (pc->transport, "%%%%EndComments" EOL);
	g_free (date);
	/* Defaults */
	gnome_print_transport_printf (pc->transport, "%%%%BeginDefaults" EOL);
	gnome_print_transport_printf (pc->transport, "%%%%PageMedia: Regular" EOL);
	gnome_print_transport_printf (pc->transport, "%%%%EndDefaults" EOL);
	/* Prolog */
	gnome_print_transport_printf (pc->transport, "%%%%BeginProlog" EOL);
	/* Abbreviations */
	gnome_print_transport_printf (pc->transport, "%%%%BeginResource: procset gnome-print-procs-%s" EOL, VERSION);
	gnome_print_transport_printf (pc->transport, "/B {load def} bind def" EOL);
	gnome_print_transport_printf (pc->transport, "/n /newpath B /m /moveto B /l /lineto B /c /curveto B /h /closepath B" EOL);
	gnome_print_transport_printf (pc->transport, "/q /gsave B /Q /grestore B" EOL);
	gnome_print_transport_printf (pc->transport, "/J /setlinecap B /j /setlinejoin B /w /setlinewidth B /M /setmiterlimit B" EOL);
	gnome_print_transport_printf (pc->transport, "/d /setdash B" EOL);
	gnome_print_transport_printf (pc->transport, "/rg /setrgbcolor B" EOL);
	gnome_print_transport_printf (pc->transport, "/W /clip B /W* /eoclip B" EOL);
	gnome_print_transport_printf (pc->transport, "/f /fill B /f* /eofill B" EOL);
	gnome_print_transport_printf (pc->transport, "/S /stroke B" EOL);
	gnome_print_transport_printf (pc->transport, "/cm /concat B" EOL);
	gnome_print_transport_printf (pc->transport, "/SP /showpage B" EOL);
	gnome_print_transport_printf (pc->transport, "/FF /findfont B /F {scalefont setfont} bind def" EOL);
	gnome_print_transport_printf (pc->transport, "%%%%EndResource" EOL);
	gnome_print_transport_printf (pc->transport, "%%%%EndProlog" EOL);
	/* Setup */
	gnome_print_transport_printf (pc->transport, "%%%%BeginSetup" EOL);
	/* setpagedevice*/
	gnome_print_transport_printf (pc->transport, "<<" EOL);
	gnome_print_transport_printf (pc->transport, "/PageSize [%d %d]" EOL, (gint) (ps2->bbox.x1 - ps2->bbox.x0), (gint) (ps2->bbox.y1 - ps2->bbox.y0));
	gnome_print_transport_printf (pc->transport, "/ImagingBBox null" EOL);
	gnome_print_transport_printf (pc->transport, ">> setpagedevice" EOL);
	/* Download fonts */
	for (font = ps2->fonts; font != NULL; font = font->next) {
		gnome_font_face_ps_embed (font->pso);
		gnome_print_transport_printf (pc->transport, "%%%%BeginResource: font %s" EOL, font->pso->encodedname);
		gnome_print_transport_write (pc->transport, font->pso->buf, font->pso->length);
		gnome_print_transport_printf (pc->transport, "%%%%EndResource" EOL);
	}
	gnome_print_transport_printf (pc->transport, "%%%%EndSetup" EOL);
	/* Write buffer */
	rewind (ps2->buf);
	len = 256;
	while (len > 0) {
		guchar b[256];
		len = fread (b, sizeof (guchar), 256, ps2->buf);
		if (len > 0) {
			gnome_print_transport_write (pc->transport, b, len);
		}
	}
	fclose (ps2->buf);
	ps2->buf = NULL;
	if (unlink (ps2->bufname)) {
		g_warning ("Error unlinking buffer");
	}
	g_free (ps2->bufname);
	ps2->bufname = NULL;
	/* END: Write buffer */

	gnome_print_transport_printf (pc->transport, "%%%%Trailer" EOL);
	gnome_print_transport_printf (pc->transport, "%%%%EOF" EOL);
 
 	gnome_print_transport_close (pc->transport);
	g_object_unref (G_OBJECT (pc->transport));
	pc->transport = NULL;
 	
	return GNOME_PRINT_OK;
}

static gint
gnome_print_ps2_set_color (GnomePrintPs2 *ps2)
{
	GnomePrintContext *ctx;
	gint ret;

	ctx = GNOME_PRINT_CONTEXT (ps2);

	ret = gnome_print_ps2_set_color_real (ps2,
					      gp_gc_get_red   (ctx->gc),
					      gp_gc_get_green (ctx->gc),
					      gp_gc_get_blue  (ctx->gc));

	g_return_val_if_fail (ret == 0, ret);

	return ret;
}

static gint
gnome_print_ps2_set_line (GnomePrintPs2 *ps2)
{
	GnomePrintContext *ctx;
	gint ret;

	ctx = GNOME_PRINT_CONTEXT (ps2);

	if (gp_gc_get_line_flag (ctx->gc) == GP_GC_FLAG_CLEAR)
		return 0;

	ret = gnome_print_ps2_fprintf (ps2, "%g w %i J %i j %g M" EOL,
				       gp_gc_get_linewidth (ctx->gc),
				       gp_gc_get_linecap (ctx->gc),
				       gp_gc_get_linejoin (ctx->gc),
				       gp_gc_get_miterlimit (ctx->gc));
	gp_gc_set_line_flag (ctx->gc, GP_GC_FLAG_CLEAR);

	return ret;
}

static gint
gnome_print_ps2_set_dash (GnomePrintPs2 *ps2)
{
	GnomePrintContext *ctx;
	const ArtVpathDash *dash;
	gint ret = 0, i;

	ctx = GNOME_PRINT_CONTEXT (ps2);

	if (gp_gc_get_dash_flag (ctx->gc) == GP_GC_FLAG_CLEAR)
		return 0;

	dash = gp_gc_get_dash (ctx->gc);
	ret += gnome_print_ps2_fprintf (ps2, "[");
	for (i = 0; i < dash->n_dash; i++)
		ret += gnome_print_ps2_fprintf (ps2, " %g", dash->dash[i]);
	ret += gnome_print_ps2_fprintf (ps2, "]%g d" EOL, dash->n_dash > 0 ? dash->offset : 0.0);
	gp_gc_set_dash_flag (ctx->gc, GP_GC_FLAG_CLEAR);

	g_return_val_if_fail (ret >= 0, ret);
		
	return ret;
}

static gint
gnome_print_ps2_set_color_real (GnomePrintPs2 *ps2, gdouble r, gdouble g, gdouble b)
{
	GnomePrintContext *ctx;
	gint ret;

	ctx = GNOME_PRINT_CONTEXT (ps2);

	if ((ps2->private_color_flag == GP_GC_FLAG_CLEAR)
	    && (r == ps2->r) && (g == ps2->g) && (b == ps2->b))
		return GNOME_PRINT_OK;
	
	ret = gnome_print_ps2_fprintf (ps2, "%.3g %.3g %.3g rg" EOL, r, g, b);

	ps2->r = r;
	ps2->g = g;
	ps2->b = b;
	ps2->private_color_flag = GP_GC_FLAG_CLEAR;

	g_return_val_if_fail (ret >= 0, ret);
	
	return ret;
}

static gint
gnome_print_ps2_set_font_real (GnomePrintPs2 *ps2, const GnomeFont *gnome_font)
{
	const GnomeFontFace *face;
	GnomePrintPs2Font *font;
	gint ret = 0;
	GSList *l;

	if ((ps2->selected_font) &&
	    (ps2->selected_font->face == gnome_font->face) &&
	    (ps2->selected_font->currentsize == gnome_font->size)) {
		return 0;
	}

	face = gnome_font_get_face (gnome_font);

	for (font = ps2->fonts; font != NULL; font = font->next) {
		if (font->face == face)
			break;
	}
	if (!font) {
		/* No entry, so create one */
		font = g_new (GnomePrintPs2Font, 1);
		font->next = ps2->fonts;
		ps2->fonts = font;
		font->face = (GnomeFontFace *) face;
		gnome_font_face_ref (face);
		font->pso = gnome_font_face_pso_new ((GnomeFontFace *) face, NULL);
		g_return_val_if_fail (font->pso != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	}

	for (l = ps2->pages->usedfonts; l != NULL; l = l->next) {
		if ((GnomePrintPs2Font *) l->data == font)
			break;
	}
	if (!l) {
		ps2->pages->usedfonts = g_slist_prepend (ps2->pages->usedfonts, font);
	}

	ret += gnome_print_ps2_fprintf (ps2, "/%s FF %g F" EOL, font->pso->encodedname, gnome_font_get_size (gnome_font));

	font->currentsize = gnome_font->size;
	ps2->selected_font = font;

	g_return_val_if_fail (ret >= 0, ret);
	
	return ret;
}

GnomePrintContext *
gnome_print_ps2_new (GnomePrintConfig *config)
{
	GnomePrintContext *ctx;
	gint ret;

	g_return_val_if_fail (config != NULL, NULL);

	ctx = g_object_new (GNOME_TYPE_PRINT_PS2, NULL);

	ret = gnome_print_context_construct (ctx, config);

	if (ret != GNOME_PRINT_OK) {
		g_object_unref (G_OBJECT (ctx));
		ctx = NULL;
	}

	return ctx;
}

static gint
gnome_print_ps2_print_bpath (GnomePrintPs2 *ps2, const ArtBpath *bpath)
{
	gboolean started, closed;
	
	gnome_print_ps2_fprintf (ps2, "n" EOL);

	started = FALSE;
	closed = FALSE;
	while (bpath->code != ART_END) {
		switch (bpath->code) {
		case ART_MOVETO_OPEN:
			if (started && closed)
				gnome_print_ps2_fprintf (ps2, "h" EOL);
			closed = FALSE;
			started = FALSE;
			gnome_print_ps2_fprintf (ps2, "%g %g m" EOL, bpath->x3, bpath->y3);
			break;
		case ART_MOVETO:
			if (started && closed)
				gnome_print_ps2_fprintf (ps2, "h" EOL);
			closed = TRUE;
			started = TRUE;
			gnome_print_ps2_fprintf (ps2, "%g %g m" EOL, bpath->x3, bpath->y3);
			break;
		case ART_LINETO:
			gnome_print_ps2_fprintf (ps2, "%g %g l" EOL, bpath->x3, bpath->y3);
			break;
		case ART_CURVETO:
			gnome_print_ps2_fprintf (ps2, "%g %g %g %g %g %g c" EOL,
					bpath->x1, bpath->y1,
					bpath->x2, bpath->y2,
					bpath->x3, bpath->y3);
			break;
		default:
			g_warning ("Path structure is corrupted");
			return -1;
		}
		bpath += 1;
	}

	if (started && closed)
		gnome_print_ps2_fprintf (ps2, "h" EOL);
	
	return 0;
}

/* Other stuff */

static gchar*
gnome_print_ps2_get_date (void)
{
	time_t clock;
	struct tm *now;
	gchar *date;

#ifdef ADD_TIMEZONE_STAMP
  extern char * tzname[];
	/* TODO : Add :
		 "[+-]"
		 "HH'" Offset from gmt in hours
		 "OO'" Offset from gmt in minutes
	   we need to use tz_time. but I don't
	   know how protable this is. Chema */
	gprint ("Timezone %s\n", tzname[0]);
	gprint ("Timezone *%s*%s*%li*\n", tzname[1], timezone);
#endif	

	clock = time (NULL);
	now = localtime (&clock);

	date = g_strdup_printf ("D:%04d%02d%02d%02d%02d%02d",
				now->tm_year + 1900,
				now->tm_mon + 1,
				now->tm_mday,
				now->tm_hour,
				now->tm_min,
				now->tm_sec);

	return date;
}

static gint
gnome_print_ps2_fprintf (GnomePrintPs2 *ps2, const char *format, ...)
{
 	va_list arguments;
 	gchar *text;
	gchar *oldlocale;
	gint len;

	oldlocale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
		
 	va_start (arguments, format);
 	text = g_strdup_vprintf (format, arguments);
 	va_end (arguments);

	len = fwrite (text, sizeof (gchar), strlen (text), ps2->buf);
	g_free (text);

	setlocale (LC_NUMERIC, oldlocale);
	g_free (oldlocale);

	return GNOME_PRINT_OK;
}

