/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-font-family.c:
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
 *    Lauris Kaplinski <lauris@ariman.ee>
 *
 *  Copyright (C) 1999-2002 Ximian, Inc. and authors
 *
 */

#include <config.h>
#include <string.h>

#include <libgnomeprint/gnome-fontmap.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-font-family.h>

struct _GnomeFontFamily {
	GObject object;

	gchar * name;
};

struct _GnomeFontFamilyClass {
	GObjectClass parent_class;
};

static void gnome_font_family_class_init (GnomeFontFamilyClass * klass);
static void gnome_font_family_init (GnomeFontFamily * family);

static void gnome_font_family_finalize (GObject * object);

static GObjectClass * parent_class = NULL;

GType
gnome_font_family_get_type (void) {
	static GType family_type = 0;
	if (!family_type) {
		static const GTypeInfo family_info = {
			sizeof (GnomeFontFamilyClass),
			NULL, NULL,
			(GClassInitFunc) gnome_font_family_class_init,
			NULL, NULL,
			sizeof (GnomeFontFamily),
			0,
			(GInstanceInitFunc) gnome_font_family_init
		};
		family_type = g_type_register_static (G_TYPE_OBJECT, "GnomeFontFamily", &family_info, 0);
	}
	return family_type;
}

static void
gnome_font_family_class_init (GnomeFontFamilyClass * klass)
{
	GObjectClass * object_class;

	object_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_font_family_finalize;
}

static void
gnome_font_family_init (GnomeFontFamily * family)
{
	family->name = NULL;
}

static void
gnome_font_family_finalize (GObject * object)
{
	GnomeFontFamily * family;

	family = (GnomeFontFamily *) object;

	if (family->name) {
		g_free (family->name);
		family->name = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GnomeFontFamily *
gnome_font_family_new (const gchar * name)
{
	GnomeFontFamily * family;
	GPFontMap * map;
	GPFamilyEntry * f;

	g_return_val_if_fail (name != NULL, NULL);

	family = NULL;

	map = gp_fontmap_get ();

	f = g_hash_table_lookup (map->familydict, name);

	if (f) {
		family = g_object_new (GNOME_TYPE_FONT_FAMILY, NULL);
		family->name = g_strdup (name);
	}

	gp_fontmap_release (map);

	return family;
}

GList *
gnome_font_family_style_list (GnomeFontFamily * family)
{
	GPFontMap * map;
	GPFamilyEntry * f;
	GList * list;
	GHashTable *styles;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FAMILY (family), NULL);

	list = NULL;

	map = gp_fontmap_get ();
	styles = g_hash_table_new (g_str_hash, g_str_equal);

	f = g_hash_table_lookup (map->familydict, family->name);

	if (f) {
		GSList * l;
		for (l = f->fonts; l != NULL; l = l->next) {
			GPFontEntry *e = l->data;
			const gchar *style = e->speciesname;
			
			if (e->is_alias)
				continue;

			if (!g_hash_table_lookup (styles, style)) {
				g_hash_table_insert (styles, (gchar *) style, (gpointer) 1);
				list = g_list_prepend (list, g_strdup (e->speciesname));
			}
		}
		list = g_list_reverse (list);
	}

	g_hash_table_destroy (styles);
	gp_fontmap_release (map);

	return list;
}

void
gnome_font_family_style_list_free (GList * list)
{
	while (list) {
		g_free (list->data);
		list = g_list_remove (list, list->data);
	}
}

GnomeFontFace *
gnome_font_family_get_face_by_stylename (GnomeFontFamily * family, const gchar * style)
{
	GnomeFontFace * face;
	GPFontMap * map;
	GPFamilyEntry * f;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FAMILY (family), NULL);
	g_return_val_if_fail (style != NULL, NULL);

	face = NULL;

	map = gp_fontmap_get ();

	f = g_hash_table_lookup (map->familydict, family->name);

	if (f) {
		GSList * l;
		for (l = f->fonts; l != NULL; l = l->next) {
			GPFontEntry * e;
			e = (GPFontEntry *) l->data;
			if (!strcmp (style, e->speciesname)) {
				face = gnome_font_face_find (e->name);
			}
		}
	}

	gp_fontmap_release (map);

	return face;
}






