/* spui.c
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
#include "SRMessages.h"
#include <glade/glade.h>
#include "srintl.h"
#include "defui.h"
#include "spui.h"
#include "spvoiceui.h"
#include "spdictui.h"
#include "gnopiui.h"

typedef enum
{
    PUNCTUATION_IGNORE,
    PUNCTUATION_SOME,
    PUNCTUATION_MOST,
    PUNCTUATION_ALL,
    PUNCTUATION_NUMBER
}PunctuationType;

typedef enum
{
    TEXT_ECHO_CHARACTER,
    TEXT_ECHO_WORD,
    TEXT_ECHO_NONE,
    TEXT_ECHO_NUMBER
}TextEchoType;

struct {
    GtkWidget	*w_speech_settings;
    GtkWidget	*ck_count;
    GtkWidget	*ck_spaces;
    GtkWidget	*ck_cursors;
    GtkWidget	*ck_modifiers;
    GtkWidget	*ck_dictionary;
    GtkWidget	*bt_dictionary;
    GtkWidget	*rb_punctuation [PUNCTUATION_NUMBER];
    GtkWidget	*rb_text_echo [TEXT_ECHO_NUMBER];
} spui_speech_settings;

extern Speech 				*speech_setting;
static gboolean				speech_changed;

gchar *punct_keys[]=
{
    "IGNORE",
    "SOME",
    "MOST",
    "ALL"
};

gchar *common_keys[]=
{
    "NONE",
    "ALL"
};

gchar *dictionary_keys[]=
{
    "NO",
    "YES"
};

gchar *text_echo_keys[]=
{
    "CHARACTER",
    "WORD",
    "NONE"
};

static gint
spui_delete_emit_response_cancel (GtkDialog *dialog,
				  GdkEventAny *event,
				  gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}


static void
spui_apply_sensitivity (gboolean state)
{
    speech_changed = state;
}

static void 
spui_speech_setting_changing (void)
{
    gint iter;
    
    if (!speech_setting) 
	return;
	
    for (iter = PUNCTUATION_IGNORE ; iter < PUNCTUATION_NUMBER ; iter++)
    {
	if (gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(spui_speech_settings.rb_punctuation[iter])) && 
	    strcmp (speech_setting->punctuation_type, punct_keys[iter]))
	{
	    g_free(speech_setting->punctuation_type);
	    speech_setting->punctuation_type = g_strdup (punct_keys[iter]);
	    spconf_punctuation_set (speech_setting->punctuation_type);
	    break;
	}
    }

    for (iter = TEXT_ECHO_CHARACTER ; iter < TEXT_ECHO_NUMBER ; iter++)
    {
	if (gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(spui_speech_settings.rb_text_echo[iter])) && 
	    strcmp (speech_setting->text_echo_type, text_echo_keys[iter]))
	{
	    g_free(speech_setting->text_echo_type);
	    speech_setting->text_echo_type = g_strdup (text_echo_keys[iter]);
	    spconf_text_echo_set (speech_setting->text_echo_type);
	    break;
	}
    }

    g_free (speech_setting->modifiers_type);
    g_free (speech_setting->cursors_type);
    g_free (speech_setting->spaces_type);
    g_free (speech_setting->count_type);
    g_free (speech_setting->dictionary);
    
    speech_setting->modifiers_type = 
	g_strdup (common_keys [gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (spui_speech_settings.ck_modifiers))]);
    spconf_modifiers_set (speech_setting->modifiers_type);		  

    speech_setting->cursors_type = 
	g_strdup (common_keys [gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (spui_speech_settings.ck_cursors))]);
    spconf_cursors_set (speech_setting->cursors_type);		  

    speech_setting->spaces_type = 
	g_strdup (common_keys [gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (spui_speech_settings.ck_spaces))]);
    spconf_spaces_set (speech_setting->spaces_type);		  

    speech_setting->dictionary = 
	g_strdup (dictionary_keys [gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (spui_speech_settings.ck_dictionary))]);
    spconf_dictionary_set (speech_setting->dictionary);		  

    speech_setting->count_type = 
	g_strdup (common_keys [gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (spui_speech_settings.ck_count))]);
    spconf_count_set (speech_setting->count_type);		  
}

static void
spui_voices_settings (GtkWidget *widget,
		      gpointer	user_data)
{
    spui_voice_settings_load ((GtkWidget*)spui_speech_settings.w_speech_settings);    
}

static void
spui_dictionary (GtkWidget *widget,
		gpointer   user_data)
{
    spui_dictionary_load ((GtkWidget*)spui_speech_settings.w_speech_settings);    
}

static void
spui_state_changed (GtkWidget	*widget,
		    gpointer	data)
{
    spui_apply_sensitivity (TRUE);
}

static void
spui_dictionary_toggled (GtkWidget *widget,
			gpointer    data)
{
    spui_apply_sensitivity (TRUE);
    gtk_widget_set_sensitive (spui_speech_settings.bt_dictionary,
    				gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (widget)));
}

static void
spui_settings_response (GtkDialog *dialog,
			gint       response_id,
			gpointer   user_data)
{
    switch (response_id)
    {
        case GTK_RESPONSE_OK: 
	    if (speech_changed)
    		spui_speech_setting_changing ();
	    gtk_widget_hide ((GtkWidget*)dialog);
        break;
        case GTK_RESPONSE_APPLY:
    	    spui_speech_setting_changing ();
	    spui_apply_sensitivity (FALSE);
        break;
        case GTK_RESPONSE_HELP:
	    gn_load_help ("gnopernicus-speech-prefs");
	break;
	default:
	    gtk_widget_hide ((GtkWidget*)dialog);
        break;
    }
}

/**
 *
 * Set event handlers and get a widgets used in this interface.
 * xml - glade interface XML pointer
 *
**/
static void 
spui_set_handlers (GladeXML *xml)
{    
    spui_speech_settings.w_speech_settings 	= glade_xml_get_widget (xml, "w_speech_settings");

    spui_speech_settings.rb_punctuation [PUNCTUATION_IGNORE]	= glade_xml_get_widget (xml, "rb_punct_ignore"); 
    spui_speech_settings.rb_punctuation [PUNCTUATION_SOME]	= glade_xml_get_widget (xml, "rb_punct_some"); 
    spui_speech_settings.rb_punctuation [PUNCTUATION_MOST]	= glade_xml_get_widget (xml, "rb_punct_most"); 
    spui_speech_settings.rb_punctuation [PUNCTUATION_ALL]	= glade_xml_get_widget (xml, "rb_punct_all"); 

    spui_speech_settings.rb_text_echo [TEXT_ECHO_CHARACTER]	= glade_xml_get_widget (xml, "rb_techo_character"); 
    spui_speech_settings.rb_text_echo [TEXT_ECHO_WORD]	= glade_xml_get_widget (xml, "rb_techo_word"); 
    spui_speech_settings.rb_text_echo [TEXT_ECHO_NONE]	= glade_xml_get_widget (xml, "rb_techo_none"); 

    spui_speech_settings.ck_modifiers 	= glade_xml_get_widget (xml, "ck_modifiers"); 
    spui_speech_settings.ck_cursors 		= glade_xml_get_widget (xml, "ck_cursors"); 
    spui_speech_settings.ck_spaces 		= glade_xml_get_widget (xml, "ck_spaces"); 
    spui_speech_settings.ck_count 		= glade_xml_get_widget (xml, "ck_count"); 
    spui_speech_settings.ck_dictionary	= glade_xml_get_widget (xml, "ck_dictionary"); 
    spui_speech_settings.bt_dictionary	= glade_xml_get_widget (xml, "bt_dictionary"); 

    g_signal_connect (spui_speech_settings.w_speech_settings, "response",
		      G_CALLBACK (spui_settings_response), NULL);
    g_signal_connect (spui_speech_settings.w_speech_settings, "delete_event",
                      G_CALLBACK (spui_delete_emit_response_cancel), NULL);

    
    glade_xml_signal_connect_data (xml,"on_ck_count_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed), 
			    (gpointer)FALSE);

    glade_xml_signal_connect_data (xml,"on_bt_voices_clicked",			
			    GTK_SIGNAL_FUNC (spui_voices_settings),
			    (gpointer)spui_speech_settings.w_speech_settings);

    glade_xml_signal_connect_data (xml,"on_bt_dictionary_clicked",			
			    GTK_SIGNAL_FUNC (spui_dictionary),
			    (gpointer)spui_speech_settings.w_speech_settings);
			    
    glade_xml_signal_connect (xml,"on_rb_punct_ignore_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));
    glade_xml_signal_connect (xml,"on_rb_punct_some_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));
    glade_xml_signal_connect (xml,"on_rb_punct_most_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));
    glade_xml_signal_connect (xml,"on_rb_punct_all_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));

    glade_xml_signal_connect (xml,"on_rb_techo_character_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));
    glade_xml_signal_connect (xml,"on_rb_techo_word_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));
    glade_xml_signal_connect (xml,"on_rb_techo_none_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));			    
    glade_xml_signal_connect (xml,"on_ck_modifiers_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));
    glade_xml_signal_connect (xml,"on_ck_dictionary_toggled",			
			    GTK_SIGNAL_FUNC (spui_dictionary_toggled));
    glade_xml_signal_connect (xml,"on_ck_cursors_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));
    glade_xml_signal_connect (xml,"on_ck_spaces_toggled",			
			    GTK_SIGNAL_FUNC (spui_state_changed));

}

/**
 *
 * Speech option user interface loader function
 *
**/
gboolean 
spui_load_speech_settings (GtkWidget *parent_window)
{
    if (!spui_speech_settings.w_speech_settings)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Speech_Settings/speech_settings.glade2", "w_speech_settings");
	sru_return_val_if_fail (xml, FALSE);
	spui_set_handlers  (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for ( GTK_WINDOW (spui_speech_settings.w_speech_settings),
				           GTK_WINDOW (parent_window));
				    
	gtk_window_set_destroy_with_parent ( GTK_WINDOW (spui_speech_settings.w_speech_settings), 
					         TRUE);
    }
    else
	gtk_widget_show (spui_speech_settings.w_speech_settings);
    
    spui_speech_setting_value_add_to_widgets (speech_setting);    
    
    spui_apply_sensitivity (FALSE);
                
    return TRUE;
}

/**
 *
 * Set the widgets with a current value.
 *
**/
gboolean
spui_speech_setting_value_add_to_widgets (Speech *speech_setting)
{    
    gint iter;
    if (!spui_speech_settings.w_speech_settings ||
	!speech_setting) 
	return FALSE;
            
    for (iter = PUNCTUATION_IGNORE ; iter < PUNCTUATION_NUMBER ; iter++)
    {
	if (!strcmp (punct_keys[iter], speech_setting->punctuation_type))
	{
	    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON 
					    (spui_speech_settings.rb_punctuation [iter]), 
					    TRUE);
	    break;
	}
    }

    for (iter = TEXT_ECHO_CHARACTER ; iter < TEXT_ECHO_NUMBER ; iter++)
    {
	if (!strcmp (text_echo_keys[iter], speech_setting->text_echo_type))
	{
	    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON 
					    (spui_speech_settings.rb_text_echo [iter]), 
					    TRUE);
	    break;
	}
    }

    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (spui_speech_settings.ck_count), 
				    !strcmp ("ALL", speech_setting->count_type)
				  );
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (spui_speech_settings.ck_modifiers), 
				    !strcmp ("ALL", speech_setting->modifiers_type)
				  );
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (spui_speech_settings.ck_cursors), 
				    !strcmp ("ALL", speech_setting->cursors_type)
				  );
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (spui_speech_settings.ck_spaces), 
				    !strcmp ("ALL", speech_setting->spaces_type)
				  );

    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (spui_speech_settings.ck_dictionary), 
				    !strcmp ("YES", speech_setting->dictionary)
				  );

    gtk_widget_set_sensitive (spui_speech_settings.bt_dictionary,
    			    gtk_toggle_button_get_active ( 
			    GTK_TOGGLE_BUTTON (spui_speech_settings.ck_dictionary)));

    return TRUE;
}
