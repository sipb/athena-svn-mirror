#define __GNOME_GLYPHLIST_C__

#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gnome-glyphlist.h>
#include <libgnomeprint/gnome-glyphlist-private.h>

static void gnome_glyphlist_class_init (GnomeGlyphListClass * klass);
static void gnome_glyphlist_init (GnomeGlyphList * glyphlist);

static void gnome_glyphlist_destroy (GtkObject * object);

static void ggl_ensure_space (GnomeGlyphList * gl, gint num);
static gint ggl_text_to_unicode (const gchar * text, gint length, gint32 * utext, gint ulength);
static GnomeFont * ggl_current_font (GnomeGlyphList * gl);

static GtkObjectClass * parent_class;

GtkType
gnome_glyphlist_get_type (void)
{
	static GtkType glyphlist_type = 0;
	if (!glyphlist_type) {
		GtkTypeInfo glyphlist_info = {
			"GnomeGlyphList",
			sizeof (GnomeGlyphList),
			sizeof (GnomeGlyphListClass),
			(GtkClassInitFunc) gnome_glyphlist_class_init,
			(GtkObjectInitFunc) gnome_glyphlist_init,
			NULL, NULL,
			NULL
		};
		glyphlist_type = gtk_type_unique (gtk_object_get_type (), &glyphlist_info);
	}
	return glyphlist_type;
}

static void
gnome_glyphlist_class_init (GnomeGlyphListClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->destroy = gnome_glyphlist_destroy;
}

static void
gnome_glyphlist_init (GnomeGlyphList * glyphlist)
{
	glyphlist->glyphs = g_new (GGLGlyph, 4);
	glyphlist->length = 0;
	glyphlist->size = 4;
}

static void
gnome_glyphlist_destroy (GtkObject * object)
{
	GnomeGlyphList * glyphlist;

	glyphlist = (GnomeGlyphList *) object;

	if (glyphlist->glyphs) {
		gint i;
		for (i = 0; i < glyphlist->length; i++) {
			GGLGlyph * glyph;
			glyph = glyphlist->glyphs + i;
			while (glyph->info) {
				GGLInfo * info;
				info = (GGLInfo *) glyph->info->data;
				if (info->type == GNOME_GL_FONT) {
					gnome_font_unref (info->value.font);
				}
				g_free (info);
				glyph->info = g_slist_remove (glyph->info, glyph->info->data);
			}
		}
		g_free (glyphlist->glyphs);
		glyphlist->glyphs = NULL;
		glyphlist->length = 0;
		glyphlist->size = 0;
		while (glyphlist->info) {
			GGLInfo * info;
			info = (GGLInfo *) glyphlist->info->data;
			if (info->type == GNOME_GL_FONT) {
				gnome_font_unref (info->value.font);
			}
			g_free (info);
			glyphlist->info = g_slist_remove (glyphlist->info, glyphlist->info->data);
		}
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

GnomeGlyphList *
gnome_glyphlist_from_text_sized_dumb (GnomeFont * font, guint32 color,
				      gdouble kerning, gdouble letterspace,
				      const guchar * text, gint length)
{
	GnomeGlyphList * gl;
	gint32 * utext;
	gint ulength;
	gint glyph, i;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (text != NULL, NULL);

	gl = gtk_type_new (GNOME_TYPE_GLYPHLIST);
	gnome_glyphlist_font (gl, font);
	gnome_glyphlist_color (gl, color);
	gnome_glyphlist_kerning (gl, kerning);
	gnome_glyphlist_letterspace (gl, letterspace);

	/*
	 * Can we expect that unicode string is not longer than original?
	 */

	if (length < 1) return gl;

	utext = g_new (gint32, length * 2);

	ulength = ggl_text_to_unicode (text, length, utext, length * 2);

	if (ulength > 0) {
		for (i = 0; i < ulength; i++) {
			glyph = gnome_font_lookup_default (font, utext[i]);
			gnome_glyphlist_glyph (gl, glyph);
		}
	}

	g_free (utext);

	return gl;
}

GnomeGlyphList *
gnome_glyphlist_from_text_dumb (GnomeFont * font, guint32 color,
				gdouble kerning, gdouble letterspace,
				const guchar * text)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (text != NULL, NULL);

	return gnome_glyphlist_from_text_sized_dumb (font, color,
						     kerning, letterspace,
						     text, strlen (text));
}

void
gnome_glyphlist_text_dumb (GnomeGlyphList * gl, const gchar * text)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (text != NULL);

	gnome_glyphlist_text_sized_dumb (gl, text, strlen (text));
}

void
gnome_glyphlist_text_sized_dumb (GnomeGlyphList * gl, const gchar * text, gint length)
{
	GnomeFont * font;
	gint32 * utext;
	gint ulength;
	gint glyph, i;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (text != NULL);

	if (length < 1) return;

	font = ggl_current_font (gl);
	g_return_if_fail (font != NULL);

	/*
	 * Can we expect that unicode string is not longer than original?
	 */

	utext = g_new (gint32, length * 2);

	ulength = ggl_text_to_unicode (text, length, utext, length * 2);

	if (ulength > 0) {
		for (i = 0; i < ulength; i++) {
			glyph = gnome_font_lookup_default (font, utext[i]);
			gnome_glyphlist_glyph (gl, glyph);
		}
	}

	g_free (utext);
}

void
gnome_glyphlist_glyph (GnomeGlyphList * gl, gint glyph)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	ggl_ensure_space (gl, 1);
	gl->glyphs[gl->length].glyph = glyph;
	gl->glyphs[gl->length].info = gl->info;
	gl->info = NULL;
	gl->length++;
}

void
gnome_glyphlist_glyphs (GnomeGlyphList * gl, gint * glyphs, gint num_glyphs)
{
	gint i;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (glyphs != NULL);

	ggl_ensure_space (gl, num_glyphs);

	for (i = 0; i < num_glyphs; i++) {
		gnome_glyphlist_glyph (gl, glyphs[i]);
	}
}

void
gnome_glyphlist_advance (GnomeGlyphList * gl, gboolean advance)
{
	GGLInfo * info;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_ADVANCE;
	info->value.bval = advance;

	gl->info = g_slist_prepend (gl->info, info);
}

void
gnome_glyphlist_moveto (GnomeGlyphList * gl, gdouble x, gdouble y)
{
	GGLInfo * info;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_MOVETOX;
	info->value.dval = x;

	gl->info = g_slist_prepend (gl->info, info);

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_MOVETOY;
	info->value.dval = y;

	gl->info = g_slist_prepend (gl->info, info);
}

void
gnome_glyphlist_rmoveto (GnomeGlyphList * gl, gdouble x, gdouble y)
{
	GGLInfo * info;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_RMOVETOX;
	info->value.dval = x;

	gl->info = g_slist_prepend (gl->info, info);

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_RMOVETOY;
	info->value.dval = y;

	gl->info = g_slist_prepend (gl->info, info);
}

void
gnome_glyphlist_font (GnomeGlyphList * gl, GnomeFont * font)
{
	GGLInfo * info;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (font != NULL);
	g_return_if_fail (GNOME_IS_FONT (font));

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_FONT;
	info->value.font = font;
	gnome_font_ref (font);

	gl->info = g_slist_prepend (gl->info, info);
}

void
gnome_glyphlist_color (GnomeGlyphList * gl, guint32 color)
{
	GGLInfo * info;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_COLOR;
	info->value.color = color;

	gl->info = g_slist_prepend (gl->info, info);
}

void
gnome_glyphlist_kerning (GnomeGlyphList * gl, gdouble kerning)
{
	GGLInfo * info;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_KERNING;
	info->value.dval = kerning;

	gl->info = g_slist_prepend (gl->info, info);
}

void
gnome_glyphlist_letterspace (GnomeGlyphList * gl, gdouble letterspace)
{
	GGLInfo * info;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	info = g_new (GGLInfo, 1);

	info->type = GNOME_GL_LETTERSPACE;
	info->value.dval = letterspace;

	gl->info = g_slist_prepend (gl->info, info);
}

/*
 * Helpers
 */

static void
ggl_ensure_space (GnomeGlyphList * gl, gint num)
{
	if (gl->length + num > gl->size) {
		while (gl->length + num > gl->size) gl->size <<= 1;
		gl->glyphs = g_renew (GGLGlyph, gl->glyphs, gl->size);
	}
}

static gint
ggl_text_to_unicode (const gchar * text, gint length, gint32 * utext, gint ulength)
{
	const gchar * p;
	gint32 *o;

	o = utext;

	for (p = text; p && p < (text + length); p = g_utf8_next_char (p)) {
		*o++ = g_utf8_get_char (p);
	}

	return o - utext;
}

static GnomeFont *
ggl_current_font (GnomeGlyphList * gl)
{
	GGLInfo * info;
	GSList * l;
	gint i;

	for (l = gl->info; l != NULL; l = l->next) {
		info = (GGLInfo *) l->data;
		if (info->type == GNOME_GL_FONT) return info->value.font;
	}

	for (i = gl->length - 1; i >= 0; i--) {
		for (l = gl->glyphs[i].info; l != NULL; l = l->next) {
			info = (GGLInfo *) l->data;
			if (info->type == GNOME_GL_FONT) return info->value.font;
		}
	}

	return NULL;
}














