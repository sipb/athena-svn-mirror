#ifndef _GNOME_GLYPHLIST_PRIVATE_H_
#define _GNOME_GLYPHLIST_PRIVATE_H_

#include <libgnomeprint/gnome-glyphlist.h>

BEGIN_GNOME_DECLS

/*
 * We are dealing with lists at moment - although arrays are probably better
 */

typedef struct {
	gint glyph;
	GSList * info;
} GGLGlyph;

struct _GnomeGlyphList {
	GtkObject object;
	GGLGlyph * glyphs;
	gint length;
	gint size;
	GSList * info;
};

struct _GnomeGlyphListClass {
	GtkObjectClass parent_class;
};

typedef enum {
	GNOME_GL_ADVANCE,
	GNOME_GL_MOVETOX,
	GNOME_GL_MOVETOY,
	GNOME_GL_RMOVETOX,
	GNOME_GL_RMOVETOY,
	GNOME_GL_PUSHCPX,
	GNOME_GL_PUSHCPY,
	GNOME_GL_POPCPX,
	GNOME_GL_POPCPY,
	GNOME_GL_FONT,
	GNOME_GL_COLOR,
	GNOME_GL_KERNING,
	GNOME_GL_LETTERSPACE
} GGLInfoType;

typedef struct {
	GGLInfoType type;
	union {
		GnomeFont * font;
		guint32 color;
		gfloat dval;
		gint ival;
		gboolean bval;
	} value;
} GGLInfo;

END_GNOME_DECLS

#endif
