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
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprint/gnome-print-meta.h>

typedef enum {
	SEQUENCE_SIMPLE,
	SEQUENCE_PATHS,
	SEQUENCE_TEXT,
	SEQUENCE_IMAGES,
	SEQUENCE_FONTS,
	SEQUENCE_ERROR,
} SequenceType;

typedef enum {
	BACKEND_PS,
	BACKEND_PDF,
	BACKEND_META,
	BACKEND_ERROR,
} BackendType;


/* Start: command line ... */
static gchar *sequence_str = NULL;
static gchar *backend_str = NULL;
static gchar *output_str = NULL;
static struct poptOption options[] = {
	{ "backend",   '\0', POPT_ARG_STRING, &backend_str, 0,
	  "Backend to genereate",          "ps,pdf,meta"},
	{ "sequence", '\0', POPT_ARG_STRING, &sequence_str,   0,
	  "Sequence of drawing commands", "simple,paths,text,images,fonts"},
	POPT_AUTOHELP
	{ NULL }
};
static void parse_command_line (int argc, const char ** argv, BackendType *backend, SequenceType *sequence, gchar **output);
/* End: command line */

static void
my_draw_square_spiral (GnomePrintContext *pc, gint increment, gint rotate)
{
	gint delta = 0;
	gint max = 100;
	
	gnome_print_gsave (pc);
	gnome_print_rotate (pc, rotate);
	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 0, 0);

	while ((delta * 2) <= max) {
		gnome_print_lineto (pc, delta - (delta ? increment : 0) , delta);
		gnome_print_lineto (pc, max - delta, delta);
		gnome_print_lineto (pc, max - delta, max - delta);
		gnome_print_lineto (pc, delta, max - delta);

		delta += increment;
	}

	gnome_print_stroke (pc);
	gnome_print_grestore (pc);
}

static void
my_draw_randomfig (GnomePrintContext *pc)
{
	gint a, b, i;
	double max = 100.0;

	srandom (47);
	
	gnome_print_gsave (pc);
	
	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 0, 0);
	for (i = 80; 0 < i; i --) {
		a = 1+(int) (max*rand()/(RAND_MAX+1.0));
		b = 1+(int) (max*rand()/(RAND_MAX+1.0));
		gnome_print_lineto (pc, a, b);
	}
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);
}

static void
my_draw_circle (GnomePrintContext *pc, gint radius)
{
	gnome_print_newpath (pc);
	gnome_print_arcto (pc, radius, radius, (gdouble) radius, 1, 360, 0);
	gnome_print_stroke (pc);
}

static void
my_draw_paths (GnomePrintContext *pc)
{
	gnome_print_beginpage (pc, "1");

	/* Some line figures */
	gnome_print_gsave (pc);

	gnome_print_translate (pc, 100, 700);
	my_draw_square_spiral (pc, 3, 0);
	
	gnome_print_translate (pc, 200, 0);
	my_draw_square_spiral (pc, 5, 45);
	
	gnome_print_translate (pc, 100, 0);
	my_draw_randomfig (pc);

	gnome_print_translate (pc, -300, -150);
	my_draw_circle (pc, 50);

	gnome_print_grestore (pc);

	/* ADD: Clipping paths with both winding rules */
	
	gnome_print_showpage (pc);
}

static void
my_draw_simple (GnomePrintContext *pc)
{
	gnome_print_beginpage (pc, "1");
	gnome_print_moveto (pc, 100, 100);
	gnome_print_lineto (pc, 200, 200);
	gnome_print_stroke (pc);
	gnome_print_showpage (pc);
}

static void
my_draw_text (GnomePrintContext *pc)
{
	GnomeFont *font;

	font = gnome_font_find_closest_from_full_name ("Courier 2");
	if (!font) {
		g_warning ("Could not find font\n");
		return;
	}

	gnome_print_beginpage (pc, "1");
	gnome_print_setfont (pc, font);
	gnome_print_moveto (pc, 50, 500);
#define	SEP "'"
	gnome_print_show (pc,
			  "123123123123123123123123123" SEP
			  "123123123123123123123123123" SEP
			  "123123123123123123123123123" SEP
			  "123123123123123123123123123" SEP
			  "123123123123123123123123123" SEP
			  "123123123123123123123123123" SEP
			  "123123123123123123123123123" SEP
			  "123123123123123123123123123" SEP
			  "123123123123123123123123123");
	gnome_print_moveto (pc, 50, 510);
	gnome_print_show (pc,
			  "456456456456456456456456456" SEP
			  "456456456456456456456456456" SEP
			  "456456456456456456456456456" SEP
			  "456456456456456456456456456" SEP
			  "456456456456456456456456456" SEP
			  "456456456456456456456456456" SEP
			  "456456456456456456456456456" SEP
			  "456456456456456456456456456" SEP
			  "456456456456456456456456456" SEP);
	gnome_print_showpage (pc);
}


static gint
my_draw (GnomePrintContext *gpc, SequenceType sequence)
{
	gint ret = 0;
	
	switch (sequence) {
	case SEQUENCE_SIMPLE:
		my_draw_simple (gpc);
		break;
	case SEQUENCE_PATHS:
		my_draw_paths (gpc);
		break;
	case SEQUENCE_TEXT:
		my_draw_text (gpc);
		break;
	case SEQUENCE_FONTS:
	case SEQUENCE_IMAGES:
	case SEQUENCE_ERROR:
		g_print ("Fatal error: Sequence not implemented. (%s)\n", sequence_str);
		ret = 1;
	}

	return ret;
}

int
main (int argc, const char * argv[])
{
	GnomePrintContext *gpc;
	GnomePrintMaster *gpm;
	GnomePrintConfig *config;
	
	SequenceType sequence;
	BackendType backend;
	gchar *output = NULL;
	int ret = 0;

	FILE *f;
	const guchar *data;
	int len;
	
	parse_command_line (argc, argv, &backend, &sequence, &output);

	g_type_init ();
	
	gpm = gnome_print_master_new ();
	gpc = gnome_print_master_get_context (gpm);
	config = gnome_print_master_get_config (gpm);

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
		fwrite (data, len, 1, f);
		fclose (f);
		goto clean_and_exit;
	case BACKEND_PS:
		break;
	case BACKEND_ERROR:
	case BACKEND_PDF:
	default:
		g_print ("Fatal error: Backend not implemented. (%s)\n", backend_str);
		ret = 1;
	}

	/* We always print to file */
	gnome_print_master_print_to_file (gpm, output);

	if (ret != 0) {
		g_print ("Ret: %d\n", ret);
		return ret;
	}

	ret += my_draw (gpc, sequence);

	gnome_print_master_close (gpm);
	gnome_print_master_print (gpm);

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
		 "    Valid drawing sequences are:\n"
		 "         -simple\n"
		 "         -paths\n"
		 "         -text\n"
		 "         -fonts\n"
		 "         -images\n\n");
	exit (-1);
}

static void
parse_command_line (int argc, const char ** argv,
		    BackendType *backend, SequenceType *sequence,
		    gchar **output)
{
	poptContext popt;
	const gchar **args;

	popt = poptGetContext ("generate", argc, argv, options, 0);
	poptGetNextOpt (popt);

	if (!backend_str)
		*backend = BACKEND_ERROR;
	else if (!strcmp ("ps", backend_str))
		*backend = BACKEND_PS;
	else if (!strcmp ("pdf", backend_str))
		*backend = BACKEND_PDF;
	else if (!strcmp ("meta", backend_str))
		*backend = BACKEND_META;
	else 
		*backend = BACKEND_ERROR;

	if (!sequence_str)
		*sequence = SEQUENCE_ERROR;
	else	if (!strcmp ("simple", sequence_str))
		*sequence = SEQUENCE_SIMPLE;
	else if (!strcmp ("paths", sequence_str))
		*sequence = SEQUENCE_PATHS;
	else if (!strcmp ("text", sequence_str))
		*sequence = SEQUENCE_TEXT;
	else if (!strcmp ("images", sequence_str))
		*sequence = SEQUENCE_IMAGES;
	else if (!strcmp ("fonts", sequence_str))
		*sequence = SEQUENCE_FONTS;
	else
		*sequence = SEQUENCE_ERROR;

	if (*backend == BACKEND_ERROR || *sequence == SEQUENCE_ERROR)
		usage ("Backend or sequence not specicied");

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

