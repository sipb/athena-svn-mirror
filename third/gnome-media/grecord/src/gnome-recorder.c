/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@prettypeople.org>
 *
 *  Copyright 2002 Iain Holmes
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public
 *  License as published by the Free Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gconf/gconf-client.h>
#include <gnome.h>

#include <gst/gst.h>

#include "gnome-recorder.h"

extern void gsr_window_close (GSRWindow *window);
extern GtkWidget * gsr_window_new (const char *filename);
extern void gnome_media_profiles_init (GConfClient *conf);
 
static GList *windows = NULL;

static void
window_destroyed (GtkWidget *window,
		  gpointer data)
{
	windows = g_list_remove (windows, window);

	if (windows == NULL) {
		gtk_main_quit ();
	}
}

void
gsr_quit (void) 
{
	GList *p;

	for (p = windows; p;) {
		GSRWindow *window = p->data;

		/* p is set here instead of in the for statement,
		   because by the time we get back to the loop,
		   p will be invalid */
		p = p->next;
		gsr_window_close (window);
	}
}

void
gsr_foreach_window (GSRForeachFunction func,
		    gpointer closure)
{
	GList *p;

	for (p = windows; p; p = p->next) {
		func (p->data, closure);
	}
}

GtkWidget *
gsr_open_window (const char *filename)
{
	GtkWidget *window;
	char *name;

	if (filename == NULL) {
		static int sample_count = 1;

		if (sample_count != 1) {
			name = g_strdup_printf ("Untitled-%d", sample_count);
		} else {
			name = g_strdup ("Untitled");
		}

		sample_count++;
	} else {
		name = g_strdup (filename);
	}

	window = GTK_WIDGET (gsr_window_new (name));
	g_free (name);
	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (window_destroyed), NULL);

	windows = g_list_prepend (windows, window);
	gtk_widget_show (window);

	return window;
}

static const char *stock_items[] =
	{
		GSR_STOCK_PLAY,
		GSR_STOCK_RECORD,
		GSR_STOCK_STOP
	};

static void
init_stock_icons (void)
{
	GtkIconFactory *factory;
	int i;

	factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (factory);

	for (i = 0; i < (int) G_N_ELEMENTS (stock_items); i++) {
		GtkIconSet *icon_set;
		GdkPixbuf *pixbuf;
		char *filename, *fullname;

		filename = g_strconcat ("gnome-media/gnome-sound-recorder/",
					stock_items[i], ".png", NULL);
		fullname = gnome_program_locate_file (NULL,
						      GNOME_FILE_DOMAIN_APP_PIXMAP,
						      filename, TRUE, NULL);
		g_free (filename);

		pixbuf = gdk_pixbuf_new_from_file (fullname, NULL);
		g_free (fullname);

		icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
		gtk_icon_factory_add (factory, stock_items[i], icon_set);
		gtk_icon_set_unref (icon_set);

		g_object_unref (G_OBJECT (pixbuf));
	}

	g_object_unref (G_OBJECT (factory));
}

int
main (int argc,
      char **argv)
{
	GConfClient *conf;
	GnomeProgram *program;
	GtkIconInfo *icon_info;
	poptContext pctx;
	GValue value = {0, };
	char **args = NULL;

	static struct poptOption gsr_options[] = {
		{ NULL, '\0', POPT_ARG_INCLUDE_TABLE, NULL, 0, "GStreamer", NULL },
		{ NULL, 'p', POPT_ARG_NONE, NULL, 1, N_("Dummy option"), NULL },
		POPT_TABLEEND
	};

	/* Init gettext */
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* init gstreamer */
	gsr_options[0].arg = (void *) gst_init_get_popt_table ();

	/* Init GNOME */
	program = gnome_program_init ("gnome-sound-recorder", VERSION,
				      LIBGNOMEUI_MODULE,
				      argc, argv,
				      GNOME_PARAM_POPT_TABLE, gsr_options,
				      GNOME_PARAM_HUMAN_READABLE_NAME,
				      "GNOME Sound Recorder",
				      GNOME_PARAM_APP_DATADIR, DATADIR,
				      NULL);
	conf = gconf_client_get_default ();

	icon_info = gtk_icon_theme_lookup_icon (gtk_icon_theme_get_default (),
						"gnome-audio2", 48, 0);
	if (icon_info) {
		gnome_window_icon_set_default_from_file (gtk_icon_info_get_filename (icon_info));
		gtk_icon_info_free (icon_info);
	}


	/* Init the icons */
	init_stock_icons ();

        /* init gnome-media-profiles */
        gnome_media_profiles_init (conf);

	/* Get the args */
	g_value_init (&value, G_TYPE_POINTER);
	g_object_get_property (G_OBJECT (program),
			       GNOME_PARAM_POPT_CONTEXT, &value);
	pctx = g_value_get_pointer (&value);
	g_value_unset (&value);

	args = (char **) poptGetArgs (pctx);
	if (args == NULL) {
		gsr_open_window (NULL);
	} else {
		int i;

		for (i = 0; args[i]; i++) {
			gsr_open_window (args[i]);
		}
	}

	poptFreeContext (pctx);

	gtk_main ();

	return 0;
}

