#define GP_GC_C

/*
 *
 * Copyright (C) Lauris Kaplinski, 2000
 *
 * TODO:
 *
 * - how is pen size calculated?
 * - how is dash size calcualted?
 *
 */

#include <config.h>
#include <math.h>
#include <string.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_svp_ops.h>
#include <libart_lgpl/art_vpath_dash.h>
#include <libart_lgpl/art_vpath_svp.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>

#include <gp-gc.h>

struct _GPGC {
	gint refcount;
	GSList * ctx;
};

typedef struct _GPCtx GPCtx;

struct _GPCtx {
	gdouble ctm[6];

	guint32 currentcolor;
	gdouble r, g, b, opacity;

	gdouble linewidth, miterlimit;
	ArtPathStrokeJoinType linejoin;
	ArtPathStrokeCapType linecap;
	ArtVpathDash dash;
	gboolean privatedash;

	GnomeFont * font;

	ArtPoint currentpoint;
	GPPath * currentpath;

	ArtSVP * clipsvp;
	gboolean privateclip;

	gpointer data;
};

static GPCtx * gp_ctx_new (void);
static GPCtx * gp_ctx_duplicate (const GPCtx * ctx);
static void gp_ctx_destroy (GPCtx * ctx);
static gint gp_ctx_clip (GPCtx * ctx, ArtWindRule wind);

static ArtBpath * art_bpath_from_vpath (const ArtVpath * vpath);

GPGC *
gp_gc_new (void)
{
	GPGC * gc;
	GPCtx * ctx;

	gc = g_new (GPGC, 1);
	gc->refcount = 1;
	ctx = gp_ctx_new ();
	gc->ctx = g_slist_prepend (NULL, ctx);

	return gc;
}

void
gp_gc_ref (GPGC * gc)
{
	g_return_if_fail (gc != NULL);

	gc->refcount++;
}

void
gp_gc_unref (GPGC * gc)
{
	g_return_if_fail (gc != NULL);

	if (--gc->refcount < 1) {
		GSList *tmp;
		for (tmp = gc->ctx; tmp; tmp = tmp->next)
			gp_ctx_destroy (tmp->data);
		g_slist_free (gc->ctx);
		g_free (gc);
	}
}

void
gp_gc_reset (GPGC * gc)
{
	GPCtx * ctx;
	GSList *tmp;

	g_return_if_fail (gc != NULL);

	for (tmp = gc->ctx; tmp; tmp = tmp->next)
		gp_ctx_destroy (tmp->data);
	g_slist_free (gc->ctx);

	ctx = gp_ctx_new ();

	gc->ctx = g_slist_prepend (NULL, ctx);
}

/* Stack manipulation */

gint
gp_gc_gsave (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = gp_ctx_duplicate ((GPCtx *) gc->ctx->data);
	gc->ctx = g_slist_prepend (gc->ctx, ctx);

	return 0;
}

gint
gp_gc_grestore (GPGC * gc)
{
	GSList *tmp;

	g_return_val_if_fail (gc != NULL, 1);
	g_return_val_if_fail (gc->ctx->next != NULL, 1);

	tmp = gc->ctx;
	gp_ctx_destroy (tmp->data);
	gc->ctx = tmp->next;
	g_slist_free_1 (tmp);

	return 0;
}


/* CTM */

gint
gp_gc_setmatrix (GPGC * gc, const gdouble * matrix)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);
	g_return_val_if_fail (matrix != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	memcpy (ctx->ctm, matrix, 6 * sizeof (double));

	return 0;
}

gint
gp_gc_concat (GPGC * gc, const gdouble * matrix)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);
	g_return_val_if_fail (matrix != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	art_affine_multiply (ctx->ctm, matrix, ctx->ctm);

	return 0;
}

const gdouble *
gp_gc_get_ctm (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, NULL);

	ctx = (GPCtx *) gc->ctx->data;

	return ctx->ctm;
}

/* Color */

gint
gp_gc_set_rgbcolor (GPGC * gc, gdouble r, gdouble g, gdouble b)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	ctx->currentcolor = (ctx->currentcolor & 0x000000ff) |
		((gint) (CLAMP (r, 0.0, 1.0) * 255.999) << 24) |
		((gint) (CLAMP (g, 0.0, 1.0) * 255.999) << 16) |
		((gint) (CLAMP (b, 0.0, 1.0) * 255.999) << 8);

	ctx->r = r;
	ctx->g = g;
	ctx->b = b;

	return 0;
}

gint
gp_gc_set_opacity (GPGC * gc, gdouble opacity)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	ctx->currentcolor = (ctx->currentcolor & 0xffffff00) |
		((gint) (CLAMP (opacity, 0.0, 1.0) * 255.999));

	ctx->opacity = opacity;

	return 0;
}


guint32
gp_gc_get_rgba (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 0x000000ff);

	return ((GPCtx *) gc->ctx->data)->currentcolor;
}

gdouble
gp_gc_get_red (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 0.0);

	return ((GPCtx *) gc->ctx->data)->r;
}

gdouble
gp_gc_get_green (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 0.0);

	return ((GPCtx *) gc->ctx->data)->g;
}

gdouble
gp_gc_get_blue (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 0.0);

	return ((GPCtx *) gc->ctx->data)->b;
}

gdouble
gp_gc_get_opacity (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 1.0);

	return ((GPCtx *) gc->ctx->data)->opacity;
}


/* Line attributes */

gint
gp_gc_set_linewidth (GPGC * gc, gdouble width)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	/* Fixme: Currently we calculate average of pen size in both directions */
#if 0
	ctx->linewidth = (fabs (width * ctx->ctm[0]) + fabs (width * ctx->ctm[3])) / 2;
#else
	/* If we are in landscape mode the CTM is : 0 1 -1 0 0 0,
	   so we need to take into account 0 thru 3. This is still not right.
	   FIXME.
	   see : ( run in "gs" ) http://www.gnome.org/~chema/line_width_problem
	*/
	ctx->linewidth = (fabs (width * ctx->ctm[0]) +
				   fabs (width * ctx->ctm[1]) +
				   fabs (width * ctx->ctm[2]) +
				   fabs (width * ctx->ctm[3])) / 2;
#endif

	return 0;
}

gint
gp_gc_set_miterlimit (GPGC * gc, gdouble limit)
{
	g_return_val_if_fail (gc != NULL, 1);

	((GPCtx *) gc->ctx->data)->miterlimit = limit;

	return 0;
}

gint
gp_gc_set_linejoin (GPGC * gc, ArtPathStrokeJoinType join)
{
	g_return_val_if_fail (gc != NULL, 1);

	((GPCtx *) gc->ctx->data)->linejoin = join;

	return 0;
}

gint
gp_gc_set_linecap (GPGC * gc, ArtPathStrokeCapType cap)
{
	g_return_val_if_fail (gc != NULL, 1);

	((GPCtx *) gc->ctx->data)->linecap = cap;

	return 0;
}

gint
gp_gc_set_dash (GPGC * gc, int num_values, const gdouble * values, gdouble offset)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);
	g_return_val_if_fail ((num_values == 0) || (values != NULL), 1);

	ctx = (GPCtx *) gc->ctx->data;

	if ((ctx->dash.dash) && (ctx->privatedash)) g_free (ctx->dash.dash);

	ctx->dash.n_dash = num_values;
	ctx->dash.offset = offset;

	if (values != NULL) {
		ctx->dash.dash = g_new (gdouble, num_values);
		memcpy (ctx->dash.dash, values, num_values * sizeof (gdouble));
	} else {
		ctx->dash.dash = NULL;
	}

	return 0;
}

gdouble
gp_gc_get_linewidth (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 1.0);

	return ((GPCtx *) gc->ctx->data)->linewidth;
}

gdouble
gp_gc_get_miterlimit (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 11.0);

	return ((GPCtx *) gc->ctx->data)->miterlimit;
}

ArtPathStrokeJoinType
gp_gc_get_linejoin (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, ART_PATH_STROKE_JOIN_MITER);

	return ((GPCtx *) gc->ctx->data)->linejoin;
}

ArtPathStrokeCapType
gp_gc_get_linecap (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, ART_PATH_STROKE_CAP_BUTT);

	return ((GPCtx *) gc->ctx->data)->linejoin;
}

const ArtVpathDash *
gp_gc_get_dash (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, NULL);

	return &((GPCtx *) gc->ctx->data)->dash;
}


/* Font */

gint
gp_gc_set_font (GPGC * gc, GnomeFont * font)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);
	g_return_val_if_fail (font != NULL, 1);
	g_return_val_if_fail (GNOME_IS_FONT (font), 1);

	ctx = (GPCtx *) gc->ctx->data;

	gtk_object_ref (GTK_OBJECT (font));
	gtk_object_unref (GTK_OBJECT (ctx->font));
	ctx->font = font;

	return 0;
}

const GnomeFont *
gp_gc_get_font (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, NULL);

	return ((GPCtx *) gc->ctx->data)->font;
}

/* Currentpath */

gint
gp_gc_newpath (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	gp_path_reset (ctx->currentpath);

	return 0;
}

gint
gp_gc_moveto (GPGC * gc, gdouble x, gdouble y)
{
	GPCtx * ctx;
	ArtPoint p;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	p.x = x;
	p.y = y;
	art_affine_point (&ctx->currentpoint, &p, ctx->ctm);

	gp_path_moveto (ctx->currentpath, ctx->currentpoint.x, ctx->currentpoint.y);

	return 0;
}

gint
gp_gc_lineto (GPGC * gc, gdouble x, gdouble y)
{
	GPCtx * ctx;
	ArtPoint p;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	g_return_val_if_fail (gp_path_has_currentpoint (ctx->currentpath), 1);

	p.x = x;
	p.y = y;
	art_affine_point (&ctx->currentpoint, &p, ctx->ctm);

	gp_path_lineto (ctx->currentpath, ctx->currentpoint.x, ctx->currentpoint.y);

	return 0;
}

gint
gp_gc_curveto (GPGC * gc, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3)
{
	GPCtx * ctx;
	ArtPoint p, p1, p2;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	g_return_val_if_fail (gp_path_has_currentpoint (ctx->currentpath), 1);

	p.x = x1;
	p.y = y1;
	art_affine_point (&p1, &p, ctx->ctm);
	p.x = x2;
	p.y = y2;
	art_affine_point (&p2, &p, ctx->ctm);
	p.x = x3;
	p.y = y3;
	art_affine_point (&ctx->currentpoint, &p, ctx->ctm);

	gp_path_curveto (ctx->currentpath, p1.x, p1.y, p2.x, p2.y, ctx->currentpoint.x, ctx->currentpoint.y);

	return 0;
}

gint
gp_gc_closepath (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	gp_path_closepath (ctx->currentpath);

	return 0;
}

gint
gp_gc_close_all (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 1);

	if (!gp_gc_currentpath_all_closed (gc)) {
		GPCtx * ctx;
		GPPath * closedpath;

		ctx = (GPCtx *) gc->ctx->data;

		closedpath = gp_path_close_all (ctx->currentpath);
		gp_path_unref (ctx->currentpath);
		ctx->currentpath = closedpath;
	}

	return 0;
}

gint
gp_gc_strokepath (GPGC * gc)
{
	GPCtx * ctx;
	ArtBpath * bpath;
	ArtVpath * vpath;
	ArtSVP * svp;

	g_return_val_if_fail (gc != NULL, 1);

	ctx = (GPCtx *) gc->ctx->data;

	g_return_val_if_fail (!gp_path_is_empty (ctx->currentpath), 1);
	g_return_val_if_fail (gp_path_length (ctx->currentpath) > 2, 1);

	vpath = art_bez_path_to_vec (gp_path_bpath (ctx->currentpath), 0.25);
	g_assert (vpath != NULL);

	if (ctx->dash.n_dash > 0) {
		ArtVpath * dvpath;
		dvpath = art_vpath_dash (vpath, &ctx->dash);
		g_assert (dvpath != NULL);
		art_free (vpath);
		vpath = dvpath;
	}

	svp = art_svp_vpath_stroke (vpath,
		ctx->linejoin,
		ctx->linecap,
		ctx->linewidth,
		ctx->miterlimit,
		0.25);
	g_assert (svp != NULL);

	vpath = art_vpath_from_svp (svp);
	g_assert (vpath != NULL);
	art_svp_free (svp);

	bpath = art_bpath_from_vpath (vpath);
	g_assert (bpath != NULL);
	art_free (vpath);

	gp_path_unref (ctx->currentpath);
	ctx->currentpath = gp_path_new_from_bpath (bpath);

	return 0;
}

gboolean
gp_gc_has_currentpath (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, FALSE);

	ctx = (GPCtx *) gc->ctx->data;

	return !gp_path_is_empty (ctx->currentpath);
}

gboolean
gp_gc_has_currentpoint (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, FALSE);

	ctx = (GPCtx *) gc->ctx->data;

	return gp_path_has_currentpoint (ctx->currentpath);
}

gboolean
gp_gc_currentpath_all_closed (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, FALSE);

	ctx = (GPCtx *) gc->ctx->data;

	return gp_path_all_closed (ctx->currentpath);
}

gboolean
gp_gc_currentpath_all_open (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, FALSE);

	ctx = (GPCtx *) gc->ctx->data;

	return gp_path_all_open (ctx->currentpath);
}

gint
gp_gc_currentpath_points (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, FALSE);

	ctx = (GPCtx *) gc->ctx->data;

	return gp_path_length (ctx->currentpath) - 1;
}


const ArtPoint *
gp_gc_get_currentpoint (GPGC * gc)
{
	GPCtx * ctx;

	g_return_val_if_fail (gc != NULL, NULL);

	ctx = (GPCtx *) gc->ctx->data;

	g_return_val_if_fail (gp_path_has_currentpoint (ctx->currentpath), NULL);

	return &ctx->currentpoint;
}

const GPPath *
gp_gc_get_currentpath (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, NULL);

	return ((GPCtx *) gc->ctx->data)->currentpath;
}

/* Clipping */

gint
gp_gc_clip (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 1);

	return gp_ctx_clip ((GPCtx *) gc->ctx->data, ART_WIND_RULE_NONZERO);
}

gint
gp_gc_eoclip (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, 1);

	return gp_ctx_clip ((GPCtx *) gc->ctx->data, ART_WIND_RULE_ODDEVEN);
}

gboolean
gp_gc_has_clipsvp (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, FALSE);

	return (((GPCtx *) gc->ctx->data)->clipsvp != NULL);
}

const ArtSVP *
gp_gc_get_clipsvp (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, NULL);

	return ((GPCtx *) gc->ctx->data)->clipsvp;
}

gint
gp_gc_set_data (GPGC * gc, gpointer data)
{
	g_return_val_if_fail (gc != NULL, 1);

	((GPCtx *) gc->ctx->data)->data = data;

	return 0;
}

gpointer
gp_gc_get_data (GPGC * gc)
{
	g_return_val_if_fail (gc != NULL, NULL);

	return ((GPCtx *) gc->ctx->data)->data;
}

/* Private stuff */

static GPCtx *
gp_ctx_new (void)
{
	GPCtx * ctx;

	ctx = g_new (GPCtx, 1);

	art_affine_identity (ctx->ctm);

	ctx->currentcolor = 0x000000ff;
	ctx->r = ctx->g = ctx->b = 0.0;
	ctx->opacity = 1.0;

	ctx->linewidth = 1.0;
	ctx->miterlimit = 11.0;
	ctx->linejoin = ART_PATH_STROKE_JOIN_MITER;
	ctx->linecap = ART_PATH_STROKE_CAP_BUTT;
	ctx->dash.n_dash = 0;
	ctx->dash.dash = NULL;
	ctx->privatedash = FALSE;

	ctx->font = gnome_font_new_closest ("Helvetica", GNOME_FONT_BOOK, FALSE, 12.0);

	g_return_val_if_fail (ctx->font != NULL, NULL);

	ctx->currentpath = gp_path_new ();

	ctx->clipsvp = NULL;
	ctx->privateclip = FALSE;

	ctx->data = NULL;

	return ctx;
}

static GPCtx *
gp_ctx_duplicate (const GPCtx * src)
{
	GPCtx * ctx;

	g_return_val_if_fail (src != NULL, NULL);

	ctx = g_new (GPCtx, 1);

	memcpy (ctx->ctm, src->ctm, 6 * sizeof (gdouble));

	ctx->currentcolor = src->currentcolor;
	ctx->r = src->r;
	ctx->g = src->g;
	ctx->b = src->b;
	ctx->opacity = src->opacity;

	ctx->linewidth = src->linewidth;
	ctx->miterlimit = src->miterlimit;
	ctx->linejoin = src->linejoin;
	ctx->linecap = src->linecap;
	ctx->dash.n_dash = src->dash.n_dash;
	ctx->dash.dash = src->dash.dash;
	ctx->dash.offset = src->dash.offset;
	ctx->privatedash = FALSE;

	gtk_object_ref (GTK_OBJECT (src->font));
	ctx->font = src->font;

	ctx->currentpoint = src->currentpoint;
	ctx->currentpath = gp_path_duplicate (src->currentpath);

	ctx->clipsvp = src->clipsvp;
	ctx->privateclip = FALSE;

	ctx->data = src->data;

	return ctx;
}

static void
gp_ctx_destroy (GPCtx * ctx)
{
	g_return_if_fail (ctx != NULL);

	if ((ctx->dash.dash != NULL) && (ctx->privatedash)) {
		g_free (ctx->dash.dash);
	}

	if ((ctx->clipsvp != NULL) && (ctx->privateclip)) {
		art_svp_free (ctx->clipsvp);
	}

	gtk_object_unref (GTK_OBJECT (ctx->font));

	gp_path_unref (ctx->currentpath);

	g_free (ctx);
}

static gint
gp_ctx_clip (GPCtx * ctx, ArtWindRule wind)
{
	GPPath * gppath;
	ArtVpath * vpath1, * vpath2;
	ArtSVP * svp1, * svp2;

	g_return_val_if_fail (ctx != NULL, 1);

	g_return_val_if_fail (!gp_path_is_empty (ctx->currentpath), 1);

	gppath = gp_path_close_all (ctx->currentpath);
	g_return_val_if_fail (!gp_path_is_empty (gppath), 1);

	vpath1 = art_bez_path_to_vec (gp_path_bpath (gppath), 0.25);
	g_assert (vpath1 != NULL);
	gp_path_unref (gppath);

	vpath2 = art_vpath_perturb (vpath1);
	g_assert (vpath2 != NULL);
	art_free (vpath1);

	svp1 = art_svp_from_vpath (vpath2);
	g_assert (svp1 != NULL);
	art_free (vpath2);

	svp2 = art_svp_uncross (svp1);
	g_assert (svp2 != NULL);
	art_svp_free (svp1);

	svp1 = art_svp_rewind_uncrossed (svp2, wind);
	g_assert (svp1 != NULL);
	art_svp_free (svp2);

	if (ctx->clipsvp) {
		svp2 = art_svp_intersect (ctx->clipsvp, svp1);
		g_assert (svp2 != NULL);
		art_svp_free (svp1);
		if (ctx->privateclip) art_svp_free (ctx->clipsvp);
		ctx->clipsvp = svp2;
		ctx->privateclip = TRUE;
	} else {
		ctx->clipsvp = svp1;
		ctx->privateclip = TRUE;
	}

	return 0;
}

static ArtBpath *
art_bpath_from_vpath (const ArtVpath * vpath)
{
	ArtBpath * bpath;
	gint i, len;

	g_assert (vpath != NULL);

	for (len = 0; vpath[len].code != ART_END; len++);
	len += 1;

	bpath = art_new (ArtBpath, len);
	g_assert (bpath != NULL);

	for (i = 0; i < len; i++) {
		bpath[i].code = vpath[i].code;
		bpath[i].x3 = vpath[i].x;
		bpath[i].y3 = vpath[i].y;
	}

	return bpath;
}



