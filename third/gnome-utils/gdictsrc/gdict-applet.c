/* $Id: gdict-applet.c,v 1.1.1.5 2004-10-04 05:06:37 ghudson Exp $ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict panel applet
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>
#include <libbonoboui.h>
#include <panel-applet.h>

#include "gdict-app.h"
#include "gdict-about.h"
#include "gdict-pref.h"
#include "gdict-applet.h"
#include "gdict-pref-dialog.h"



#define DOCKED_APPLET_WIDTH 74
#define FLOATING_APPLET_WIDTH 14
#define SHORT_APPLET_HEIGHT 22
#define TALL_APPLET_HEIGHT 44

/* This is a hack around the fact that gtk+ doesn't
 * propogate button presses on button2/3.
 */
static gboolean 
button_press_hack (GtkWidget *w, GdkEventButton *event, gpointer data)
{
    GtkWidget *applet = GTK_WIDGET (data);
    
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (applet, (GdkEvent *) event);

	return TRUE;
    }
    
    return FALSE;
    
}

static void
entry_activate_cb (GtkEntry *entry, gpointer data)
{
	GDictApplet *applet = data;

	gchar *text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    
    	if (!context)
		gdict_init_context ();
    	if (!GTK_WIDGET_VISIBLE (gdict_app))
        	gtk_widget_show (gdict_app);
   
	gtk_window_set_screen (GTK_WINDOW (gdict_app),
			       gtk_widget_get_screen (applet->applet_widget));
	
    	gtk_window_present (GTK_WINDOW (gdict_app));
    
    	if (!text || *text == 0)
        	return;
        
    	g_utf8_strdown (text, strlen (text));
	gtk_editable_delete_text (GTK_EDITABLE (entry), 0, -1);
    	gdict_app_do_lookup (text);
    	g_free (text);
 

}

static void
button_press_cb (GtkButton *button, gpointer data)
{
	GDictApplet *applet = data;
	
	entry_activate_cb (GTK_ENTRY (applet->entry_widget), applet);
	
}

static void
lookup_button_drag_cb (GtkWidget *widget, GdkDragContext *context, gint x, gint y,
		       GtkSelectionData *sd, guint info, guint t, gpointer data)
{
	GDictApplet *applet = data;
	gchar *text;
	
	text = gtk_selection_data_get_text (sd);
	
	if (text) {		
    		gtk_entry_set_text (GTK_ENTRY (applet->entry_widget), text);
		entry_activate_cb (GTK_ENTRY (applet->entry_widget), applet);
    		g_free (text);
	}
	
}

static void
setup_applet_widgets (GDictApplet *applet)
{
 	static GtkTargetEntry drop_targets [] = {
    		{ "UTF8_STRING", 0, 0 },
  		{ "COMPOUND_TEXT", 0, 0 },
  		{ "TEXT", 0, 0 },
  		{ "text/plain", 0, 0 },
  		{ "STRING",     0, 0 }
    	};
	
	applet->about_dialog = NULL;

	if (applet->box) {
		gtk_widget_destroy (applet->box);
	}

	if ((applet->orient == PANEL_APPLET_ORIENT_LEFT || applet->orient == PANEL_APPLET_ORIENT_RIGHT) || 
	    ((applet->orient == PANEL_APPLET_ORIENT_UP || applet->orient == PANEL_APPLET_ORIENT_DOWN) && applet->panel_size > 48)) {
		applet->box = gtk_vbox_new (FALSE, 2);
	}
	else {
		applet->box = gtk_hbox_new (FALSE, 2);
	} 
	
	gtk_container_set_border_width (GTK_CONTAINER (applet->box), 0);

	applet->button_widget = gtk_button_new_with_mnemonic (_("_Lookup"));

	gtk_drag_dest_set (applet->button_widget, GTK_DEST_DEFAULT_ALL, drop_targets, 
    		           G_N_ELEMENTS (drop_targets), GDK_ACTION_COPY | GDK_ACTION_MOVE);
	gtk_box_pack_end (GTK_BOX (applet->box), applet->button_widget, TRUE, TRUE, 0);
	
        if (GTK_IS_ACCESSIBLE (gtk_widget_get_accessible (applet->button_widget)))          
        add_atk_namedesc (GTK_WIDGET (applet->button_widget), _("Dictionary Lookup"), NULL);

	applet->entry_widget = gtk_entry_new ();
	if (GTK_IS_ACCESSIBLE (gtk_widget_get_accessible (applet->entry_widget)))
		add_atk_namedesc (GTK_WIDGET (applet->entry_widget), _("Dictionary word entry"), NULL);

	gtk_entry_set_width_chars (GTK_ENTRY (applet->entry_widget), 12);
	gtk_box_pack_start (GTK_BOX (applet->box), applet->entry_widget, FALSE, FALSE, 0);

 	gtk_widget_show_all (applet->box);
	  	
 	gtk_container_add(GTK_CONTAINER(applet->applet_widget), applet->box);                              

 	gtk_widget_show (applet->applet_widget);

	gtk_tooltips_set_tip (GTK_TOOLTIPS(applet->tooltip_widget), applet->entry_widget,
			      _("Lookup words in an online dictionary"), NULL);

 	g_signal_connect (G_OBJECT (applet->entry_widget), "activate",
 			  G_CALLBACK (entry_activate_cb), applet);
			  
	g_signal_connect (G_OBJECT (applet->button_widget), "button_press_event",
			  G_CALLBACK (button_press_hack), applet->applet_widget);
	g_signal_connect (G_OBJECT (applet->button_widget), "clicked",
 			  G_CALLBACK (button_press_cb), applet);
 	g_signal_connect (G_OBJECT (applet->button_widget), "drag_data_received",
    		          G_CALLBACK (lookup_button_drag_cb), applet);
}

static void
change_size_cb (PanelApplet *widget, gint size, GDictApplet *applet)
{
	applet->panel_size = size;
	setup_applet_widgets (applet);

}

static void
change_orient_cb (PanelApplet *widget, PanelAppletOrient orient, GDictApplet *applet)
{
	applet->orient = orient;
	setup_applet_widgets (applet);
	
}

static gboolean
set_show_dictionary_menu_state (GtkWidget *widget, GdkEventButton *event, gpointer *data)
{
	BonoboUIComponent *component;
	    
 	if (event->button == 3) { 
    
		if (!context) {
			gdict_init_context ();
		}

		component = panel_applet_get_popup_component ((PanelApplet *)widget);
		bonobo_ui_component_set_prop (component,
					      "/commands/def win",
					      "state",
					      GTK_WIDGET_VISIBLE (gdict_app) ? "1" : "0",
				 	      NULL); 
				      
    }
    return FALSE;
}
                                                                                
static gboolean
send_button_to_entry_event (GtkWidget *widget, GdkEventButton *event, GDictApplet *applet)
{
	if (event->button == 1) {
		gtk_widget_grab_focus (applet->entry_widget);
		return TRUE;
	}
	return FALSE;
}

static gboolean
destroy_applet (GtkWidget *widget, gpointer data)
{
	GDictApplet *applet = data;

	if (applet->about_dialog)
		gtk_widget_destroy (applet->about_dialog);

	gtk_widget_destroy (applet->button_widget);
	gtk_widget_destroy (applet->box);
	gtk_widget_destroy (applet->entry_widget);
	gtk_widget_destroy (applet->applet_widget);
}

static void 
menu_event_cb (BonoboUIComponent            *comp,
               const gchar                  *path,
               Bonobo_UIComponent_EventType  type,
               const gchar                  *state_string,
               gpointer                     *data)
{
	if (!context) {
		gdict_init_context ();
	}
		
	if (strcmp (path, "def win") == 0) {
		
		if (strcmp (state_string, "1") == 0) {
			if (!GTK_WIDGET_VISIBLE (gdict_app)) { 
        			gtk_widget_show (gdict_app);
			}
		}
		else if (GTK_WIDGET_VISIBLE (gdict_app)) { 
			gtk_widget_hide (gdict_app);
		}
	}
}

static void
text_received (GtkClipboard *clipboard, const gchar *text, gpointer data)
{
	GDictApplet *applet = data;
	gchar *lookup_text;
	
	if (!text)
		return;
	
	lookup_text = g_strdup (text);
	g_strstrip (lookup_text);
	
	if (*lookup_text == 0)
        	return;
    	g_utf8_strdown (lookup_text, strlen(lookup_text));
    	gtk_entry_set_text (GTK_ENTRY (applet->entry_widget), text);
    	entry_activate_cb (GTK_ENTRY (applet->entry_widget), applet);
#if 0    	
    	gdict_app_do_lookup (lookup_text);
    	gtk_entry_set_text (GTK_ENTRY (word_entry), lookup_text);
#endif
    	g_free (lookup_text);
 
	
}

static void
lookup_selected_text (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	GDictApplet *applet = data;
	GtkClipboard *clipboard;

	clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
 	gtk_clipboard_request_text (clipboard, text_received, applet);
 	
}
static void
about_cb (BonoboUIComponent *uic,
          gpointer           user_data,
          const gchar       *verbname)
{
	GDictApplet *applet = user_data;

	if (applet->about_dialog) {
		gtk_window_set_screen (GTK_WINDOW (applet->about_dialog),
				       gtk_widget_get_screen (applet->applet_widget));

		gtk_window_present (GTK_WINDOW (applet->about_dialog));
		return;
	}

	applet->about_dialog = gdict_about_new ();

	gtk_window_set_screen (GTK_WINDOW (applet->about_dialog),
			       gtk_widget_get_screen (applet->applet_widget));

	g_signal_connect (G_OBJECT (applet->about_dialog), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &applet->about_dialog);

	gtk_widget_show (applet->about_dialog);
}

static void
help_cb (BonoboUIComponent *uic,
          gpointer           user_data,
          const gchar       *verbname)
{
	GError *error = NULL;
	gnome_help_display("gnome-dictionary",NULL,&error);

	if (error)
	{
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_MODAL,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_OK,
						 ("There was an error displaying help: \n%s"),
						 error->message);

		g_signal_connect (G_OBJECT (dialog), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_widget_show (dialog);
		g_error_free (error);
	}
}

static void
prefs_cb (BonoboUIComponent *uic,
          gpointer           user_data,
          const gchar       *verbname)
{
	if (!context)
		gdict_init_context ();

	gdict_app_show_preferences ();
}

static const BonoboUIVerb gdict_applet_menu_verbs [] = {
	BONOBO_UI_VERB ("sel", lookup_selected_text),
        BONOBO_UI_VERB ("prefs", prefs_cb),
        BONOBO_UI_VERB ("help", help_cb),
        BONOBO_UI_VERB ("about", about_cb),
       
        BONOBO_UI_VERB_END
};

static gboolean
gdict_applet_new (PanelApplet *parent_applet)
{
	GDictApplet *applet;
	BonoboUIComponent *component;
	GtkIconInfo *icon_info;
	GtkTooltips *tooltips;

 	applet = g_new0 (GDictApplet, 1);
 	
	applet->applet_widget = GTK_WIDGET(parent_applet);
	applet->panel_size = panel_applet_get_size (PANEL_APPLET (applet->applet_widget));
 	applet->orient = panel_applet_get_orient (PANEL_APPLET (applet->applet_widget));
	
	icon_info = gtk_icon_theme_lookup_icon (gtk_icon_theme_get_default (), "gdict", 48, 0);
	if (icon_info) {
		gnome_window_icon_set_default_from_file (gtk_icon_info_get_filename (icon_info));
		gtk_icon_info_free (icon_info);
	}
 	
	panel_applet_set_flags (parent_applet, PANEL_APPLET_EXPAND_MINOR);

	applet->tooltip_widget = GTK_WIDGET(gtk_tooltips_new ());
	g_object_ref (applet->tooltip_widget);
	gtk_object_sink (GTK_OBJECT (applet->tooltip_widget));
	g_object_set_data_full (G_OBJECT (applet->applet_widget), "tooltips",
				applet->tooltip_widget, (GDestroyNotify) g_object_unref);

	setup_applet_widgets (applet);

 	gdict_pref_load ();
 	
 	gdict_app_create (TRUE);
 	
 	panel_applet_setup_menu_from_file (PANEL_APPLET (applet->applet_widget),
					   NULL,
                            		   "GNOME_GDictApplet.xml",
					   NULL,
                            		   gdict_applet_menu_verbs,
                            		   applet);                               

	if (panel_applet_get_locked_down (PANEL_APPLET (applet->applet_widget))) {
		BonoboUIComponent *popup_component;

		popup_component = panel_applet_get_popup_component (PANEL_APPLET (applet->applet_widget));

		bonobo_ui_component_set_prop (popup_component,
					      "/commands/prefs",
					      "hidden", "1",
					      NULL);
	}

 	/* server will be contacted when an action is performed */
 	context = NULL;
	
	component = panel_applet_get_popup_component ((PanelApplet *)applet->applet_widget);
	g_signal_connect (component, "ui-event", (GCallback) menu_event_cb, NULL);
	
 	g_signal_connect (G_OBJECT (applet->applet_widget), "change_size",
 			  G_CALLBACK (change_size_cb), applet);
 	g_signal_connect (G_OBJECT (applet->applet_widget), "change_orient",
 			  G_CALLBACK (change_orient_cb), applet);
 	g_signal_connect (G_OBJECT (applet->applet_widget), "button_press_event",                         
			  G_CALLBACK (send_button_to_entry_event), applet); 
 	g_signal_connect (G_OBJECT (applet->applet_widget), "button_press_event",                         
			  G_CALLBACK (set_show_dictionary_menu_state), NULL);
	g_signal_connect (G_OBJECT (applet->applet_widget), "destroy",
			  G_CALLBACK (destroy_applet), applet);

	return TRUE;
}

static gboolean
gdict_applet_factory (PanelApplet *applet,
                     const gchar          *iid,
                     gpointer              data)
{
        gboolean retval = FALSE;

        if (!strcmp (iid, "OAFIID:GNOME_GDictApplet"))
                retval = gdict_applet_new (applet);

        return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_GDictApplet_Factory",
			     PANEL_TYPE_APPLET,
                             "gnome-dictionary",
                             "0",
                             gdict_applet_factory,
                             NULL)
 	
