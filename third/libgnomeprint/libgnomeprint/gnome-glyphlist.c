/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-glyphlist.c: Device independent rich text representation system
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
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian Inc. and authors
 */

#include <config.h>
#include <string.h>

#include <libgnomeprint/gnome-glyphlist-private.h>

static void ggl_ensure_glyph_space (GnomeGlyphList *gl, gint space);
static void ggl_ensure_rule_space (GnomeGlyphList *gl, gint space);

#define GGL_ENSURE_GLYPH_SPACE(ggl,s) {if ((ggl)->g_size < (ggl)->g_length + (s)) ggl_ensure_glyph_space (ggl, s);}
#define GGL_ENSURE_RULE_SPACE(ggl,s) {if((ggl)->r_size < (ggl)->r_length + (s)) ggl_ensure_rule_space (ggl, s);}

void gnome_glyphlist_moveto_x  (GnomeGlyphList *gl, gdouble distance);
void gnome_glyphlist_moveto_y  (GnomeGlyphList *gl, gdouble distance);
void gnome_glyphlist_rmoveto_x (GnomeGlyphList *gl, gdouble distance);
void gnome_glyphlist_rmoveto_y (GnomeGlyphList *gl, gdouble distance);


GType
gnome_glyphlist_get_type(void)
{
	static GType type = 0;
	
	if (type == 0) {
		type = g_boxed_type_register_static
			("GnomeGlyphList",
			 (GBoxedCopyFunc) gnome_glyphlist_ref,
			 (GBoxedFreeFunc) gnome_glyphlist_unref);
	}

	return type;
}

GnomeGlyphList *
gnome_glyphlist_new (void)
{
	GnomeGlyphList *gl;

	gl = g_new0 (GnomeGlyphList, 1);

	gl->refcount = 1;

	return gl;
}

GnomeGlyphList *
gnome_glyphlist_ref (GnomeGlyphList *gl)
{
	g_return_val_if_fail (gl != NULL, NULL);
	g_return_val_if_fail (gl->refcount > 0, NULL);

	gl->refcount++;

	return gl;
}

GnomeGlyphList *
gnome_glyphlist_unref (GnomeGlyphList *gl)
{
	g_return_val_if_fail (gl != NULL, NULL);
	g_return_val_if_fail (gl->refcount > 0, NULL);

	if (--gl->refcount < 1) {
		if (gl->glyphs) {
			g_free (gl->glyphs);
			gl->glyphs = NULL;
		}
		if (gl->rules) {
			gint r;
			for (r = 0; r < gl->r_length; r++) {
				if (gl->rules[r].code == GGL_FONT) {
					gnome_font_unref (gl->rules[r].value.font);
				}
			}
			g_free (gl->rules);
			gl->rules = NULL;
		}
		g_free (gl);
	}

	return NULL;
}

GnomeGlyphList *
gnome_glyphlist_duplicate (GnomeGlyphList *gl)
{
	GnomeGlyphList *new;
	gint i;

	g_return_val_if_fail (gl != NULL, NULL);

	new = g_new (GnomeGlyphList, 1);

	new->refcount = 1;
	new->glyphs    = g_new (gint,    gl->g_length);
	new->rules     = g_new (GGLRule, gl->r_length);

	new->g_length = gl->g_length;
	new->g_size   = gl->g_length;
	new->r_length = gl->r_length;
	new->r_size   = gl->r_length;

	memcpy (new->glyphs, gl->glyphs, gl->g_length * sizeof (gint));
	memcpy (new->rules,  gl->rules,  gl->r_length * sizeof (GGLRule));
	
	for (i = 0; i < new->r_length; i++) {
		if (new->rules[i].code == GGL_FONT)
			gnome_font_ref (new->rules[i].value.font);
	}

	return new;
}

gboolean
gnome_glyphlist_check (const GnomeGlyphList *gl, gboolean rules)
{
	if (gl == NULL)
		return FALSE;

	return TRUE;
}

/**
 * gnome_glyphlist_glyph:
 * @gl: 
 * @glyph: 
 * 
 * Appends a single glyph to the glyphlist. It will be connected
 * to the previous glyphs by the previously defined rules
 *
 **/
void
gnome_glyphlist_glyph (GnomeGlyphList *gl, gint glyph)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (glyph >= 0);

	GGL_ENSURE_GLYPH_SPACE (gl, 1);

	gl->glyphs[gl->g_length] = glyph;
	gl->g_length++;
}

/**
 * gnome_glyphlist_glyphs:
 * @gl: 
 * @glyphs: 
 * @num_glyphs: 
 * 
 * Append a string of glyphs
 **/
void
gnome_glyphlist_glyphs (GnomeGlyphList *gl, gint *glyphs, gint num_glyphs)
{
	gint i;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (glyphs != NULL);

	GGL_ENSURE_GLYPH_SPACE (gl, num_glyphs);

	for (i = 0; i < num_glyphs; i++) {
		gnome_glyphlist_glyph (gl, glyphs[i]);
	}
}


void
gnome_glyphlist_moveto_x (GnomeGlyphList *gl, gdouble distance)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if ((gl->rules[r].code == GGL_MOVETOX) || (gl->rules[r].code == GGL_RMOVETOX)) {
						/* There is moveto or rmoveto in ruleset */
						gl->rules[r].code = GGL_MOVETOX;
						gl->rules[r].value.dval = distance;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_MOVETOX;
				gl->rules[r].value.dval = distance;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_MOVETOX;
	gl->rules[gl->r_length].value.dval = distance;
	gl->r_length++;
}

void
gnome_glyphlist_rmoveto_x (GnomeGlyphList *gl, gdouble distance)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if ((gl->rules[r].code == GGL_MOVETOX) || (gl->rules[r].code == GGL_RMOVETOX)) {
						/* There is moveto or rmoveto in ruleset */
						gl->rules[r].value.dval += distance;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_RMOVETOX;
				gl->rules[r].value.dval = distance;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_RMOVETOX;
	gl->rules[gl->r_length].value.dval = distance;
	gl->r_length++;
}

void
gnome_glyphlist_moveto_y (GnomeGlyphList *gl, gdouble distance)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if ((gl->rules[r].code == GGL_MOVETOY) || (gl->rules[r].code == GGL_RMOVETOY)) {
						/* There is moveto or rmoveto in ruleset */
						gl->rules[r].code = GGL_MOVETOY;
						gl->rules[r].value.dval = distance;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_MOVETOY;
				gl->rules[r].value.dval = distance;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_MOVETOY;
	gl->rules[gl->r_length].value.dval = distance;
	gl->r_length++;
}

void
gnome_glyphlist_rmoveto_y (GnomeGlyphList *gl, gdouble distance)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if ((gl->rules[r].code == GGL_MOVETOY) || (gl->rules[r].code == GGL_RMOVETOY)) {
						/* There is moveto or rmoveto in ruleset */
						gl->rules[r].value.dval += distance;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_RMOVETOY;
				gl->rules[r].value.dval = distance;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_RMOVETOY;
	gl->rules[gl->r_length].value.dval = distance;
	gl->r_length++;
}

/**
 * gnome_glyphlist_moveto:
 * @gl: 
 * @x: 
 * @y: 
 * 
 * Position manually the glyph following
 **/
void
gnome_glyphlist_moveto (GnomeGlyphList *gl, gdouble x, gdouble y)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	gnome_glyphlist_moveto_x (gl, x);
	gnome_glyphlist_moveto_y (gl, y);
}

/**
 * gnome_glyphlist_rmoveto:
 * @gl: 
 * @x: 
 * @y: 
 * 
 * Position the glyph following relative to current pen position
 *
 **/
void
gnome_glyphlist_rmoveto (GnomeGlyphList *gl, gdouble x, gdouble y)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	gnome_glyphlist_rmoveto_x (gl, x);
	gnome_glyphlist_rmoveto_y (gl, y);
}

/**
 * gnome_glyphlist_advance:
 * @gl: 
 * @advance: 
 *
 * Whether or not to move the pen position by the font standard
 * advance vector. Advancing happens immediately after glyph is
 * sent through pipeline.
 **/
void
gnome_glyphlist_advance (GnomeGlyphList *gl, gboolean advance)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if (gl->rules[r].code == GGL_ADVANCE) {
						gl->rules[r].value.bval = advance;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_ADVANCE;
				gl->rules[r].value.bval = advance;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_ADVANCE;
	gl->rules[gl->r_length].value.bval = advance;
	gl->r_length++;
}

/**
 * gnome_glyphlist_letterspace:
 * @gl: 
 * @letterspace:
 *
 * Amount of white space to add between glyphs connected by
 * an advance rule. It is specified in font units (i.e. 12 for 12pt
 * font is the width of an em square). If glyph is manually positioned,
 * letterspace value will be ignored. Letterspace will be added
 * immediately before placing a new glyph
 * FIXME: what does the last sentence mean? (Chema)
 *
 **/
void
gnome_glyphlist_letterspace (GnomeGlyphList *gl, gdouble letterspace)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if (gl->rules[r].code == GGL_LETTERSPACE) {
						gl->rules[r].value.dval = letterspace;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_LETTERSPACE;
				gl->rules[r].value.dval = letterspace;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_LETTERSPACE;
	gl->rules[gl->r_length].value.dval = letterspace;
	gl->r_length++;
}



/**
 * gnome_glyphlist_kerning:
 * @gl: 
 * @kerning:
 *
 * Amount of kerning to add between glyphs connected by an advance
 * rule. It is specified as fraction of a full kerning value 
 * If a glyph is manually positioned, the kerning value is ignored
 * Kerning will be added immediately before placing a new glyph
 * FIXME: What does the last sentence mean? (Chema)
 **/
void
gnome_glyphlist_kerning (GnomeGlyphList *gl, gdouble kerning)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if (gl->rules[r].code == GGL_KERNING) {
						gl->rules[r].value.dval = kerning;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_KERNING;
				gl->rules[r].value.dval = kerning;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_KERNING;
	gl->rules[gl->r_length].value.dval = kerning;
	gl->r_length++;
}

/**
 * gnome_glyphlist_font:
 * @gl: 
 * @font: 
 * 
 * Specify the font to be used for the glyphs that follow
 **/
void
gnome_glyphlist_font (GnomeGlyphList *gl, GnomeFont * font)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (font != NULL);
	g_return_if_fail (GNOME_IS_FONT (font));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if (gl->rules[r].code == GGL_FONT) {
						gnome_font_ref (font);
						gnome_font_unref (gl->rules[r].value.font);
						gl->rules[r].value.font = font;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_FONT;
				gnome_font_ref (font);
				gl->rules[r].value.font = font;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_FONT;
	gnome_font_ref (font);
	gl->rules[gl->r_length].value.font = font;
	gl->r_length++;
}

/**
 * gnome_glyphlist_color:
 * @gl: 
 * @color: 
 * 
 * Specify the color as RRGGBBAA to be used for the glyphs
 * that follow
 *
 **/
void
gnome_glyphlist_color (GnomeGlyphList *gl, guint32 color)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				for (r = r + 1; r < gl->r_length; r++) {
					if (gl->rules[r].code == GGL_COLOR) {
						gl->rules[r].value.color = color;
						return;
					}
				}
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_COLOR;
				gl->rules[r].value.color = color;
				gl->r_length++;
				return;
			}
			break;
		}
	}

	GGL_ENSURE_RULE_SPACE (gl, 2);
	gl->rules[gl->r_length].code = GGL_POSITION;
	gl->rules[gl->r_length].value.ival = gl->g_length;
	gl->r_length++;
	gl->rules[gl->r_length].code = GGL_COLOR;
	gl->rules[gl->r_length].value.color = color;
	gl->r_length++;
}

/**
 * gnome_glyphlist_from_text_sized_dumb:
 * @font: 
 * @color: 
 * @kerning: 
 * @letterspace: 
 * @text: 
 * @length: 
 * 
 * Appends utf8 text, converting it to glyphs and connecting it
 * as specified by rules. You cannot expect anything about
 * language-specific typesetting rules, so if the given script
 * does not use trivial placement, you should better avoid this
 * 
 * Return Value: 
 **/
GnomeGlyphList *
gnome_glyphlist_from_text_sized_dumb (GnomeFont * font, guint32 color,
				      gdouble kerning, gdouble letterspace,
				      const guchar * text, gint length)
{
	GnomeGlyphList *gl;
	const guchar * p;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (text != NULL, NULL);

	gl = gnome_glyphlist_new ();
	gnome_glyphlist_font (gl, font);
	gnome_glyphlist_color (gl, color);
	gnome_glyphlist_advance (gl, TRUE);
	gnome_glyphlist_kerning (gl, kerning);
	gnome_glyphlist_letterspace (gl, letterspace);

	for (p = text; p && p < (text + length); p = g_utf8_next_char (p)) {
		gint unival, glyph;
		unival = g_utf8_get_char (p);
		glyph = gnome_font_lookup_default (font, unival);
		gnome_glyphlist_glyph (gl, glyph);
	}

	return gl;
}

/**
 * gnome_glyphlist_from_text_dumb:
 * @font: 
 * @color: 
 * @kerning: 
 * @letterspace: 
 * @text: 
 * 
 * See _from_text_sized_dumb
 * 
 * Return Value: 
 **/
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

/**
 * gnome_glyphlist_text_sized_dumb:
 * @gl: 
 * @text: 
 * @length: 
 * 
 * The 'dumb' versions of glyphlist creation
 * It just places glyphs one after another - no ligaturing etc.
 * text is utf8, of course
 *
 **/
void
gnome_glyphlist_text_sized_dumb (GnomeGlyphList *gl, const guchar *text, gint length)
{
	GnomeFont * font;
	const guchar * p;
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (text != NULL);

	if (length < 1)
		return;

	font = NULL;
	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_FONT) {
			font = gl->rules[r].value.font;
			break;
		}
	}
	g_return_if_fail (font != NULL);

	for (p = text; p && p < (text + length); p = g_utf8_next_char (p)) {
		gint unival, glyph;
		unival = g_utf8_get_char (p);
		glyph = gnome_font_lookup_default (font, unival);
		gnome_glyphlist_glyph (gl, glyph);
	}
}

/**
 * gnome_glyphlist_text_dumb:
 * @gl: 
 * @text: 
 * 
 * See _text_sized_dumb
 **/
void
gnome_glyphlist_text_dumb (GnomeGlyphList *gl, const guchar *text)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (text != NULL);

	gnome_glyphlist_text_sized_dumb (gl, text, strlen (text));
}

static void
ggl_ensure_glyph_space (GnomeGlyphList *gl, gint space)
{
	if (gl->g_size >= gl->g_length + space)
		return;

	/* FIXME: This is wrong! what if space is greater than BLOCK_SIZE? (Chema) */
	gl->g_size += GGL_RULE_BLOCK_SIZE;
	gl->glyphs = g_renew (gint, gl->glyphs, gl->g_size);
}

static void
ggl_ensure_rule_space (GnomeGlyphList *gl, gint space)
{
	if (gl->r_size >= gl->r_length + space)
		return;

	/* FIXME: This is wrong! what if space is greater than BLOCK_SIZE? (Chema) */
	gl->r_size += GGL_RULE_BLOCK_SIZE;
	gl->rules = g_renew (GGLRule, gl->rules, gl->r_size);
}


