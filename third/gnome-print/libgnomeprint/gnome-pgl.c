#define _GNOME_PGL_C_

/*
 * WARNING! EXPERIMENTAL API - USE ONLY IF YOU KNOW WHAT YOU ARE DOING!
 *
 * GnomePosGlyphList - Positioned glyphlist
 *
 * Author:
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 2000 Helix Code, Inc.
 *
 */

#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gnome-pgl.h>
#include <libgnomeprint/gnome-pgl-private.h>
#include <libgnomeprint/gnome-glyphlist-private.h>

GnomePosGlyphList *
gnome_pgl_from_gl (const GnomeGlyphList * gl, gdouble * transform, guint flags)
{
	GnomePosGlyphList * pgl;
	GnomePosString * string;
	ArtPoint abspos, relpos, pos;
	gdouble kerning, letterspace;
	guint32 color;
	gboolean advance, absset, relset;
	gint i;
	GSList * l, * sx, * sy;

	g_return_val_if_fail (gl != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_GLYPHLIST (gl), NULL);
	g_return_val_if_fail (transform != NULL, NULL);

	pgl = g_new (GnomePosGlyphList, 1);
	pgl->glyphs = g_new (GnomePosGlyph, gl->length);
	pgl->strings = NULL;

	string = NULL;

	pos.x = transform[4];
	pos.y = transform[5];

	abspos.x = abspos.y = 0.0;
	advance = TRUE;
	kerning = 0.0;
	letterspace = 0.0;
	color = 0x000000ff;
	sx = sy = NULL;

	for (i = 0; i < gl->length; i++) {
		relpos.x = relpos.y = 0.0;
		absset = relset = FALSE;
		for (l = gl->glyphs[i].info; l != NULL; l = l->next) {
			GGLInfo * info;
			info = (GGLInfo *) l->data;
			switch (info->type) {
			case GNOME_GL_ADVANCE:
				advance = info->value.bval;
				break;
			case GNOME_GL_MOVETOX:
				abspos.x = info->value.dval;
				absset = TRUE;
				break;
			case GNOME_GL_MOVETOY:
				abspos.y = info->value.dval;
				absset = TRUE;
				break;
			case GNOME_GL_RMOVETOX:
				relpos.x = info->value.dval;
				relset = TRUE;
				break;
			case GNOME_GL_RMOVETOY:
				relpos.y = info->value.dval;
				relset = TRUE;
				break;
			case GNOME_GL_PUSHCPX:
				break;
			case GNOME_GL_PUSHCPY:
				break;
			case GNOME_GL_POPCPX:
				break;
			case GNOME_GL_POPCPY:
				break;
			case GNOME_GL_FONT:
				string = g_new (GnomePosString, 1);
				pgl->strings = g_slist_prepend (pgl->strings, string);
				string->rfont = gnome_font_get_rfont (info->value.font, transform);
				string->glyphs = &pgl->glyphs[i];
				string->length = 0;
				break;
			case GNOME_GL_COLOR:
				color = info->value.color;
				break;
			case GNOME_GL_KERNING:
				kerning = info->value.dval;
				break;
			case GNOME_GL_LETTERSPACE:
				letterspace = info->value.dval;
				break;
			}
		}
		if (!string) {
			g_warning ("No font specified");
			g_free (pgl->glyphs);
			g_free (pgl);
			return NULL;
		}
		/* Position calculation */
		if (absset) {
			art_affine_point (&pos, &abspos, transform);
		} else if (relset) {
			pos.x += relpos.x * transform[0] + relpos.y * transform[2];
			pos.y += relpos.x * transform[1] + relpos.y * transform[3];
		} else {
			if (kerning > 0.0) {
				/* fixme: */
			}
			if (letterspace != 0.0) {
				ArtPoint dir;
				gnome_rfont_get_stdadvance (string->rfont, &dir);
				pos.x += letterspace * dir.x;
				pos.y += letterspace * dir.y;
			}
		}
		string->length++;
		pgl->glyphs[i].glyph = gl->glyphs[i].glyph;
		pgl->glyphs[i].x = pos.x;
		pgl->glyphs[i].y = pos.y;
		pgl->glyphs[i].color = color;
		/* Position advancement */
		if (advance) {
			ArtPoint adv;
			gnome_rfont_get_glyph_stdadvance (string->rfont, pgl->glyphs[i].glyph, &adv);
			pos.x += adv.x;
			pos.y += adv.y;
		}
	}

	return pgl;
}

void
gnome_pgl_destroy (GnomePosGlyphList * pgl)
{
	g_return_if_fail (pgl != NULL);

	if (pgl->glyphs) g_free (pgl->glyphs);

	while (pgl->strings) {
		GnomePosString * string;
		string = (GnomePosString *) pgl->strings->data;
		gnome_rfont_unref (string->rfont);
		g_free (string);
		pgl->strings = g_slist_remove (pgl->strings, pgl->strings->data);
	}

	g_free (pgl);
}

ArtDRect *
gnome_pgl_bbox (const GnomePosGlyphList * pgl, ArtDRect * bbox)
{
	GSList * l;

	g_return_val_if_fail (pgl != NULL, NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	bbox->x0 = bbox->y0 = 1.0;
	bbox->x1 = bbox->y1 = 0.0;

	for (l = pgl->strings; l != NULL; l = l->next) {
		GnomePosString * string;
		gint i;
		string = (GnomePosString *) l->data;
		for (i = 0; i < string->length; i++) {
			ArtDRect gbox;
			gnome_rfont_get_glyph_stdbbox (string->rfont, string->glyphs[i].glyph, &gbox);
			art_drect_union (bbox, bbox, &gbox);
		}
	}

	return bbox;
}






