/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  generate.c: generate gnome-print output
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
 *
 *  Copyright (C) 2002 Ximian Inc. and authors
 *
 */



#include <popt.h>
#include <string.h>
#include <stdlib.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprint/gnome-print-meta.h>

typedef enum {
	BACKEND_PS,
	BACKEND_PDF,
	BACKEND_META,
	BACKEND_UNKNOWN,
} BackendType;

static gchar *sequence_str = NULL;
static gchar *backend_str = NULL;
static gchar *replay_str = NULL;
static gchar *output_str = NULL;
gboolean debug = FALSE;

static struct poptOption options[] = {
	{ "backend",   '\0', POPT_ARG_STRING, &backend_str, 0,
	  "Backend to genereate",          "ps,pdf,meta"},
	{ "sequence", '\0', POPT_ARG_STRING, &sequence_str,   0,
	  "Sequence number of drawing commands, where nn is the sequence number", "nn, error_nn"},
	{ "replay",   '\0', POPT_ARG_STRING, &replay_str,   0,
	  "Replay a metafile rather than using an internal sequence", "metafile"},
	{ "debug",    '\0', POPT_ARG_NONE, &debug, 0,
	  "Print debugging output",          NULL},
	POPT_AUTOHELP
	{ NULL }
};

static void parse_command_line (int argc, const char ** argv, BackendType *backend,
				gint *sequence, gchar **output, gchar **metafile);

static void
my_error (gchar *err_message)
{
	gchar *m = g_strdup_printf ("%s failed", err_message);
	g_warning (m);
	g_free (m);
	exit (1);
}

static gint
my_draw_square_spiral (GnomePrintContext *pc, gint increment, gint rotate)
{
	gint delta = 0;
	gint max = 100;
	gint ret = 0;
	
	ret += gnome_print_gsave (pc);
	ret += gnome_print_rotate (pc, rotate);
	ret += gnome_print_newpath (pc);
	ret += gnome_print_moveto (pc, 0, 0);

	while ((delta * 2) <= max) {
		ret += gnome_print_lineto (pc, delta - (delta ? increment : 0) , delta);
		ret += gnome_print_lineto (pc, max - delta, delta);
		ret += gnome_print_lineto (pc, max - delta, max - delta);
		ret += gnome_print_lineto (pc, delta, max - delta);

		delta += increment;
	}

	ret += gnome_print_stroke (pc);
	ret += gnome_print_grestore (pc);

	return ret;
}

static gint
my_draw_randomfig (GnomePrintContext *pc)
{
	gint a, b, i;
	double max = 100.0;
	gint ret = 0;

	srandom (47);
	
	ret += gnome_print_gsave (pc);
	
	ret += gnome_print_newpath (pc);
	ret += gnome_print_moveto (pc, 0, 0);
	for (i = 80; 0 < i; i --) {
		a = 1+(int) (max*rand()/(RAND_MAX+1.0));
		b = 1+(int) (max*rand()/(RAND_MAX+1.0));
		ret += gnome_print_lineto (pc, a, b);
	}
	ret += gnome_print_stroke (pc);
	ret += gnome_print_grestore (pc);

	return ret;
}

static gint
my_draw_circle (GnomePrintContext *pc, gint radius)
{
	gint ret = 0;
	
	ret += gnome_print_newpath (pc);
	ret += gnome_print_arcto (pc, radius, radius, (gdouble) radius, 1, 360, 0);
	ret += gnome_print_stroke (pc);

	return ret;
}

static gint
my_draw_paths (GnomePrintContext *pc)
{
	gint ret = 0;
	
	ret += gnome_print_beginpage (pc, "1");

	/* Some line figures */
	ret += gnome_print_gsave (pc);

	ret += gnome_print_translate (pc, 100, 700);
	ret += my_draw_square_spiral (pc, 3, 0);
	
	ret += gnome_print_translate (pc, 200, 0);
	ret += my_draw_square_spiral (pc, 5, 45);
	
	ret += gnome_print_translate (pc, 100, 0);
	ret += my_draw_randomfig (pc);

	ret += gnome_print_translate (pc, -300, -150);
	ret += my_draw_circle (pc, 50);

	ret += gnome_print_grestore (pc);

	/* ADD: Clipping paths with both winding rules */
	
	ret += gnome_print_showpage (pc);

	return ret;
}

static gint
my_draw_simple (GnomePrintContext *pc)
{
	gint ret = 0;
	
	ret += gnome_print_beginpage (pc, "1");
	ret += gnome_print_moveto (pc, 100, 100);
	ret += gnome_print_lineto (pc, 200, 200);
	ret += gnome_print_stroke (pc);
	ret += gnome_print_showpage (pc);

	return ret;
}

#define G_P_PIXELS 256
static gint
my_draw_image (GnomePrintContext *pc, gboolean color)
{
	double matrix[6] = {100, 0, 0, 100, 50, 300};
	char colorimg [G_P_PIXELS] [G_P_PIXELS] [3];
	char img      [G_P_PIXELS] [G_P_PIXELS];
	gint pixels, y, x;
	gint ret = 0;
	gint band = 4;
	
	pixels = G_P_PIXELS;

	/* Generate the image in memory */
	if (color) {
		for (y = 0; y < pixels; y++)
			for (x = 0; x < pixels; x++)
			{
				colorimg[y][x][0] = (x + y) >> 1;
				colorimg[y][x][1] = (x + (255 - y)) >> 1;
				colorimg[y][x][2] = ((255 - x) + y) >> 1;
			}
		for (y = 0; y < pixels; y++){
			for (x = 0; x < band; x++){
				colorimg [y][x][0] = 0;
				colorimg [y][x][1] = 0;
				colorimg [y][x][2] = 0;
			}
			for (x = pixels - band; x < pixels; x++){
				colorimg [y][x][0] = 0;
				colorimg [y][x][1] = 0;
				colorimg [y][x][2] = 0;
			}
		}
		for (x = 0; x < pixels; x++){
			for (y = 0; y < band; y++){
				colorimg [y][x][0] = 0;
				colorimg [y][x][1] = 0;
				colorimg [y][x][2] = 0;
			}
			for (y = pixels - band; y < pixels; y++){
				colorimg [y][x][0] = 0;
				colorimg [y][x][1] = 0;
				colorimg [y][x][2] = 0;
			}
		}
	} else {
		for (y = 0; y < pixels; y++)
			for (x = 0; x < pixels; x++)
				img[y][x] = ((x+y)*pixels/pixels)/2;
	}
		
	gnome_print_beginpage   (pc, "1");
	gnome_print_gsave (pc);
	gnome_print_concat (pc, matrix);
	gnome_print_moveto (pc, 0, 0);
	if (color)
		gnome_print_rgbimage  (pc, (char *) colorimg, pixels, pixels, pixels * 3);
	else
		gnome_print_grayimage (pc, (char *)img, pixels, pixels, pixels);
	gnome_print_grestore (pc);
	gnome_print_showpage    (pc);
	
	return ret;
}

static gint
my_draw_text (GnomePrintContext *pc)
{
	GnomeFont *font;
	gchar *font_name;
	gint ret = 0;
	gint i;
	gint start = 0;
	gint end = 3;

	ret += gnome_print_beginpage (pc, "1");

	for (i = start; i <= end; i++) {
		     switch (i) {
		     case 0:
			     font_name = "Caslon Roman";
			     break;
		     case 1:
			     font_name = "New Century Schoolbook Roman";
			     break;
		     case 2:
			     font_name = "Arioso Bold";
			     break;
		     case 3:
			     font_name = "Verdana Bold Italic";
			     break;
		     }

		     font = gnome_font_find_closest (font_name, 12);
		     g_print ("Using font. %s, which has %d glyphs\n",
			      gnome_font_get_ps_name (font),
			      gnome_font_face_get_num_glyphs (gnome_font_get_face (font)));
		     if (!font) {
			     g_warning ("Could not find font\n");
			     continue;
		     }
		     
		     ret += gnome_print_translate (pc, 0, 100);
		     ret += gnome_print_setfont (pc, font);
		     ret += gnome_print_moveto (pc, 10, 100);
		     ret += gnome_print_show (pc, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		     ret += gnome_print_moveto (pc, 10, 120);
		     ret += gnome_print_show (pc, "abcdefghijklmnopqrstuvwxyz");
		     ret += gnome_print_moveto (pc, 10, 140);
		     ret += gnome_print_show (pc, "1234567890~`!@#$%^&*()-_+=[]{}\\|'\";:/.></?");
		     ret += gnome_print_moveto (pc, 10, 160);
		     ret += gnome_print_show (pc, font_name);
	}

	ret += gnome_print_showpage (pc);
	return ret;
}

static gint
my_draw_glyphlist (GnomePrintContext *pc)
{
	GnomeGlyphList *gl;
	GnomeFont *font;
	gint ret;
	gchar *p, *end;

	ret += gnome_print_beginpage (pc, "1");

	font = gnome_font_find_closest ("Albany AMT", 12);
	gnome_print_setfont (pc, font);
	gl = gnome_glyphlist_from_text_dumb (font, 0x000000ff, 0.0, 0.0, "");
	
	gnome_glyphlist_advance (gl, TRUE);
	gnome_glyphlist_moveto (gl, 100, 100);

	gnome_print_setfont (pc, font);
	
	p = g_strdup ("This is generate.c, glyphlist test\n");
	end = p + strlen (p);
	
	while (p < end) {
		gunichar ch;
	       	gint glyph;
		
		ch = g_utf8_get_char (p);
		
		glyph = gnome_font_lookup_default (font, ch);
		gnome_glyphlist_glyph (gl, glyph);

		p = g_utf8_next_char (p);
	}

	gnome_print_moveto (pc, 100, 100);
	gnome_print_glyphlist (pc, gl);
	gnome_print_setfont (pc, font);
	
	gnome_glyphlist_unref (gl);

	gnome_print_setfont (pc, font);
	
	ret += gnome_print_showpage (pc);

	return GNOME_PRINT_OK;
	return ret;
}


/**
 * my_draw_error:
 * @gpc: 
 * 
 * Generates drawing commands that are errors
 * 
 * Return Value: 
 **/
static gint
my_draw_error (GnomePrintContext *gpc, gint sequence)
{
	gint ret = 0;
	gdouble ctm [6]={1, 0, 0, 1, 1, 1};

	switch (sequence) {
	case 0:
		/* Showpage without beginpage */
		ret += gnome_print_showpage (gpc);
		break;
	case 1:
		/* Beginpage without closing previous beginpage */
		ret += gnome_print_beginpage (gpc, "1");
		ret += gnome_print_beginpage (gpc, "2");
		break;
	case 2:
		/* Stroke without having a path */
		ret += gnome_print_beginpage (gpc, "1");
		ret += gnome_print_stroke (gpc);
		break;
	case 4:
		/* Start drawing before beginpage */
		ret += gnome_print_moveto (gpc, 100, 100);
		ret += gnome_print_lineto (gpc, 200, 200);
		ret += gnome_print_stroke (gpc);
		break;
	case 5:
		/* Concat before beginpage */
		ret += gnome_print_concat (gpc, ctm);
		break;
	}
	/* FIXME write more error cases (a lot more)
	 * then add to run-tests.pl a flag to run a number of
	 * errors
	 */
#if 0
FIXME:
	test a crash;
#endif	
	
	return ret;
}

static gint
my_draw (GnomePrintContext *gpc, gint sequence)
{
	gint ret = 0;

	if (sequence < 0) {
		ret += my_draw_error (gpc, (sequence * -1) - 1);
		return ret;
	}

	if (debug)
		g_print ("Running sequence %d\n", sequence);
	
	switch (sequence) {
	case 0:
		ret += my_draw_simple (gpc);
		break;
	case 1:
		ret += my_draw_paths (gpc);
		break;
	case 2:
		ret += my_draw_image (gpc, FALSE);
		break;
	case 3:
		ret += my_draw_image (gpc, TRUE);
		break;
	case 4:
		ret += my_draw_text (gpc);
		break;
	case 5:
		ret += my_draw_glyphlist (gpc);
		break;
	default:
		g_print ("Fatal error: Sequence not implemented. (%s)\n", sequence_str);
		ret = 1;
	}

	return ret;
}

static gint
my_replay (GnomePrintContext *gpc, const gchar *metafile)
{
	gint ret = 0;

	gnome_print_meta_render_file (gpc, metafile);
	
	return ret;
}

int
main (int argc, const char * argv[])

{
	GnomePrintContext *gpc;
	GnomePrintJob *job;
	GnomePrintConfig *config;

	BackendType backend;
	gint sequence;
	gchar *output = NULL;
	gchar *metafile = NULL;
	gchar *test;
	int ret = 0;

	FILE *f;
	const guchar *data;
	int len;
	
	parse_command_line (argc, argv, &backend, &sequence, &output, &metafile);

	g_type_init ();
	
	job = gnome_print_job_new (NULL);
	if (!job)
		my_error ("Print job new");
	gpc = gnome_print_job_get_context (job);
	if (!gpc)
		my_error ("Print job get context");
	config = gnome_print_job_get_config (job);
	if (!config)
		my_error ("Print job get config");

	/* Set backend */
	switch (backend) {
	case BACKEND_META:
		gpc = GNOME_PRINT_CONTEXT (gnome_print_meta_new ());
		ret += my_draw (gpc, sequence);
		gnome_print_context_close (gpc);
		data = gnome_print_meta_get_buffer (GNOME_PRINT_META (gpc));
		len  = gnome_print_meta_get_length (GNOME_PRINT_META (gpc));
		f = fopen (output, "w");
		if (!f) {
			g_warning ("Could not create %s\n", output);
			return 1;
		}
		if (!fwrite (data, len, 1, f))
			g_warning ("Could not write data\n");
		fclose (f);
		goto clean_and_exit;
	case BACKEND_PS:
		if (!gnome_print_config_set (config, "Printer", "GENERIC"))
			my_error ("gnome_print_config_set Printer-GENERIC");
		break;
	case BACKEND_PDF:
		if (debug)
			g_print ("Setting printer to PDF\n");
		if (!gnome_print_config_set (config, "Printer", "PDF"))
			my_error ("gnome_print_config_set Printer-PDF");
		test = gnome_print_config_get (config, "Printer");
		if (!test)
			my_error ("gnome_print_config_get Printer returned NULL\n");
		if (strcmp (test, "PDF") != 0)
			my_error ("Could not set printer to PDF.\n");
		g_free (test);		
		break;		
	case BACKEND_UNKNOWN:
	default:
		g_print ("Fatal error: Backend not implemented. (%s)\n", backend_str);
		ret = 1;
	}

	/* We always print to file */
	if (gnome_print_job_print_to_file (job, output) != GNOME_PRINT_OK)
		my_error ("print_job_print_to_file");

	if (replay_str)
		ret += my_replay (gpc, replay_str);
	else
		ret += my_draw (gpc, sequence);

	if (ret > 0)
		my_error ("drawing functions");
	if (gnome_print_job_close (job) != GNOME_PRINT_OK)
		my_error ("gnome_print_job_close");
	if (gnome_print_job_print (job) != GNOME_PRINT_OK)
		my_error ("gnome_print_job_print");

	g_object_unref (G_OBJECT (config));
	g_object_unref (G_OBJECT (gpc));
	g_object_unref (G_OBJECT (job));
clean_and_exit:

	g_free (output_str);
	g_free (output);
	
	return ret;
}


/* Command line parsing */
static void
usage (gchar *error)
{
	g_print ("Error: %s\n\n", error);
	g_print ("Usage: generate --backend=[backend] --sequence=[sequence] <outputfile>\n\n"
		 "    Valid backends are:\n"
		 "         -ps\n"
		 "         -pdf\n"
		 "         -meta\n\n"
		 "    Valid drawing sequences are: 0 thru 999 fixme\n"
		 "\n");
	exit (-1);
}

static void
parse_command_line (int argc, const char ** argv, BackendType *backend,
		    gint *sequence, gchar **output, gchar **metafile)
{
	poptContext popt;
	const gchar **args;

	popt = poptGetContext ("generate", argc, argv, options, 0);
	poptGetNextOpt (popt);

	if (debug)
		g_print ("Backend strings is: %s\n", backend_str);
	
	if (!backend_str)
		*backend = BACKEND_UNKNOWN;
	else if (!strcmp ("ps", backend_str))
		*backend = BACKEND_PS;
	else if (!strcmp ("pdf", backend_str))
		*backend = BACKEND_PDF;
	else if (!strcmp ("meta", backend_str))
		*backend = BACKEND_META;
	else 
		*backend = BACKEND_UNKNOWN;

	if (*backend == BACKEND_UNKNOWN)
		usage ("Backend not specicied or invalid.");
	
	if (replay_str && replay_str[0] != '\0') {
		FILE *file;
		file = fopen (replay_str, "r");
		if (!file) {
			g_print ("File \"%s\" could not be opened\n", replay_str);
			exit (-1);
		}
		fclose (file);
		*metafile = g_strdup (replay_str);
		*sequence = -1;
		goto end_parse_command_line;
	}

	if (debug)
		g_print ("Sequence string is %s\n", sequence_str);
	
	if (!sequence_str || sequence_str[0] == '\0') {
		*sequence = 0;
	} else if (!strncmp ("error_", sequence_str, strlen ("error_"))) {
		*sequence = atoi (sequence_str + strlen ("error_"));
		/* Negative number means we do an error sequence */
		*sequence = (*sequence + 1) * - 1;
	} else {
		*sequence = atoi (sequence_str);
	}

	if (debug)
		g_print ("Sequence is %d\n", *sequence);
	
end_parse_command_line:
	/* Get output filename */
	output_str = NULL;
	args = poptGetArgs (popt);

	if (!args || !args[0]) {
		usage ("Output file not specified");
	}
	
	*output = g_strdup (args [0]);
	output_str = g_strdup (args [0]);

	poptFreeContext (popt);
}

