/* genui.c
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
#include "genui.h"
#include "magconf.h"
#include "srintl.h"
#include "gnopiui.h"
#include "SRMessages.h"
#include <signal.h>
#include <glade/glade.h>

extern General 	 *general_setting;
static gboolean	 general_changed = FALSE;
/**
 * CheckButtons
**/
GtkWidget	*ck_magnifier,
		*ck_braille,
		*ck_speech,
		*ck_braille_monitor,
		*ck_minimize;
extern gboolean sensitive_list [];
/**
 * General Settings Window
**/
GtkWidget	*w_general_settings = NULL;


/**
 *
 * Set the setting changes in struture and sent to SRCore.
 * 
**/
void
genui_set_sensitive_for (gint no, 
			 gboolean val)
{
    if (!w_general_settings) 
	return;
    
    switch (no)
    {
	case BRAILLE:
	    
	    if (val)	
		gtk_button_set_label ( GTK_BUTTON (ck_braille), 
				    _("_Braille"));
	    else	
		gtk_button_set_label ( GTK_BUTTON (ck_braille), 
				    _("_Braille (unavailable)"));
	    break;
	case MAGNIFIER:		
	    if (val)	
		gtk_button_set_label ( GTK_BUTTON (ck_magnifier), 
				    _("_Magnifier"));
	    else	
		gtk_button_set_label ( GTK_BUTTON (ck_magnifier), 
				    _("_Magnifier (unavailable)"));
	    break;
	case SPEECH:
	    if (val)	
		gtk_button_set_label ( GTK_BUTTON (ck_speech), 
				    _("_Speech"));
	    else	
		gtk_button_set_label ( GTK_BUTTON (ck_speech), 
				    _("_Speech (unavailable)"));
	    break;	
	case BRAILLE_MONITOR:	
	    if (val)	
		gtk_button_set_label ( GTK_BUTTON (ck_braille_monitor), 
				    _("B_raille monitor"));
	    else	
		gtk_button_set_label ( GTK_BUTTON (ck_braille_monitor), 
				    _("B_raille monitor (unavailable)"));
	    break;
	default:
	    break;
    }
}

static void
genui_apply_sensitivity (gboolean state)
{
    general_changed = state;
}

void 
genui_changeing (void)
{
    gboolean check_val;
    
    sru_return_if_fail (general_setting);
    
    check_val = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_magnifier));
    if (general_setting->magnifier != check_val)
    {
	general_setting->magnifier = check_val;
	srcore_magnifier_status_set (general_setting->magnifier);
    }
	    
    check_val = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_braille));
    if (general_setting->braille != check_val)
    {
	general_setting->braille = check_val;
	srcore_braille_status_set (general_setting->braille);
    }
    
    check_val = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_braille_monitor));
    if (general_setting->braille_monitor != check_val)
    {
	general_setting->braille_monitor = check_val;
	srcore_braille_monitor_status_set (general_setting->braille_monitor);
    }
    
    check_val = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_speech));
    if (general_setting->speech != check_val)
    {
	general_setting->speech = check_val;
	srcore_speech_status_set (general_setting->speech);
    }
				    
    check_val = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_minimize));
    if (general_setting->minimize != check_val)
    {
	general_setting->minimize = check_val;
	srcore_minimize_set (general_setting->minimize);
    }
}

static void
genui_state_changes (GtkWidget *widget,
		    gpointer	data)
{
    genui_apply_sensitivity (TRUE);
}


static void
genui_response_event (GtkDialog *dialog,
		      gint       response_id,
		      gpointer   user_data)
{
    switch (response_id)
    {
     case GTK_RESPONSE_OK: 
        {
	    if (general_changed)
    		genui_changeing ();
	    gtk_widget_hide ((GtkWidget*)dialog);
	}
        break;
     case GTK_RESPONSE_APPLY:
        {
	    genui_apply_sensitivity (FALSE);
	    genui_changeing ();
	}
        break;
    case GTK_RESPONSE_HELP:
	    gn_load_help ("gnopernicus-started");
	break;
     default:
	    gtk_widget_hide ((GtkWidget*)dialog);
        break;
    }
}


static gint
genui_delete_emit_response_cancel (GtkDialog *dialog,
				   GdkEventAny *event,
				   gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}

/**
 *
 * Set event handlers and get a widgets used in this interface.
 * xml - glade interface XML pointer
 *
**/
void 
genui_set_handlers (GladeXML 	*xml,
		    General 	*general_setting)
{
    w_general_settings 	= glade_xml_get_widget (xml, "w_general_settings");
    ck_magnifier 	= glade_xml_get_widget (xml, "ck_magnifier");
    ck_braille 		= glade_xml_get_widget (xml, "ck_braille");
    ck_braille_monitor	= glade_xml_get_widget (xml, "ck_braille_monitor");
    ck_speech 		= glade_xml_get_widget (xml, "ck_speech");
    ck_minimize		= glade_xml_get_widget (xml, "ck_minimize");

    g_signal_connect (w_general_settings, "response",
		      G_CALLBACK (genui_response_event), NULL);
    g_signal_connect (w_general_settings, "delete_event",
                      G_CALLBACK (genui_delete_emit_response_cancel), NULL);
    glade_xml_signal_connect (xml, "on_ck_braille_toggled",		
			    GTK_SIGNAL_FUNC (genui_state_changes));
    glade_xml_signal_connect (xml, "on_ck_braille_monitor_toggled",		
			    GTK_SIGNAL_FUNC (genui_state_changes));
    glade_xml_signal_connect (xml, "on_ck_speech_toggled",		
			    GTK_SIGNAL_FUNC (genui_state_changes));
    glade_xml_signal_connect (xml, "on_ck_magnifier_toggled",		
			    GTK_SIGNAL_FUNC (genui_state_changes));
    glade_xml_signal_connect (xml, "on_ck_minimize_toggled",		
			    GTK_SIGNAL_FUNC (genui_state_changes));

}

/**
 *
 * Generals Settings interface loader function
 *
**/
gboolean 
genui_load_general_settings_interface (GtkWidget *parent_window)
{
    if (!w_general_settings)
    {
	GladeXML *xml;
	xml = gn_load_interface ("General_Settings/general_settings.glade2", "w_general_settings");
	sru_return_val_if_fail (xml, FALSE);
	genui_set_handlers (xml, general_setting);	    
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_general_settings),
				      GTK_WINDOW (parent_window));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_general_settings), TRUE);
    }
    else
	gtk_widget_show (w_general_settings);
        
    genui_value_add_to_widgets (general_setting);    
    
    genui_apply_sensitivity (FALSE);
        
    return TRUE;
}

/**
 *
 * Set the widgets with a current value.
 *
**/
gboolean
genui_value_add_to_widgets (General *general_setting)
{
    if (!w_general_settings) 
	return FALSE;
    
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_magnifier), 
				    general_setting->magnifier);
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_braille),   
				    general_setting->braille);
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_braille_monitor),   
				    general_setting->braille_monitor);
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_speech),    
				    general_setting->speech);
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_minimize),  
				    general_setting->minimize);
    
    genui_set_sensitive_for ( BRAILLE, 	
			    sensitive_list [BRAILLE]);
    genui_set_sensitive_for ( SPEECH, 		
			    sensitive_list [SPEECH]);
    genui_set_sensitive_for ( MAGNIFIER, 	
			    sensitive_list [MAGNIFIER]);
    genui_set_sensitive_for ( BRAILLE_MONITOR, 
			    sensitive_list [BRAILLE_MONITOR]);
    	
    return TRUE;
}

