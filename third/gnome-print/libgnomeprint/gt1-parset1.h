#ifndef __PARSET1_H__
#define __PARSET1_H__

BEGIN_GNOME_DECLS

/* Header file for parset1.

   You have to include these headers additionally:

   libart_lgpl/art_bpath.h
   libart_lgpl/art_rect.h

*/

typedef struct _Gt1LoadedFont Gt1LoadedFont;
Gt1LoadedFont * gt1_load_font (const char *filename, GHashTable * privencoding);
void gt1_unload_font (Gt1LoadedFont *);

typedef struct {
	int       ch;
	ArtBpath *bpath;
	ArtDRect  bbox;
	double    wx;
} Gt1GlyphOutline;

Gt1GlyphOutline *gt1_glyph_outline_lookup (Gt1LoadedFont *, int glyphnum);

ArtBpath *gt1_get_glyph_outline (Gt1LoadedFont *font, int glyphnum, double *p_wx);

double
gt1_get_kern_pair (Gt1LoadedFont *font, int glyph1, int glyph2);

char *
gt1_get_font_name (Gt1LoadedFont *font);

END_GNOME_DECLS

#endif /* __PARSET1_H__ */
