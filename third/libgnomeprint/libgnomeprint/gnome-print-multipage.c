/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-multipage.c: Wrapper for printing several pages onto single output page
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
 *    Chris Lahey <clahey@helixcode.com>
 *
 *  Copyright (C) 1999-2003 Ximian Inc.
 */

#include <config.h>
#include <string.h>
#include <math.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_bpath.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gp-gc-private.h>
#include <libgnomeprint/gnome-print-multipage.h>
#include <libgnomeprint/gnome-font.h>

struct _GnomePrintMultipage {
	GnomePrintContext pc;

	GnomePrintContext *subpc;
	GList *affines; /* Of type double[6] */
	GList *subpage;
};

struct _GnomePrintMultipageClass {
	GnomePrintContextClass parent_class;
};

static void gnome_print_multipage_class_init (GnomePrintMultipageClass *klass);
static void gnome_print_multipage_init (GnomePrintMultipage *multipage);

static void gnome_print_multipage_finalize (GObject *object);

static int gnome_print_multipage_beginpage (GnomePrintContext *pc, const guchar *name);
static int gnome_print_multipage_showpage (GnomePrintContext *pc);
static int gnome_print_multipage_end_doc (GnomePrintContext *pc);
static int gnome_print_multipage_gsave (GnomePrintContext *pc);
static int gnome_print_multipage_grestore (GnomePrintContext *pc);
static int gnome_print_multipage_clip (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule);
static int gnome_print_multipage_fill (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule);
static int gnome_print_multipage_stroke (GnomePrintContext *pc, const ArtBpath *bpath);
static int gnome_print_multipage_image (GnomePrintContext *pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch);
static int gnome_print_multipage_glyphlist (GnomePrintContext *pc, const gdouble *affine, GnomeGlyphList *gl);
static int gnome_print_multipage_close (GnomePrintContext *pc);

static GList *gnome_print_multipage_affine_list_duplicate (GList *affines);

static GnomePrintContextClass *parent_class = NULL;

GType
gnome_print_multipage_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintMultipageClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_multipage_class_init,
			NULL, NULL,
			sizeof (GnomePrintMultipage),
			0,
			(GInstanceInitFunc) gnome_print_multipage_init
		};
		type = g_type_register_static (GNOME_TYPE_PRINT_CONTEXT, "GnomePrintMultipage", &info, 0);
	}
	return type;
}

static void
gnome_print_multipage_class_init (GnomePrintMultipageClass *klass)
{
	GObjectClass *object_class;
	GnomePrintContextClass *pc_class;

	object_class = (GObjectClass *) klass;
	pc_class = (GnomePrintContextClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_print_multipage_finalize;

	pc_class->beginpage = gnome_print_multipage_beginpage;
	pc_class->showpage = gnome_print_multipage_showpage;
	pc_class->end_doc = gnome_print_multipage_end_doc;

	pc_class->gsave    = gnome_print_multipage_gsave;
	pc_class->grestore = gnome_print_multipage_grestore;

	pc_class->clip   = gnome_print_multipage_clip;
	pc_class->fill   = gnome_print_multipage_fill;
	pc_class->stroke = gnome_print_multipage_stroke;

	pc_class->image = gnome_print_multipage_image;

	pc_class->glyphlist = gnome_print_multipage_glyphlist;

	pc_class->close = gnome_print_multipage_close;
}

static void
gnome_print_multipage_init (GnomePrintMultipage *multipage)
{
	multipage->affines = NULL;
	multipage->subpage = NULL;
	multipage->subpc = NULL;
}

static void
gnome_print_multipage_finalize (GObject *object)
{
	GnomePrintMultipage *mp;

	mp = GNOME_PRINT_MULTIPAGE (object);
  
	while (mp->affines) {
		g_free (mp->affines->data);
		mp->affines = g_list_remove (mp->affines, mp->affines->data);
	}

	if (mp->subpc) {
		g_object_unref (G_OBJECT (mp->subpc));
		mp->subpc = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* Easy wrappers */

static int
gnome_print_multipage_gsave (GnomePrintContext *pc)
{
	GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
	return gnome_print_gsave (multipage->subpc);
}

static int
gnome_print_multipage_grestore (GnomePrintContext *pc)
{
	GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
	return gnome_print_grestore (multipage->subpc);
}

static int
gnome_print_multipage_end_doc (GnomePrintContext *pc)
{
	GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
	return gnome_print_end_doc (multipage->subpc);
}

static int
gnome_print_multipage_clip (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintMultipage *mp;
	ArtBpath *p;
	gint ret;

	mp = GNOME_PRINT_MULTIPAGE (pc);

	p = art_bpath_affine_transform (bpath, mp->subpage->data);

	ret = gnome_print_clip_bpath_rule (mp->subpc, p, rule);

	art_free (p);

	return ret;
}

static int
gnome_print_multipage_fill (GnomePrintContext *pc, const ArtBpath *bpath, ArtWindRule rule)
{
	GnomePrintMultipage *mp;
	ArtBpath *p;
	gint ret;

	mp = GNOME_PRINT_MULTIPAGE(pc);

	p = art_bpath_affine_transform (bpath, mp->subpage->data);

	gnome_print_setrgbcolor (mp->subpc, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
	gnome_print_setopacity (mp->subpc, gp_gc_get_opacity (pc->gc));

	ret = gnome_print_fill_bpath_rule (mp->subpc, p, rule);

	art_free (p);

	return ret;
}

static int
gnome_print_multipage_stroke (GnomePrintContext *pc, const ArtBpath *bpath)
{
	GnomePrintMultipage *mp;
	const ArtVpathDash *dash;
	ArtBpath *p;
	gint ret;

	mp = GNOME_PRINT_MULTIPAGE (pc);
	dash = gp_gc_get_dash (pc->gc);
	p = art_bpath_affine_transform (bpath, mp->subpage->data);

	gnome_print_setrgbcolor   (mp->subpc, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
	gnome_print_setopacity    (mp->subpc, gp_gc_get_opacity    (pc->gc));
	gnome_print_setlinewidth  (mp->subpc, gp_gc_get_linewidth  (pc->gc));
	gnome_print_setmiterlimit (mp->subpc, gp_gc_get_miterlimit (pc->gc));
	gnome_print_setlinejoin   (mp->subpc, gp_gc_get_linejoin   (pc->gc));
	gnome_print_setlinecap    (mp->subpc, gp_gc_get_linecap    (pc->gc));
	gnome_print_setdash       (mp->subpc, dash->n_dash, dash->dash, dash->offset);

	ret = gnome_print_stroke_bpath (mp->subpc, p);

	art_free (p);

	return ret;
}

static int
gnome_print_multipage_image (GnomePrintContext *pc, const gdouble *affine, const guchar *px, gint w, gint h, gint rowstride, gint ch)
{
	GnomePrintMultipage *mp;
	gdouble a[6];

	mp = GNOME_PRINT_MULTIPAGE(pc);

	art_affine_multiply (a, affine, mp->subpage->data);

	return gnome_print_image_transform (mp->subpc, a, px, w, h, rowstride, ch);
}

static int
gnome_print_multipage_glyphlist (GnomePrintContext *pc, const gdouble *affine, GnomeGlyphList *gl)
{
	GnomePrintMultipage *mp;
	gdouble a[6];

	mp = GNOME_PRINT_MULTIPAGE(pc);

	art_affine_multiply (a, affine, mp->subpage->data);

	return gnome_print_glyphlist_transform (mp->subpc, a, gl);
}

/**
 * gnome_print_multipage_finish_page:
 * @mp: 
 *
 * Finishes a half-filled page, NOP otherwise
 * This allows you to flush all pages, without closing the
 * multipage context. Closing the multipage context calls
 * this implicitly
 * 
 * Return Value: GNOME_PRINT_OK on success, a different value otherwise
 **/
gint
gnome_print_multipage_finish_page (GnomePrintMultipage *mp)
{
	g_return_val_if_fail (mp != NULL, GNOME_PRINT_ERROR_BADCONTEXT);
	g_return_val_if_fail (GNOME_IS_PRINT_MULTIPAGE (mp), GNOME_PRINT_ERROR_BADCONTEXT);

	if (mp->subpage != mp->affines) {
		/* We have not filled whole page yet */
		mp->subpage = mp->affines;
		return gnome_print_showpage (mp->subpc);
	}

	return GNOME_PRINT_OK;
}

static int
gnome_print_multipage_close (GnomePrintContext *ctx)
{
	GnomePrintMultipage *mp;

	mp = GNOME_PRINT_MULTIPAGE (ctx);

	gnome_print_multipage_finish_page (mp);

	return gnome_print_context_close (mp->subpc);
}

static int
gnome_print_multipage_beginpage (GnomePrintContext *pc, const guchar *name)
{
	GnomePrintMultipage *mp;
	gint ret;

	mp = GNOME_PRINT_MULTIPAGE (pc);

	/* We count on ::showpage advancing subpage */

	if (mp->subpage == mp->affines) {
		/* Have to start new global page */
		ret = gnome_print_beginpage (mp->subpc, name);
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	/* Start new local page gsave and current affine matrix */
	ret = gnome_print_gsave (mp->subpc);
	g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);

	return ret;
}

static int
gnome_print_multipage_showpage (GnomePrintContext *pc)
{
	GnomePrintMultipage *mp;
	gint ret;

	mp = GNOME_PRINT_MULTIPAGE (pc);
	ret = gnome_print_grestore (mp->subpc);

	g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);

	mp->subpage = mp->subpage->next;
	if (mp->subpage == NULL) {
		/* Finished global page, start from the
		 * beginning and show it 
		 */
		mp->subpage = mp->affines;
		ret = gnome_print_showpage (mp->subpc);
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	return GNOME_PRINT_OK;
}

static GList *
gnome_print_multipage_affine_list_duplicate(GList *affines)
{
	GList *list, *l;
  
	if (affines == NULL) return NULL;

	list = NULL;

	for (l = affines; l != NULL; l = l->next) {
		gdouble *affine;
		affine = g_new (gdouble, 6);
		memcpy (affine, l->data, 6 * sizeof (gdouble));
		list = g_list_prepend (list, affine);
	}

	list = g_list_reverse (list);

	return list;
}

/**
 * gnome_print_multipage_new:
 * @subpc: Where do we print
 * @affines: List of positions for pages.  There must be
 *           at least one item in this list, childs are of
 *           type double[6]
 *
 * Creates a new Postscript printing context
 *
 * Returns: a new GnomePrintMultipage object in which you can issue GnomePrint commands.
 */
GnomePrintContext *
gnome_print_multipage_new (GnomePrintContext *subpc, GList *affines /* Of type double[6] */)
{
	GnomePrintMultipage *multipage;

	g_return_val_if_fail (subpc != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (subpc), NULL);
	g_return_val_if_fail (affines != NULL, NULL);

	multipage = g_object_new (GNOME_TYPE_PRINT_MULTIPAGE, NULL);

	multipage->subpc = subpc;
	g_object_ref (G_OBJECT(subpc));

	multipage->affines = gnome_print_multipage_affine_list_duplicate (affines);
	multipage->subpage = multipage->affines;

	return (GnomePrintContext *) multipage;
}
