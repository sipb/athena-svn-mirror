/* gnopiui.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "gnopiui.h"
#include "SRMessages.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <libgnomeui/libgnomeui.h>
#include "srintl.h"
#include <signal.h>
#include "brlui.h"
#include "defui.h"
#include "genui.h"
#include "kbui.h"
#include "magui.h"
#include "spui.h"
#include "cmdmapui.h"
#include "findui.h"
#include "presui.h"
#include "langui.h"
#include "scrui.h"
#include "bmui.h"
enum 
{
    BRAILLE_SETTINGS = 0,
    BRAILLE_MONITOR_SETTINGS,
    MAGNIFIER_SETTINGS,
    SPEECH_SETTINGS,
    NUMBER_OF_SETTINGS
};

GtkWidget		*w_gnopernicus;
GtkWidget		*w_assistive_preferences;
static GtkWidget	*bt_settings [NUMBER_OF_SETTINGS];
extern gboolean 	exitackget;
extern General 	 	*general_setting;

GladeXML* 
gn_load_interface (const gchar *glade_file, 
		   const gchar *window)
{
    GladeXML *xml;
    gchar *path;
    
    path = g_strdup_printf("./gnopi_files/%s", glade_file);
	
    if (g_file_test ( path , G_FILE_TEST_EXISTS|G_FILE_TEST_IS_REGULAR))
    {
	xml = glade_xml_new_with_domain (path , window, GETTEXT_PACKAGE);
	g_free (path);
	if (!xml) 
	{
	    sru_warning (_("We could not load the interface!"));
	    return NULL;
	}
    }
    else	
    {
	g_free (path);
	path = g_strdup_printf("%sgnopi_files/%s", GNOPI_GLADEDIR, glade_file);
	if (g_file_test (path, G_FILE_TEST_EXISTS|G_FILE_TEST_IS_REGULAR))
	{
	    xml = glade_xml_new_with_domain (path , window, GETTEXT_PACKAGE);
	    g_free (path);
	    if (!xml) 
	    {
	        sru_warning (_("We could not load the interface!"));
	        return NULL;
	    }
	}
	else
	{
	    g_free (path);
	    if (g_file_test (glade_file ,G_FILE_TEST_EXISTS|G_FILE_TEST_IS_REGULAR))
	    {
		xml = glade_xml_new_with_domain (glade_file, window, GETTEXT_PACKAGE);
		if (!xml) 
		{
		    sru_warning (_("We could not load the interface!"));
		    return NULL;
		}
	    }
	    else
	    {
		sru_warning (_("We could not load the interface!"));
		return NULL;
	    }
	}
    }
		    
    return xml;
}

void
gn_load_help (const gchar *section)
{
    GError *error = NULL;

    if (!g_file_test ("../help/gnopernicus/C/gnopernicus.xml", G_FILE_TEST_EXISTS))
	gnome_help_display ("gnopernicus", section, &error);
	
    if (error != NULL)
    {
	sru_warning (error->message);
	g_error_free (error);
    }
}

void 
gn_show_message (const gchar *msg)
{
    GtkWidget *dialog = NULL;
    dialog = gtk_message_dialog_new (
	    NULL,
	    GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_MESSAGE_INFO,GTK_BUTTONS_OK,
	    "%s",
	    msg);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

static void
gn_load_gnopernicus_about (void)
{
    GladeXML 		*xml;
    gchar		*file = NULL;
    GtkWidget		*about = NULL;
    GdkPixbuf		*logo = NULL;
    
    xml = gn_load_interface ("about.glade2", "dl_about");

    about = glade_xml_get_widget (xml, "dl_about");

    file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, 
				     "gnopernicus.png", 
				      TRUE, NULL);

    if (!file)
    {
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, 
				     "gnome-about-logo.png", 
				      TRUE, NULL);
    }

    g_object_set (about, "version", VERSION, NULL);

    if (file)
    {
	logo = gdk_pixbuf_new_from_file (file, NULL);
	g_object_set (about, "logo", logo, NULL);
    }

    if (xml) 
	g_object_unref (G_OBJECT (xml));

    if (logo)
	g_object_unref (logo);

    gtk_widget_show (about);
}

void
gn_iconify_gnopernicus ()
{
    gtk_window_iconify (GTK_WINDOW (w_gnopernicus));
}

void
gn_iconify_menu_clicked (GtkButton       *button,
                	 gpointer         user_data)
{
    gn_iconify_gnopernicus ();
}

/**
 *
 * Time count callback function used at exit.
 * data - user data (not used)
 *
**/
#define COUNT_OF_TIME_OUT_CALLS 	7
gboolean 
gn_out_gnopernicus (gpointer data)
{
    static int n = 0;
    
    if (n < COUNT_OF_TIME_OUT_CALLS && 
	exitackget == FALSE)
    {
	srcore_exit_all (FALSE);
	srcore_exit_all (TRUE);
	n++;
	return TRUE;
    }
    else
    {
        gtk_main_quit ();
    }
    
    return FALSE;
}

#define EXIT_TIME_OUT_INTERVAL 300
static void
gn_exit_gnopernicus_clicked 	(GtkWidget       *button,
                                gpointer         user_data)
{
    signal (SIGCHLD, SIG_IGN);		
    g_timeout_add (EXIT_TIME_OUT_INTERVAL, gn_out_gnopernicus, NULL);
}

static void
gn_general_settings_clicked (GtkButton       *button,
                             gpointer         user_data)
{
    genui_load_general_settings_interface (w_gnopernicus);
}

static void
gn_io_settings_clicked (GtkButton       *button,
                	gpointer         user_data)
{
    gn_load_io_settings ();
}
/*
static void
gn_configure_clicked (GtkButton       *button,
                      gpointer         user_data)
{
    gn_load_configure ();
}
*/
static void
gn_load_default_settings_clicked (GtkButton       *button,
                            	  gpointer         user_data)
{
    defui_load_default_load (w_gnopernicus);
}

static void
gn_help_clicked	(GtkButton       *button,
                 gpointer         user_data)
{
    gn_load_help (NULL);
}

static void
gn_about_clicked (GtkButton       *button,
                  gpointer         user_data)
{
    gn_load_gnopernicus_about ();
}


/**
 *
 * Set event handlers and get a widgets used in this interface.
 * xml - glade interface XML pointer
 *
**/
void 
gn_set_handlers_gnopi (GladeXML *xml)
{        
    glade_xml_signal_connect (xml, "on_w_gnopernicus_window",		
			GTK_SIGNAL_FUNC (gn_exit_gnopernicus_clicked));
    glade_xml_signal_connect (xml, "on_bt_general_settings_clicked",	
			GTK_SIGNAL_FUNC (gn_general_settings_clicked));
    glade_xml_signal_connect (xml, "on_bt_io_settings_clicked",		
			GTK_SIGNAL_FUNC (gn_io_settings_clicked));
    glade_xml_signal_connect (xml, "on_bt_help_clicked",		
			GTK_SIGNAL_FUNC (gn_help_clicked));
    glade_xml_signal_connect (xml, "on_bt_about_clicked",		
			GTK_SIGNAL_FUNC (gn_about_clicked));
    glade_xml_signal_connect (xml, "on_bt_load_default_settings_clicked", 
			GTK_SIGNAL_FUNC (gn_load_default_settings_clicked));
    glade_xml_signal_connect (xml, "on_bt_exit_gnopernicus_clicked",	
			GTK_SIGNAL_FUNC (gn_exit_gnopernicus_clicked));
    glade_xml_signal_connect (xml, "on_bt_iconify_menu_clicked",	
			GTK_SIGNAL_FUNC (gn_iconify_menu_clicked));			
}



/**
 *
 * Main menu user interface loader function
 *
**/
gboolean 
gn_load_gnopi (void)
{
    GladeXML *xml;
    xml = gn_load_interface ("gnopi.glade2", "gnopernicus_window");

    sru_return_val_if_fail (xml, FALSE);
    
    w_gnopernicus = glade_xml_get_widget (xml, "gnopernicus_window");

    gn_set_handlers_gnopi (xml);

    if (xml) 
	g_object_unref (G_OBJECT (xml));
        
    return TRUE;
}

static void
gn_keyboard_clicked (GtkButton       *button,
                     gpointer         user_data)
{
    kbui_load_keyboard_settings (w_assistive_preferences);
}

static void
gn_magnifier_settings_clicked (GtkButton       *button,
                               gpointer         user_data)
{
    magui_load_magnifier_settings_interface (w_assistive_preferences);
}

static void
gn_braille_settings_clicked (GtkButton       *button,
                             gpointer         user_data)
{
    brlui_load_braille_settings (w_assistive_preferences);
}

static void
gn_speech_settings_clicked (GtkButton       *button,
                            gpointer         user_data)
{
    spui_load_speech_settings (w_assistive_preferences);
}

static void
gn_braille_monitor_clicked (GtkButton	*button,
			    gpointer	user_data)
{
    bmui_load_braille_monitor_settings (w_assistive_preferences);
}

static void
gn_preferences_close_clicked (GtkButton       *button,
                    	      gpointer         user_data)
{
    gtk_widget_hide (w_assistive_preferences);
}

static void
gn_assistive_preferences_remove (GtkWidget       *widget,
                    		 gpointer         user_data)
{
    gtk_widget_hide (w_assistive_preferences);
    w_assistive_preferences = NULL;
}

static void
gn_command_mapping_clicked (GtkButton       *button,
                    	    gpointer         user_data)
{
    cmdui_load_user_properties (w_assistive_preferences);
}


static void
gn_presentation_clicked (GtkWidget 	*widget,
			 gpointer	user_data)
{
    presui_load_presentation (w_assistive_preferences);
}

static void
gn_screen_review_clicked (GtkWidget 	*widget,
			  gpointer	user_data)
{
    scrui_load_screen_review (w_assistive_preferences);
}

static void
gn_find_clicked (GtkButton       *button,
                 gpointer         user_data)
{    
    fnui_load_find ();
}


static void
gn_set_modules_sensitivity (void)
{
    gtk_widget_set_sensitive (GTK_WIDGET (bt_settings[BRAILLE_SETTINGS]), general_setting->braille);
    gtk_widget_set_sensitive (GTK_WIDGET (bt_settings[SPEECH_SETTINGS]),  general_setting->speech);
    gtk_widget_set_sensitive (GTK_WIDGET (bt_settings[MAGNIFIER_SETTINGS]), general_setting->magnifier);
    gtk_widget_set_sensitive (GTK_WIDGET (bt_settings[BRAILLE_MONITOR_SETTINGS]), general_setting->braille_monitor);
}
/**
 *
 * Set event handlers and get a widgets used in this interface.
 * xml - glade interface XML pointer
 *
**/
void 
gn_set_handlers_io_settings (GladeXML *xml)
{    
    bt_settings[MAGNIFIER_SETTINGS] = glade_xml_get_widget (xml, "bt_magnifier_settings");
    bt_settings[BRAILLE_SETTINGS]   = glade_xml_get_widget (xml, "bt_braille_settings");
    bt_settings[BRAILLE_MONITOR_SETTINGS] = glade_xml_get_widget (xml, "bt_braille_monitor");
    bt_settings[SPEECH_SETTINGS]    = glade_xml_get_widget (xml, "bt_speech_settings");
    
    glade_xml_signal_connect (xml, "on_w_assistive_preferences_remove",		
			    GTK_SIGNAL_FUNC (gn_assistive_preferences_remove));
    glade_xml_signal_connect (xml, "on_bt_speech_settings_clicked",	
			    GTK_SIGNAL_FUNC (gn_speech_settings_clicked));
    glade_xml_signal_connect (xml, "on_bt_braille_settings_clicked",	
			    GTK_SIGNAL_FUNC (gn_braille_settings_clicked));
    glade_xml_signal_connect (xml, "on_bt_magnifier_settings_clicked",	
			    GTK_SIGNAL_FUNC (gn_magnifier_settings_clicked));
    glade_xml_signal_connect (xml, "on_bt_keyboard_clicked",		
			    GTK_SIGNAL_FUNC (gn_keyboard_clicked));
    glade_xml_signal_connect (xml, "on_bt_command_mapping_clicked",	
			    GTK_SIGNAL_FUNC (gn_command_mapping_clicked));
    glade_xml_signal_connect (xml, "on_bt_role_presentation_clicked",
			    GTK_SIGNAL_FUNC (gn_presentation_clicked));
    glade_xml_signal_connect (xml, "on_bt_screen_review_clicked",
			    GTK_SIGNAL_FUNC (gn_screen_review_clicked));
    glade_xml_signal_connect (xml, "on_bt_find_clicked",		
			    GTK_SIGNAL_FUNC (gn_find_clicked));
    glade_xml_signal_connect (xml, "on_bt_preferences_close_clicked",		
			    GTK_SIGNAL_FUNC (gn_preferences_close_clicked));
    glade_xml_signal_connect (xml, "on_bt_braille_monitor_clicked",	
			GTK_SIGNAL_FUNC (gn_braille_monitor_clicked));			
}



/**
 *
 * Main menu user interface loader function
 *
**/
gboolean 
gn_load_io_settings (void)
{
    if (!w_assistive_preferences)
    {
	GladeXML *xml;
	xml = gn_load_interface ("gnopi.glade2", "w_assistive_preferences");
	
	sru_return_val_if_fail (xml, FALSE);
	
	w_assistive_preferences = glade_xml_get_widget (xml, "w_assistive_preferences");
	
	gn_set_handlers_io_settings (xml);

	if (xml) 
	    g_object_unref (G_OBJECT (xml));
	    
	gtk_window_set_transient_for (GTK_WINDOW (w_assistive_preferences),
				      GTK_WINDOW (w_gnopernicus));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_assistive_preferences), TRUE);
    }
    else
	gtk_widget_show (w_assistive_preferences);
    
    
    gn_set_modules_sensitivity ();
    
    return TRUE;
}

