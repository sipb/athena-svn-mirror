/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-fontmap.c: fontmap implementation
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
 *    Chema Celorio <chema@celorio.com>
 *    Tambet Ingo <tambet@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc.
 */

/* TODO: Recycle font entries, if they are identical for different maps */

#include <config.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <fontconfig/fontconfig.h>
#include <libgnomeprint/gnome-fontmap.h>

static GPFontMap *gp_fontmap_load (void);

static void gp_fontmap_ref (GPFontMap * map);
static void gp_fontmap_unref (GPFontMap * map);
static void gp_family_entry_ref (GPFamilyEntry * entry);
static void gp_family_entry_unref (GPFamilyEntry * entry);

static gint gp_fe_sortname (gconstpointer a, gconstpointer b);
static gint gp_fe_sortspecies (gconstpointer a, gconstpointer b);
static gint gp_familyentry_sortname (gconstpointer a, gconstpointer b);

/* Fontlist -> FontMap */
static GHashTable * fontlist2map = NULL;
/* Familylist -> FontMap */
static GHashTable * familylist2map = NULL;

#define my_g_free(x) if(x)g_free(x)

GPFontMap *
gp_fontmap_get (void)
{
	static GPFontMap *map = NULL;

	if (!map)
		map = gp_fontmap_load ();

	map->refcount++;

	return map;
}

void
gp_fontmap_release (GPFontMap * map)
{
	gp_fontmap_unref (map);
}

static void
gp_fontmap_ref (GPFontMap * map)
{
	g_return_if_fail (map != NULL);

	map->refcount++;
}

static void
gp_fontmap_unref (GPFontMap * map)
{
	g_return_if_fail (map != NULL);

	if (--map->refcount < 1) {
		if (map->familydict)
			g_hash_table_destroy (map->familydict);
		if (map->fontdict)
			g_hash_table_destroy (map->fontdict);
		if (map->filenamedict)
			g_hash_table_destroy (map->filenamedict);
		if (map->familylist) {
			g_hash_table_remove (familylist2map, map->familylist);
			g_list_free (map->familylist);
		}
		if (map->fontlist) {
			g_hash_table_remove (fontlist2map, map->fontlist);
			g_list_free (map->fontlist);
		}
		while (map->families) {
			gp_family_entry_unref ((GPFamilyEntry *) map->families->data);
			map->families = g_slist_remove (map->families, map->families->data);
		}
		while (map->fonts) {
			gp_font_entry_unref ((GPFontEntry *) map->fonts->data);
			map->fonts = g_slist_remove (map->fonts, map->fonts->data);
		}
		g_free (map);
		map = NULL;
	}
}

static void
gp_family_entry_ref (GPFamilyEntry * entry)
{
	entry->refcount++;
}

static void
gp_family_entry_unref (GPFamilyEntry * entry)
{
	if (--entry->refcount < 1) {
		if (entry->name)
			g_free (entry->name);
		if (entry->fonts)
			g_slist_free (entry->fonts);
		g_free (entry);
	}
}

static GnomeFontWeight
convert_fc_weight (gint i)
{
	if (i < FC_WEIGHT_LIGHT)
		return GNOME_FONT_LIGHTEST;
	if (i < (FC_WEIGHT_LIGHT + FC_WEIGHT_MEDIUM) / 2)
		return GNOME_FONT_LIGHT;
	if (i < (FC_WEIGHT_MEDIUM + FC_WEIGHT_DEMIBOLD) / 2)
		return GNOME_FONT_REGULAR;
	if (i < (FC_WEIGHT_DEMIBOLD + FC_WEIGHT_BOLD) / 2)
		return GNOME_FONT_DEMI;
	if (i < (FC_WEIGHT_BOLD + FC_WEIGHT_BLACK) / 2)
		return GNOME_FONT_BOLD;

	return GNOME_FONT_HEAVIEST;
}

static GPFontEntry *
fcpattern_to_gp_font_entry (FcPattern *font)
{
	GPFontEntryType font_type = GP_FONT_ENTRY_UNKNOWN;
	GPFontEntry *e = NULL;
	FcResult result;
	FcChar8 *fc_family, *fc_style, *dup_fc_style = NULL, *fc_file;
	int italic_angle, weight, face_index;
	gboolean outline;
	gchar *c;

	result = FcPatternGetBool (font, FC_OUTLINE, 0, &outline);
	if (!outline)
		return NULL;

	result = FcPatternGetString (font, FC_FAMILY, 0, &fc_family);
	if (result != FcResultMatch || fc_family == NULL) {
		g_warning ("Can't create GPFontEntry, FC_FAMILY not found\n");
		return NULL;
	}

	result = FcPatternGetString (font, FC_STYLE, 0, &fc_style);
	if (result != FcResultMatch || fc_style == NULL) {
		int fc_slant, fc_weight;	
		int res_slant = FcPatternGetInteger (font, FC_SLANT, 0, &fc_slant);
		int res_weight = FcPatternGetInteger (font, FC_WEIGHT, 0, &fc_weight);
		if (res_slant == FcResultMatch && res_weight == FcResultMatch) {
			switch (fc_slant) {
				case FC_SLANT_ITALIC: 
						if (fc_weight >= FC_WEIGHT_BOLD)
							dup_fc_style = (FcChar8*) g_strdup ("Bold Italic");
						else
							dup_fc_style = (FcChar8*) g_strdup ("Italic");
						break;	
				case FC_SLANT_OBLIQUE:
						dup_fc_style = (FcChar8*) g_strdup ("Bold Italic");
						break;	
				default:
						if (fc_weight >= FC_WEIGHT_BOLD)
							dup_fc_style = (FcChar8*) g_strdup ("Bold");
						else
							dup_fc_style = (FcChar8*) g_strdup ("Regular");
						
			}
		} else {
			dup_fc_style = (FcChar8*) g_strdup ("Regular");
		}
	}

	result = FcPatternGetString (font, FC_FILE, 0, &fc_file);
	if (result != FcResultMatch || fc_file == NULL) {
		g_warning ("Can't create GPFontEntry for %s-%s, FC_FILE not found\n", fc_family, dup_fc_style ? dup_fc_style : fc_style);
		return NULL;
	}

	face_index = 0;
	result = FcPatternGetInteger (font, FC_INDEX, 0, &face_index);

	italic_angle = 0;
	FcPatternGetInteger (font, FC_SLANT, 0, &italic_angle);

	/* Determining the font type based on the extension is not very clean, pango uses
	 *     strcmp (FT_MODULE_CLASS (face->driver)->module_name, "truetype") == 0;
	 * which isn't prety either (Chema)
 	 */
	c = strrchr ((gchar *) fc_file, '.');
	if (!c)
		return NULL;
	else if (strcasecmp (c, ".pfb") == 0)
		font_type = GP_FONT_ENTRY_TYPE1;
	else if (strcasecmp (c, ".pfa") == 0)
		font_type = GP_FONT_ENTRY_TYPE1;
	else if (strcasecmp (c, ".ttf") == 0)
		font_type = GP_FONT_ENTRY_TRUETYPE;
	else if (strcasecmp (c, ".ttc") == 0)
		font_type = GP_FONT_ENTRY_TRUETYPE;
	else if (strcasecmp (c, ".font") == 0) /* See #102400 */
		font_type = GP_FONT_ENTRY_TRUETYPE;
	else {
		return NULL;
	}

	e = g_new0 (GPFontEntry, 1);
	e->type         = font_type;
	e->file         = g_strdup ((gchar *) fc_file);
	e->index        = face_index;
	e->refcount     = 1;
	e->face         = NULL;
	e->speciesname  = dup_fc_style ? (gchar *) dup_fc_style : g_strdup ((gchar *) fc_style);
	e->familyname   = g_strdup ((gchar *) fc_family);
	e->name         = g_strdup_printf ("%s %s", e->familyname, e->speciesname);
	e->italic_angle = italic_angle;
	e->is_alias     = FALSE;
	e->unused       = NULL;
	
	result = FcPatternGetInteger (font, FC_WEIGHT, 0, &weight);
	if (result == FcResultMatch)
		e->Weight = convert_fc_weight (weight);
	else
		e->Weight = GNOME_FONT_REGULAR;

	return e;
}

static GPFontEntry *
fcpattern_to_gp_font_entry_alias (FcPattern *font, const gchar *name)
{
	GPFontEntry *e;
	gint len;

	e = fcpattern_to_gp_font_entry (font);
	if (e == NULL)
		return NULL;

	g_free (e->name);
	g_free (e->familyname);

	len = strchr (name, ' ') - (char *) name;

	e->is_alias   = TRUE;
	e->name       = g_strdup (name);
	e->familyname = g_strndup (name, len);

	return e;
}

typedef struct {
	const gchar *name;
	const gchar *family;
	gint slant;
	gint weight;
} GPAliasInfo;

/**
 * gp_fontmap_add_aliases:
 * @map: 
 * 
 * Add well known aliases to our fontmap
 **/
static void
gp_fontmap_add_aliases (GPFontMap *map)
{
	FcPattern *match_pattern, *result_pattern;
	GPFontEntry *e;
	GPAliasInfo aliases[] = {
		{ "Sans Regular",          "sans-serif", FC_SLANT_ROMAN,  FC_WEIGHT_MEDIUM },
		{ "Sans Bold",             "sans-serif", FC_SLANT_ROMAN,  FC_WEIGHT_BOLD },
		{ "Sans Italic",           "sans-serif", FC_SLANT_ITALIC, FC_WEIGHT_MEDIUM },
		{ "Sans Bold Italic",      "sans-serif", FC_SLANT_ITALIC, FC_WEIGHT_BOLD },
		{ "Serif Regular",         "serif",      FC_SLANT_ROMAN,  FC_WEIGHT_MEDIUM },
		{ "Serif Bold",            "serif",      FC_SLANT_ROMAN,  FC_WEIGHT_BOLD },
		{ "Serif Italic",          "serif",      FC_SLANT_ITALIC, FC_WEIGHT_MEDIUM },
		{ "Serif Bold Italic",     "serif",      FC_SLANT_ITALIC, FC_WEIGHT_BOLD },
		{ "Monospace Regular",     "monospace",  FC_SLANT_ROMAN,  FC_WEIGHT_MEDIUM },
		{ "Monospace Bold",        "monospace",  FC_SLANT_ROMAN,  FC_WEIGHT_BOLD },
		{ "Monospace Italic",      "monospace",  FC_SLANT_ITALIC, FC_WEIGHT_MEDIUM },
		{ "Monospace Bold Italic", "monospace",  FC_SLANT_ITALIC, FC_WEIGHT_BOLD },
		{ NULL }
	};
	FcResult res;
	gint i;

	for (i = 0; aliases[i].name; i++) {
		match_pattern = FcPatternBuild (NULL,
						FC_FAMILY, FcTypeString,  aliases[i].family,
						FC_SLANT,  FcTypeInteger, aliases[i].slant,
						FC_WEIGHT, FcTypeInteger, aliases[i].weight,
						NULL);
		FcConfigSubstitute (NULL, match_pattern, FcMatchPattern);
		FcDefaultSubstitute (match_pattern);

		if (!match_pattern) {
			g_warning ("Could not create match patern for alias %s\n", aliases[i].name);
			continue;
		}
		
		result_pattern = FcFontMatch (NULL, match_pattern, &res);

		if (!result_pattern) {
			g_warning ("Could not create result patern for alias %s\n", aliases[i].name);
			FcPatternDestroy (match_pattern);
			continue;
		}
		
		e = fcpattern_to_gp_font_entry_alias (result_pattern, aliases[i].name);
		if (e) {
			g_hash_table_insert (map->fontdict, e->name, e);
			map->num_fonts++;
			map->fonts = g_slist_prepend (map->fonts, e);
		}
		FcPatternDestroy (result_pattern);
		FcPatternDestroy (match_pattern);
	}
}

static void
gp_fontmap_load_fontconfig (GPFontMap *map)
{
	FcFontSet *fontset;
	FcPattern *font;
	GPFontEntry *e;
	int i;

	fontset = FcConfigGetFonts (NULL, FcSetSystem);
	if (fontset == NULL) {
		return;
	}

	for (i = 0; i < fontset->nfont; i++) {
		font = fontset->fonts[i];

		e = fcpattern_to_gp_font_entry (font);
		if (e) {
			g_hash_table_insert (map->fontdict, e->name, e);
			g_hash_table_insert (map->filenamedict, e, e);
			map->num_fonts++;
			map->fonts = g_slist_prepend (map->fonts, e);
		}
	}
}

static void
gp_fontmap_sort (GPFontMap *map)
{
	GSList * l;

	/* Sort fonts */
	map->fonts = g_slist_sort (map->fonts, gp_fe_sortname);

	/* Sort fonts into familia */
	for (l = map->fonts; l != NULL; l = l->next) {
		GPFontEntry * e;
		GPFamilyEntry * f;
		e = (GPFontEntry *) l->data;
		f = g_hash_table_lookup (map->familydict, e->familyname);
		if (!f) {
			f = g_new0 (GPFamilyEntry, 1);
			gp_family_entry_ref (f);
			f->name     = g_strdup (e->familyname);
			f->fonts    = g_slist_prepend (f->fonts, e);
			f->is_alias = e->is_alias;
			g_hash_table_insert (map->familydict, f->name, f);
			map->families = g_slist_prepend (map->families, f);
		} else {
			if (f->is_alias != e->is_alias) {
				g_warning ("Family %s contains alias and "
					   "non-alias entries",  f->name);
			}
			f->fonts = g_slist_prepend (f->fonts, e);
		}
	}

	/* Sort familia alphabetically */
	map->families = g_slist_sort (map->families, gp_familyentry_sortname);

	/* Sort fonts inside familia */
	for (l = map->families; l != NULL; l = l->next) {
		GPFamilyEntry * f;
		f = (GPFamilyEntry *) l->data;
		f->fonts = g_slist_sort (f->fonts, gp_fe_sortspecies);
	}
}

static guint
filename_hash (gconstpointer v)
{
	const GPFontEntry *e = v;

	return g_str_hash (e->file) ^ (e->index << 16);
}

static gboolean
filename_equal (gconstpointer v1,
		gconstpointer v2)
{
	const GPFontEntry *e1 = v1;
	const GPFontEntry *e2 = v2;

	return (e1->index == e2->index) && strcmp (e1->file, e2->file) == 0;
}

static GPFontMap *
gp_fontmap_load (void)
{
	GPFontMap *map;

	map = g_new0 (GPFontMap, 1);
	map->refcount  = 1;
	map->num_fonts = 0;
	map->fontdict   = g_hash_table_new (g_str_hash, g_str_equal);
	map->filenamedict = g_hash_table_new (filename_hash, filename_equal);
	map->familydict = g_hash_table_new (g_str_hash, g_str_equal);

	gp_fontmap_load_fontconfig (map);
	if (map->fonts == NULL) {
		g_warning ("No fonts could be loaded into fontmap.");
		return map;
	}

	gp_fontmap_add_aliases (map);
	gp_fontmap_sort (map);

	return map;
}

void
gp_font_entry_ref (GPFontEntry * entry)
{
	g_return_if_fail (entry != NULL);
	/* refcount can be 1 or 2 at this moment */
	g_return_if_fail (entry->refcount > 0);
	g_return_if_fail (entry->refcount < 2);

	entry->refcount++;
}

void
gp_font_entry_unref (GPFontEntry * entry)
{
	g_return_if_fail (entry != NULL);
	/* refcount can be 1 or 2 at this moment */
	g_return_if_fail (entry->refcount > 0);
	g_return_if_fail (entry->refcount < 3);

	if (--entry->refcount < 1) {
		g_return_if_fail (entry->face == NULL);

		my_g_free (entry->name);
		my_g_free (entry->familyname);
		my_g_free (entry->speciesname);
		my_g_free (entry->file);
		g_free (entry);
	}
}

/*
 * Font list stuff
 *
 * We use Hack'O'Hacks here:
 * Getting list saves list->fontmap mapping and refs fontmap
 * Freeing list releases mapping and frees fontmap
 */

GList *
gnome_font_list (void)
{
	GPFontMap * map;
	GSList * l;

	map = gp_fontmap_get ();

	if (!map->fontlist) {
		for (l = map->fonts; l != NULL; l = l->next) {
			GPFontEntry * e;
			e = (GPFontEntry *) l->data;
			map->fontlist = g_list_prepend (map->fontlist, e->name);
		}
		map->fontlist = g_list_reverse (map->fontlist);
		if (!fontlist2map)
			fontlist2map = g_hash_table_new (NULL, NULL);
		g_hash_table_insert (fontlist2map, map->fontlist, map);
	}

	return map->fontlist;
}

void
gnome_font_list_free (GList * fontlist)
{
	GPFontMap * map;

	g_return_if_fail (fontlist != NULL);

	map = g_hash_table_lookup (fontlist2map, fontlist);
	g_return_if_fail (map != NULL);

	gp_fontmap_unref (map);
}

GList *
gnome_font_family_list (void)
{
	GPFontMap * map;
	GSList * l;

	map = gp_fontmap_get ();

	if (!map->familylist) {
		for (l = map->families; l != NULL; l = l->next) {
			GPFamilyEntry * f;
			f = (GPFamilyEntry *) l->data;
			if (f->is_alias)
				continue;
			map->familylist = g_list_prepend (map->familylist, f->name);
		}
		map->familylist = g_list_reverse (map->familylist);
		if (!familylist2map)
			familylist2map = g_hash_table_new (NULL, NULL);
		g_hash_table_insert (familylist2map, map->familylist, map);
	}

	gp_fontmap_ref (map);

	gp_fontmap_release (map);

	return map->familylist;
}

void
gnome_font_family_list_free (GList * fontlist)
{
	GPFontMap * map;

	g_return_if_fail (fontlist != NULL);

	map = g_hash_table_lookup (familylist2map, fontlist);
	g_return_if_fail (map != NULL);

	gp_fontmap_unref (map);
}

static gint
gp_fe_sortname (gconstpointer a, gconstpointer b)
{
	return strcasecmp (((GPFontEntry *) a)->name, ((GPFontEntry *) b)->name);
}

static gint
gp_fe_sortspecies (gconstpointer a, gconstpointer b)
{
	if (((GPFontEntry *)a)->speciesname == NULL)
		return -1;
	if (((GPFontEntry *)b)->speciesname == NULL)
		return 1;
	
	return strcasecmp (((GPFontEntry *) a)->speciesname, ((GPFontEntry *) b)->speciesname);
}

static gint
gp_familyentry_sortname (gconstpointer a, gconstpointer b)
{
	return strcasecmp (((GPFamilyEntry *) a)->name, ((GPFamilyEntry *) b)->name);
}

GnomeFontWeight
gp_fontmap_lookup_weight (const gchar * weight)
{
	static GHashTable * weights = NULL;
	GnomeFontWeight wcode;

	if (!weights) {
		weights = g_hash_table_new (g_str_hash, g_str_equal);

		g_hash_table_insert (weights, "Extra Light", GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));
		g_hash_table_insert (weights, "Extralight",  GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));

		g_hash_table_insert (weights, "Thin",        GINT_TO_POINTER (GNOME_FONT_THIN));

		g_hash_table_insert (weights, "Light",       GINT_TO_POINTER (GNOME_FONT_LIGHT));

		g_hash_table_insert (weights, "Book",        GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (weights, "Roman",       GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (weights, "Regular",     GINT_TO_POINTER (GNOME_FONT_BOOK));

		g_hash_table_insert (weights, "Medium",      GINT_TO_POINTER (GNOME_FONT_MEDIUM));

		g_hash_table_insert (weights, "Semi",        GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Semibold",    GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Demi",        GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Demibold",    GINT_TO_POINTER (GNOME_FONT_SEMI));

		g_hash_table_insert (weights, "Bold",        GINT_TO_POINTER (GNOME_FONT_BOLD));

		g_hash_table_insert (weights, "Heavy",       GINT_TO_POINTER (GNOME_FONT_HEAVY));
 
		g_hash_table_insert (weights, "Extra",       GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));
		g_hash_table_insert (weights, "Extra Bold",  GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));

		g_hash_table_insert (weights, "Black",       GINT_TO_POINTER (GNOME_FONT_BLACK));

		g_hash_table_insert (weights, "Extra Black", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
		g_hash_table_insert (weights, "Extrablack",  GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
		g_hash_table_insert (weights, "Ultra Bold",  GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
	};

	wcode = GPOINTER_TO_INT (g_hash_table_lookup (weights, weight));

	return wcode;
}
