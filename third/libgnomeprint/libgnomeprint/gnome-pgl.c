/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-pgl.c: Experimental device adjusted rich text representation system
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
 */

#include <config.h>

#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-pgl.h>
#include <libgnomeprint/gnome-pgl-private.h>
#include <libgnomeprint/gnome-glyphlist-private.h>

#define PGL_STACK_SIZE 128

/* I hate state machines! (Lauris Kaplinski, 2001) */

GnomePosGlyphList *
gnome_pgl_from_gl (const GnomeGlyphList * gl, const gdouble * transform, guint flags)
{
	static gdouble identity[] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
	GnomePosGlyphList * pgl;
	gboolean fontfound;
	gint allocated_strings;
	gint r;
	gint cg, cr;
	ArtPoint pos, pen;
	gboolean usemetrics;
	gint lastglyph;
	gboolean advance;
	ArtPoint letterspace;
	gdouble kerning;
	GnomeFont * font;
	guint32 color;
	gboolean needstring;
	gint currentstring;
	gint sptr;
	ArtPoint p;

	g_return_val_if_fail (gl != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_GLYPHLIST (gl), NULL);
	if (!transform)
		transform = identity;

	/* Special case */
	if (gl->g_length < 1) {
		pgl = g_new (GnomePosGlyphList, 1);
		pgl->glyphs = NULL;
		pgl->strings = NULL;
		pgl->num_strings = 0;
		return pgl;
	}
	/* We need font rule */
	g_return_val_if_fail (gl->r_length > 0, NULL);
	g_return_val_if_fail (gl->rules[0].code == GGL_POSITION, NULL);
	g_return_val_if_fail (gl->rules[0].value.ival == 0, NULL);
	fontfound = FALSE;
	for (r = 1; (r < gl->r_length) && (gl->rules[r].code != GGL_POSITION); r++) {
		if (gl->rules[r].code == GGL_FONT) {
			g_return_val_if_fail (gl->rules[r].value.font != NULL, NULL);
			g_return_val_if_fail (GNOME_IS_FONT (gl->rules[r].value.font), NULL);
			fontfound = TRUE;
			break;
		}
	}
	g_return_val_if_fail (fontfound, NULL);

	/* Initialize pgl */
	pgl = g_new (GnomePosGlyphList, 1);
	pgl->glyphs = g_new (GnomePosGlyph, gl->g_length);
	pgl->strings = g_new (GnomePosString, 1);
	pgl->num_strings = 0;
	allocated_strings = 1;

	/* State machine */
	sptr = 0;
	pen.x = transform[4];
	pen.y = transform[5];
	usemetrics = FALSE;
	lastglyph = -1;
	advance = TRUE;
	letterspace.x = 0.0;
	letterspace.y = 0.0;
	kerning = 0.0;
	font = NULL;
	color = 0x000000ff;
	needstring = TRUE;
	currentstring = -1;
	cr = 0;
	for (cg = 0; cg < gl->g_length; cg++) {
		/* Collect rules */
		while ((cr < gl->r_length) && ((gl->rules[cr].code != GGL_POSITION) || (gl->rules[cr].value.ival <= cg))) {
			switch (gl->rules[cr].code) {
			case GGL_MOVETOX:
				/* moveto is special case */
				g_return_val_if_fail (cr + 1 < gl->r_length, NULL);
				g_return_val_if_fail (gl->rules[cr + 1].code == GGL_MOVETOY, NULL);
				pos.x = gl->rules[cr].value.dval;
				pos.y = gl->rules[cr + 1].value.dval;
				cr += 1;
				usemetrics = FALSE;
				art_affine_point (&pen, &pos, transform);
				break;
			case GGL_RMOVETOX:
				/* rmoveto is special case */
				g_return_val_if_fail (cr + 1 < gl->r_length, NULL);
				g_return_val_if_fail (gl->rules[cr + 1].code == GGL_RMOVETOY, NULL);
				pos.x = gl->rules[cr].value.dval;
				pos.y = gl->rules[cr + 1].value.dval;
				cr += 1;
				usemetrics = FALSE;
				pen.x += pos.x * transform[0] + pos.y * transform[2];
				pen.y += pos.x * transform[1] + pos.y * transform[3];
				break;
			case GGL_ADVANCE:
				advance = gl->rules[cr].value.bval;
				break;
			case GGL_LETTERSPACE:
				p.x = gl->rules[cr].value.dval;
				p.y = 0.0;
				letterspace.x = p.x * transform[0] + p.y * transform[2];
				letterspace.y = p.x * transform[1] + p.y * transform[3];
				break;
			case GGL_KERNING:
				kerning = gl->rules[cr].value.dval;
				break;
			case GGL_FONT:
				font = gl->rules[cr].value.font;
				g_return_val_if_fail (font != NULL, NULL);
				g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
				needstring = TRUE;
				break;
			case GGL_COLOR:
				color = gl->rules[cr].value.color;
				needstring = TRUE;
				break;
			}
			cr += 1;
		}

		if (needstring) {
			/* Add new string instance */
			g_assert (GNOME_IS_FONT (font));
			if (pgl->num_strings >= allocated_strings) {
				allocated_strings += 4;
				pgl->strings = g_renew (GnomePosString, pgl->strings, allocated_strings);
			}
			currentstring = pgl->num_strings;
			pgl->num_strings += 1;
			pgl->strings[currentstring].start = cg;
			pgl->strings[currentstring].length = 0;
			pgl->strings[currentstring].rfont = gnome_font_get_rfont (font, transform);
			pgl->strings[currentstring].color = color;
			needstring = FALSE;
		}
		/* Rules are parsed and currentstring points to active string */
		/* Process glyph */
		pgl->glyphs[cg].glyph = gl->glyphs[cg];
		pgl->strings[currentstring].length += 1;
		if (usemetrics && (lastglyph > 0) && (pgl->glyphs[cg].glyph > 0)) {
			/* Need to add kerning */
			if (gnome_rfont_get_glyph_stdkerning (pgl->strings[currentstring].rfont, lastglyph, pgl->glyphs[cg].glyph, &p)) {
				pen.x += p.x;
				pen.y += p.y;
			}
			pen.x += letterspace.x;
			pen.y += letterspace.y;
		}
		pgl->glyphs[cg].x = pen.x;
		pgl->glyphs[cg].y = pen.y;
		if (advance) {
			if (gnome_rfont_get_glyph_stdadvance (pgl->strings[currentstring].rfont, pgl->glyphs[cg].glyph, &p)) {
				pen.x += p.x;
				pen.y += p.y;
			}
		}
		usemetrics = TRUE;
		lastglyph = pgl->glyphs[cg].glyph;
	}

	return pgl;
}

void
gnome_pgl_destroy (GnomePosGlyphList * pgl)
{
	gint s;

	g_return_if_fail (pgl != NULL);

	if (pgl->glyphs)
		g_free (pgl->glyphs);
	for (s = 0; s < pgl->num_strings; s++) {
		gnome_rfont_unref (pgl->strings[s].rfont);
	}
	if (pgl->strings)
		g_free (pgl->strings);

	g_free (pgl);
}

ArtDRect *
gnome_pgl_bbox (const GnomePosGlyphList * pgl, ArtDRect * bbox)
{
	gint s;

	g_return_val_if_fail (pgl != NULL, NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	bbox->x0 = bbox->y0 = 1.0;
	bbox->x1 = bbox->y1 = 0.0;

	for (s = 0; s < pgl->num_strings; s++) {
		gint i;

		for (i = pgl->strings[s].start; i < pgl->strings[s].start + pgl->strings[s].length; i++) {
			ArtDRect gbox;
			gnome_rfont_get_glyph_stdbbox (pgl->strings[s].rfont, pgl->glyphs[i].glyph, &gbox);
			gbox.x0 += pgl->glyphs[i].x;
			gbox.y0 += pgl->glyphs[i].y;
			gbox.x1 += pgl->glyphs[i].x;
			gbox.y1 += pgl->glyphs[i].y;
			art_drect_union (bbox, bbox, &gbox);
		}
	}

	return bbox;
}

gboolean
gnome_pgl_test_point (const GnomePosGlyphList *pgl, gdouble x, gdouble y)
{
	gint s;

	g_return_val_if_fail (pgl != NULL, FALSE);

	for (s = 0; s < pgl->num_strings; s++) {
		GnomePosString * string;
		gint i;

		string = pgl->strings + s;

		for (i = string->start; i < string->start + string->length; i++) {
			ArtDRect bbox;
			/* fixme: We should use inverse transform and font bbox here */
			if (gnome_rfont_get_glyph_stdbbox (string->rfont, pgl->glyphs[i].glyph, &bbox)) {
				gdouble gx, gy;
				gx = x - pgl->glyphs[i].x;
				gy = y - pgl->glyphs[i].y;
				if ((gx >= bbox.x0) && (gy >= bbox.y0) && (gx <= bbox.x1) && (gy <= bbox.y1))
					return TRUE;
			}
		}
	}

	return FALSE;
}

void
gnome_pgl_render_rgba8 (const GnomePosGlyphList * pgl,
			gdouble x, gdouble y,
			guchar * buf,
			gint width, gint height, gint rowstride,
			guint flags)
{
	gint s;
	gint i;

	g_return_if_fail (pgl != NULL);
	g_return_if_fail (buf != NULL);

	for (s = 0; s < pgl->num_strings; s++) {
		GnomePosString * string;
		string = pgl->strings + s;
		for (i = string->start; i < string->start + string->length; i++) {
			gnome_rfont_render_glyph_rgba8 (string->rfont, pgl->glyphs[i].glyph,
							string->color,
							x + pgl->glyphs[i].x, y + pgl->glyphs[i].y,
							buf,
							width, height, rowstride,
							flags);
		}
	}
}

void
gnome_pgl_render_rgb8 (const GnomePosGlyphList * pgl,
		       gdouble x, gdouble y,
		       guchar * buf,
		       gint width, gint height, gint rowstride,
		       guint flags)
{
	gint s;
	gint i;

	g_return_if_fail (pgl != NULL);
	g_return_if_fail (buf != NULL);

	for (s = 0; s < pgl->num_strings; s++) {
		GnomePosString * string;
		string = pgl->strings + s;
		for (i = string->start; i < string->start + string->length; i++) {
			gnome_rfont_render_glyph_rgb8 (string->rfont, pgl->glyphs[i].glyph,
						       string->color,
						       x + pgl->glyphs[i].x, y + pgl->glyphs[i].y,
						       buf,
						       width, height, rowstride,
						       flags);
		}
	}
}


/**
 * gnome_glyphlist_bbox:
 * @gl: 
 * @transform: 
 * @flags: should be 0 for now
 * @bbox:
 *
 * Get ink dimensions of transformed glyphlist
 * Flags are to specify user preferences, should be 0 for now
 * 
 * Return Value: bbox, NULL on error
 **/
ArtDRect *
gnome_glyphlist_bbox (const GnomeGlyphList *gl, const gdouble *transform, gint flags, ArtDRect *bbox)
{
	GnomePosGlyphList *pgl;
	ArtDRect *b;

	pgl = gnome_pgl_from_gl (gl, transform, flags);
	b = gnome_pgl_bbox (pgl, bbox);
	gnome_pgl_destroy (pgl);

	return b;
}

