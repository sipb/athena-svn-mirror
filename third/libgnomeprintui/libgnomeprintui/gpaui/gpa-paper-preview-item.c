/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-paper-preview-item.c: A paper preview canvas item
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
 *    James Henstridge <james@daa.com.au>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 1998 James Henstridge <james@daa.com.au>, 2001-2002 Ximian Inc.
 *
 */

#include <config.h>

#include <math.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <libgnomeprint/private/gpa-node.h>
#include <libgnomeprint/private/gnome-print-private.h>
#include <libgnomeprint/private/gnome-print-config-private.h>

#include "gpa-paper-preview-item.h"

typedef struct _GpaPaperPreviewItemClass GpaPaperPreviewItemClass;

GnomeCanvasItem * gpa_paper_preview_item_new (GnomePrintConfig *config, GtkWidget *canvas);

void gpa_paper_preview_item_set_physical_size        (GpaPaperPreviewItem *item, gdouble width, gdouble height);
void gpa_paper_preview_item_set_physical_orientation (GpaPaperPreviewItem *item, const gdouble *orientation);
void gpa_paper_preview_item_set_logical_orientation  (GpaPaperPreviewItem *item, const gdouble *orientation);
void gpa_paper_preview_item_set_physical_margins     (GpaPaperPreviewItem *item, gdouble l, gdouble r, gdouble t, gdouble b);
void gpa_paper_preview_item_set_logical_margins      (GpaPaperPreviewItem *item, gdouble l, gdouble r, gdouble t, gdouble b);
void gpa_paper_preview_item_set_layout               (GpaPaperPreviewItem *item, const GSList *affines, gdouble width, gdouble height);

#define MM(v) ((v) * 72.0 / 25.4)
#define CM(v) ((v) * 72.0 / 2.54)
#define M(v)  ((v) * 72.0 / 0.0254)

#define PAD 4
#define EPSILON 1e-9
#define SHADOW_SIZE 5

struct _GpaPaperPreviewItem {
	GnomeCanvasItem canvasitem;

	/* Width and height of physical page */
	gdouble pw, ph;
	/* Transform from 1x1 abstract physical page to 1x1 abstract printing area */
	gdouble porient[6];

	gdouble lorient[6];
	gdouble pml, pmr, pmt, pmb;
	gdouble lml, lmr, lmt, lmb;
	gboolean lml_hl, lmr_hl, lmt_hl, lmb_hl;
	gdouble lyw, lyh;
	gint num_affines;
	gdouble *affines;

	/* FIXME: remove this (Lauris) */
	/* Physical page -> Printed area */
	gdouble PP2PA[6];
	/* Printed area width and height */
	gdouble PAW, PAH;
	/* Layout page width and height */
	gdouble LYW, LYH;
	/* Logical page -> Layout page */
	gdouble LP2LY[6];
	/* Logical width and height */
	gdouble LW, LH;
	/* State data */
	gdouble PPDP2C[6], PP2C[6];
	ArtIRect area;

	/* Render data */
	ArtSVP *up, *right;

	GnomePrintConfig *config;

	/* Colors */
	guint32 color_page;
	guint32 color_border;
	guint32 color_shadow;
	guint32 color_arrow;
	guint32 color_pmargin;
	guint32 color_lmargin;
	guint32 color_lmargin_hl;
	guint32 color_stripe;

	/* Nodes and handlers we are listening to */
	GPANode *gpa_config;
	GPANode *node[4];
	gulong handler[4];
	gulong handler_config;
};

struct _GpaPaperPreviewItemClass {
	GnomeCanvasItemClass parent_class;
};

static void gpa_paper_preview_item_class_init (GpaPaperPreviewItemClass *klass);
static void gpa_paper_preview_item_init (GpaPaperPreviewItem *item);
static void gpa_paper_preview_item_finalize (GObject *object);

static void gpa_paper_preview_item_update (GnomeCanvasItem *item, gdouble *affine, ArtSVP *clip, gint flags);
static void gpa_paper_preview_item_render (GnomeCanvasItem *item, GnomeCanvasBuf *buf);

static void gpa_paper_preview_item_disconnect (GpaPaperPreviewItem *pp);

static void gppi_hline (GnomeCanvasBuf *buf, gint y, gint xs, gint xe, guint32 rgba);
static void gppi_vline (GnomeCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba);
static void gppi_rect (GnomeCanvasBuf *buf, gint xs, gint ys, gint xe, gint ye, guint32 rgba);
static void gppi_tvline (GnomeCanvasBuf *buf, gdouble x, gdouble sy, gdouble ey, gdouble *affine, guint32 rgba);
static void gppi_thline (GnomeCanvasBuf *buf, gdouble y, gdouble sx, gdouble ex, gdouble *affine, guint32 rgba);

static GnomeCanvasItemClass *item_parent_class;

GType
gpa_paper_preview_item_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GpaPaperPreviewItemClass),
			NULL, NULL,
			(GClassInitFunc) gpa_paper_preview_item_class_init,
			NULL, NULL,
			sizeof (GpaPaperPreviewItem),
			0,
			(GInstanceInitFunc) gpa_paper_preview_item_init
		};
		type = g_type_register_static (GNOME_TYPE_CANVAS_ITEM, "GpaPaperPreviewItem", &info, 0);
	}
	return type;
}

static void
gpa_paper_preview_item_class_init (GpaPaperPreviewItemClass *klass)
{
	GObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	object_class = G_OBJECT_CLASS (klass);
	item_class = GNOME_CANVAS_ITEM_CLASS (klass);

	item_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_paper_preview_item_finalize;

	item_class->update = gpa_paper_preview_item_update;
	item_class->render = gpa_paper_preview_item_render;
}

static void
gpa_paper_preview_item_init (GpaPaperPreviewItem *pp)
{
	pp->pw = MM(210);
	pp->ph = MM(297);
	pp->lyw = 1.0;
	pp->lyh = 1.0;
	art_affine_identity (pp->porient);
	art_affine_identity (pp->lorient);
	pp->pml = pp->pmr = pp->pmt = pp->pmb = MM(5);
	pp->lml = pp->lmr = pp->lmt = pp->lmb = MM(15);
	pp->lml_hl = pp->lmr_hl = pp->lmt_hl = pp->lmb_hl = FALSE;
	pp->num_affines = 1;
	pp->affines = g_new (gdouble, 6);
	art_affine_identity (pp->affines);
}

static void
gpa_paper_preview_item_finalize (GObject *object)
{
	GpaPaperPreviewItem *pp;

	pp = GPA_PAPER_PREVIEW_ITEM (object);

	if (pp->affines) {
		g_free (pp->affines);
		pp->affines = NULL;
		pp->num_affines = 0;
	}

	if (pp->up) {
		art_svp_free (pp->up);
		pp->up = NULL;
	}

	if (pp->right) {
		art_svp_free (pp->right);
		pp->right = NULL;
	}

	gpa_paper_preview_item_disconnect (pp);

	g_signal_handler_disconnect (G_OBJECT (pp->gpa_config), pp->handler_config);
	pp->handler_config = 0;
	pp->gpa_config = NULL;

	pp->config = gnome_print_config_unref (pp->config);

	G_OBJECT_CLASS (item_parent_class)->finalize (object);
}

#define WIDGET_WIDTH(w) (GTK_WIDGET (w)->allocation.width)
#define WIDGET_HEIGHT(w) (GTK_WIDGET (w)->allocation.height)

#ifdef GPP_VERBOSE
#define PRINT_2(s,a,b)    g_print ("%s %g %g\n", (s), (a), (b))
#define PRINT_DRECT(s,a)  g_print ("%s %g %g %g %g\n", (s), (a)->x0, (a)->y0, (a)->x1, (a)->y1)
#define PRINT_AFFINE(s,a) g_print ("%s %g %g %g %g %g %g\n", (s), *(a), *((a) + 1), *((a) + 2), *((a) + 3), *((a) + 4), *((a) + 5))
#else
#define PRINT_2(s,a,b)
#define PRINT_DRECT(s,a)
#define PRINT_AFFINE(s,a)
#endif

#define SQ(v) ((v) * (v))

static void
gpa_paper_preview_item_update (GnomeCanvasItem *item, gdouble *affine, ArtSVP *clip, gint flags)
{
	GpaPaperPreviewItem *pp;
	ArtDRect area, r, PPDC;
	gdouble xscale, yscale, scale, t;
	gdouble PPDP2C[6], a[6], b[6];
	ArtVpath arrow[] = {{ART_MOVETO, -1, 0.001}, {ART_LINETO, 0, 1}, {ART_LINETO, 1, 0}, {ART_LINETO, -1, 0.001}, {ART_END, 0, 0}};
	ArtVpath *vpath;
	ArtSVP *svp;

	pp = GPA_PAPER_PREVIEW_ITEM (item);

	if (((GnomeCanvasItemClass *) item_parent_class)->update)
		((GnomeCanvasItemClass *) item_parent_class)->update (item, affine, clip, flags);

	gnome_canvas_item_reset_bounds (item);

	if (pp->up) {
		art_svp_free (pp->up);
		pp->up = NULL;
	}

	if (pp->right) {
		art_svp_free (pp->right);
		pp->right = NULL;
	}

	/* Req redraw old */
	gnome_canvas_request_redraw (item->canvas,
				     pp->area.x0 - 1, pp->area.y0 - 1,
				     pp->area.x1 + SHADOW_SIZE + 1, pp->area.y1 + SHADOW_SIZE + 1);

	/* Now comes the fun part */

	if (pp->num_affines < 1)
		return;
	if ((fabs (pp->pw) < EPSILON) || (fabs (pp->ph) < EPSILON))
		return;

	/* Initial setup */
	/* Calculate PP2PA */
	/* We allow only rectilinear setups, so we can cheat */
	pp->PP2PA[0] = pp->porient[0];
	pp->PP2PA[1] = pp->porient[1];
	pp->PP2PA[2] = pp->porient[2];
	pp->PP2PA[3] = pp->porient[3];
	t = pp->pw * pp->PP2PA[0] + pp->ph * pp->PP2PA[2];
	pp->PP2PA[4] = (t < 0) ? -t : 0.0;
	t = pp->pw * pp->PP2PA[1] + pp->ph * pp->PP2PA[3];
	pp->PP2PA[5] = (t < 0) ? -t : 0.0;
	PRINT_AFFINE ("PP2PA:", &pp->PP2PA[0]);

	/* PPDP - Physical Page Dimensions in Printer */
	/* A: PhysicalPage X PhysicalOrientation X TRANSLATE -> Physical Page in Printer */
	area.x0 = 0.0;
	area.y0 = 0.0;
	area.x1 = pp->pw;
	area.y1 = pp->ph;
	art_drect_affine_transform (&r, &area, pp->PP2PA);
	pp->PAW = r.x1 - r.x0;
	pp->PAH = r.y1 - r.y0;
	if ((pp->PAW < EPSILON) || (pp->PAH < EPSILON))
		return;

	/* Now we have to find the size of layout page */
	/* Again, knowing that layouts are rectilinear helps us */
	art_affine_invert (a, pp->affines);
	PRINT_AFFINE ("INV LY:", &a[0]);
	pp->LYW = pp->lyw * fabs (pp->pw * a[0] + pp->ph * a[2]);
	pp->LYH = pp->lyh * fabs (pp->pw * a[1] + pp->ph * a[3]);
	PRINT_2 ("LY Dimensions:", pp->LYW, pp->LYH);

	/* Calculate LP2LY */
	/* We allow only rectilinear setups, so we can cheat */
	pp->LP2LY[0] = pp->lorient[0];
	pp->LP2LY[1] = pp->lorient[1];
	pp->LP2LY[2] = pp->lorient[2];
	pp->LP2LY[3] = pp->lorient[3];
	/* Delay */
	pp->LP2LY[4] = 0.0;
	pp->LP2LY[5] = 0.0;
	/* Meanwhile find logical width and height */
	area.x0 = 0.0;
	area.y0 = 0.0;
	area.x1 = pp->LYW;
	area.y1 = pp->LYH;
	art_affine_invert (a, pp->LP2LY);
	art_drect_affine_transform (&r, &area, a);
	pp->LW = r.x1 - r.x0;
	pp->LH = r.y1 - r.y0;
	if ((pp->LW < EPSILON) || (pp->LH < EPSILON))
		return;
	PRINT_2 ("L Dimensions", pp->LW, pp->LH);
	/* Now complete matrix calculation */
	t = pp->LW * pp->LP2LY[0] + pp->LH * pp->LP2LY[2];
	pp->LP2LY[4] = (t < 0) ? -t : 0.0;
	t = pp->LW * pp->LP2LY[1] + pp->LH * pp->LP2LY[3];
	pp->LP2LY[5] = (t < 0) ? -t : 0.0;
	PRINT_AFFINE ("LP2LY:", &pp->LP2LY[0]);

	/* PPDC - Physical Page Dimensions on Canvas */

	/* Now we calculate PPDP -> Canvas transformation */
	/* FIXME: This should be done by group or something? (Lauris)  */

	/* Find view box in canvas coordinates */
	gnome_canvas_window_to_world (item->canvas, 0, 0, &PPDC.x0, &PPDC.y0);
	gnome_canvas_w2c_d (item->canvas, PPDC.x0, PPDC.y0, &PPDC.x0, &PPDC.y0);
	gnome_canvas_window_to_world (item->canvas, WIDGET_WIDTH (item->canvas), WIDGET_HEIGHT (item->canvas), &PPDC.x1, &PPDC.y1);
	gnome_canvas_w2c_d (item->canvas, PPDC.x1, PPDC.y1, &PPDC.x1, &PPDC.y1);
	PRINT_DRECT ("Visible area:", &PPDC);
	/* Clip it by shadow and stuff */
	PPDC.x0 += PAD;
	PPDC.y0 += PAD;
	PPDC.x1 -= (SHADOW_SIZE + PAD);
	PPDC.y1 -= (SHADOW_SIZE + PAD);
	PRINT_DRECT ("Drawable area:", &PPDC);
	/* Check for too small drawing area */
	if ((PPDC.x0 >= PPDC.x1) || (PPDC.y0 >= PPDC.y1))
		return;
	/* Crop to right aspect ratio */
	xscale = (PPDC.x1 - PPDC.x0) / pp->PAW;
	yscale = (PPDC.y1 - PPDC.y0) / pp->PAH;
	scale = MIN (xscale, yscale);
	t = 0.5 * ((PPDC.x1 - PPDC.x0) - scale * pp->PAW);
	PPDC.x0 += t;
	PPDC.x1 -= t;
	t = 0.5 * ((PPDC.y1 - PPDC.y0) - scale * pp->PAH);
	PPDC.y0 += t;
	PPDC.y1 -= t;
	PRINT_DRECT ("Actual page area:", &PPDC);

	/* Find physical page area -> canvas transformation */
	PPDP2C[0] = scale;
	PPDP2C[1] = 0.0;
	PPDP2C[2] = 0.0;
	PPDP2C[3] = -scale;
	PPDP2C[4] = PPDC.x0;
	PPDP2C[5] = PPDC.y0 + (PPDC.y1 - PPDC.y0);
	PRINT_AFFINE ("PPDP -> Canvas:", &PPDP2C[0]);

	/* Find canvas area in integers */
	art_drect_to_irect (&pp->area, &PPDC);

	memcpy (pp->PPDP2C, PPDP2C, 6 * sizeof (gdouble));
	art_affine_multiply (pp->PP2C, pp->PP2PA, pp->PPDP2C);
	PRINT_AFFINE ("Physical Page to Canvas:", &pp->PP2C[0]);

	/* Create SVP-s */
	a[0] = 0.2 * pp->pw;
	a[1] = 0.0;
	a[2] = 0.0;
	a[3] = 0.2 * pp->pw;
	a[4] = 0.5 * pp->pw;
	a[5] = 0.0;
	art_affine_multiply (b, a, pp->PP2C);
	vpath = art_vpath_affine_transform (arrow, b);
	svp = art_svp_from_vpath (vpath);
	art_free (vpath);
	pp->up = art_svp_rewind_uncrossed (svp, ART_WIND_RULE_NONZERO);
	art_svp_free (svp);
	art_drect_svp (&area, pp->up);
	PRINT_DRECT ("Up arrow:", &area);
	a[0] = 0.0;
	a[1] = -0.2 * pp->pw;
	a[2] = -0.2 * pp->pw;
	a[3] = 0.0;
	a[4] = pp->pw;
	a[5] = 0.5 * pp->ph;
	art_affine_multiply (b, a, pp->PP2C);
	vpath = art_vpath_affine_transform (arrow, b);
	svp = art_svp_from_vpath (vpath);
	pp->right = art_svp_rewind_uncrossed (svp, ART_WIND_RULE_NONZERO);
	art_svp_free (svp);
	art_free (vpath);
	art_drect_svp (&area, pp->up);
	PRINT_DRECT ("Up arrow:", &area);

	/* Req redraw new */
	gnome_canvas_request_redraw (item->canvas,
				     pp->area.x0 - 1, pp->area.y0 - 1,
				     pp->area.x1 + SHADOW_SIZE + 1, pp->area.y1 + SHADOW_SIZE + 1);

	item->x1 = pp->area.x0 - 1;
	item->y1 = pp->area.y0 - 1;
	item->x2 = pp->area.x1 + SHADOW_SIZE + 1;
	item->y2 = pp->area.y1 + SHADOW_SIZE + 1;
}

static void
gpa_paper_preview_item_render (GnomeCanvasItem *item, GnomeCanvasBuf *buf)
{
	GpaPaperPreviewItem *pp;

	pp = GPA_PAPER_PREVIEW_ITEM (item);

	if ((pp->area.x0 < buf->rect.x1) &&
	    (pp->area.y0 < buf->rect.y1) &&
	    ((pp->area.x1 + SHADOW_SIZE) >= buf->rect.x0) &&
	    ((pp->area.y1 + SHADOW_SIZE) >= buf->rect.y0)) {
		gint imargin, i;
		/* Initialize buffer, if needed */
		gnome_canvas_buf_ensure_buf (buf);
		buf->is_buf = TRUE;
		buf->is_bg = FALSE;
		/* Top */
		gppi_hline (buf, pp->area.y0, pp->area.x0, pp->area.x1, pp->color_border);
		/* Bottom */
		gppi_hline (buf, pp->area.y1, pp->area.x0, pp->area.x1, pp->color_border);
		/* Left */
		gppi_vline (buf, pp->area.x0, pp->area.y0 + 1, pp->area.y1 - 1, pp->color_border);
		/* Right */
		gppi_vline (buf, pp->area.x1, pp->area.y0 + 1, pp->area.y1 - 1, pp->color_border);
		/* Shadow */
		if (SHADOW_SIZE > 0) {
			/* Right */
			gppi_rect (buf, pp->area.x1 + 1, pp->area.y0 + SHADOW_SIZE,
				   pp->area.x1 + SHADOW_SIZE, pp->area.y1 + SHADOW_SIZE, pp->color_shadow);
			/* Bottom */
			gppi_rect (buf, pp->area.x0 + SHADOW_SIZE, pp->area.y1 + 1,
				   pp->area.x1, pp->area.y1 + SHADOW_SIZE, pp->color_shadow);
		}
		/* Fill */
		gppi_rect (buf, pp->area.x0 + 1, pp->area.y0 + 1,
			   pp->area.x1 - 1, pp->area.y1 - 1, pp->color_page);

		/* Arrows */
		if (pp->up)
			gnome_canvas_render_svp (buf, pp->up, pp->color_arrow);
		if (pp->right)
			gnome_canvas_render_svp (buf, pp->right, pp->color_arrow);

		/* Fun part */

		/* Physical margins */
		imargin = (gint) fabs (pp->pml * pp->PPDP2C[0]);
		if (imargin > 0)
			gppi_vline (buf, pp->area.x0 + imargin, pp->area.y0 + 1, pp->area.y1 - 1, pp->color_pmargin);
		imargin = (gint) fabs (pp->pmr * pp->PPDP2C[0]);
		if (imargin > 0)
			gppi_vline (buf, pp->area.x1 - imargin, pp->area.y0 + 1, pp->area.y1 - 1, pp->color_pmargin);
		imargin = (gint) fabs (pp->pmt * pp->PPDP2C[3]);
		if (imargin > 0)
			gppi_hline (buf, pp->area.y0 + imargin, pp->area.x0 + 1, pp->area.x1 - 1, pp->color_pmargin);
		imargin = (gint) fabs (pp->pmb * pp->PPDP2C[3]);
		if (imargin > 0)
			gppi_hline (buf, pp->area.y1 - imargin, pp->area.x0 + 1, pp->area.x1 - 1, pp->color_pmargin);

		/* Extra fun */

		for (i = 0; i < pp->num_affines; i++) {
			gdouble l2p[6], l2c[6], lp2c[6];
			gdouble w, h, y;

			/* Calculate Layout -> Physical Page affine */
			memcpy (l2p, pp->affines + 6 * i, 6 * sizeof (gdouble));
			l2p[4] *= pp->pw;
			l2p[5] *= pp->ph;
			/* PRINT_AFFINE ("Layout -> Physical:", &l2p[0]); */
			/* Calculate Layout -> Canvas affine */
			art_affine_multiply (l2c, l2p, pp->PP2C);
			/* PRINT_AFFINE ("Layout -> Canvas:", &l2c[0]); */
			/* Calcualte Logical Page -> Canvas affine */
			art_affine_multiply (lp2c, pp->LP2LY, l2c);
			/* PRINT_AFFINE ("Logical Page -> Canvas:", &lp2c[0]); */

			/* Draw logical margins */
			gppi_tvline (buf, pp->lml, 0, pp->LH, lp2c,
				     pp->lml_hl ? pp->color_lmargin_hl : pp->color_lmargin);
			gppi_tvline (buf, pp->LW - pp->lmr, 0, pp->LH, lp2c,
				     pp->lml_hl ? pp->color_lmargin_hl : pp->color_lmargin);
			gppi_thline (buf, pp->LH - pp->lmt, 0, pp->LW, lp2c,
				     pp->lml_hl ? pp->color_lmargin_hl : pp->color_lmargin);
			gppi_thline (buf, pp->lmb, 0, pp->LW, lp2c,
				     pp->lml_hl ? pp->color_lmargin_hl : pp->color_lmargin);
			/* Render fancy page */
			w = pp->LW - pp->lml - pp->lmr;
			h = pp->LH - pp->lmt - pp->lmb;
			if ((w > 0) && (h > 0)) {
				ArtDRect a, r;
				y = h;
				if ((y >= CM(5)) && (w > CM(5))) {
					/* 5CM x 5CM box */
					r.x0 = pp->lml + 0;
					r.y0 = pp->lmb + y - CM(5);
					r.x1 = pp->lml + CM(5);
					r.y1 = pp->lmb + y;
					art_drect_affine_transform (&a, &r, lp2c);
					gppi_rect (buf, a.x0, a.y0, a.x1, a.y1, 0x0000007f);
					if (w >= CM(7)) {
						gint l;
						for (l = 0; l < 3; l++) {
							/* Short line */
							r.x0 = pp->lml + CM(6);
							r.y0 = pp->lmb + y - l * CM(2) - CM(1.5);
							r.x1 = pp->lml + w;
							r.y1 = pp->lmb + y - l * CM(2) - CM(0.5);
							art_drect_affine_transform (&a, &r, lp2c);
							gppi_rect (buf, a.x0, a.y0, a.x1, a.y1, 0x0000005f);
						}
					}
					y -= CM(6.5);
				}
				while (y > CM(1)) {
					/* Long line */
					r.x0 = pp->lml + 0;
					r.y0 = pp->lmb + y - CM(1);
					r.x1 = pp->lml + w;
					r.y1 = pp->lmb + y;
					art_drect_affine_transform (&a, &r, lp2c);
					gppi_rect (buf, a.x0, a.y0, a.x1, a.y1, 0x0000005f);
					y -= CM(2);
				}
			}
		}
	}
}

void
gpa_paper_preview_item_set_lm_highlights (GpaPaperPreviewItem *pp,
					    gboolean mt, gboolean mb, gboolean ml, gboolean mr)
{
	pp->lmt_hl = mt;
	pp->lmb_hl = mb;
	pp->lmr_hl = mr;
	pp->lml_hl = ml;

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

/* EXPERIMENTAL */
/* These will be remodelled via properties, if makes sense */

void
gpa_paper_preview_item_set_physical_size (GpaPaperPreviewItem *pp, gdouble width, gdouble height)
{
	pp->pw = CLAMP (MM(1), width, M(10));
	pp->ph = CLAMP (MM(1), height, M(10));

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

/**
 * gpa_paper_preview_item_set_physical_orientation:
 * @pp: 
 * @orientation: 
 * 
 * Only 4 fist values are used
 **/
void
gpa_paper_preview_item_set_physical_orientation (GpaPaperPreviewItem *pp, const gdouble *orientation)
{
	memcpy (pp->porient, orientation, 6 * sizeof (gdouble));

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

/**
 * gpa_paper_preview_item_set_logical_orientation:
 * @pp: 
 * @orientation: 
 * 
 * Only 4 first values are used
 **/
void
gpa_paper_preview_item_set_logical_orientation (GpaPaperPreviewItem *pp, const gdouble *orientation)
{
	memcpy (pp->lorient, orientation, 6 * sizeof (gdouble));

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

void
gpa_paper_preview_item_set_physical_margins (GpaPaperPreviewItem *pp, gdouble l, gdouble r, gdouble t, gdouble b)
{
	l = MAX (0, l);
	r = MAX (0, r);
	t = MAX (0, t);
	b = MAX (0, b);

	if ((l + r) > 0.0 && (l + r) > pp->pw) {
		l = l * pp->pw / (l + r);
		r = r * pp->pw / (l + r);
	}

	if ((t + b) > 0.0 && (t + b) > pp->ph) {
		t = t * pp->ph / (t + b);
		b = b * pp->ph / (t + b);
	}

	pp->pml = l;
	pp->pmr = r;
	pp->pmt = t;
	pp->pmb = b;

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

void
gpa_paper_preview_item_set_logical_margins (GpaPaperPreviewItem *pp, gdouble l, gdouble r, gdouble t, gdouble b)
{
	l = MAX (0, l);
	r = MAX (0, r);
	t = MAX (0, t);
	b = MAX (0, b);

	pp->lml = l;
	pp->lmr = r;
	pp->lmt = t;
	pp->lmb = b;

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

/**
 * gpa_paper_preview_item_set_layout:
 * @pp: 
 * @affines: 
 * @width: 
 * @height: 
 * 
 *  Layout size is applicable width and height in layout subpage 
 **/
void
gpa_paper_preview_item_set_layout (GpaPaperPreviewItem *pp, const GSList *affines, gdouble width, gdouble height)
{
	if (pp->affines) {
		g_free (pp->affines);
		pp->affines = NULL;
	}

	pp->num_affines = g_slist_length ((GSList *) affines);

	if (pp->num_affines > 0) {
		const GSList *l;
		gint i;
		pp->affines = g_new (gdouble, pp->num_affines * 6);
		i = 0;
		for (l = affines; l != NULL; l = l->next) {
			memcpy (pp->affines + 6 * i, l->data, 6 * sizeof (gdouble));
			i += 1;
		}
	}

	pp->lyw = CLAMP (0.001, width, 1000.0);
	pp->lyh = CLAMP (0.001, height, 1000.0);

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}


static void
gpa_paper_preview_item_reload_values (GPANode *node, guint flags, GpaPaperPreviewItem *item)
{
	gdouble ml, mr, mt, mb, w, h;
	GnomePrintLayoutData *lyd;
	GnomePrintConfig *config;

	config = item->config;
	
	/* Physical Size */
	w = 1;
	h = 1;

	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_WIDTH, &w, NULL);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_HEIGHT, &h, NULL);
	gpa_paper_preview_item_set_physical_size (item, w, h);

	/* Layout */
	lyd = gnome_print_config_get_layout_data (config, NULL, NULL, NULL, NULL);
	if (lyd != NULL) {
		GSList *l = NULL;
		gint i;

		gpa_paper_preview_item_set_logical_orientation  (item, lyd->lorient);
		gpa_paper_preview_item_set_physical_orientation (item, lyd->porient);
		for (i = lyd->num_pages; i ; ) {
			l = g_slist_prepend (l, &lyd->pages[--i].matrix);
		}
		gpa_paper_preview_item_set_layout (item, l, lyd->lyw, lyd->lyh);
		g_slist_free (l);
		gnome_print_layout_data_free (lyd);
	}


	/* Physical Margins */
	ml = MM(10);
	mr = MM(10);
	mt = MM(10);
	mb = MM(10);

	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_MARGIN_LEFT, &ml, NULL);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_MARGIN_RIGHT, &mr, NULL);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_MARGIN_TOP, &mt, NULL);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_MARGIN_BOTTOM, &mb, NULL);

	gpa_paper_preview_item_set_physical_margins (item, ml, mr, mt, mb);

	/* Logical Margins */
	ml = MM(10);
	mr = MM(10);
	mt = MM(10);
	mb = MM(10);

	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, &ml, NULL);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, &mr, NULL);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAGE_MARGIN_TOP, &mt, NULL);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, &mb, NULL);

	gpa_paper_preview_item_set_logical_margins (item, ml, mr, mt, mb);
}

#define PMARGIN_ALPHA_VALUE    0xbf
#define LMARGIN_ALPHA_VALUE    0xbf
#define STRIPE_ALPHA_VALUE     0x5f
#define FULL_ALPHA             0xff
#define GPA_COLOR_RGBA(color, ALPHA)              \
		((guint32) (ALPHA               | \
                (((color).red / 256) << 24)     | \
                (((color).green / 256) << 16)   | \
                (((color).blue  / 256) << 8)))
static void
gpa_paper_preview_item_load_colors (GpaPaperPreviewItem *item, GtkWidget *widget)
{
	GtkStyle *style;

	style = gtk_widget_get_style (widget);

	item->color_page       = GPA_COLOR_RGBA (style->base [GTK_STATE_NORMAL],   FULL_ALPHA);
	item->color_border     = GPA_COLOR_RGBA (style->text [GTK_STATE_NORMAL],   FULL_ALPHA);
	item->color_shadow     = GPA_COLOR_RGBA (style->bg   [GTK_STATE_ACTIVE],   FULL_ALPHA);
	item->color_arrow      = GPA_COLOR_RGBA (style->base [GTK_STATE_SELECTED], FULL_ALPHA);
	item->color_pmargin    = GPA_COLOR_RGBA (style->base [GTK_STATE_SELECTED], PMARGIN_ALPHA_VALUE);
	item->color_lmargin    = GPA_COLOR_RGBA (style->bg   [GTK_STATE_SELECTED], LMARGIN_ALPHA_VALUE);
	item->color_lmargin_hl = GPA_COLOR_RGBA (style->bg   [GTK_STATE_SELECTED], FULL_ALPHA);
	item->color_stripe     = GPA_COLOR_RGBA (style->text [GTK_STATE_NORMAL],   STRIPE_ALPHA_VALUE);
}

static void 
gpa_paper_preview_item_style_color_cb (GtkWidget *widget, GtkStyle *ps, GpaPaperPreviewItem *item)
{
	gpa_paper_preview_item_load_colors (item, widget);
}

static void
gpa_paper_preview_item_disconnect (GpaPaperPreviewItem *pp)
{
	gint max, i;

	max = sizeof (pp->node) / sizeof (GPANode *);
	for (i = 0; i < max; i++) {
		if (pp->handler[i]) {
			g_signal_handler_disconnect (pp->node[i],
						     pp->handler[i]);
			pp->handler[i] = 0;
			gpa_node_unref (pp->node[i]);
			pp->node[i] = NULL;
		}
	}
}
				   
static void
gpa_paper_preview_item_connect (GpaPaperPreviewItem *pp)
{
	const guchar *paths[] = {
		GNOME_PRINT_KEY_PAPER_SIZE,
		GNOME_PRINT_KEY_PAPER_ORIENTATION,
		GNOME_PRINT_KEY_PAGE_ORIENTATION,
		GNOME_PRINT_KEY_LAYOUT,
	};
	gint i, max;
	
	max = sizeof (pp->node) / sizeof (GPANode *);
	g_assert (max == (sizeof (paths) / sizeof (guchar *)));
	for (i = 0; i < max; i++) {
		pp->node[i] = gpa_node_get_child_from_path (pp->gpa_config, paths[i]);
		if (pp->node[i])
			pp->handler[i] = g_signal_connect (G_OBJECT (pp->node[i]), "modified",
							   (GCallback) gpa_paper_preview_item_reload_values, pp);
		else
			pp->handler[i] = 0;
	}
}

static void
gpa_paper_preview_item_config_modified_cb (GPANode *node, guint flags, GpaPaperPreviewItem *pp)
{
	gpa_paper_preview_item_disconnect (pp);
	gpa_paper_preview_item_connect (pp);
	gpa_paper_preview_item_reload_values (NULL, 0, pp);
}

GnomeCanvasItem *
gpa_paper_preview_item_new (GnomePrintConfig *config, GtkWidget *canvas)
{
	GpaPaperPreviewItem *pp;
	GnomeCanvasItem *item;

	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (canvas != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CANVAS (canvas), NULL);
	
	item = gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (canvas)), GPA_TYPE_PAPER_PREVIEW_ITEM, NULL);
	pp = GPA_PAPER_PREVIEW_ITEM (item);

	pp->config = gnome_print_config_ref (config);
	pp->gpa_config = GNOME_PRINT_CONFIG_NODE (pp->config);
	pp->handler_config = g_signal_connect (G_OBJECT (pp->gpa_config), "modified",(GCallback)
					       gpa_paper_preview_item_config_modified_cb, pp);
	gpa_paper_preview_item_load_colors (pp, canvas);
	g_signal_connect (G_OBJECT (canvas), "style_set", G_CALLBACK (gpa_paper_preview_item_style_color_cb), pp);
	gpa_paper_preview_item_reload_values (NULL, 0, pp);

	gpa_paper_preview_item_connect (pp);
	
	return item;
}



/* Drawing utils */


#define RGBA_R(v) ((v) >> 24)
#define RGBA_G(v) (((v) >> 16) & 0xff)
#define RGBA_B(v) (((v) >> 8) & 0xff)
#define RGBA_A(v) ((v) & 0xff)

/* Non-premultiplied alpha composition */
#define COMPOSE(b,f,a) (((255 - (a)) * b + (f * a) + 127) / 255)

static void
gppi_hline (GnomeCanvasBuf *buf, gint y, gint xs, gint xe, guint32 rgba)
{
	if ((y >= buf->rect.y0) && (y < buf->rect.y1)) {
		guint r, g, b, a;
		gint x0, x1, x;
		guchar *p;
		r = RGBA_R (rgba);
		g = RGBA_G (rgba);
		b = RGBA_B (rgba);
		a = RGBA_A (rgba);
		x0 = MAX (buf->rect.x0, xs);
		x1 = MIN (buf->rect.x1, xe + 1);
		p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x0 - buf->rect.x0) * 3;
		for (x = x0; x < x1; x++) {
			p[0] = COMPOSE (p[0], r, a);
			p[1] = COMPOSE (p[1], g, a);
			p[2] = COMPOSE (p[2], b, a);
			p += 3;
		}
	}
}

static void
gppi_vline (GnomeCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba)
{
	if ((x >= buf->rect.x0) && (x < buf->rect.x1)) {
		guint r, g, b, a;
		gint y0, y1, y;
		guchar *p;
		r = RGBA_R (rgba);
		g = RGBA_G (rgba);
		b = RGBA_B (rgba);
		a = RGBA_A (rgba);
		y0 = MAX (buf->rect.y0, ys);
		y1 = MIN (buf->rect.y1, ye + 1);
		p = buf->buf + (y0 - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 3;
		for (y = y0; y < y1; y++) {
			p[0] = COMPOSE (p[0], r, a);
			p[1] = COMPOSE (p[1], g, a);
			p[2] = COMPOSE (p[2], b, a);
			p += buf->buf_rowstride;
		}
	}
}

static void
gppi_rect (GnomeCanvasBuf *buf, gint xs, gint ys, gint xe, gint ye, guint32 rgba)
{
	guint r, g, b, a;
	gint x0, x1, x;
	gint y0, y1, y;
	guchar *p;
	r = RGBA_R (rgba);
	g = RGBA_G (rgba);
	b = RGBA_B (rgba);
	a = RGBA_A (rgba);
	x0 = MAX (buf->rect.x0, xs);
	x1 = MIN (buf->rect.x1, xe + 1);
	y0 = MAX (buf->rect.y0, ys);
	y1 = MIN (buf->rect.y1, ye + 1);
	for (y = y0; y < y1; y++) {
		p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride + (x0 - buf->rect.x0) * 3;
		for (x = x0; x < x1; x++) {
			p[0] = COMPOSE (p[0], r, a);
			p[1] = COMPOSE (p[1], g, a);
			p[2] = COMPOSE (p[2], b, a);
			p += 3;
		}
	}
}

static void
gppi_tvline (GnomeCanvasBuf *buf, gdouble x, gdouble sy, gdouble ey, gdouble *affine, guint32 rgba)
{
	gdouble x0, y0, x1, y1;

	x0 = affine[0] * x + affine[2] * sy + affine[4];
	y0 = affine[1] * x + affine[3] * sy + affine[5];
	x1 = affine[0] * x + affine[2] * ey + affine[4];
	y1 = affine[1] * x + affine[3] * ey + affine[5];

	if (fabs (x1 - x0) > fabs (y1 - y0)) {
		gppi_hline (buf, 0.5 * (y0 + y1), MIN (x0, x1), MAX (x0, x1), rgba);
	} else {
		gppi_vline (buf, 0.5 * (x0 + x1), MIN (y0, y1), MAX (y0, y1), rgba);
	}
}

static void
gppi_thline (GnomeCanvasBuf *buf, gdouble y, gdouble sx, gdouble ex, gdouble *affine, guint32 rgba)
{
	gdouble x0, y0, x1, y1;

	x0 = affine[0] * sx + affine[2] * y + affine[4];
	y0 = affine[1] * sx + affine[3] * y + affine[5];
	x1 = affine[0] * ex + affine[2] * y + affine[4];
	y1 = affine[1] * ex + affine[3] * y + affine[5];

	if (fabs (x1 - x0) > fabs (y1 - y0)) {
		gppi_hline (buf, 0.5 * (y0 + y1), MIN (x0, x1), MAX (x0, x1), rgba);
	} else {
		gppi_vline (buf, 0.5 * (x0 + x1), MIN (y0, y1), MAX (y0, y1), rgba);
	}
}

