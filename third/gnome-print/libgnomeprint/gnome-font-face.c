#define __GNOME_FONT_FACE_C__

#include <config.h>
#include <stdio.h>
#include <gnome.h>
#include <libgnomeprint/gnome-print-i18n.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gp-ps-unicode.h>
#include "parseAFM.h"

#if defined(LIBXML_VERSION) && LIBXML_VERSION >= 20000
#include <libxml/parser.h>
#include <libxml/parser.h>
#define root children
#define childs children
#else
#include <gnome-xml/parser.h>
#include <gnome-xml/xmlmemory.h>
#endif

static void gnome_font_face_class_init (GnomeFontFaceClass * klass);
static void gnome_font_face_init (GnomeFontFace * face);

static void gnome_font_face_destroy (GtkObject * object);

static void gff_refresh_fontmap (void);
static gboolean gnome_font_face_gt1_load (GnomeFontFace * face);
static gboolean gff_load_afm (GnomeFontFace * face);
static void gff_fill_zero_glyph (GnomeFontFace * face);

#define GFF_LOADEDFONT(f) ((f)->private->loadedfont || gnome_font_face_gt1_load ((GnomeFontFace *) f))
#define GFF_METRICS(f) ((f)->private->glyphs || gff_load_afm ((GnomeFontFace *) f))

static GHashTable * fontmap = NULL;
static GList * fontlist = NULL;

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

	object_class->destroy = gnome_font_face_destroy;
}

static void
gnome_font_face_init (GnomeFontFace * face)
{
	face->private = g_new0 (GnomeFontFacePrivate, 1);
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

	g_warning ("Destroying typefaces shouldn't occur");

	face = (GnomeFontFace *) object;

	if (face->private) {
		GnomeFontFacePrivate * priv;
		priv = face->private;
		if (priv->font_name) g_free (priv->font_name);
		if (priv->afm_fn) g_free (priv->afm_fn);
		if (priv->pfb_fn) g_free (priv->pfb_fn);
		if (priv->fullname) g_free (priv->fullname);
		if (priv->familyname) g_free (priv->familyname);
		if (priv->weight) g_free (priv->weight);
		if (priv->alias) g_free (priv->alias);
		if (priv->glyphs) g_free (priv->glyphs);
		if (priv->glyphmap) g_hash_table_destroy (priv->glyphmap);
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
		if (priv->cov_pages) {
			gint i;
			for (i = 0; i < priv->num_cov_pages; i++) {
				if (priv->cov_pages[i]) g_free (priv->cov_pages[i]);
			}
			g_free (priv->cov_pages);
		}
		if (priv->loadedfont) gt1_unload_font (priv->loadedfont);
		g_free (priv);
		face->private = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

GnomeFontFace *
gnome_font_face_new (const gchar * name)
{
	GnomeFontFace * face;

	g_return_val_if_fail (name != NULL, NULL);

	if (!fontmap) gff_refresh_fontmap ();
	g_return_val_if_fail (fontmap != NULL, NULL);

	face = g_hash_table_lookup (fontmap, name);
	g_return_val_if_fail (face != NULL, NULL);

	gnome_font_face_ref (face);

	return face;
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
	GnomeFontFace * face, * best;
	int best_dist, dist;
	GList * l;

	g_return_val_if_fail (family_name != NULL, NULL);

	if (!fontmap) gff_refresh_fontmap ();
	g_return_val_if_fail (fontmap != NULL, NULL);
	g_return_val_if_fail (fontlist != NULL, NULL);

	/* This should be reimplemented to use the gnome_font_family_hash. */

	best = NULL;
	best_dist = 1000000;

	for (l = fontlist; l != NULL; l = l->next) {
		face = (GnomeFontFace *) l->data;
		if (!strcmp (family_name, face->private->familyname)) {
			dist = abs (weight - face->private->weight_code) +
				100 * (italic != face->private->italic);
			if (dist < best_dist) {
				best_dist = dist;
				best = face;
			}
		}
	}

	if (!best) {
		best = g_hash_table_lookup (fontmap, "Helvetica");
	}

	if (!best) {
		best = GNOME_FONT_FACE (g_list_last (fontlist)->data);
	}

	gnome_font_face_ref (best);

	return best;
}

const gchar * gnome_font_face_get_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return gnome_font_face_get_ps_name (face);
}

const gchar * gnome_font_face_get_family_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->private->familyname;
}

const gchar * gnome_font_face_get_species_name (const GnomeFontFace * face)
{
	gchar * stylename;

	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	stylename = g_strchug (face->private->fullname + strlen (face->private->familyname));
	if (*stylename == '-') stylename++;
	if (*stylename == '\0') stylename = "Normal";

	return stylename;
}

/* return a pointer to the (PostScript) name of the font */

const gchar * gnome_font_face_get_ps_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->private->font_name;
}

/* fixme: */
/* return a pointer to the (PostScript) name of the font */

const gchar * gnome_font_unsized_get_glyph_name (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	if (face->private->alias)
		return face->private->alias;
	else
		return face->private->font_name;
}

ArtPoint *
gnome_font_face_get_glyph_stdadvance (const GnomeFontFace * face, gint glyph, ArtPoint * advance)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (advance != NULL, NULL);
	g_return_val_if_fail (GFF_METRICS (face), NULL);

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
	g_return_val_if_fail (GFF_METRICS (face), NULL);

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
	g_return_val_if_fail (GFF_METRICS (face), NULL);
	g_return_val_if_fail (GFF_LOADEDFONT (face), NULL);

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

	return gnome_font_new (face->private->font_name, size);
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
	g_return_val_if_fail (GFF_METRICS (face), 0.0);

	return face->private->ascender;
}

gdouble
gnome_font_face_get_descender (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	g_return_val_if_fail (GFF_METRICS (face), 0.0);

	return face->private->descender;
}

gdouble
gnome_font_face_get_underline_position (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	g_return_val_if_fail (GFF_METRICS (face), 0.0);

	return face->private->underline_position;
}

gdouble
gnome_font_face_get_underline_thickness (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	g_return_val_if_fail (GFF_METRICS (face), 0.0);

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
	g_return_val_if_fail (GFF_METRICS (face), 0.0);

	if ((glyph < 0) || (glyph >= face->private->num_glyphs)) glyph = 0;

	return face->private->glyphs[glyph].advance.x;
}

const gchar *
gnome_font_face_get_sample (const GnomeFontFace * face)
{
	GnomeFontFacePrivate * priv;
	static gchar s[256];
	gchar *p;
	gint i;

	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (GFF_METRICS (face), NULL);

	/* This is slow and experimental */

	priv = face->private;

	if (!g_hash_table_lookup (priv->glyphmap, GINT_TO_POINTER ('a')) ||
	    !g_hash_table_lookup (priv->glyphmap, GINT_TO_POINTER ('e')) ||
	    !g_hash_table_lookup (priv->glyphmap, GINT_TO_POINTER ('q'))) {
		/* We do not have a, e and q - test for greek letters */
		if (g_hash_table_lookup (priv->glyphmap, GINT_TO_POINTER (0x0391)) &&
		    g_hash_table_lookup (priv->glyphmap, GINT_TO_POINTER (0x0395)) &&
		    g_hash_table_lookup (priv->glyphmap, GINT_TO_POINTER (0x03a8))) {
			p = s;
			for (i = 0; i < 16; i++ ) {
				p += g_unichar_to_utf8 (0x0391 + i, p);
				p += g_unichar_to_utf8 (0x03b1 + i, p);
			}
			*p = '\0';
			return s;
		} else {
			/* Nope - we do not know coverage for now */
			p = s;
			for (i = 1; (i < 33) && (i < face->private->num_glyphs); i++ ) {
				p += g_unichar_to_utf8 (face->private->glyphs[i].unicode, p);
			}
			*p = '\0';
			return s;
		}
	} else {
		/* We have latin encoding */
		return _("The quick brown fox jumps over the lazy dog.");
	}
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
	g_return_val_if_fail (face != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);
	g_return_val_if_fail (GFF_METRICS (face), -1);

	return GPOINTER_TO_INT (g_hash_table_lookup (face->private->glyphmap, GINT_TO_POINTER (unicode)));
}

/*
 * Returns number of defined glyphs in typeface
 */

gint
gnome_font_face_get_num_glyphs (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0);
	g_return_val_if_fail (GFF_METRICS (face), 0);

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
	g_return_val_if_fail (GFF_METRICS (face), NULL);

	if ((glyph < 0) || (glyph >= face->private->num_glyphs)) glyph = 0;

	return face->private->glyphs[glyph].psname;
}

/* Return a list of fonts, as a g_list of strings */

GList *
gnome_font_list ()
{
	static GList * the_list = NULL;

	if (!fontmap) gff_refresh_fontmap ();

	if (!the_list) {
		GList * l;
		for (l = fontlist; l != NULL; l = l->next) {
			GnomeFontFace * face;
			face = (GnomeFontFace *) l->data;
			the_list = g_list_prepend (the_list, face->private->font_name);
		}
		the_list = g_list_reverse (the_list);
	}

	return the_list;
}

void
gnome_font_list_free (GList * fontlist)
{
}

/* These two should probably go into the class */

static GHashTable *gnome_font_family_hash = NULL;
static GList *gnome_font_family_the_list = NULL;

/* Return a list of font families, as a g_list of newly allocated strings */

GList *
gnome_font_family_list ()
{
  GList *list, *l;
  GHashTable *hash;
  GList *the_family;

  if (gnome_font_family_the_list != NULL) return gnome_font_family_the_list;

  if (!fontmap) gff_refresh_fontmap ();

  list = NULL;
  hash = g_hash_table_new (g_str_hash, g_str_equal);

  for (l = fontlist; l != NULL; l = l->next)
    {
	    GnomeFontFace * face = (GnomeFontFace *) l->data;
	    the_family = g_hash_table_lookup (hash, face->private->familyname);
	    if (the_family == NULL)
	    {
		    the_family = g_list_prepend (NULL, face);
		    g_hash_table_insert (hash, face->private->familyname, the_family);
		    list = g_list_append (list, face->private->familyname);
	    }
	    else
		    g_list_append (the_family, face);
    }

  gnome_font_family_the_list = list;
  gnome_font_family_hash = hash;

  return list;
}

void
gnome_font_family_list_free (GList *fontlist)
{
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
  const char *pfb_fn;
  FILE *f;
#if 0
  const GnomeFontMap *subst;
#endif

  char *pfb;
  int pfb_size, pfb_size_max;
  int bytes_read;

  char *flat;

  if (font == NULL)
    return NULL;

  pfb_fn = font->private->pfb_fn;
#if 0
  if (!strcmp (pfb_fn, "-"))
    {
      subst = font->subst_glyph;
      g_return_val_if_fail (subst != NULL, NULL);

      pfb_fn = subst->pfb_fn;
    }
#endif
  f = fopen (pfb_fn, "r");
  if (f == NULL)
    {
      g_warning (_("Couldn't open font file %s\n"), pfb_fn);
      return NULL;
    }
  
  pfb_size = 0;
  pfb_size_max = 32768;
  pfb = g_new (char, pfb_size_max);
  while (1)
    {
      bytes_read = fread (pfb + pfb_size, 1, pfb_size_max - pfb_size, f);
      if (bytes_read == 0) break;
      pfb_size += bytes_read;
      pfb = g_realloc (pfb, pfb_size_max <<= 1);
    }

  if (pfb_size)
    {
      if (((unsigned char *)pfb)[0] == 128)
	flat = pfb_to_flat (pfb, pfb_size);
      else
	{
	  flat = g_new (char, pfb_size + 1);
	  memcpy (flat, pfb, pfb_size);
	  flat[pfb_size] = 0;
	}
    }
  else
    flat = NULL;

  g_free (pfb);
  return flat;
}

static void
gff_add_mapping (const char *fontname,
		 const char *afm_fn, const char *pfb_fn,
		 const char *fullname, const char *familyname,
		 const char *weight, const char *alias)
{
	static GHashTable * weights = NULL;
	int len_full;
	GnomeFontWeight weight_code;
	gchar w[32];
	GnomeFontFace * face;

	if (!weights) {
		weights = g_hash_table_new (g_str_hash, g_str_equal);

		g_hash_table_insert (weights, "Extra Light", GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));
		g_hash_table_insert (weights, "Extralight", GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));

		g_hash_table_insert (weights, "Thin", GINT_TO_POINTER (GNOME_FONT_THIN));

		g_hash_table_insert (weights, "Light", GINT_TO_POINTER (GNOME_FONT_LIGHT));

		g_hash_table_insert (weights, "Book", GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (weights, "Roman", GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (weights, "Regular", GINT_TO_POINTER (GNOME_FONT_BOOK));

		g_hash_table_insert (weights, "Medium", GINT_TO_POINTER (GNOME_FONT_MEDIUM));

		g_hash_table_insert (weights, "Semi", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Semibold", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Demi", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Demibold", GINT_TO_POINTER (GNOME_FONT_SEMI));

		g_hash_table_insert (weights, "Bold", GINT_TO_POINTER (GNOME_FONT_BOLD));

		g_hash_table_insert (weights, "Heavy", GINT_TO_POINTER (GNOME_FONT_HEAVY));
 
		g_hash_table_insert (weights, "Extra", GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));
		g_hash_table_insert (weights, "Extra Bold", GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));

		g_hash_table_insert (weights, "Black", GINT_TO_POINTER (GNOME_FONT_BLACK));

		g_hash_table_insert (weights, "Extra Black", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
		g_hash_table_insert (weights, "Extrablack", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
		g_hash_table_insert (weights, "Ultra Bold", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
	};

	if (!fontmap) fontmap = g_hash_table_new (g_str_hash, g_str_equal);

	face = g_hash_table_lookup (fontmap, fontname);
	if (face) return;

	face = gtk_type_new (GNOME_TYPE_FONT_FACE);

	face->private->font_name = g_strdup (fontname);
	face->private->afm_fn = g_strdup (afm_fn);
	face->private->pfb_fn = g_strdup (pfb_fn);
	face->private->fullname = g_strdup (fullname);
	face->private->familyname = g_strdup (familyname);
	face->private->weight = g_strdup (weight);

	if ( alias )
		face->private->alias = g_strdup( alias );
	else
		face->private->alias = NULL;

	face->private->cov_pages = NULL;

	strncpy(w, weight, 31);
	w[31] = '\0';
	weight_code = GPOINTER_TO_INT (g_hash_table_lookup (weights, w));

	face->private->weight_code = weight_code;

	len_full = strlen (fullname);

	face->private->italic =
		(len_full >= 7 && !g_strcasecmp (fullname + len_full - 7, "Oblique")) ||
		(len_full >= 6 && !g_strcasecmp (fullname + len_full - 6, "Italic"));

	face->private->kerns = NULL;
	face->private->num_kerns = 0;
	face->private->ligs = NULL; /* one liglist for each glyph */

	face->private->first_cov_page = 0;
	face->private->num_cov_pages = 0;

	face->private->loadedfont = NULL;

	g_hash_table_insert (fontmap, face->private->font_name, face);

	fontlist = g_list_prepend (fontlist, face);
}

/*
 * Get a value for a node either carried as an attibute or as
 * the content of a child.
 */
static char *
xmlGetValue (xmlNodePtr node, const char *name)
{
	char *ret;
	xmlNodePtr child;

	ret = (char *) xmlGetProp (node, name);
	if (ret != NULL) {
		char *gret;
		gret = g_strdup (ret);
		xmlFree (ret);
		return gret;
	}
	child = node->childs;

	while (child != NULL) {
		if (!strcmp (child->name, name)) {
		        /*
			 * !!! Inefficient, but ...
			 */
			ret = xmlNodeGetContent(child);
			if (ret != NULL)
			  {
			    char *ret2 = g_strdup(ret);
			    xmlFree(ret);
			    return (ret2);
			  }
		}
		child = child->next;
	}

	return NULL;
}

static void
gnome_font_load_fontmap (const char * fn)
{
  char *type;
  char *fontname, *afmfile, *pfbfile, *fullname, *familyname, *weight, *alias;
  xmlDoc *doc;

#ifdef VERBOSE
  debugmsg ("filename %s\n", fn);
#endif

#if defined(LIBXML_VERSION) && LIBXML_VERSION >= 20000
  xmlKeepBlanksDefault(0);
#endif
  doc = xmlParseFile( fn );
  if ( doc && doc->root && doc->root->name && ( ! strcmp( doc->root->name, "fontmap" ) ) ) /* && doc->root->ns && doc->root->ns->href && ( ! strcmp( doc->root->ns->href, "http://www.gnome.org/gnome-font/0.0" ) ) ) */
    {
      xmlNode *font = doc->root->childs;
      while(font)
	{
	  /* parse the fontmap line */
	  type = xmlGetValue( font, "format" );
	  if (type && !strcmp (type, "type1"))
	    {
              gboolean have_all_info = FALSE;

	      fontname = xmlGetValue( font, "name" );
	      afmfile = xmlGetValue( font, "metrics" );
	      pfbfile = xmlGetValue( font, "glyphs" );
	      fullname = xmlGetValue( font, "fullname" );
	      familyname = xmlGetValue( font, "familyname" );
	      weight = xmlGetValue( font, "weight" );
	      alias = xmlGetValue( font, "alias" );

              /* alias is the only optional one, AFAICT */
              have_all_info = fontname && afmfile && pfbfile && 
                fullname && familyname && weight;

              if (have_all_info)
                {
                  gff_add_mapping (fontname, afmfile, pfbfile,
                                          fullname, familyname, weight, alias);
                }
              else 
                {
                  g_warning("Missing data in font map `%s':\n"
                            "  Font name: %s\n"
                            "  Metrics:   %s\n"
                            "  Glyphs:    %s\n"
                            "  Full name: %s\n"
                            "  Family:    %s\n"
                            "  Weight:    %s\n",
                            fn,
                            fontname ? fontname : "**missing**",
                            afmfile ? afmfile : "**missing**",
                            pfbfile ? pfbfile : "**missing**",
                            fullname ? fullname : "**missing**",
                            familyname ? familyname : "**missing**",
                            weight ? weight : "**missing**");
                }
#ifdef VERBOSE
	      debugmsg ("%s %s %s %s %s %s\n",
			fontname, afmfile, pfbfile, fullname, familyname, weight);
#endif
	      g_free (alias);
	      g_free (weight);
	      g_free (familyname);
	      g_free (fullname);
	      g_free (pfbfile);
	      g_free (afmfile);
	      g_free (fontname);
	    }
	  g_free (type);
	  font = font->next;
	}
    }
  if (doc)
    xmlFreeDoc (doc);
}

static void
gff_refresh_fontmap ()
{
	char * fontmap_fn;
	char * env_home;

	/* build fontmap from scanning the font path */

	env_home = getenv ("HOME");
	g_return_if_fail (env_home != NULL);

	fontmap_fn = g_strconcat (env_home, "/.gnome/fonts/fontmap", NULL);

	if (g_file_exists (fontmap_fn)) {
		gnome_font_load_fontmap (fontmap_fn);
	}

	g_free (fontmap_fn);

	fontmap_fn = gnome_datadir_file ("fonts/fontmap");

	if (fontmap_fn && g_file_exists (fontmap_fn)) {
		gnome_font_load_fontmap (fontmap_fn);
		g_free (fontmap_fn);
	} else {
		/* Check gnome-print installation prefix */

		fontmap_fn = g_strconcat(DATADIR, "/fonts/fontmap", NULL);

		if (g_file_exists (fontmap_fn)) gnome_font_load_fontmap (fontmap_fn);
		g_free (fontmap_fn);
	}

	fontlist = g_list_reverse (fontlist);
}

static gboolean
gnome_font_face_gt1_load (GnomeFontFace * face)
{
	GnomeFontFacePrivate * priv;

	priv = face->private;

	if (!priv->loadedfont) {
		g_return_val_if_fail (priv->pfb_fn != NULL, FALSE);
		priv->loadedfont = gt1_load_font (priv->pfb_fn, priv->privencoding);
		g_return_val_if_fail (priv->loadedfont != NULL, FALSE);
	}

	return TRUE;
}

#define KERN_PAIR_HASH(g1, g2) ((g1) * 367 + (g2) * 31)

static gboolean
gff_load_afm (GnomeFontFace * face)
{
	GnomeFontFacePrivate * priv;
	Font_Info *fi;
	FILE *afm_f;
	int status;
	gint nglyphs;
	gint privcode;

	priv = face->private;

	afm_f = fopen (face->private->afm_fn, "r");
	if (afm_f != NULL) {
		status = parseFile (afm_f, &fi, P_G | P_M | P_P);
#ifdef VERBOSE
		debugmsg (_("status loading %s = %d\n"), fontmap_entry->private->afm_fn, status);
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

			priv->glyphs = g_new0 (GFFGlyphInfo, fi->numOfChars + 1);
			priv->glyphmap = g_hash_table_new (NULL, NULL);

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
							g_hash_table_insert (priv->glyphmap, l->data, GINT_TO_POINTER (nglyphs));
							l = l->next;
						}
					} else {
						g_hash_table_insert (priv->glyphmap, GINT_TO_POINTER (unicode), GINT_TO_POINTER (nglyphs));
					}

					nglyphs++;
				}
			}

			priv->glyphs = g_renew (GFFGlyphInfo, priv->glyphs, nglyphs);
			priv->num_glyphs = nglyphs;

	  for (i = 0; i < fi->numOfChars; i++)
	    {
	      int code;

	      /* Get the width */
	      code = fi->cmi[i].code;
	      if (code >= 0 && code < 256)
		{
		  /* Get the ligature info */
		  for (ligs = fi->cmi[i].ligs; ligs != NULL; ligs = ligs->next)
		    {
		      int succ, lig;
		      GnomeFontLigList *ll;
		      gint u;

		      /* We assume here that dingbats do not have ligatures */
		      u = gp_unicode_from_ps (ligs->succ);
		      succ = GPOINTER_TO_INT (g_hash_table_lookup (face->private->glyphmap, GINT_TO_POINTER (u)));
		      u = gp_unicode_from_ps (ligs->succ);
		      lig = GPOINTER_TO_INT (g_hash_table_lookup (face->private->glyphmap, GINT_TO_POINTER (u)));
		      if ((succ > 0) && (lig > 0))
			{
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
	  for (i = 0; i < ktabsize; i++)
	    {
	      ktab[i].glyph1 = -1;
	      ktab[i].glyph2 = -1;
	      ktab[i].x_amt = 0;
	    }

	  for (i = 0; i < fi->numOfPairs; i++)
	    {
	      int glyph1, glyph2;
	      int j;
	      gint u;

	      /* We assume here that dingbats do not have kerning */
	      u = gp_unicode_from_ps (fi->pkd[i].name1);
	      glyph1 = GPOINTER_TO_INT (g_hash_table_lookup (face->private->glyphmap, GINT_TO_POINTER (u)));
	      u = gp_unicode_from_ps (fi->pkd[i].name2);
	      glyph2 = GPOINTER_TO_INT (g_hash_table_lookup (face->private->glyphmap, GINT_TO_POINTER (u)));

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
	}

      if (fi)
	parseFileFree (fi);

      fclose (afm_f);

      return TRUE;
    }

  return FALSE;
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


double
gnome_font_face_get_capheight (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	g_return_val_if_fail (GFF_METRICS (face), 0.0);

	return face->private->capheight;
}

double
gnome_font_face_get_italics_angle (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	g_return_val_if_fail (GFF_METRICS (face), 0.0);

	return face->private->italics_angle;
}

double
gnome_font_face_get_xheight (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	g_return_val_if_fail (GFF_METRICS (face), 0.0);

	return face->private->xheight;
}

const ArtDRect *
gnome_font_face_get_stdbbox (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (GFF_METRICS (face), NULL);

	return &face->private->bbox;
}
