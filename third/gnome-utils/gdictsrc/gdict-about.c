/* $Id: gdict-about.c,v 1.1.1.4 2004-10-04 05:06:52 ghudson Exp $ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Mike Hughes <mfh@psilord.com>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict About box
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>

#include "gdict-about.h"


GtkWidget *gdict_about_new (void)
{
    GdkPixbuf   *pixbuf = NULL;
    GError  	*error = NULL;
    GtkIconInfo *icon_info;
    
    const gchar *authors[] = {
        "Mike Hughes <mfh@psilord.com>",
        "Spiros Papadimitriou <spapadim+@cs.cmu.edu>",
        "Bradford Hovinen <hovinen@udel.edu>",
        NULL
    };
    gchar *documenters[] = {
	    NULL
    };
    /* Translator credits */
    gchar *translator_credits = _("translator-credits");
    GtkWidget *about;
    
    icon_info = gtk_icon_theme_lookup_icon (gtk_icon_theme_get_default (), "gdict", 48, 0);
    if (icon_info) {
        pixbuf = gtk_icon_info_load_icon (icon_info, &error);
        
        if (error) {
    	   g_warning (G_STRLOC ": cannot open %s: %s", gtk_icon_info_get_filename (icon_info), error->message);
	   g_error_free (error);	
        }
    }
    
    about = gnome_about_new (_("Dictionary"), VERSION,
                            "Copyright \xc2\xa9 1999-2003 Mike Hughes",
                            _("A client for the MIT dictionary server."),
			     (const char **)authors,
			     (const char **)documenters,
			     strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
                             pixbuf);
    if (pixbuf) {
    	   gdk_pixbuf_unref (pixbuf);
    }

    gnome_window_icon_set_from_file (GTK_WINDOW (about), gtk_icon_info_get_filename (icon_info));
    
    if (icon_info) {
    	gtk_icon_info_free (icon_info);
    }
    
    return about;
}

void gdict_about (GtkWindow *parent)
{
    static GtkWidget *about = NULL;

    if (about == NULL) {
      about = gdict_about_new();
      g_signal_connect (G_OBJECT (about), "destroy",
			G_CALLBACK (gtk_widget_destroyed), &about);
      if (parent) {
        gtk_window_set_transient_for (GTK_WINDOW (about), parent) ;
      }
    }
    gtk_window_present (GTK_WINDOW (about));
}

