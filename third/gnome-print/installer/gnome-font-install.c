#define _GNOME_FONT_INSTALL_C_

/*
 * Fontmap file generator for gnome-print
 *
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Chris Lahey <clahey@ximian.com>
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <popt-gnome.h>
#include <glib.h>
/* I know, that is is not nice, but that is exactly, what xml-config gives us */
#include <parser.h>
#include <xmlmemory.h>
/* End of ugly thing */
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <libgnome/gnome-i18n.h>
#include "parseAFM.h"
#include "gf-fontmap.h"
#include "gf-pfb.h"

/* Known file types */

typedef enum {
	GFI_FILE_UNKNOWN,
	GFI_FILE_PFB,
	GFI_FILE_AFM
} GFIFileType;

/* Known data about any font file */

typedef struct {
	GFIFileType type;
	GFFileEntry entry;
	gchar *name;
	gchar *familyname;
	gchar *version;
} GFIFileData;

typedef struct {
	guchar *name; /* Font name */
	guchar *alias; /* PSName of glyphs */
} GFIAliasData;

typedef struct {
	gchar *name;
	gchar *familyname;
	gchar *psname;
	GSList *afm_list;
	GSList *pfb_list;
} GFIFontData;

static void gfi_verify_font_entry (GFFontEntry *e);
static void gfi_verify_afm_file (GFFontEntry *e, GFFileEntry *f);
static void gfi_verify_pfb_file (GFFontEntry *e, GFFileEntry *f);
static GFIFileData * gfi_read_afm_file_data (const guchar *name);
static GFIFileData * gfi_read_pfb_file_data (const guchar *name);
static gboolean gfi_test_file_changed (GFFileEntry * f);

static void gfi_read_aliases (const guchar *path);
static void gfi_read_alias (xmlNodePtr node);

static void gfi_scan_path (const guchar *path, gint level);
static void gfi_try_font_file (const guchar *path);

static void gfi_sort_fonts (void);

static void gfi_process_aliases (void);
static void gfi_process_alias (GFIAliasData *a);

static void gfi_build_fonts (void);
static void gfi_build_font (GFIFontData * fd);

static gboolean gfi_font_is_registered (const guchar *name);

static FILE *gfi_get_output_stream (void);
static gboolean gfi_ensure_directory (const gchar *path);

static void gfi_write_fontmap (FILE * f);
static void gfi_write_font (xmlNodePtr root, GFFontEntry *e);

static guchar * gfi_get_species_name (const guchar *fullname, const guchar *familyname);

/*
 * We use simpler arguments than original version
 *
 * --debug prints debugging information
 * --assignment creates assignment (for both pfb and afm)
 * --fontmap-path directory for .font files
 * --afm-path directory(ies) are searched for relative afm files
 * --pfb-path directory(ies) are searched for relative pfb files
 * --target specifies output file (stdout if none)
 *
 */

static gboolean gfi_debug = FALSE;
static gboolean gfi_recursive = FALSE;
static gboolean gfi_clean = FALSE;
static gboolean gfi_usermap = TRUE;
static gboolean gfi_dynamicmap = FALSE;
static gboolean gfi_staticmap = FALSE;
static gchar *gfi_target = NULL;

static void add_path (poptContext ctx, enum poptCallbackReason reason, const struct poptOption *opt, const char *arg, void *data);

static const struct poptOption options[] = {
	{ "debug", 'd', POPT_ARG_NONE, &gfi_debug, 0,
	  N_("Print out debugging information"), NULL },
	{ "recursive", 'r', POPT_ARG_NONE, &gfi_recursive, 0,
	  N_("Search directories recursively for font files"), NULL },
	{ "clean", 'c', POPT_ARG_NONE, &gfi_clean, 0,
	  N_("Start from zero fontmap, instead of parsing old one"), NULL },
	{ "user", 'u', POPT_ARG_NONE, &gfi_usermap, 0,
	  N_("Create $HOME/.gnome/fonts/gnome-print.fontmap"), NULL },
	{ "dynamic", 'd', POPT_ARG_NONE, &gfi_dynamicmap, 0,
	  N_("Create $SYSCONFDIR/gnome/fonts/gnome-print.fontmap"), NULL },
	{ "static", 's', POPT_ARG_NONE, &gfi_staticmap, 0,
	  N_("Create $DATADIR/gnome/fonts/gnome-print.fontmap"), NULL },
	{ "target", 't', POPT_ARG_STRING, &gfi_target, 0,
	  N_("Write output fontmap to specified file (- for stdout)"), NULL },
	{ NULL, '\0', POPT_ARG_CALLBACK, &add_path, 0 },
	{ "aliases", 'a', POPT_ARG_STRING, NULL, 0,
	  N_("File describing known aliases"), N_("PATH") },
	POPT_AUTOHELP
	{ NULL, '\0', 0, NULL, 0 }
};

#if 0
static GSList * fontmappath_list = NULL;
static GSList * afmpath_list = NULL;
static GSList * pfbpath_list = NULL;
static GHashTable * assignment_dict = NULL;
#endif
static GSList *aliaspath_list = NULL;

/* List of GFIAliasData structures */
static GSList *alias_list = NULL;

static GSList * goodafm_list = NULL;
static GHashTable * goodafm_dict = NULL;
static GSList * goodpfb_list = NULL;
static GHashTable * goodpfb_dict = NULL;

static GSList * font_list = NULL;
static GHashTable * font_dict = NULL;
static GSList * goodfont_list = NULL;
static GHashTable * goodfont_dict = NULL;

/* Master font database */
static GFFontDB *masterdb;
/* Existing master map and target fontmap */
static GFFontMap *mastermap, *newmap;

int main (int argc, char ** argv)
{
	poptContext ctx;
	GFFontDB *db;
	char ** args;
	FILE *of;
	gint i;
        int result;

	/* Initialize dictionaries */

	goodafm_dict = g_hash_table_new (g_str_hash, g_str_equal);
	goodpfb_dict = g_hash_table_new (g_str_hash, g_str_equal);
	font_dict = g_hash_table_new (g_str_hash, g_str_equal);
	goodfont_dict = g_hash_table_new (g_str_hash, g_str_equal);

	/* Parse arguments */

	ctx = poptGetContext (NULL, argc, argv, options, 0);

	result = poptGetNextOpt (ctx);
	if (result != -1) { 
                fprintf(stderr, "%s: %s: %s\n",
                        "gnome-print-install",
                        poptBadOption(ctx, POPT_BADOPTION_NOALIAS),
                        poptStrerror(result));
                return 1;
	}
	args = (char **) poptGetArgs (ctx);

	/* Step 1: Read existing fontmap */
	if (gfi_debug) fprintf (stderr, "Reading fontmap... ");
	if (gfi_clean) {
		db = gf_font_db_new ();
	} else {
		db = gf_font_db_load ();
	}
	masterdb = db;
	if (gfi_debug) fprintf (stderr, "Done\n");

	/* Get fontmap we are going to write into */
	if (gfi_staticmap) {
		mastermap = db->staticmaps;
		newmap = gf_fontmap_new (GF_FONTMAP_STATIC, mastermap->path);
		if (!gfi_target) gfi_target = g_concat_dir_and_file (FONTMAPDIR_STATIC, "gnome-print.fontmap");
	} else if (gfi_dynamicmap) {
		mastermap = db->dynamicmaps;
		newmap = gf_fontmap_new (GF_FONTMAP_DYNAMIC, mastermap->path);
		if (!gfi_target) gfi_target = g_concat_dir_and_file (FONTMAPDIR_DYNAMIC, "gnome-print.fontmap");
	} else {
		mastermap = db->usermaps;
		newmap = gf_fontmap_new (GF_FONTMAP_USER, mastermap->path);
		if (!gfi_target) gfi_target = g_concat_dir_and_file (g_get_home_dir (), ".gnome/fonts/gnome-print.fontmap");
	}
	g_assert (mastermap != NULL);

	/* Verify files from selected map, remove fonts */
	if (gfi_debug) fprintf (stderr, "Verifying fontmap entries ");
	while (mastermap->fonts) {
		GFFontEntry *e;
		e = mastermap->fonts;
		mastermap->fonts = e->next;
		gfi_verify_font_entry (e);
		if (gfi_debug) fprintf (stderr, ".");
	}
	if (gfi_debug) fprintf (stderr, "Done\n");

	/* Process alias files */
	if (gfi_debug) fprintf (stderr, "Scanning alias maps... ");
	while (aliaspath_list) {
		gfi_read_aliases ((guchar *) aliaspath_list->data);
		g_free (aliaspath_list->data);
		aliaspath_list = g_slist_remove (aliaspath_list, aliaspath_list->data);
	}
	if (gfi_debug) fprintf (stderr, "Done\n");

	/* Process directories/fontmaps */
	if (gfi_debug) fprintf (stderr, "Scanning directories: ");
	for (i = 0; args && args[i]; i++) {
		if (gfi_debug) fprintf (stderr, "%s ", args[i]);
		gfi_scan_path (args[i], 0);
	}
	if (gfi_debug) fprintf (stderr, "Done\n");

	/* Free popt context */
	poptFreeContext (ctx);

	/*
	 * Now we have:
	 *
	 * alias_list, pointing to GFIAliasData entries
	 * goodafm_list, goodpfb_list pointing to new FontData
	 * goodafm_dict, goodpfb_dict using list member names
	 *
	 */

	/* Sort all files into fonts */
	if (gfi_debug) fprintf (stderr, "Sorting fonts... ");
	gfi_sort_fonts ();
	if (gfi_debug) fprintf (stderr, "Done\n");

	/*
	 * Now we are ready to process fonts
	 *
	 * We start of Type1Aliases
	 *
	 */

	if (gfi_debug) fprintf (stderr, "Sorting Type1 aliases... ");
	gfi_process_aliases ();
	if (gfi_debug) fprintf (stderr, "Done\n");

	/*
	 * And build fonts from remaining afm/pfb files
	 *
	 */

	if (gfi_debug) fprintf (stderr, "Building fonts... ");
	gfi_build_fonts ();
	if (gfi_debug) fprintf (stderr, "Done\n");

	if (!goodfont_list) {
		if (gfi_debug) fprintf (stderr, "NO FONTS\n");
	} else {
		/*
		 * Write fontmap
		 *
		 */
		of = gfi_get_output_stream ();
		if (of) {
			gfi_write_fontmap (of);
			if (of != stdout) fclose (of);
		}
	}

	gf_font_db_free (db);

	return 0;
}

/*
 * Process GFFontEntry
 *
 * Tests, whether both afm and pfb files are valid
 * (Indirect) Saves file entries to good{$filetype}list
 * If it is alias (i.e. afm.name != pfb.name), save entry to alias_list
 *
 */

static void
gfi_verify_font_entry (GFFontEntry *e)
{
	switch (e->type) {
	case GF_FONT_ENTRY_TYPE1:
		gfi_verify_afm_file (e, &e->files[0]);
		gfi_verify_pfb_file (e, &e->files[1]);
		if (strcmp (e->files[0].psname, e->files[0].psname)) {
			GFIAliasData *a;
			/* We are aliased entry */
			a = g_new (GFIAliasData, 1);
			a->name = g_strdup (e->name);
			a->alias = g_strdup (e->files[0].psname);
			alias_list = g_slist_prepend (alias_list, a);
		}
		break;
	case GF_FONT_ENTRY_TRUETYPE:
		/* No TrueType support at moment */
		break;
	default:
		g_assert_not_reached ();
	}
}

/*
 * Verifies afm entry
 *
 * Checkes, whether file is intact
 * (Indirectly) If not intact, or no data, try to parse it
 * (Directly, Indirectly) If good, create new GFIFileData
 * and save it to goodafm_list/goodafm_dict
 *
 */

static void
gfi_verify_afm_file (GFFontEntry *e, GFFileEntry *f)
{
	GFIFileData *fd;

	/* Test, whether we are already verified and registered */
	fd = g_hash_table_lookup (goodafm_dict, f->path);
	if (fd) return;

	if (!gfi_test_file_changed (f)) {
		fd = g_new0 (GFIFileData, 1);
		fd->type = GFI_FILE_AFM;
		fd->entry.path = g_strdup (f->path);
		fd->entry.size = f->size;
		fd->entry.mtime = f->mtime;
		fd->entry.psname = g_strdup (f->psname);
		fd->name = g_strdup (e->name);
		fd->familyname = g_strdup (e->familyname);
		fd->version = g_strdup (e->version);
	} else {
		fd = gfi_read_afm_file_data (f->path);
	}

	if (fd) {
		goodafm_list = g_slist_prepend (goodafm_list, fd);
		g_hash_table_insert (goodafm_dict, fd->entry.path, fd);
	} else {
#ifdef GFI_VERBOSE
		if (gfi_debug) fprintf (stderr, "Not good: %s\n", f->name);
#endif
	}
}

/* Same as previous for pfb files */

static void
gfi_verify_pfb_file (GFFontEntry *e, GFFileEntry *f)
{
	GFIFileData * fd;

	/* Test, whether we are already verified and registered */
	fd = g_hash_table_lookup (goodpfb_dict, f->path);
	if (fd) return;

	if (!gfi_test_file_changed (f)) {
		fd = g_new0 (GFIFileData, 1);
		fd->type = GFI_FILE_PFB;
		fd->entry.path = g_strdup (f->path);
		fd->entry.size = f->size;
		fd->entry.mtime = f->mtime;
		fd->entry.psname = g_strdup (f->psname);
		fd->name = g_strdup (e->name);
		fd->familyname = g_strdup (e->familyname);
		fd->version = g_strdup (e->version);
	} else {
		fd = gfi_read_pfb_file_data (f->path);
	}

	if (fd) {
		goodpfb_list = g_slist_prepend (goodpfb_list, fd);
		g_hash_table_insert (goodpfb_dict, fd->entry.path, fd);
	} else {
#ifdef GFI_VERBOSE
		if (gfi_debug) fprintf (stderr, "Not good: %s\n", f->name);
#endif
	}
}

/*
 * Tries to parse afm file, if sucessful return GFIFileData
 * Return NULL if file does not exist/cannot be parsed
 */

static GFIFileData *
gfi_read_afm_file_data (const guchar *name)
{
	GFIFileData * fd;
	FILE * f;
	int status;
	Font_Info * fi;
	struct stat s;

	fi = NULL;

	if (stat (name, &s) < 0) return NULL;
	if (!gf_afm_check (name)) return NULL;

	f = fopen (name, "r");
	if (!f) return NULL;

	status = parseFile (f, &fi, P_G);

	fclose (f);
	if (status != AFM_ok) {
		if (fi) parseFileFree (fi);
		return NULL;
	}

	/* Loading afm succeeded, so go ahead */

	fd = g_new (GFIFileData, 1);

	fd->type = GFI_FILE_AFM;
	fd->entry.path = g_strdup (name);
	fd->entry.size = s.st_size;
	fd->entry.mtime = s.st_mtime;
	fd->entry.psname = g_strdup (fi->gfi->fontName);
	fd->name = g_strdup (fi->gfi->fullName);
	fd->familyname = g_strdup (fi->gfi->familyName);
	fd->version = g_strdup (fi->gfi->version);

	parseFileFree (fi);

	return fd;
}

/*
 * Same as previous for pfb files
 */

static GFIFileData *
gfi_read_pfb_file_data (const guchar * name)
{
	GFIFileData * fd;
	GFPFB * pfb;
	struct stat s;

	if (stat (name, &s) < 0) return NULL;

	pfb = gf_pfb_open (name);
	if (!pfb) return NULL;

	/* Loading pfb succeeded, so go ahead */

	fd = g_new (GFIFileData, 1);

	fd->type = GFI_FILE_PFB;
	fd->entry.path = g_strdup (name);
	fd->entry.size = s.st_size;
	fd->entry.mtime = s.st_mtime;
	fd->entry.psname = g_strdup (pfb->gfi.fontName);
	fd->name = g_strdup (pfb->gfi.fullName);
	fd->familyname = g_strdup (pfb->gfi.familyName);
	fd->version = g_strdup (pfb->gfi.version);

	gf_pfb_close (pfb);

	return fd;
}

/*
 * Checks, whether file has been changed, according to its
 * size and mtime fields
 */

static gboolean
gfi_test_file_changed (GFFileEntry *f)
{
	struct stat s;

	if (stat (f->path, &s) < 0) return TRUE;

	/* If we do not have file info, expect it to be changed */

	if ((f->size == 0) || (s.st_size != f->size)) return TRUE;
	if ((f->mtime == 0) || (s.st_mtime != f->mtime)) return TRUE;

	return FALSE;
}

/*
 * Scan directory for font files, update goodafm and goodpfb
 */

#define MAX_RECURSION_DEPTH 32

static void
gfi_scan_path (const guchar *path, gint level)
{
	static ino_t inodes[MAX_RECURSION_DEPTH + 1];
	struct stat s;

	if (level > MAX_RECURSION_DEPTH) return;

	if (gfi_debug) fprintf (stderr, "Scanning path %s (%s)\n", path, gfi_recursive ? "recursive" : "non-recursive");

	if (!stat (path, &s)) {
		if (S_ISDIR (s.st_mode)) {
			static gboolean cracktest = FALSE;
			static gboolean crackroot = FALSE;
			static gboolean crackdev = FALSE;
			static gboolean crackproc = FALSE;
			static struct stat s_root, s_dev, s_proc;
			gboolean seemsok;
			gint i;
			/* Check for crack-smoking */
			if (!cracktest) {
				crackroot = !stat ("/", &s_root);
				crackdev = !stat ("/dev", &s_dev);
				crackproc = !stat ("/proc", &s_proc);
				cracktest = TRUE;
			}
			seemsok = TRUE;
			/* Successful stat, check if already scanned */
			for (i = 0; i < level; i++) {
				if (s.st_ino == inodes[i]) {
					seemsok = FALSE;
					break;
				}
			}
			if (crackroot && (level > 0) && (s.st_ino == s_root.st_ino)) seemsok = FALSE;
			if (crackdev && (s.st_ino == s_dev.st_ino)) seemsok = FALSE;
			if (crackproc && (s.st_ino == s_proc.st_ino)) seemsok = FALSE;
			if (seemsok) {
				DIR * dir;
				struct dirent * dent;
				/* Not yet scanned */
				inodes[level] = s.st_ino;
				dir = opendir (path);
				if (dir) {
					while ((dent = readdir (dir))) {
						gchar *fn;
						if (!strcmp (dent->d_name, ".") || !strcmp (dent->d_name, "..")) continue;
						fn = g_concat_dir_and_file (path, dent->d_name);
						if (!stat (fn, &s)) {
							if (S_ISREG (s.st_mode)) {
								gfi_try_font_file (fn);
							} else if (S_ISDIR (s.st_mode) && gfi_recursive) {
								gfi_scan_path (fn, level + 1);
							}
						}
						g_free (fn);
					}
					closedir (dir);
				}
			} else {
				if (gfi_debug) fprintf (stderr, "Circular link or weird path: %s\n", path);
			}
		} else if (S_ISREG (s.st_mode)) {
			gfi_try_font_file (path);
		} else {
			if (gfi_debug) fprintf (stderr, "Invalid path: %s\n", path);
		}
	} else {
		if (gfi_debug) fprintf (stderr, "Unsuccessful stat: %s\n", path);
	}
}

/*
 * tries to determine file type by parsing it
 */

static void
gfi_try_font_file (const guchar *fn)
{
	GFIFileData * fd;
	struct stat s;
	gchar *name;

	if (stat (fn, &s) < 0) return;
	if (!S_ISREG (s.st_mode)) return;

	if (!g_path_is_absolute (fn)) {
		gchar *cdir;
		cdir = g_get_current_dir ();
		name = g_concat_dir_and_file (cdir, fn);
	} else {
		name = g_strdup (fn);
	}

	fd = g_hash_table_lookup (goodafm_dict, name);
	if (fd) {
		g_free (name);
		return;
	}
	fd = g_hash_table_lookup (goodpfb_dict, name);
	if (fd) {
		g_free (name);
		return;
	}

	/* Not registered, so try to determine file type */

	fd = gfi_read_afm_file_data (name);
	if (fd) {
		goodafm_list = g_slist_prepend (goodafm_list, fd);
		g_hash_table_insert (goodafm_dict, fd->entry.path, fd);
		g_free (name);
		return;
	}

	fd = gfi_read_pfb_file_data (name);
	if (fd) {
		goodpfb_list = g_slist_prepend (goodpfb_list, fd);
		g_hash_table_insert (goodpfb_dict, fd->entry.path, fd);
		g_free (name);
		return;
	}

	g_free (name);
	/* Cannot read :( */
}

/*
 * Arranges all afm and pfb FileData into FontData structures
 * goodfont_list - list of new FontData entries
 * goodfont_dict - use FontData name strings
 * Original lists are cleaned
 *
 */

static void
gfi_sort_fonts (void)
{
	GFIFileData *file;
	GFIFontData *font;

	while (goodafm_list) {
		file = (GFIFileData *) goodafm_list->data;
		font = g_hash_table_lookup (font_dict, file->name);
		if (!font) {
			font = g_new (GFIFontData, 1);
			font->name = g_strdup (file->name);
			font->familyname = g_strdup (file->familyname);
			font->psname = g_strdup (file->entry.psname);
			font->afm_list = font->pfb_list = NULL;
			font_list = g_slist_prepend (font_list, font);
			g_hash_table_insert (font_dict, font->name, font);
		}
		font->afm_list = g_slist_prepend (font->afm_list, file);
		goodafm_list = g_slist_remove (goodafm_list, file);
	}

	while (goodpfb_list) {
		file = (GFIFileData *) goodpfb_list->data;
		font = g_hash_table_lookup (font_dict, file->name);
		if (!font) {
			font = g_new (GFIFontData, 1);
			font->name = g_strdup (file->name);
			font->familyname = g_strdup (file->familyname);
			font->psname = g_strdup (file->entry.psname);
			font->afm_list = font->pfb_list = NULL;
			font_list = g_slist_prepend (font_list, font);
			g_hash_table_insert (font_dict, font->name, font);
		}
		font->pfb_list = g_slist_prepend (font->pfb_list, file);
		goodpfb_list = g_slist_remove (goodpfb_list, file);
	}
}

/*
 * Fontmap creation step
 */

static void
gfi_process_aliases (void)
{
	while (alias_list) {
		gfi_process_alias ((GFIAliasData *) alias_list->data);
		alias_list = g_slist_remove (alias_list, alias_list->data);
	}
}

static void
gfi_process_alias (GFIAliasData *a)
{
	GFFontEntry *new;
	GFIFontData *afmfd, *pfbfd;
	GFIFileData *afm, *pfb;
	GSList *l;
	gdouble afmversion, pfbversion;
	FILE *f;
	int status;
	Font_Info *fi;

	/* Return if we are already registered */
	/* fixme: We should test versions here */
	if (g_hash_table_lookup (goodfont_dict, a->name)) return;

	/* Check for fontdata with afm entry */
	afmfd = g_hash_table_lookup (font_dict, a->name);
	if (!afmfd) return;
	if (!afmfd->afm_list) return;
	afm = afmfd->afm_list->data;

	/* fixme: mess with locale */
	afmversion = (afm->version) ? atof (afm->version) : 1.0;

	/* Search, whether we have same or better pfb */
	for (l = afmfd->pfb_list; l != NULL; l = l->next) {
		GFIFileData *fd;
		gdouble pfbversion;
		fd = l->data;
		pfbversion = (fd->version) ? atof (fd->version) : 1.0;
		/* If we have original pfb with same or higher version, we shouldn't use type1 alias */
		if (pfbversion >= afmversion) return;
	}

	/* So there wasn't original pfb file */
	/* Check for fontdata with pfb entry */
	pfbfd = NULL;
	pfb = NULL;
	for (l = font_list; l != NULL; l = l->next) {
		pfbfd = (GFIFontData *) l->data;
		if (pfbfd->pfb_list) {
			pfb = pfbfd->pfb_list->data;
			if (!strcmp (a->alias, pfb->entry.psname)) break;
		}
	}
	if (l == NULL) return;

	/* fixme: mess with locale */
	pfbversion = (pfb->version) ? atof (pfb->version) : 1.0;

	for (l = pfbfd->pfb_list; l != NULL; l = l->next) {
		GFIFileData *fd;
		gdouble v;
		fd = l->data;
		v = (fd->version) ? atof (fd->version) : 1.0;
		if (v > pfbversion) {
			pfbversion = v;
			pfb = fd;
		}
	}

	/* We have to read afm to get weight and italicangle */

	fi = NULL;
	f = fopen (afm->entry.path, "r");
	/* This shouldn't happen */
	if (!f) return;
	status = parseFile (f, &fi, P_G);
	fclose (f);
	/* This shouldn't happen! */
	if (status != AFM_ok) {
		if (fi) parseFileFree (fi);
		return;
	}

	/* Now we should have everything we need */

	/* Final check - test, whether given font is already registered */
	if (gfi_font_is_registered (afmfd->name)) {
		if (gfi_debug) fprintf (stderr, "Font %s is already registered\n", afmfd->name);
		return;
	}

	new = g_new (GFFontEntry, 1);
	new->next = NULL;
	new->type = GF_FONT_ENTRY_TYPE1;
	new->name = g_strdup (afmfd->name);
	new->version = g_strdup (afm->version);
	new->familyname = g_strdup (fi->gfi->familyName);
	new->speciesname = gfi_get_species_name (new->name, new->familyname);
	new->weight = g_strdup (fi->gfi->weight);
	/* AFM */
	new->files[0] = afm->entry;
	/* PFB */
	new->files[1] = pfb->entry;
	/* Misc */
	new->italicangle = fi->gfi->italicAngle;

	/* Release AFM info */
	parseFileFree (fi);

	/* Register it */

	if (gfi_debug) fprintf (stderr, "Registered Type1 alias: %s\n", new->name);

	goodfont_list = g_slist_prepend (goodfont_list, new);
	g_hash_table_insert (goodfont_dict, new->name, new);
}

static void
gfi_build_fonts (void)
{
	while (font_list) {
		gfi_build_font ((GFIFontData *) font_list->data);
		font_list = g_slist_remove (font_list, font_list->data);
	}
}

static void
gfi_build_font (GFIFontData *fd)
{
	GFFontEntry *new;
	GFIFileData *afmdata, *pfbdata;
	gdouble afmversion, pfbversion;
	GSList * l;
	FILE * f;
	int status;
	Font_Info * fi;

	/* Return if we are already registered */
	/* Fixme: We should free structs */
	if (g_hash_table_lookup (goodfont_dict, fd->name)) return;

	pfbdata = NULL;
	pfbversion = -1e18;

	/* Find pfb vith highest version */
	for (l = fd->pfb_list; l != NULL; l = l->next) {
		GFIFileData *d;
		gdouble v;
		d = (GFIFileData *) l->data;
		v = (d->version) ? atof (d->version) : 1.0;
		if (v > pfbversion) {
			pfbversion = v;
			pfbdata = d;
		}
	}

	/* If we do not have pfb file return */
	if (!pfbdata) return;

	afmdata = NULL;
	afmversion = -1e18;

	/* Find afm vith highest version <= pfb version */
	for (l = fd->afm_list; l != NULL; l = l->next) {
		GFIFileData *d;
		gdouble v;
		d = (GFIFileData *) l->data;
		v = (d->version) ? atof (d->version) : 1.0;
		if ((v > afmversion) && (v <= pfbversion)) {
			afmversion = v;
			afmdata = d;
			if (afmversion == pfbversion) break;
		}
	}

	/* If we do not have afm file return */
	if (!afmdata) return;

	/* We have to read afm to get weight and italicangle */

	fi = NULL;
	f = fopen (afmdata->entry.path, "r");
	/* This shouldn't happen */
	if (!f) return;
	status = parseFile (f, &fi, P_G);
	fclose (f);
	/* This shouldn't happen! */
	if (status != AFM_ok) {
		if (fi) parseFileFree (fi);
		return;
	}

	/* Now we should have everything we need */

	/* Final check - test, whether given font is already registered */
	if (gfi_font_is_registered (fd->name)) {
		if (gfi_debug) fprintf (stderr, "Font %s is already registered\n", fd->name);
		return;
	}

	new = g_new (GFFontEntry, 1);
	new->next = NULL;
	new->type = GF_FONT_ENTRY_TYPE1;
	new->name = g_strdup (fd->name);
	new->version = g_strdup (afmdata->version);
	new->familyname = g_strdup (fi->gfi->familyName);
	new->speciesname = gfi_get_species_name (new->name, new->familyname);
	new->weight = g_strdup (fi->gfi->weight);
	/* AFM */
	new->files[0] = afmdata->entry;
	/* PFB */
	new->files[1] = pfbdata->entry;
	/* Misc */
	new->italicangle = fi->gfi->italicAngle;

	/* Release AFM info */
	parseFileFree (fi);

	/* Register it */
	if (gfi_debug) fprintf (stderr, "Registered font: %s\n", new->name);

	goodfont_list = g_slist_prepend (goodfont_list, new);
	g_hash_table_insert (goodfont_dict, new->name, new);
}

static gboolean
gfi_font_is_registered (const guchar *name)
{
	GFFontMap *map;

	/* Step 1 - other fontmaps of same level */
	for (map = mastermap->next; map != NULL; map = map->next) {
		if (g_hash_table_lookup (map->fontdict, name)) return TRUE;
	}
	/* Step 2 - next levels */
	if (gfi_staticmap) {
		/* Not found in static map, return */
		return FALSE;
	} else if (gfi_dynamicmap) {
		/* Search from static maps */
		for (map = masterdb->staticmaps; map != NULL; map = map->next) {
			if (g_hash_table_lookup (map->fontdict, name)) return TRUE;
		}
		return FALSE;
	} else {
		/* Search from dynamic maps */
		for (map = masterdb->dynamicmaps; map != NULL; map = map->next) {
			if (g_hash_table_lookup (map->fontdict, name)) return TRUE;
		}
		/* Search from static maps */
		for (map = masterdb->staticmaps; map != NULL; map = map->next) {
			if (g_hash_table_lookup (map->fontdict, name)) return TRUE;
		}
		return FALSE;
	}
}

static FILE *
gfi_get_output_stream (void)
{
	FILE *ostream;

	umask (022);

	if (!strcmp (gfi_target, "-")) {
		if (gfi_debug) fprintf (stderr, "Writing fontmap to stdout\n");
		return stdout;
	}
	if (!gfi_ensure_directory (gfi_target)) {
		g_warning ("Cannot create path for %s", gfi_target);
		return NULL;
	}
	ostream = fopen (gfi_target, "w");
	if (ostream) {
		if (gfi_debug) fprintf (stderr, "Writing fontmap to %s\n", gfi_target);
		return ostream;
	}

        fprintf (stderr, "gnome-font-install: Cannot open output file %s: %s\n",
                 gfi_target, g_strerror (errno));
	return NULL;
}

static gboolean
gfi_ensure_directory (const gchar *path)
{
	struct stat s;
	gchar *t, *p;
	GSList *l;

	if (!g_path_is_absolute (path)) {
		gchar *cdir;
		cdir = g_get_current_dir ();
		t = g_concat_dir_and_file (cdir, path);
	} else {
		t = g_strdup (path);
	}
	l = NULL;
	p = t;
	while (*p) {
		while (*p && (*p == '/')) p++;
		if (*p) l = g_slist_prepend (l, p);
		while (*p && (*p != '/')) p++;
		if (*p) {
			*p = '\0';
			p++;
		}
	}
	if (!l) return FALSE;
	l = g_slist_reverse (l);
	p = g_strdup ("");
	while (l->next) {
		gchar *n;
		n = g_strdup_printf ("%s/%s", p, (gchar *) l->data);
		l = g_slist_remove (l, l->data);
		g_free (p);
		p = n;
		if (stat (p, &s)) {
			if (mkdir (p, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) {
				g_warning ("Cannot create directory %s", p);
				return FALSE;
			}
		} else if (!S_ISDIR (s.st_mode)) {
			g_warning ("Fontmap base %s is not directory", p);
			return FALSE;
		}
	}
	g_free (p);

	return TRUE;
}

static void
gfi_write_fontmap (FILE * f)
{
	xmlDocPtr doc;
	xmlNodePtr root;

	doc = xmlNewDoc ("1.0");
	root = xmlNewDocNode (doc, NULL, "fontmap", NULL);
	xmlDocSetRootElement (doc, root);
	xmlSetProp (root, "version", "2.0");

	while (goodfont_list) {
		gfi_write_font (root, (GFFontEntry *) goodfont_list->data);
		goodfont_list = g_slist_remove (goodfont_list, goodfont_list->data);
	}

	xmlDocDump (f, doc);
}

static void
gfi_write_font (xmlNodePtr root, GFFontEntry *e)
{
	xmlNodePtr n, f;
	gchar c[128];

	n = xmlNewDocNode (root->doc, NULL, "font", NULL);
	xmlAddChild (root, n);

	/* Set format */
	if (strcmp (e->files[0].psname, e->files[1].psname)) {
		xmlSetProp (n, "format", "type1alias");
	} else {
		xmlSetProp (n, "format", "type1");
	}

	/* afm file */
	f = xmlNewDocNode (root->doc, NULL, "file", NULL);
	xmlAddChild (n, f);
	xmlSetProp (f, "type", "afm");
	xmlSetProp (f, "path", e->files[0].path);
	g_snprintf (c, 128, "%d", (gint) e->files[0].size);
	xmlSetProp (f, "size", c);
	g_snprintf (c, 128, "%d", (gint) e->files[0].mtime);
	xmlSetProp (f, "mtime", c);

	/* pfb file */
	f = xmlNewDocNode (root->doc, NULL, "file", NULL);
	xmlAddChild (n, f);
	xmlSetProp (f, "type", "pfb");
	xmlSetProp (f, "path", e->files[1].path);
	g_snprintf (c, 128, "%d", (gint) e->files[1].size);
	xmlSetProp (f, "size", c);
	g_snprintf (c, 128, "%d", (gint) e->files[1].mtime);
	xmlSetProp (f, "mtime", c);

	/* Other properties */
	xmlSetProp (n, "name", e->name);
	xmlSetProp (n, "version", e->version);
	xmlSetProp (n, "familyname", e->familyname);
	xmlSetProp (n, "speciesname", e->speciesname);
	xmlSetProp (n, "psname", e->files[0].psname);
	xmlSetProp (n, "weight", e->weight);
	g_snprintf (c, 128, "%g", e->italicangle);
	xmlSetProp (n, "italicangle", c);

	if (strcmp (e->files[0].psname, e->files[1].psname)) {
		xmlSetProp (n, "alias", e->files[1].psname);
	}
}

static guchar *
gfi_get_species_name (const guchar *fullname, const guchar *familyname)
{
	gchar * p;

	p = strstr (fullname, familyname);

	if (!p) return g_strdup ("Normal");

	p = p + strlen (familyname);

	while (*p && (*p < 'A')) p++;

	if (!*p) return g_strdup ("Normal");

	return g_strdup (p);
}

static void
gfi_read_aliases (const guchar *path)
{
	xmlDocPtr doc;

	if (gfi_debug) fprintf (stderr, "Trying alias file %s\n", path);

	doc = xmlParseFile (path);

	if (doc) {
		xmlNodePtr root;
		root = xmlDocGetRootElement (doc);
		if (!strcmp (root->name, "fontfile")) {
			xmlNodePtr child;
			/* List of font entries */
			for (child = root->xmlChildrenNode; child != NULL; child = child->next) {
				if (!strcmp (child->name, "font")) {
					gfi_read_alias (child);
				}
			}
		}
		xmlFreeDoc (doc);
	}
}

static void
gfi_read_alias (xmlNodePtr node)
{
/* Here we should read alias entry and create GFIAliasData */
	xmlChar *xmlfullname;
	xmlChar *xmlalias;

	xmlfullname = xmlGetProp (node, "fullname");
	xmlalias = xmlGetProp (node, "alias");

	if (xmlfullname && xmlalias) {
		GFIAliasData *a;
		a = g_new (GFIAliasData, 1);
		a->name = g_strdup (xmlfullname);
		a->alias = g_strdup (xmlalias);
		alias_list = g_slist_prepend (alias_list, a);
	}

	if (xmlfullname) xmlFree (xmlfullname);
	if (xmlalias) xmlFree (xmlalias);
}

/* Popt callback */

static void
add_path (poptContext ctx, enum poptCallbackReason reason, const struct poptOption *opt, const char *arg, void *data)
{
	struct stat s;

	if ((stat (arg, &s) == 0) && (S_ISREG (s.st_mode))) {
		if (opt->shortName == 'a') {
			aliaspath_list = g_slist_append (aliaspath_list, g_strdup (arg));
		}
	} else {
		if (gfi_debug) fprintf (stderr, "%s is not a regular file\n", arg);
	}
}

