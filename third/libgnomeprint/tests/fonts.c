/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  fonts.c: test functions for the libgpa config database
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
 *    Chema Celorio <chema@ximian.com>
 *    Tambet Ingo <tambet@ximian.com>
 *
 *  Copyright (C) 2002-2003 Ximian Inc.
 *
 */

#include <config.h>
#include <popt.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-fontmap.h>

#define FONT_SAMPLE_SIZE 12.0

gboolean options_dump = FALSE;
gboolean options_dump_full = FALSE;
gint     options_generate = -1;
gint     options_max = -1;
gboolean options_catalog = FALSE;
gboolean options_number = FALSE;
gchar*   options_output = NULL;
gboolean options_pdf = FALSE;

poptContext popt;

static struct poptOption options[] = {
	{ "dump",      'd', POPT_ARG_NONE,   &options_dump,   0,
	  "Dump the list of fonts", NULL},
 	{ "dump-full", 'f', POPT_ARG_NONE,   &options_dump_full,   0,
	  "Dump the list of fonts with their properties",  NULL},
	{ "generate",  'g', POPT_ARG_INT,    &options_generate,   0,
	  "Generate test output for a font",  NULL},
	{ "pdf",       'p', POPT_ARG_NONE,    &options_pdf,   0,
	  "Generate a pdf file instead of a Postscript one",  NULL},
	{ "number",    'n', POPT_ARG_NONE,   &options_number,   0,
	  "Return the number of fonts known to gnome-print",  NULL},
	{ "catalog",   'c', POPT_ARG_NONE,   &options_catalog,   0,
	  "Generate a font catalog",  NULL},
	{ "max",       'm', POPT_ARG_INT,    &options_max,     0,
	  "Max number of fonts to embed",    NULL},
	{ "output",    'o', POPT_ARG_STRING, &options_output,   0,
	  "Specify the output file",  NULL},
	POPT_AUTOHELP
	{ NULL }
};


#define CONFIG_FILE "catalog.GnomePrintConfig"

static GnomePrintConfig *
my_config_load_from_file (void)
{
	GnomePrintConfig *config;
	FILE *file;
	gchar *str;
	gint read, allocated;

	file = fopen (CONFIG_FILE, "rb");
	if (!file) {
		g_print ("Config not found [%s]\n", CONFIG_FILE);
		return gnome_print_config_default ();
	}

#define BLOCK_SIZE 10
	read = 0;
	allocated = BLOCK_SIZE;
	str = g_malloc (allocated);
	while (TRUE) {
		gint chars;
		chars = fread (str + read, sizeof (gchar), allocated - read, file);
		read += chars;
		if (chars < 1)
			break;
		if ((read + BLOCK_SIZE + 1) > allocated) {
			allocated += BLOCK_SIZE;
			str = g_realloc (str, allocated);
		}
	}
	str[read]=0;
	config = gnome_print_config_from_string (str, 0);

	return config;
}

static void
print_font_info (GPFontEntry *entry, GnomePrintContext *gpc)
{
	GnomeFont *font;
	gdouble x, y, row_size;
	gchar *c, *d;

	font = gnome_font_find (entry->name, FONT_SAMPLE_SIZE);
	if (!font || (strcmp (gnome_font_get_name (font), entry->name) != 0)) {
		g_print ("Could not get font: %s\n", entry->name);
		exit (-2);
	}
	g_print ("Printing with \"%s\".\n", gnome_font_get_name (font));

	y = 600;
	x = 20;
	row_size = 20;
	
	gnome_print_beginpage (gpc, "1");
	gnome_print_setfont (gpc, font);

	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Name:");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Family Name:");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Speciesname:");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Weight:");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Italic Angle:");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Type:");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "File:");
	y -= row_size;

	y = 600;
	x = x + 100;

	c = g_strdup_printf ("%d", entry->italic_angle);
	d = g_strdup_printf ("%d", entry->Weight);

	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, entry->name);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, entry->familyname);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, entry->speciesname);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, d);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, c);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);

	if (entry->type == GP_FONT_ENTRY_TRUETYPE) {
		gnome_print_show (gpc, "TrueType");
	} else if (entry->type == GP_FONT_ENTRY_TYPE1) {
		gnome_print_show (gpc, "Type1");
	} else {
		gnome_print_show (gpc, "Unknown");
	}
	
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, entry->file);
	y -= row_size;

	g_free (c);
	g_free (d);

	gnome_print_showpage (gpc);
	g_object_unref (G_OBJECT (font));

	return;
}

static gint
number_of_fonts (void)
{
	GPFontMap *map;
	gint num;

	map = gp_fontmap_get ();
	num = g_slist_length (map->fonts);
	gp_fontmap_release (map);

	return num;
}
      

static void
generate_font_info (gint num)
{
	GnomePrintContext *gpc;
	GnomePrintConfig *config;
	GnomePrintJob *job;
	GPFontEntry *entry;
	GPFontMap *map;
	GSList *list;
	gint max;
	gchar *out_file;

	map = gp_fontmap_get ();
	list = g_slist_copy (map->fonts);
	max = g_slist_length (list);
      
	g_assert ((num > 0) && (num <= max));

	entry = (GPFontEntry *) ((GSList *)(g_slist_nth (map->fonts, num - 1)))->data;

	g_assert (entry);
	
	job = gnome_print_job_new (NULL);
	gpc = gnome_print_job_get_context (job);
	config = gnome_print_job_get_config (job);

	if (options_pdf) {
		if (!gnome_print_config_set (config, "Printer", "PDF"))
			g_print ("Could not set the printer to PDF\n");
	} else {
		gnome_print_config_set (config, "Printer", "GENERIC");
	}

	out_file = options_output ? g_strdup (options_output) : g_strdup_printf ("o%03d.%s", num, options_pdf ? "pdf" : "ps");
	gnome_print_job_print_to_file (job, out_file);
	g_free (out_file);

	if (!gnome_print_config_set (config, GNOME_PRINT_KEY_PAPER_SIZE, "USLetter"))
		g_print ("Could not set the Paper Size\n");
	
	print_font_info (entry, gpc);

	gnome_print_job_close (job);
	gnome_print_job_print (job);
	g_object_unref (G_OBJECT (gpc));
	g_object_unref (G_OBJECT (job));
}

static void
catalog_beginpage (GnomePrintContext *pc)
{
	static gint page = 0;
	gchar *page_name;

	page_name = g_strdup_printf ("%d\n", page);
	gnome_print_beginpage (pc, page_name);
	g_free (page_name);
}

static void
catalog_endpage (GnomePrintContext *pc)
{
	gnome_print_showpage (pc);
}

#define TOP_MARGIN    72.0
#define BOTTOM_MARGIN 72.0

/* The space between the font name and the font info */
#define DELTA_INFO 7.0
#define DELTA_SAMPLE_TEXT 25.0

#define SAMPLE_TEXT_INDENTATION 15.0

#define FONT_BASE "Sans "
#define _FONT_BASE "Albany AMT "
#define FONT_NAME_SIZE 8.0
#define FONT_INFO_SIZE 6.0
#define FONT_NAME FONT_BASE "Regular"
#define FONT_INFO FONT_BASE "Regular"

#define HEADER_FONT_TITLE_SIZE  14
#define HEADER_FONT_KEYS_SIZE   8
#define HEADER_FONT_VALUES_SIZE 8
#define HEADER_FONT_TITLE  FONT_BASE "Bold"
#define HEADER_FONT_KEYS   FONT_BASE "Regular"
#define HEADER_FONT_VALUES FONT_BASE "Italic"

static gchar *
get_date (void)
{
	time_t clock;
	gchar *s;

	clock = time (NULL);
	s = ctime (&clock);

	return g_strndup (s, strlen (s) - 1);
}

static double
generate_header (GnomePrintConfig *config, GnomePrintContext *gpc, double x, double y)
{
	GnomeFont *font1;
	GnomeFont *font2;
	GnomeFont *font3;
	gdouble save, row_size;
	guchar *s;

	font1 = gnome_font_find_closest (HEADER_FONT_TITLE,  HEADER_FONT_TITLE_SIZE);
	font2 = gnome_font_find_closest (HEADER_FONT_KEYS,   HEADER_FONT_KEYS_SIZE);
	font3 = gnome_font_find_closest (HEADER_FONT_VALUES, HEADER_FONT_VALUES_SIZE);

	row_size = 10;
	
	gnome_print_setfont (gpc, font1);
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "GNOME PRINT FONT CATALOG");
	y -= row_size * 2;

	save = y;
	gnome_print_setfont (gpc, font2);

	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Libgnomeprint Ver.");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Hostname");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Date");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "No. of fonts");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Media Size");
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, "Format");
	y -= row_size;

	gnome_print_setfont (gpc, font3);
	y = save;
	x += 72;

	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, VERSION);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	s = g_malloc (256);
	gethostname (s, 256);
	gnome_print_show (gpc, s);
	g_free (s);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	s = get_date ();
	gnome_print_show (gpc, s);
	g_free (s);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	if (options_max == -1)
		s = g_strdup_printf ("%d", number_of_fonts ());
	else	
		s = g_strdup_printf ("%d (max %d)", number_of_fonts (), options_max);
	gnome_print_show (gpc, s);
	g_free (s);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	s = gnome_print_config_get (config, GNOME_PRINT_KEY_PAPER_SIZE);	
	gnome_print_show (gpc, s);
	g_free (s);
	y -= row_size;
	gnome_print_moveto (gpc, x, y);
	gnome_print_show (gpc, options_pdf ? "PDF" : "Postscript");
 	
	g_object_unref (G_OBJECT (font1));
	g_object_unref (G_OBJECT (font2));
	g_object_unref (G_OBJECT (font3));

	return y - 40;
}

static void
generate_catalog (void)
{
	GnomePrintContext *gpc;
	GnomePrintConfig *config;
	GnomePrintJob *job;
	GnomeFont *font1, *font2;
 	GPFontMap *map;
	GSList *list, *l;
	gdouble x, y, row_size;
	gdouble width, height;
	gint i = 0;

	config = my_config_load_from_file ();
	job = gnome_print_job_new (config);
	gpc = gnome_print_job_get_context (job);

	gnome_print_config_set (config, "Printer", "GENERIC");
	if (options_pdf) {
		if (!gnome_print_config_set (config, "Printer", "PDF"))
			g_print ("Could not set the printer to PDF\n");
	}
	gnome_print_job_print_to_file (job, options_output ? options_output :
				       (options_pdf ? "catalog.pdf" : "catalog.ps"));
	gnome_print_config_get_page_size (config, &width, &height);
	
	map = gp_fontmap_get ();
	list = g_slist_copy (map->fonts);
	l = list;

	font1 = gnome_font_find_closest (FONT_NAME, FONT_NAME_SIZE);
	font2 = gnome_font_find_closest (FONT_INFO, FONT_INFO_SIZE);

	y = height - TOP_MARGIN;
	x = 20;
	row_size = 40;

	catalog_beginpage (gpc);
	y = generate_header (config, gpc, x, y);
	
	while (l) {
		GPFontEntry *entry = l->data;
		GnomeFont *font;
		gchar *info;
		
		if ((options_max != -1) && (i == options_max))
			break;
		
		if (y < BOTTOM_MARGIN) {
			catalog_endpage (gpc);
			catalog_beginpage (gpc);
			y = height - TOP_MARGIN;
		}

		info = g_strdup_printf ("#%d [%s]", i + 1, entry->file);
		gnome_print_setfont (gpc, font1);
		gnome_print_moveto (gpc, x, y);
		gnome_print_show (gpc, entry->name);
		gnome_print_setfont (gpc, font2);
		gnome_print_moveto (gpc, x, y - DELTA_INFO);
		gnome_print_show (gpc, info);

		font = gnome_font_find (entry->name, FONT_SAMPLE_SIZE);
		if (!font || (strcmp (gnome_font_get_name (font), entry->name) != 0)) {
			g_print ("Could not get font: %s\n", entry->name);
			exit (-2);
		}

		gnome_print_setfont (gpc, font);
		gnome_print_moveto (gpc, x + SAMPLE_TEXT_INDENTATION, y - DELTA_SAMPLE_TEXT);
		gnome_print_show (gpc, gnome_font_face_get_sample (font->face));
		g_object_unref (font);

		
		y -= row_size;
		g_free (info);

		i++;
		l = l->next;
	}

	catalog_endpage (gpc);
	
	g_slist_free (list);

	gnome_print_job_close (job);
	gnome_print_job_print (job);
}

static void
dump_font_info (GPFontEntry *entry, gint num)
{
	g_print ("\n\nEntry:\t\t%d\n", num);
	g_print ("Name:\t\t%s\n",       entry->name);
	g_print ("Family Name:\t%s\n",  entry->familyname);
	g_print ("Speciesname:\t%s\n",  entry->speciesname);
	g_print ("Weight:\t\t%d\n",     entry->Weight);
	g_print ("Italic Angle:\t%d\n", entry->italic_angle);

	switch (entry->type) {
	case GP_FONT_ENTRY_UNKNOWN:
		g_print ("Type:\t\tUnknown\n");
		break;
	case GP_FONT_ENTRY_TYPE1:
		g_print ("Type:\t\tType 1\n");
		break;
	case GP_FONT_ENTRY_TRUETYPE:
		g_print ("Type:\t\tTrue Type\n");
		break;
	case GP_FONT_ENTRY_ALIAS:
		g_print ("Type:\t\tAlias\n");
		break;
	}
	g_print ("File:\t\t%s\n", entry->file);
}

static void
dump_short_list (void)
{
	GList *list, *tmp;
	gchar *font;

	tmp = list = gnome_font_list ();

	while (tmp) {
		font = tmp->data;
		tmp = tmp->next;

		g_print ("%s\n", font);
	}

	gnome_font_list_free (list);
}

static void
dump_long_list (void)
{
 	GPFontMap *map;
	GSList *list, *l;
	gint i = 0;

	map = gp_fontmap_get ();
	list = g_slist_copy (map->fonts);
	l = list;

	while (l) {
		dump_font_info (l->data, ++i);
		l = l->next;
	}

	g_slist_free (list);

	g_print ("\n");
}

static void
usage (gint num)
{
	g_print ("\n");
	switch (num) {
	case 0:
		g_print ("Please specify an action\n");
		break;
	case 1:
		g_print ("Only one of --dump --dump-full --generate --catalog and --number can be specified\n");
		break;
	case 2:
		g_print ("Font number out of range\n");
		break;
	case 3:
		g_print ("Option --generate or --max need number to be specified\n");
		break;
	case 4:
		g_print ("Option --output can only be used with the --generate or --catalog options\n");
		break;
	case 5:
		g_print ("Option --pdf can only be used with the --generate or --catalog option\n");
		break;
	default:
		g_assert_not_reached ();
	}

	g_print ("\n");
	poptPrintHelp (popt, stdout, FALSE);
}

static void
check_options (int argc, const char *argv[])
{
	GPFontMap *map;
	gint max;
	gint num = 0;
	gint retval;

	popt = poptGetContext ("test", argc, argv, options, 0);
	retval = poptGetNextOpt (popt);

	if (options_dump)
		num++;
	if (options_dump_full)
		num++;
	if (options_generate != -1)
		num++;
	if (options_catalog)
		num++;
	if (options_number)
		num++;

	map = gp_fontmap_get ();
	max = g_slist_length (map->fonts);
	gp_fontmap_release (map);

	if (num > 1) {
		usage (1);
	} else if (retval == POPT_ERROR_NOARG ||
		   retval == POPT_ERROR_BADNUMBER) {
		usage (3);
	} else if (num == 0) {
		usage (0);
	} else if ((options_generate != -1) &&
		   ((options_generate < 1) ||
		    (options_generate > max))) {
		usage (2);
	} else if ((options_generate == -1) &&
		   (options_catalog == FALSE) &&
		   (options_output != NULL)) {
		usage (4);
	} else if ((options_generate == -1) &&
		   (options_catalog == FALSE) &&
		   (options_pdf)) {
		usage (5);
	} else {
		poptFreeContext (popt);
		return;
	}

	poptFreeContext (popt);
	exit (1);
}

static void
handle_sigsegv (int i)
{
	g_print ("\n./fonts crashed \n");
	exit (-5);
}

int
main (int argc, const char *argv[])
{
	struct sigaction sig;

	/* We need to catch crashes */
	sig.sa_handler = handle_sigsegv;
	sig.sa_flags = 0;
	sigaction (SIGSEGV, &sig, NULL);
	
	check_options (argc, argv);

	g_type_init ();

	if (options_dump)
		dump_short_list ();
	if (options_dump_full)
		dump_long_list ();
	if (options_catalog)
		generate_catalog ();
	if (options_generate > 0)
		generate_font_info (options_generate);
	if (options_number)
		return number_of_fonts ();

	return 0;
}
