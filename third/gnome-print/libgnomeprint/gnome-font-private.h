#ifndef __GNOME_FONT_PRIVATE_H__
#define __GNOME_FONT_PRIVATE_H__

#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gp-character-block.h>
#include <libgnomeprint/gp-fontmap.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-rfont.h>
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
	/* Pointer to our fontmap entry */
	GPFontEntry * entry;

	GnomeFontWeight weight_code;
	gboolean italic;
	gboolean fixed_width;

	/* Glyph storage */

	gint num_glyphs;
	GFFGlyphInfo * glyphs;

	/* Encoding */

	GPUCMap * unimap;

	gint num_private;
	GHashTable * privencoding;

	/* Font metric info follows */
	/* above is font metric info stored in the fontmap, below is font
	 * metric info parsed from the afm file. */

	int   ascender;
	int   descender;
	int   underline_position;
	int   underline_thickness;
	double capheight;
	double italics_angle;
	double xheight;
	ArtDRect bbox;
	GnomeFontKernPair *kerns;
	int num_kerns;
	GnomeFontLigList **ligs; /* one liglist for each glyph */

	Gt1LoadedFont * loadedfont;
};

/*
 * Removed from the structure:

	int first_cov_page;
	int num_cov_pages;
	int **cov_pages;
	int  *widths;
	char * afm_fn;
	char * pfb_fn;
	char * fullname;
	char * familyname;
	char * speciesname;
	char * psname;
	char * alias;
  */

struct _GnomeFontKernPair {
	int glyph1;
	int glyph2;
	int x_amt;
};

struct _GnomeFontLigList {
	GnomeFontLigList *next;
	int succ, lig;
};

/*
 * Returns PostScript name for glyph
 */

const gchar * gnome_font_face_get_glyph_ps_name (const GnomeFontFace * face, gint glyph);
const gchar * gnome_font_unsized_get_glyph_name (const GnomeFontFace * face);

ArtPoint * gnome_rfont_get_glyph_stdkerning (const GnomeRFont * rfont, gint glyph0, gint glyph1, ArtPoint * kerning);

/*
 * AFM metric info can now be retrieved via GtkArg system
 */

END_GNOME_DECLS

#endif

