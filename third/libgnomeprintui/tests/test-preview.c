/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  test-preview.c: Test the print preview
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
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprintui/gnome-print-job-preview.h>

poptContext popt;
gboolean option_timeout_kill;
gboolean option_theme_colors;

static struct poptOption options[] = {
	{ "kill", '\0', POPT_ARG_NONE, &option_timeout_kill, 0,
	  "Kill ourselves after 1 second timeout", NULL},
	{ "theme-colors", '\0', POPT_ARG_NONE, &option_theme_colors, 0,
	  "Use only the theme colors", NULL},
	POPT_AUTOHELP
	{ NULL }
};

static void
my_draw (GnomePrintContext *pc, guint page)
{
	gint i, j;
	gint max_i = 10;
	gint max_j = 10;
	gint size, spacing, x, y;
	GnomeFont *font;
	gchar *txt;

	gnome_print_beginpage (pc, "1");
	
	font = gnome_font_find_closest ("Times New Roman", 44);
	gnome_print_setfont (pc, font);

	gnome_print_moveto (pc, 50, 50);
	gnome_print_show (pc, "Print Preview test");

	txt = g_strdup_printf ("Page %i", page);
	gnome_print_moveto (pc, 50, 800);
	gnome_print_show (pc, txt);
	g_free (txt);

	gnome_print_moveto (pc, 50, 100);
	gnome_print_lineto (pc, 450, 100);
	gnome_print_lineto (pc, 450, 150);
	gnome_print_lineto (pc, 50, 150);
	gnome_print_closepath (pc);
	gnome_print_fill (pc);

	
	gnome_print_setrgbcolor (pc, 1, 1, 1);
	gnome_print_moveto (pc, 55, 105);
	gnome_print_show (pc, "Inverted text");

	gnome_print_setrgbcolor (pc, 1, 0, 0);
	gnome_print_moveto (pc, 55, 160);
	gnome_print_show (pc, "Red");

	gnome_print_setrgbcolor (pc, 0, 1, 0);
	gnome_print_moveto (pc, 155, 160);
	gnome_print_show (pc, "Green");

	gnome_print_setrgbcolor (pc, 0, 0, 1);
	gnome_print_moveto (pc, 305, 160);
	gnome_print_show (pc, "Blue");
	
	for (i = 0; i < max_i; i++) {
		for (j = 0; j < max_j; j++) {
			gdouble r;
			gdouble g;
			gdouble b;

			r = ((gdouble) i) / ((gdouble) max_i);
			g = ((gdouble) j) / ((gdouble) max_j);
			b = 1 - r;

			size = 15;
			spacing = size + 2;
			x = 100;
			y = 250;

			gnome_print_setrgbcolor (pc, r, g, b);
			gnome_print_moveto (pc, x + (i * spacing), y + (j * spacing));
			gnome_print_lineto (pc, x + (i * spacing), y + (j * spacing) + size);
			gnome_print_lineto (pc, x + (i * spacing) + size, y + (j * spacing) + size);
			gnome_print_lineto (pc, x + (i * spacing) + size, y + (j * spacing));
			gnome_print_closepath (pc);
			gnome_print_fill (pc);
		}
	}

	for (i = 0; i < max_i; i++) {
		for (j = 0; j < max_j; j++) {
			gdouble r;
			gdouble g;
			gdouble b;

			r = ((gdouble) i) / ((gdouble) max_i);
			g = ((gdouble) j) / ((gdouble) max_j);
			b = 1 - r;

			size = 15;
			spacing = size + 2;
			x = 300;
			y = y;

			gnome_print_setrgbcolor (pc, r, g, b);
			gnome_print_moveto (pc, x + (i * spacing), y + (j * spacing));
			gnome_print_lineto (pc, x + (i * spacing), y + (j * spacing) + size);
			gnome_print_lineto (pc, x + (i * spacing) + size, y + (j * spacing) + size);
			gnome_print_lineto (pc, x + (i * spacing) + size, y + (j * spacing));
			gnome_print_closepath (pc);
			gnome_print_stroke (pc);
		}
	}


	for (i = 0; i < max_i; i++) {
		for (j = 0; j < max_j; j++) {
			gdouble r;
			gdouble g;
			gdouble b;

			r = ((gdouble) i) / ((gdouble) max_i);
			g = r;
			b = r;

			size = 15;
			spacing = size + 2;
			x = 100;
			y = 450;

			gnome_print_setrgbcolor (pc, r, g, b);
			gnome_print_moveto (pc, x + (i * spacing), y + (j * spacing));
			gnome_print_lineto (pc, x + (i * spacing), y + (j * spacing) + size);
			gnome_print_lineto (pc, x + (i * spacing) + size, y + (j * spacing) + size);
			gnome_print_lineto (pc, x + (i * spacing) + size, y + (j * spacing));
			gnome_print_closepath (pc);
			gnome_print_stroke (pc);
		}
	}

	for (i = 0; i < max_i; i++) {
		for (j = 0; j < max_j; j++) {
			gdouble r;
			gdouble g;
			gdouble b;

			r = ((gdouble) i) / ((gdouble) max_i);
			g = r;
			b = r;

			size = 15;
			spacing = size + 2;
			x = 300;
			y = 450;

			gnome_print_setrgbcolor (pc, r, g, b);
			gnome_print_moveto (pc, x + (i * spacing), y + (j * spacing));
			gnome_print_lineto (pc, x + (i * spacing), y + (j * spacing) + size);
			gnome_print_lineto (pc, x + (i * spacing) + size, y + (j * spacing) + size);
			gnome_print_lineto (pc, x + (i * spacing) + size, y + (j * spacing));
			gnome_print_closepath (pc);
			gnome_print_fill (pc);
		}
	}

	
	gnome_print_showpage (pc);
}

static void
my_print (void)
{
	GnomePrintContext *gpc;
	GnomePrintJob *job;
	GtkWidget *preview;
	guint n;

	job    = gnome_print_job_new (NULL);
	gpc    = gnome_print_job_get_context (job);

	for (n = 0; n < 6; n++) my_draw (gpc, n + 1);

	gnome_print_job_close (job);

	preview = gnome_print_job_preview_new (job, "test-preview.c");
	g_signal_connect (G_OBJECT (preview), "unrealize",
			  G_CALLBACK (gtk_main_quit), NULL);

	gtk_widget_show (preview);

}

int
main (int argc, char * argv[])
{
	popt = poptGetContext ("test-preview", argc, (const char **)argv, options, 0);
	poptGetNextOpt (popt);
	
	gtk_init (&argc, &argv);
	
	my_print ();

	if (option_timeout_kill)
		g_timeout_add (2000, (GSourceFunc) gtk_main_quit, NULL);
	
	gtk_main ();
	
	return 0;
}
