/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-font-face.c: unscaled typeface
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
 *    Jody Goldberg <jody@ximian.com>
 *    Miguel de Icaza <miguel@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Christopher James Lahey <clahey@ximian.com>
 *    Michael Meeks <michael@ximian.com>
 *    Morten Welinder <terra@diku.dk>
 *
 *  Copyright (C) 2000-2003 Ximian Inc. and authors
 */

#include <config.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <locale.h>

#include <freetype/ftoutln.h>

#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-print-i18n.h>
#include <libgnomeprint/gnome-fontmap.h>
#include <libgnomeprint/gnome-font-family.h>
#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/ttsubset/gnome-print-tt-subset.h>

enum {
	PROP_0,
	PROP_FONTNAME,
	PROP_FULLNAME,
	PROP_FAMILYNAME,
	PROP_WEIGHT,
	PROP_ITALICANGLE,
	PROP_ISFIXEDPITCH,
	PROP_FONTBBOX,
	PROP_UNDERLINEPOSITION,
	PROP_UNDERLINETHICKNESS,
	PROP_VERSION,
	PROP_CAPHEIGHT,
	PROP_XHEIGHT,
	PROP_ASCENDER,
	PROP_DESCENDER
};

static void gnome_font_face_class_init (GnomeFontFaceClass * klass);
static void gnome_font_face_init (GnomeFontFace * face);
static void gnome_font_face_finalize (GObject * object);
static void gnome_font_face_get_prop (GObject *o, guint id, GValue *value, GParamSpec *pspec);

static void gff_load_metrics (GnomeFontFace *face, gint glyph);
static void gff_load_outline (GnomeFontFace *face, gint glyph);

static void gf_pso_print_sized (GnomeFontPsObject *pso, const guchar *text, gint size);
static void gf_pso_sprintf (GnomeFontPsObject * pso, 
			    const gchar * format, ...);

static void gnome_font_face_ps_embed_ensure_size (GnomeFontPsObject * pso, gint size);

#define GFE_IS_T1(e) ((e)->type == GP_FONT_ENTRY_TYPE1)
#define GFE_IS_TT(e) ((e)->type == GP_FONT_ENTRY_TRUETYPE)

static GObjectClass * parent_class;

GType
gnome_font_face_get_type (void)
{
	static GType face_type = 0;
	if (!face_type) {
		static const GTypeInfo face_info = {
			sizeof (GnomeFontFaceClass),
			NULL, NULL,
			(GClassInitFunc) gnome_font_face_class_init,
			NULL, NULL,
			sizeof (GnomeFontFace),
			0,
			(GInstanceInitFunc) gnome_font_face_init
		};
		face_type = g_type_register_static (G_TYPE_OBJECT, "GnomeFontFace", &face_info, 0);
	}
	return face_type;
}

static void
gnome_font_face_class_init (GnomeFontFaceClass * klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_font_face_finalize;
	object_class->get_property = gnome_font_face_get_prop;

	g_object_class_install_property (object_class, PROP_FONTNAME,
					 g_param_spec_string ("FontName", NULL, NULL, NULL, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_FULLNAME,
					 g_param_spec_string ("FullName", NULL, NULL, NULL, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_FAMILYNAME,
					 g_param_spec_string ("FamilyName", NULL, NULL, NULL, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_WEIGHT,
					 g_param_spec_string ("Weight", NULL, NULL, NULL, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_ITALICANGLE,
					 g_param_spec_double ("ItalicAngle", NULL, NULL, -90.0, 90.0, 0.0, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_ISFIXEDPITCH,
					 g_param_spec_boolean ("IsFixedPitch", NULL, NULL, FALSE, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_FONTBBOX,
					 g_param_spec_pointer ("FontBBox", NULL, NULL, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_UNDERLINEPOSITION,
					 g_param_spec_double ("UnderlinePosition", NULL, NULL, -1000.0, 1000.0, -100.0, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_UNDERLINETHICKNESS,
					 g_param_spec_double ("UnderlineThickness", NULL, NULL, 0.1, 100.0, 10.0, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_VERSION,
					 g_param_spec_string ("Version", NULL, NULL, "0.0", (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_CAPHEIGHT,
					 g_param_spec_double ("CapHeight", NULL, NULL, 100.0, 2000.0, 800.0, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_XHEIGHT,
					 g_param_spec_double ("XHeight", NULL, NULL, 100.0, 2000.0, 500.0, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_ASCENDER,
					 g_param_spec_double ("Ascender", NULL, NULL, -1000.0, 10000.0, 1000.0, (G_PARAM_READABLE)));
	g_object_class_install_property (object_class, PROP_DESCENDER,
					 g_param_spec_double ("Descender", NULL, NULL, -10000.0, 1000.0, -100.0, (G_PARAM_READABLE)));

}

static void
gnome_font_face_init (GnomeFontFace * face)
{
	face->entry = NULL;
	face->num_glyphs = 0;
	face->glyphs = NULL;
	face->ft_face = NULL;
	face->fonts = NULL;
}

static void
gnome_font_face_finalize (GObject * object)
{
	GnomeFontFace * face;

	face = (GnomeFontFace *) object;

	if (face->entry) {
		g_assert (face->entry->face == face);
		face->entry->face = NULL;
		gp_font_entry_unref (face->entry);
		face->entry = NULL;
	}

	if (face->glyphs) {
		gint i;
		for (i = 0; i < face->num_glyphs; i++) {
			if (face->glyphs[i].bpath)
				g_free (face->glyphs[i].bpath);
		}
		g_free (face->glyphs);
		face->glyphs = NULL;
	}

	if (face->ft_face) {
		FT_Done_Face (face->ft_face);
		face->ft_face = NULL;
	}

	if (face->psname) {
		g_free (face->psname);
		face->psname = NULL;
	}

	/* Fonts should reference us */
	g_assert (face->fonts == NULL);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnome_font_face_get_prop (GObject *o, guint id, GValue *value, GParamSpec *pspec)
{
	GnomeFontFace *face;
	const ArtDRect *fbbox;
	ArtDRect *bbox;

	face = GNOME_FONT_FACE (o);

	switch (id) {
	case PROP_FONTNAME:
		g_value_set_string (value, face->psname);
		break;
	case PROP_FULLNAME:
		g_value_set_string (value, face->entry->name);
		break;
	case PROP_FAMILYNAME:
		g_value_set_string (value, face->entry->familyname);
		break;
	case PROP_WEIGHT:
		/* FIXME: we should be using the GnomeFontWeight Weight */
		g_value_set_string (value, face->entry->speciesname);
		break;
	case PROP_ITALICANGLE:
		/* FIXME: implement (Lauris) */
		g_value_set_double (value, gnome_font_face_is_italic (face) ? -20.0 : 0.0);
		break;
	case PROP_ISFIXEDPITCH:
		/* FIXME: implement (Lauris) */
		g_value_set_boolean (value, FALSE);
		break;
	case PROP_FONTBBOX:
		fbbox = gnome_font_face_get_stdbbox (face);
		g_return_if_fail (fbbox != NULL);
		bbox = g_new (ArtDRect, 1);
		*bbox = *fbbox;
		g_value_set_pointer (value, bbox);
		break;
	case PROP_UNDERLINEPOSITION:
		g_value_set_double (value, gnome_font_face_get_underline_position (face));
		break;
	case PROP_UNDERLINETHICKNESS:
		g_value_set_double (value, gnome_font_face_get_underline_thickness (face));
		break;
	case PROP_VERSION:
		g_value_set_string (value, "0.0");
		break;
	case PROP_CAPHEIGHT:
		/* FIXME: implement (Lauris) */
		g_value_set_double (value, 900.0);
		break;
	case PROP_XHEIGHT:
		/* FIXME: implement (Lauris) */
		g_value_set_double (value, 600.0);
		break;
	case PROP_ASCENDER:
		g_value_set_double (value, gnome_font_face_get_ascender (face));
		break;
	case PROP_DESCENDER:
		g_value_set_double (value, gnome_font_face_get_descender (face));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (o, id, pspec);
		break;
	}
}

/*
 * Naming
 *
 * gnome_font_face_get_name () should return one "true" name for font, that
 * does not have to be its PostScript name.
 * In future those names can be possibly localized (except ps_name)
 */



/**
 * gnome_font_face_get_name:
 * @face: 
 * 
 * Return the name of the Font
 * 
 * Return Value: a const pointer to the name, NULL on error
 **/
const guchar *
gnome_font_face_get_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->entry->name;
}

/**
 * gnome_font_face_get_family_name:
 * @face: 
 * 
 * Get the family name
 * 
 * Return Value: a const pointer to the family name, NULL on error
 **/
const guchar *
gnome_font_face_get_family_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->entry->familyname;
}

/**
 * gnome_font_face_get_species_name:
 * @face: 
 * 
 * Get the species name of the font
 * 
 * Return Value: a const pointer to the species name, NULL on error
 **/
const guchar *
gnome_font_face_get_species_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->entry->speciesname;
}

/**
 * gnome_font_face_get_ps_name:
 * @face: 
 * 
 * The postscript name of the font. This is the name with which
 * the font is embeded inside Postscript/PDF jobs.
 * 
 * Return Value: a const pointer to the name, NULL on error
 **/
const guchar *
gnome_font_face_get_ps_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->psname;
}

/**
 * gnome_font_face_get_num_glyphs:
 * @face: 
 * 
 * Returns the number of glyphs in the font
 * 
 * Return Value: number of glyphs, 0 on error
 **/
gint
gnome_font_face_get_num_glyphs (GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0);

	if (!GFF_LOADED (face)) {
		g_warning ("Could not load FACE %s, inside _get_num_glyphs", face->entry->name);
		return 0;
	}

	return face->num_glyphs;
}

/*
 * Metrics
 *
 * Currently those return standard values for left to right, horizontal script
 * The prefix std means, that there (a) will hopefully be methods to extract
 * different metric values and (b) for given face one text direction can
 * be defined as "default"
 * All face metrics are given in 0.001 em units
 */

const ArtDRect *
gnome_font_face_get_stdbbox (GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	return &face->bbox;
}

ArtPoint *
gnome_font_face_get_glyph_stdadvance (GnomeFontFace * face, gint glyph, ArtPoint * advance)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (advance != NULL, NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->num_glyphs))
		glyph = 0;

	if (!face->glyphs[glyph].metrics)
		gff_load_metrics ((GnomeFontFace *) face, glyph);

	*advance = face->glyphs[glyph].advance;

	return advance;
}

ArtDRect *
gnome_font_face_get_glyph_stdbbox (GnomeFontFace * face, gint glyph, ArtDRect * bbox)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->num_glyphs))
		glyph = 0;

	if (!face->glyphs[glyph].metrics)
		gff_load_metrics ((GnomeFontFace *) face, glyph);

	*bbox = face->glyphs[glyph].bbox;

	return bbox;
}

const ArtBpath *
gnome_font_face_get_glyph_stdoutline (GnomeFontFace * face, gint glyph)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->num_glyphs))
		glyph = 0;

	if (!face->glyphs[glyph].bpath)
		gff_load_outline ((GnomeFontFace *) face, glyph);

	return face->glyphs[glyph].bpath;
}

ArtPoint *
gnome_font_face_get_glyph_stdkerning (GnomeFontFace *face, gint glyph0, gint glyph1, ArtPoint *kerning)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (kerning != NULL, NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	if ((glyph0 < 0) || (glyph0 >= face->num_glyphs))
		glyph0 = 0;
	if ((glyph1 < 0) || (glyph1 >= face->num_glyphs))
		glyph1 = 0;

	if (!FT_HAS_KERNING (face->ft_face)) {
		kerning->x = kerning->y = 0.0;
	} else {
		FT_Error result;
		FT_Vector akern;
		result = FT_Get_Kerning (face->ft_face, glyph0, glyph1, ft_kerning_unscaled, &akern);
		g_return_val_if_fail (result == FT_Err_Ok, NULL);
		kerning->x = akern.x * face->ft2ps;
		kerning->y = akern.y * face->ft2ps;
	}

	return kerning;
}

/*
 * Give (possibly localized) demonstration text for a given face
 * Most usually this tells about quick fox and lazy dog...
 */
const guchar *
gnome_font_face_get_sample (GnomeFontFace * face)
{
	return _("The quick brown fox jumps over the lazy dog.");
}

/*
 * Creates new face and creates link with FontEntry
 */
static void
gff_face_from_entry (GPFontEntry *e)
{
	GnomeFontFace *face;

	g_return_if_fail (e->face == NULL);

	face = g_object_new (GNOME_TYPE_FONT_FACE, NULL);

	gp_font_entry_ref (e);
	face->entry = e;
	e->face = face;
}

GnomeFontFace *
gnome_font_face_find (const guchar *name)
{
	GPFontMap * map;
	GPFontEntry * e;

	if (name) {
		map = gp_fontmap_get ();

		e = g_hash_table_lookup (map->fontdict, name);
		if (!e) {
			gp_fontmap_release (map);
			return NULL;
		}
		if (e->face) {
			gnome_font_face_ref (e->face);
			gp_fontmap_release (map);
			return e->face;
		}

		gff_face_from_entry (e);

		gp_fontmap_release (map);

		return (e->face);
	} else {
		return gnome_font_face_find_closest (NULL);
	}
}

GnomeFontFace *
gnome_font_face_find_closest (const guchar *name)
{
	GnomeFontFace *face = NULL;

	if (name)
		face = gnome_font_face_find (name);

	if (!face)
		face = gnome_font_face_find ("Sans Regular");

	if (!face) {
		GPFontMap *map = gp_fontmap_get ();
		if (map && map->fonts) {
			/* No face, no default, load whatever font is first */
			GPFontEntry *e;
			e = (GPFontEntry *) map->fonts->data;
			if (e->face) {
				gnome_font_face_ref (e->face);
			} else {
				gff_face_from_entry (e);
			}
			face = e->face;
		}
		
		gp_fontmap_release (map);
	}

	g_return_val_if_fail (face != NULL, NULL);

	return face;
}

/* 
 * Find the closest weight matching the family name, weight, and italic
 * specs. Return the unsized font.
 */

GnomeFontFace *
gnome_font_face_find_closest_from_weight_slant (const guchar *family, GnomeFontWeight weight, gboolean italic)
{
	GPFontMap * map;
	GPFontEntry * best, * entry;
	GnomeFontFace * face;
	int best_dist, dist;
	GSList * l;

	g_return_val_if_fail (family != NULL, NULL);

	/* This should be reimplemented to use the gnome_font_family_hash. */

	map = gp_fontmap_get ();

	best = NULL;
	best_dist = 1000000;
	face = NULL;

	for (l = map->fonts; l != NULL; l = l->next) {
		entry = (GPFontEntry *) l->data;
		if (!g_strcasecmp (family, entry->familyname)) {
			if (entry->type == GP_FONT_ENTRY_ALIAS)
				entry = ((GPFontEntryAlias *) entry)->ref;
			dist = abs (weight - entry->Weight) +
				100 * (italic != (entry->italic_angle != 0));
			if (dist < best_dist) {
				best_dist = dist;
				best = entry;
			}
		}
	}

	if (best)
		face = gnome_font_face_find (best->name);

	gp_fontmap_release (map);

	/* If nothing found, go with default */
	if (!face)
		return gnome_font_face_find (NULL);

	return face;
}

GnomeFontFace *
gnome_font_face_find_from_family_and_style (const guchar *family, const guchar *style)
{
	GnomeFontFamily *gff;
	GnomeFontFace *face;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (*family != '\0', NULL);
	g_return_val_if_fail (style != NULL, NULL);

	gff = gnome_font_family_new (family);
	if (!gff)
		return gnome_font_face_find (NULL);
	face = gnome_font_family_get_face_by_stylename (gff, style);
	gnome_font_family_unref (gff);
	if (!face)
		return gnome_font_face_find (NULL);

	return face;
}

/**
 * gnome_font_face_find_from_filename:
 * @filename: filename of a font face in the system font database
 * @index_: index of the face within @filename. (Font formats such as
 *          TTC/TrueType Collections can have multiple fonts within
 *          a single file.
 * 
 * Looks up the #GnomeFontFace for a particular pair of filename and
 * index of the font within the file. The font must already be within
 * the system font database; this can't be used to access arbitrary
 * fonts on disk.
 * 
 * Return value: the matching #GnomeFontFace, if any, otherwise %NULL
 **/
GnomeFontFace *
gnome_font_face_find_from_filename (const guchar *filename, gint index_)
{
	GPFontMap * map;
	GPFontEntry match_e;
	GPFontEntry * e;

	/* The hash table for map->filenamedict uses this GPFontEntry
	 * itself as a key but only the file/index are used as input
	 * to the hash and equal functions.
	 */
	match_e.file = (guchar *)filename;
	match_e.index = index_;

	map = gp_fontmap_get ();
		
	e = g_hash_table_lookup (map->filenamedict, &match_e);
	if (!e) {
		gp_fontmap_release (map);
		return NULL;
	}

	if (e->face) {
		gnome_font_face_ref (e->face);
		gp_fontmap_release (map);
		return e->face;
	}

	gff_face_from_entry (e);

	gp_fontmap_release (map);

	return (e->face);
}

GnomeFontFace *
gnome_font_face_find_closest_from_pango_font (PangoFont *pfont)
{
	PangoFontDescription *desc;
	GnomeFontFace *face;

	g_return_val_if_fail (pfont != NULL, NULL);

	desc = pango_font_describe (pfont);
	g_return_val_if_fail (desc != NULL, NULL);

	face = gnome_font_face_find_closest_from_pango_description (desc);

	pango_font_description_free (desc);

	return face;
}

GnomeFontFace *
gnome_font_face_find_closest_from_pango_description (const PangoFontDescription *desc)
{
	PangoStyle style;
	PangoWeight weight;
	gboolean italic;
	const char *family_name;

	g_return_val_if_fail (desc != NULL, NULL);

	/* pango_font_description_get_weight returns a numerical enum */
	/* value in the range from 100 to 900 with                    */
	/* PANGO_WEIGHT_ULTRALIGHT = 200                              */
	/* PANGO_WEIGHT_NORMAL     = 400                              */
        /* PANGO_WEIGHT_HEAVY      = 900                              */

	weight = pango_font_description_get_weight (desc);
	style = pango_font_description_get_style (desc);
	italic = ((style == PANGO_STYLE_OBLIQUE) 
		  || (style == PANGO_STYLE_ITALIC));
	family_name = pango_font_description_get_family (desc);

	/* gnome_font_face_find_closest_from_weight_slant requires as */
	/* weight a GnomeFontWeight, a numerical value ranging from   */
	/* 100 to 1100 with                                           */
        /* 	GNOME_FONT_LIGHTEST = 100, */
        /* 	GNOME_FONT_EXTRA_LIGHT = 100, */
        /* 	GNOME_FONT_THIN = 200, */
        /* 	GNOME_FONT_LIGHT = 300, */
        /* 	GNOME_FONT_BOOK = 400, */
        /* 	GNOME_FONT_REGULAR = 400, */
        /* 	GNOME_FONT_MEDIUM = 500, */
        /* 	GNOME_FONT_SEMI = 600, */
        /* 	GNOME_FONT_DEMI = 600, */
        /* 	GNOME_FONT_BOLD = 700, */
        /* 	GNOME_FONT_HEAVY = 900, */
        /* 	GNOME_FONT_EXTRABOLD = 900, */
        /* 	GNOME_FONT_BLACK = 1000, */
        /* 	GNOME_FONT_EXTRABLACK = 1100, */
        /* 	GNOME_FONT_HEAVIEST = 1100 */

	/* Since these values are reasonably close, we retain the same */
        /* numerical values. */

	return gnome_font_face_find_closest_from_weight_slant 
		(family_name, weight, italic);
}

/* This returns GList of (guchar *) */

GList *
gnome_font_style_list (const guchar *family)
{
	GnomeFontFamily *gff;
	GList *fsl;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (*family != '\0', NULL);

	gff = gnome_font_family_new (family);
	g_return_val_if_fail (gff != NULL, NULL);
	fsl = gnome_font_family_style_list (gff);
	gnome_font_family_unref (gff);

	return fsl;
}

void
gnome_font_style_list_free (GList *styles)
{
	gnome_font_family_style_list_free (styles);
}

GnomeFont *
gnome_font_face_get_font (GnomeFontFace *face, gdouble size, gdouble xres, gdouble yres)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return gnome_font_face_get_font_full (face, size, NULL);
}

GnomeFont *
gnome_font_face_get_font_default (GnomeFontFace * face, gdouble size)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return gnome_font_face_get_font (face, size, 600.0, 600.0);
}



/* fixme: */
/* Returns the glyph width in 0.001 unit */

gdouble
gnome_font_face_get_glyph_width (GnomeFontFace * face, gint glyph)
{
	ArtPoint p;

	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);

	gnome_font_face_get_glyph_stdadvance (face, glyph, &p);

	return p.x;
}

/*
 * Get the glyph number corresponding to a given unicode, or -1 if it
 * is not mapped.
 *
 * fixme: We use ugly hack to avoid segfaults everywhere
 */

gint
gnome_font_face_lookup_default (GnomeFontFace * face, gint unicode)
{
	g_return_val_if_fail (face != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return -1;
	}

	/* fixme: Nobody should ask mapping of 0 */
	if (unicode < 1)
		return 0;

	return FT_Get_Char_Index (face->ft_face, unicode);
}

gboolean
gnome_font_face_load (GnomeFontFace *face)
{
	FT_Face ft_face;
	FT_Error ft_result;
	static FT_Library ft_library = NULL;
	GPFontEntry *entry;
	FT_CharMap found = NULL;
	FT_CharMap charmap;
	FT_CharMap appleRoman = NULL;
	FT_CharMap symbol = NULL;
	const guchar *psname;
	int n;

	if (!ft_library) {
		ft_result = FT_Init_FreeType (&ft_library);
		g_return_val_if_fail (ft_result == FT_Err_Ok, FALSE);
	}

	if (face->entry->type == GP_FONT_ENTRY_ALIAS) {
		entry = ((GPFontEntryAlias *) face->entry)->ref;
	} else {
		entry = face->entry;
	}

	ft_result = FT_New_Face (ft_library, entry->file, entry->index, &ft_face);
	g_return_val_if_fail (ft_result == FT_Err_Ok, FALSE);

	psname = FT_Get_Postscript_Name (ft_face);
	if (psname == NULL) {
		g_warning ("PS name is NULL, for \"%s\" using fallback", entry->file);
		face->psname = g_strdup ("Helvetica");
	} else {
		face->psname = g_strdup (psname);
	}
	
	/* FIXME: scalability (Lauris) */
	/* FIXME: glyph names (Lauris) */

	face->ft_face = ft_face;

	/* Microsoft symbol and Adobe fontspecific take priority over unicode
 	 * so that non lingual fonts work [#80409]
	 */
 	for (n = 0; n < ft_face->num_charmaps; n++) {
 		charmap = ft_face->charmaps[n];
 
 		if ((charmap->platform_id == 7) &&  (charmap->encoding_id == 2))
 		{
 			found = charmap;
 			break;
 		}
 		else if ((charmap->platform_id == 3) &&  (charmap->encoding_id == 0))
 			symbol = charmap;
 		else if ((charmap->platform_id == 1) &&  (charmap->encoding_id == 0))
 			appleRoman = charmap;
 		else if ((charmap->platform_id == 3) &&  (charmap->encoding_id == 1))
 			found = charmap;
 	}
 
 	/* If we have no Unicode or AdobeFontspecific, will try Apple roman first,
	 * followed by symbol.  For TT symbol fonts, apple roman works better?
	 */
 	if ((!found) && (appleRoman))
		found = appleRoman;
 	if ((!found) && (symbol))
		found = symbol;
 
 	/* Hope for the best with whatever is the default */
 	if (!found) {
 		g_warning ("file %s: line %d: Face %s does not have a recognized charmap", __FILE__, __LINE__, entry->name);
 	} else {
 		ft_result = FT_Set_Charmap (ft_face, found);
 		if (ft_result != FT_Err_Ok) {
 			g_warning ("file %s: line %d: Face %s could not set charmap", __FILE__, __LINE__, entry->name);
 		}
	}
	
	ft_result = FT_Select_Charmap (ft_face, ft_encoding_unicode);
	if (ft_result != FT_Err_Ok) {
		g_warning ("file %s: line %d: Face %s does not have unicode charmap", __FILE__, __LINE__, face->entry->name);
	}

	face->num_glyphs = ft_face->num_glyphs;
	g_return_val_if_fail (face->num_glyphs > 0, FALSE);
	face->glyphs = g_new0 (GFFGlyphInfo, face->num_glyphs);

	face->ft2ps = 1000.0 / ft_face->units_per_EM;

	face->bbox.x0 = ft_face->bbox.xMin / face->ft2ps;
	face->bbox.y0 = ft_face->bbox.yMin * face->ft2ps;
	face->bbox.x1 = ft_face->bbox.xMax * face->ft2ps;
	face->bbox.y1 = ft_face->bbox.yMax * face->ft2ps;

	return TRUE;
}

static void
gff_load_metrics (GnomeFontFace *face, gint glyph)
{
	GFFGlyphInfo * gi;

	g_assert (face->ft_face);
	g_assert (!face->glyphs[glyph].metrics);

	gi = face->glyphs + glyph;

	FT_Load_Glyph (face->ft_face, glyph,
		       FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING |
		       FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM);

        gi->bbox.x0 = -face->ft_face->glyph->metrics.horiBearingX * face->ft2ps;
	gi->bbox.y1 = face->ft_face->glyph->metrics.horiBearingY * face->ft2ps;
	gi->bbox.y0 = gi->bbox.y1 - face->ft_face->glyph->metrics.height * face->ft2ps;
	gi->bbox.x1 = gi->bbox.x0 + face->ft_face->glyph->metrics.width * face->ft2ps;
	gi->advance.x = face->ft_face->glyph->metrics.horiAdvance * face->ft2ps;
	gi->advance.y = 0.0;

	face->glyphs[glyph].metrics = TRUE;
}

static ArtBpath *gff_ol2bp (FT_Outline * ol, gdouble transform[]);

static void
gff_load_outline (GnomeFontFace *face, gint glyph)
{
	gdouble a[6];

	g_assert (face->ft_face);
	g_assert (!face->glyphs[glyph].bpath);

	FT_Load_Glyph (face->ft_face, glyph,
		       FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING |
		       FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM);

	a[0] = a[3] = face->ft2ps;
	a[1] = a[2] = a[4] = a[5] = 0.0;

	face->glyphs[glyph].bpath = gff_ol2bp (&face->ft_face->glyph->outline, a);
}

/* Bpath methods */

typedef struct {
	ArtBpath * bp;
	gint start, end;
	gdouble * t;
} GFFT2OutlineData;

static int gfft2_move_to (FT_Vector * to, void * user)
{
	GFFT2OutlineData * od;
	ArtPoint p;

	od = (GFFT2OutlineData *) user;

	p.x = to->x * od->t[0] + to->y * od->t[2];
	p.y = to->x * od->t[1] + to->y * od->t[3];

	if (od->end == 0 ||
	    p.x != od->bp[od->end - 1].x3 ||
	    p.y != od->bp[od->end - 1].y3) {
		od->bp[od->end].code = ART_MOVETO;
		od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2];
		od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3];
		od->end++;
	}

	return 0;
}

static int gfft2_line_to (FT_Vector * to, void * user)
{
	GFFT2OutlineData * od;
	ArtBpath * s;
	ArtPoint p;

	od = (GFFT2OutlineData *) user;

	s = &od->bp[od->end - 1];

	p.x = to->x * od->t[0] + to->y * od->t[2];
	p.y = to->x * od->t[1] + to->y * od->t[3];

	if ((p.x != s->x3) || (p.y != s->y3)) {
		od->bp[od->end].code = ART_LINETO;
		od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2];
		od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3];
		od->end++;
	}

	return 0;
}

static int gfft2_conic_to (FT_Vector * control, FT_Vector * to, void * user)
{
	GFFT2OutlineData * od;
	ArtBpath * s, * e;
	ArtPoint c;

	od = (GFFT2OutlineData *) user;
	g_return_val_if_fail (od->end > 0, -1);

	s = &od->bp[od->end - 1];
	e = &od->bp[od->end];

	e->code = ART_CURVETO;

	c.x = control->x * od->t[0] + control->y * od->t[2];
	c.y = control->x * od->t[1] + control->y * od->t[3];
	e->x3 = to->x * od->t[0] + to->y * od->t[2];
	e->y3 = to->x * od->t[1] + to->y * od->t[3];

	od->bp[od->end].x1 = c.x - (c.x - s->x3) / 3;
	od->bp[od->end].y1 = c.y - (c.y - s->y3) / 3;
	od->bp[od->end].x2 = c.x + (e->x3 - c.x) / 3;
	od->bp[od->end].y2 = c.y + (e->y3 - c.y) / 3;
	od->end++;

	return 0;
}

static int gfft2_cubic_to (FT_Vector * control1, FT_Vector * control2, FT_Vector * to, void * user)
{
	GFFT2OutlineData * od;

	od = (GFFT2OutlineData *) user;

	od->bp[od->end].code = ART_CURVETO;
	od->bp[od->end].x1 = control1->x * od->t[0] + control1->y * od->t[2];
	od->bp[od->end].y1 = control1->x * od->t[1] + control1->y * od->t[3];
	od->bp[od->end].x2 = control2->x * od->t[0] + control2->y * od->t[2];
	od->bp[od->end].y2 = control2->x * od->t[1] + control2->y * od->t[3];
	od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2];
	od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3];
	od->end++;

	return 0;
}

static FT_Outline_Funcs gfft2_outline_funcs = {
	gfft2_move_to,
	gfft2_line_to,
	gfft2_conic_to,
	gfft2_cubic_to,
	0, 0
};

/*
 * We support only 4x4 matrix here (do you need more?)
 */

static ArtBpath *
gff_ol2bp (FT_Outline * ol, gdouble transform[])
{
	GFFT2OutlineData od;

	od.bp = g_new (ArtBpath, ol->n_points * 2 + ol->n_contours + 1);
	od.start = od.end = 0;
	od.t = transform;

	FT_Outline_Decompose (ol, &gfft2_outline_funcs, &od);

	od.bp[od.end].code = ART_END;

	/* fixme: g_renew */

	return od.bp;
}

/* PSO (PostScriptObject stuff */

static void gnome_font_face_ps_embed_t1 (GnomeFontPsObject *pso);
static void gnome_font_face_ps_embed_tt (GnomeFontPsObject *pso);
static void gnome_font_face_ps_embed_empty (GnomeFontPsObject *pso);
static void gf_pso_sprintf (GnomeFontPsObject * pso, const gchar * format, ...);
static void gnome_font_face_ps_embed_ensure_size (GnomeFontPsObject *pso, gint size);

/*
 * Generates intial PSObject
 */

GnomeFontPsObject *
gnome_font_face_pso_new (GnomeFontFace *face, guchar *residentname, gint instance)
{
	GnomeFontPsObject *pso;

	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	pso = g_new0 (GnomeFontPsObject, 1);

	pso->face = face;
	g_object_ref (G_OBJECT (face));
	if (residentname)
		pso->residentname = g_strdup (residentname);
	if (instance == 0)
		pso->encodedname = g_strdup_printf ("GnomeUni-%s", face->psname);
	else
		pso->encodedname = g_strdup_printf ("GnomeUni-%s_%03d", face->psname, instance);
	pso->bufsize = 0;
	pso->length = 0;
	pso->buf = NULL;
	if (GFF_LOADED (face)) {
		pso->encodedbytes = (face->num_glyphs < 256) ? 1 : 2;
		pso->num_glyphs = face->num_glyphs;
		pso->glyphs = g_new0 (guint32, (pso->num_glyphs + 32) >> 5);
	} else {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		pso->encodedbytes = 1;
		pso->num_glyphs = 1;
		pso->glyphs = NULL;
		gnome_font_face_ps_embed_empty (pso);
	}

	return pso;
}

#define PSO_GLYPH_MARKED(pso,g) (pso->glyphs[(g) >> 5] & (1 << ((g) & 0x1f)))

void
gnome_font_face_pso_mark_glyph (GnomeFontPsObject *pso, gint glyph)
{
	g_return_if_fail (pso != NULL);

	if (!pso->glyphs)
		return;

	glyph = CLAMP (glyph, 0, pso->num_glyphs);

	pso->glyphs[glyph >> 5] |= (1 << (glyph & 0x1f));
}

void
gnome_font_face_pso_free (GnomeFontPsObject *pso)
{
	g_return_if_fail (pso != NULL);

	if (pso->face)
		g_object_unref (G_OBJECT (pso->face));
	if (pso->residentname)
		g_free (pso->residentname);
	if (pso->encodedname)
		g_free (pso->encodedname);
	if (pso->glyphs)
		g_free (pso->glyphs);
	if (pso->buf)
		g_free (pso->buf);
	
	g_free (pso);
}

void
gnome_font_face_ps_embed (GnomeFontPsObject *pso)
{
	g_return_if_fail (pso != NULL);

	switch (pso->face->entry->type) {
	case GP_FONT_ENTRY_TYPE1:
		gnome_font_face_ps_embed_t1 (pso);
		break;
	case GP_FONT_ENTRY_TRUETYPE:
		gnome_font_face_ps_embed_tt (pso);
		break;
	default:
		g_warning ("file %s: line %d: Unknown face entry type %s:%d",
			   __FILE__, __LINE__, pso->face->entry->name, pso->face->entry->type);
		gnome_font_face_ps_embed_empty (pso);
		break;
	}
}

#define INT32_LSB(q) ((q)[0] + ((q)[1] << 8) + ((q)[2] << 16) + ((q)[3] << 24))

static void
gnome_font_face_ps_embed_t1 (GnomeFontPsObject *pso)
{
	GnomePrintBuffer b;
	guchar * fbuf;
	const gchar *file_name;
	const gchar *embeddedname;

	g_return_if_fail (pso->face->entry->type == GP_FONT_ENTRY_TYPE1);
	file_name = pso->face->entry->file;	
	embeddedname = pso->face->psname;

	if (!GFF_LOADED (pso->face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, pso->face->entry->name);
		gnome_font_face_ps_embed_empty (pso);
		return;
	}

	if (GNOME_PRINT_OK != gnome_print_buffer_mmap (&b, file_name)) {
		g_warning ("file %s: line %d: Cannot open font file %s", __FILE__, __LINE__, file_name);
		gnome_font_face_ps_embed_empty (pso);
		return;
	}

	fbuf = b.buf;
	/* Step 1: */	   
	if (*fbuf == 0x80) {
		const char hextab[16] = "0123456789abcdef";
		gint idx;
		/* this is actually the same as a pfb to pfa converter
		 * Reference: Adobe technical note 5040, "Supporting Downloadable PostScript
		 * Language Fonts", page 9
		 */
		idx = 0;
		while (idx < b.buf_size) {
			gint length, i;
			if (fbuf[idx] != 0x80) {
				g_warning ("file %s: line %d: Corrupt %s", __FILE__, __LINE__, file_name);
				gnome_font_face_ps_embed_empty (pso);
				return;
			}
			switch (fbuf[idx + 1]) {
			case 1:
				length = INT32_LSB (fbuf + idx + 2);
				gnome_font_face_ps_embed_ensure_size (pso, length);
				idx += 6;
				memcpy (pso->buf + pso->length, fbuf + idx, length);
				pso->length += length;
				idx += length;
				break;
			case 2:
				length = INT32_LSB (fbuf + idx + 2);
				gnome_font_face_ps_embed_ensure_size (pso, length * 3);
				idx += 6;
				for (i = 0; i < length; i++) {
					pso->buf[pso->length++] = hextab[fbuf[idx] >> 4];
					pso->buf[pso->length++] = hextab[fbuf[idx] & 15];
					idx += 1;
					if ((i & 31) == 31 || i == length - 1)
						pso->buf[pso->length++] = '\n';
				}
				break;
			case 3:
				/* Finished */
				gnome_font_face_ps_embed_ensure_size (pso, 1);
				pso->buf[pso->length++] = '\n';
				idx = b.buf_size;
				break;
			default:
				g_warning ("file %s: line %d: Corrupt %s", __FILE__, __LINE__, file_name);
				gnome_font_face_ps_embed_empty (pso);
				return;
			}
		}
	} else {
		gnome_font_face_ps_embed_ensure_size (pso, b.buf_size + 1);
		memcpy (pso->buf, fbuf, b.buf_size);
		pso->buf[b.buf_size] = '\0';
		pso->length = b.buf_size;
	}

	gnome_print_buffer_munmap (&b);

	if (pso->encodedbytes == 1) {
		gint glyph;
		/* 8-bit vector */
		gf_pso_sprintf (pso, "(%s) cvn findfont dup length dict begin\n", embeddedname);
		gf_pso_sprintf (pso, "{1 index /FID ne {def} {pop pop} ifelse} forall\n");
		gf_pso_sprintf (pso, "/Encoding [\n");
		for (glyph = 0; glyph < 256; glyph++) {
			guint g;
			gchar c[256];
			FT_Error status;
			g = (glyph < pso->face->num_glyphs) ? glyph : 0;
			status = FT_Get_Glyph_Name (pso->face->ft_face, g, c, 256);
			if (status != FT_Err_Ok) {
				g_warning ("file %s: line %d: Glyph %d has no name in %s", __FILE__, __LINE__, g, file_name);
				g_snprintf (c, 256, ".notdef");
			}
			gf_pso_sprintf (pso, ((glyph & 0xf) == 0xf) ? "/%s\n" : "/%s ", c);
		}
		gf_pso_sprintf (pso, "] def currentdict end\n");
		gf_pso_sprintf (pso, "(%s) cvn exch definefont pop\n", pso->encodedname);
	} else {
		gint nfonts, nglyphs, i, j;
		/* 16-bit vector */
		nglyphs = pso->face->num_glyphs;
		nfonts = (nglyphs + 255) >> 8;

		gf_pso_sprintf (pso, "32 dict begin\n");
		/* Common entries */
		gf_pso_sprintf (pso, "/FontType 0 def\n");
		gf_pso_sprintf (pso, "/FontMatrix [1 0 0 1 0 0] def\n");
		gf_pso_sprintf (pso, "/FontName (%s-Glyph-Composite) cvn def\n", embeddedname);
		gf_pso_sprintf (pso, "/LanguageLevel 2 def\n");

		/* Type 0 entries */
		gf_pso_sprintf (pso, "/FMapType 2 def\n");

		/* Bitch 'o' bitches */
		gf_pso_sprintf (pso, "/FDepVector [\n");
		for (i = 0; i < nfonts; i++) {
			gf_pso_sprintf (pso, "(%s) cvn findfont dup length dict begin\n", embeddedname);
			gf_pso_sprintf (pso, "{1 index /FID ne {def} {pop pop} ifelse} forall\n");
			gf_pso_sprintf (pso, "/Encoding [\n");
			for (j = 0; j < 256; j++) {
				gint glyph;
				gchar c[256];
				FT_Error status;
				glyph = 256 * i + j;
				if (glyph >= nglyphs)
					glyph = 0;
				status = FT_Get_Glyph_Name (pso->face->ft_face, glyph, c, 256);
				if (status != FT_Err_Ok) {
					g_warning ("file %s: line %d: Glyph %d has no name in %s", __FILE__, __LINE__, glyph, file_name);
					g_snprintf (c, 256, ".notdef");
				}
				gf_pso_sprintf (pso, ((j & 0xf) == 0xf) ? "/%s\n" : "/%s ", c);
			}
			gf_pso_sprintf (pso, "] def\n");
			gf_pso_sprintf (pso, "currentdict end (%s-Glyph-Page-%d) cvn exch definefont\n", embeddedname, i);
		}
		gf_pso_sprintf (pso, "] def\n");
		gf_pso_sprintf (pso, "/Encoding [\n");
		for (i = 0; i < 256; i++) {
			gint fn;
			fn = (i < nfonts) ? i : 0;
			gf_pso_sprintf (pso, ((i & 0xf) == 0xf) ? "%d\n" : "%d  ", fn);
		}
		gf_pso_sprintf (pso, "] def\n");
		gf_pso_sprintf (pso, "currentdict end\n");
		gf_pso_sprintf (pso, "(%s) cvn exch definefont pop\n", pso->encodedname);
	}
}

/*
 * References:
 * Adobe Inc., PostScript Language Reference, 3rd edition, Addison Wesley 1999
 * Adobe Inc., The Type42 Font Format Specification, 1998 <http://partners.adobe.com/asn/developer/PDFS/TN/5012.Type42_Spec.pdf>
 */

#define TT_BLOCK_SIZE 1024

static void
gnome_font_face_ps_embed_tt (GnomeFontPsObject *pso)
{
	GnomePrintBuffer b;
	const gchar *file_name;
	gint nglyphs, j, k, lower, upper, len;
	guchar *subfont_file = NULL;
	gushort glyphArray[256];
        guchar encoding[256];

	g_return_if_fail (pso->face->entry->type == GP_FONT_ENTRY_TRUETYPE);
 	file_name = pso->face->entry->file;

	nglyphs = pso->face->num_glyphs;

	len = pso->encodedname ? strlen (pso->encodedname) : 0;

	if (len > 4)
		lower = *(pso->encodedname + len - 4) == '_' ? atoi (pso->encodedname + len - 3) : 0;
	else
		lower = 0;

	upper = lower + 1;

	k = 1;
	lower *= 255;
	upper *= 255;

	glyphArray[0] = encoding[0] = 0;

	for ( j = lower; j < upper && j < nglyphs; j++) {
		if (PSO_GLYPH_MARKED (pso, j)) {
			glyphArray [k] = j;
			encoding [k] = j%255 + 1;
			k++;
		}
	}

	(void) gnome_print_ps_tt_create_subfont (file_name, pso->encodedname, &subfont_file, glyphArray, encoding, k);

	if (GNOME_PRINT_OK != gnome_print_buffer_mmap (&b, subfont_file)) {
		gnome_font_face_ps_embed_empty (pso);
		g_warning ("Could not parse TrueType font from %s\n", subfont_file);
		goto ps_truetype_error;
	}

	if (b.buf_size < 8)
		goto ps_truetype_error;

	gf_pso_print_sized (pso, b.buf, b.buf_size);

ps_truetype_error:

	if (b.buf)
		gnome_print_buffer_munmap (&b);

	if (subfont_file)
		unlink (subfont_file);
}	

/**
 * gnome_font_face_ps_embed_empty:
 * @pso: 
 * 
 * We use this function when something goes wrong but we still want to
 * get some output printed.
 **/
static void
gnome_font_face_ps_embed_empty (GnomeFontPsObject *pso)
{
	pso->length = 0;

	gf_pso_sprintf (pso, "%%Empty font generated by gnome-print\n");
	gf_pso_sprintf (pso, "8 dict begin /FontType 3 def /FontMatrix [.001 0 0 .001 0 0] def /FontBBox [0 0 750 950] def\n");
	gf_pso_sprintf (pso, "/Encoding 256 array def 0 1 255 {Encoding exch /.notdef put} for\n");
	gf_pso_sprintf (pso, "/CharProcs 2 dict def CharProcs begin /.notdef {\n");
	gf_pso_sprintf (pso, "0 0 moveto 750 0 lineto 750 950 lineto 0 950 lineto closepath\n");
	gf_pso_sprintf (pso, "50 50 moveto 700 50 lineto 700 900 lineto 50 900 lineto closepath\n");
	gf_pso_sprintf (pso, "eofill } bind def end\n");
	gf_pso_sprintf (pso, "/BuildGlyph {1000 0 0 0 750 950 setcachedevice exch /CharProcs get exch\n");
	gf_pso_sprintf (pso, "2 copy known not {pop /.notdef} if get exec } bind def\n");
	gf_pso_sprintf (pso, "/BuildChar {1 index /Encoding get exch get 1 index /BuildGlyph get exec } bind def\n");
	if (pso->encodedbytes == 1) {
		/* 8-bit empty font */
		gf_pso_sprintf (pso, "currentdict end (%s) cvn exch definefont pop\n", pso->encodedname);
	} else {
		/* 16-bit empty font */
		gf_pso_sprintf (pso, "currentdict end (%s-Base) cvn exch definefont pop\n", pso->encodedname);
		gf_pso_sprintf (pso, "32 dict begin /FontType 0 def /FontMatrix [1 0 0 1 0 0] def\n");
		gf_pso_sprintf (pso, "/FontName (%s-Glyph-Composite) cvn def\n", pso->encodedname);
		gf_pso_sprintf (pso, "/LanguageLevel 2 def /FMapType 2 def\n");
		gf_pso_sprintf (pso, "/FDepVector [(%s-Base) cvn findfont] def", pso->encodedname);
		gf_pso_sprintf (pso, "/Encoding 256 array def 0 1 255 {Encoding exch 0 put} for\n");
		gf_pso_sprintf (pso, "currentdict end (%s) cvn exch definefont pop\n", pso->encodedname);
	}
}

static void
gf_pso_print_sized (GnomeFontPsObject *pso, const guchar *text, gint size)
{
	gnome_font_face_ps_embed_ensure_size (pso, size);
	memcpy (pso->buf + pso->length, text, size);
	pso->length += size;
}

/* Note "format" should be locale independent, so it should not use %g */
/* and friends */
static void
gf_pso_sprintf (GnomeFontPsObject *pso, const gchar * format, ...)
{
	va_list arguments;
	gchar *text;
	
	va_start (arguments, format);
	text = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	gf_pso_print_sized (pso, text, strlen (text));
	g_free (text);
}

static void
gnome_font_face_ps_embed_ensure_size (GnomeFontPsObject *pso, gint size)
{
	gint need = pso->length + size;

	if (need > pso->bufsize) {
		if (pso->bufsize < 1) {
			pso->bufsize = MAX (size, 1024);
			pso->buf = g_new (guchar, pso->bufsize);
		} else {
			while (need > pso->bufsize)
				pso->bufsize <<= 1;
			pso->buf = g_renew (guchar, pso->buf, pso->bufsize);
		}
	}
}

gdouble
gnome_font_face_get_ascender (GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, 1000.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 1000.0);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return 1000.0;
	}

	return face->ft_face->ascender * face->ft2ps;
}

gdouble
gnome_font_face_get_descender (GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 500.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 500.0);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return 500.0;
	}

	/* fixme: I am clueless - FT spec says it is positive, but actually is not (Lauris) */
	return face->ft_face->descender * face->ft2ps;
}

gdouble
gnome_font_face_get_underline_position (GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, -100.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -100.0);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return -100.0;
	}

	return face->ft_face->underline_position * face->ft2ps;
}

gdouble
gnome_font_face_get_underline_thickness (GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 10.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 10.0);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return 10.0;
	}

	return face->ft_face->underline_thickness * face->ft2ps;
}

GnomeFontWeight
gnome_font_face_get_weight_code (GnomeFontFace *face)
{
	GPFontEntry *e;

	g_return_val_if_fail (face != NULL, GNOME_FONT_BOOK);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), GNOME_FONT_BOOK);

	if (face->entry->type == GP_FONT_ENTRY_ALIAS) {
		e = ((GPFontEntryAlias *) face->entry)->ref;
	} else {
		e = face->entry;
	}

	return e->Weight;

	return GNOME_FONT_BOOK;
}

gboolean
gnome_font_face_is_italic (GnomeFontFace * face)
{
	GPFontEntry *e;

	g_return_val_if_fail (face != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), FALSE);

	if (face->entry->type == GP_FONT_ENTRY_ALIAS) {
		e = ((GPFontEntryAlias *) face->entry)->ref;
	} else {
		e = face->entry;
	}

	return (e->italic_angle < 0 ? TRUE : FALSE);
}

gboolean
gnome_font_face_is_fixed_width (GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), FALSE);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return FALSE;
	}

	return FT_IS_FIXED_WIDTH (face->ft_face);
}

/*
 * Returns PostScript name for glyph
 */

const guchar *
gnome_font_face_get_glyph_ps_name (GnomeFontFace *face, gint glyph)
{
	static GHashTable *sgd = NULL;
	FT_Error status;
	gchar c[256], *name;

	g_return_val_if_fail (face != NULL, ".notdef");
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), ".notdef");

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return ".notdef";
	}

	if (!sgd)
		sgd = g_hash_table_new (g_str_hash, g_str_equal);

	if ((glyph < 0) || (glyph >= face->num_glyphs))
		glyph = 0;

	status = FT_Get_Glyph_Name (face->ft_face, glyph, c, 256);
	if (status != FT_Err_Ok)
		return ".notdef";

	name = g_hash_table_lookup (sgd, c);
	if (!name) {
		name = g_strdup (c);
		g_hash_table_insert (sgd, name, name);
	}

	return name;
}

