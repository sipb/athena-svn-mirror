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
 *  Copyright (C) 2000-2001 Ximian Inc. and authors
 *
 */

#define __GNOME_GLYPHLIST_C__

#include <string.h>
#include "gnome-glyphlist-private.h"

static void ggl_ensure_glyph_space (GnomeGlyphList * gl, gint space);
static void ggl_ensure_rule_space (GnomeGlyphList * gl, gint space);

#define GGL_ENSURE_GLYPH_SPACE(ggl,s) {if ((ggl)->g_size < (ggl)->g_length + (s)) ggl_ensure_glyph_space (ggl, s);}
#define GGL_ENSURE_RULE_SPACE(ggl,s) {if((ggl)->r_size < (ggl)->r_length + (s)) ggl_ensure_rule_space (ggl, s);}

void gnome_glyphlist_moveto_x (GnomeGlyphList * gl, gdouble distance);
void gnome_glyphlist_moveto_y (GnomeGlyphList * gl, gdouble distance);
void gnome_glyphlist_rmoveto_x (GnomeGlyphList * gl, gdouble distance);
void gnome_glyphlist_rmoveto_y (GnomeGlyphList * gl, gdouble distance);
void gnome_glyphlist_push_cp (GnomeGlyphList * gl);
void gnome_glyphlist_pop_cp (GnomeGlyphList * gl);

GnomeGlyphList *
gnome_glyphlist_new (void)
{
	GnomeGlyphList *gl;

	gl = g_new (GnomeGlyphList, 1);

	gl->refcount = 1;
	gl->glyphs = NULL;
	gl->g_length = 0;
	gl->g_size = 0;
	gl->rules = NULL;
	gl->r_length = 0;
	gl->r_size = 0;

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
	new->glyphs = g_new (gint, gl->g_length);
	memcpy (new->glyphs, gl->glyphs, gl->g_length * sizeof (gint));
	new->g_length = gl->g_length;
	new->g_size = gl->g_length;
	new->rules = g_new (GGLRule, gl->r_length);
	memcpy (new->rules, gl->rules, gl->r_length * sizeof (GGLRule));
	new->r_length = gl->r_length;
	new->r_size = gl->r_length;

	for (i = 0; i < new->r_length; i++) {
		if (new->rules[i].code == GGL_FONT) gnome_font_ref (new->rules[i].value.font);
	}

	return new;
}

gboolean
gnome_glyphlist_check (const GnomeGlyphList *gl, gboolean rules)
{
	if (gl == NULL) return FALSE;

	return TRUE;
}

void
gnome_glyphlist_glyph (GnomeGlyphList * gl, gint glyph)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (glyph >= 0);

	GGL_ENSURE_GLYPH_SPACE (gl, 1);

	gl->glyphs[gl->g_length] = glyph;
	gl->g_length++;
}

void
gnome_glyphlist_moveto_x (GnomeGlyphList * gl, gdouble distance)
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
gnome_glyphlist_rmoveto_x (GnomeGlyphList * gl, gdouble distance)
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
gnome_glyphlist_moveto_y (GnomeGlyphList * gl, gdouble distance)
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
gnome_glyphlist_rmoveto_y (GnomeGlyphList * gl, gdouble distance)
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

void
gnome_glyphlist_moveto (GnomeGlyphList * gl, gdouble x, gdouble y)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	gnome_glyphlist_moveto_x (gl, x);
	gnome_glyphlist_moveto_y (gl, y);
}

void
gnome_glyphlist_rmoveto (GnomeGlyphList * gl, gdouble x, gdouble y)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	gnome_glyphlist_rmoveto_x (gl, x);
	gnome_glyphlist_rmoveto_y (gl, y);
}

void
gnome_glyphlist_push_cp (GnomeGlyphList * gl)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_PUSHCP;
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
	gl->rules[gl->r_length].code = GGL_PUSHCP;
	gl->r_length++;
}

void
gnome_glyphlist_pop_cp (GnomeGlyphList * gl)
{
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));

	for (r = gl->r_length - 1; r >= 0; r--) {
		if (gl->rules[r].code == GGL_POSITION) {
			g_return_if_fail (gl->rules[r].value.ival <= gl->g_length);
			if (gl->rules[r].value.ival == gl->g_length) {
				/* There is ruleset at the end of glyphlist */
				GGL_ENSURE_RULE_SPACE (gl, 1);
				gl->rules[r].code = GGL_POPCP;
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
	gl->rules[gl->r_length].code = GGL_POPCP;
	gl->r_length++;
}

void
gnome_glyphlist_advance (GnomeGlyphList * gl, gboolean advance)
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

void
gnome_glyphlist_letterspace (GnomeGlyphList * gl, gdouble letterspace)
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

void
gnome_glyphlist_kerning (GnomeGlyphList * gl, gdouble kerning)
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

void
gnome_glyphlist_font (GnomeGlyphList * gl, GnomeFont * font)
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

void
gnome_glyphlist_color (GnomeGlyphList * gl, guint32 color)
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

GnomeGlyphList *
gnome_glyphlist_from_text_sized_dumb (GnomeFont * font, guint32 color,
				      gdouble kerning, gdouble letterspace,
				      const guchar * text, gint length)
{
	GnomeGlyphList * gl;
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
gnome_glyphlist_text_sized_dumb (GnomeGlyphList * gl, const guchar * text, gint length)
{
	GnomeFont * font;
	const guchar * p;
	gint r;

	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (text != NULL);

	if (length < 1) return;

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

void
gnome_glyphlist_text_dumb (GnomeGlyphList * gl, const guchar * text)
{
	g_return_if_fail (gl != NULL);
	g_return_if_fail (GNOME_IS_GLYPHLIST (gl));
	g_return_if_fail (text != NULL);

	gnome_glyphlist_text_sized_dumb (gl, text, strlen (text));
}

void
gnome_glyphlist_glyphs (GnomeGlyphList * gl, gint * glyphs, gint num_glyphs)
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

static void
ggl_ensure_glyph_space (GnomeGlyphList * gl, gint space)
{
	if (gl->g_size >= gl->g_length + space) return;

	gl->g_size += GGL_RULE_BLOCK_SIZE;
	gl->glyphs = g_renew (gint, gl->glyphs, gl->g_size);
}

static void
ggl_ensure_rule_space (GnomeGlyphList * gl, gint space)
{
	if (gl->r_size >= gl->r_length + space) return;

	gl->r_size += GGL_RULE_BLOCK_SIZE;
	gl->rules = g_renew (GGLRule, gl->rules, gl->r_size);
}


