/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-paper-selector.c: print preview driver
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
 *
 *  Copyright (C) 1998 James Henstridge <james@daa.com.au>
 *
 */

#define __GNOME_PRINT_PAPER_SELECTOR_C__

#include <config.h>

#include <string.h>
#include <math.h>
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_rect_svp.h>
#include <atk/atkobject.h>
#include <atk/atkrelationset.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtktable.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtktogglebutton.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <libgnomeprint/gnome-print-paper.h>
#include <libgnomeprint/private/gnome-print-private.h>
#include <libgnomeprint/private/gpa-private.h>
#include <libgnomeprint/gnome-print-config.h>
#include "gnome-print-i18n.h"
#include "gnome-print-paper-selector.h"

#define noGPP_VERBOSE

/* Paper preview CanvasItem */

#define GNOME_TYPE_PAPER_PREVIEW_ITEM (gnome_paper_preview_item_get_type ())
#define GNOME_PAPER_PREVIEW_ITEM(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PAPER_PREVIEW_ITEM, GnomePaperPreviewItem))
#define GNOME_PAPER_PREVIEW_ITEM_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GNOME_TYPE_PAPER_PREVIEW_ITEM, GnomePaperPreviewItemClass))
#define GNOME_IS_PAPER_PREVIEW_ITEM(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PAPER_PREVIEW_ITEM))
#define GNOME_IS_PAPER_PREVIEW_ITEM_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GNOME_TYPE_PAPER_PREVIEW_ITEM))
#define GNOME_PAPER_PREVIEW_ITEM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GNOME_TYPE_PAPER_PREVIEW_ITEM, GnomePaperPreviewItemClass))

typedef struct _GnomePaperPreviewItem GnomePaperPreviewItem;
typedef struct _GnomePaperPreviewItemClass GnomePaperPreviewItemClass;

GType gnome_paper_preview_item_get_type (void);

GnomeCanvasItem *gnome_paper_preview_item_new (GnomePrintConfig *config, GnomeCanvasGroup *parent);

/* EXPERIMENTAL */
/* These will be remodelled via properties, if makes sense */

void gnome_paper_preview_item_set_physical_size (GnomePaperPreviewItem *item, gdouble width, gdouble height);
/* NB! Only first 4 values are used */
void gnome_paper_preview_item_set_physical_orientation (GnomePaperPreviewItem *item, const gdouble *orientation);
/* NB! Only first 4 values are used */
void gnome_paper_preview_item_set_logical_orientation (GnomePaperPreviewItem *item, const gdouble *orientation);
void gnome_paper_preview_item_set_physical_margins (GnomePaperPreviewItem *item, gdouble l, gdouble r, gdouble t, gdouble b);
void gnome_paper_preview_item_set_logical_margins (GnomePaperPreviewItem *item, gdouble l, gdouble r, gdouble t, gdouble b);
/* NB! Layout size is applicable width and height in layout subpage */
void gnome_paper_preview_item_set_layout (GnomePaperPreviewItem *item, const GSList *affines, gdouble width, gdouble height);

#define MM(v) ((v) * 72.0 / 25.4)
#define CM(v) ((v) * 72.0 / 2.54)
#define M(v) ((v) * 72.0 / 0.0254)
#define GDK_COLOR_RGBA(color, ALPHA)                                        \
                               ((guint32) (ALPHA                          | \
                                          (((color).red / 256) << 24)     | \
                                          (((color).green / 256) << 16)   | \
                                          (((color).blue  / 256) << 8)))

#define EPSILON 1e-9
#define PAD 4
#define SHADOW_SIZE 5
#define BORDER_ALPHA_VALUE     0xff
#define SHADOW_ALPHA_VALUE     0xff
#define PAGE_ALPHA_VALUE       0xff
#define PMARGIN_ALPHA_VALUE    0xbf
#define LMARGIN_ALPHA_VALUE    0xbf
#define LMARGIN_ALPHA_VALUE_HL 0xff
#define ARROW_ALPHA_VALUE      0xff
#define STRIPE_ALPHA_VALUE     0x5f

struct _GnomePaperPreviewItem {
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

	/* fixme: remove this (Lauris) */
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
};

struct _GnomePaperPreviewItemClass {
	GnomeCanvasItemClass parent_class;
};

static void gnome_paper_preview_item_class_init (GnomePaperPreviewItemClass *klass);
static void gnome_paper_preview_item_init (GnomePaperPreviewItem *item);
static void gnome_paper_preview_item_finalize (GObject *object);

static void gnome_paper_preview_item_update (GnomeCanvasItem *item, gdouble *affine, ArtSVP *clip, gint flags);
static void gnome_paper_preview_item_render (GnomeCanvasItem *item, GnomeCanvasBuf *buf);

static void gppi_hline (GnomeCanvasBuf *buf, gint y, gint xs, gint xe, guint32 rgba);
static void gppi_vline (GnomeCanvasBuf *buf, gint x, gint ys, gint ye, guint32 rgba);
static void gppi_rect (GnomeCanvasBuf *buf, gint xs, gint ys, gint xe, gint ye, guint32 rgba);
static void gppi_tvline (GnomeCanvasBuf *buf, gdouble x, gdouble sy, gdouble ey, gdouble *affine, guint32 rgba);
static void gppi_thline (GnomeCanvasBuf *buf, gdouble y, gdouble sx, gdouble ex, gdouble *affine, guint32 rgba);
static void style_color_callback(GtkWidget *widget, GtkStyle *ps, void *data);
static void theme_color_init();


static GnomeCanvasItemClass *item_parent_class;

guint32 PAGE_COLOR;
guint32 BORDER_COLOR;
guint32 SHADOW_COLOR;
guint32 ARROW_COLOR;
guint32 PMARGIN_COLOR;
guint32 LMARGIN_COLOR;
guint32 LMARGIN_COLOR_HL;
guint32 STRIPE_COLOR;

GType
gnome_paper_preview_item_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePaperPreviewItemClass),
			NULL, NULL,
			(GClassInitFunc) gnome_paper_preview_item_class_init,
			NULL, NULL,
			sizeof (GnomePaperPreviewItem),
			0,
			(GInstanceInitFunc) gnome_paper_preview_item_init
		};
		type = g_type_register_static (GNOME_TYPE_CANVAS_ITEM, "GnomePaperPreviewItem", &info, 0);
	}
	return type;
}

static void
gnome_paper_preview_item_class_init (GnomePaperPreviewItemClass *klass)
{
	GObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	object_class = G_OBJECT_CLASS (klass);
	item_class = GNOME_CANVAS_ITEM_CLASS (klass);

	item_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_paper_preview_item_finalize;

	item_class->update = gnome_paper_preview_item_update;
	item_class->render = gnome_paper_preview_item_render;
}

static void
gnome_paper_preview_item_init (GnomePaperPreviewItem *pp)
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
gnome_paper_preview_item_finalize (GObject *object)
{
	GnomePaperPreviewItem *pp;

	pp = GNOME_PAPER_PREVIEW_ITEM (object);

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

	G_OBJECT_CLASS (item_parent_class)->finalize (object);
}

#define WIDGET_WIDTH(w) (GTK_WIDGET (w)->allocation.width)
#define WIDGET_HEIGHT(w) (GTK_WIDGET (w)->allocation.height)

#ifdef GPP_VERBOSE
#define PRINT_2(s,a,b) g_print ("%s %g %g\n", (s), (a), (b))
#define PRINT_DRECT(s,a) g_print ("%s %g %g %g %g\n", (s), (a)->x0, (a)->y0, (a)->x1, (a)->y1)
#define PRINT_AFFINE(s,a) g_print ("%s %g %g %g %g %g %g\n", (s), *(a), *((a) + 1), *((a) + 2), *((a) + 3), *((a) + 4), *((a) + 5))
#else
#define PRINT_2(s,a,b)
#define PRINT_DRECT(s,a)
#define PRINT_AFFINE(s,a)
#endif

#define SQ(v) ((v) * (v))


static void 
theme_color_init (GtkWidget *widget)
{
	GdkColor color;
	GtkStyle *style;

	style = gtk_widget_get_style (widget);

	PAGE_COLOR = 0;
	color = style->base[GTK_STATE_NORMAL];
	PAGE_COLOR = GDK_COLOR_RGBA (color, PAGE_ALPHA_VALUE);

	BORDER_COLOR = 0;
	color = style->text[GTK_STATE_NORMAL];
	BORDER_COLOR = GDK_COLOR_RGBA (color, BORDER_ALPHA_VALUE);

	SHADOW_COLOR = 0;
	color = style->bg[GTK_STATE_ACTIVE];
	SHADOW_COLOR = GDK_COLOR_RGBA (color, SHADOW_ALPHA_VALUE);

	ARROW_COLOR = 0;
	color = style->base[GTK_STATE_SELECTED];
	ARROW_COLOR = GDK_COLOR_RGBA (color, ARROW_ALPHA_VALUE);

	PMARGIN_COLOR = 0;
	color = style->base[GTK_STATE_SELECTED];
	PMARGIN_COLOR = GDK_COLOR_RGBA (color, PMARGIN_ALPHA_VALUE);

	LMARGIN_COLOR = 0;
	color = style->bg[GTK_STATE_SELECTED];
	LMARGIN_COLOR = GDK_COLOR_RGBA (color, LMARGIN_ALPHA_VALUE);

	LMARGIN_COLOR_HL = 0;
	color = style->bg[GTK_STATE_SELECTED];
	LMARGIN_COLOR_HL = GDK_COLOR_RGBA (color, LMARGIN_ALPHA_VALUE_HL); 

	STRIPE_COLOR = 0;
	color = style->text[GTK_STATE_NORMAL];
	STRIPE_COLOR = GDK_COLOR_RGBA (color, STRIPE_ALPHA_VALUE);

}

static void 
style_color_callback (GtkWidget *widget, GtkStyle *ps, void *data)
{

	theme_color_init (widget);
}



static void
gnome_paper_preview_item_update (GnomeCanvasItem *item, gdouble *affine, ArtSVP *clip, gint flags)
{
	GnomePaperPreviewItem *pp;
	ArtDRect area, r, PPDC;
	gdouble xscale, yscale, scale, t;
	gdouble PPDP2C[6], a[6], b[6];
	ArtVpath arrow[] = {{ART_MOVETO, -1, 0.001}, {ART_LINETO, 0, 1}, {ART_LINETO, 1, 0}, {ART_LINETO, -1, 0.001}, {ART_END, 0, 0}};
	ArtVpath *vpath;
	ArtSVP *svp;

	pp = GNOME_PAPER_PREVIEW_ITEM (item);

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

	if (pp->num_affines < 1) return;
	if ((fabs (pp->pw) < EPSILON) || (fabs (pp->ph) < EPSILON)) return;

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
	if ((pp->PAW < EPSILON) || (pp->PAH < EPSILON)) return;

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
	if ((pp->LW < EPSILON) || (pp->LH < EPSILON)) return;
	PRINT_2 ("L Dimensions", pp->LW, pp->LH);
	/* Now complete matrix calculation */
	t = pp->LW * pp->LP2LY[0] + pp->LH * pp->LP2LY[2];
	pp->LP2LY[4] = (t < 0) ? -t : 0.0;
	t = pp->LW * pp->LP2LY[1] + pp->LH * pp->LP2LY[3];
	pp->LP2LY[5] = (t < 0) ? -t : 0.0;
	PRINT_AFFINE ("LP2LY:", &pp->LP2LY[0]);

	/* PPDC - Physical Page Dimensions on Canvas */

	/* Now we calculate PPDP -> Canvas transformation */
	/* fixme: This should be done by group or something? (Lauris)  */

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
	if ((PPDC.x0 >= PPDC.x1) || (PPDC.y0 >= PPDC.y1)) return;
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
cb_gnome_paper_preview_item_request_update (GPANode *node, guint flags, GnomeCanvasItem *pp)
{
	gnome_canvas_item_request_update (pp);
}

static void
gnome_paper_preview_item_render (GnomeCanvasItem *item, GnomeCanvasBuf *buf)
{
	GnomePaperPreviewItem *pp;

	pp = GNOME_PAPER_PREVIEW_ITEM (item);

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
		gppi_hline (buf, pp->area.y0, pp->area.x0, pp->area.x1, BORDER_COLOR);
		/* Bottom */
		gppi_hline (buf, pp->area.y1, pp->area.x0, pp->area.x1, BORDER_COLOR);
		/* Left */
		gppi_vline (buf, pp->area.x0, pp->area.y0 + 1, pp->area.y1 - 1, BORDER_COLOR);
		/* Right */
		gppi_vline (buf, pp->area.x1, pp->area.y0 + 1, pp->area.y1 - 1, BORDER_COLOR);
		/* Shadow */
		if (SHADOW_SIZE > 0) {
			/* Right */
			gppi_rect (buf, pp->area.x1 + 1, pp->area.y0 + SHADOW_SIZE,
				   pp->area.x1 + SHADOW_SIZE, pp->area.y1 + SHADOW_SIZE, SHADOW_COLOR);
			/* Bottom */
			gppi_rect (buf, pp->area.x0 + SHADOW_SIZE, pp->area.y1 + 1,
				   pp->area.x1, pp->area.y1 + SHADOW_SIZE, SHADOW_COLOR);
		}
		/* Fill */
		gppi_rect (buf, pp->area.x0 + 1, pp->area.y0 + 1,
			   pp->area.x1 - 1, pp->area.y1 - 1, PAGE_COLOR);

		/* Arrows */
		if (pp->up) gnome_canvas_render_svp (buf, pp->up, ARROW_COLOR);
		if (pp->right) gnome_canvas_render_svp (buf, pp->right, ARROW_COLOR);

		/* Fun part */

		/* Physical margins */
		imargin = (gint) fabs (pp->pml * pp->PPDP2C[0]);
		if (imargin > 0) gppi_vline (buf, pp->area.x0 + imargin, pp->area.y0 + 1, pp->area.y1 - 1, PMARGIN_COLOR);
		imargin = (gint) fabs (pp->pmr * pp->PPDP2C[0]);
		if (imargin > 0) gppi_vline (buf, pp->area.x1 - imargin, pp->area.y0 + 1, pp->area.y1 - 1, PMARGIN_COLOR);
		imargin = (gint) fabs (pp->pmt * pp->PPDP2C[3]);
		if (imargin > 0) gppi_hline (buf, pp->area.y0 + imargin, pp->area.x0 + 1, pp->area.x1 - 1, PMARGIN_COLOR);
		imargin = (gint) fabs (pp->pmb * pp->PPDP2C[3]);
		if (imargin > 0) gppi_hline (buf, pp->area.y1 - imargin, pp->area.x0 + 1, pp->area.x1 - 1, PMARGIN_COLOR);

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
				     pp->lml_hl ? LMARGIN_COLOR_HL : LMARGIN_COLOR);
			gppi_tvline (buf, pp->LW - pp->lmr, 0, pp->LH, lp2c,
				     pp->lmr_hl ? LMARGIN_COLOR_HL : LMARGIN_COLOR);
			gppi_thline (buf, pp->LH - pp->lmt, 0, pp->LW, lp2c,
				     pp->lmt_hl ? LMARGIN_COLOR_HL : LMARGIN_COLOR);
			gppi_thline (buf, pp->lmb, 0, pp->LW, lp2c,
				     pp->lmb_hl ? LMARGIN_COLOR_HL : LMARGIN_COLOR);
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
					gppi_rect (buf, a.x0, a.y0, a.x1, a.y1, STRIPE_COLOR);
					if (w >= CM(7)) {
						gint l;
						for (l = 0; l < 3; l++) {
							/* Short line */
							r.x0 = pp->lml + CM(6);
							r.y0 = pp->lmb + y - l * CM(2) - CM(1.5);
							r.x1 = pp->lml + w;
							r.y1 = pp->lmb + y - l * CM(2) - CM(0.5);
							art_drect_affine_transform (&a, &r, lp2c);
							gppi_rect (buf, a.x0, a.y0, a.x1, a.y1, STRIPE_COLOR);
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
					gppi_rect (buf, a.x0, a.y0, a.x1, a.y1, STRIPE_COLOR);
					y -= CM(2);
				}
			}
		}
	}
}

GnomeCanvasItem *
gnome_paper_preview_item_new (GnomePrintConfig *config, GnomeCanvasGroup *parent)
{
	GnomeCanvasItem *item;

	item = gnome_canvas_item_new (parent, GNOME_TYPE_PAPER_PREVIEW_ITEM, NULL);

	return item;
}

static void
gnome_paper_preview_item_set_lm_highlights (GnomePaperPreviewItem *pp,
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
gnome_paper_preview_item_set_physical_size (GnomePaperPreviewItem *pp, gdouble width, gdouble height)
{
	pp->pw = CLAMP (MM(1), width, M(10));
	pp->ph = CLAMP (MM(1), height, M(10));

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

void
gnome_paper_preview_item_set_physical_orientation (GnomePaperPreviewItem *pp, const gdouble *orientation)
{
	memcpy (pp->porient, orientation, 6 * sizeof (gdouble));

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

void
gnome_paper_preview_item_set_logical_orientation (GnomePaperPreviewItem *pp, const gdouble *orientation)
{
	memcpy (pp->lorient, orientation, 6 * sizeof (gdouble));

	gnome_canvas_item_request_update (GNOME_CANVAS_ITEM (pp));
}

void
gnome_paper_preview_item_set_physical_margins (GnomePaperPreviewItem *pp, gdouble l, gdouble r, gdouble t, gdouble b)
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
gnome_paper_preview_item_set_logical_margins (GnomePaperPreviewItem *pp, gdouble l, gdouble r, gdouble t, gdouble b)
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

void
gnome_paper_preview_item_set_layout (GnomePaperPreviewItem *pp, const GSList *affines, gdouble width, gdouble height)
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

/* Drawing helpers */

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


/*
 * GnomePaperPreview widget
 */

struct _GnomePaperPreview {
	GtkHBox box;

	GtkWidget *canvas;

	GnomeCanvasItem *item;
};

struct _GnomePaperPreviewClass {
	GtkHBoxClass parent_class;
};

static void gnome_paper_preview_class_init (GnomePaperPreviewClass *klass);
static void gnome_paper_preview_init (GnomePaperPreview *preview);
static void gnome_paper_preview_finalize (GObject *object);

static void gnome_paper_preview_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static GtkHBoxClass *preview_parent_class;

GType
gnome_paper_preview_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePaperPreviewClass),
			NULL, NULL,
			(GClassInitFunc) gnome_paper_preview_class_init,
			NULL, NULL,
			sizeof (GnomePaperPreview),
			0,
			(GInstanceInitFunc) gnome_paper_preview_init
		};
		type = g_type_register_static (GTK_TYPE_HBOX, "GnomePaperPreview", &info, 0);
	}
	return type;
}

static void
gnome_paper_preview_class_init (GnomePaperPreviewClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	preview_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_paper_preview_finalize;

	widget_class->size_allocate = gnome_paper_preview_size_allocate;
}

static void
gnome_paper_preview_init (GnomePaperPreview *preview)
{
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	preview->canvas = gnome_canvas_new_aa ();
	gtk_widget_pop_colormap ();

	preview->item = gnome_paper_preview_item_new (NULL, gnome_canvas_root (GNOME_CANVAS (preview->canvas)));

	gtk_box_pack_start (GTK_BOX (preview), GTK_WIDGET (preview->canvas), TRUE, TRUE, 0);
	gtk_widget_show (GTK_WIDGET (preview->canvas));
}

static void
gnome_paper_preview_finalize (GObject *object)
{
	GnomePaperPreview *preview;

	preview = GNOME_PAPER_PREVIEW (object);

	preview->item = NULL;
	preview->canvas = NULL;

	G_OBJECT_CLASS (item_parent_class)->finalize (object);
}

static void
gnome_paper_preview_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GnomePaperPreview *pp;

	pp = GNOME_PAPER_PREVIEW (widget);

	gnome_canvas_set_scroll_region (GNOME_CANVAS (pp->canvas), 0, 0, allocation->width + 50, allocation->height + 50);

	if (((GtkWidgetClass *) preview_parent_class)->size_allocate)
		((GtkWidgetClass *) preview_parent_class)->size_allocate (widget, allocation);

	gnome_canvas_item_request_update (pp->item);
}

GtkWidget *
gnome_paper_preview_new (GnomePrintConfig *config)
{
	GnomePaperPreview *preview;
	GnomePaperPreviewItem *item;
	gdouble ml, mr, mt, mb, w, h;
	GnomePrintLayoutData *lyd;

	preview = GNOME_PAPER_PREVIEW (gtk_type_new (GNOME_TYPE_PAPER_PREVIEW));
	item = GNOME_PAPER_PREVIEW_ITEM (preview->item);

	theme_color_init(GTK_WIDGET(preview));

	/* Read out some important values */
	w = 1;
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_WIDTH, &w, NULL);
	h = 1;
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_HEIGHT, &h, NULL);
	gnome_paper_preview_item_set_physical_size (item, w, h);

	lyd = gnome_print_config_get_layout_data (config, NULL, NULL, NULL, NULL);
	if (lyd != NULL) {
		GSList *l = NULL;
		gint i;

		gnome_paper_preview_item_set_logical_orientation (item, lyd->lorient);
		gnome_paper_preview_item_set_physical_orientation (item, lyd->porient);
		for (i = lyd->num_pages; i ; ) {
			l = g_slist_prepend (l, &lyd->pages[--i].matrix);
		}
		gnome_paper_preview_item_set_layout (item, l, lyd->lyw, lyd->lyh);
		g_slist_free (l);
		gnome_print_layout_data_free (lyd);
	}


	ml = MM(10);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_MARGIN_LEFT, &ml, NULL);

	mr = MM(10);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_MARGIN_RIGHT, &mr, NULL);

	mt = MM(10);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_MARGIN_TOP, &mt, NULL);

	mb = MM(10);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_MARGIN_BOTTOM, &mb, NULL);

	gnome_paper_preview_item_set_physical_margins (item, ml, mr, mt, mb);

	ml = MM(10);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, &ml, NULL);

	mr = MM(10);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, &mr, NULL);

	mt = MM(10);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAGE_MARGIN_TOP, &mt, NULL);

	mb = MM(10);
	gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, &mb, NULL);

	gnome_paper_preview_item_set_logical_margins (item, ml, mr, mt, mb);

	g_signal_connect (G_OBJECT (preview), "style_set", G_CALLBACK (style_color_callback), preview);

	return GTK_WIDGET(preview);
}

/*
 * GnomePaperSelector widget
 */

struct _GnomePaperSelector {
	GtkHBox box;

	guint blocked : 1;

	GnomePrintConfig *config;
	gint flags;

	GtkWidget *preview;

	GtkWidget *pmenu, *pomenu, *lomenu, *lymenu;
	GtkWidget *pw, *ph, *us;
	GtkWidget *pf;
	gdouble w, h;

	GtkWidget *margin_frame;
	GtkSpinButton *margin_top,  *margin_bottom, *margin_left, *margin_right;
	gdouble mt, mb, ml, mr;

	guint handler_unit, handler_preview;
};

struct _GnomePaperSelectorClass {
	GtkHBoxClass parent_class;
};

static void gnome_paper_selector_class_init (GnomePaperSelectorClass *klass);
static void gnome_paper_selector_init (GnomePaperSelector *selector);
static void gnome_paper_selector_finalize (GObject *object);

static GtkHBoxClass *selector_parent_class;

GType
gnome_paper_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePaperSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gnome_paper_selector_class_init,
			NULL, NULL,
			sizeof (GnomePaperSelector),
			0,
			(GInstanceInitFunc) gnome_paper_selector_init
		};
		type = g_type_register_static (GTK_TYPE_HBOX, "GnomePaperSelector", &info, 0);
	}
	return type;
}

static void
gnome_paper_selector_class_init (GnomePaperSelectorClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	selector_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_paper_selector_finalize;
}

static void
gps_paper_activate (GtkWidget *widget, GnomePaperSelector *ps)
{
	GPANode *paper;
	guchar *id;
	const GnomePrintUnit *keyunit, *unit;
	gdouble val;

	paper = gtk_object_get_data (GTK_OBJECT (widget), "node");
	id = gpa_node_get_value (paper);
	gnome_print_config_set (ps->config, GNOME_PRINT_KEY_PAPER_SIZE, id);

	unit = gnome_print_unit_selector_get_unit (GNOME_PRINT_UNIT_SELECTOR (ps->us));

	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAPER_WIDTH, &val, &keyunit);
	gnome_print_convert_distance (&val, keyunit, unit);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (ps->pw), val);
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAPER_HEIGHT, &val, &keyunit);
	gnome_print_convert_distance (&val, keyunit, unit);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (ps->ph), val);

	/* fixme: Translation fucks that up */
	if (!strcmp (id, "Custom")) {
		gtk_widget_set_sensitive (ps->pw, TRUE);
		gtk_widget_set_sensitive (ps->ph, TRUE);
	} else {
		gtk_widget_set_sensitive (ps->pw, FALSE);
		gtk_widget_set_sensitive (ps->ph, FALSE);
	}

	g_free (id);
}

/* Create menu */
/* Lets be a bit more smart here (Lauris) */

static void
gps_menu_create (GtkWidget *om, GnomePrintConfig *config, const guchar *key, const guchar *emsg, GtkSignalFunc cb, gpointer cbdata)
{
	GPANode *cnode;
	GtkWidget *m;
	gint pos, sel;

	m = gtk_menu_new ();
	gtk_widget_show (m);

	pos = sel = 0;
	cnode = GNOME_PRINT_CONFIG_NODE (config);
	if (cnode) {
		GPANode *option;
		guchar *optionkey, *current;
		current = gnome_print_config_get (config, key);
		optionkey = g_strdup_printf ("%s.Option", key);
		option = gpa_node_get_path_node (cnode, optionkey);
		g_free (optionkey);
		if (option) {
			GPANode *item;
			for (item = gpa_node_get_child (option, NULL); item != NULL; item = gpa_node_get_child (option, item)) {
				guchar *id, *name;
				id = gpa_node_get_value (item);
				name = gpa_node_get_path_value (item, "Name");
				if (id && *id && name && *name) {
					GtkWidget *i;
					gpa_node_ref (item);
					i = gtk_menu_item_new_with_label (name);
					gtk_object_set_data_full (GTK_OBJECT (i), "node", item, (GtkDestroyNotify) gpa_node_unref);
					gtk_signal_connect (GTK_OBJECT (i), "activate", GTK_SIGNAL_FUNC (cb), cbdata);
					gtk_widget_show (i);
					gtk_menu_shell_append (GTK_MENU_SHELL (m), i);
					if (current && !strcmp (id, current)) sel = pos;
					pos += 1;
				}
				if (name) g_free (name);
				if (id) g_free (id);
				gpa_node_unref (item);
			}
			gpa_node_unref (option);
		}
		if (current) g_free (current);
	}

	if (pos < 1) {
		GtkWidget *i;
		i = gtk_menu_item_new_with_label (emsg);
		gtk_widget_show (i);
		gtk_menu_shell_append (GTK_MENU_SHELL (m), i);
	}

	gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
	gtk_option_menu_set_history (GTK_OPTION_MENU (om), sel);
	gtk_widget_set_sensitive (om, (pos > 0));
}

typedef struct {
	guchar *id, *name;
	gdouble affine[6];
} GPPOrientation;

static const GPPOrientation porient[] = {
	{"R0", "Straight", {1, 0, 0, 1, 0, 0}},
	{"R90", "Rotated 90 degrees", {0, -1, 1, 0, 0, 1}},
	{"R180", "Rotated 180 degrees", {-1, 0, 0, -1, 1, 1}},
	{"R270", "Rotated 270 degrees", {0, 1, -1, 0, 1, 0}}
};

static const GPPOrientation lorient[] = {
	{"R0", "Portrait", {1, 0, 0, 1, 0, 0}},
	{"R90", "Landscape", {0, 1, -1, 0, 0, 1}},
	{"R180", "Upside down portrait", {-1, 0, 0, -1, 1, 1}},
	{"R270", "Upside down landscape", {0, -1, 1, 0, 1, 0}}
};

static void
gps_feed_orientation_activate (GtkWidget *widget, GnomePaperSelector *ps)
{
	GPANode *node;
	guchar *id;
	GnomePrintLayoutData *lyd;

	node = gtk_object_get_data (GTK_OBJECT (widget), "node");
	id = gpa_node_get_value (node);
	gnome_print_config_set (ps->config, GNOME_PRINT_KEY_PAPER_ORIENTATION, id);
	g_free (id);

	/* Parse config */
	lyd = gnome_print_config_get_layout_data (ps->config, NULL, NULL, NULL, NULL);
	g_return_if_fail (lyd != NULL);
	gnome_paper_preview_item_set_physical_orientation (GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item), lyd->porient);
	gnome_print_layout_data_free (lyd);
}

static void
gps_page_orientation_activate (GtkWidget *widget, GnomePaperSelector *ps)
{
	GPANode *node;
	guchar *id;
	GnomePrintLayoutData *lyd;

	node = gtk_object_get_data (GTK_OBJECT (widget), "node");
	id = gpa_node_get_value (node);
	gnome_print_config_set (ps->config, GNOME_PRINT_KEY_PAGE_ORIENTATION, id);
	g_free (id);

	/* Parse config */
	lyd = gnome_print_config_get_layout_data (ps->config, NULL, NULL, NULL, NULL);
	g_return_if_fail (lyd != NULL);
	gnome_paper_preview_item_set_logical_orientation (GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item), lyd->lorient);
	gnome_print_layout_data_free (lyd);
}

typedef struct {
	guchar *id;
	guchar *name;
	gdouble width, height;
	gint num_affines;
	const gdouble *affines;
} GPPLayout;

static const gdouble lyid[] = {1, 0, 0, 1, 0, 0};
static const gdouble ly21[] = {0, -0.707, 0.707, 0, 0, 1,
			       0, -0.707, 0.707, 0, 0, 0.5};
static const gdouble ly41[] = {0.5, 0, 0, 0.5, 0, 0.5,
			       0.5, 0, 0, 0.5, 0.5, 0.5,
			       0.5, 0, 0, 0.5, 0, 0,
			       0.5, 0, 0, 0.5, 0.5, 0};
static const gdouble ly2[] = {0, -1, 1, 0, 0, 1,
			       0, -1, 1, 0, 0, 0.5};
static const gdouble ly2f[] = {0, -1, 1, 0, 0, 0.5,
			       0, 1, -1, 0, 1, 0.5};

static const GPPLayout layout[] = {
	{"Plain", "Plain", 1, 1, 1, lyid},
	{"2_1", "2 Pages to 1", 0.5, 1, 2, ly21},
	{"4_1", "4 Pages to 1", 0.5, 0.5, 4, ly41},
	{"I2_1", "Divided", 0.5, 1, 2, ly2},
	{"IM2_1", "Folded", 0.5, 1, 2, ly2f}
};
#define NUM_LAYOUTS (sizeof (layout) / sizeof (layout[0]))

static void
gps_layout_activate (GtkWidget *widget, GnomePaperSelector *ps)
{
	GnomePrintLayoutData *lyd;
	GPANode *node;
	guchar *id;
	GSList *l;
	gint i;

	node = gtk_object_get_data (GTK_OBJECT (widget), "node");
	id = gpa_node_get_value (node);
	gnome_print_config_set (ps->config, GNOME_PRINT_KEY_LAYOUT, id);
	g_free (id);

	/* Parse config */
	lyd = gnome_print_config_get_layout_data (ps->config, NULL, NULL, NULL, NULL);
	g_return_if_fail (lyd != NULL);
	l = NULL;
	for (i = 0; i < lyd->num_pages; i++) {
		l = g_slist_prepend (l, &lyd->pages[i].matrix);
	}
	l = g_slist_reverse (l);
	gnome_paper_preview_item_set_layout (GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item), l, lyd->lyw, lyd->lyh);
	g_slist_free (l);
	gnome_print_layout_data_free (lyd);
}

static void
gps_psize_value_changed (GtkAdjustment *adj, GnomePaperSelector *ps)
{
	const GnomePrintUnit *unit;
	gdouble w, h, max_wh;

	unit = gnome_print_unit_selector_get_unit (GNOME_PRINT_UNIT_SELECTOR (ps->us));
	w = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->pw));

	gnome_print_convert_distance (&w, unit, GNOME_PRINT_PS_UNIT);
	h = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->ph));

	gnome_print_convert_distance (&h, unit, GNOME_PRINT_PS_UNIT);

	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAPER_WIDTH,
				       w, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAPER_HEIGHT,
				       h, GNOME_PRINT_PS_UNIT);

	if ((fabs (ps->w - w) < 0.1) && (fabs (ps->h - h) < 0.1))
		return;

	gnome_paper_preview_item_set_physical_size (GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item), w, h);
	ps->w = w;
	ps->h = h;
	max_wh = MAX (w, h);
	gtk_spin_button_get_adjustment (ps->margin_top)->upper = max_wh;
	gtk_adjustment_changed (gtk_spin_button_get_adjustment (ps->margin_top));
	gtk_spin_button_get_adjustment (ps->margin_bottom)->upper = max_wh;
	gtk_adjustment_changed (gtk_spin_button_get_adjustment (ps->margin_bottom));
	gtk_spin_button_get_adjustment (ps->margin_left)->upper = max_wh;
	gtk_adjustment_changed (gtk_spin_button_get_adjustment (ps->margin_left));
	gtk_spin_button_get_adjustment (ps->margin_right)->upper = max_wh;
	gtk_adjustment_changed (gtk_spin_button_get_adjustment (ps->margin_right));
}

static void
gps_m_size_value_changed (GtkAdjustment *adj, GnomePaperSelector *ps)
{
	const GnomePrintUnit *unit;
	gdouble mt, mb, ml, mr;

	unit = gnome_print_unit_selector_get_unit (GNOME_PRINT_UNIT_SELECTOR (ps->us));
	mt = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->margin_top));
#ifdef GPP_VERBOSE
	g_print ("Top Margin %g\n", mt);
#endif
	gnome_print_convert_distance (&mt, unit, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_TOP, mt, GNOME_PRINT_PS_UNIT);

	mb = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->margin_bottom));
#ifdef GPP_VERBOSE
	g_print ("Bottom Margin %g\n", mb);
#endif
	gnome_print_convert_distance (&mb, unit, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, mb, GNOME_PRINT_PS_UNIT);

	ml = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->margin_left));
#ifdef GPP_VERBOSE
	g_print ("Bottom Margin %g\n", ml);
#endif
	gnome_print_convert_distance (&ml, unit, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, ml, GNOME_PRINT_PS_UNIT);

	mr = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->margin_right));
#ifdef GPP_VERBOSE
	g_print ("Bottom Margin %g\n", mr);
#endif
	gnome_print_convert_distance (&mr, unit, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, mr, GNOME_PRINT_PS_UNIT);

	if ((fabs (ps->mt - mt) < 0.1) && (fabs (ps->mb - mb) < 0.1) &&
	    (fabs (ps->ml - ml) < 0.1) && (fabs (ps->mr - mr) < 0.1)) return;

	gnome_paper_preview_item_set_logical_margins (GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item), ml, mr, mt, mb);
	ps->ml = ml;
	ps->mr = mr;
	ps->mt = mt;
	ps->mb = mb;

}

static void
gps_set_labelled_by_relation (GtkWidget *widget, GtkWidget *label)
{
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *relation_targets[1];
	AtkObject *atko;

	atko = gtk_widget_get_accessible (widget);
	relation_set = atk_object_ref_relation_set (atko);
	relation_targets[0] = gtk_widget_get_accessible (label);
	relation = atk_relation_new (relation_targets, 1,
				     ATK_RELATION_LABELLED_BY);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
	g_object_unref (G_OBJECT (relation_set));
}

static void
gnome_paper_selector_init (GnomePaperSelector *ps)
{
	ps->config = NULL;
	ps->flags = 0;
}

static void
gnome_paper_selector_finalize (GObject *object)
{
	GnomePaperSelector *selector;

	selector = GNOME_PAPER_SELECTOR (object);

	selector->preview = NULL;

	if (selector->config) {
		GObject *node;

		node = G_OBJECT (gnome_print_config_get_node (selector->config));

		if (selector->handler_preview) {
			g_signal_handler_disconnect (node, selector->handler_preview);
			selector->handler_preview = 0;
		}

		if (selector->handler_unit) {
			g_signal_handler_disconnect (node, selector->handler_unit);
			selector->handler_unit = 0;
		}

		selector->config = gnome_print_config_unref (selector->config);
	}

	G_OBJECT_CLASS (item_parent_class)->finalize (object);
}

static gboolean
lmargin_top_unit_activated (GtkSpinButton *spin_button,
		GdkEventFocus *event,
		GnomePaperSelector *ps)
{
	gnome_paper_preview_item_set_lm_highlights
		(GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 TRUE, FALSE, FALSE, FALSE);
	return FALSE;
}

static gboolean
lmargin_bottom_unit_activated (GtkSpinButton *spin_button,
		GdkEventFocus *event,
		GnomePaperSelector *ps)
{
	gnome_paper_preview_item_set_lm_highlights
		(GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 FALSE, TRUE, FALSE, FALSE);
	return FALSE;
}

static gboolean
lmargin_left_unit_activated (GtkSpinButton *spin_button,
		GdkEventFocus *event,
		GnomePaperSelector *ps)
{
	gnome_paper_preview_item_set_lm_highlights
		(GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 FALSE, FALSE, TRUE, FALSE);
	return FALSE;
}

static gboolean
lmargin_right_unit_activated (GtkSpinButton *spin_button,
		GdkEventFocus *event,
		GnomePaperSelector *ps)
{
	gnome_paper_preview_item_set_lm_highlights
		(GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 FALSE, FALSE, FALSE, TRUE);
	return FALSE;
}

static gboolean
lmargin_unit_deactivated (GtkSpinButton *spin_button,
		  GdkEventFocus *event,
		  GnomePaperSelector *ps)
{
	gnome_paper_preview_item_set_lm_highlights
		(GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 FALSE, FALSE, FALSE, FALSE);
	return FALSE;
}

typedef struct {
	guchar *abbr;
	gint digits;
	gfloat step_increment;
} GnomePrintPaperSelectorSpinProps_t ;

static const GnomePrintPaperSelectorSpinProps_t gpps_spin_props[] = {
	{N_("Pt"), 1, 1.0},
	{N_("mm"), 1, 1.0},
	{N_("cm"), 2, 0.5},
	{N_("m"),  3, 0.01},
	{N_("in"), 2, 0.25},
	{NULL,     2, 1.0}          /* Default must be last */
};

static void
gnome_paper_selector_spin_adapt_to_unit (GtkSpinButton *spin, const GnomePrintUnit *unit)
{
	gint num_of_units = sizeof (gpps_spin_props) / sizeof (GnomePrintPaperSelectorSpinProps_t);
	gint i;
	GtkAdjustment *adjustment;

	g_return_if_fail (GTK_IS_SPIN_BUTTON (spin));
	adjustment = gtk_spin_button_get_adjustment (spin);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

	for (i = 0; i < num_of_units; i++) {
		if (gpps_spin_props[i].abbr == NULL ||
		    !strcmp (unit->abbr, gpps_spin_props[i].abbr)) {
			adjustment->step_increment = gpps_spin_props[i].step_increment;
			adjustment->page_increment = gpps_spin_props[i].step_increment * 10;
			gtk_adjustment_changed (adjustment);
			gtk_spin_button_set_digits (spin, gpps_spin_props[i].digits);
			return;
		}
	}
}

static void
gnome_paper_selector_unit_changed_cb (GnomePrintUnitSelector *sel, GnomePaperSelector *ps)
{
	const GnomePrintUnit *unit;

	g_return_if_fail (ps != NULL);

	unit = gnome_print_unit_selector_get_unit (sel);
	if (unit) {
		gnome_print_config_set (ps->config, GNOME_PRINT_KEY_PREFERED_UNIT, unit->abbr);
		gnome_paper_selector_spin_adapt_to_unit (GTK_SPIN_BUTTON (ps->pw), unit);
		gnome_paper_selector_spin_adapt_to_unit (GTK_SPIN_BUTTON (ps->ph), unit);
		gnome_paper_selector_spin_adapt_to_unit (ps->margin_top, unit);
		gnome_paper_selector_spin_adapt_to_unit (ps->margin_bottom, unit);
		gnome_paper_selector_spin_adapt_to_unit (ps->margin_left, unit);
		gnome_paper_selector_spin_adapt_to_unit (ps->margin_right, unit);
	}
}

static void
gnome_paper_unit_selector_request_update_cb (GPANode *node, guint flags,  GnomePaperSelector *ps)
{
	guchar *unit_txt;
	unit_txt = gnome_print_config_get (ps->config, GNOME_PRINT_KEY_PREFERED_UNIT);
	if (unit_txt) {
		gnome_print_unit_selector_set_unit (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						    gnome_print_unit_get_by_abbreviation
						    (unit_txt));
		g_free (unit_txt);
	}
}

static void
gnome_paper_selector_construct (GnomePaperSelector *ps)
{
	GtkWidget *vb, *f, *t, *l;
	GtkWidget *margin_table, *margin_label;
	gdouble ml, mr, mt, mb;
	GtkObject *wa, *ha;
	AtkObject *atko;
	gdouble config_height, config_width, config_max;
	guchar *id;

	g_return_if_fail (ps != NULL);
	g_return_if_fail (ps->config != NULL);

	gtk_box_set_spacing (GTK_BOX (ps), PAD);

	/* VBox for controls */
	vb = gtk_vbox_new (FALSE, PAD);
	gtk_widget_show (vb);
	gtk_box_pack_start (GTK_BOX (ps), vb, FALSE, FALSE, 0);

	/* Create frame for selection menus */
	f = gtk_frame_new (_("Paper and layout"));
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

	/* Create table for packing menus */
	t = gtk_table_new (4, 6, FALSE);
	gtk_widget_show (t);
	gtk_container_set_border_width (GTK_CONTAINER (t), PAD);
	gtk_table_set_row_spacings (GTK_TABLE (t), 2);
	gtk_table_set_col_spacings (GTK_TABLE (t), 4);
	gtk_container_add (GTK_CONTAINER (f), t);

	l = gtk_label_new_with_mnemonic (_("Paper _size:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 0, 1);

	ps->pmenu = gtk_option_menu_new ();
	gtk_widget_show (ps->pmenu);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->pmenu, 1, 4, 0, 1);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->pmenu);
	gps_set_labelled_by_relation (ps->pmenu, l);

	/* Create spinbuttons for paper size */
	l = gtk_label_new_with_mnemonic (_("_Width:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 1, 2, 1, 2);

	config_width = 1;
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAPER_WIDTH,
				       &config_width, NULL);
	ps->w = config_width;
	wa = gtk_adjustment_new (config_width, 0.0001, 10000, 1, 10, 10);
	ps->pw = gtk_spin_button_new (GTK_ADJUSTMENT (wa), 1, 2);
	gtk_widget_show (ps->pw);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->pw, 2, 3, 1, 2);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->pw);
	gps_set_labelled_by_relation (ps->pw, l);

	l = gtk_label_new_with_mnemonic (_("_Height:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 1, 2, 2, 3);

	config_height = 1;
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAPER_HEIGHT,
				       &config_height, NULL);
	ps->h = config_height;
	ha = gtk_adjustment_new (config_height, 0.0001, 10000, 1, 10, 10);
	ps->ph = gtk_spin_button_new (GTK_ADJUSTMENT (ha), 1, 2);
	gtk_widget_show (ps->ph);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->ph, 2, 3, 2, 3);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->ph);
	gps_set_labelled_by_relation (ps->ph, l);

	ps->us = gnome_print_unit_selector_new (GNOME_PRINT_UNIT_ABSOLUTE);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->us, 3, 4, 1, 2);
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us), GTK_ADJUSTMENT (wa));
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us), GTK_ADJUSTMENT (ha));
	gtk_signal_connect (GTK_OBJECT (wa), "value_changed", GTK_SIGNAL_FUNC (gps_psize_value_changed), ps);
	gtk_signal_connect (GTK_OBJECT (ha), "value_changed", GTK_SIGNAL_FUNC (gps_psize_value_changed), ps);
	atko = gtk_widget_get_accessible (ps->us);
	atk_object_set_name (atko, _("Metric selector"));
	atk_object_set_description (atko, _("Specifies the metric to use when setting the width and height of the paper"));

	/* Feed orientation */
	l = gtk_label_new_with_mnemonic (_("_Feed orientation:"));
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 3, 4);

	ps->pomenu = gtk_option_menu_new ();
	gtk_table_attach_defaults (GTK_TABLE (t), ps->pomenu, 1, 4, 3, 4);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->pomenu);
	gps_set_labelled_by_relation (ps->pomenu, l);

	if (ps->flags & GNOME_PAPER_SELECTOR_FEED_ORIENTATION) {
		gtk_widget_show_all (ps->pomenu);
		gtk_widget_show_all (l);
	}

	/* Page orientation */
	l = gtk_label_new_with_mnemonic (_("Page _orientation:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 4, 5);

	ps->lomenu = gtk_option_menu_new ();
	gtk_widget_show (ps->lomenu);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->lomenu, 1, 4, 4, 5);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->lomenu);
	gps_set_labelled_by_relation (ps->lomenu, l);

	/* Layout */
	l = gtk_label_new_with_mnemonic (_("_Layout:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 5, 6);

	ps->lymenu = gtk_option_menu_new ();
	gtk_widget_show (ps->lymenu);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->lymenu, 1, 4, 5, 6);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->lymenu);
	gps_set_labelled_by_relation (ps->lymenu, l);

	/* Preview frame */
	ps->pf = gtk_frame_new (_("Preview"));
	gtk_widget_show (ps->pf);
	gtk_box_pack_start (GTK_BOX (ps), ps->pf, TRUE, TRUE, 0);

	ps->preview = gnome_paper_preview_new (ps->config);
	gtk_widget_set_usize (ps->preview, 160, 160);
	gtk_widget_show (ps->preview);
	gtk_container_add (GTK_CONTAINER (ps->pf), ps->preview);

	atko = gtk_widget_get_accessible (ps->preview);
	atk_object_set_name (atko, _("Preview"));
	atk_object_set_description (atko, _("Preview of the page size, orientation and layout"));

	/* Margins */
	ml = MM(10);
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, &ml, NULL);
	ps->ml = ml;

	mr = MM(10);
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, &mr, NULL);
	ps->mr = mr;

	mt = MM(10);
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_TOP, &mt, NULL);
	ps->mt = mt;

	mb = MM(10);
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, &mb, NULL);
	ps->mb = mb;

	ps->margin_frame = gtk_frame_new (_("Margins"));
	gtk_box_pack_start (GTK_BOX (ps), ps->margin_frame, FALSE, FALSE, 0);
	margin_table = gtk_table_new ( 8, 1, TRUE);
	gtk_container_set_border_width  (GTK_CONTAINER (margin_table), 4);

	config_max = MAX (config_height, config_width);
	ps->margin_top = GTK_SPIN_BUTTON (gtk_spin_button_new
					  (GTK_ADJUSTMENT (gtk_adjustment_new
							   (mt, 0, config_max,
							    1, 10, 10)),
					   1, 2));
	gtk_table_attach (GTK_TABLE (margin_table), GTK_WIDGET (ps->margin_top),
			  0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	ps->margin_bottom = GTK_SPIN_BUTTON (gtk_spin_button_new
					     (GTK_ADJUSTMENT (gtk_adjustment_new
							      (mb, 0, config_max,
							       1, 10, 10)),
					      1, 2));
	gtk_table_attach (GTK_TABLE (margin_table), GTK_WIDGET (ps->margin_bottom),
			   0, 1, 7, 8,  GTK_FILL, GTK_FILL, 0, 0);
	ps->margin_left = GTK_SPIN_BUTTON (gtk_spin_button_new
					   (GTK_ADJUSTMENT (gtk_adjustment_new
							    (ml, 0, config_max,
							     1, 10, 10)),
					    1, 2));
	gtk_table_attach (GTK_TABLE (margin_table), GTK_WIDGET (ps->margin_left),
			  0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	ps->margin_right = GTK_SPIN_BUTTON (gtk_spin_button_new
					    (GTK_ADJUSTMENT (gtk_adjustment_new
							     (mr, 0, config_max,
							      1, 10, 10)),
					     1, 2));
	gtk_table_attach (GTK_TABLE (margin_table), GTK_WIDGET (ps->margin_right),
			  0, 1, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
	margin_label = gtk_label_new (_("Top"));
	gtk_table_attach (GTK_TABLE (margin_table), margin_label,
			  0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	margin_label = gtk_label_new (_("Bottom"));
	gtk_table_attach (GTK_TABLE (margin_table), margin_label,
			  0, 1, 6, 7, GTK_FILL, GTK_FILL, 0, 0);
	margin_label = gtk_label_new (_("Left"));
	gtk_table_attach (GTK_TABLE (margin_table), margin_label,
			  0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	margin_label = gtk_label_new (_("Right"));
	gtk_table_attach (GTK_TABLE (margin_table), margin_label,
			  0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);

	gtk_container_add (GTK_CONTAINER (ps->margin_frame), margin_table);
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						  gtk_spin_button_get_adjustment (ps->margin_top));
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						  gtk_spin_button_get_adjustment (ps->margin_bottom));
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						  gtk_spin_button_get_adjustment (ps->margin_left));
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						  gtk_spin_button_get_adjustment (ps->margin_right));
	g_signal_connect (G_OBJECT (gtk_spin_button_get_adjustment (ps->margin_top)),
			    "value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (gtk_spin_button_get_adjustment (ps->margin_bottom)),
			    "value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (gtk_spin_button_get_adjustment (ps->margin_left)),
			    "value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (gtk_spin_button_get_adjustment (ps->margin_right)),
			    "value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (ps->margin_top), "focus_in_event",
		G_CALLBACK (lmargin_top_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->margin_top), "focus_out_event",
		G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->margin_left), "focus_in_event",
		G_CALLBACK (lmargin_left_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->margin_left), "focus_out_event",
		G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->margin_right), "focus_in_event",
		G_CALLBACK (lmargin_right_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->margin_right), "focus_out_event",
		G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->margin_bottom), "focus_in_event",
		G_CALLBACK (lmargin_bottom_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->margin_bottom), "focus_out_event",
		G_CALLBACK (lmargin_unit_deactivated), ps);

	if (ps->flags & GNOME_PAPER_SELECTOR_MARGINS)
		gtk_widget_show_all (ps->margin_frame);

	/* Fill optionmenus from config */
	gps_menu_create (ps->pmenu, ps->config, GNOME_PRINT_KEY_PAPER_SIZE, _("No papers defined"),
			 GTK_SIGNAL_FUNC (gps_paper_activate), ps);
	id = gnome_print_config_get (ps->config, GNOME_PRINT_KEY_PAPER_SIZE);
	if (!id || !strcmp (id, "Custom")) {
		gtk_widget_set_sensitive (ps->pw, TRUE);
		gtk_widget_set_sensitive (ps->ph, TRUE);
	} else {
		gtk_widget_set_sensitive (ps->pw, FALSE);
		gtk_widget_set_sensitive (ps->ph, FALSE);
	}
	g_free (id);
	gps_menu_create (ps->pomenu, ps->config, GNOME_PRINT_KEY_PAPER_ORIENTATION, _("No orientations defined"),
			 GTK_SIGNAL_FUNC (gps_feed_orientation_activate), ps);
	gps_menu_create (ps->lomenu, ps->config, GNOME_PRINT_KEY_PAGE_ORIENTATION, _("No orientations defined"),
			 GTK_SIGNAL_FUNC (gps_page_orientation_activate), ps);
	gps_menu_create (ps->lymenu, ps->config, GNOME_PRINT_KEY_LAYOUT, _("No layouts defined"),
			 GTK_SIGNAL_FUNC (gps_layout_activate), ps);

	ps->handler_preview = g_signal_connect (
		G_OBJECT (gnome_print_config_get_node (ps->config)), "modified",
		G_CALLBACK (cb_gnome_paper_preview_item_request_update),
		GNOME_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item));

	/* We are waiting until now to set the unit to make sure also signal handlers */
        /* and adjustments  are in place. */
	gnome_paper_unit_selector_request_update_cb (NULL, 0, ps);

	g_signal_connect (G_OBJECT (ps->us), "modified",
			  G_CALLBACK (gnome_paper_selector_unit_changed_cb), ps);

	ps->handler_unit = g_signal_connect (
		G_OBJECT (gnome_print_config_get_node (ps->config)), "modified",
		G_CALLBACK (gnome_paper_unit_selector_request_update_cb), ps);


	gtk_widget_show (ps->us);

}

GtkWidget *
gnome_paper_selector_new_with_flags (GnomePrintConfig *config, gint flags)
{
	GnomePaperSelector *selector;

	selector = gtk_type_new (GNOME_TYPE_PAPER_SELECTOR);

	if (config) {
		selector->config = gnome_print_config_ref (config);
	} else {
		selector->config = gnome_print_config_default ();
	}
	selector->flags = flags;

	gnome_paper_selector_construct (selector);


	return (GtkWidget *) selector;
}

GtkWidget *
gnome_paper_selector_new (GnomePrintConfig *config)
{
	return gnome_paper_selector_new_with_flags (config, 0);
}

/*
 * GnomePrintUnitSelector widget
 */

struct _GnomePrintUnitSelector {
	GtkHBox box;

	GtkWidget *menu;

	guint bases;
	GList *units;
	const GnomePrintUnit *unit;
	gdouble ctmscale, devicescale;
	guint plural : 1;
	guint abbr : 1;

	GList *adjustments;
};

struct _GnomePrintUnitSelectorClass {
	GtkHBoxClass parent_class;

	void (* modified) (GnomePrintUnitSelector *selector);
};

static void gnome_print_unit_selector_class_init (GnomePrintUnitSelectorClass *klass);
static void gnome_print_unit_selector_init (GnomePrintUnitSelector *selector);
static void gnome_print_unit_selector_finalize (GObject *object);

static GtkHBoxClass *unit_selector_parent_class;

/* Signals */
enum {
	GNOME_PRINT_UNIT_SELECTOR_MODIFIED,
	GNOME_PRINT_UNIT_SELECTOR_LAST_SIGNAL
};
static guint gnome_print_unit_selector_signals [GNOME_PRINT_UNIT_SELECTOR_LAST_SIGNAL] = { 0 };

GType
gnome_print_unit_selector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintUnitSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_unit_selector_class_init,
			NULL, NULL,
			sizeof (GnomePrintUnitSelector),
			0,
			(GInstanceInitFunc) gnome_print_unit_selector_init
		};
		type = g_type_register_static (GTK_TYPE_HBOX, "GnomePrintUnitSelector", &info, 0);
	}
	return type;
}

static void
gnome_print_unit_selector_class_init (GnomePrintUnitSelectorClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	unit_selector_parent_class = g_type_class_peek_parent (klass);

	gnome_print_unit_selector_signals[GNOME_PRINT_UNIT_SELECTOR_MODIFIED] =
		g_signal_new ("modified",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (GnomePrintUnitSelectorClass,
					       modified),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	object_class->finalize = gnome_print_unit_selector_finalize;
}

static void
cb_gpus_opt_menu_changed (GtkOptionMenu *menu, GnomePrintUnitSelector *us)
{
	g_signal_emit (G_OBJECT (us),
		       gnome_print_unit_selector_signals[GNOME_PRINT_UNIT_SELECTOR_MODIFIED],
		       0);
}

static void
gnome_print_unit_selector_init (GnomePrintUnitSelector *us)
{
	us->ctmscale = 1.0;
	us->devicescale = 1.0;
	us->abbr = FALSE;
	us->plural = TRUE;

	us->menu = gtk_option_menu_new ();
	g_signal_connect (G_OBJECT (us->menu),
			    "changed", G_CALLBACK (cb_gpus_opt_menu_changed), us);
	gtk_widget_show (us->menu);
	gtk_box_pack_start (GTK_BOX (us), us->menu, TRUE, TRUE, 0);
}

static void
gnome_print_unit_selector_finalize (GObject *object)
{
	GnomePrintUnitSelector *selector;

	selector = GNOME_PRINT_UNIT_SELECTOR (object);

	if (selector->menu) {
		selector->menu = NULL;
	}

	while (selector->adjustments) {
		g_object_unref (G_OBJECT (selector->adjustments->data));
		selector->adjustments = g_list_remove (selector->adjustments, selector->adjustments->data);
	}

	if (selector->units) {
		gnome_print_unit_free_list (selector->units);
	}

	selector->unit = NULL;

	G_OBJECT_CLASS (item_parent_class)->finalize (object);
}

GtkWidget *
gnome_print_unit_selector_new (guint bases)
{
	GnomePrintUnitSelector *us;

	us = gtk_type_new (GNOME_TYPE_PRINT_UNIT_SELECTOR);

	gnome_print_unit_selector_set_bases (us, bases);

	return (GtkWidget *) us;
}

static void
gnome_print_unit_selector_recalculate_adjustments (GnomePrintUnitSelector *us,
						   const GnomePrintUnit *unit)
{
	GList *l;
	const GnomePrintUnit *old;

	old = us->unit;
	us->unit = unit;
	for (l = us->adjustments; l != NULL; l = l->next) {
		GtkAdjustment *adj;
		adj = GTK_ADJUSTMENT (l->data);
#ifdef GPP_VERBOSE
		g_print ("Old val %g ... ", val);
#endif
		gnome_print_convert_distance_full (&adj->value, old, unit,
						   us->ctmscale, us->devicescale);
		gnome_print_convert_distance_full (&adj->lower, old, unit,
						   us->ctmscale, us->devicescale);
		gnome_print_convert_distance_full (&adj->upper, old, unit,
						   us->ctmscale, us->devicescale);
#ifdef GPP_VERBOSE
		g_print ("new val %g\n", &adj->value);
#endif
		gtk_adjustment_changed (adj);
		gtk_adjustment_value_changed (adj);

	}

}

const GnomePrintUnit *
gnome_print_unit_selector_get_unit (GnomePrintUnitSelector *us)
{
	g_return_val_if_fail (us != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us), NULL);

	return us->unit;
}

static void
gpus_unit_activate (GtkWidget *widget, GnomePrintUnitSelector *us)
{
	const GnomePrintUnit *unit;

	unit = g_object_get_data (G_OBJECT (widget), "unit");
	g_return_if_fail (unit != NULL);

#ifdef GPP_VERBOSE
	g_print ("Old unit %s new unit %s\n", us->unit->name, unit->name);
#endif

	gnome_print_unit_selector_recalculate_adjustments (us, unit);
}

static void
gpus_rebuild_menu (GnomePrintUnitSelector *us)
{
	GtkWidget *m, *i;
	GList *l;
	gint pos, p;

	if (GTK_OPTION_MENU (us->menu)->menu) {
		gtk_option_menu_remove_menu (GTK_OPTION_MENU (us->menu));
	}

	m = gtk_menu_new ();
	gtk_widget_show (m);

	pos = p = 0;
	for (l = us->units; l != NULL; l = l->next) {
		const GnomePrintUnit *u;
		u = l->data;
		i = gtk_menu_item_new_with_label ((us->abbr) ? (us->plural) ? u->abbr_plural : u->abbr : (us->plural) ? u->plural : u->name);
		g_object_set_data (G_OBJECT (i), "unit", (gpointer) u);
		gtk_signal_connect (GTK_OBJECT (i), "activate", GTK_SIGNAL_FUNC (gpus_unit_activate), us);
		gtk_widget_show (i);
		gtk_menu_shell_append (GTK_MENU_SHELL (m), i);
		if (u == us->unit) pos = p;
		p += 1;
	}

	gtk_option_menu_set_menu (GTK_OPTION_MENU (us->menu), m);

	gtk_option_menu_set_history (GTK_OPTION_MENU (us->menu), pos);
}

void
gnome_print_unit_selector_set_bases (GnomePrintUnitSelector *us, guint bases)
{
	GList *units;

	g_return_if_fail (us != NULL);
	g_return_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us));

	if (bases == us->bases) return;

	units = gnome_print_unit_get_list (bases);
	g_return_if_fail (units != NULL);
	gnome_print_unit_free_list (us->units);
	us->units = units;
	us->unit = units->data;

	gpus_rebuild_menu (us);
}

void
gnome_print_unit_selector_set_unit (GnomePrintUnitSelector *us, const GnomePrintUnit *unit)
{
	gint pos;

	g_return_if_fail (us != NULL);
	g_return_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us));
	g_return_if_fail (unit != NULL);

	if (unit == us->unit) return;

	pos = g_list_index (us->units, unit);
	g_return_if_fail (pos >= 0);

	gnome_print_unit_selector_recalculate_adjustments (us,  unit);
	gtk_option_menu_set_history (GTK_OPTION_MENU (us->menu), pos);
}

void
gnome_print_unit_selector_add_adjustment (GnomePrintUnitSelector *us, GtkAdjustment *adj)
{
	g_return_if_fail (us != NULL);
	g_return_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us));
	g_return_if_fail (adj != NULL);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adj));

	g_return_if_fail (!g_list_find (us->adjustments, adj));

	g_object_ref (G_OBJECT (adj));
	us->adjustments = g_list_prepend (us->adjustments, adj);
}

void
gnome_print_unit_selector_remove_adjustment (GnomePrintUnitSelector *us, GtkAdjustment *adj)
{
	g_return_if_fail (us != NULL);
	g_return_if_fail (GNOME_IS_PRINT_UNIT_SELECTOR (us));
	g_return_if_fail (adj != NULL);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adj));

	g_return_if_fail (g_list_find (us->adjustments, adj));

	us->adjustments = g_list_remove (us->adjustments, adj);
	g_object_unref (G_OBJECT (adj));
}
