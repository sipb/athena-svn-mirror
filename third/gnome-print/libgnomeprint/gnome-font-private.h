#ifndef __GNOME_FONT_PRIVATE_H__
#define __GNOME_FONT_PRIVATE_H__

#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gt1-parset1.h>

BEGIN_GNOME_DECLS

struct _GnomeFontFace {
	GtkObject object;
	GnomeFontFacePrivate * private;
};

struct _GnomeFontFaceClass {
	GtkObjectClass parent_class;
};

struct _GnomeFont {
	GtkObject object;
	GnomeFontPrivate * private;
};

struct _GnomeFontClass {
	GtkObjectClass parent_class;
};

typedef struct _GnomeFontKernPair	GnomeFontKernPair;
typedef struct _GnomeFontLigList	GnomeFontLigList;

struct _GnomeFontPrivate {
	double size;
	GnomeFontMap *fontmap_entry;
	double scale;
	gchar * name;
	GHashTable *outlines;
};

typedef struct {
	gint unicode;
	const gchar * psname;
	ArtPoint advance;
	ArtDRect bbox;
	ArtBpath * bpath;
} GFFGlyphInfo;

struct _GnomeFontFacePrivate {
	char *font_name;
	char *afm_fn;
	char *pfb_fn;
	char *fullname;
	char *familyname;
	char *weight;
	char *alias;

	GnomeFontWeight weight_code;
	gboolean italic;
	gboolean fixed_width;

	/* Font metric info follows */
	/* above is font metric info stored in the fontmap, below is font
	 * metric info parsed from the afm file. */

	gint num_glyphs;
	GFFGlyphInfo * glyphs;
	GHashTable * glyphmap;

	gint num_private;
	GHashTable * privencoding;

	int   ascender;
	int   descender;
	int   underline_position;
	int   underline_thickness;
	double capheight;
	double italics_angle;
	double xheight;
	ArtDRect bbox;
#if 0
	int  *widths;
#endif
	GnomeFontKernPair *kerns;
	int num_kerns;
	GnomeFontLigList **ligs; /* one liglist for each glyph */

	int first_cov_page;
	int num_cov_pages;
	int **cov_pages; /* each page is an array of 256 ints */

	Gt1LoadedFont * loadedfont;
};

struct _GnomeFontKernPair {
	int glyph1;
	int glyph2;
	int x_amt;
};

struct _GnomeFontLigList {
	GnomeFontLigList *next;
	int succ, lig;
};

/* Should be renamed to gnome_xx_get_alias, if preserved */

const char * gnome_font_get_glyph_name (const GnomeFont *font);

/* These should be renamed to gnome_xx_get_alias, if preserved */

const gchar * gnome_font_unsized_get_glyph_name (const GnomeFontFace * face);

/*
 * Returns number of defined glyphs in typeface
 */

gint gnome_font_face_get_num_glyphs (const GnomeFontFace * face);

/*
 * Returns PostScript name for glyph
 */

const gchar * gnome_font_face_get_glyph_ps_name (const GnomeFontFace * face, gint glyph);

/*
 * Return font caracteristics, grabbed from the AFM file, needed for PDF generation 
 */
double gnome_font_face_get_capheight    (const GnomeFontFace * face);
double gnome_font_face_get_italics_angle (const GnomeFontFace * face);
double gnome_font_face_get_xheight (const GnomeFontFace * face);

END_GNOME_DECLS

#endif

