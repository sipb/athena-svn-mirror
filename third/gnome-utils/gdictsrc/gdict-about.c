/* $Id: gdict-about.c,v 1.1.1.3 2003-01-29 20:33:24 ghudson Exp $ */

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


static GtkWidget *gdict_about_new (void)
{
    GdkPixbuf   *pixbuf;
    GError  	*error = NULL;
    gchar 	*file;
    
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
    gchar *translator_credits = _("translator_credits");
    GtkWidget *about;
    
    file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gdict.png", FALSE, NULL);
    pixbuf = gdk_pixbuf_new_from_file (file, &error);
    
    if (error) {
    	   g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
	   g_error_free (error);	
    }
    
    g_free (file);    
    
    about = gnome_about_new (_("GNOME Dictionary"), VERSION,
                            _("Copyright 1999 by Mike Hughes"),
                            _("Client for MIT dictionary server.\n"),
			     (const char **)authors,
			     (const char **)documenters,
			     strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
                             pixbuf);
    if (pixbuf) {
    	   gdk_pixbuf_unref (pixbuf);
    }

    gnome_window_icon_set_from_file (GTK_WINDOW (about), GNOME_ICONDIR"/gdict.png");				     
			     
    return about;
}

void gdict_about (GtkWindow *parent)
{
    GtkWidget *about = gdict_about_new();
    if (parent) {
      gtk_window_set_transient_for (GTK_WINDOW (about), parent) ;
    }
    gtk_widget_show(about);
}

