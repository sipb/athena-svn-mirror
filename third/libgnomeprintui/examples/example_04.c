/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  example_04.c: sample gnome-print code
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

/*
 * See README
 */

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkmain.h>
#include <string.h>

static void
my_draw (GnomePrintContext *gpc)
{
	GnomeFont *font;
	const guchar *font_name;
	/* Make some UTF-8 strings */
	const guchar acented [] = {0xC3, 0xA0, 0xC3, 0xA8, 0xC3, 0xAC,
				   0xC3, 0xB2, 0xC3, 0xB9, 0x20, 0xC3,
				   0xB1, 0xC3, 0x91, 0x20, 0xC3, 0xBB,
				   0xC3, 0xB4, 0x20, 0x0A, 0x00};
	const guchar cyrillic[] = {0xD0, 0xA1, 0xD0, 0xBE, 0xD0, 0xBC, 0xD0, 0xB5,
				   0x20, 0xD1, 0x80, 0xD0, 0xB0, 0xD0, 0xBD,
				   0xD0, 0xB4, 0xD0, 0xBE, 0xD0, 0xBC, 0x20, 0xD1,
				   0x86, 0xD1, 0x8B, 0xD1, 0x80, 0xD1, 0x83,
				   0xD0, 0xBB, 0xD0, 0xBB, 0xD0, 0xB8, 0xD1, 0x86,
				   0x20, 0xD1, 0x87, 0xD0, 0xB0, 0xD1, 0x80,
				   0xD1, 0x81, 0x00};

	/* Get this font from:
	 *   http://bibliofile.mc.duke.edu/gww/fonts/Unicode.html
	 * I used the TTF Caslon Roman.
	 */
	font = gnome_font_find_closest ("Caslon Roman", 12);
	font_name = gnome_font_get_name (font);
	g_print ("Found: %s\n", font_name);
	if (strcmp (font_name, "Caslon Roman") != 0) {
		g_print ("You might not see cyrillic characters because Caslon Roman was not found.\n");
	}
	
	gnome_print_beginpage (gpc, "1");

	gnome_print_setfont (gpc, font);
	
	gnome_print_moveto (gpc, 100, 700);
	gnome_print_show (gpc, "Some acented characters:");
	gnome_print_moveto (gpc, 100, 680);
	gnome_print_show (gpc, acented);

	gnome_print_moveto (gpc, 100, 650);
	gnome_print_show (gpc, "Some cyrillic:");
	gnome_print_moveto (gpc, 100, 630);
	gnome_print_show (gpc, cyrillic);

	gnome_print_showpage (gpc);

	g_object_unref (G_OBJECT (font));
}

static void
my_print (void)
{
	GnomePrintJob *job;
	GnomePrintContext *gpc;

	job = gnome_print_job_new (NULL);
	gpc = gnome_print_job_get_context (job);

	my_draw (gpc);

	gnome_print_job_close (job);
	gnome_print_job_print (job);

	g_object_unref (G_OBJECT (gpc));
	g_object_unref (G_OBJECT (job));
}

int
main (int argc, char * argv[])
{
	gtk_init (&argc, &argv);
	
	my_print ();

	g_print ("Done...\n");

	return 0;
}
