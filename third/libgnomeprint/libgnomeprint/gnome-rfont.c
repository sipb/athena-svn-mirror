/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-rfont.c: grid fitted font
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
 *  Copyright (C) 2000-2001 Ximian Inc. and authors
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftbbox.h>
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libart_lgpl/art_gray_svp.h>
#include <libart_lgpl/art_rgb_svp.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-rfont.h>

#define noGRF_VERBOSE
#define noGRF_DEBUG

/* Be careful, we are using 10.6 fixed point numbers */
/* So absolute maximum linear glyph distance is 511 */
#define GRF_MAX_GLYPH_SIZE 32
#define GRF_MAX_GLYPH_AREA (32 * 32)
#define GRF_GREEK_TRESHOLD 3
#define GRF_GLYPH_BLOCK_SIZE 64
/* Be cautious - greek bitmaps has to fint into pointer */
#define GRF_GLYPH_GREEK_TRESHOLD (sizeof (guchar *))

typedef struct _GRFGlyphSlot GRFGlyphSlot;

struct _GnomeRFont {
	GObject object;
	GnomeFont *font;
	gdouble transform[6];
	GHashTable * bpaths;
	GHashTable * svps;

	/* Rendering data */
	guint vector : 1; /* Whether to render everything vectorially */
	guint greek : 1; /* Whether to render everything as boxes */
	guint flipy : 1; /* Flip y in FT blitting */
	guint flipx : 1; /* Flip x in FT blitting */
	/* FreeType values */
	FT_UInt load_glyph_flags; /* Flags to FT_Load_Glyph */
	FT_UInt pixel_width;
	FT_UInt pixel_height;
	FT_Matrix matrix;

	/* Glyph cache */
	gint *idx;
	gint glen;
	gint gsize;
	GRFGlyphSlot *glyphs;
};

struct _GnomeRFontClass {
	GObjectClass parent_class;
};

struct _GRFGlyphSlot {
	guint has_advance : 1;
	guint has_bbox : 1;
	guint has_graymap : 1;
	/* fixme: This is not greek so rename it (Lauris) */
	guint is_greek : 1;
	gint32 advancex, advancey; /* 10.6 fixed point */
	gint16 x0, y0, x1, y1; /* 10.6 fixed point */
	union {
		guchar *px;
		guchar d[GRF_GLYPH_GREEK_TRESHOLD];
	} bd;
};

#define GRF_COORD_INT_LOWER(i) ((i) >> 6)
#define GRF_COORD_INT_UPPER(i) (((i) + 63) >> 6)
#define GRF_COORD_INT_SIZE(i0,i1) (GRF_COORD_INT_UPPER (i1) - GRF_COORD_INT_LOWER (i0))
#define GRF_COORD_TO_FLOAT(i) ((gdouble) (i) / 64.0)
#define GRF_COORD_FROM_FLOAT_LOWER(f) ((gint) floor (f * 64.0))
#define GRF_COORD_FROM_FLOAT_UPPER(f) ((gint) ceil (f * 64.0))

static inline gint gnome_rfont_get_num_glyphs (GnomeRFont *rfont);

#define GRF_NUM_GLYPHS(f) (gnome_rfont_get_num_glyphs (f))
#define G__RF_NUM_GLYPHS(f) ((f)->font->face->num_glyphs)
#define GRF_FT_FACE(f) ((f)->font->face->ft_face)
#define FT_MATRIX_IS_IDENTITY(f) (((f)->matrix.xx == 0x10000) && ((f)->matrix.yx == 0) && ((f)->matrix.xy == 0) && ((f)->matrix.yy == 0x10000))

static GHashTable * rfonts = NULL;

static void gnome_rfont_class_init (GnomeRFontClass * klass);
static void gnome_rfont_init (GnomeRFont * rfont);

static void gnome_rfont_finalize (GObject * object);

static gboolean rfont_free_svp (gpointer key, gpointer value, gpointer data);
static gboolean rfont_free_bpath (gpointer key, gpointer value, gpointer data);

static guint rfont_hash (gconstpointer key);
static gboolean rfont_equal (gconstpointer key1, gconstpointer key2);

static GObjectClass * parent_class;

GType
gnome_rfont_get_type (void)
{
	static GType rfont_type = 0;
	if (!rfont_type) {
		static const GTypeInfo rfont_info = {
			sizeof (GnomeRFontClass),
			NULL, NULL,
			(GClassInitFunc) gnome_rfont_class_init,
			NULL, NULL,
			sizeof (GnomeRFont),
			0,
			(GInstanceInitFunc) gnome_rfont_init
		};
		rfont_type = g_type_register_static (G_TYPE_OBJECT, "GnomeRFont", &rfont_info, 0);
	}
	return rfont_type;
}

static void
gnome_rfont_class_init (GnomeRFontClass * klass)
{
	GObjectClass * object_class;

	object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_rfont_finalize;
}

static void
gnome_rfont_init (GnomeRFont * rfont)
{
	art_affine_identity (rfont->transform);
	rfont->bpaths = g_hash_table_new (NULL, NULL);
	rfont->svps = g_hash_table_new (NULL, NULL);

	rfont->glyphs = NULL;
}

static void
gnome_rfont_finalize (GObject * object)
{
	GnomeRFont *rfont;

	rfont = GNOME_RFONT (object);

	g_hash_table_remove (rfonts, rfont);

	if (rfont->idx) {
		g_free (rfont->idx);
	}

	if (rfont->glyphs) {
		gint i;
		for (i = 0; i < rfont->glen; i++) {
			if (!rfont->glyphs[i].is_greek && rfont->glyphs[i].bd.px)
				g_free (rfont->glyphs[i].bd.px);
		}
		g_free (rfont->glyphs);
	}

	if (rfont->svps) {
		g_hash_table_foreach_remove (rfont->svps, rfont_free_svp, NULL);
		g_hash_table_destroy (rfont->svps);
		rfont->svps = NULL;
	}
	if (rfont->bpaths) {
		g_hash_table_foreach_remove (rfont->bpaths, rfont_free_bpath, NULL);
		g_hash_table_destroy (rfont->bpaths);
		rfont->bpaths = NULL;
	}

	if (rfont->font) {
		g_object_unref (G_OBJECT (rfont->font));
		rfont->font = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

#define EPSILON 1e-9

#ifdef GRF_VERBOSE
#define PRINT_2(s,a,b) g_print ("%s %g %g\n", s, (a), (b))
#define PRINT_DRECT(s,a) g_print ("%s %g %g %g %g\n", (s), (a)->x0, (a)->y0, (a)->x1, (a)->y1)
#define PRINT_AFFINE(s,a) g_print ("%s %g %g %g %g %g %g\n", (s), *(a), *((a) + 1), *((a) + 2), *((a) + 3), *((a) + 4), *((a) + 5))
#else
#define PRINT_2(s,a,b)
#define PRINT_DRECT(s,a)
#define PRINT_AFFINE(s,a)
#endif

/**
 * gnome_font_get_rfont:
 * @font: 
 * @t: 
 * 
 * Creates a new RFont from @font and font->raster affine matrix
 * Matrix can be 2x2, although if read, all 2x3 values are
 * retrieved. RFont is referenced, so you have to unref it
 * somewhere
 * 
 * Return Value: the font created, NULL on error.
 **/
GnomeRFont *
gnome_font_get_rfont (GnomeFont * font, const gdouble *t)
{
	GnomeRFont search;
	GnomeRFont *rfont;
	gdouble dpixx, dpixy;
	gdouble dcm[6];
	gint i;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (t != NULL, NULL);

	if (rfonts == NULL) {
		rfonts = g_hash_table_new (rfont_hash, rfont_equal);
	}

	search.font = font;
	memcpy (search.transform, t, 4 * sizeof (gdouble));
	search.transform[4] = search.transform[5] = 0.0;

	rfont = g_hash_table_lookup (rfonts, &search);

	if (rfont != NULL) {
#ifdef GRF_VERBOSE
		g_print ("found cached rfont %s\n", gnome_font_get_name (font));
#endif
		gnome_rfont_ref (rfont);
		return rfont;
	}

	/* Create new object */

	rfont = g_object_new (GNOME_TYPE_RFONT, NULL);

	g_object_ref (G_OBJECT (font));
	rfont->font = font;
	memcpy (rfont->transform, t, 4 * sizeof (gdouble));
	rfont->transform[4] = rfont->transform[5] = 0.0;

	/* Create index table */
	rfont->idx = g_new (gint, GRF_NUM_GLYPHS (rfont));
	for (i = 0; i < GRF_NUM_GLYPHS (rfont); i++)
		rfont->idx[i] = -1;

	PRINT_AFFINE ("RFont matrix:", &t[0]);
	/* Calculate integer resolutions */
	dpixx = floor (sqrt (t[0] * t[0] + t[1] * t[1]) * rfont->font->size + 0.5);
	dpixy = floor (sqrt (t[2] * t[2] + t[3] * t[3]) * rfont->font->size + 0.5);
	PRINT_2 ("RFont pixel sizes:", dpixx, dpixy);
	/* Find resolution compensated matrix */
	if ((fabs (dpixx) > 1e-6) && (fabs (dpixy) > 1e-6)) {
		dcm[0] = t[0] * rfont->font->size / dpixx;
		dcm[1] = t[1] * rfont->font->size / dpixx;
		dcm[2] = t[2] * rfont->font->size / dpixy;
		dcm[3] = t[3] * rfont->font->size / dpixy;
		dcm[4] = 0.0;
		dcm[5] = 0.0;
	} else {
		art_affine_identity (dcm);
	}
	PRINT_AFFINE ("RFont compensated matrix:", &dcm[0]);
	if (dcm[3] < 0) {
		dcm[1] = -dcm[1];
		dcm[3] = -dcm[3];
		rfont->flipy = TRUE;
	} else {
		rfont->flipy = FALSE;
	}
	/* Save values in FT format */
	rfont->load_glyph_flags = 0;
	if (FT_MATRIX_IS_IDENTITY (rfont))
		rfont->load_glyph_flags |= FT_LOAD_IGNORE_TRANSFORM;
	rfont->pixel_width = (FT_UInt) dpixx;
	rfont->pixel_height = (FT_UInt) dpixy;
#ifdef GRF_VERBOSE
	g_print ("RFont pixel dimensions: %d %d\n", rfont->pixel_width, rfont->pixel_height);
#endif
	/* FIXME: Use font bbox here (Lauris) */
	rfont->vector = ((rfont->pixel_width > GRF_MAX_GLYPH_SIZE) ||
			 (rfont->pixel_height > GRF_MAX_GLYPH_SIZE) ||
			 (rfont->pixel_width * rfont->pixel_height > GRF_MAX_GLYPH_AREA));
        /* FIXME: Find the reason for having ->vector, I am setting it to TRUE to fix
	 * http://bugzilla.gnome.org/show_bug.cgi?id=81792 (Chema) */
	rfont->vector = TRUE;
	rfont->greek = ((rfont->pixel_width <= GRF_GREEK_TRESHOLD) ||
			(rfont->pixel_height <= GRF_GREEK_TRESHOLD));
	rfont->matrix.xx = (FT_Fixed) floor (dcm[0] * 65536 + 0.5);
	rfont->matrix.yx = (FT_Fixed) floor (dcm[1] * 65536 + 0.5);
	rfont->matrix.xy = (FT_Fixed) floor (dcm[2] * 65536 + 0.5);
	rfont->matrix.yy = (FT_Fixed) floor (dcm[3] * 65536 + 0.5);

	g_hash_table_insert (rfonts, rfont, rfont);

	return rfont;
}

GnomeFont *
gnome_rfont_get_font (const GnomeRFont * rfont)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);

	return rfont->font;
}

GnomeFontFace *
gnome_rfont_get_face (const GnomeRFont * rfont)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);

	return rfont->font->face;
}

gdouble *
gnome_rfont_get_matrix (const GnomeRFont * rfont, gdouble * matrix)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (matrix != NULL, NULL);

	memcpy (matrix, rfont->transform, 4 * sizeof (gdouble));

	return matrix;
}

static GRFGlyphSlot *
grf_ensure_slot (GnomeRFont *rfont, gint glyph)
{
	gint idx;

	idx = rfont->idx[glyph];

	if (idx < 0) {
		/* No glyph slot allocated */
		if (rfont->glen >= rfont->gsize) {
			/* Need to realloc slots */
			rfont->gsize += GRF_GLYPH_BLOCK_SIZE;
			if (rfont->glyphs) {
				/* Realloc */
				rfont->glyphs = g_renew (GRFGlyphSlot, rfont->glyphs, rfont->gsize);
			} else {
				/* Alloc */
				rfont->glyphs = g_new (GRFGlyphSlot, rfont->gsize);
			}
		}
		idx = rfont->glen;
		/* Initialize slot */
		rfont->glyphs[idx].has_advance = FALSE;
		rfont->glyphs[idx].has_bbox = FALSE;
		rfont->glyphs[idx].has_graymap = FALSE;
		rfont->glyphs[idx].is_greek = FALSE;
		rfont->glyphs[idx].advancex = 0;
		rfont->glyphs[idx].advancey = 0;
		rfont->glyphs[idx].x0 = 0;
		rfont->glyphs[idx].y0 = 0;
		rfont->glyphs[idx].x1 = 0;
		rfont->glyphs[idx].y1 = 0;
		rfont->glyphs[idx].bd.px = NULL;
		/* Assign index */
		rfont->idx[glyph] = idx;
		rfont->glen += 1;
	}

	return rfont->glyphs + idx;
}

static GRFGlyphSlot *
grf_ensure_slot_stdadvance (GnomeRFont *rfont, gint glyph)
{
	GRFGlyphSlot *slot;
	FT_Error status;

	slot = grf_ensure_slot (rfont, glyph);

	if (!slot->has_advance) {
#ifdef GRF_DEBUG
		ArtPoint a;
#endif
		slot->has_advance = TRUE;
#ifdef GRF_DEBUG
		gnome_font_get_glyph_stdadvance (rfont->font, glyph, &a);
		art_affine_point (&a, &a, rfont->transform);
		PRINT_2 ("RFont vector advance", a.x, a.y);
#endif
		status = FT_Set_Pixel_Sizes (GRF_FT_FACE (rfont), rfont->pixel_width, rfont->pixel_height);
		g_return_val_if_fail (status == FT_Err_Ok, slot);
		FT_Set_Transform (GRF_FT_FACE (rfont), &rfont->matrix, NULL);
		status = FT_Load_Glyph (GRF_FT_FACE (rfont), glyph, rfont->load_glyph_flags);
		g_return_val_if_fail (status == FT_Err_Ok, slot);
		/* fixme: This is correct only if we do scaled glyphs (always?) */
		slot->advancex = GRF_FT_FACE (rfont)->glyph->advance.x;
		slot->advancey = GRF_FT_FACE (rfont)->glyph->advance.y;
		PRINT_2 ("RFont FT advance", GRF_COORD_TO_FLOAT (slot->advancex), GRF_COORD_TO_FLOAT (slot->advancey));
	}

	return slot;
}

static GRFGlyphSlot *
grf_ensure_slot_bbox (GnomeRFont *rfont, gint glyph)
{
	GRFGlyphSlot *slot;
	FT_Error status;

	slot = grf_ensure_slot (rfont, glyph);

	if (!slot->has_bbox) {
		FT_OutlineGlyph ol;
		FT_BBox bbox;
#ifdef GRF_DEBUG
		ArtSVP *svp;
		ArtDRect dbox;

		/* Empty slot */
		/* FIXME: I'll write the real thing on holiday, honestly (Lauris) */
		svp = (ArtSVP *) gnome_rfont_get_glyph_svp (rfont, glyph);
		art_drect_svp (&dbox, svp);
		slot->x0 = GRF_COORD_FROM_FLOAT_LOWER (dbox.x0);
		slot->y0 = GRF_COORD_FROM_FLOAT_LOWER (dbox.y0);
		slot->x1 = GRF_COORD_FROM_FLOAT_UPPER (dbox.x1);
		slot->y1 = GRF_COORD_FROM_FLOAT_UPPER (dbox.y1);

		g_print ("RFont SVP size: %d %d %d %d\n", slot->x0, slot->y0, slot->x1, slot->y1);
#endif
		slot->has_bbox = TRUE;
		status = FT_Set_Pixel_Sizes (GRF_FT_FACE (rfont), rfont->pixel_width, rfont->pixel_height);
		g_return_val_if_fail (status == FT_Err_Ok, slot);
		FT_Set_Transform (GRF_FT_FACE (rfont), &rfont->matrix, NULL);
		status = FT_Load_Glyph (GRF_FT_FACE (rfont), glyph, FT_LOAD_DEFAULT);
		g_return_val_if_fail (status == FT_Err_Ok, slot);
		status = FT_Get_Glyph (GRF_FT_FACE (rfont)->glyph, (FT_Glyph *) &ol);
		g_return_val_if_fail (status == FT_Err_Ok, slot);
		status = FT_Outline_Get_BBox (&ol->outline, &bbox);
		g_return_val_if_fail (status == FT_Err_Ok, slot);

		/* FIXME: This is correct only if we do scaled glyphs (always?) (Lauris) */
		if (!rfont->flipy) {
			slot->x0 = bbox.xMin;
			slot->y0 = bbox.yMin;
			slot->x1 = bbox.xMax + 1;
			slot->y1 = bbox.yMax + 1;
		} else {
			slot->x0 =  bbox.xMin;
			slot->y0 = -bbox.yMax;
			slot->x1 =  bbox.xMax + 1;
			slot->y1 = -bbox.yMin + 1;
		}
		FT_Done_Glyph ((FT_Glyph) ol);
#ifdef GRF_VERBOSE
		g_print ("RFont FT size: %d %d %d %d\n", slot->x0, slot->y0, slot->x1, slot->y1);
#endif
	}

	return slot;
}

static GRFGlyphSlot *
grf_ensure_slot_graymap (GnomeRFont *rfont, gint glyph)
{
	GRFGlyphSlot *slot;
	FT_Error status;

	slot = grf_ensure_slot_bbox (rfont, glyph);

	if (!slot->has_graymap) {
		FT_BitmapGlyph bm;
		gint sw, sh, cw, ch, r;

		slot->has_graymap = TRUE;

		/* No graymap for vector fonts */
		if (rfont->vector)
			return slot;

		/* Do not render glyphs with coverage < 1/64th of pixel */
		if (((slot->x1 - slot->x0) < 8) || ((slot->y1 - slot->y0) < 8))
			return slot;

		/* We deliberately allow up to 1 pixel misplacing of greek glyhs */
		if ((GRF_COORD_INT_UPPER (slot->x1 - slot->x0) * GRF_COORD_INT_UPPER (slot->y1 - slot->y0)) <= GRF_GLYPH_GREEK_TRESHOLD) {
			/* Render greek glyph in 4x resolution */
			status = FT_Set_Pixel_Sizes (GRF_FT_FACE (rfont), rfont->pixel_width << 2, rfont->pixel_height << 2);
			g_return_val_if_fail (status == FT_Err_Ok, slot);
			FT_Set_Transform (GRF_FT_FACE (rfont), &rfont->matrix, NULL);
			status = FT_Load_Glyph (GRF_FT_FACE (rfont), glyph, FT_LOAD_DEFAULT);
			g_return_val_if_fail (status == FT_Err_Ok, slot);
			status = FT_Get_Glyph (GRF_FT_FACE (rfont)->glyph, (FT_Glyph *) &bm);
			g_return_val_if_fail (status == FT_Err_Ok, slot);
			if (((FT_OutlineGlyph) bm)->outline.n_points < 3)
				return slot;
			status = FT_Glyph_To_Bitmap ((FT_Glyph *) &bm, ft_render_mode_normal, 0, 1);
			g_return_val_if_fail (status == FT_Err_Ok, slot);

			slot->is_greek = TRUE;

			sw = GRF_COORD_INT_UPPER (slot->x1 - slot->x0);
			sh = GRF_COORD_INT_UPPER (slot->y1 - slot->y0);

			for (r = 0; r < sh; r++) {
				gint c;
				for (c = 0; c < sw; c++) {
					gint x, y;
					guint val;
					val = 0;
					for (y = (r << 2); y < ((r + 1) << 2); y++) {
						for (x = (c << 2); x < ((c + 1) << 2); x++) {
							if ((y < bm->bitmap.rows) && (x < bm->bitmap.width)) {
								val += bm->bitmap.buffer[y * bm->bitmap.pitch + x];
							}
						}
					}
					slot->bd.d[r * sw + c] = val >> 4;
				}
			}
			return slot;
		}

		sw = GRF_COORD_INT_SIZE (slot->x0, slot->x1);
		sh = GRF_COORD_INT_SIZE (slot->y0, slot->y1);

		status = FT_Set_Pixel_Sizes (GRF_FT_FACE (rfont), rfont->pixel_width, rfont->pixel_height);
		g_return_val_if_fail (status == FT_Err_Ok, slot);
		FT_Set_Transform (GRF_FT_FACE (rfont), &rfont->matrix, NULL);
		status = FT_Load_Glyph (GRF_FT_FACE (rfont), glyph, FT_LOAD_DEFAULT);
		g_return_val_if_fail (status == FT_Err_Ok, slot);
		status = FT_Get_Glyph (GRF_FT_FACE (rfont)->glyph, (FT_Glyph *) &bm);
		g_return_val_if_fail (status == FT_Err_Ok, slot);
		if (((FT_OutlineGlyph) bm)->outline.n_points < 3)
			return slot;
		status = FT_Glyph_To_Bitmap ((FT_Glyph *) &bm, ft_render_mode_normal, 0, 1);
		g_return_val_if_fail (status == FT_Err_Ok, slot);

		slot->bd.px = g_new0 (guchar, sw * sh);
		cw = MIN (bm->bitmap.width, sw);
		ch = MIN (bm->bitmap.rows, sh);

		if (!rfont->flipy) {
			for (r = 0; r < ch; r++) {
				memcpy (slot->bd.px + r * sw, bm->bitmap.buffer + (bm->bitmap.rows - r - 1) * bm->bitmap.pitch, cw);
			}
		} else {
			for (r = 0; r < ch; r++) {
				memcpy (slot->bd.px + r * sw, bm->bitmap.buffer + r * bm->bitmap.pitch, cw);
			}
		}
	}

	return slot;
}

ArtPoint *
gnome_rfont_get_glyph_stdadvance (GnomeRFont *rfont, gint glyph, ArtPoint *advance)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (glyph >= 0, NULL);
	g_return_val_if_fail (glyph < GRF_NUM_GLYPHS (rfont), NULL);
	g_return_val_if_fail (advance != NULL, NULL);

	if (rfont->vector) {
		gnome_font_get_glyph_stdadvance (rfont->font, glyph, advance);
		art_affine_point (advance, advance, rfont->transform);
	} else {
		GRFGlyphSlot *slot;
		slot = grf_ensure_slot_stdadvance (rfont, glyph);
		advance->x = GRF_COORD_TO_FLOAT (slot->advancex);
		advance->y = GRF_COORD_TO_FLOAT (slot->advancey);
	}

	return advance;
}

ArtDRect *
gnome_rfont_get_glyph_stdbbox (GnomeRFont *rfont, gint glyph, ArtDRect *bbox)
{
	GRFGlyphSlot *slot;

	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (glyph >= 0, NULL);
	g_return_val_if_fail (glyph < GRF_NUM_GLYPHS (rfont), NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	slot = grf_ensure_slot_bbox (rfont, glyph);

	bbox->x0 = GRF_COORD_TO_FLOAT (slot->x0);
	bbox->y0 = GRF_COORD_TO_FLOAT (slot->y0);
	bbox->x1 = GRF_COORD_TO_FLOAT (slot->x1);
	bbox->y1 = GRF_COORD_TO_FLOAT (slot->y1);

	return bbox;
}

ArtPoint *
gnome_rfont_get_glyph_stdkerning (GnomeRFont *rfont, gint glyph0, gint glyph1, ArtPoint *kerning)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (glyph0 >= 0, NULL);
	g_return_val_if_fail (glyph0 < GRF_NUM_GLYPHS (rfont), NULL);
	g_return_val_if_fail (glyph1 >= 0, NULL);
	g_return_val_if_fail (glyph1 < GRF_NUM_GLYPHS (rfont), NULL);
	g_return_val_if_fail (kerning != NULL, NULL);

	if (!gnome_font_get_glyph_stdkerning (rfont->font, glyph0, glyph1, kerning)) {
		g_warning ("file %s: line %d: Font stdkerning failed", __FILE__, __LINE__);
		return NULL;
	}

	art_affine_point (kerning, kerning, rfont->transform);

	return kerning;
}

const ArtBpath *
gnome_rfont_get_glyph_bpath (GnomeRFont * rfont, gint glyph)
{
	ArtBpath * bpath;
	gdouble affine[6];
	gdouble size;

	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (glyph >= 0, NULL);
	g_return_val_if_fail (glyph < GRF_NUM_GLYPHS (rfont), NULL);

	bpath = g_hash_table_lookup (rfont->bpaths, GINT_TO_POINTER (glyph));

	if (bpath)
		return bpath;

	size = gnome_font_get_size (rfont->font);
	affine[0] = rfont->transform[0] * size * 0.001;
	affine[1] = rfont->transform[1] * size * 0.001;
	affine[2] = rfont->transform[2] * size * 0.001;
	affine[3] = rfont->transform[3] * size * 0.001;
	affine[4] = affine[5] = 0.0;

	bpath = (ArtBpath *) gnome_font_face_get_glyph_stdoutline (rfont->font->face, glyph);

	g_return_val_if_fail (bpath != NULL, NULL);

	bpath = art_bpath_affine_transform (bpath, affine);

	g_hash_table_insert (rfont->bpaths, GINT_TO_POINTER (glyph), bpath);

	return bpath;
}

const ArtSVP *
gnome_rfont_get_glyph_svp (GnomeRFont * rfont, gint glyph)
{
	ArtSVP * svp, * svp1;
	ArtBpath * bpath;
	ArtVpath * vpath, * vpath1;

	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (glyph >= 0, NULL);
	g_return_val_if_fail (glyph < GRF_NUM_GLYPHS (rfont), NULL);

	svp = g_hash_table_lookup (rfont->svps, GINT_TO_POINTER (glyph));

	if (svp)
		return svp;

	bpath = (ArtBpath *) gnome_rfont_get_glyph_bpath (rfont, glyph);

	g_return_val_if_fail (bpath != NULL, NULL);

	vpath = art_bez_path_to_vec (bpath, 0.25);
	vpath1 = art_vpath_perturb (vpath);
	art_free (vpath);
	svp = art_svp_from_vpath (vpath1);
	art_free (vpath1);
	svp1 = art_svp_uncross (svp);
	art_svp_free (svp);
	svp = art_svp_rewind_uncrossed (svp1, ART_WIND_RULE_ODDEVEN);
	art_svp_free (svp1);

	g_hash_table_insert (rfont->svps, GINT_TO_POINTER (glyph), svp);

	return svp;
}

/*
 * Pixel operations
 *
 * Code: [Foreground][Existing_background][Resulting_background]
 *
 * P  - premultiplied alpha
 * N  - non-premultiplied alpha
 * 1  - alpha 255 (i.e. RGB)
 * A7 - precomputed alpha (* 255)
 *
 */

#define NR_RGBA32_R(v) ((v) >> 24)
#define NR_RGBA32_G(v) (((v) >> 16) & 0xff)
#define NR_RGBA32_B(v) (((v) >> 8) & 0xff)
#define NR_RGBA32_A(v) ((v) & 0xff)

#define PREMUL(c,a) (((c) * (a) + 127) / 255)
#define COMPOSENNN_A7(fc,fa,bc,ba,a) (((255 - (fa)) * (bc) * (ba) + (fa) * (fc) * 255 + 127) / a)
#define COMPOSEPNN_A7(fc,fa,bc,ba,a) (((255 - (fa)) * (bc) * (ba) + (fc) * 65025 + 127) / a)
#define COMPOSENNP(fc,fa,bc,ba) (((255 - (fa)) * (bc) * (ba) + (fa) * (fc) * 255 + 32512) / 65025)
#define COMPOSEPNP(fc,fa,bc,ba) (((255 - (fa)) * (bc) * (ba) + (fc) * 65025 + 32512) / 65025)
#define COMPOSENPP(fc,fa,bc,ba) (((255 - (fa)) * (bc) + (fa) * (fc) + 127) / 255)
#define COMPOSEPPP(fc,fa,bc,ba) (((255 - (fa)) * (bc) + (fc) * 255 + 127) / 255)
#define COMPOSEP11(fc,fa,bc) (((255 - (fa)) * (bc) + (fc) * 255 + 127) / 255)
#define COMPOSEN11(fc,fa,bc) (((255 - (fa)) * (bc) + (fc) * (fa) + 127) / 255)

void
gnome_rfont_render_glyph_rgba8 (GnomeRFont *rfont, gint glyph,
				guint32 rgba,
				gdouble x, gdouble y,
				guchar *buf,
				gint width, gint height, gint rowstride,
				guint flags)
{
	GRFGlyphSlot *slot;
	gint xp, yp, x0, y0, x1, y1, gmw;
	guint inkr, inkg, inkb, inka;
	guchar *s, *s0, *d, *d0;

	g_return_if_fail (rfont != NULL);
	g_return_if_fail (GNOME_IS_RFONT (rfont));
	g_return_if_fail (glyph >= 0);
	g_return_if_fail (glyph < GRF_NUM_GLYPHS (rfont));

	slot = grf_ensure_slot_graymap (rfont, glyph);
	g_return_if_fail (slot && slot->has_graymap);
	if (slot->is_greek || !slot->bd.px)
		return;

	xp = (gint) floor (x + 0.5);
	yp = (gint) floor (y + 0.5);

	x0 = MAX (0, xp + GRF_COORD_INT_LOWER (slot->x0));
	y0 = MAX (0, yp + GRF_COORD_INT_LOWER (slot->y0));
	x1 = MIN (width, xp + GRF_COORD_INT_UPPER (slot->x1));
	y1 = MIN (height, yp + GRF_COORD_INT_UPPER (slot->y1));

	gmw = GRF_COORD_INT_SIZE (slot->x0, slot->x1);

	inkr = NR_RGBA32_R (rgba);
	inkg = NR_RGBA32_G (rgba);
	inkb = NR_RGBA32_B (rgba);
	inka = NR_RGBA32_A (rgba);

	d0 = buf + y0 * rowstride + x0 * 4;
	s0 = slot->bd.px + (y0 - yp - GRF_COORD_INT_LOWER (slot->y0)) * gmw + (x0 - xp - GRF_COORD_INT_LOWER (slot->x0));

	for (y = y0; y < y1; y++) {
		s = s0;
		d = d0;
		for (x = x0; x < x1; x++) {
			guint a;
			a = PREMUL (s[0], inka);
			if (a == 255) {
				d[0] = inkr;
				d[1] = inkg;
				d[2] = inkb;
				d[3] = 255;
			} else if (a != 0) {
				guint ca;
				ca = 65025 - (255 - a) * (255 - d[3]);
				d[0] = COMPOSENNN_A7 (inkr, a, d[0], d[3], ca);
				d[1] = COMPOSENNN_A7 (inkg, a, d[1], d[3], ca);
				d[2] = COMPOSENNN_A7 (inkb, a, d[2], d[3], ca);
				d[3] = (ca + 127) / 255;
			}
			s += 1;
			d += 4;
		}
		s0 += GRF_COORD_INT_SIZE (slot->x0, slot->x1);
		d0 += rowstride;
	}
}

void
gnome_rfont_render_glyph_rgb8 (GnomeRFont *rfont, gint glyph,
			       guint32 rgba,
			       gdouble px, gdouble py,
			       guchar *buf,
			       gint width, gint height, gint rowstride,
			       guint flags)
{
	GRFGlyphSlot *slot;
	gint x, y, x0, y0, x1, y1, gmw, gmh;
	guint inkr, inkg, inkb, inka;
	guchar *s, *s0, *d, *d0;

	g_return_if_fail (rfont != NULL);
	g_return_if_fail (GNOME_IS_RFONT (rfont));
	g_return_if_fail (glyph >= 0);
	g_return_if_fail (glyph < GRF_NUM_GLYPHS (rfont));

	x = (gint) floor (px + 0.5);
	y = (gint) floor (py + 0.5);

	if (rfont->vector) {
		const ArtSVP *svp;
		svp = gnome_rfont_get_glyph_svp (rfont, glyph);
		if (svp) {
			art_rgb_svp_alpha (svp, -x, -y, width - x, height - y, rgba, buf, rowstride, 0);
		}
		return;
	}

	slot = grf_ensure_slot_graymap (rfont, glyph);
	g_return_if_fail (slot && slot->has_graymap);

	inkr = NR_RGBA32_R (rgba);
	inkg = NR_RGBA32_G (rgba);
	inkb = NR_RGBA32_B (rgba);
	inka = NR_RGBA32_A (rgba);

	if (slot->is_greek) {
		gint sx, sy;

		sx = GRF_COORD_INT_LOWER (slot->x0);
		sy = GRF_COORD_INT_LOWER (slot->y0);

		gmw = GRF_COORD_INT_UPPER (slot->x1 - slot->x0);
		gmh = GRF_COORD_INT_UPPER (slot->y1 - slot->y0);

		x0 = MAX (0, x + sx);
		y0 = MAX (0, y + sy);
		x1 = MIN (width, x + sx + gmw);
		y1 = MIN (height, y + sy + gmh);

		d0 = d = buf + y0 * rowstride + x0 * 3;
		s0 = s = &slot->bd.d[(y0 - y - sy) * gmw + (x0 - x - sx)];

		for (y = y0; y < y1; y++) {
			s = s0;
			d = d0;
			for (x = x0; x < x1; x++) {
				guint a;
				a = PREMUL (s[0], inka);
				if (a == 255) {
					d[0] = inkr;
					d[1] = inkg;
					d[2] = inkb;
				} else if (a != 0) {
					d[0] = COMPOSEN11 (inkr, a, d[0]);
					d[1] = COMPOSEN11 (inkg, a, d[1]);
					d[2] = COMPOSEN11 (inkb, a, d[2]);
				}
				s += 1;
				d += 3;
			}
			s0 += gmw;
			d0 += rowstride;
		}
		return;
	}

	if (!slot->bd.px)
		return;

	x0 = MAX (0, x + GRF_COORD_INT_LOWER (slot->x0));
	y0 = MAX (0, y + GRF_COORD_INT_LOWER (slot->y0));
	x1 = MIN (width, x + GRF_COORD_INT_UPPER (slot->x1));
	y1 = MIN (height, y + GRF_COORD_INT_UPPER (slot->y1));

	gmw = GRF_COORD_INT_SIZE (slot->x0, slot->x1);
	gmh = GRF_COORD_INT_SIZE (slot->y0, slot->y1);

	d0 = d = buf + y0 * rowstride + x0 * 3;
	s0 = s = slot->bd.px + (y0 - y - GRF_COORD_INT_LOWER (slot->y0)) * gmw + (x0 - x - GRF_COORD_INT_LOWER (slot->x0));

	for (y = y0; y < y1; y++) {
		s = s0;
		d = d0;
		for (x = x0; x < x1; x++) {
			guint a;
			a = PREMUL (s[0], inka);
			if (a == 255) {
				d[0] = inkr;
				d[1] = inkg;
				d[2] = inkb;
			} else if (a != 0) {
				d[0] = COMPOSEN11 (inkr, a, d[0]);
				d[1] = COMPOSEN11 (inkg, a, d[1]);
				d[2] = COMPOSEN11 (inkb, a, d[2]);
			}
			s += 1;
			d += 3;
		}
		s0 += gmw;
		d0 += rowstride;
	}
}

/*
 * These are somewhat tricky, as you cannot do arbotrarily transformed
 * fonts with Pango. So be cautious and try to figure out the best
 * solution.
 */

PangoFont *
gnome_rfont_get_closest_pango_font (const GnomeRFont *rfont, PangoFontMap *map)
{
	gdouble dx, dy, dpi;

	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (map != NULL, NULL);
	g_return_val_if_fail (PANGO_IS_FONT_MAP (map), NULL);

	dx = rfont->transform[2] - rfont->transform[0];
	dx = dx * dx;
	dy = rfont->transform[1] - rfont->transform[3];
	dy = dy * dy;

	dpi = sqrt (dx * dy * 0.5);

	return gnome_font_get_closest_pango_font (rfont->font, map, dpi);
}

PangoFontDescription *
gnome_rfont_get_pango_description (const GnomeRFont *rfont)
{
	gdouble dx, dy, dpi;

	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);

	dx = rfont->transform[2] - rfont->transform[0];
	dx = dx * dx;
	dy = rfont->transform[1] - rfont->transform[3];
	dy = dy * dy;

	dpi = sqrt (dx * dy * 0.5);

	return gnome_font_get_pango_description (rfont->font, dpi);
}

/* Helpers */

static gboolean
rfont_free_svp (gpointer key, gpointer value, gpointer data)
{
	art_svp_free ((ArtSVP *) value);

	return TRUE;
}

static gboolean
rfont_free_bpath (gpointer key, gpointer value, gpointer data)
{
	art_free (value);

	return TRUE;
}

static guint
rfont_hash (gconstpointer key)
{
	GnomeRFont * rfont;

	rfont = (GnomeRFont *) key;

	return (guint) rfont->font;
}

static gint
rfont_equal (gconstpointer key1, gconstpointer key2)
{
	GnomeRFont * f1, * f2;

	f1 = (GnomeRFont *) key1;
	f2 = (GnomeRFont *) key2;

	if (f1->font != f2->font)
		return FALSE;

	return art_affine_equal (f1->transform, f2->transform);
}

static inline gint
gnome_rfont_get_num_glyphs (GnomeRFont *rfont)
{
	if (!GFF_LOADED (rfont->font->face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face",
			   __FILE__, __LINE__, rfont->font->face->entry->name);
		return 0;
	}
	return rfont->font->face->num_glyphs;
}
