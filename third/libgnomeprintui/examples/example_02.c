/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  example_02.c: sample gnome-print code
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
#include <gdk-pixbuf/gdk-pixbuf.h>

#define NUMBER_OF_PIXELS 256

static void
my_print_image_from_pixbuf (GnomePrintContext *gpc, GdkPixbuf *pixbuf)
{
	guchar *raw_image;
	gboolean has_alpha;
	gint rowstride, height, width;
	
	raw_image = gdk_pixbuf_get_pixels (pixbuf);
	has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	height    = gdk_pixbuf_get_height (pixbuf);
	width     = gdk_pixbuf_get_width (pixbuf);
	
	if (has_alpha)
		gnome_print_rgbaimage (gpc, (char *)raw_image, width, height, rowstride);
	else
		gnome_print_rgbimage (gpc, (char *)raw_image, width, height, rowstride);
}

static void
my_print_image_from_disk (GnomePrintContext *gpc)
{
	GdkPixbuf *pixbuf;
	GError *error;

	/* Load the image into a pixbuf */
	error = NULL;
	pixbuf = gdk_pixbuf_new_from_file ("sample-image.png", &error);
	if (!pixbuf) {
		g_print ("Could not load sample_image.png.\n");
		return;
	}

	/* Save the graphic context, scale, print the image and restore */
	gnome_print_gsave (gpc);
	gnome_print_scale (gpc, 144, 144);
	my_print_image_from_pixbuf (gpc, pixbuf);
	gnome_print_grestore (gpc);

	g_object_unref (G_OBJECT (pixbuf));
}

static void
my_print_image_from_memory (GnomePrintContext *gpc)
{
	gchar color_image [NUMBER_OF_PIXELS] [NUMBER_OF_PIXELS] [3];
	gint pixels = NUMBER_OF_PIXELS;
	gint x, y;

	/* Create the image in memory */
	for (y = 0; y < pixels; y++) {
		for (x = 0; x < pixels; x++) {
			color_image [y][x][0] = (x + y) >> 1;
			color_image [y][x][1] = (x + (pixels - 1  - y)) >> 1;
			color_image [y][x][2] = ((pixels - 1 - x) + y) >> 1;
		}
	}

	/* All images in postscript are printed on a 1 x 1 square, since we
	 * want an image which has a size of 2" by 2" inches, we have to scale
	 * the CTM (Current Transformation Matrix). Save the graphic state and
	 * restore it after we are done so that our scaling does not affect the
	 * drawing calls that follow.
	 */
	gnome_print_gsave (gpc);
	gnome_print_scale (gpc, 144, 144);
	gnome_print_rgbimage (gpc, (char *)color_image, pixels, pixels, pixels * 3);
	gnome_print_grestore (gpc);
}

static void
my_draw (GnomePrintContext *gpc)
{
	gnome_print_beginpage (gpc, "1");

	gnome_print_translate (gpc, 200, 100);
	my_print_image_from_memory (gpc);

	gnome_print_translate (gpc, 0, 150);
	my_print_image_from_disk (gpc);
	
	gnome_print_showpage (gpc);
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

	g_object_unref (gpc);
	g_object_unref (job);
}

int
main (int argc, char * argv[])
{
	g_type_init ();
	
	my_print ();

	g_print ("Done...\n");

	return 0;
}
