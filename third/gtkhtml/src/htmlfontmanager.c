/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.

   Copyright (C) 2000 Helix Code, Inc.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <string.h>

#include "gtkhtmlfontstyle.h"

#include "htmlfontmanager.h"
#include "htmlpainter.h"

static void
html_font_set_init (HTMLFontSet *set, gchar *face)
{
	memset (set, 0, GTK_HTML_FONT_STYLE_MAX_FONT * sizeof (HTMLFont *));
	set->ref_count = 1;
	set->face = g_strdup (face);
}

static HTMLFontSet *
html_font_set_new (gchar *face)
{
	HTMLFontSet *set;

	set = g_new (HTMLFontSet, 1);
	html_font_set_init (set, face);

	return set;
}

static gboolean
html_font_set_face (HTMLFontSet *set, gchar *face)
{
	if (!set->face || strcmp (set->face, face)) {
		if (set->face)
			g_free (set->face);
		set->face = g_strdup (face);
		return TRUE;
	}
	return FALSE;
}

static void
html_font_set_release (HTMLFontSet *set, HTMLPainter *painter)
{
	gint i;

	for (i=0; i<GTK_HTML_FONT_STYLE_MAX_FONT; i++) {
		if (set->font [i])
			html_painter_unref_font (painter, set->font [i]);
		set->font [i] = NULL;
	}
}

static void
html_font_set_unref (HTMLFontSet *set, HTMLPainter *painter)
{
	set->ref_count --;
	if (!set->ref_count) {
		html_font_set_release (set, painter);
		if (set->face)
			g_free (set->face);

		g_free (set);
	}
}

void
html_font_manager_init (HTMLFontManager *manager, HTMLPainter *painter)
{
	manager->font_sets     = g_hash_table_new (g_str_hash, g_str_equal);
	manager->var_size      = 12;
	manager->var_points    = FALSE;
	manager->fix_size      = 12;
	manager->fix_points    = FALSE;
	manager->magnification = 1.0;
	manager->painter       = painter;

	html_font_set_init (&manager->variable, NULL);
	html_font_set_init (&manager->fixed, NULL);
}

void
html_font_manager_set_magnification (HTMLFontManager *manager, gdouble magnification)
{
	g_return_if_fail (magnification > 0.0);

	if (magnification != manager->magnification) {
		manager->magnification = magnification;
		html_font_manager_clear_font_cache (manager);
	}
}

static gboolean
destroy_font_set_foreach (gpointer key, gpointer font_set, gpointer data)
{
	g_free (key);
	html_font_set_unref (font_set, HTML_PAINTER (data));

	return TRUE;
}

static void
clear_additional_font_sets (HTMLFontManager *manager)
{
	g_hash_table_foreach_remove (manager->font_sets, destroy_font_set_foreach, manager->painter);	
}

void
html_font_manager_clear_font_cache (HTMLFontManager *manager)
{
	html_font_set_release (&manager->variable, manager->painter);
	html_font_set_release (&manager->fixed, manager->painter);
	clear_additional_font_sets (manager);
}

void
html_font_manager_finalize (HTMLFontManager *manager)
{
	html_font_set_release (&manager->variable, manager->painter);
	html_font_set_release (&manager->fixed, manager->painter);
	g_free (manager->fixed.face);
	g_free (manager->variable.face);

	clear_additional_font_sets (manager);
	g_hash_table_destroy (manager->font_sets);
}

void
html_font_manager_set_default (HTMLFontManager *manager, gchar *variable, gchar *fixed,
			       gint var_size, gboolean var_points, gint fix_size, gboolean fix_points)
{
	gboolean changed = FALSE;

	/* variable width fonts */
	changed = html_font_set_face (&manager->variable, variable);
	if (manager->var_size != var_size || manager->var_points != var_points) {
		manager->var_size = var_size;
		manager->var_points = var_points;
		clear_additional_font_sets (manager);
		changed = TRUE;
	}
	if (changed) {
		html_font_set_release (&manager->variable, manager->painter);
	}
	changed = FALSE;

	/* fixed width fonts */
	changed = html_font_set_face (&manager->fixed, fixed);
	if (manager->fix_size != fix_size || manager->fix_points != fix_points) {
		manager->fix_size = fix_size;
		manager->fix_points = fix_points;
		changed = TRUE;
	}
	if (changed)
		html_font_set_release (&manager->fixed, manager->painter);
}

static gint
get_font_num (GtkHTMLFontStyle style)
{
	return (style == GTK_HTML_FONT_STYLE_DEFAULT)
		? GTK_HTML_FONT_STYLE_SIZE_3
		: (style & GTK_HTML_FONT_STYLE_MAX_FONT_MASK);
}

static gint
html_font_set_get_idx (GtkHTMLFontStyle style)
{
	return get_font_num (style) - 1;
}

static HTMLFontSet *
get_font_set (HTMLFontManager *manager, gchar *face, GtkHTMLFontStyle style)
{
	return (face)
		? g_hash_table_lookup (manager->font_sets, face)
		: ((style & GTK_HTML_FONT_STYLE_FIXED) ? &manager->fixed : &manager->variable);
}

static gdouble
get_real_font_size (HTMLFontManager *manager, GtkHTMLFontStyle style)
{
	gint size = (get_font_num (style) & GTK_HTML_FONT_STYLE_SIZE_MASK) -  GTK_HTML_FONT_STYLE_SIZE_3;
	gint base_size = style & GTK_HTML_FONT_STYLE_FIXED ? manager->fix_size : manager->var_size;

	return manager->magnification * (base_size + (size > 0 ? (1 << size) : size) * base_size/8.0);
}

static void
html_font_set_font (HTMLFontManager *manager, HTMLFontSet *set, GtkHTMLFontStyle style, HTMLFont *font)
{
	gint idx;

	g_assert (font);
	g_assert (set);

	/* set font in font set */
	idx = html_font_set_get_idx (style);
	if (set->font [idx] && font != set->font [idx])
		html_painter_unref_font (manager->painter, set->font [idx]);
	set->font [idx] = font;
}

static HTMLFont *
get_font (HTMLFontManager *manager, HTMLFontSet **set, gchar *face, GtkHTMLFontStyle style)
{
	HTMLFont *font = NULL;

	*set = get_font_set (manager, face, style);
	if (*set)
		font = (*set)->font [html_font_set_get_idx (style)];
	return font;
}

gchar *
html_font_manager_get_attr (gchar *font_name, gint n)
{
    gchar *s, *end;

    /* Search paramether */
    for (s=font_name; n; n--,s++)
	    s = strchr (s,'-');

    if (s && *s != 0) {
	    end = strchr (s, '-');
	    if (end)
		    return g_strndup (s, end - s);
	    else
		    return g_strdup (s);
    } else
	    return g_strdup ("Unknown");
}

static gchar *
get_name_from_face (HTMLFontManager *m, const gchar *face)
{
	gchar *enc1, *enc2, *rv;

	enc1 = html_font_manager_get_attr (m->variable.face, 13);
	enc2 = html_font_manager_get_attr (m->variable.face, 14);

	rv = g_strdup_printf ("-*-%s-*-*-*-*-*-*-*-*-*-*-%s-%s", face, enc1, enc2);

	g_free (enc1);
	g_free (enc2);

	return rv;
}

static gboolean
get_points (HTMLFontManager *manager, GtkHTMLFontStyle style)
{
	return (style & GTK_HTML_FONT_STYLE_FIXED) ? manager->fix_points : manager->var_points;
}

static gpointer
manager_alloc_font (HTMLFontManager *manager, const gchar *face, GtkHTMLFontStyle style)
{
	gchar *name = strcasecmp (face, manager->variable.face)
		&& strcasecmp (face, manager->fixed.face)
		? get_name_from_face (manager, face)
		: g_strdup (face);
	HTMLFont *font;

	font = html_painter_alloc_font (manager->painter, name,
					get_real_font_size (manager, style), get_points (manager, style), style);
	g_free (name);

	return font;
}

static gchar *
strip_white_space (gchar *name)
{
	gint end;
	while (name [0] == ' ' || name [0] == '\t')
		name ++;
	end = strlen (name);
	while (end && (name [end - 1] == ' ' || name [end - 1] == '\t')) {
		name [end - 1] = 0;
		end --;
	}

	return name;
}

static HTMLFont *
alloc_new_font (HTMLFontManager *manager, HTMLFontSet **set, gchar *face_list, GtkHTMLFontStyle style)
{
	HTMLFont *font = NULL;
	gchar   **faces;
	gchar   **face;

	if (!(*set)) {
		face = faces = g_strsplit (face_list, ",", 0);
		while (*face) {
			gchar *face_name = strip_white_space (*face);

			/* first try to get font from available sets */
			font = get_font (manager, set, face_name, style);
			if (!font)
				font = manager_alloc_font (manager, face_name, style);
			if (font) {
				if (!(*set)) {
					*set = html_font_set_new (face_name);
					g_hash_table_insert (manager->font_sets, g_strdup (face_name), *set);
				}
				if (strcmp (face_list, *face)) {
					(*set)->ref_count ++;
					g_hash_table_insert (manager->font_sets, g_strdup (face_list), *set);
				}
				break;
			}
			face++;
		}
		g_strfreev (faces);
		if (!(*set)) {
			/* none of faces exist, so create empty set for him and let manager later set fixed font here */
			*set = html_font_set_new (face_list);
			g_hash_table_insert (manager->font_sets, g_strdup (face_list), *set);
		}
	} else
		font = manager_alloc_font (manager, (*set)->face, style);

	if ((*set) && font)
		html_font_set_font (manager, (*set), style, font);

	return font;
}

HTMLFont *
html_font_manager_get_font (HTMLFontManager *manager, gchar *face_list, GtkHTMLFontStyle style)
{
	HTMLFontSet *set;
	HTMLFont *font = NULL;

	font = get_font (manager, &set, face_list, style);

	if (!font) {
		/* first try to alloc right one */
		font = alloc_new_font (manager, &set, face_list, style);
		if (!font) {
			g_assert (set);
			if (!face_list) {
				/* default font, so the last chance is to get fixed font */
				font = html_painter_alloc_font (manager->painter, NULL,
								get_real_font_size (manager, style),
								get_points (manager, style), style);
				if (!font)
					g_warning ("Cannot allocate fixed font\n");
			} else {
				/* some unavailable non-default font => use default one */
				font = html_font_manager_get_font (manager, NULL, style);
				html_font_ref (font, manager->painter);
			}
			if (font)
				html_font_set_font (manager, set, style, font);
		}
	}

	return font;
}

HTMLFont *
html_font_new (gpointer data, guint space_width)
{
	HTMLFont *font = g_new (HTMLFont, 1);

	font->data        = data;
	font->space_width = space_width;
	font->ref_count   = 1;

	return font;
}

void
html_font_destroy (HTMLFont *font)
{
	g_free (font);
}

void
html_font_ref (HTMLFont *font, HTMLPainter *painter)
{
	html_painter_ref_font (painter, font);
	font->ref_count ++;
}

void
html_font_unref (HTMLFont *font, HTMLPainter *painter)
{
	html_painter_unref_font (painter, font);
	font->ref_count --;
	if (font->ref_count <= 0)
		html_font_destroy (font);
}
