/* spvoiceui.c
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
#include "spvoiceui.h"
#include "SRMessages.h"
#include "libsrconf.h"
#include <glade/glade.h>
#include "srintl.h"
#include "gnopiui.h"

#define INVALID_VOICE 		"test"

#define SPEECH_ALL_DRIVERS	_("<all drivers>")
#define SPEECH_ALL_VOICES	_("<all voices>")


#define CHECK_RANGE_OF_VALUE(val,min,max)\
	(min > val ? min : (val > max ? max : val))

enum 
{
    VOICE_GNOPERNICUS_SPEAKER,
    VOICE_DRIVER_NAME,
    VOICE_DRIVER_VOICE,
    VOICE_VOLUME,
    VOICE_RATE,
    VOICE_PITCH,
    
    VOICE_NO_COLUMN
};

struct {
    GtkWidget	*w_voice_setting;
    GtkWidget	*tv_voice_table;
    GtkWidget	*bt_voice_remove;
    GtkWidget	*bt_voice_modify;
} spui_speech_voice;

struct {
    GtkWidget	*w_voice_setting_add_modify;
    GtkWidget	*cb_engine_voice;
    GtkWidget	*cb_engine_driver;
    GtkWidget	*et_gnopernicus_voice;
    GtkWidget	*sp_voice_volume;
    GtkWidget	*sp_voice_rate;
    GtkWidget	*sp_voice_pitch;
    GtkWidget	*hs_voice_volume;
    GtkWidget	*hs_voice_rate;
    GtkWidget	*hs_voice_pitch;
} spui_speech_voice_add_modify;

struct {
    GtkWidget	*w_voice_setting_relative_values;
    GtkWidget	*cb_rel_engine_driver;
    GtkWidget   *cb_rel_engine_voice;
} spui_speech_voice_relative;

struct {
    GtkWidget	*w_voice_setting_absolute_values;
    GtkWidget	*cb_abs_engine_driver;
    GtkWidget   *cb_abs_engine_voice;
    GtkWidget	*sp_abs_volume;
    GtkWidget	*sp_abs_rate;
    GtkWidget	*sp_abs_pitch;
    GtkWidget	*hs_abs_volume;
    GtkWidget	*hs_abs_rate;
    GtkWidget	*hs_abs_pitch;
    GtkWidget	*ck_abs_volume;
    GtkWidget	*ck_abs_rate;
    GtkWidget	*ck_abs_pitch;
    gboolean	abs_volume_sens;
    gboolean	abs_rate_sens;
    gboolean	abs_pitch_sens;
} spui_speech_voice_absolute;

static GtkTreeIter			sp_curr_iter;		
static gboolean				sp_new_item;

static SpeakerSettings			sp_old_speaker_settings;
static gboolean				sp_changes_activable;
static gboolean				sp_abs_changes_activable;

static GnopernicusSpeakerListType 	*sp_gnopernicus_speakers 	= NULL;
static GnopernicusSpeakerListType 	*sp_gnopernicus_speakers_backup = NULL;
static GnopernicusSpeakerListType	*sp_curr_gnopernicus_speaker	= NULL;
static SpeakerSettingsListType  	*sp_speakers_settings		= NULL;
static SpeakerSettingsListType  	*sp_speakers_settings_backup	= NULL;
static SpeakerSettingsListType		*sp_curr_speaker_settings	= NULL;
static SpeechDriverListType		*sp_speech_driver_list		= NULL;

#if 0
static void
spui_debug (GSList *list)
{
    GSList *elem = list;
    fprintf (stderr,"\n\n-----------------------");
    while (elem)
    {
	SpeakerSettings *ss = NULL;
	ss = (SpeakerSettings*)elem->data;
	if (ss)
	{
	    if (ss->gnopernicus_speaker)
		fprintf (stderr,"\ngn_speker:%s", ss->gnopernicus_speaker);
	    if (ss->voice_name)
		fprintf (stderr,"\nvoice_name:%s", ss->voice_name);
	    if (ss->driver_name)
		fprintf (stderr,"\ndriver_name:%s", ss->driver_name);
	    fprintf (stderr,"\nvolume:%d", ss->volume);
	    fprintf (stderr,"\nrate:%d", ss->rate);
	    fprintf (stderr,"\npitch:%d", ss->pitch);
	}
	elem = elem->next;
    }
    
}
#endif

static void
spui_voice_old_speaker_setting_clean (void)
{
    sp_old_speaker_settings.volume = 0;
    sp_old_speaker_settings.rate   = 0;
    sp_old_speaker_settings.pitch  = 0;
    
    g_free (sp_old_speaker_settings.driver_name);
    g_free (sp_old_speaker_settings.driver_name_descr);
    g_free (sp_old_speaker_settings.voice_name);
    g_free (sp_old_speaker_settings.voice_name_descr);
    g_free (sp_old_speaker_settings.gnopernicus_speaker);
    
    sp_old_speaker_settings.driver_name 	= NULL;
    sp_old_speaker_settings.driver_name_descr 	= NULL;
    sp_old_speaker_settings.voice_name 		= NULL;
    sp_old_speaker_settings.voice_name_descr 	= NULL;
    sp_old_speaker_settings.gnopernicus_speaker = NULL;
}

static void
spui_voice_speaker_parameter_in_list_store_at_iter_set (SpeakerSettings *item, 
					    		GtkListStore *store,
					    		GtkTreeIter  iter)
{
    sru_return_if_fail (store);
    sru_return_if_fail (item);

    gtk_list_store_set (GTK_LIST_STORE (store), &iter, 
			VOICE_GNOPERNICUS_SPEAKER,	(gchar*)item->gnopernicus_speaker,
			VOICE_DRIVER_NAME,		(gchar*)item->driver_name_descr,
			VOICE_DRIVER_VOICE,  		(gchar*)item->voice_name_descr,
			VOICE_VOLUME,			item->volume,
			VOICE_RATE,			item->rate,
			VOICE_PITCH,			item->pitch,
		    	-1);
}

static gboolean
spui_voice_iter_for_speaker_get (const gchar *key, 
				 GtkTreeIter *rv_iter)
{
    GtkTreeModel     *model;
    GtkTreeIter	     iter;
    gboolean	     valid;
    
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));

    if (!GTK_IS_LIST_STORE (model))
	return FALSE;
    
    valid = gtk_tree_model_get_iter_first (model, &iter);

    while (valid)
    {
	gchar *ikey = NULL;
	gtk_tree_model_get (model, 			&iter,
			    VOICE_GNOPERNICUS_SPEAKER,  &ikey,
                    	    -1);
	if (!strcmp (ikey, key))
	{
	    g_free (ikey);
	    *rv_iter = iter;
	    return TRUE;
	}
	g_free (ikey);
	
	valid = gtk_tree_model_iter_next (model, &iter);
    }    
    
    return FALSE;
}




/************************************************************************/
static SpeakerSettings*
spui_voice_add_modify_new_speaker_settings_get (void)
{
    SpeakerSettings *new_settings = NULL;
    
    if (!spui_speech_voice_add_modify.w_voice_setting_add_modify)
	return NULL;
    	    
    new_settings = spconf_speaker_settings_new ();
    new_settings->volume 		= CHECK_RANGE_OF_VALUE (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_volume)), MIN_SPEECH_VOLUME, MAX_SPEECH_VOLUME);
    new_settings->rate   		= CHECK_RANGE_OF_VALUE (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_rate)), MIN_SPEECH_RATE, MAX_SPEECH_RATE);
    new_settings->pitch  		= CHECK_RANGE_OF_VALUE (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_pitch)), MIN_SPEECH_PITCH, MAX_SPEECH_PITCH);
    new_settings->voice_name 		= g_strdup (gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_add_modify.cb_engine_voice)->entry)));
    new_settings->voice_name_descr	= spconf_driver_list_voice_descr_name_get (new_settings->voice_name);
    new_settings->driver_name_descr	= g_strdup (gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_add_modify.cb_engine_driver)->entry)));
    new_settings->driver_name 		= g_strdup (spconf_driver_list_get_driver_name (sp_speech_driver_list,
											new_settings->driver_name_descr));
    new_settings->gnopernicus_speaker 	= g_strdup (gtk_entry_get_text (GTK_ENTRY (spui_speech_voice_add_modify.et_gnopernicus_voice)));    
    
    return new_settings;
}

static gboolean
spui_voice_add_modify_properties_get (void)
{
    SpeakerSettingsListType    *ss_found = NULL;
    GnopernicusSpeakerListType *gs_found = NULL;
    SpeakerSettings *new_setting = NULL;
    GtkTreeModel    *model = NULL;

    if (!spui_speech_voice_add_modify.w_voice_setting_add_modify ||
	!sp_curr_speaker_settings    ||
	!sp_curr_gnopernicus_speaker) 
	return FALSE;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));    
        
    new_setting = spui_voice_add_modify_new_speaker_settings_get ();

    ss_found = spconf_speaker_settings_list_find (sp_speakers_settings, new_setting->gnopernicus_speaker);
    gs_found = spconf_gnopernicus_speakers_find (sp_gnopernicus_speakers, new_setting->gnopernicus_speaker);
    
    if (!ss_found && !gs_found &&
	strcmp (NONE_ELEMENT,  new_setting->gnopernicus_speaker) && 
	strcmp (INVALID_VOICE, new_setting->gnopernicus_speaker)) 
    {
	sp_curr_speaker_settings->data = 
		spconf_speaker_settings_copy (sp_curr_speaker_settings->data, 
					      new_setting);
	g_free (sp_curr_gnopernicus_speaker->data);
	sp_curr_gnopernicus_speaker->data 	= g_strdup (new_setting->gnopernicus_speaker);
    }
    else
    {
	sp_speakers_settings =
	    spconf_speaker_settings_list_remove (sp_speakers_settings,
						     ((SpeakerSettings*)(sp_curr_speaker_settings->data))->gnopernicus_speaker);
	sp_gnopernicus_speakers =
	    spconf_gnopernicus_speakers_remove (sp_gnopernicus_speakers,
						    sp_curr_gnopernicus_speaker->data);
	gtk_list_store_remove (GTK_LIST_STORE (model), &sp_curr_iter);

	gn_show_message (_("Invalid voice!"));
	    
	return FALSE;
    }
    
    spui_voice_speaker_parameter_in_list_store_at_iter_set (new_setting, 
						    	    GTK_LIST_STORE (model),
					    	    	    sp_curr_iter);

    spconf_speaker_settings_free (new_setting);

    return TRUE;
}

static void
spui_voice_add_modify_restore_state (void)
{
    GtkTreeModel     *model;
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));
    
    if (sp_new_item)
    {
	gtk_list_store_remove (GTK_LIST_STORE (model), &sp_curr_iter);
	
	if (sp_curr_speaker_settings && 
	    sp_curr_speaker_settings->data)
	{
	    sp_speakers_settings =
		spconf_speaker_settings_list_remove (sp_speakers_settings,
		((SpeakerSettings*)sp_curr_speaker_settings->data)->gnopernicus_speaker);
	}
    }
    else
    {
	spui_voice_speaker_parameter_in_list_store_at_iter_set (&sp_old_speaker_settings, 
								GTK_LIST_STORE (model),
					    			sp_curr_iter);
	sp_curr_speaker_settings->data =
		spconf_speaker_settings_copy (sp_curr_speaker_settings->data, 
			    		      &sp_old_speaker_settings);

	spconf_speaker_settings_save ((SpeakerSettings*)sp_curr_speaker_settings->data);
    }
}

static void
spui_voice_add_modify_combo_list_set (GList *list, GtkWidget *combo)
{
    if (!list) 
	return;
	
    gtk_combo_set_popdown_strings (GTK_COMBO (combo), list);
}

static gboolean
spui_voice_add_modify_speaker_values_modify (SpeakerSettings *settings)
{
    GList *tmp = NULL;

    if (!spui_speech_voice_add_modify.w_voice_setting_add_modify) 
	return FALSE;
	
    if (strcmp (((SpeakerSettings*)sp_curr_speaker_settings->data)->gnopernicus_speaker, 
		settings->gnopernicus_speaker))
	return FALSE;
	
    tmp = spconf_driver_list_for_combo_get (sp_speech_driver_list, NULL);
    spui_voice_add_modify_combo_list_set (tmp, spui_speech_voice_add_modify.cb_engine_driver);
    spconf_list_for_combo_free (tmp);
    
    tmp = spconf_voice_list_for_combo_get (sp_speech_driver_list, settings->driver_name_descr, NULL);
    spui_voice_add_modify_combo_list_set (tmp, spui_speech_voice_add_modify.cb_engine_voice);
    spconf_list_for_combo_free (tmp);

    
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_volume), 	(gdouble)settings->volume);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_rate), 	(gdouble)settings->rate);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_pitch), 	(gdouble)settings->pitch);
    gtk_range_set_value (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_volume), settings->volume);
    gtk_range_set_value (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_rate), 	settings->rate);
    gtk_range_set_value (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_pitch), 	settings->pitch);
    gtk_entry_set_text  (GTK_ENTRY (spui_speech_voice_add_modify.et_gnopernicus_voice), _(settings->gnopernicus_speaker));
    gtk_entry_set_text  (GTK_ENTRY (GTK_COMBO (spui_speech_voice_add_modify.cb_engine_voice)->entry), settings->voice_name_descr);
    gtk_entry_set_text  (GTK_ENTRY (GTK_COMBO (spui_speech_voice_add_modify.cb_engine_driver)->entry), settings->driver_name_descr);
    
    return TRUE;
}

static gboolean
spui_voice_add_modify_properties_settings_set (SpeakerSettings *settings)
{
    if (!spui_speech_voice_add_modify.w_voice_setting_add_modify) 
	return FALSE;
	
    spui_voice_old_speaker_setting_clean ();
    
    sp_old_speaker_settings.volume 		= settings->volume;
    sp_old_speaker_settings.rate   		= settings->rate;
    sp_old_speaker_settings.pitch  		= settings->pitch;
    sp_old_speaker_settings.voice_name 		= g_strdup (settings->voice_name);
    sp_old_speaker_settings.voice_name_descr 	= g_strdup (settings->voice_name_descr);
    sp_old_speaker_settings.driver_name 	= g_strdup (settings->driver_name);
    sp_old_speaker_settings.driver_name_descr 	= g_strdup (settings->driver_name_descr);
    sp_old_speaker_settings.gnopernicus_speaker = g_strdup (settings->gnopernicus_speaker);
    
    return spui_voice_add_modify_speaker_values_modify (settings);
}

static void 
spui_voice_add_modify_hs_value_changed (GtkWidget	*widget,
			    	    	gpointer	data)
{
    gint value1, value2;
    SpeakerSettings *curr_setting = NULL;
    
    if (!sp_curr_speaker_settings) 
	return;

    value1 = (gint) gtk_range_get_value (GTK_RANGE (widget));
    value2 = (gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON ((GtkWidget*)data));

    if (value1 == value2 || 
	!sp_changes_activable)
	return;
    
    curr_setting = (SpeakerSettings*)sp_curr_speaker_settings->data;
    
    curr_setting->volume = 
		    CHECK_RANGE_OF_VALUE ((gint)gtk_range_get_value (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_volume)), MIN_SPEECH_VOLUME, MAX_SPEECH_VOLUME);
    curr_setting->rate = 
		    CHECK_RANGE_OF_VALUE ((gint)gtk_range_get_value (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_rate)), MIN_SPEECH_RATE, MAX_SPEECH_RATE);
    curr_setting->pitch = 
		    CHECK_RANGE_OF_VALUE ((gint)gtk_range_get_value (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_pitch)), MIN_SPEECH_PITCH, MAX_SPEECH_PITCH);
    
    if (!sp_new_item)
        spconf_speaker_settings_save (curr_setting);
    else
	gtk_spin_button_set_value (GTK_SPIN_BUTTON ((GtkWidget*)data), (gdouble)value1);
    	
}


static void
spui_voice_add_modify_sp_value_changed (GtkWidget	*widget,
				    	gpointer	data)
{

    gint value1, value2;
    SpeakerSettings *curr_setting = NULL;
    
    if (!sp_curr_speaker_settings) 
	return;

    value1 = (gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
    value2 = (gint) gtk_range_get_value (GTK_RANGE ((GtkWidget*)data));

    if (value1 == value2 || 
	!sp_changes_activable)
	return;

    curr_setting = (SpeakerSettings*)sp_curr_speaker_settings->data;

    curr_setting->volume = 
		    CHECK_RANGE_OF_VALUE ((gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_volume)), MIN_SPEECH_VOLUME, MAX_SPEECH_VOLUME);
    curr_setting->rate   = 
		    CHECK_RANGE_OF_VALUE ((gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_rate)), MIN_SPEECH_RATE, MAX_SPEECH_RATE);
    curr_setting->pitch  = 
		    CHECK_RANGE_OF_VALUE ((gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_pitch)), MIN_SPEECH_PITCH, MAX_SPEECH_PITCH);
    
    if (!sp_new_item)
	spconf_speaker_settings_save (curr_setting);
    else
	gtk_range_set_value (GTK_RANGE ((GtkWidget*)data), (gdouble) value1);	
}

static gboolean
spui_voice_add_modify_sp_value_output (GtkWidget	*widget,
				    	    gpointer	data)

{
    spui_voice_add_modify_sp_value_changed (widget, data);

    return FALSE;
}


static void
spui_voice_add_modify_driver_combo_changed (GtkWidget *widget,
		        		    gpointer  user_data)
{
    const gchar  	*new_driver_name;
    const SpeechDriver  *driver;
    const SpeechVoice	*sp_voice;
    SpeakerSettings 	*curr_setting = NULL;
    GList		*sp_list = NULL;
    
    if (!sp_curr_speaker_settings ||
	!sp_changes_activable) 
	return;
	
    curr_setting = 
	(SpeakerSettings*)sp_curr_speaker_settings->data;
    
    new_driver_name = 
	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_add_modify.cb_engine_driver)->entry));    

    if (!new_driver_name ||
	!strlen (new_driver_name))
	return;

    /* get a new driver */	
    driver = spconf_driver_list_get_driver_from_name_descr (sp_speech_driver_list,
						            new_driver_name);

    /* verify if it is a valid driver */
    if (!driver)
	return;
	
    if (!strcmp (driver->driver_name, 
		 curr_setting->driver_name))
	return;

    /* get list of speaker for driver name */	
    sp_list = spconf_voice_list_for_combo_get (sp_speech_driver_list, 
					    driver->driver_name_descr, 
					    NULL);

    /* verify if the driver has a speaker */
    if (!sp_list)
	return;

    /* set speaker list in combo box */
    spui_voice_add_modify_combo_list_set (sp_list, spui_speech_voice_add_modify.cb_engine_voice); 
    	
    /* get speaker voice */
    sp_voice  = spconf_driver_list_get_speech_voice (sp_speech_driver_list,
						     driver->driver_name,
						     (gchar*)sp_list->data);

    /* free speaker list */
    spconf_list_for_combo_free (sp_list);

    /* free current driver and speaker values */
    g_free (curr_setting->driver_name);
    g_free (curr_setting->driver_name_descr);
    g_free (curr_setting->voice_name);
    g_free (curr_setting->voice_name_descr);
    
    /* set current driver and speaker values */
    curr_setting->voice_name 	    = g_strdup (sp_voice->voice_name);
    curr_setting->voice_name_descr  = g_strdup (sp_voice->voice_name_descr);
    curr_setting->driver_name 	    = g_strdup (driver->driver_name);
    curr_setting->driver_name_descr = g_strdup (driver->driver_name_descr);
    
    /* save new settings if it is a new item*/
    if (!sp_new_item)
	 spconf_speaker_settings_save (curr_setting);
}

static void
spui_voice_add_modify_speaker_combo_changed (GtkWidget *widget,
		        		     gpointer  user_data)
{

    const gchar 	*voice_name;
    SpeakerSettings 	*curr_setting = NULL;
    const SpeechVoice	*sp_voice;
    
    if (!sp_curr_speaker_settings ||
	!sp_changes_activable) 
	return;
    
    curr_setting = (SpeakerSettings*)sp_curr_speaker_settings->data;
    voice_name   = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_add_modify.cb_engine_voice)->entry));

    if (!strcmp (curr_setting->voice_name_descr, voice_name)) 
	return;

    sp_voice  = spconf_driver_list_get_speech_voice (sp_speech_driver_list,
						     curr_setting->driver_name,
						     voice_name);

    if (!sp_voice)
	return;
	
    if (!strcmp (curr_setting->gnopernicus_speaker, NONE_ELEMENT))
	return;

    g_free (curr_setting->voice_name);
    g_free (curr_setting->voice_name_descr);
    curr_setting->voice_name 		= g_strdup (sp_voice->voice_name);
    curr_setting->voice_name_descr 	= g_strdup (sp_voice->voice_name_descr);

    if (!sp_new_item)
        spconf_speaker_settings_save (curr_setting);
}

static void
spui_voice_add_modify_response (GtkDialog *dialog,
				gint       response_id,
				gpointer   user_data)
{
    switch (response_id)
    {
        case GTK_RESPONSE_OK: 
        {
	    if (sp_new_item)
	    {
		if (spui_voice_add_modify_properties_get ())
 		{
    		    spconf_speaker_settings_save ((SpeakerSettings*)sp_curr_speaker_settings->data);
		    spconf_gnopernicus_speakers_save (sp_gnopernicus_speakers);
		}
	    }
	    spui_voice_old_speaker_setting_clean ();
	    sp_changes_activable = FALSE;    
	}
        break;
	case GTK_RESPONSE_CANCEL:
	    spui_voice_add_modify_restore_state ();
	    spui_voice_old_speaker_setting_clean ();
	    sp_changes_activable = FALSE;
        break;
	case GTK_RESPONSE_HELP: 
	    gn_load_help ("gnopernicus-speech-prefs");
	    return;
	break;
        default:
        break;
    }
    gtk_widget_hide ((GtkWidget*)dialog);
}


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
spui_voice_add_modify_handlers_set  (GladeXML *xml)
{
    spui_speech_voice_add_modify.w_voice_setting_add_modify = glade_xml_get_widget (xml, "w_voice_setting_add_modify");
    
    spui_speech_voice_add_modify.cb_engine_voice 	= glade_xml_get_widget (xml, "cb_engine_voice");
    spui_speech_voice_add_modify.cb_engine_driver 	= glade_xml_get_widget (xml, "cb_engine_driver");
    spui_speech_voice_add_modify.et_gnopernicus_voice 	= glade_xml_get_widget (xml, "et_gnopernicus_voice");
    
    spui_speech_voice_add_modify.hs_voice_volume	= glade_xml_get_widget (xml, "hs_voice_volume");
    spui_speech_voice_add_modify.hs_voice_rate		= glade_xml_get_widget (xml, "hs_voice_rate");
    spui_speech_voice_add_modify.hs_voice_pitch		= glade_xml_get_widget (xml, "hs_voice_pitch");
    spui_speech_voice_add_modify.sp_voice_volume	= glade_xml_get_widget (xml, "sp_voice_volume");
    spui_speech_voice_add_modify.sp_voice_rate		= glade_xml_get_widget (xml, "sp_voice_rate");
    spui_speech_voice_add_modify.sp_voice_pitch		= glade_xml_get_widget (xml, "sp_voice_pitch");
        
    g_signal_connect (spui_speech_voice_add_modify.w_voice_setting_add_modify, "response",
		      G_CALLBACK (spui_voice_add_modify_response), NULL);
    g_signal_connect (spui_speech_voice_add_modify.w_voice_setting_add_modify, "delete_event",
                      G_CALLBACK (spui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect (xml,"on_combo-entry6_changed",		
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_driver_combo_changed));

    glade_xml_signal_connect (xml,"on_combo-entry3_changed",		
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_speaker_combo_changed));

    glade_xml_signal_connect_data (xml, "on_hs_voice_volume_value_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_hs_value_changed), 
			    (gpointer)spui_speech_voice_add_modify.sp_voice_volume);
    glade_xml_signal_connect_data (xml, "on_hs_voice_rate_value_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_hs_value_changed), 
			    (gpointer)spui_speech_voice_add_modify.sp_voice_rate);
    glade_xml_signal_connect_data (xml, "on_hs_voice_pitch_value_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_hs_value_changed),
			    (gpointer)spui_speech_voice_add_modify.sp_voice_pitch);

    glade_xml_signal_connect_data (xml, "on_sp_voice_volume_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_sp_value_changed), 
			    (gpointer)spui_speech_voice_add_modify.hs_voice_volume);
    glade_xml_signal_connect_data (xml, "on_sp_voice_volume_output",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_sp_value_output), 
			    (gpointer)spui_speech_voice_add_modify.hs_voice_volume);

    glade_xml_signal_connect_data (xml, "on_sp_voice_rate_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_sp_value_changed), 
			    (gpointer)spui_speech_voice_add_modify.hs_voice_rate);
    glade_xml_signal_connect_data (xml, "on_sp_voice_rate_output",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_sp_value_output), 
			    (gpointer)spui_speech_voice_add_modify.hs_voice_rate);

    glade_xml_signal_connect_data (xml, "on_sp_voice_pitch_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_sp_value_changed),
			    (gpointer)spui_speech_voice_add_modify.hs_voice_pitch);
    glade_xml_signal_connect_data (xml, "on_sp_voice_pitch_output",			
			    GTK_SIGNAL_FUNC (spui_voice_add_modify_sp_value_output), 
			    (gpointer)spui_speech_voice_add_modify.hs_voice_pitch);

}

static void
spui_voice_add_modify_set_ranges ()
{
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_volume), MIN_SPEECH_VOLUME, MAX_SPEECH_VOLUME);
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_rate), MIN_SPEECH_RATE, MAX_SPEECH_RATE);
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (spui_speech_voice_add_modify.sp_voice_pitch), MIN_SPEECH_PITCH, MAX_SPEECH_PITCH);
	
    gtk_range_set_range (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_volume), MIN_SPEECH_VOLUME, MAX_SPEECH_VOLUME);
    gtk_range_set_range (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_rate), MIN_SPEECH_RATE, MAX_SPEECH_RATE);
    gtk_range_set_range (GTK_RANGE (spui_speech_voice_add_modify.hs_voice_pitch), MIN_SPEECH_PITCH, MAX_SPEECH_PITCH);
}

static gboolean 
spui_voice_add_modify_load (GtkWidget *parent_window)
{         
    if (!spui_speech_voice_add_modify.w_voice_setting_add_modify)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Speech_Settings/speech_settings.glade2", "w_voice_setting_add_modify");
	sru_return_val_if_fail (xml, FALSE);
	spui_voice_add_modify_handlers_set (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (spui_speech_voice_add_modify.w_voice_setting_add_modify),
				      GTK_WINDOW (parent_window));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (spui_speech_voice_add_modify.w_voice_setting_add_modify), 
				            TRUE);
    }
    else
	gtk_widget_show (spui_speech_voice_add_modify.w_voice_setting_add_modify);
    
    spui_voice_add_modify_set_ranges ();    
    return TRUE;
}




/******************************************************************************/
static void
spui_voice_relative_modify_for_all_voices  (gint volume,
				    	    gint rate,
				    	    gint pitch)
{
    SpeakerSettingsListType *tmp = NULL;
    gboolean all_drivers = FALSE;
    gboolean all_voices = FALSE;
    const gchar *driver_name;
    const gchar *voice_name;
    
    driver_name = 
	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_relative.cb_rel_engine_driver)->entry));    
    voice_name = 	
	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_relative.cb_rel_engine_voice)->entry));    
    
    all_drivers = !strcmp (driver_name, SPEECH_ALL_DRIVERS);
    all_voices  = !strcmp (voice_name, SPEECH_ALL_VOICES);
    
    for (tmp = sp_speakers_settings; tmp ; tmp = tmp->next)
    {
	SpeakerSettings *sp_set = (SpeakerSettings *)tmp->data;
	if (!all_drivers)
	{
	    if (strcmp (sp_set->driver_name_descr, driver_name))
		continue;
		
	    if (!all_voices)
	    {
		if (strcmp (sp_set->voice_name_descr, voice_name))
		    continue;
	    }
	}
    
	if (volume)
	    sp_set->volume = CHECK_RANGE_OF_VALUE (sp_set->volume + volume, MIN_SPEECH_VOLUME, MAX_SPEECH_VOLUME);
	
	if (rate)
	    sp_set->rate = CHECK_RANGE_OF_VALUE (sp_set->rate + rate, MIN_SPEECH_RATE, MAX_SPEECH_RATE);
	
	if (pitch)
	    sp_set->pitch = CHECK_RANGE_OF_VALUE (sp_set->pitch + pitch, MIN_SPEECH_PITCH, MAX_SPEECH_PITCH);
    }

    spconf_speaker_settings_list_save (sp_speakers_settings);
}


static void
spui_voice_relative_volume_mode_clicked (GtkWidget *widget,
					 gpointer  user_data)
{
    const gchar *command = (const gchar*)user_data;
    gint     volume = 0;

    if (!strcmp (command, "INCREMENT"))
	volume = SPEECH_VOLUME_STEP;
    else
	volume = -SPEECH_VOLUME_STEP;

    spui_voice_relative_modify_for_all_voices (volume, 0, 0);
}

static void
spui_voice_relative_rate_mode_clicked (GtkWidget *widget,
				       gpointer  user_data)
{
    const gchar *command = (const gchar*)user_data;
    gint     rate = 0;

    if (!strcmp (command, "INCREMENT"))
	rate = SPEECH_RATE_STEP;
    else
	rate = -SPEECH_RATE_STEP;
    
    spui_voice_relative_modify_for_all_voices (0, rate, 0);
}

static void
spui_voice_relative_pitch_mode_clicked (GtkWidget *widget,
					gpointer  user_data)
{
    const gchar *command = (const gchar*)user_data;
    gint     pitch = 0;

    if (!strcmp (command, "INCREMENT"))
	pitch = SPEECH_PITCH_STEP;
    else
	pitch = -SPEECH_PITCH_STEP;

    spui_voice_relative_modify_for_all_voices (0, 0, pitch);
}

static void
spui_voice_relative_driver_combo_changed (GtkWidget *widget,
					 gpointer  user_data)
{
    GList *tmp = NULL;
    const gchar *driver_name;
    
    driver_name = 
	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_relative.cb_rel_engine_driver)->entry));    
    
    tmp = spconf_voice_list_for_combo_get (sp_speech_driver_list, driver_name, SPEECH_ALL_VOICES);
    spui_voice_add_modify_combo_list_set (tmp, spui_speech_voice_relative.cb_rel_engine_voice);
    spconf_list_for_combo_free (tmp);
}

static void
spui_voice_relative_response (GtkDialog *dialog,
			      gint       response_id,
			      gpointer   user_data)
{
    switch (response_id)
    {
	case GTK_RESPONSE_CLOSE:
	case GTK_RESPONSE_OK: 
        break;
	case GTK_RESPONSE_HELP: 
	    gn_load_help ("gnopernicus-speech-prefs");
	    return;
	break;
	default:
        break;
    }
    gtk_widget_hide ((GtkWidget*)dialog);
}

static void
spui_voice_relative_clear_all_combo (void)
{
    GList *tmp = NULL;
    
    tmp = spconf_driver_list_for_combo_get (sp_speech_driver_list, SPEECH_ALL_DRIVERS);
    spui_voice_add_modify_combo_list_set (tmp, spui_speech_voice_relative.cb_rel_engine_driver);
    spconf_list_for_combo_free (tmp);
    
    tmp = spconf_voice_list_for_combo_get (sp_speech_driver_list, "", SPEECH_ALL_VOICES);
    spui_voice_add_modify_combo_list_set (tmp, spui_speech_voice_relative.cb_rel_engine_voice);
    spconf_list_for_combo_free (tmp);
}

static void
spui_voice_relative_handlers_set  (GladeXML *xml)
{
    spui_speech_voice_relative.w_voice_setting_relative_values = glade_xml_get_widget (xml, "w_voice_setting_with_relative_values");
    
    spui_speech_voice_relative.cb_rel_engine_voice 	= glade_xml_get_widget (xml, "cb_rel_voice");
    spui_speech_voice_relative.cb_rel_engine_driver 	= glade_xml_get_widget (xml, "cb_rel_driver");
    
    g_signal_connect (spui_speech_voice_relative.w_voice_setting_relative_values, "response",
		      G_CALLBACK (spui_voice_relative_response), NULL);
    g_signal_connect (spui_speech_voice_relative.w_voice_setting_relative_values, "delete_event",
                      G_CALLBACK (spui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect_data (xml, "on_bt_rel_inc_volume_clicked",			
			    GTK_SIGNAL_FUNC (spui_voice_relative_volume_mode_clicked), 
			    (gpointer)"INCREMENT");
    glade_xml_signal_connect_data (xml, "on_bt_rel_dec_volume_clicked",			
			    GTK_SIGNAL_FUNC (spui_voice_relative_volume_mode_clicked), 
			    (gpointer)"DECREMENT");
			    
    glade_xml_signal_connect_data (xml, "on_bt_rel_inc_rate_clicked",			
			    GTK_SIGNAL_FUNC (spui_voice_relative_rate_mode_clicked), 
			    (gpointer)"INCREMENT");
    glade_xml_signal_connect_data (xml, "on_bt_rel_dec_rate_clicked",			
			    GTK_SIGNAL_FUNC (spui_voice_relative_rate_mode_clicked), 
			    (gpointer)"DECREMENT");

    glade_xml_signal_connect_data (xml, "on_bt_rel_inc_pitch_clicked",			
			    GTK_SIGNAL_FUNC (spui_voice_relative_pitch_mode_clicked), 
			    (gpointer)"INCREMENT");
    glade_xml_signal_connect_data (xml, "on_bt_rel_dec_pitch_clicked",			
			    GTK_SIGNAL_FUNC (spui_voice_relative_pitch_mode_clicked), 
			    (gpointer)"DECREMENT");


    glade_xml_signal_connect (xml,"on_rel_drv_entry_changed",		
			    GTK_SIGNAL_FUNC (spui_voice_relative_driver_combo_changed));
}

static gboolean 
spui_voice_relative_load (GtkWidget *parent_window)
{         
    if (!spui_speech_voice_relative.w_voice_setting_relative_values)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Speech_Settings/speech_settings.glade2", "w_voice_setting_with_relative_values");
	sru_return_val_if_fail (xml, FALSE);
	spui_voice_relative_handlers_set (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (spui_speech_voice_relative.w_voice_setting_relative_values),
				      GTK_WINDOW (parent_window));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (spui_speech_voice_relative.w_voice_setting_relative_values), 
				            TRUE);
    }
    else
	gtk_widget_show (spui_speech_voice_relative.w_voice_setting_relative_values);
    
    spui_voice_relative_clear_all_combo ();
    
    return TRUE;
}




/******************************************************************************/
static void
spui_voice_absolute_properties_sensitivity_set (gboolean sensitivity)
{
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.ck_abs_volume),  
			    sensitivity);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.ck_abs_rate),  
			    sensitivity);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.ck_abs_pitch),  
			    sensitivity);
}

static void
spui_voice_absolute_save_modify (gint volume,
				 gint rate,
				 gint pitch)
{
    SpeakerSettingsListType *tmp = NULL;
    
    const gchar *driver_name;
    const gchar *voice_name;
    gboolean all_drivers = FALSE;
    gboolean all_voices = FALSE;
    
    
    driver_name = 
	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_absolute.cb_abs_engine_driver)->entry));    
    voice_name = 	
    	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_absolute.cb_abs_engine_voice)->entry));    
    
    all_drivers = !strcmp (driver_name, SPEECH_ALL_DRIVERS);
    all_voices  = !strcmp (voice_name, SPEECH_ALL_VOICES);
    
    for (tmp = sp_speakers_settings; tmp ; tmp = tmp->next)
    {
	SpeakerSettings *sp_set = (SpeakerSettings *)tmp->data;
	if (!all_drivers)
	{

	    if (strcmp (sp_set->driver_name_descr, driver_name))
		continue;

	    if (!all_voices)
	    {
		if (strcmp (sp_set->voice_name_descr, voice_name))
		    continue;
	    }
	}

	if (volume > -1)
	    sp_set->volume = volume;
	if (rate > -1)
	    sp_set->rate = rate;
	if (pitch > -1)
	    sp_set->pitch = pitch;
	    
	spconf_speaker_settings_save (sp_set);
    }    
}

static gboolean
spui_voice_absolute_median_values_return (const gchar *driver_name,
					  const gchar *voice_name,
					  gint	*volume,
					  gint	*rate,
					  gint	*pitch)
{
    SpeakerSettingsListType *tmp = NULL;
    gboolean all_drivers = FALSE;
    gboolean all_voices = FALSE;
    
    gint count;
    gint mvolume = 0;
    gint mrate   = 0;
    gint mpitch  = 0;
    
    all_drivers = !strcmp (driver_name, SPEECH_ALL_DRIVERS);
    all_voices  = !strcmp (voice_name, SPEECH_ALL_VOICES);
    
    count = 0;
    
    *volume = -1;
    *rate   = -1;
    *pitch  = -1;
    
    for (tmp = sp_speakers_settings; tmp ; tmp = tmp->next)
    {
	SpeakerSettings *sp_set = (SpeakerSettings *)tmp->data;

	if (!all_drivers)
	{
	    if (strcmp (sp_set->driver_name_descr, driver_name))
		continue;
		
	    if (!all_voices)
	    {
		if (strcmp (sp_set->voice_name_descr, voice_name))
		    continue;
	    }
	}
	
	count   = count   + 1;
	mvolume = mvolume + sp_set->volume;
	mrate 	= mrate   + sp_set->rate;
	mpitch	= mpitch  + sp_set->pitch;
    }

    if (!count)
	return FALSE;
	
    *volume = (gint) (mvolume / count);
    *rate   = (gint) (mrate / count);
    *pitch  = (gint) (mpitch / count);
    
    return TRUE;
}

static void
spui_voice_absolute_set_values_to_parameters (void)
{
    const gchar *driver_name;
    const gchar *voice_name;
    gint volume = 0;
    gint rate   = 0;
    gint pitch  = 0;
    
    driver_name = 
	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_absolute.cb_abs_engine_driver)->entry));    
    voice_name = 	
    	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_absolute.cb_abs_engine_voice)->entry));    

    if (spui_voice_absolute_median_values_return (driver_name,
					          voice_name,
					          &volume,
					    	  &rate,
					          &pitch))
    {				
	sp_abs_changes_activable = FALSE;
	gtk_range_set_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_rate),   rate);
	gtk_range_set_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_volume), volume);
	gtk_range_set_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_pitch),  pitch);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_pitch),  (gdouble)pitch);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_volume), (gdouble)volume);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_rate),   (gdouble)rate);
	sp_abs_changes_activable = TRUE;
	spui_voice_absolute_properties_sensitivity_set (TRUE);
    }
    else
    {
	spui_voice_absolute_properties_sensitivity_set (FALSE);
    }
}

static void
spui_voice_absolute_clear_all_param (void)
{
    GList *tmp = NULL;
    GtkWidget *label;
    
    spui_speech_voice_absolute.abs_volume_sens = FALSE;
    spui_speech_voice_absolute.abs_rate_sens = FALSE;
    spui_speech_voice_absolute.abs_pitch_sens = FALSE;

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_volume), 
				    spui_speech_voice_absolute.abs_volume_sens);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_rate),  
				    spui_speech_voice_absolute.abs_rate_sens);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_pitch), 
				    spui_speech_voice_absolute.abs_pitch_sens);

    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_volume),
				"abs_volume");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_volume_sens);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_volume),
				"abs_volume_perc");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_volume_sens);

    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_rate),
				"abs_rate");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_rate_sens);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_rate),
				"abs_rate_word_min");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_rate_sens);

    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_pitch),
				"abs_pitch");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_pitch_sens);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_pitch),
				"abs_pitch_hz");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_pitch_sens);
			      
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.sp_abs_volume), 
			      spui_speech_voice_absolute.abs_volume_sens);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.hs_abs_volume), 
			      spui_speech_voice_absolute.abs_volume_sens);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.sp_abs_rate), 
			      spui_speech_voice_absolute.abs_rate_sens);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.hs_abs_rate), 
			      spui_speech_voice_absolute.abs_rate_sens);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.sp_abs_pitch), 
			      spui_speech_voice_absolute.abs_pitch_sens);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.hs_abs_pitch), 
			      spui_speech_voice_absolute.abs_pitch_sens);

    tmp = spconf_driver_list_for_combo_get (sp_speech_driver_list, SPEECH_ALL_DRIVERS);
    spui_voice_add_modify_combo_list_set (tmp, spui_speech_voice_absolute.cb_abs_engine_driver);
    spconf_list_for_combo_free (tmp);
    
    tmp = spconf_voice_list_for_combo_get (sp_speech_driver_list, "", SPEECH_ALL_VOICES);
    spui_voice_add_modify_combo_list_set (tmp, spui_speech_voice_absolute.cb_abs_engine_voice);
    spconf_list_for_combo_free (tmp);
}

static void
spui_voice_absolute_force_all_clicked (GtkWidget 	*widget,
			    	       gpointer		user_data)
{
    SpeakerSettingsListType *tmp = NULL;
    const gchar 	 *driver_name;
    const gchar 	 *voice_name;
    const SpeechDriver	 *sp_driver;
    const SpeechVoice	 *sp_voice;
    
    driver_name = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_absolute.cb_abs_engine_driver)->entry));    
    voice_name  = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_absolute.cb_abs_engine_voice)->entry));    

    if (!strcmp (driver_name, SPEECH_ALL_DRIVERS) ||
	!strcmp (voice_name, SPEECH_ALL_VOICES)  ||
	!driver_name || !voice_name)
    {
	gn_show_message (_("Invalid driver or voice!"));	
	return;
    }
    
    sp_driver = spconf_driver_list_get_driver_from_name_descr (sp_speech_driver_list,
							    driver_name);
    sp_voice  = spconf_driver_list_get_speech_voice (sp_speech_driver_list,
						     sp_driver->driver_name, 
						     voice_name);
    if (!sp_voice  ||
	!sp_driver)
	return;
	
    for (tmp = sp_speakers_settings; tmp ; tmp = tmp->next)
    {
	SpeakerSettings *sp_set = (SpeakerSettings *)tmp->data;
	
	if (strcmp (sp_set->driver_name_descr, driver_name))
	{
	    g_free (sp_set->driver_name);
	    g_free (sp_set->driver_name_descr);
	    sp_set->driver_name_descr = g_strdup (sp_driver->driver_name_descr);
	    sp_set->driver_name       = g_strdup (sp_driver->driver_name);
	}
	
	if (strcmp (sp_set->voice_name_descr, voice_name))
	{
	    g_free (sp_set->voice_name);
	    g_free (sp_set->voice_name_descr);
	    sp_set->voice_name 		= g_strdup (sp_voice->voice_name);
	    sp_set->voice_name_descr 	= g_strdup (sp_voice->voice_name_descr);
	}
    }

    spconf_speaker_settings_list_save (sp_speakers_settings);
    spui_voice_absolute_properties_sensitivity_set (TRUE);
}

static void
spui_voice_absolute_voice_combo_changed (GtkWidget *widget,
					 gpointer  user_data)
{
    spui_voice_absolute_set_values_to_parameters ();
}

static void
spui_voice_absolute_driver_combo_changed (GtkWidget *widget,
					 gpointer  user_data)
{
    GList *tmp = NULL;
    const gchar *driver_name;
    
    driver_name = 
	gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (spui_speech_voice_absolute.cb_abs_engine_driver)->entry));    
    
    tmp = spconf_voice_list_for_combo_get (sp_speech_driver_list, driver_name, SPEECH_ALL_VOICES);
    spui_voice_add_modify_combo_list_set (tmp, spui_speech_voice_absolute.cb_abs_engine_voice);
    spconf_list_for_combo_free (tmp);
    
    spui_voice_absolute_set_values_to_parameters ();
}

static void
spui_voice_absolute_volume_active (GtkWidget *widget,
				   gpointer  user_data)
{
    GtkWidget *label;
    spui_speech_voice_absolute.abs_volume_sens =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.sp_abs_volume), 
			      spui_speech_voice_absolute.abs_volume_sens);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.hs_abs_volume), 
			      spui_speech_voice_absolute.abs_volume_sens);
			      
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_volume),
				"abs_volume");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_volume_sens);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_volume),
				"abs_volume_perc");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_volume_sens);

}

static void
spui_voice_absolute_rate_active (GtkWidget *widget,
				 gpointer  user_data)
{
    GtkWidget *label;
    spui_speech_voice_absolute.abs_rate_sens =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.sp_abs_rate), 
			      spui_speech_voice_absolute.abs_rate_sens);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.hs_abs_rate), 
			      spui_speech_voice_absolute.abs_rate_sens);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_rate),
				"abs_rate");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_rate_sens);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_rate),
				"abs_rate_word_min");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_rate_sens);

}

static void
spui_voice_absolute_pitch_active (GtkWidget *widget,
				  gpointer  user_data)
{
    GtkWidget *label;
    spui_speech_voice_absolute.abs_pitch_sens =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.sp_abs_pitch), 
			      spui_speech_voice_absolute.abs_pitch_sens);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_absolute.hs_abs_pitch), 
			      spui_speech_voice_absolute.abs_pitch_sens);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_pitch),
				"abs_pitch");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_pitch_sens);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_pitch),
				"abs_pitch_hz");
    gtk_widget_set_sensitive (GTK_WIDGET (label), 
			      spui_speech_voice_absolute.abs_pitch_sens);

}

static void
spui_voice_absolute_hs_value_changed (GtkWidget *widget,
				      gpointer   user_data)
{
    gint value1, value2;
    gint volume = -1;
    gint rate   = -1;
    gint pitch  = -1;

    value1 = (gint) gtk_range_get_value (GTK_RANGE (widget));
    value2 = (gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON ((GtkWidget*)user_data));

    if (value1 == value2)
	return;
    
    gtk_spin_button_set_value (GTK_SPIN_BUTTON ((GtkWidget*)user_data), (gdouble)value1);	

    if (!sp_abs_changes_activable)
	return;
	
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_pitch)))
	pitch = (gint) gtk_range_get_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_pitch));
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_rate)))
	rate = (gint) gtk_range_get_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_rate));	
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_volume)))
	volume = (gint) gtk_range_get_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_volume));	
	
    spui_voice_absolute_save_modify (volume,
				     rate,
				     pitch);				     

}

static void
spui_voice_absolute_sp_value_changed (GtkWidget *widget,
				      gpointer   user_data)
{
    gint value1, value2;
    gint volume = -1;
    gint rate   = -1;
    gint pitch  = -1;

    value1 = (gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
    value2 = (gint) gtk_range_get_value (GTK_RANGE ((GtkWidget*)user_data));

    if (value1 == value2)
	return;

    gtk_range_set_value (GTK_RANGE ((GtkWidget*)user_data), (gdouble) value1);	

    if (!sp_abs_changes_activable)
	return;
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_pitch)))
	pitch = (gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_pitch));
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_rate)))
	rate = (gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_rate));	
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (spui_speech_voice_absolute.ck_abs_volume)))
	volume = (gint) gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_volume));	
	
    spui_voice_absolute_save_modify (volume,
				     rate,
				     pitch);				     

}


static gboolean
spui_voice_absolute_sp_value_output (GtkWidget *widget,
				    	   gpointer   user_data)
{
    spui_voice_absolute_sp_value_changed (widget, user_data);
    return FALSE;
}

static void
spui_voice_absolute_response (GtkDialog *dialog,
			      gint       response_id,
			      gpointer   user_data)
{
    switch (response_id)
    {
	case GTK_RESPONSE_CLOSE:
	break;
	case GTK_RESPONSE_OK: 
        break;
	case GTK_RESPONSE_HELP: 
	    gn_load_help ("gnopernicus-speech-prefs");
	    return;
	break;
	default:
        break;
    }
    gtk_widget_hide ((GtkWidget*)dialog);
    sp_abs_changes_activable = FALSE;
}

static void
spui_voice_absolute_handlers_set  (GladeXML *xml)
{
    spui_speech_voice_absolute.w_voice_setting_absolute_values = glade_xml_get_widget (xml, "w_voice_setting_with_absolute_values");
    
    spui_speech_voice_absolute.cb_abs_engine_voice 	= glade_xml_get_widget (xml, "cb_abs_voice");
    spui_speech_voice_absolute.cb_abs_engine_driver 	= glade_xml_get_widget (xml, "cb_abs_driver");
    
    spui_speech_voice_absolute.sp_abs_pitch 	   	= glade_xml_get_widget (xml, "sp_abs_pitch");
    spui_speech_voice_absolute.hs_abs_pitch 	   	= glade_xml_get_widget (xml, "hs_abs_pitch");
    spui_speech_voice_absolute.sp_abs_rate 	   	= glade_xml_get_widget (xml, "sp_abs_rate");
    spui_speech_voice_absolute.hs_abs_rate 	   	= glade_xml_get_widget (xml, "hs_abs_rate");
    spui_speech_voice_absolute.sp_abs_volume 	   	= glade_xml_get_widget (xml, "sp_abs_volume");
    spui_speech_voice_absolute.hs_abs_volume 	   	= glade_xml_get_widget (xml, "hs_abs_volume");
    spui_speech_voice_absolute.ck_abs_pitch 	   	= glade_xml_get_widget (xml, "ck_abs_pitch_active");
    spui_speech_voice_absolute.ck_abs_rate 	   	= glade_xml_get_widget (xml, "ck_abs_rate_active");
    spui_speech_voice_absolute.ck_abs_volume 	   	= glade_xml_get_widget (xml, "ck_abs_volume_active");    

    gtk_spin_button_set_range (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_pitch),
				    MIN_SPEECH_PITCH, MAX_SPEECH_PITCH);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_pitch),
				    DEFAULT_SPEECH_PITCH);

    gtk_spin_button_set_range (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_rate),
				    MIN_SPEECH_RATE, MAX_SPEECH_RATE);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_rate),
				    DEFAULT_SPEECH_RATE);

    gtk_spin_button_set_range (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_volume),
				    MIN_SPEECH_VOLUME, MAX_SPEECH_VOLUME);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spui_speech_voice_absolute.sp_abs_volume),
				    DEFAULT_SPEECH_VOLUME);

    gtk_range_set_range (GTK_RANGE (spui_speech_voice_absolute.hs_abs_pitch),
				    MIN_SPEECH_PITCH, MAX_SPEECH_PITCH);
    gtk_range_set_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_pitch),
				    DEFAULT_SPEECH_PITCH);

    gtk_range_set_range (GTK_RANGE (spui_speech_voice_absolute.hs_abs_rate),
				    MIN_SPEECH_RATE, MAX_SPEECH_RATE);
    gtk_range_set_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_rate),
				    DEFAULT_SPEECH_RATE);

    gtk_range_set_range (GTK_RANGE (spui_speech_voice_absolute.hs_abs_volume),
				    MIN_SPEECH_VOLUME, MAX_SPEECH_VOLUME);
    gtk_range_set_value (GTK_RANGE (spui_speech_voice_absolute.hs_abs_volume),
				    DEFAULT_SPEECH_VOLUME);

    g_object_set_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_volume), 
			"abs_volume", glade_xml_get_widget (xml, "lb_abs_volume"));
    g_object_set_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_volume), 
			"abs_volume_perc", glade_xml_get_widget (xml, "lb_abs_volume_perc"));
    g_object_set_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_rate), 
			"abs_rate", glade_xml_get_widget (xml, "lb_abs_rate"));
    g_object_set_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_rate), 
			"abs_rate_word_min", glade_xml_get_widget (xml, "lb_abs_rate_word_min"));
    g_object_set_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_pitch), 
			"abs_pitch", glade_xml_get_widget (xml, "lb_abs_pitch"));
    g_object_set_data (G_OBJECT (spui_speech_voice_absolute.ck_abs_pitch), 
			"abs_pitch_hz", glade_xml_get_widget (xml, "lb_abs_pitch_hz"));

    g_signal_connect (spui_speech_voice_absolute.w_voice_setting_absolute_values, "response",
		      G_CALLBACK (spui_voice_absolute_response), NULL);
    g_signal_connect (spui_speech_voice_absolute.w_voice_setting_absolute_values, "delete_event",
                      G_CALLBACK (spui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect (xml, "on_bt_abs_force_all_clicked",			
			      GTK_SIGNAL_FUNC (spui_voice_absolute_force_all_clicked));

    glade_xml_signal_connect (xml,"on_abs_drv_entry_changed",		
			    GTK_SIGNAL_FUNC (spui_voice_absolute_driver_combo_changed));

    glade_xml_signal_connect (xml,"on_abs_voice_entry_changed",		
			    GTK_SIGNAL_FUNC (spui_voice_absolute_voice_combo_changed));

    glade_xml_signal_connect (xml,"on_ck_abs_volume_active_toggled",		
			    GTK_SIGNAL_FUNC (spui_voice_absolute_volume_active));
    glade_xml_signal_connect (xml,"on_ck_abs_rate_active_toggled",		
			    GTK_SIGNAL_FUNC (spui_voice_absolute_rate_active));
    glade_xml_signal_connect (xml,"on_ck_abs_pitch_active_toggled",		
			    GTK_SIGNAL_FUNC (spui_voice_absolute_pitch_active));

    glade_xml_signal_connect_data (xml, "on_hs_abs_volume_value_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_hs_value_changed), 
			    (gpointer)spui_speech_voice_absolute.sp_abs_volume);
    glade_xml_signal_connect_data (xml, "on_hs_abs_rate_value_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_hs_value_changed), 
			    (gpointer)spui_speech_voice_absolute.sp_abs_rate);
    glade_xml_signal_connect_data (xml, "on_hs_abs_pitch_value_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_hs_value_changed),
			    (gpointer)spui_speech_voice_absolute.sp_abs_pitch);

    glade_xml_signal_connect_data (xml, "on_sp_abs_volume_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_sp_value_changed), 
			    (gpointer)spui_speech_voice_absolute.hs_abs_volume);
    glade_xml_signal_connect_data (xml, "on_sp_abs_volume_output",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_sp_value_output), 
			    (gpointer)spui_speech_voice_absolute.hs_abs_volume);

    glade_xml_signal_connect_data (xml, "on_sp_abs_rate_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_sp_value_changed), 
			    (gpointer)spui_speech_voice_absolute.hs_abs_rate);
    glade_xml_signal_connect_data (xml, "on_sp_abs_rate_output",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_sp_value_output), 
			    (gpointer)spui_speech_voice_absolute.hs_abs_rate);
    glade_xml_signal_connect_data (xml, "on_sp_abs_pitch_changed",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_sp_value_changed),
			    (gpointer)spui_speech_voice_absolute.hs_abs_pitch);
    glade_xml_signal_connect_data (xml, "on_sp_abs_pitch_output",			
			    GTK_SIGNAL_FUNC (spui_voice_absolute_sp_value_output), 
			    (gpointer)spui_speech_voice_absolute.hs_abs_pitch);
}

static gboolean 
spui_voice_absolute_load (GtkWidget *parent_window)
{         
    if (!spui_speech_voice_absolute.w_voice_setting_absolute_values)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Speech_Settings/speech_settings.glade2", "w_voice_setting_with_absolute_values");
	sru_return_val_if_fail (xml, FALSE);
	spui_voice_absolute_handlers_set (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (spui_speech_voice_absolute.w_voice_setting_absolute_values),
				      GTK_WINDOW (parent_window));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (spui_speech_voice_absolute.w_voice_setting_absolute_values), 
				            TRUE);
    }
    else
	gtk_widget_show (spui_speech_voice_absolute.w_voice_setting_absolute_values);
    
    sp_abs_changes_activable = TRUE;
    spui_voice_absolute_clear_all_param ();
    spui_voice_absolute_set_values_to_parameters ();
    
    return TRUE;
}




/******************************************************************************/
static gboolean
spui_voice_settings_button_add_clicked (GtkWidget *widget,
	    				gpointer  user_data)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;

    model 	  = gtk_tree_view_get_model 	(GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));
    selection     = gtk_tree_view_get_selection (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));

    if (!GTK_IS_LIST_STORE (model))
	return FALSE;

    /* load UI and show it*/
    spui_voice_add_modify_load (spui_speech_voice.w_voice_setting);

    gtk_list_store_append (GTK_LIST_STORE (model), &sp_curr_iter);
	
    sp_new_item = TRUE;
    
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_add_modify.et_gnopernicus_voice), sp_new_item);
    
    /* add default item in lists */
    sp_gnopernicus_speakers = 
	spconf_gnopernicus_speakers_add (sp_gnopernicus_speakers, NONE_ELEMENT);
    sp_speakers_settings = 
	spconf_speaker_settings_list_add (sp_speakers_settings, NONE_ELEMENT,
							     DEFAULT_SPEECH_ENGINE_DRIVER,
							     DEFAULT_SPEECH_ENGINE_VOICE,
							     DEFAULT_SPEECH_VOLUME, 
							     DEFAULT_SPEECH_PITCH,
							     DEFAULT_SPEECH_RATE);
    /* find item in lists */				    
    sp_curr_speaker_settings    = g_slist_last (sp_speakers_settings);
    sp_curr_gnopernicus_speaker = g_slist_last (sp_gnopernicus_speakers);
    
    /* set add/modify UI */
    if (sp_curr_speaker_settings)
	spui_voice_add_modify_properties_settings_set ((SpeakerSettings*)sp_curr_speaker_settings->data);
	    
    sp_changes_activable = TRUE;
    
    return TRUE;
}

static gboolean
spui_voice_settings_button_modify_clicked (GtkWidget *widget,
			    		    gpointer  user_data)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    gchar 	     *gnopernicus_speaker;

    model 	  = gtk_tree_view_get_model 	( GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));
    selection     = gtk_tree_view_get_selection ( GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));

    if (!GTK_IS_LIST_STORE (model))
	return FALSE;
	
    if (!gtk_tree_selection_get_selected (selection, NULL, &sp_curr_iter))
    {
    	gn_show_message (_("No selected voice to modify!"));
	return FALSE;
    }
    /* load UI and show it*/
    spui_voice_add_modify_load (spui_speech_voice.w_voice_setting);
    
    sp_new_item = FALSE;
    
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice_add_modify.et_gnopernicus_voice), sp_new_item);

    gtk_tree_model_get (model, 	&sp_curr_iter,
			VOICE_GNOPERNICUS_SPEAKER,	
			&gnopernicus_speaker,
                    	-1);

    /* find item in lists */				    
    sp_curr_speaker_settings = 
	    spconf_speaker_settings_list_find (sp_speakers_settings,  
					      gnopernicus_speaker);
    sp_curr_gnopernicus_speaker = 
	    spconf_gnopernicus_speakers_find (sp_gnopernicus_speakers, 
					      gnopernicus_speaker);
    /* set add/modify UI */
    if (sp_curr_speaker_settings)
	spui_voice_add_modify_properties_settings_set ((SpeakerSettings*)sp_curr_speaker_settings->data);
    
    g_free (gnopernicus_speaker);
    
    sp_changes_activable = TRUE;

    return TRUE;
}

static void
spui_voice_settings_row_activated_cb (GtkTreeView       *tree_view,
            	    		      GtkTreePath       *path,
		    		      GtkTreeViewColumn *column)
{
    spui_voice_settings_button_modify_clicked  (NULL, NULL);
}


static void
spui_voice_settings_button_remove_clicked (GtkWidget *widget,
					   gpointer user_data)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    GtkTreeIter	     iter;
    gchar 	     *gnoper_speaker;
    
    model 	= gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));
    selection	= gtk_tree_view_get_selection (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));

    if (!GTK_IS_LIST_STORE (model))
	return;
	
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
    	gn_show_message (_("No selected voice to remove!"));
	return;
    }
    
    gtk_tree_model_get (model, 	&iter,
			VOICE_GNOPERNICUS_SPEAKER, 
			&gnoper_speaker,
                    	-1);

    if (!gnoper_speaker)
	return;
	
    /* remove from gnopernicus_speaker list the elem */
    sp_gnopernicus_speakers =
	spconf_gnopernicus_speakers_remove (sp_gnopernicus_speakers,
					    gnoper_speaker);
    /* remove from sp_speakers_settings list the elem */	    
    sp_speakers_settings =
	spconf_speaker_settings_list_remove (sp_speakers_settings,
					     gnoper_speaker);

    /* remove from list store the elem */					  
    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    
    g_free (gnoper_speaker);
}

static void
spui_voice_settings_button_relative_clicked (GtkWidget *widget,
					     gpointer user_data)
{
    spui_voice_relative_load ((GtkWidget*)user_data);
}

static void
spui_voice_settings_button_absolute_clicked (GtkWidget *widget,
					     gpointer user_data)
{
    spui_voice_absolute_load ((GtkWidget*)user_data);
}

static void
spui_voice_settings_free_lists (void)
{
    sp_gnopernicus_speakers_backup =
	    spconf_gnopernicus_speakers_free (sp_gnopernicus_speakers_backup);
    sp_speakers_settings_backup  =
	    spconf_speaker_settings_list_free (sp_speakers_settings_backup);
    sp_gnopernicus_speakers =
	    spconf_gnopernicus_speakers_free (sp_gnopernicus_speakers);
    sp_speakers_settings  =
	    spconf_speaker_settings_list_free (sp_speakers_settings);
    sp_speech_driver_list =
	    spconf_driver_list_free (sp_speech_driver_list);
}

static void
spui_voice_settings_test_clicked (GtkWidget *widget,
				  gpointer  user_data)
{
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    GtkTreeIter	     iter;
    gchar 	     *gnop_speak;
    
    model 	  = gtk_tree_view_get_model 	( GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));
    selection     = gtk_tree_view_get_selection ( GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));

    if (!GTK_IS_LIST_STORE (model))
	return;
	
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
    	gn_show_message (_("No selected voice to test!"));
	return;
    }
    
    gtk_tree_model_get (model, 	&iter,
    			VOICE_GNOPERNICUS_SPEAKER, &gnop_speak,
                    	-1);
    if (!gnop_speak)
	return;

    /* set the voice which want to play */	    
    spconf_play_voice (gnop_speak);
    
    g_free (gnop_speak);    
}


static void
spui_voice_settings_speaker_values_in_table_modify (SpeakerSettings *value)
{
    GtkTreeModel     *model;
    GtkTreeIter	     iter;
    
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));

    /* return iter for speaker and set in treeview */
    if (spui_voice_iter_for_speaker_get (value->gnopernicus_speaker, &iter))
        spui_voice_speaker_parameter_in_list_store_at_iter_set (value, 
								GTK_LIST_STORE (model), 
								iter);
}

void
spui_voice_settings_modify_speaker_values (SpeakerSettings *value)
{
    gboolean old = sp_changes_activable;
    
    /* display new values for speaker in main table */
    spui_voice_settings_speaker_values_in_table_modify (value);
    sp_changes_activable = FALSE;
    
    /* if the add/modify window display is opened display the nwe values */
    spui_voice_add_modify_speaker_values_modify (value);
    sp_changes_activable = old;
}

static void
spui_selection_changed (GtkTreeSelection *selection,
			gpointer  user_data)
{
    gboolean 	     sensitive;

    sensitive = gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL, NULL);
    
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice.bt_voice_remove), sensitive);
    gtk_widget_set_sensitive (GTK_WIDGET (spui_speech_voice.bt_voice_modify), sensitive);
}

void
spui_voice_settings_gconf_callback (const gchar *path)
{
    GnopernicusSpeakerListType  *gs_list = NULL;
    SpeakerSettingsListType	*sp_list = NULL;
    SpeakerSettings 		*speaker = NULL;
        
    /* check if the modified speaker parameter exist */
    gs_list = spconf_gnopernicus_speakers_find  (sp_gnopernicus_speakers, path);
    sp_list = spconf_speaker_settings_list_find (sp_speakers_settings, path);
    
    if (!gs_list || !sp_list)
	return;

    /* load from gconf the speaker */			
    speaker = spconf_speaker_settings_load (path);
	
    if (!speaker)
	return;
	    
    /* clean current item */
    spconf_speaker_settings_free ((SpeakerSettings *)sp_list->data);
	    
    sp_list->data = speaker;
	    
    /* display new values in UI */
    spui_voice_settings_modify_speaker_values ((SpeakerSettings *)sp_list->data);
}

static gboolean
spui_voice_settings_search_equal_func (GtkTreeModel  	*model,
				    gint 		column,
				    const gchar		*key,
				    GtkTreeIter		*iter,
				    gpointer		search_data)
{
    gchar *list_key = NULL;
    
    gtk_tree_model_get (model, 	iter,
    			column, &list_key,
                    	-1);

    return g_strcasecmp (key, list_key) > 0; 
}

static void
spui_voice_settings_add_elem_in_table (GtkListStore *store)
{
    SpeakerSettingsListType *elem = NULL;
    GtkTreeIter iter;
    
    for (elem = sp_speakers_settings; elem ; elem = elem->next)
    {
	SpeakerSettings *item = (SpeakerSettings*)elem->data;
	
	if (!item)
	    continue;
	    
	gtk_list_store_append (GTK_LIST_STORE (store), &iter);
	
	spui_voice_speaker_parameter_in_list_store_at_iter_set (item, store, iter);
    }    
}

static GtkTreeModel*
spui_voice_settings_create_model (void)
{
    GtkListStore *store;      
    store = gtk_list_store_new (VOICE_NO_COLUMN, 
				G_TYPE_STRING,
				G_TYPE_STRING,
				G_TYPE_STRING,
				G_TYPE_INT,
				G_TYPE_INT,
				G_TYPE_INT);
    spui_voice_settings_add_elem_in_table (store);
    return GTK_TREE_MODEL (store) ;
}

static void
spui_voice_settings_response (GtkDialog *dialog,
			    gint       response_id,
			    gpointer   user_data)
{
    switch (response_id)
    {
	case GTK_RESPONSE_OK: 
        {
	    gtk_widget_hide ((GtkWidget*)dialog);
	    spconf_gnopernicus_speakers_save  (sp_gnopernicus_speakers);
	    spconf_speaker_settings_list_save (sp_speakers_settings);
	    spui_voice_settings_free_lists ();
	}
        break;
	case GTK_RESPONSE_CANCEL:
        {
	    gtk_widget_hide ((GtkWidget*)dialog);
	    spconf_gnopernicus_speakers_save  (sp_gnopernicus_speakers_backup);
	    spconf_speaker_settings_list_save (sp_speakers_settings_backup);
	    spui_voice_settings_free_lists ();
	}
        break;
	case GTK_RESPONSE_APPLY:
        {
	    spconf_gnopernicus_speakers_save  (sp_gnopernicus_speakers);
	    spconf_speaker_settings_list_save (sp_speakers_settings);
	    sp_gnopernicus_speakers_backup =
		    spconf_gnopernicus_speakers_free (sp_gnopernicus_speakers_backup);
	    sp_speakers_settings_backup  =
		    spconf_speaker_settings_list_free (sp_speakers_settings_backup);
	    sp_gnopernicus_speakers_backup = 
		    spconf_gnopernicus_speakers_clone (sp_gnopernicus_speakers);			   
	    sp_speakers_settings_backup  =
		    spconf_speaker_settings_list_clone (sp_speakers_settings);
	}
        break;
	case GTK_RESPONSE_HELP: 
	    gn_load_help ("gnopernicus-speech-prefs");
	break;
	default:
	    gtk_widget_hide ((GtkWidget*)dialog);
        break;
    }
    
}


static void
spui_voice_settings_handlers_set  (GladeXML *xml)
{
    GtkTreeModel 	*model;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;

    spui_speech_voice.w_voice_setting = glade_xml_get_widget (xml, "w_voice_setting");
    spui_speech_voice.tv_voice_table  = glade_xml_get_widget (xml, "tv_voice_table");
    spui_speech_voice.bt_voice_modify = glade_xml_get_widget (xml,"bt_voice_modify");	
    spui_speech_voice.bt_voice_remove =	glade_xml_get_widget (xml,"bt_voice_remove");	

    
    g_signal_connect (spui_speech_voice.w_voice_setting, "response",
		      G_CALLBACK (spui_voice_settings_response), NULL);
    g_signal_connect (spui_speech_voice.w_voice_setting, "delete_event",
                      G_CALLBACK (spui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect (xml,"on_bt_voice_add_clicked",	
			    GTK_SIGNAL_FUNC (spui_voice_settings_button_add_clicked));

    glade_xml_signal_connect (xml,"on_bt_voice_modify_clicked",	
			    GTK_SIGNAL_FUNC (spui_voice_settings_button_modify_clicked));

    glade_xml_signal_connect (xml,"on_bt_voice_remove_clicked",	
			     GTK_SIGNAL_FUNC (spui_voice_settings_button_remove_clicked));

    glade_xml_signal_connect_data (xml,"on_bt_rel_voice_set_clicked",	
			     GTK_SIGNAL_FUNC (spui_voice_settings_button_relative_clicked),
			     (gpointer)spui_speech_voice.w_voice_setting);

    glade_xml_signal_connect_data (xml,"on_bt_abs_voice_set_clicked",	
			     GTK_SIGNAL_FUNC (spui_voice_settings_button_absolute_clicked),
			     (gpointer)spui_speech_voice.w_voice_setting);

    glade_xml_signal_connect (xml, "on_bt_add_mod_voice_test_clicked",			
			     GTK_SIGNAL_FUNC (spui_voice_settings_test_clicked));

    model = spui_voice_settings_create_model ();
                    
    gtk_tree_view_set_model (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), model);
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table));
    
    g_signal_connect (selection, "changed",
                      G_CALLBACK (spui_selection_changed), NULL);
    
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), 
					  VOICE_GNOPERNICUS_SPEAKER, 
					  GTK_SORT_ASCENDING);

    g_signal_connect (spui_speech_voice.tv_voice_table, "row_activated", 
		      G_CALLBACK (spui_voice_settings_row_activated_cb), model);					        

    cell_renderer = gtk_cell_renderer_text_new ();
    
    column = gtk_tree_view_column_new_with_attributes   (_("Voice"),
    							cell_renderer,
							"text", VOICE_GNOPERNICUS_SPEAKER,
							NULL);	
    gtk_tree_view_column_set_sort_column_id (column, VOICE_GNOPERNICUS_SPEAKER);
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), column);
    
    column = gtk_tree_view_column_new_with_attributes   (_("Driver"),
    							cell_renderer,
							"text", VOICE_DRIVER_NAME,
							NULL);	
    gtk_tree_view_column_set_sort_column_id (column, VOICE_DRIVER_NAME);
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), column);
    
    column = gtk_tree_view_column_new_with_attributes   (_("Speaker"),
    							cell_renderer,
							"text", VOICE_DRIVER_VOICE,
							NULL);	
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), column);

    column = gtk_tree_view_column_new_with_attributes   (_("Volume"),
    							cell_renderer,
							"text", VOICE_VOLUME,
							NULL);	
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), column);

    column = gtk_tree_view_column_new_with_attributes   (_("Rate"),
    							cell_renderer,
							"text", VOICE_RATE,
							NULL);	
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), column);

    column = gtk_tree_view_column_new_with_attributes   (_("Pitch"),
    							cell_renderer,
							"text", VOICE_PITCH,
							NULL);	
    gtk_tree_view_column_set_clickable (column, TRUE);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), column);
    
    g_object_unref (G_OBJECT (model));
    
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);    

    spui_selection_changed (selection, NULL);
    
    gtk_tree_view_set_search_column 
	(GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), 
	VOICE_GNOPERNICUS_SPEAKER);    
    gtk_tree_view_set_search_equal_func 
	(GTK_TREE_VIEW (spui_speech_voice.tv_voice_table),
	spui_voice_settings_search_equal_func,
	NULL, NULL);
    gtk_tree_view_set_enable_search 
	(GTK_TREE_VIEW (spui_speech_voice.tv_voice_table), 
	TRUE);

}

gboolean 
spui_voice_settings_load (GtkWidget *parent_window)
{    
    g_assert (!sp_gnopernicus_speakers);
    g_assert (!sp_speakers_settings);
    g_assert (!sp_gnopernicus_speakers_backup);
    g_assert (!sp_speakers_settings_backup);
    g_assert (!sp_speech_driver_list);
    
    sp_gnopernicus_speakers = 
	    spconf_gnopernicus_speakers_load (sp_gnopernicus_speakers,
					      &sp_speakers_settings);			   
    sp_gnopernicus_speakers_backup = 
	    spconf_gnopernicus_speakers_clone (sp_gnopernicus_speakers);			   
    sp_speakers_settings_backup  =
	    spconf_speaker_settings_list_clone (sp_speakers_settings);
    sp_speech_driver_list = 
	    spconf_driver_list_init ();
    
    if (!spui_speech_voice.w_voice_setting)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Speech_Settings/speech_settings.glade2", "w_voice_setting");
	sru_return_val_if_fail (xml, FALSE);
	spui_voice_settings_handlers_set (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (spui_speech_voice.w_voice_setting),
				      GTK_WINDOW (parent_window));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (spui_speech_voice.w_voice_setting), 
					    TRUE);
    }
    else
    {
	GtkTreeModel *model;
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (spui_speech_voice.tv_voice_table) );
	gtk_list_store_clear (GTK_LIST_STORE (model));
	spui_voice_settings_add_elem_in_table (GTK_LIST_STORE (model));
	gtk_widget_show (spui_speech_voice.w_voice_setting);
    }
                
    return TRUE;
}
