/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-preview.c: print preview driver
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
 *    Miguel de Icaza <miguel@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 1999-2002 Ximian Inc. and authors
 *
 */

#include <config.h>

#include <string.h>
#include <math.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libgnomecanvas/gnome-canvas-clipgroup.h>
#include <libgnomeprint/private/gp-gc-private.h>
#include <libgnomeprint/private/gnome-glyphlist-private.h>

#include "gnome-canvas-hacktext.h"
#include "gnome-print-preview-private.h"

struct _GnomePrintPreviewPrivate {
	GPGC * gc;

	/* Current page displayed */
	int top_page;
	int current_page;

	/* The root group, with a translation setup */
	GnomeCanvasItem *root;

	/* The current page */
	GnomeCanvasItem *page;

	/* Strict theme compliance [#96802] */
	gboolean theme_compliance; 
};

static GnomePrintContextClass *parent_class;
static gboolean use_theme;

static void
outline_set_style_cb (GtkWidget *canvas, GnomeCanvasItem *item)
{
	gint32 color;
	GtkStyle *style;

	style = gtk_widget_get_style (GTK_WIDGET (canvas));
	color = GPP_COLOR_RGBA (style->text [GTK_STATE_NORMAL], 0xff);
	
	gnome_canvas_item_set (item, "outline_color_rgba", color, NULL);
}
	

static int
gpp_stroke (GnomePrintContext *pc, const ArtBpath *bpath)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);
	GnomePrintPreviewPrivate *priv = pp->priv;
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
	GnomeCanvasPathDef *path;

	/* fixme: currentpath invariants */

	group = (GnomeCanvasGroup *) gp_gc_get_data (priv->gc);
	g_assert (group != NULL);
	g_assert (GNOME_IS_CANVAS_GROUP (group));

	path = gnome_canvas_path_def_new_from_foreign_bpath ((ArtBpath *) bpath);

	/* Fixme: Can we assume that linewidth == width_units? */
	/* Probably yes, as object->page ctm == identity */

	item = gnome_canvas_item_new (group,
		gnome_canvas_bpath_get_type (),
		"bpath",	path,
		"width_units",	gp_gc_get_linewidth (pc->gc),
		"cap_style",	gp_gc_get_linecap (pc->gc) + 1 /* See #104932 */,
		"join_style",	gp_gc_get_linejoin (pc->gc),
		"outline_color_rgba", gp_gc_get_rgba (pc->gc),
		"miterlimit",	gp_gc_get_miterlimit (pc->gc),
		"dash",		gp_gc_get_dash (pc->gc),
		NULL);

	gnome_canvas_path_def_unref (path);

	if (use_theme)
		outline_set_style_cb (GTK_WIDGET (item->canvas), item);	
	return 1;
}

static void
fill_set_style_cb (GtkWidget *canvas, GnomeCanvasItem *item)
{
	gint32 color;
	GtkStyle *style;

	style = gtk_widget_get_style (GTK_WIDGET (canvas));
	color = GPP_COLOR_RGBA (style->bg [GTK_STATE_NORMAL], 0xff);
	
	gnome_canvas_item_set (item, "fill_color_rgba", color, NULL);
}
	
static int
gpp_fill (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintPreview *pp;
	GnomePrintPreviewPrivate * priv;
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
	GnomeCanvasPathDef *path;
	
	pp = GNOME_PRINT_PREVIEW (pc);
	priv = pp->priv;

	group = (GnomeCanvasGroup *) gp_gc_get_data (priv->gc);
	g_assert (group != NULL);
	g_assert (GNOME_IS_CANVAS_GROUP (group));

	path = gnome_canvas_path_def_new_from_foreign_bpath ((ArtBpath *) bpath);

	item = gnome_canvas_item_new (group,
		gnome_canvas_bpath_get_type (),
		"bpath", path,
		"outline_color", NULL,
		"fill_color_rgba", gp_gc_get_rgba (pc->gc),
		"wind", rule,
		NULL);
	gnome_canvas_path_def_unref (path);

	if (use_theme)
		fill_set_style_cb (GTK_WIDGET (item->canvas), item);
	return 1;
}

static int
gpp_clip (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintPreview *pp;
	GnomePrintPreviewPrivate * priv;
	GnomeCanvasGroup * group;
	GnomeCanvasItem * clip;
	GnomeCanvasPathDef *path;

	pp = GNOME_PRINT_PREVIEW (pc);
	priv = pp->priv;

	group = (GnomeCanvasGroup *) gp_gc_get_data (priv->gc);

	path = gnome_canvas_path_def_new_from_foreign_bpath ((ArtBpath *) bpath);

	clip = gnome_canvas_item_new (group,
		gnome_canvas_clipgroup_get_type (),
		"path", path,
		"wind", rule,
		NULL);

	gp_gc_set_data (priv->gc, clip);

	gnome_canvas_path_def_unref (path);

	return 1;
}

static void
gpp_image_free_pix (guchar *pixels, gpointer data)
{
	g_free (pixels);

}

static int
gpp_image (GnomePrintContext *pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);
	GnomeCanvasGroup * group;
	GnomeCanvasItem *item;
	GdkPixbuf *pixbuf;
	int size, bpp;
	void *dup;
	
	/*
	 * We do convert gray scale images to RGB
	 */

	if (ch == 1) {
		bpp = 3;
	} else {
		bpp = ch;
	}
	
	size = w * h * bpp;
	dup = g_malloc (size);
	if (!dup) return -1;

	if (ch == 3) {
		memcpy (dup, px, size);
		pixbuf = gdk_pixbuf_new_from_data (dup, GDK_COLORSPACE_RGB,
						   FALSE, 8, w, h, rowstride,
						   gpp_image_free_pix, NULL);
	} else if (ch == 4) {
		memcpy (dup, px, size);
		pixbuf = gdk_pixbuf_new_from_data (dup, GDK_COLORSPACE_RGB,
				                   TRUE, 8, w, h, rowstride,
						   gpp_image_free_pix, NULL);
	} else if (ch == 1) {
		const char *source;
		char *target;
		int  ix, iy;

		source = px;
		target = dup;

		for (iy = 0; iy < h; iy++){
			for (ix = 0; ix < w; ix++){
				*target++ = *source;
				*target++ = *source;
				*target++ = *source;
				source++;
			}
		}
		pixbuf = gdk_pixbuf_new_from_data (dup, GDK_COLORSPACE_RGB,
						   FALSE, 8, w, h, rowstride * 3,
						   gpp_image_free_pix, NULL);
	} else
		return -1;

	group = (GnomeCanvasGroup *) gp_gc_get_data (pp->priv->gc);

	item = gnome_canvas_item_new (group,
				      GNOME_TYPE_CANVAS_PIXBUF,
				      "pixbuf", pixbuf,
				      "x",      0.0,
				      "y",      0.0,
				      "width",  (gdouble) w,
				      "height", (gdouble) h,
				      "anchor", GTK_ANCHOR_NW,
				      NULL);
	g_object_unref (G_OBJECT (pixbuf));

	/* Apply the transformation for the image */
	{
		double transform[6];
		double flip[6];
		flip[0] = 1.0 / w;
		flip[1] = 0.0;
		flip[2] = 0.0;
		flip[3] = -1.0 / h;
		flip[4] = 0.0;
		flip[5] = 1.0;

		art_affine_multiply (transform, flip, affine);
		gnome_canvas_item_affine_absolute (item, transform);
	}
	
	return 1;
}

static int
gpp_showpage (GnomePrintContext *pc)
{
	return GNOME_PRINT_OK;
}

static int
gpp_beginpage (GnomePrintContext *pc, const guchar *name)
{
	return GNOME_PRINT_OK;
}

static void
glyphlist_set_style_cb (GtkWidget *canvas, GnomeCanvasItem *item)
{
	GnomeGlyphList *gl, *new;
	gint32 color;
	GtkStyle *style;
	gint i;

	style = gtk_widget_get_style (GTK_WIDGET (canvas));
	color = GPP_COLOR_RGBA (style->text [GTK_STATE_NORMAL], 0xff);

	g_object_get (G_OBJECT (item), "glyphlist", &gl, NULL);
	new = gnome_glyphlist_duplicate (gl);
	for (i = 0; i < new->r_length; i++) {
		if (new->rules[i].code ==  GGL_COLOR) {
			new->rules[i].value.ival = color;
		}
	}
	gnome_canvas_item_set (item, "glyphlist", new, NULL);
}

static int
gpp_glyphlist (GnomePrintContext *pc, const gdouble *affine, GnomeGlyphList * glyphlist)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
	double transform[6], a[6];

	/*
	 * The X and Y positions were computed to be the base
	 * with the translation already done for the new
	 * Postscript->Canvas translation
	 */
	memcpy (transform, gp_gc_get_ctm (pc->gc), sizeof (transform));
	art_affine_scale (a, 1.0, -1.0);
	art_affine_multiply (transform, a, affine);

	group = (GnomeCanvasGroup *) gp_gc_get_data (pp->priv->gc);
	item = gnome_canvas_item_new (group,
				      gnome_canvas_hacktext_get_type (),
				      "x", 0.0,
				      "y", 0.0,
				      "glyphlist", glyphlist,
				      NULL);

	gnome_canvas_item_affine_absolute (item, transform);

	if (use_theme)
		glyphlist_set_style_cb (GTK_WIDGET (item->canvas), item);	
	return 0;
}

static int
gpp_gsave (GnomePrintContext *ctx)
{
	GnomePrintPreview *pp;

	pp = GNOME_PRINT_PREVIEW (ctx);

	gp_gc_gsave (pp->priv->gc);

	return GNOME_PRINT_OK;
}

static int
gpp_grestore (GnomePrintContext *ctx)
{
	GnomePrintPreview *pp;

	pp = GNOME_PRINT_PREVIEW (ctx);

	gp_gc_grestore (pp->priv->gc);

	return GNOME_PRINT_OK;
}

static int
gpp_close (GnomePrintContext *pc)
{
	return 0;
}

static void
gpp_finalize (GObject *object)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (object);
	GnomePrintPreviewPrivate *priv = pp->priv;

	gp_gc_unref (priv->gc);

	if (pp->canvas)
		g_object_unref (G_OBJECT (pp->canvas));

	if (priv->page)
		gtk_object_destroy (GTK_OBJECT (priv->page));

	if (priv->root)
		gtk_object_destroy (GTK_OBJECT (priv->root));
	
	g_free (priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
class_init (GnomePrintPreviewClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	GnomePrintContextClass *pc_class = (GnomePrintContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpp_finalize;
	
	pc_class->beginpage = gpp_beginpage;
	pc_class->showpage  = gpp_showpage;
	pc_class->clip      = gpp_clip;
	pc_class->fill      = gpp_fill;
	pc_class->stroke    = gpp_stroke;
	pc_class->image     = gpp_image;
	pc_class->glyphlist = gpp_glyphlist;
	pc_class->gsave     = gpp_gsave;
	pc_class->grestore  = gpp_grestore;
	pc_class->close     = gpp_close;
}

static void
instance_init (GnomePrintPreview *preview)
{
	preview->priv = g_new0 (GnomePrintPreviewPrivate, 1);

	preview->priv->gc = gp_gc_new ();
}

static void
clear_val (GtkObject *object, void **val)
{
	*val = NULL;
}

void
gnome_print_preview_set_use_theme (gboolean theme)
{
	use_theme = theme;
}
gboolean
gnome_print_preview_get_use_theme ()
{
	return use_theme;
}

/**
 * gnome_print_preview_theme_compliance:
 * @pp: 
 * @compliance: 
 * 
 * This has to go away for GNOME 2.4. Make compliance an argument of
 * gnome_print_preview_new_full
 **/
void
gnome_print_preview_theme_compliance (GnomePrintPreview *pp,
				      gboolean compliance)
{
	pp->priv->theme_compliance = compliance;
}

GnomePrintContext *
gnome_print_preview_new_full (GnomePrintConfig *config, GnomeCanvas *canvas,
			      const gdouble *transform, const ArtDRect *region)
{
	GnomePrintPreview *preview;
	GnomePrintContext *ctx;
	GnomeCanvasGroup *group;
	gint ret;
	
	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (canvas != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CANVAS (canvas), NULL);
	g_return_val_if_fail (transform != NULL, NULL);
	g_return_val_if_fail (region != NULL, NULL);

	ctx = g_object_new (GNOME_TYPE_PRINT_PREVIEW, NULL);
	ret = gnome_print_context_construct (ctx, config);
	if (ret != GNOME_PRINT_OK) {
		g_object_unref (ctx);
		g_warning ("Could not construct the GnomePrintPreview context\n");
		return NULL;
	}
	preview = GNOME_PRINT_PREVIEW (ctx);

	g_object_ref (G_OBJECT (canvas));
	preview->canvas = canvas;

	gnome_canvas_set_scroll_region (canvas, region->x0, region->y0, region->x1, region->y1);

	preview->priv->root = gnome_canvas_item_new (gnome_canvas_root (preview->canvas),
						     GNOME_TYPE_CANVAS_GROUP,
						     "x", 0.0, "y", 0.0, NULL);

	preview->priv->page = gnome_canvas_item_new (gnome_canvas_root (preview->canvas),
						     GNOME_TYPE_CANVAS_GROUP,
						     "x", 0.0, "y", 0.0, NULL);

	g_signal_connect (G_OBJECT (preview->priv->page), "destroy",
			  (GCallback) clear_val, &preview->priv->page);
	g_signal_connect (G_OBJECT (preview->priv->root), "destroy",
			  (GCallback) clear_val, &preview->priv->root);

	/* Setup base group */
	group = GNOME_CANVAS_GROUP (preview->priv->page);
	gp_gc_set_data (preview->priv->gc, group);

	gnome_canvas_item_affine_absolute (preview->priv->page, transform);

	return ctx;
}

/**
 * gnome_print_preview_new:
 * @config:
 * @canvas: Canvas on which we display the print preview
 *
 * Creates a new PrintPreview object that use the @canvas GnomeCanvas 
 * as its rendering backend.
 *
 * Returns: A GnomePrintContext suitable for using with the GNOME print API.
 */
GnomePrintContext *
gnome_print_preview_new (GnomePrintConfig *config, GnomeCanvas *canvas)
{
	ArtDRect bbox;
	gdouble page2root[6];
	const GnomePrintUnit *unit;
	
	g_return_val_if_fail (config != NULL, NULL);
	g_return_val_if_fail (canvas != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CANVAS (canvas), NULL);

	if (getenv ("GNOME_PRINT_DEBUG_WIDE")) {
		bbox.x0 = bbox.y0 = -900.0;
		bbox.x1 = bbox.y1 = 900.0;
	} else {
		bbox.x0 = 0.0;
		bbox.y0 = 0.0;
		bbox.x1 = 21.0 * (72.0 / 2.54);
		bbox.y1 = 29.7 * (72.0 / 2.54);
		if (gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_WIDTH, &bbox.x1, &unit)) {
			gnome_print_convert_distance (&bbox.x1, unit, GNOME_PRINT_PS_UNIT);
		}
		if (gnome_print_config_get_length (config, GNOME_PRINT_KEY_PAPER_HEIGHT, &bbox.y1, &unit)) {
			gnome_print_convert_distance (&bbox.y1, unit, GNOME_PRINT_PS_UNIT);
		}
	}

	art_affine_scale (page2root, 1.0, -1.0);
	page2root[5] = bbox.y1;

	return gnome_print_preview_new_full (config, canvas, page2root, &bbox);
}

/**
 * gnome_print_preview_get_type:
 *
 * GType identification routine for #GnomePrintPreview
 *
 * Returns: The GType for the #GnomePrintPreview object
 */

GType
gnome_print_preview_get_type (void)
{
	static GType preview_type = 0;
	
	if (!preview_type) {
		static const GTypeInfo preview_info = {
			sizeof (GnomePrintPreviewClass),
			NULL, NULL,
			(GClassInitFunc) class_init,
			NULL, NULL,
			sizeof (GnomePrintPreview),
			0,
			(GInstanceInitFunc) instance_init
		};
		preview_type = g_type_register_static (GNOME_TYPE_PRINT_CONTEXT, "GnomePrintPreview", &preview_info, 0);
	}

	return preview_type;
}
