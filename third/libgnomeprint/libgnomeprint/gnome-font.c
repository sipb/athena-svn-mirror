/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-font.c: basic user visible handle to scaled typeface
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
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gnome-font-private.h>

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
	PROP_DESCENDER,
	PROP_SIZE
};

static void gnome_font_class_init (GnomeFontClass *klass);
static void gnome_font_init (GnomeFont *font);
static void gnome_font_finalize (GObject *object);
static void gnome_font_get_prop (GObject *o, guint id, GValue *value, GParamSpec *pspec);

static gboolean gf_free_outline (gpointer a, gpointer b, gpointer data);

static GObjectClass *parent_class = NULL;

GType
gnome_font_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomeFontClass),
			NULL, NULL,
			(GClassInitFunc) gnome_font_class_init,
			NULL, NULL,
			sizeof (GnomeFont),
			0,
			(GInstanceInitFunc) gnome_font_init
		};
		type = g_type_register_static (G_TYPE_OBJECT, "GnomeFont", &info, 0);
	}
	return type;
}

static void
gnome_font_class_init (GnomeFontClass *klass)
{
	GObjectClass *object_class;

	object_class = (GObjectClass*) klass;

	parent_class = g_type_class_peek_parent (klass);
  
	object_class->finalize = gnome_font_finalize;
	object_class->get_property = gnome_font_get_prop;

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
	g_object_class_install_property (object_class, PROP_SIZE,
					 g_param_spec_double ("Size", NULL, NULL, 0.001, 1000.0, 12.0, (G_PARAM_READABLE)));
}

static void
gnome_font_init (GnomeFont *font)
{
	font->face = NULL;
	font->size = 0.0;
	font->name = NULL;
	font->outlines = g_hash_table_new (NULL, NULL);
}

static void
gnome_font_finalize (GObject *object)
{
	GnomeFont *font;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_FONT (object));

	font = GNOME_FONT (object);

	if (font->face) {
		font->face->fonts = g_slist_remove (font->face->fonts, font);
		gnome_font_face_unref (font->face);
		font->face = NULL;
		if (font->name) {
			g_free (font->name);
			font->name = NULL;
		}
		if (font->outlines) {
			g_hash_table_foreach_remove (font->outlines, gf_free_outline, NULL);
			g_hash_table_destroy (font->outlines);
			font->outlines = NULL;
		}
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnome_font_get_prop (GObject *o, guint id, GValue *value, GParamSpec *pspec)
{
	GnomeFont *font;
	const ArtDRect *fbbox;
	ArtDRect *bbox;

	font = GNOME_FONT (o);

	switch (id) {
	case PROP_FONTNAME:
		g_value_set_string (value, gnome_font_face_get_ps_name (font->face));
		break;
	case PROP_FULLNAME:
		g_value_set_string (value, gnome_font_face_get_name (font->face));
		break;
	case PROP_FAMILYNAME:
		g_value_set_string (value, gnome_font_face_get_family_name (font->face));
		break;
	case PROP_WEIGHT:                      
                /* FIXME: we  should be using GnomeFontWeight */
		g_value_set_string (value, font->face->entry->speciesname);
		break;
	case PROP_ITALICANGLE:
		/* fixme: implement */
		g_value_set_double (value, gnome_font_is_italic (font) ? -20.0 : 0.0);
		break;
	case PROP_ISFIXEDPITCH:
		/* fixme: implement */
		g_value_set_boolean (value, FALSE);
		break;
	case PROP_FONTBBOX:
		fbbox = gnome_font_face_get_stdbbox (font->face);
		g_return_if_fail (fbbox != NULL);
		bbox = g_new (ArtDRect, 1);
		bbox->x0 = fbbox->x0 * font->size * 0.001;
		bbox->y0 = fbbox->y0 * font->size * 0.001;
		bbox->x1 = fbbox->x1 * font->size * 0.001;
		bbox->y1 = fbbox->y1 * font->size * 0.001;
		g_value_set_pointer (value, bbox);
		break;
	case PROP_UNDERLINEPOSITION:
		g_value_set_double (value, gnome_font_get_underline_position (font));
		break;
	case PROP_UNDERLINETHICKNESS:
		g_value_set_double (value, gnome_font_get_underline_thickness (font));
		break;
	case PROP_VERSION:
		/* fixme: implement */
		g_value_set_string (value, "0.0");
		break;
	case PROP_CAPHEIGHT:
		/* fixme: implement */
		g_value_set_double (value, 0.9 * font->size);
		break;
	case PROP_XHEIGHT:
		/* fixme: implement */
		g_value_set_double (value, 0.6 * font->size);
		break;
	case PROP_ASCENDER:
		g_value_set_double (value, gnome_font_get_ascender (font));
		break;
	case PROP_DESCENDER:
		g_value_set_double (value, gnome_font_get_descender (font));
		break;
	case PROP_SIZE:
		g_value_set_double (value, font->size);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (o, id, pspec);
		break;
	}
}

GnomeFont *
gnome_font_face_get_font_full (GnomeFontFace *face, gdouble size, gdouble *affine)
{
	GnomeFont *font;
	GSList *l;

	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	for (l = face->fonts; l != NULL; l = l->next) {
		font = (GnomeFont *) l->data;
		if (font->size == size) {
			gnome_font_ref (font);
			return font;
		}
	}

	font = g_object_new (GNOME_TYPE_FONT, NULL);

	gnome_font_face_ref (face);
	font->face = face;
	font->size = size;
	face->fonts = g_slist_prepend (face->fonts, font);

	return font;
}

GnomeFont *
gnome_font_find (const guchar *name, gdouble size)
{
	GnomeFontFace *face;
	GnomeFont *font;

	face = gnome_font_face_find (name);
	g_return_val_if_fail (face != NULL, NULL);

	font = gnome_font_face_get_font_full (face, size, NULL);

	gnome_font_face_unref (face);

	return font;
}

GnomeFont *
gnome_font_find_closest (const guchar *name, gdouble size)
{
	GnomeFontFace *face;
	GnomeFont *font;

	face = gnome_font_face_find_closest (name);

	g_return_val_if_fail (face != NULL, NULL);

	font = gnome_font_face_get_font_full (face, size, NULL);
	gnome_font_face_unref (face);

	return font;
}

GnomeFont *
gnome_font_find_from_full_name (const guchar *name)
{
	char *copy;
	char *str_size;
	double size;
	GnomeFont *font;

	g_return_val_if_fail(name != NULL, NULL);

	copy = g_strdup (name);
	str_size = strrchr (copy, ' ');
	if (str_size) {
		*str_size = 0;
		str_size ++;
		size = atof (str_size);
	} else {
		size = 12;
	}

	font = gnome_font_find (copy, size);
	g_free (copy);

	return font;
}

GnomeFont *
gnome_font_find_closest_from_full_name (const guchar *name)
{
	char *copy;
	char *str_size;
	double size;
	GnomeFont *font;

	g_return_val_if_fail (name != NULL, NULL);

	copy = g_strdup (name);
	str_size = strrchr (copy, ' ');
	if (str_size) {
		*str_size = 0;
		str_size ++;
		size = atof (str_size);
	} else {
		size = 12;
	}

	font = gnome_font_find_closest (copy, size);
	g_free (copy);

	return font;
}

/**
 * gnome_font_find_from_filename:
 * @filename: filename of a font face in the system font database
 * @index_: index of the face within @filename. (Font formats such as
 *          TTC/TrueType Collections can have multiple fonts within
 *          a single file.
 * @size: size (in points) at which to load the font
 * 
 * Creates a font using the filename and index of the face within the file to
 * identify the #GnomeFontFace. The font must already be within
 * the system font database; this can't be used to access arbitrary
 * fonts on disk.
 * 
 * Return value: a newly created font if the face could be located,
 *  otherwise %NULL
 **/
GnomeFont *
gnome_font_find_from_filename (const guchar *filename, gint index_, gdouble size)
{
	GnomeFontFace *face;
	GnomeFont *font;

	face = gnome_font_face_find_from_filename (filename, index_);
	if (!face)
		return NULL;

	font = gnome_font_face_get_font_full (face, size, NULL);

	gnome_font_face_unref (face);

	return font;
}

/* Find the closest weight matching the family name, weight, and italic
   specs. */

GnomeFont *
gnome_font_find_closest_from_weight_slant (const guchar *family, GnomeFontWeight weight, gboolean italic, gdouble size)
{
	GnomeFontFace * face;
	GnomeFont * font;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (*family != '\0', NULL);

	face = gnome_font_face_find_closest_from_weight_slant (family, weight, italic);
	g_return_val_if_fail (face != NULL, NULL);

	font = gnome_font_face_get_font_full (face, size, NULL);

	gnome_font_face_unref (face);

	return font;
}

ArtPoint *
gnome_font_get_glyph_stdadvance (GnomeFont *font, gint glyph, ArtPoint *advance)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (advance != NULL, NULL);

	if (!gnome_font_face_get_glyph_stdadvance (font->face, glyph, advance)) {
		g_warning ("file %s: line %d: Face stdadvance failed", __FILE__, __LINE__);
		return NULL;
	}

	advance->x *= 0.001 * font->size;
	advance->y *= 0.001 * font->size;

	return advance;
}

/* fixme: */

ArtDRect *
gnome_font_get_glyph_stdbbox (GnomeFont *font, gint glyph, ArtDRect *bbox)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	if (!gnome_font_face_get_glyph_stdbbox (font->face, glyph, bbox)) {
		g_warning ("file %s: line %d: Face stdbbox failed", __FILE__, __LINE__);
		return NULL;
	}

	bbox->x0 *= 0.001 * font->size;
	bbox->y0 *= 0.001 * font->size;
	bbox->x1 *= 0.001 * font->size;
	bbox->y1 *= 0.001 * font->size;

	return bbox;
}

const ArtBpath *
gnome_font_get_glyph_stdoutline (GnomeFont *font, gint glyph)
{
	const ArtBpath *outline, *faceoutline;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	outline = g_hash_table_lookup (font->outlines, GINT_TO_POINTER (glyph));

	if (!outline) {
		gdouble affine[6];

		faceoutline = gnome_font_face_get_glyph_stdoutline (font->face, glyph);

		if (!faceoutline) {
			g_warning ("file %s: line %d: Face stdoutline failed", __FILE__, __LINE__);
			return NULL;
		}

		art_affine_scale (affine, 0.001 * font->size, 0.001 * font->size);
		outline = art_bpath_affine_transform (faceoutline, affine);

		g_hash_table_insert (font->outlines, GINT_TO_POINTER (glyph), (gpointer) outline);
	}

	return outline;
}

ArtPoint *
gnome_font_get_glyph_stdkerning (GnomeFont *font, gint glyph1, gint glyph2, ArtPoint *kerning)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (kerning != NULL, NULL);

	if (!gnome_font_face_get_glyph_stdkerning (font->face, glyph1, glyph2, kerning)) {
		g_warning ("file %s: line %d: Face stdkerning failed", __FILE__, __LINE__);
		return NULL;
	}

	kerning->x *= 0.001 * font->size;
	kerning->y *= 0.001 * font->size;

	return kerning;
}

/**
 * gnome_font_get_ascender:
 * @font: the GnomeFont to operate on
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: The ascender of the font.
 */
double
gnome_font_get_ascender (GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0); 

	return gnome_font_face_get_ascender (font->face) * 0.001 * font->size;
}

/**
 * gnome_font_get_descender:
 * @font: the GnomeFont to operate on
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: The descender of the font.
 */
double
gnome_font_get_descender (GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0); 

	return gnome_font_face_get_descender (font->face) * 0.001 * font->size;
}

/**
 * gnome_font_get_underline_position:
 * @font: the GnomeFont to operate on
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: The underline position of the font.
 */
double
gnome_font_get_underline_position (GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0); 

	return gnome_font_face_get_underline_position (font->face) * 0.001 * font->size;
}

/**
 * gnome_font_get_underline_thickness:
 * @font: the GnomeFont to operate on
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: The underline thickness of the font.
 */
double
gnome_font_get_underline_thickness (GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0); 

	return gnome_font_face_get_underline_thickness (font->face) * 0.001 * font->size;
}

/* return a pointer to the (PostScript) name of the font */
const guchar *
gnome_font_get_name (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	return gnome_font_face_get_name (font->face);
}

const guchar *
gnome_font_get_ps_name (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	return gnome_font_face_get_ps_name (font->face);
}

guchar *
gnome_font_get_full_name (GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	return g_strdup_printf("%s %f", gnome_font_get_name (font), font->size);
}

const guchar *
gnome_font_get_family_name (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	return gnome_font_face_get_family_name (font->face);
}

const guchar *
gnome_font_get_species_name (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	return gnome_font_face_get_species_name (font->face);
}

gdouble
gnome_font_get_size (const GnomeFont * font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);

	return font->size;
}

GnomeFontFace *
gnome_font_get_face (const GnomeFont * font)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	return font->face;
}

gdouble
gnome_font_get_glyph_width (GnomeFont *font, gint ch)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);

	if (ch < 0 || ch >= 256) return 0.0;

	return gnome_font_face_get_glyph_width (font->face, ch) * 0.001 * font->size;
}

/*
 * These are somewhat tricky, as you cannot do arbitrarily transformed
 * fonts with Pango. So be cautious and try to figure out the best
 * solution.
 */

PangoFont *
gnome_font_get_closest_pango_font (const GnomeFont *font, PangoFontMap *map, gdouble dpi)
{
	PangoFontDescription *desc;
	PangoFont *pfont;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (map != NULL, NULL);
	g_return_val_if_fail (PANGO_IS_FONT_MAP (map), NULL);
	g_return_val_if_fail (dpi > 0, NULL);

	desc = gnome_font_get_pango_description (font, dpi);
	g_return_val_if_fail (desc != NULL, NULL);

	pfont = pango_font_map_load_font (map, NULL, desc);

	pango_font_description_free (desc);

	return pfont;
}

PangoFontDescription *
gnome_font_get_pango_description (const GnomeFont *font, gdouble dpi)
{
	PangoFontDescription *desc;
	gchar *str;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (dpi > 0, NULL);

	str = g_strdup_printf ("%s %d", gnome_font_face_get_name (font->face), (gint) font->size);

	desc = pango_font_description_from_string (str);

	g_free (str);

	return desc;
}

/**
 * gnome_font_lookup_default:
 * @font: 
 * @unicode: 
 *
 * Get the glyph number corresponding to a given unicode
 * 
 * Return Value: glyph number, -1 if it is not mapped
 **/
gint
gnome_font_lookup_default (GnomeFont *font, gint unicode)
{
	g_return_val_if_fail (font != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT (font), -1);

	return gnome_font_face_lookup_default (font->face, unicode);
}

static gboolean
gf_free_outline (gpointer a, gpointer b, gpointer data)
{
	g_free (b);

	return TRUE;
}



/**
 * gnome_font_get_width_utf8:
 * @font: 
 * @s: 
 * 
 * To be deprecated soon
 * 
 * Return Value: 
 **/
double
gnome_font_get_width_utf8 (GnomeFont *font, const char *s)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);
	g_return_val_if_fail (s != NULL, 0.0);
 
	return gnome_font_get_width_utf8_sized (font, s, strlen (s));
}

/**
 * gnome_font_get_width_utf8_sized:
 * @font: 
 * @text: 
 * @n: 
 * 
 * To be depreacated soon
 * 
 * Return Value: 
 **/
double
gnome_font_get_width_utf8_sized (GnomeFont *font, const char *text, int n)
{
	double width;
	const gchar *p;
 
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);
	g_return_val_if_fail (text != NULL, 0.0);

	width = 0.0;

	for (p = text; p && p < (text + n); p = g_utf8_next_char (p)) {
		gint unival, glyph;
		unival = g_utf8_get_char (p);
		glyph = gnome_font_lookup_default (font, unival);
		width += gnome_font_face_get_glyph_width (font->face, glyph);
	}
	
	return width * 0.001 * font->size;
}
