#ifndef _GNOME_PGL_PRIVATE_H_
#define _GNOME_PGL_PRIVATE_H_

#include <libgnomeprint/gnome-rfont.h>
#include <libgnomeprint/gnome-glyphlist.h>

BEGIN_GNOME_DECLS

typedef struct _GnomePosGlyph GnomePosGlyph;
typedef struct _GnomePosString GnomePosString;

/*
 * Positioned Glyph
 */

struct _GnomePosGlyph {
	gint glyph;
	guint32 color;
	gfloat x, y;
};

struct _GnomePosString {
	GnomeRFont * rfont;
	GnomePosGlyph * glyphs;
	gint length;
};

struct _GnomePosGlyphList {
	GnomePosGlyph * glyphs;
	GSList * strings;
};

END_GNOME_DECLS

#endif
