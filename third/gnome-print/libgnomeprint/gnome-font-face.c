#define __GNOME_FONT_FACE_C__

#include <config.h>
#include <stdio.h>
#include <gnome.h>
#include <libgnomeprint/gnome-print-i18n.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gp-ps-unicode.h>
#include "gp-fontmap.h"
#include "parseAFM.h"

/*
 * Standard AFM attributes
 * Notice, that all distances are doubles (and FontBBox is ArtDRect)
 *
 * ItalicAngle, FontBBox, CapHeight, XHeight
 *
 * Type1 file names, if face is trivial type1 font, otherwise NULL
 * Notice, that caller has to free strings
 * Notice, that these WILL NOT be supported in gnome-font base face class,
 * but instead in some subclass
 *
 * afm, pfb, pfbname
 *
 */

enum {ARG_0,
      /* AFM Attributes */
      ARG_ITALICANGLE, ARG_FONTBBOX, ARG_CAPHEIGHT, ARG_XHEIGHT,
      /* Type1 file names */
      /* DO NOT USE THESE OUTSIDE GNOME_PRINT */
      ARG_AFM, ARG_PFB, ARG_PFBNAME
};

static void gnome_font_face_class_init (GnomeFontFaceClass * klass);
static void gnome_font_face_init (GnomeFontFace * face);
static void gnome_font_face_destroy (GtkObject * object);
static void gnome_font_face_get_arg (GtkObject * object, GtkArg * arg, guint arg_id);

static void gff_face_from_entry (GPFontEntry * e);

static gboolean gnome_font_face_gt1_load (GnomeFontFace * face);
static gboolean gff_load_afm (GnomeFontFace * face);
static void gff_fill_zero_glyph (GnomeFontFace * face);

#define GFF_LOADEDFONT(f) ((f)->private->loadedfont || gnome_font_face_gt1_load ((GnomeFontFace *) f))
#define GFF_METRICS(f) ((f)->private->glyphs || gff_load_afm ((GnomeFontFace *) f))

static GtkObjectClass * parent_class;

GtkType
gnome_font_face_get_type (void)
{
	static GtkType face_type = 0;
	if (!face_type) {
		GtkTypeInfo face_info = {
			"GnomeFontFace",
			sizeof (GnomeFontFace),
			sizeof (GnomeFontFaceClass),
			(GtkClassInitFunc) gnome_font_face_class_init,
			(GtkObjectInitFunc) gnome_font_face_init,
			NULL, NULL,
			NULL
		};
		face_type = gtk_type_unique (gtk_object_get_type (), &face_info);
	}
	return face_type;
}

static void
gnome_font_face_class_init (GnomeFontFaceClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	gtk_object_add_arg_type ("GnomeFontFace::ItalicAngle", GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_ITALICANGLE);
	gtk_object_add_arg_type ("GnomeFontFace::FontBBox", GTK_TYPE_BOXED, GTK_ARG_READABLE, ARG_FONTBBOX);
	gtk_object_add_arg_type ("GnomeFontFace::CapHeight", GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_CAPHEIGHT);
	gtk_object_add_arg_type ("GnomeFontFace::XHeight", GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_XHEIGHT);
	gtk_object_add_arg_type ("GnomeFontFace::afm", GTK_TYPE_STRING, GTK_ARG_READABLE, ARG_AFM);
	gtk_object_add_arg_type ("GnomeFontFace::pfb", GTK_TYPE_STRING, GTK_ARG_READABLE, ARG_PFB);
	gtk_object_add_arg_type ("GnomeFontFace::pfbname", GTK_TYPE_STRING, GTK_ARG_READABLE, ARG_PFBNAME);

	object_class->destroy = gnome_font_face_destroy;
	object_class->get_arg = gnome_font_face_get_arg;
}

static void
gnome_font_face_init (GnomeFontFace * face)
{
	face->private = g_new0 (GnomeFontFacePrivate, 1);

	/* Fill fake values to be used, if file is bad */
	face->private->weight_code = GNOME_FONT_BOOK;
	face->private->italic = FALSE;
	face->private->fixed_width = TRUE;

	face->private->ascender = 900;
	face->private->descender = 0;
	face->private->underline_position = -20;
	face->private->underline_thickness = 10;
	face->private->capheight = 900.0;
	face->private->italics_angle = 0.0;
	face->private->xheight = 600.0;
	face->private->bbox.x0 = 0.0;
	face->private->bbox.y0 = 0.0;
	face->private->bbox.x1 = 750.0;
	face->private->bbox.y1 = 950.0;
}

static gboolean
gff_free_privencoding (gpointer key, gpointer value, gpointer data)
{
	g_free (key);
	return TRUE;
}

static void
gnome_font_face_destroy (GtkObject * object)
{
	GnomeFontFace * face;

	face = (GnomeFontFace *) object;

	if (face->private) {
		GnomeFontFacePrivate * priv;
		priv = face->private;
		if (priv->entry) {
			g_assert (priv->entry->face == face);
			priv->entry->face = NULL;
			gp_font_entry_unref (priv->entry);
			priv->entry = NULL;
		}
		if (priv->glyphs) g_free (priv->glyphs);
		if (priv->unimap) gp_uc_map_unref (priv->unimap);
		if (priv->privencoding) {
			g_hash_table_foreach_remove (priv->privencoding, gff_free_privencoding, NULL);
			g_hash_table_destroy (priv->privencoding);
		}
		if (priv->kerns) g_free (priv->kerns);
		if (priv->ligs) {
			gint i;
			for (i = 0; i < 256; i++) {
				if (priv->ligs[i]) g_free (priv->ligs[i]);
			}
			g_free (priv->ligs);
		}

		if (priv->loadedfont) gt1_unload_font (priv->loadedfont);
		g_free (priv);
		face->private = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
gnome_font_face_get_arg (GtkObject * object, GtkArg * arg, guint arg_id)
{
	GnomeFontFace * face;

	face = GNOME_FONT_FACE (object);

	switch (arg_id) {
	case ARG_ITALICANGLE:
		if (!GFF_METRICS (face)) {
			GTK_VALUE_DOUBLE (*arg) = 0.0;
		} else {
			GTK_VALUE_DOUBLE (*arg) = face->private->italics_angle;
		}
		break;
	case ARG_FONTBBOX:
		if (!GFF_METRICS (face)) {
			GTK_VALUE_BOXED (*arg) = NULL;
		} else {
			GTK_VALUE_BOXED (*arg) = &face->private->bbox;
		}
		break;
	case ARG_CAPHEIGHT:
		if (!GFF_METRICS (face)) {
			GTK_VALUE_DOUBLE (*arg) = 0.0;
		} else {
			GTK_VALUE_DOUBLE (*arg) = face->private->capheight;
		}
		break;
	case ARG_XHEIGHT:
		if (!GFF_METRICS (face)) {
			GTK_VALUE_DOUBLE (*arg) = 0.0;
		} else {
			GTK_VALUE_DOUBLE (*arg) = face->private->xheight;
		}
		break;
	case ARG_AFM:
		if ((face->private->entry->type == GP_FONT_ENTRY_TYPE1) ||
		    (face->private->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS)) {
			GTK_VALUE_STRING (*arg) = g_strdup (((GPFontEntryT1 *) face->private->entry)->afm.name);
		} else {
			GTK_VALUE_STRING (*arg) = NULL;
		}
		break;
	case ARG_PFB:
		if ((face->private->entry->type == GP_FONT_ENTRY_TYPE1) ||
		    (face->private->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS)) {
			GTK_VALUE_STRING (*arg) = g_strdup (((GPFontEntryT1 *) face->private->entry)->pfb.name);
		} else {
			GTK_VALUE_STRING (*arg) = NULL;
		}
		break;
	case ARG_PFBNAME:
		if (face->private->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS) {
			GTK_VALUE_STRING (*arg) = g_strdup (((GPFontEntryT1Alias *) face->private->entry)->alias);
		} else if (face->private->entry->type == GP_FONT_ENTRY_TYPE1) {
			GTK_VALUE_STRING (*arg) = g_strdup (face->private->entry->psname);
		} else {
			GTK_VALUE_STRING (*arg) = NULL;
		}
		break;
	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

/* fixme: */
/* return a pointer to the (PostScript) name of the font */
const gchar * gnome_font_unsized_get_glyph_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	if (face->private->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS) {
		return ((GPFontEntryT1Alias *) face->private->entry)->alias;
	} else {
		return face->private->entry->psname;
	}
}

GnomeFontFace *
gnome_font_face_new (const gchar * name)
{
	GPFontMap * map;
	GPFontEntry * e;

	g_return_val_if_fail (name != NULL, NULL);

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
}

/* 
 * Find the closest weight matching the family name, weight, and italic
 * specs. Return the unsized font.
 */

GnomeFontUnsized *
gnome_font_unsized_closest (const char *family_name,
			    GnomeFontWeight weight,
			    gboolean italic)
{
	GPFontMap * map;
	GPFontEntry * best, * entry;
	GnomeFontFace * face;
	int best_dist, dist;
	GSList * l;

	g_return_val_if_fail (family_name != NULL, NULL);

	/* This should be reimplemented to use the gnome_font_family_hash. */

	map = gp_fontmap_get ();

	best = NULL;
	best_dist = 1000000;
	face = NULL;

	for (l = map->fonts; l != NULL; l = l->next) {
		entry = (GPFontEntry *) l->data;
		if ((entry->type == GP_FONT_ENTRY_TYPE1) || (entry->type == GP_FONT_ENTRY_TYPE1_ALIAS)) {
			if (!strcasecmp (family_name, entry->familyname)) {
				GPFontEntryT1 * t1;
				t1 = (GPFontEntryT1 *) entry;
				dist = abs (weight - t1->Weight) + 100 * (italic != (t1->ItalicAngle != 0));
				/* Hack to prefer normal to narrow */
				if (strstr (entry->speciesname, "Narrow")) dist += 6;
				if (dist < best_dist) {
					best_dist = dist;
					best = entry;
				}
			}
		}
	}

	if (best) {
		face = gnome_font_face_new (best->name);
	} else {
		face = gnome_font_face_new ("Helvetica");
	}
	if (!face && map->fonts) {
		/* No face, no helvetica, load whatever font is first */
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

	g_return_val_if_fail (face != NULL, NULL);

	return face;
}

const gchar * gnome_font_face_get_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->private->entry->name;
}

const gchar * gnome_font_face_get_family_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->private->entry->familyname;
}

const gchar * gnome_font_face_get_species_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->private->entry->speciesname;
}

/* return a pointer to the (PostScript) name of the font */

const gchar * gnome_font_face_get_ps_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->private->entry->psname;
}

ArtPoint *
gnome_font_face_get_glyph_stdadvance (const GnomeFontFace * face, gint glyph, ArtPoint * advance)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (advance != NULL, NULL);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->private->num_glyphs)) glyph = 0;

	*advance = face->private->glyphs[glyph].advance;

	return advance;
}

ArtDRect *
gnome_font_face_get_glyph_stdbbox (const GnomeFontFace * face, gint glyph, ArtDRect * bbox)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (bbox != NULL, NULL);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->private->num_glyphs)) glyph = 0;

	*bbox = face->private->glyphs[glyph].bbox;

	return bbox;
}

const ArtBpath *
gnome_font_face_get_glyph_stdoutline (const GnomeFontFace * face, gint glyph)
{
	GFFGlyphInfo * info;
	Gt1GlyphOutline * gt1gol;
	static ArtBpath empty = {ART_END};

	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return NULL;
	}
	if (!GFF_LOADEDFONT (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load font", __FILE__, __LINE__, face->private->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->private->num_glyphs)) glyph = 0;

	info = face->private->glyphs + glyph;

	if (info->bpath) return info->bpath;

	gt1gol = gt1_glyph_outline_lookup (face->private->loadedfont, info->unicode);

	if (gt1gol == NULL) {
		info->bpath = face->private->glyphs[0].bpath;
	} else if (gt1gol->bpath == NULL) {
		info->bpath = &empty;
	} else {
		info->bpath = gt1gol->bpath;
	}

	return info->bpath;
}

GnomeFont *
gnome_font_face_get_font (const GnomeFontFace * face, gdouble size, gdouble xres, gdouble yres)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return gnome_font_new (face->private->entry->name, size);
}

GnomeFont *
gnome_font_face_get_font_default (const GnomeFontFace * face, gdouble size)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return gnome_font_face_get_font (face, size, 600.0, 600.0);
}

gdouble
gnome_font_face_get_ascender (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return 0.0;
	}

	return face->private->ascender;
}

gdouble
gnome_font_face_get_descender (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return 0.0;
	}

	return face->private->descender;
}

gdouble
gnome_font_face_get_underline_position (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return 0.0;
	}

	return face->private->underline_position;
}

gdouble
gnome_font_face_get_underline_thickness (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return 0.0;
	}

	return face->private->underline_thickness;
}

GnomeFontWeight
gnome_font_face_get_weight_code (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0);

	return face->private->weight_code;
}

gboolean
gnome_font_face_is_italic (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), FALSE);

	return face->private->italic;
}

gboolean
gnome_font_face_is_fixed_width (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), FALSE);

	return face->private->fixed_width;
}



/* fixme: */
/* Returns the glyph width in 0.001 unit */

gdouble
gnome_font_face_get_glyph_width (const GnomeFontFace * face, gint glyph)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return 0.0;
	}

	if ((glyph < 0) || (glyph >= face->private->num_glyphs)) glyph = 0;

	return face->private->glyphs[glyph].advance.x;
}

const gchar *
gnome_font_face_get_sample (const GnomeFontFace * face)
{
	GnomeFontFacePrivate * priv;
	GPUCMap * unimap;
	static gchar s[256];
	gchar *p;
	gint start, i;

	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return NULL;
	}

	/* This is slow and experimental */

	priv = face->private;
	unimap = priv->unimap;

	start = 0;

	/* Test Greek */
	if (unimap->entry[GP_CB_GREEK] && unimap->entry[GP_CB_GREEK]->mapped > 32) {
		/* Greek */
		start = 0x0391;
	} else if (unimap->entry[GP_CB_CYRILLIC] && unimap->entry[GP_CB_CYRILLIC]->mapped > 32) {
		start = 0x0410;
	}

	if (start > 0) {
		p = s;
		for (i = 0; i < 16; i++ ) {
			p += g_unichar_to_utf8 (start + 32 + i, p);
			p += g_unichar_to_utf8 (start + i, p);
		}
		*p = '\0';
		return s;
	}

	if (unimap->entry[GP_CB_BASIC_LATIN] && unimap->entry[GP_CB_BASIC_LATIN]->mapped > 32) {
		return _("The quick brown fox jumps over the lazy dog.");
	}

	p = s;
	for (i = 1; (i < 33) && (i < face->private->num_glyphs); i++ ) {
		p += g_unichar_to_utf8 (face->private->glyphs[i].unicode, p);
	}
	*p = '\0';
	return s;
}

/*
 * Get the glyph number corresponding to a given unicode, or -1 if it
 * is not mapped.
 *
 * fixme: We use ugly hack to avoid segfaults everywhere
 */

gint
gnome_font_face_lookup_default (const GnomeFontUnsized * face, gint unicode)
{
	const GPCharBlock * cb;

	g_return_val_if_fail (face != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return -1;
	}

	/* fixme: Nobody should ask mapping of 0 */
	if (unicode < 1) return 0;

	cb = gp_unicode_get_char_block (unicode);
	g_return_val_if_fail (cb != NULL, -1);

	return gp_uc_map_lookup (face->private->unimap, unicode);
}

/*
 * Returns number of defined glyphs in typeface
 */

gint
gnome_font_face_get_num_glyphs (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return 0;
	}

	return face->private->num_glyphs;
}

/*
 * Returns PostScript name for glyph
 */

const gchar *
gnome_font_face_get_glyph_ps_name (const GnomeFontFace * face, gint glyph)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->private->num_glyphs)) glyph = 0;

	return face->private->glyphs[glyph].psname;
}

static int
read_int32_lsb (const char *p)
{
  const unsigned char *q = (unsigned char *)p;

  return q[0] + (q[1] << 8) + (q[2] << 16) + (q[3] << 24);
}

/* this is actually the same as a pfb to pfa converter

 Reference: Adobe technical note 5040, "Supporting Downloadable PostScript
 Language Fonts", page 9 */
static char *
pfb_to_flat (const char *input, int input_size)
{
	const unsigned char *in = (unsigned char *)input;
  char *flat;
  int flat_size, flat_size_max;
  int in_idx;
  int length;
  int i;
  const char hextab[16] = "0123456789abcdef";

  flat_size = 0;
  flat_size_max = 32768;
  flat = g_new (char, flat_size_max);

  for (in_idx = 0; in_idx < input_size;)
    {
      if (in[in_idx] != 128)
	{
	  g_free (flat);
	  return NULL;
	}
      switch (in[in_idx + 1])
	{
	case 1:
	  length = read_int32_lsb (input + in_idx + 2);
	  if (flat_size + length > flat_size_max)
	    {
	      do
		flat_size_max <<= 1;
	      while (flat_size + length > flat_size_max);
	      flat = g_realloc (flat, flat_size_max);
	    }
	  in_idx += 6;
	  memcpy (flat + flat_size, in + in_idx, length);
	  flat_size += length;
	  in_idx += length;
	  break;
	case 2:
	  length = read_int32_lsb (input + in_idx + 2);
	  if (flat_size + length * 3 > flat_size_max)
	    {
	      do
		flat_size_max <<= 1;
	      while (flat_size + length * 3 > flat_size_max);
	      flat = g_realloc (flat, flat_size_max);
	    }
	  in_idx += 6;
	  for (i = 0; i < length; i++)
	    {
	      flat[flat_size++] = hextab[in[in_idx] >> 4];
	      flat[flat_size++] = hextab[in[in_idx] & 15];
	      in_idx++;
	      if ((i & 31) == 31 || i == length - 1)
		flat[flat_size++] = '\n';
	    }
	  break;
	case 3:
	  /* zero terminate the returned string */
	  if (flat_size == flat_size_max)
	    flat = g_realloc (flat, flat_size_max <<= 1);
	  flat[flat_size] = 0;
	  return flat;
	default:
	    g_free (flat);
	    return NULL;
	}
    }
  return flat;
}

gchar *
gnome_font_face_get_pfa (const GnomeFontUnsized *font)
{
	GPFontEntryT1 * t1;
	const char *pfb_fn;
	FILE *f;
	char *pfb;
	int pfb_size, pfb_size_max;
	int bytes_read;

	char *flat = NULL;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (font), NULL);

	t1 = (GPFontEntryT1 *) font->private->entry;
	pfb_fn = t1->pfb.name;
	f = fopen (pfb_fn, "r");
	if (f != NULL) {
		pfb_size = 0;
		pfb_size_max = 32768;
		pfb = g_new (char, pfb_size_max);
		while (1) {
			bytes_read = fread (pfb + pfb_size, 1, pfb_size_max - pfb_size, f);
			if (bytes_read == 0) break;
			pfb_size += bytes_read;
			pfb = g_realloc (pfb, pfb_size_max <<= 1);
		}

		if (pfb_size) {
			if (((unsigned char *)pfb)[0] == 128) {
				flat = pfb_to_flat (pfb, pfb_size);
			} else {
				flat = g_new (char, pfb_size + 1);
				memcpy (flat, pfb, pfb_size);
				flat[pfb_size] = 0;
			}
		} else {
			flat = NULL;
		}
		g_free (pfb);
		fclose (f);
	} else {
		gchar *privname;
		/* Encode empty font */
		g_warning (_("Couldn't generate pfa for face %s\n"), pfb_fn);
		if (font->private->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS) {
			privname = ((GPFontEntryT1Alias *) font->private->entry)->alias;
		} else {
			privname = font->private->entry->psname;
		}
		flat = g_strdup_printf ("%%Empty font generated by gnome-print\n"
					"8 dict begin"
					"/FontType 3 def\n"
					"/FontMatrix [.001 0 0 .001 0 0] def\n"
					"/FontBBox [0 0 750 950] def\n"
					"/Encoding 256 array def\n"
					"0 1 255 {Encoding exch /.notdef put} for\n"
					"/CharProcs 2 dict def\n"
					"CharProcs begin\n"
					"/.notdef {\n"
					"0 0 moveto 750 0 lineto 750 950 lineto 0 950 lineto closepath\n"
					"50 50 moveto 700 50 lineto 700 900 lineto 50 900 lineto closepath\n"
					"eofill\n"
					"} bind def\n"
					"end\n"
					"/BuildGlyph {\n"
					"1000 0 0 0 750 950 setcachedevice\n"
					"exch /CharProcs get exch\n"
					"2 copy known not {pop /.notdef} if\n"
					"get exec\n"
					"} bind def\n"
					"/BuildChar {1 index /Encoding get exch get\n"
					"1 index /BuildGlyph get exec\n"
					"} bind def\n"
					"currentdict\n"
					"end\n"
					"/%s exch definefont pop\n",
					privname);
	}

	return flat;
}

/*
 * Creates new face and creates link with FontEntry
 */

static void
gff_face_from_entry (GPFontEntry * e)
{
	GnomeFontFace * face;
	GPFontEntryT1 * t1;

	g_return_if_fail (e->face == NULL);
	g_return_if_fail ((e->type == GP_FONT_ENTRY_TYPE1) || (e->type == GP_FONT_ENTRY_TYPE1_ALIAS));

	t1 = (GPFontEntryT1 *) e;

	face = gtk_type_new (GNOME_TYPE_FONT_FACE);

	gp_font_entry_ref (e);
	face->private->entry = e;
	e->face = face;

	face->private->weight_code = t1->Weight;
	face->private->italic = (t1->ItalicAngle < 0);

	face->private->kerns = NULL;
	face->private->num_kerns = 0;
	face->private->ligs = NULL; /* one liglist for each glyph */

	face->private->loadedfont = NULL;
}

static gboolean
gnome_font_face_gt1_load (GnomeFontFace * face)
{
	GPFontEntryT1 * t1;

	g_return_val_if_fail ((face->private->entry->type == GP_FONT_ENTRY_TYPE1) ||
			      (face->private->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS),
			      FALSE);

	t1 = (GPFontEntryT1 *) face->private->entry;

	if (!face->private->loadedfont) {
		g_return_val_if_fail (t1->pfb.name != NULL, FALSE);
		face->private->loadedfont = gt1_load_font (t1->pfb.name, face->private->privencoding);
		if (!face->private->loadedfont) {
			face->private->num_glyphs = 1;
		}
	}

	return TRUE;
}

#define KERN_PAIR_HASH(g1, g2) ((g1) * 367 + (g2) * 31)

static gboolean
gff_load_afm (GnomeFontFace * face)
{
	GnomeFontFacePrivate * priv;
	GPFontEntryT1 * t1;
	Font_Info *fi;
	FILE *afm_f;
	int status;
	gint nglyphs;
	gint privcode;
	gint ret = FALSE;

	g_return_val_if_fail ((face->private->entry->type == GP_FONT_ENTRY_TYPE1) ||
			      (face->private->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS),
			      FALSE);

	priv = face->private;
	t1 = (GPFontEntryT1 *) face->private->entry;

	afm_f = fopen (t1->afm.name, "r");
	if (afm_f != NULL) {
		status = parseFile (afm_f, &fi, P_G | P_M | P_P);
#ifdef VERBOSE
		debugmsg (_("status loading %s = %d\n"), t1->afm.name, status);
#endif
		if (status == 0) {
			GnomeFontLigList **ligtab;
			Ligature *ligs;
			int i;
			GnomeFontKernPair *ktab;
			int ktabsize;

			priv->ascender = fi->gfi->ascender;
			/* This is negated because adobe fonts use a negative number
			   for descenders below the baseline, but most people expect
			   a positive number for descenders below the line. */
			priv->descender = -fi->gfi->descender;
			priv->fixed_width = fi->gfi->isFixedPitch;

			/* Added for pdf */
			priv->capheight = (gdouble) fi->gfi->capHeight;
			priv->italics_angle = (gdouble) fi->gfi->italicAngle;
			priv->xheight = (gdouble) fi->gfi->xHeight;
			priv->bbox.x0 = (gdouble) fi->gfi->fontBBox.llx;
			priv->bbox.y0 = (gdouble) fi->gfi->fontBBox.lly;
			priv->bbox.x1 = (gdouble) fi->gfi->fontBBox.urx;
			priv->bbox.y1 = (gdouble) fi->gfi->fontBBox.ury;
			/* */
	  
			priv->underline_position = fi->gfi->underlinePosition;
			priv->underline_thickness = fi->gfi->underlineThickness;

			ligtab = g_malloc (256 * sizeof (GnomeFontLigList *));
			priv->ligs = ligtab;

			for (i = 0; i < 256; i++) {
				ligtab[i] = 0;
			}

			priv->glyphs = g_new0 (GFFGlyphInfo, MAX (fi->numOfChars + 1, 1));
			priv->unimap = gp_uc_map_new ();

			gff_fill_zero_glyph (face);

			nglyphs = 1;
			privcode = 0xe000;

			for (i = 0; i < fi->numOfChars; i++) {
				CharMetricInfo * info;
				gint unicode;
				gchar * constname;

				constname = NULL; /* Kill warning */
				info = fi->cmi + i;

				unicode = gp_unicode_from_ps (info->name);
				if (unicode < 1) unicode = gp_unicode_from_dingbats (info->name);

				if (unicode < 1) {
					if (!priv->privencoding) priv->privencoding = g_hash_table_new (g_str_hash, g_str_equal);
					if (!g_hash_table_lookup (priv->privencoding, info->name)) {
						unicode = privcode++;
						constname = g_strdup (info->name);
						g_hash_table_insert (priv->privencoding, constname, GINT_TO_POINTER (unicode));
					}
				} else {
					constname = (gchar *) gp_const_ps_from_ps (info->name);
				}

				if (unicode) {
					GFFGlyphInfo * gi;

					gi = priv->glyphs + nglyphs;

					gi->unicode = unicode;
					gi->psname = constname;

					gi->advance.x = info->wx;
					gi->advance.y = info->wy;

					gi->bbox.x0 = info->charBBox.llx;
					gi->bbox.y0 = info->charBBox.lly;
					gi->bbox.x1 = info->charBBox.urx;
					gi->bbox.y1 = info->charBBox.ury;

					if (gp_multi_from_ps (info->name)) {
						const GSList * l;
						l = gp_multi_from_ps (info->name);
						while (l) {
							gp_uc_map_insert (priv->unimap, GPOINTER_TO_INT (l->data), nglyphs);
							l = l->next;
						}
					} else {
						gp_uc_map_insert (priv->unimap, unicode, nglyphs);
					}

					nglyphs++;
				}
			}

			priv->glyphs = g_renew (GFFGlyphInfo, priv->glyphs, nglyphs);
			priv->num_glyphs = nglyphs;

			for (i = 0; i < fi->numOfChars; i++) {
				int code;

				/* Get the width */
				code = fi->cmi[i].code;
				if (code >= 0 && code < 256) {
					/* Get the ligature info */
					for (ligs = fi->cmi[i].ligs; ligs != NULL; ligs = ligs->next) {
						int succ, lig;
						GnomeFontLigList *ll;
						gint u;

						/* We assume here that dingbats do not have ligatures */
						u = gp_unicode_from_ps (ligs->succ);
						succ = gnome_font_face_lookup_default (face, u);
						u = gp_unicode_from_ps (ligs->succ);
						lig = gnome_font_face_lookup_default (face, u);
						if ((succ > 0) && (lig > 0)) {
							ll = g_new (GnomeFontLigList, 1);
							ll->succ = succ;
							ll->lig = lig;
							ll->next = ligtab[code];
							ligtab[code] = ll;
						}
					}
				}
			}

			/* process the kern pairs */
			for (ktabsize = 1; ktabsize < fi->numOfPairs << 1; ktabsize <<= 1);
			ktab = g_new (GnomeFontKernPair, ktabsize);
			face->private->kerns = ktab;
			face->private->num_kerns = ktabsize;
			for (i = 0; i < ktabsize; i++) {
				ktab[i].glyph1 = -1;
				ktab[i].glyph2 = -1;
				ktab[i].x_amt = 0;
			}

			for (i = 0; i < fi->numOfPairs; i++) {
				int glyph1, glyph2;
				int j;
				gint u;

				/* We assume here that dingbats do not have kerning */
				u = gp_unicode_from_ps (fi->pkd[i].name1);
				glyph1 = gnome_font_face_lookup_default (face, u);
				u = gp_unicode_from_ps (fi->pkd[i].name2);
				glyph2 = gnome_font_face_lookup_default (face, u);

				for (j = KERN_PAIR_HASH (glyph1, glyph2) & (ktabsize - 1);
				     ktab[j].glyph1 != -1;
				     j = (j + 1) & (ktabsize - 1));
				ktab[j].glyph1 = glyph1;
				ktab[j].glyph2 = glyph2;
				ktab[j].x_amt = fi->pkd[i].xamt;
#ifdef VERBOSE
				debugmsg ("kern pair %s(%d) %s(%d) %d\n",
					  fi->pkd[i].name1, glyph1,
					  fi->pkd[i].name2, glyph2,
					  fi->pkd[i].xamt);
#endif
			}

#ifdef VERBOSE
			for (i = 0; i < ktabsize; i++)
				debugmsg ("%d %d %d\n",
					  ktab[i].glyph1, ktab[i].glyph2, ktab[i].x_amt);
#endif

			ret = TRUE;
		}

		if (fi) parseFileFree (fi);
		fclose (afm_f);
	}

	if (!ret) {
		priv->glyphs = g_new0 (GFFGlyphInfo, 1);
		priv->num_glyphs = 1;
		priv->unimap = gp_uc_map_new ();
		gff_fill_zero_glyph (face);
	}

	return TRUE;
}

static void
gff_fill_zero_glyph (GnomeFontFace * face)
{
	GFFGlyphInfo * info;
	static ArtBpath undef[] = {{ART_MOVETO, 0.0, 0.0, 0.0, 0.0, 50.0, 50.0},
				   {ART_LINETO, 0.0, 0.0, 0.0, 0.0, 750.0, 50.0},
				   {ART_LINETO, 0.0, 0.0, 0.0, 0.0, 750.0, 950.0},
				   {ART_LINETO, 0.0, 0.0, 0.0, 0.0, 50.0, 950.0},
				   {ART_LINETO, 0.0, 0.0, 0.0, 0.0, 50.0, 50.0},
				   {ART_MOVETO, 0.0, 0.0, 0.0, 0.0, 100.0, 100.0},
				   {ART_LINETO, 0.0, 0.0, 0.0, 0.0, 700.0, 100.0},
				   {ART_LINETO, 0.0, 0.0, 0.0, 0.0, 700.0, 900.0},
				   {ART_LINETO, 0.0, 0.0, 0.0, 0.0, 100.0, 900.0},
				   {ART_LINETO, 0.0, 0.0, 0.0, 0.0, 100.0, 100.0},
				   {ART_END, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};

	info = face->private->glyphs;

	info->unicode = 0;
	info->psname = ".notdef";

	info->advance.x = 800.0;
	info->advance.y = 0.0;

	info->bbox.x0 = 50.0;
	info->bbox.y0 = 50.0;
	info->bbox.x1 = 750.0;
	info->bbox.y1 = 900.0;

	info->bpath = undef;
}


const ArtDRect *
gnome_font_face_get_stdbbox (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	if (!GFF_METRICS (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->private->entry->name);
		return NULL;
	}

	return &face->private->bbox;
}
