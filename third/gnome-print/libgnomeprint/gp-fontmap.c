#define _GP_FONTMAP_C_

/*
 * Fontmap implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * TODO: Recycle font entries, if they are identical for different maps
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <gnome-xml/parser.h>
#include <gnome-xml/xmlmemory.h>
#include "gp-fontmap.h"

/*
 * We parse following locations for fontmaps:
 *
 * $FONTMAPDIR_STATIC ($prefix/share/gnome/fonts)
 * $FONTMAPDIR_DYNAMIC ($sysconfdir/gnome/fonts)
 * $HOME/.gnome/fonts
 *
 */

static GPFontMap *gp_fontmap_load (void);
static void gp_fontmap_load_dir (GPFontMap *map, const guchar *dirname);
static void gp_fontmap_load_file (GPFontMap *map, const guchar *filename);
static void gp_fm_load_font_2_0_type1 (GPFontMap *map, xmlNodePtr node);
static void gp_fm_load_font_2_0_type1alias (GPFontMap *map, xmlNodePtr node);
static void gp_font_entry_2_0_load_data (GPFontEntry *e, xmlNodePtr node);
static void gp_font_entry_2_0_type1_load_files (GPFontEntryT1 *t1, xmlNodePtr node);
static void gp_fm_load_fonts_2_0 (GPFontMap * map, xmlNodePtr root);

static void gp_fontmap_ref (GPFontMap * map);
static void gp_fontmap_unref (GPFontMap * map);
static void gp_family_entry_ref (GPFamilyEntry * entry);
static void gp_family_entry_unref (GPFamilyEntry * entry);

static gchar * gp_xmlGetPropString (xmlNodePtr node, const gchar * name);
static gint gp_fe_sortname (gconstpointer a, gconstpointer b);
static gint gp_fe_sortspecies (gconstpointer a, gconstpointer b);
static gint gp_familyentry_sortname (gconstpointer a, gconstpointer b);
static gchar * gp_fm_get_species_name (const gchar * fullname, const gchar * familyname);
static gboolean gp_fm_is_changed (GPFontMap * map);

/* Fontlist -> FontMap */
static GHashTable * fontlist2map = NULL;
/* Familylist -> FontMap */
static GHashTable * familylist2map = NULL;

GPFontMap *
gp_fontmap_get (void)
{
	static GPFontMap *map = NULL;
	static time_t lastaccess = 0;

	if (map) {
		/* If > 1 sec is passed from last query, check timestamps */
		if ((time (NULL) > lastaccess) && gp_fm_is_changed (map)) {
			/* Any directory is changed, so force rereading of map */
			g_print ("Fontmap is changed, rereading\n");
			gp_fontmap_release (map);
			map = NULL;
		}
	}

	if (!map) {
		map = gp_fontmap_load ();
	}

	/* Save acess time */
	lastaccess = time (NULL);

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
		g_print ("Releasing fontmap\n");
		if (map->familydict) g_hash_table_destroy (map->familydict);
		if (map->fontdict) g_hash_table_destroy (map->fontdict);
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
		if (entry->name) g_free (entry->name);
		if (entry->fonts) g_slist_free (entry->fonts);
		g_free (entry);
	}
}

static GPFontMap *
gp_fontmap_load (void)
{
	GPFontMap *map;
	struct stat s;
	gchar *homedir, *mapdir;
	GSList * l;

	map = g_new (GPFontMap, 1);
	/* We always hold private ref to fontmap, this is released if directories change */
	map->refcount = 1;
	map->num_fonts = 0;
	/* Clear timestamps */
	map->mtime_static = 0;
	map->mtime_dynamic = 0;
	map->mtime_user = 0;
	map->fontdict = g_hash_table_new (g_str_hash, g_str_equal);
	map->familydict = g_hash_table_new (g_str_hash, g_str_equal);
	map->fonts = NULL;
	map->families = NULL;
	map->fontlist = NULL;
	map->familylist = NULL;

	/* User map */
	homedir = g_get_home_dir ();
	mapdir = g_concat_dir_and_file (homedir, ".gnome/fonts");
	if (!stat (mapdir, &s) && S_ISDIR (s.st_mode)) {
		map->mtime_user = s.st_mtime;
		gp_fontmap_load_dir (map, mapdir);
	}
	g_free (mapdir);
	/* Dynamic map */
	if (!stat (FONTMAPDIR_DYNAMIC, &s) && S_ISDIR (s.st_mode)) {
		map->mtime_dynamic = s.st_mtime;
		gp_fontmap_load_dir (map, FONTMAPDIR_DYNAMIC);
	}
	/* Static map */
	if (!stat (FONTMAPDIR_STATIC, &s) && S_ISDIR (s.st_mode)) {
		map->mtime_static = s.st_mtime;
		gp_fontmap_load_dir (map, FONTMAPDIR_STATIC);
	}

	/* Sanity check */
	if (map->num_fonts < 24) {
		/* Less than 24 fonts means, you do not have PS ones */
		if (!stat (DATADIR "/fonts/fontmap2", &s) && S_ISREG (s.st_mode)) {
			gp_fontmap_load_file (map, DATADIR "/fonts/fontmap2");
		}
	}
	/* More sanity check */
	if (map->num_fonts < 24) {
		gchar *filename;
		/* Less than 24 fonts means, you do not have PS ones */
		filename = g_concat_dir_and_file (g_get_home_dir (), ".gnome/fonts/fontmap");
		if (!stat (filename, &s) && S_ISREG (s.st_mode)) {
			gp_fontmap_load_file (map, filename);
		}
		g_free (filename);
	}

	/* Sort fonts alphabetically */
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
			f->name = g_strdup (e->familyname);
			f->fonts = g_slist_prepend (f->fonts, e);
			g_hash_table_insert (map->familydict, f->name, f);
			map->families = g_slist_prepend (map->families, f);
		} else {
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

	return map;
}

static gint
gp_fontmap_compare_names (gconstpointer a, gconstpointer b)
{
	if (!strcmp (a, "gnome-print.fontmap")) return -1;
	if (!strcmp (b, "gnome-print.fontmap")) return 1;
	return strcmp (a, b);
}

/* Tries to load all *.fontmap files from given directory */

static void
gp_fontmap_load_dir (GPFontMap *map, const guchar *dirname)
{
	DIR *dir;
	struct dirent *dent;

	dir = opendir (dirname);

	if (dir) {
		GSList *files;
		files = NULL;
		while ((dent = readdir (dir))) {
			gint len;
			len = strlen (dent->d_name);
			if ((len > 8) && !strcmp (dent->d_name + len - 8, ".fontmap")) {
				/* Seems to be what we are looking for */
				files = g_slist_prepend (files, g_strdup (dent->d_name));
			}
		}
		closedir (dir);
		/* Sort names alphabetically */
		files = g_slist_sort (files, gp_fontmap_compare_names);
		while (files) {
			struct stat s;
			gchar *filename;
			filename = g_concat_dir_and_file (dirname, (gchar *) files->data);
			g_free (files->data);
			if (!stat (filename, &s) && S_ISREG (s.st_mode)) {
				gp_fontmap_load_file (map, filename);
			}
			g_free (filename);
			files = g_slist_remove (files, files->data);
		}
	}
}

/* Parse file, and add fonts to map, if it is valid fontmap */

static void
gp_fontmap_load_file (GPFontMap *map, const guchar *filename)
{
	xmlDocPtr doc;

	doc = xmlParseFile (filename);
	if (doc) {
		xmlNodePtr root;
		/* In minimum we are valid xml file */
		root = xmlDocGetRootElement (doc);
		if (root && !strcmp (root->name, "fontmap")) {
			xmlChar *version;
			/* We are really fontmap */
			version = xmlGetProp (root, "version");
			if (version) {
				if (!strcmp (version, "2.0")) {
					xmlChar *test;
					gboolean try;
					/* We are even right version */
					try = TRUE;
					test = xmlGetProp (root, "test");
					if (test) {
						struct stat s;
						if (stat (test, &s) || !S_ISREG (s.st_mode)) try = FALSE;
						xmlFree (test);
					}
					if (try) gp_fm_load_fonts_2_0 (map, root);
				}
				xmlFree (version);
			}
		}
		xmlFreeDoc (doc);
	}
}

#if 0
static void
gp_fm_load_fonts (GPFontMap * map, xmlDoc * doc)
{
	xmlNodePtr root;

	root = xmlDocGetRootElement (doc);

	if (!strcmp (root->name, "fontmap")) {
		xmlChar * version;

		version = xmlGetProp (root, "version");

		if (!version) {
			/* Version 1.0 */
			gp_fm_load_fonts_1_0 (map, root);
		} else if (!strcmp (version, "2.0")) {
			/* Version 2.0 */
			xmlFree (version);
			gp_fm_load_fonts_2_0 (map, root);
		} else {
			xmlFree (version);
		}
	}
}

static void
gp_fm_load_fonts_1_0 (GPFontMap * map, xmlNodePtr root)
{
	xmlNodePtr child;

	child = root->xmlChildrenNode;

	while (child) {
		xmlChar * format;
		format = xmlGetProp (child, "format");
		if (format) {
			if (!strcmp (format, "type1")) {
				/* We are type1 entry */
				gp_fm_load_font_1_0 (map, child);
			}
			xmlFree (format);
		}
		child = child->next;
	}
}

static void
gp_fm_load_font_1_0 (GPFontMap * map, xmlNodePtr node)
{
	GPFontEntryT1 * t1;
	GPFontEntry * e;
	gchar * alias, * p;

	alias = gp_xmlGetPropString (node, "alias");

	if (alias) {
		GPFontEntryT1Alias * t1a;
		t1a = g_new0 (GPFontEntryT1Alias, 1);
		t1a->t1.entry.type = GP_FONT_ENTRY_TYPE1_ALIAS;
		t1a->alias = alias;
		t1 = (GPFontEntryT1 *) t1a;
	} else {
		t1 = g_new0 (GPFontEntryT1, 1);
		t1->entry.type = GP_FONT_ENTRY_TYPE1;
	}

	e = (GPFontEntry *) t1;

	e->refcount = 1;
	e->face = NULL;

	t1->afm.name = gp_xmlGetPropString (node, "metrics");
	t1->pfb.name = gp_xmlGetPropString (node, "glyphs");
	e->name = gp_xmlGetPropString (node, "fullname");
	e->version = gp_xmlGetPropString (node, "version");
	e->familyname = gp_xmlGetPropString (node, "familyname");
	e->psname = gp_xmlGetPropString (node, "name");

	if (!(t1->afm.name && t1->pfb.name && e->name && e->familyname && e->psname)) {
		gp_font_entry_unref (e);
		return;
	}

	/* fixme: check integrity */
	/* fixme: load afm */

	/* Read fontmap 1.0 weight string */

	e->weight = gp_xmlGetPropString (node, "weight");
	if (e->weight) {
		t1->Weight = gp_fontmap_lookup_weight (e->weight);
	} else {
		e->weight = g_strdup ("Book");
		t1->Weight = GNOME_FONT_BOOK;
	}

	/* Discover species name */

	e->speciesname = gp_fm_get_species_name (e->name, e->familyname);

	/* Parse Italic from species name */

	p = strstr (e->speciesname, "Italic");
	if (!p) p = strstr (e->speciesname, "Oblique");

	if (p) {
		t1->ItalicAngle = -10.0;
	} else {
		t1->ItalicAngle = 0.0;
	}

	/* fixme: fixme: fixme: */

	if (g_hash_table_lookup (map->fontdict, e->name)) {
		gp_font_entry_unref (e);
		return;
	}

	g_hash_table_insert (map->fontdict, e->name, e);
	map->num_fonts++;
	map->fonts = g_slist_prepend (map->fonts, e);
}
#endif

/* Parse root element and build fontmap step 1 */

static void
gp_fm_load_fonts_2_0 (GPFontMap * map, xmlNodePtr root)
{
	xmlNodePtr child;

	for (child = root->xmlChildrenNode; child != NULL; child = child->next) {
		if (!strcmp (child->name, "font")) {
			xmlChar *format;
			/* We are font */
			format = xmlGetProp (child, "format");
			if (format) {
				if (!strcmp (format, "type1")) {
					/* We are type1/type1alias entry */
					gp_fm_load_font_2_0_type1 (map, child);
				} else if (!strcmp (format, "type1alias")) {
					/* We are type1/type1alias entry */
					gp_fm_load_font_2_0_type1alias (map, child);
				}
				xmlFree (format);
			}
		}
	}
}

static void
gp_fm_load_font_2_0_type1 (GPFontMap *map, xmlNodePtr node)
{
	GPFontEntryT1 *t1;
	GPFontEntry *e;
	xmlChar *xmlname, *t;

	/* Get our unique name */
	xmlname = xmlGetProp (node, "name");
	/* Check, whether we are already registered */
	if (g_hash_table_lookup (map->fontdict, xmlname)) {
		xmlFree (xmlname);
		return;
	}

	t1 = g_new0 (GPFontEntryT1, 1);
	e = (GPFontEntry *) t1;

	e->type = GP_FONT_ENTRY_TYPE1;
	e->refcount = 1;
	e->face = NULL;
	e->name = g_strdup (xmlname);
	xmlFree (xmlname);

	gp_font_entry_2_0_load_data (e, node);
	gp_font_entry_2_0_type1_load_files (t1, node);
	if (!e->familyname || !e->psname || !t1->afm.name || !t1->pfb.name) {
		gp_font_entry_unref (e);
		return;
	}

	t1->Weight = gp_fontmap_lookup_weight (e->weight);

	if (!e->speciesname) {
		e->speciesname = gp_fm_get_species_name (e->name, e->familyname);
	}

	t = xmlGetProp (node, "italicangle");
	if (t == NULL) {
		gchar *p;
		p = strstr (e->speciesname, "Italic");
		if (!p) p = strstr (e->speciesname, "Oblique");
		t1->ItalicAngle = p ? -10.0 : 0.0;
	} else {
		t1->ItalicAngle = atof (t);
		xmlFree (t);
	}

	g_hash_table_insert (map->fontdict, e->name, e);
	map->num_fonts++;
	map->fonts = g_slist_prepend (map->fonts, e);
}

static void
gp_fm_load_font_2_0_type1alias (GPFontMap *map, xmlNodePtr node)
{
	GPFontEntryT1Alias *t1a;
	GPFontEntryT1 *t1;
	GPFontEntry *e;
	xmlChar *xmlname, *xmlalias, *t;

	/* Get our unique name */
	xmlname = xmlGetProp (node, "name");
	/* Check, whether we are already registered */
	if (g_hash_table_lookup (map->fontdict, xmlname)) {
		xmlFree (xmlname);
		return;
	}
	/* Get our alternate PS name */
	xmlalias = xmlGetProp (node, "alias");
	if (!xmlalias) {
		xmlFree (xmlname);
		return;
	}

	t1a = g_new0 (GPFontEntryT1Alias, 1);
	t1 = (GPFontEntryT1 *) t1a;
	e = (GPFontEntry *) t1a;

	e->type = GP_FONT_ENTRY_TYPE1_ALIAS;
	e->refcount = 1;
	e->face = NULL;
	e->name = g_strdup (xmlname);
	xmlFree (xmlname);
	t1a->alias = g_strdup (xmlalias);
	xmlFree (xmlalias);

	gp_font_entry_2_0_load_data (e, node);
	gp_font_entry_2_0_type1_load_files (t1, node);
	if (!e->familyname || !e->psname || !t1->afm.name || !t1->pfb.name) {
		gp_font_entry_unref (e);
		return;
	}

	t1->Weight = gp_fontmap_lookup_weight (e->weight);

	if (!e->speciesname) {
		e->speciesname = gp_fm_get_species_name (e->name, e->familyname);
	}

	t = xmlGetProp (node, "italicangle");
	if (t == NULL) {
		gchar *p;
		p = strstr (e->speciesname, "Italic");
		if (!p) p = strstr (e->speciesname, "Oblique");
		t1->ItalicAngle = p ? -10.0 : 0.0;
	} else {
		t1->ItalicAngle = atof (t);
		xmlFree (t);
	}

	g_hash_table_insert (map->fontdict, e->name, e);
	map->num_fonts++;
	map->fonts = g_slist_prepend (map->fonts, e);
}

/* Loads common font property data */

static void
gp_font_entry_2_0_load_data (GPFontEntry *e, xmlNodePtr node)
{
	/* fixme: We could do some checking here to save alloc/free calls */
	/* name is parsed by parent */
	e->version = gp_xmlGetPropString (node, "version");
	e->familyname = gp_xmlGetPropString (node, "familyname");
	e->speciesname = gp_xmlGetPropString (node, "speciesname");
	e->psname = gp_xmlGetPropString (node, "psname");
	/* Read Weight attribute */
	e->weight = gp_xmlGetPropString (node, "weight");
	if (!e->weight) e->weight = g_strdup ("Book");
}

/* Loads "afm" and "pfb" file nodes */

static void
gp_font_entry_2_0_type1_load_files (GPFontEntryT1 *t1, xmlNodePtr node)
{
	xmlNodePtr child;

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		/* Scan all children nodes */
		if (!strcmp (child->name, "file")) {
			xmlChar *type;
			/* We are <file> node */
			type = xmlGetProp (child, "type");
			if (!strcmp (type, "afm") && !t1->afm.name) {
				t1->afm.name = gp_xmlGetPropString (child, "path");
			} else if (!strcmp (type, "pfb") && !t1->pfb.name) {
				t1->pfb.name = gp_xmlGetPropString (child, "path");
			}
			xmlFree (type);
		}
		if (t1->afm.name && t1->pfb.name) return;
	}
}

#if 0
static void
gp_fm_load_font_2_0 (GPFontMap * map, xmlNodePtr node, gboolean alias)
{
	GPFontEntryT1 * t1;
	GPFontEntry * e;
	gchar * p;
	xmlNodePtr child;
	xmlChar * t;

	if (alias) {
		GPFontEntryT1Alias * t1a;
		t1a = g_new0 (GPFontEntryT1Alias, 1);
		t1a->t1.entry.type = GP_FONT_ENTRY_TYPE1_ALIAS;
		t1a->alias = gp_xmlGetPropString (node, "alias");
		t1 = (GPFontEntryT1 *) t1a;
	} else {
		t1 = g_new0 (GPFontEntryT1, 1);
		t1->entry.type = GP_FONT_ENTRY_TYPE1;
	}

	child = node->xmlChildrenNode;
	while (child) {
		if (!strcmp (child->name, "file")) {
			xmlChar * type;
			type = xmlGetProp (child, "type");
			if (!strcmp (type, "afm")) {
				t1->afm.name = gp_xmlGetPropString (child, "path");
				t = xmlGetProp (child, "size");
				if (t) t1->afm.size = atoi (t);
				if (t) xmlFree (t);
				t = xmlGetProp (child, "mtime");
				if (t) t1->afm.mtime = atoi (t);
				if (t) xmlFree (t);
			} else if (!strcmp (type, "pfb")) {
				t1->pfb.name = gp_xmlGetPropString (child, "path");
				t = xmlGetProp (child, "size");
				if (t) t1->pfb.size = atoi (t);
				if (t) xmlFree (t);
				t = xmlGetProp (child, "mtime");
				if (t) t1->pfb.mtime = atoi (t);
				if (t) xmlFree (t);
			}
			xmlFree (type);
		}
		if (t1->afm.name && t1->pfb.name) break;
		child = child->next;
	}

	e = (GPFontEntry *) t1;

	e->refcount = 1;
	e->face = NULL;

	if (!(t1->afm.name && t1->pfb.name)) {
		gp_font_entry_unref (e);
		return;
	}

	e->name = gp_xmlGetPropString (node, "name");
	e->version = gp_xmlGetPropString (node, "version");
	e->familyname = gp_xmlGetPropString (node, "familyname");
	e->speciesname = gp_xmlGetPropString (node, "speciesname");
	e->psname = gp_xmlGetPropString (node, "psname");

	if (!(e->name && e->familyname && e->psname)) {
		gp_font_entry_unref (e);
		return;
	}

	/* fixme: check integrity */
	/* fixme: load afm */

	/* Read fontmap 1.0 weight string */

	e->weight = gp_xmlGetPropString (node, "weight");
	if (e->weight) {
		t1->Weight = gp_fontmap_lookup_weight (e->weight);
	} else {
		e->weight = g_strdup ("Book");
		t1->Weight = GNOME_FONT_BOOK;
	}

	/* Discover species name */

	if (!e->speciesname) e->speciesname = gp_fm_get_species_name (e->name, e->familyname);

	/* Parse Italic */

	t = xmlGetProp (node, "italicangle");
	if (!t) {
		p = strstr (e->speciesname, "Italic");
		if (!p) p = strstr (e->speciesname, "Oblique");
		if (p) {
			t1->ItalicAngle = -10.0;
		} else {
			t1->ItalicAngle = 0.0;
		}
	} else {
		t1->ItalicAngle = atof (t);
		xmlFree (t);
	}

	/* fixme: fixme: fixme: */

	if (g_hash_table_lookup (map->fontdict, e->name)) {
		gp_font_entry_unref (e);
		return;
	}

	g_hash_table_insert (map->fontdict, e->name, e);
	map->num_fonts++;
	map->fonts = g_slist_prepend (map->fonts, e);
}

static void
gp_fm_load_aliases (GPFontMap * map, xmlDoc * doc)
{
}
#endif

/*
 * Font Entry stuff
 *
 * If face is created, it has to reference entry
 */

void
gp_font_entry_ref (GPFontEntry * entry)
{
	g_return_if_fail (entry != NULL);
	/* refcount can be 1 or 2 at moment */
	g_return_if_fail (entry->refcount > 0);
	g_return_if_fail (entry->refcount < 2);

	entry->refcount++;
}

void
gp_font_entry_unref (GPFontEntry * entry)
{
	g_return_if_fail (entry != NULL);
	/* refcount can be 1 or 2 at moment */
	g_return_if_fail (entry->refcount > 0);
	g_return_if_fail (entry->refcount < 3);

	if (--entry->refcount < 1) {
		GPFontEntryT1 * t1;
		GPFontEntryT1Alias * t1a;

		g_return_if_fail (entry->face == NULL);

		if (entry->name) g_free (entry->name);
		if (entry->version) g_free (entry->version);
		if (entry->familyname) g_free (entry->familyname);
		if (entry->speciesname) g_free (entry->speciesname);
		if (entry->psname) g_free (entry->psname);
		if (entry->weight) g_free (entry->weight);

		switch (entry->type) {
		case GP_FONT_ENTRY_TYPE1_ALIAS:
			t1a = (GPFontEntryT1Alias *) entry;
			if (t1a->alias) g_free (t1a->alias);
		case GP_FONT_ENTRY_TYPE1:
			t1 = (GPFontEntryT1 *) entry;
			if (t1->afm.name) g_free (t1->afm.name);
			if (t1->pfb.name) g_free (t1->pfb.name);
			break;
		case GP_FONT_ENTRY_ALIAS:
			break;
		default:
			g_assert_not_reached ();
			break;
		}
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
gnome_font_list ()
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
		if (!fontlist2map) fontlist2map = g_hash_table_new (NULL, NULL);
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
gnome_font_family_list ()
{
	GPFontMap * map;
	GSList * l;

	map = gp_fontmap_get ();

	if (!map->familylist) {
		for (l = map->families; l != NULL; l = l->next) {
			GPFamilyEntry * f;
			f = (GPFamilyEntry *) l->data;
			map->familylist = g_list_prepend (map->familylist, f->name);
		}
		map->familylist = g_list_reverse (map->familylist);
		if (!familylist2map) familylist2map = g_hash_table_new (NULL, NULL);
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

/*
 * Returns newly allocated string or NULL
 */

static gchar *
gp_xmlGetPropString (xmlNodePtr node, const gchar * name)
{
	xmlChar * prop;
	gchar * str;

	prop = xmlGetProp (node, name);
	if (prop) {
		str = g_strdup (prop);
		xmlFree (prop);
		return str;
	}

	return NULL;
}

static gint
gp_fe_sortname (gconstpointer a, gconstpointer b)
{
	return strcasecmp (((GPFontEntry *) a)->name, ((GPFontEntry *) b)->name);
}

static gint
gp_fe_sortspecies (gconstpointer a, gconstpointer b)
{
	return strcasecmp (((GPFontEntry *) a)->speciesname, ((GPFontEntry *) b)->speciesname);
}

static gint
gp_familyentry_sortname (gconstpointer a, gconstpointer b)
{
	return strcasecmp (((GPFamilyEntry *) a)->name, ((GPFamilyEntry *) b)->name);
}

static gchar *
gp_fm_get_species_name (const gchar * fullname, const gchar * familyname)
{
	gchar * p;

	p = strstr (fullname, familyname);

	if (!p) return g_strdup ("Normal");

	p = p + strlen (familyname);

	while (*p && (*p < 'A')) p++;

	if (!*p) return g_strdup ("Normal");

	return g_strdup (p);
}

GnomeFontWeight
gp_fontmap_lookup_weight (const gchar * weight)
{
	static GHashTable * weights = NULL;
	GnomeFontWeight wcode;

	if (!weights) {
		weights = g_hash_table_new (g_str_hash, g_str_equal);

		g_hash_table_insert (weights, "Extra Light", GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));
		g_hash_table_insert (weights, "Extralight", GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));

		g_hash_table_insert (weights, "Thin", GINT_TO_POINTER (GNOME_FONT_THIN));

		g_hash_table_insert (weights, "Light", GINT_TO_POINTER (GNOME_FONT_LIGHT));

		g_hash_table_insert (weights, "Book", GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (weights, "Roman", GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (weights, "Regular", GINT_TO_POINTER (GNOME_FONT_BOOK));

		g_hash_table_insert (weights, "Medium", GINT_TO_POINTER (GNOME_FONT_MEDIUM));

		g_hash_table_insert (weights, "Semi", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Semibold", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Demi", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Demibold", GINT_TO_POINTER (GNOME_FONT_SEMI));

		g_hash_table_insert (weights, "Bold", GINT_TO_POINTER (GNOME_FONT_BOLD));

		g_hash_table_insert (weights, "Heavy", GINT_TO_POINTER (GNOME_FONT_HEAVY));
 
		g_hash_table_insert (weights, "Extra", GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));
		g_hash_table_insert (weights, "Extra Bold", GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));

		g_hash_table_insert (weights, "Black", GINT_TO_POINTER (GNOME_FONT_BLACK));

		g_hash_table_insert (weights, "Extra Black", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
		g_hash_table_insert (weights, "Extrablack", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
		g_hash_table_insert (weights, "Ultra Bold", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
	};

	wcode = GPOINTER_TO_INT (g_hash_table_lookup (weights, weight));

	return wcode;
}

/*
 * This is not correct, if you only edit some file,
 * but I do not want to keep timestamps for all fontmaps
 * files. So please touch directory, after editing
 * files manually.
 */

static gboolean
gp_fm_is_changed (GPFontMap * map)
{
	struct stat s;
	gchar *homedir, *userdir;

	homedir = g_get_home_dir ();
	if (homedir) {
		userdir = g_concat_dir_and_file (homedir, ".gnome/fonts");
		if (!stat (userdir, &s) && !S_ISDIR (s.st_mode)) {
			/* User dir does not exist */
			g_free (userdir);
			if (s.st_mtime != map->mtime_user) return TRUE;
		} else {
			g_free (userdir);
		}
	}

	if (!stat (FONTMAPDIR_DYNAMIC, &s) && S_ISDIR (s.st_mode)) {
		/* Dynamic dir exists */
		if (s.st_mtime != map->mtime_dynamic) return TRUE;
	}

	if (!stat (FONTMAPDIR_STATIC, &s) && S_ISDIR (s.st_mode)) {
		/* Static dir exists */
		if (s.st_mtime != map->mtime_static) return TRUE;
	}

	return FALSE;
}
