/* kbui.c
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
#include "kbui.h"
#include "SRMessages.h"
#include <glade/glade.h>
#include "srintl.h"
#include "gnopiui.h"


extern Keyboard 	*keyboard_setting;
GtkWidget		*w_keyboard_settings;
static GtkWidget	*ck_move_mouse;
static GtkWidget	*ck_simulate_click;



static void
kbui_keyboard_setting_changeing (void)
{
    gboolean active;
    
    sru_return_if_fail (keyboard_setting);
    
    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_move_mouse));
    if (active != keyboard_setting->take_mouse)
    {
        keyboard_setting->take_mouse = active;
        kbconf_take_mouse_set (keyboard_setting->take_mouse);
    }
    
    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_simulate_click));
    if (active != keyboard_setting->simulate_click)
    {
         keyboard_setting->simulate_click = active;
         kbconf_simulate_click_set (keyboard_setting->simulate_click);
    }
}

void
kbui_settings_response (GtkDialog *dialog,
			gint       response_id,
			gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_OK) 
    {
	kbui_keyboard_setting_changeing ();
	gtk_widget_hide ((GtkWidget*)dialog);
    }
    else
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-mouse-prefs");
    }
    else	
	gtk_widget_hide ((GtkWidget*)dialog);
}


static gint
kbui_delete_emit_response_cancel (GtkDialog *dialog,
				   GdkEventAny *event,
				   gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}



void
kbui_move_mouse_toggled (GtkToggleButton *button,
                         gpointer         user_data)
{
    if (ck_move_mouse == GTK_WIDGET (button) && 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_move_mouse))== TRUE)
    	    gtk_widget_set_sensitive (ck_simulate_click, TRUE);
	else
	    gtk_widget_set_sensitive (ck_simulate_click, FALSE);
}

void
kbui_set_handlers_keyboard_settings (GladeXML *xml)
{

    w_keyboard_settings = glade_xml_get_widget (xml, "w_mouse_settings");
    ck_move_mouse	= glade_xml_get_widget (xml, "ck_move_mouse");
    ck_simulate_click	= glade_xml_get_widget (xml, "ck_simulate_click");

    g_signal_connect (w_keyboard_settings, "response",
		      G_CALLBACK (kbui_settings_response), NULL);
    g_signal_connect (w_keyboard_settings, "delete_event",
                      G_CALLBACK (kbui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect (xml, "on_ck_move_mouse_toggled",			
			    GTK_SIGNAL_FUNC (kbui_move_mouse_toggled));
}

/**
 * Keyboard and Mouse APIs
**/
gboolean
kbui_load_keyboard_settings (GtkWidget *parent_window)
{
    if (!w_keyboard_settings)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Keyboard_Mouse_Settings/keyboard_mouse_settings.glade2", "w_mouse_settings");	
	sru_return_val_if_fail (xml, FALSE);
	kbui_set_handlers_keyboard_settings (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_keyboard_settings),
				      GTK_WINDOW (parent_window));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_keyboard_settings), TRUE);
    }
    else
	gtk_widget_show (w_keyboard_settings);
    
    kbui_keyboard_settings_value_add_to_widgets (keyboard_setting);
            
    return TRUE;
}



gboolean 
kbui_keyboard_settings_value_add_to_widgets (Keyboard *keyboard_setting)
{
    sru_return_val_if_fail (keyboard_setting, FALSE);
    if (!w_keyboard_settings) 
	return FALSE;
        
    gtk_widget_set_sensitive (ck_simulate_click, 
			    keyboard_setting->take_mouse);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_move_mouse), 	 
				keyboard_setting->take_mouse);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_simulate_click), 
				keyboard_setting->simulate_click);

    return TRUE;
}
