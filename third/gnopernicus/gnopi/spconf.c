/* spconf.c
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
#include "spconf.h"
#include "libsrconf.h"
#include "spvoiceui.h"
#include "gnopiconf.h"

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include "srintl.h"
#include <gnome.h>

void	spconf_changes_cb	(GConfClient	*client,
				guint		cnxn_id,
				GConfEntry	*entry,
				gpointer	user_data);

extern Speech 	*speech_setting;
static guint 	sp_notify_id;

/**
 *
 * Default values for voices.
 *
**/
SpeakerSettings speakers[]=
{
    {"accelerator",	DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"childcount",	DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"empty",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"location",	DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"message",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"name",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"description",	DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"role",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"column_header",	DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"row_header",	DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"cell",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"shortcut",	DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"state",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"system",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"text",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH},
    {"value",		DEFAULT_SPEECH_ENGINE_DRIVER, NULL, DEFAULT_SPEECH_ENGINE_VOICE, NULL,	DEFAULT_SPEECH_VOLUME,	DEFAULT_SPEECH_RATE,	DEFAULT_SPEECH_PITCH}
};

extern GConfClient *gnopernicus_client;
DictionaryListType *dictionary_list = NULL;

/**
 *
 * Create a new gconf client for Speech settings
 * <return> - gconf client
 *
**/
gboolean
spconf_gconf_client_init (void)
{
    GError *error = NULL;
    	 
    sru_return_val_if_fail (gnopiconf_client_add_dir (CONFIG_PATH SPEECH_PATH), FALSE);

    gconf_client_notify_add (gnopernicus_client,
			    CONFIG_PATH SPEECH_PARAMETER_SECTION,
			    spconf_changes_cb,
			    NULL,NULL,&error);
    if (error != NULL)
    {
        sru_warning (_("Failed to add notify."));
	sru_warning (_(error->message));
        g_error_free (error);
        error = NULL;
        return FALSE;
    }
    
    return TRUE;
}

/**
 *
 * Load value for Speech settings
 *
**/
Speech* 
spconf_setting_init (gboolean set_struct)
{
    Speech *value = NULL;
    
    value = spconf_setting_new ();

    sru_assert (value);
    
    value->count_type       = spconf_count_get ();
    value->punctuation_type = spconf_punctuation_get ();
    value->text_echo_type   = spconf_text_echo_get ();
    value->modifiers_type   = spconf_modifiers_get ();
    value->cursors_type     = spconf_cursors_get ();
    value->spaces_type      = spconf_spaces_get ();
    value->dictionary	    = spconf_dictionary_get ();
    
    spconf_set_default_voices (FALSE); /* default for voices */
    
    dictionary_list =
	spconf_dictionary_free (dictionary_list);
    
    dictionary_list =
        spconf_dictionary_load ();

    if (!set_struct)
	return value;

    spconf_setting_free (speech_setting);
    speech_setting = value;

    return value;
}

/**
 *
 * Create a new Speech structure				
 * <return> - new Speech structure
 *
**/
Speech* 
spconf_setting_new (void)
{ 
    Speech *new_speech = NULL ; 
    
    new_speech = (Speech *) g_new0 (Speech, 1) ; 
    
    if (!new_speech) 
	sru_error (_("Unable to allocate memory."));
    
    return new_speech ; 
} 

/**
 *
 * Load default value for Speech structure					
 *
**/
void 
spconf_load_default_settings (Speech* speech)
{   
    GConfValue *value = NULL;
    
    sru_return_if_fail (speech);
    
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_COUNT, 
						NULL);
    g_free (speech->count_type);
    if (value)
    {
	speech->count_type = g_strdup ((gchar*)gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	speech->count_type  = g_strdup (DEFAULT_SPEECH_COUNT_TYPE);
		
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_PUNCTUATION, 
						NULL);
    g_free (speech->punctuation_type);
    if (value)
    {
	speech->punctuation_type = g_strdup ((gchar*)gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	speech->punctuation_type  = g_strdup (DEFAULT_SPEECH_PUNCTUATION);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_TEXT_ECHO,
						NULL);
    g_free (speech->text_echo_type);
    if (value)
    {
	speech->text_echo_type = g_strdup ((gchar*)gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	speech->text_echo_type  = g_strdup (DEFAULT_SPEECH_TEXT_ECHO);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_MODIFIERS,
						NULL);
    g_free (speech->modifiers_type);
    if (value)
    {
	speech->modifiers_type = g_strdup ((gchar*)gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	speech->modifiers_type  = g_strdup (DEFAULT_SPEECH_MODIFIERS);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_CURSORS,
						NULL);
    g_free (speech->cursors_type);
    if (value)
    {
	speech->cursors_type = g_strdup ((gchar*)gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	speech->cursors_type  = g_strdup (DEFAULT_SPEECH_CURSORS);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_SPACES,
						NULL);
    g_free (speech->spaces_type);
    if (value)
    {
	speech->spaces_type = g_strdup ((gchar*)gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	speech->spaces_type  = g_strdup (DEFAULT_SPEECH_SPACES);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_DICTIONARY_ACTIVE,
						NULL);
    g_free (speech->dictionary);
    if (value)
    {
	speech->dictionary = g_strdup ((gchar*)gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	speech->dictionary  = g_strdup (DEFAULT_SPEECH_DICTIONARY_ACTIVE);

    spconf_set_default_voices (TRUE);
    
    dictionary_list =
	spconf_dictionary_free (dictionary_list);
    
    dictionary_list =
	spconf_dictionary_load_default ();
}


/**
 *
 * Free Speech structure					
 *
**/
void 
spconf_setting_free (Speech* speech)
{
    if (!speech)
	return;
    
    g_free (speech->count_type);
    g_free (speech->punctuation_type);
    g_free (speech->text_echo_type);
    g_free (speech->modifiers_type);
    g_free (speech->cursors_type);
    g_free (speech->spaces_type);
    g_free (speech->dictionary);
    g_free (speech);
    speech = NULL;
    
    dictionary_list =
	spconf_dictionary_free (dictionary_list);

}

/**
 *
 * Terminate Speech configure					
 *
**/
void 
spconf_terminate (Speech *speech)
{
    spconf_setting_free (speech);
    
    gconf_client_notify_remove (gnopernicus_client, sp_notify_id);
    	
    if (gconf_client_dir_exists (gnopernicus_client, CONFIG_PATH SPEECH_PATH,NULL))
	gconf_client_remove_dir (gnopernicus_client, CONFIG_PATH SPEECH_PATH,NULL);
	
}

/**
 *
 * Set Methods 
 *
**/
void 
spconf_setting_set (const Speech *speech)
{
    spconf_cursors_set 	   (speech->cursors_type);
    spconf_spaces_set 	   (speech->spaces_type);
    spconf_modifiers_set   (speech->modifiers_type);
    spconf_count_set 	   (speech->count_type);
    spconf_punctuation_set (speech->punctuation_type);
    spconf_dictionary_set  (speech->dictionary);
    spconf_text_echo_set   (speech->text_echo_type);
}

void
spconf_dictionary_changes (void)
{
    gnopiconf_unset_key (CONFIG_PATH SPEECH_VOICE_KEY_PATH SPEECH_DICTIONARY_CHANGES);
    if (!gnopiconf_set_bool (TRUE, CONFIG_PATH SPEECH_VOICE_KEY_PATH SPEECH_DICTIONARY_CHANGES))
	sru_warning (_("Failed to set dictionary changes."));
}

void
spconf_speech_voice_removed (const gchar *voice)
{
    gnopiconf_unset_key (CONFIG_PATH SPEECH_VOICE_KEY_PATH SPEECH_VOICE_REMOVED);
    if (!gnopiconf_set_string (voice, CONFIG_PATH SPEECH_VOICE_KEY_PATH SPEECH_VOICE_REMOVED))
	sru_warning (_("Failed to set removed voice value."));
}



void 
spconf_count_set (const gchar *count)
{
    if (!gnopiconf_set_string (count, CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_COUNT))
    {
	sru_warning (_("Failed to set count: %s."),count);
    }
}

void 
spconf_punctuation_set (const gchar *punctuation)
{
    if (!gnopiconf_set_string (punctuation, CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_PUNCTUATION))
    {
	sru_warning (_("Failed to set Punctuation: %s."),punctuation);
    }
}

void 
spconf_text_echo_set (const gchar *text_echo)
{
    if (!gnopiconf_set_string (text_echo, CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_TEXT_ECHO))
    {
	sru_warning (_("Failed to set text echo value: %s."),text_echo);
    }
}

void 
spconf_modifiers_set (const gchar *modifiers)
{
    if (!gnopiconf_set_string (modifiers, CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_MODIFIERS))
    {
	sru_warning (_("Failed to set modifiers value: %s."),modifiers);
    }
}

void 
spconf_cursors_set (const gchar *cursors)
{
    if (!gnopiconf_set_string (cursors, CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_CURSORS))
    {
	sru_warning (_("Failed to set cursors value: %s."),cursors);
    }
}

void 
spconf_spaces_set (const gchar *spaces)
{
    if (!gnopiconf_set_string (spaces, CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_SPACES))
    {
	sru_warning (_("Failed to set spaces value: %s."),spaces);
    }
}

void 
spconf_dictionary_set (const gchar *dictionary)
{
    if (!gnopiconf_set_string (dictionary, CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_DICTIONARY_ACTIVE))
    {
	sru_warning (_("Failed to set dictionary value: %s."),dictionary);
    }
}

void
spconf_play_voice (const gchar *gnopernicus_speaker)
{
    gnopiconf_unset_key (CONFIG_PATH SPEECH_VOICE_KEY_PATH SPEECH_VOICE_TEST);
    if (!gnopiconf_set_string (gnopernicus_speaker, CONFIG_PATH SPEECH_VOICE_KEY_PATH SPEECH_VOICE_TEST))
	sru_warning (_("Failed to set testable voice %s."),gnopernicus_speaker);
}

/**
 *
 * Get Methods
 *
**/

gchar*
spconf_count_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_COUNT, 
					     DEFAULT_SPEECH_COUNT_TYPE);
}

gchar*
spconf_punctuation_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_PUNCTUATION, 
					      DEFAULT_SPEECH_PUNCTUATION);						     
}

gchar*
spconf_text_echo_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_TEXT_ECHO, 
					      DEFAULT_SPEECH_TEXT_ECHO);						     
}

gchar*
spconf_modifiers_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_MODIFIERS, 
					      DEFAULT_SPEECH_MODIFIERS);						     
}

gchar*
spconf_cursors_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_CURSORS, 
					      DEFAULT_SPEECH_CURSORS);						     
}

gchar*
spconf_spaces_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_SPACES, 
					      DEFAULT_SPEECH_SPACES);						     
}

gchar*
spconf_dictionary_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SPEECH_SETTING_KEY_PATH SPEECH_DICTIONARY_ACTIVE, 
					      DEFAULT_SPEECH_DICTIONARY_ACTIVE);						     
}

/******************************************************************************/
/**
 * spconf_speech_voice_new:
 *
 * Create a new SpeechVoice structure
 *
 * return: new allocated structure.
**/
static SpeechVoice*
spconf_speech_voice_new (void)
{
    SpeechVoice *sp_voice;
    
    sp_voice = (SpeechVoice *) g_new0 (SpeechVoice, 1) ; 
    
    sru_assert (sp_voice);
    
    return sp_voice;
}

/**
 * spconf_speech_voice_free:
 *
 * @sp_voice: structure.
 *
 * Free Voice strucure
 *
**/
static void
spconf_speech_voice_free (SpeechVoice *sp_voice)
{
    if (!sp_voice)
	return;
	
    g_free (sp_voice->voice_name);
    g_free (sp_voice->voice_name_descr);
    g_free (sp_voice);
    sp_voice = NULL;    
}

/**
 * spconf_speech_driver_new:
 *
 * Create a new driver structure.
 *
 * return: new allocated driver.
**/
static SpeechDriver*
spconf_speech_driver_new (void)
{
    SpeechDriver *sp_driver;
    
    sp_driver = (SpeechDriver *) g_new0 (SpeechDriver, 1) ; 
    
    sru_assert (sp_driver);
    
    return sp_driver;
}

/**
 * spconf_speech_driver_free:
 *
 * @sp_driver:
 *
 * Free driver structure.
**/
static void
spconf_speech_driver_free (SpeechDriver *sp_driver)
{
    GSList *tmp;
    
    for (tmp = sp_driver->driver_voice_list; tmp ; tmp = tmp->next)
    {
	spconf_speech_voice_free ((SpeechVoice *)tmp->data);
	tmp->data = NULL;
    }
	
    g_slist_free (sp_driver->driver_voice_list);
    
    g_free (sp_driver->driver_name);
    g_free (sp_driver->driver_name_descr);
    g_free (sp_driver);
}

/**
 * spconf_driver_list_driver_descr_name_get:
 *
 * @driver_name:
 *
 * Get a description name of driver. (e.g. "IBM Viavoce GNOME Speech Driver")
 *
 * return: driver name. If no longer use need to free it.
**/
gchar*
spconf_driver_list_driver_descr_name_get (const gchar *driver_name)
{
    if (!driver_name)
	return NULL;
	
    return g_strdelimit (g_strdup (driver_name), "_", ' ');
}

/**
 * spconf_driver_list_voice_descr_name_get:
 *
 * @voice_name:
 *
 * Get a description name of voice. (e.g. V1 Child1)
 *
 * return: voice name. If no longer use need to free it.
**/
gchar*
spconf_driver_list_voice_descr_name_get (const gchar *voice_name)
{
    gchar **tokens;
    gchar *rv = NULL;
    
    if (!voice_name)
	return NULL;
	
    /* voice pattern is "V%d voice_name - engine_driver" */
    
    tokens = g_strsplit (voice_name, "-", 0);
    
    if (tokens[0])
    {
	gchar *vname = NULL;
	
	/* 4 = strlen ("V%d voice_name ") if voice_name == NULL */
	vname = g_strchomp (tokens[0]);
	
	if (vname[2] == '\0' ||
	    vname[3] == '\0')
	{
	    rv = g_strdup_printf (_("%s no name"), vname);
	}
	else
	{
	    rv = g_strdup (tokens[0]);
	}	
    }
	
    g_strfreev (tokens);
    
    return rv;
}

/**
 * spconf_driver_list_load_speakers:
 *
 * @driver_name:
 *
 * Return a speaker list for a driver_name driver.
 *
 * return: speaker list
**/
static GSList*
spconf_driver_list_load_speakers (const gchar *driver_name)
{
    GSList *tmp_list = NULL;
    GSList *voice_list = NULL;
    GSList *iter = NULL;
    GConfValueType type;
    gchar *path;
    
    path = g_strdup_printf ("%s%s%s", 
			    CONFIG_PATH, 
			    SPEECH_DRIVERS_PATH, 
			    driver_name);
			    
    tmp_list = gnopiconf_get_list_with_default (path, NULL, &type);
    g_free (path);
    
    for (iter = tmp_list; iter; iter = iter->next)
    {
	SpeechVoice *sp_voice 	    = spconf_speech_voice_new ();
	sp_voice->voice_name_descr  = spconf_driver_list_voice_descr_name_get (iter->data);
	sp_voice->voice_name  	    = iter->data;
	iter->data 		    = NULL;
	voice_list                  = g_slist_append (voice_list, sp_voice);
    }
        
     g_slist_free (tmp_list);
    
    return voice_list;
}

/**
 * spconf_driver_list_init:
 *
 * Initialize active driver list. (driver and voices)
 *
 * return: driver list
**/
SpeechDriverListType*
spconf_driver_list_init (void)
{
    GSList *tmp_list = NULL, *list = NULL;
    SpeechDriverListType *driver_list = NULL;
    GConfValueType type;
    SpeechDriver *driver = NULL;

    tmp_list = gnopiconf_get_list_with_default (
		CONFIG_PATH SPEECH_DRIVERS_PATH SPEECH_ENGINE_DRIVERS,
		NULL, &type);

    if (!tmp_list)
	return NULL;
    
    for (list = tmp_list; list ; list = list->next)
    {
	if (!list->data)
	    continue;
	    
	driver = spconf_speech_driver_new ();
	driver->driver_name 	  = list->data;
	driver->driver_name_descr = spconf_driver_list_driver_descr_name_get (list->data);
	driver->driver_voice_list = spconf_driver_list_load_speakers (driver->driver_name);
	list->data = NULL;
	
	driver_list = g_slist_append (driver_list, driver);
    }
    
    g_slist_free (tmp_list);
    
    return driver_list;
}

/**
 * spconf_driver_list_free:
 *
 * @driver_list:
 *
 * Free driver list.
**/
SpeechDriverListType*
spconf_driver_list_free (SpeechDriverListType *driver_list)
{
    SpeechDriverListType *tmp;
    
    for (tmp = driver_list; tmp ; tmp = tmp->next)
    {
	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	
	spconf_speech_driver_free (driver);
	tmp->data = NULL;
    }
    
    g_slist_free (driver_list);    
    
    driver_list = NULL;
    
    return driver_list;
}

/**
 * spconf_driver_list_get_driver_name:
 * 
 * @driver_list:
 * @driver_name_descr:
 *
 * Return a driver name from description name
 *
 * return: const driver name.
**/
const gchar*
spconf_driver_list_get_driver_name (SpeechDriverListType *driver_list,
				    const gchar *driver_name_descr)
{
    SpeechDriverListType *tmp;
    
    if (!driver_list)
	return NULL;
    if (!driver_name_descr)
	return NULL;

    for (tmp = driver_list ; tmp ; tmp = tmp->next)
    {
	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	
	if (!strcmp (driver->driver_name_descr, driver_name_descr))
	    return driver->driver_name;
    }
    
    return NULL;
}

/**
 * spconf_driver_list_get_driver_name_descr:
 *
 * @driver_list:
 * @driver_name:
 *
 * Return a driver description name for drive_name
 *
 * return: const driver_name_descr
**/
const gchar*
spconf_driver_list_get_driver_name_descr (SpeechDriverListType *driver_list,
					 const gchar *driver_name)
{
    SpeechDriverListType *tmp;
    
    if (!driver_list)
	return NULL;
    if (!driver_name)
	return NULL;

    for (tmp = driver_list ; tmp ; tmp = tmp->next)
    {
	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	
	if (!strcmp (driver->driver_name, driver_name))
	    return driver->driver_name_descr;
    }
    
    return NULL;
}

/**
 * spconf_driver_list_get_driver_from_name_descr:
 *
 * @driver_list:
 * @driver_name_desc:
 *
 * Return a driver whit driver_name_descr.
 *
 * return:
**/
const SpeechDriver*
spconf_driver_list_get_driver_from_name_descr (SpeechDriverListType *driver_list,
					      const gchar *driver_name_descr)
{
    SpeechDriverListType *tmp;
    
    if (!driver_list)
	return NULL;
    if (!driver_name_descr)
	return NULL;

    for (tmp = driver_list ; tmp ; tmp = tmp->next)
    {
	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	
	if (!strcmp (driver->driver_name_descr, driver_name_descr))
	    return driver;
    }
    
    return NULL;
}

/**
 * spconf_driver_list_get_driver:
 *
 * @driver_list:
 * @driver_name:
 *
 * Return driver whit driver_name
 *
 * return: NULL if the driver_name not found or error 
**/
const SpeechDriver*
spconf_driver_list_get_driver (SpeechDriverListType *driver_list,
			       const gchar *driver_name)
{
    SpeechDriverListType *tmp;
    
    if (!driver_list)
	return NULL;
    if (!driver_name)
	return NULL;

    for (tmp = driver_list ; tmp ; tmp = tmp->next)
    {
	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	
	if (!strcmp (driver->driver_name, driver_name))
	    return driver;
    }
    
    return NULL;
}

/**
 * spconf_driver_list_get_driver_by_voice:
 * (This function is deprecated)
 *
 * @driver_list:
 * @voice_name:
 *
 * Return driver which has a voice.
 *
 * return: NULL if the voice_name not found or error. 
**/
const SpeechDriver*
spconf_driver_list_get_driver_by_voice (SpeechDriverListType *driver_list,
			    		const gchar *voice_name)
{
    SpeechDriverListType *tmp;
    
    if (!driver_list)
	return NULL;
    if (!voice_name)
	return NULL;

    for (tmp = driver_list ; tmp ; tmp = tmp->next)
    {
	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	GSList *voice_list   = NULL;
	
	for (voice_list = driver->driver_voice_list; voice_list ; voice_list = voice_list->next)
	{
	    SpeechVoice *speech_voice = (SpeechVoice*)voice_list->data;
	    
	    if (!strcmp (speech_voice->voice_name, voice_name))
		return driver;
	}
    }
    
    return NULL;
}

/**
 * spconf_driver_list_get_voice_name:
 *
 * @driver_list:
 * @driver_name:
 * @voice_name_descr:
 *
 * Return a voice name, from driver_name driver and which has voice_name_descr.
 *
 * return: const
**/
const gchar*
spconf_driver_list_get_voice_name (SpeechDriverListType *driver_list,
				   const gchar *driver_name,
			    	   const gchar *voice_name_descr)
{
    SpeechDriverListType *tmp;
    
    if (!driver_list)
	return NULL;
    if (!driver_name)
	return NULL;
    if (!voice_name_descr)
	return NULL;

    for (tmp = driver_list ; tmp ; tmp = tmp->next)
    {
	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	GSList *voice_list   = NULL;
	
	if (strcmp (driver->driver_name, driver_name))
	    continue;

	for (voice_list = driver->driver_voice_list; voice_list ; voice_list = voice_list->next)
	{
	    SpeechVoice *speech_voice = (SpeechVoice*)voice_list->data;
	    
	    if (!strcmp (speech_voice->voice_name_descr, voice_name_descr))
		return speech_voice->voice_name;
	}
    }
    
    return NULL;
}

const SpeechVoice*
spconf_driver_list_get_speech_voice (SpeechDriverListType *driver_list,
				     const gchar *driver_name,
			    	     const gchar *voice_name_descr)
{
    SpeechDriverListType *tmp;
    
    if (!driver_list)
	return NULL;
    if (!driver_name)
	return NULL;
    if (!voice_name_descr)
	return NULL;

    for (tmp = driver_list ; tmp ; tmp = tmp->next)
    {
	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	GSList *voice_list   = NULL;
	
	if (strcmp (driver->driver_name, driver_name))
	    continue;

	for (voice_list = driver->driver_voice_list; voice_list ; voice_list = voice_list->next)
	{
	    SpeechVoice *speech_voice = (SpeechVoice*)voice_list->data;
	    
	    if (!strcmp (speech_voice->voice_name_descr, voice_name_descr))
		return speech_voice;
	}
    }
    
    return NULL;
}

/*****************************************************************************/
GnopernicusSpeakerListType*
spconf_gnopernicus_speakers_clone (GnopernicusSpeakerListType* gs_list)
{
    GnopernicusSpeakerListType *elem;
    GnopernicusSpeakerListType *clone = NULL;
    
    if (!gs_list) 
	return NULL;
    
    for (elem = gs_list; elem ; elem = elem->next)
    {
	if (elem->data)
	    clone = g_slist_append (clone, g_strdup ((gchar*)elem->data));
    }
    return clone;
}

GnopernicusSpeakerListType*
spconf_gnopernicus_speakers_free (GnopernicusSpeakerListType *gs_list)
{
    if (gs_list)
	gs_list = spconf_free_slist_with_char_data (gs_list);
	
    return gs_list;
}

GnopernicusSpeakerListType*
spconf_gnopernicus_speakers_add (GnopernicusSpeakerListType *gs_list,
				 const gchar *gn_speaker)
{   
    GnopernicusSpeakerListType *elem;

    sru_return_val_if_fail (gn_speaker, NULL);
    
    elem = spconf_gnopernicus_speakers_find (gs_list, gn_speaker);
    
    if (elem)
	return gs_list;

    gs_list = g_slist_append (gs_list, g_strdup (gn_speaker));
					    
    return gs_list;
}

GnopernicusSpeakerListType*
spconf_gnopernicus_speakers_remove (GnopernicusSpeakerListType *gs_list,
				    const gchar *gn_speaker)
{
    GSList *elem = NULL;
    
    sru_return_val_if_fail (gs_list, NULL);
    sru_return_val_if_fail (gn_speaker, gs_list);

    elem = gs_list;
    
    while (elem)
    {
	if (elem->data && !strcmp (gn_speaker, (gchar*)elem->data))
	{
	    gs_list = g_slist_remove_link (gs_list, elem);
	    
	    spconf_gnopernicus_speakers_save (gs_list);
	    
	    g_free (elem->data);
	    g_slist_free (elem);
	    elem = NULL;
	    break;
	}
	elem = elem->next;
    }
    
    return gs_list;
}


GnopernicusSpeakerListType*
spconf_gnopernicus_speakers_find (GnopernicusSpeakerListType *gs_list,
				  const gchar *gn_speaker)
{   
    GnopernicusSpeakerListType *elem;

    sru_return_val_if_fail (gn_speaker, NULL);    
    
    if (!gs_list)
	return NULL;
        
    for (elem = gs_list; elem ; elem = elem->next)
    {
	if (!strcmp(elem->data, gn_speaker))
	    return elem;
    }
    
    return NULL;
}


GnopernicusSpeakerListType*
spconf_gnopernicus_speakers_load (GnopernicusSpeakerListType *gs_list,
				  SpeakerSettingsListType   **speakers_settings)
{
    GnopernicusSpeakerListType *tmp_list = NULL;
    GConfValueType type;
    
    gs_list = spconf_gnopernicus_speakers_free (gs_list);
	
    tmp_list = gnopiconf_get_list_with_default (
		CONFIG_PATH SPEECH_VOICE_PARAM_KEY_PATH SPEECH_GNOPERNICUS_SPEAKERS,
		NULL, &type);
		
    if (!tmp_list)
    {
	gint iter;
	for (iter = 0 ; iter < G_N_ELEMENTS(speakers); iter++)
	    gs_list = g_slist_append (gs_list, g_strdup (speakers[iter].gnopernicus_speaker));

	spconf_gnopernicus_speakers_save (gs_list);
	
	*speakers_settings =
	    spconf_speaker_settings_list_load_default ();
    }
    else
    {
	gs_list = tmp_list;
	*speakers_settings =
	    spconf_speaker_settings_list_load_from_gconf (gs_list);
    }
    
    return gs_list;
}

void
spconf_gnopernicus_speakers_save ( GnopernicusSpeakerListType *gs_list)
{    
    if (gs_list)
	gnopiconf_set_list (gs_list, GCONF_VALUE_STRING,
			    CONFIG_PATH SPEECH_VOICE_PARAM_KEY_PATH SPEECH_GNOPERNICUS_SPEAKERS);
			    
}


/*****************************************************************************/
SpeakerSettings *
spconf_speaker_settings_load (const gchar *key)
{
    GSList *list = NULL;
    GConfValueType type;
    SpeakerSettings *speaker = NULL;
    gchar *path = NULL;
	
    path = g_strconcat (CONFIG_PATH, SPEECH_VOICE_PARAM_KEY_PATH, key, NULL);
    
    sru_return_val_if_fail (path, NULL);    
    
    list = gnopiconf_get_list_with_default (path, NULL, &type);
    g_free (path);
		
    if (type == GCONF_VALUE_STRING && list)
    {
	    GSList *iter = list;
	    speaker = spconf_speaker_settings_new ();
	    
	    if (speaker)
	    {
		speaker->gnopernicus_speaker = g_strdup (key);	    
		speaker->driver_name 	     = iter->data;
		speaker->driver_name_descr   = spconf_driver_list_driver_descr_name_get (iter->data);
		iter->data = NULL;
		iter = iter->next;
		
		if (iter && iter->data)
		{
		    speaker->voice_name       = iter->data;
		    speaker->voice_name_descr = spconf_driver_list_voice_descr_name_get (iter->data);
		    iter->data = NULL;
		    iter = iter->next;
		    
		    if (iter && iter->data)
		    {
			speaker->volume = atoi((gchar*)iter->data);
			iter = iter->next;
			
			if (iter && iter->data)
			{
			    speaker->rate   = atoi((gchar*)iter->data);
			    iter = iter->next;
			    
			    if (iter && iter->data)
			        speaker->pitch  = atoi((gchar*)iter->data);
			}
		    }
		}		    
	    }
		
        list = spconf_free_slist_with_char_data (list);		
    }
    
    return speaker;
}

void
spconf_speaker_settings_save (SpeakerSettings *speaker_settings)
{
    gchar *path;
    GSList *list = NULL;

    path = g_strdup_printf ("%s%s", CONFIG_PATH SPEECH_VOICE_PARAM_KEY_PATH,
			    speaker_settings->gnopernicus_speaker);
    				    
    sru_assert (path);    

    list = spconf_create_speaker_item (speaker_settings);

    gnopiconf_set_list (list, GCONF_VALUE_STRING, path);
    
    g_free (path);	

    list = spconf_free_speaker_item (list);
}

SpeakerSettings*
spconf_speaker_settings_new (void)
{
    SpeakerSettings *new_speaker_table = NULL ; 
    
    new_speaker_table = (SpeakerSettings *) g_new0 (SpeakerSettings, 1) ; 
        
    if (!new_speaker_table) 
	sru_error (_("Unable to allocate memory."));
    
    return new_speaker_table ; 
}


SpeakerSettings*
spconf_speaker_settings_clear (SpeakerSettings *speaker_settings)
{
    if (!speaker_settings) 
	return NULL;
	
    g_free (speaker_settings->gnopernicus_speaker);
    g_free (speaker_settings->driver_name);
    g_free (speaker_settings->driver_name_descr);
    g_free (speaker_settings->voice_name);
    g_free (speaker_settings->voice_name_descr);
    
    return speaker_settings;
}

SpeakerSettings*
spconf_speaker_settings_free (SpeakerSettings *speaker_settings)
{
    if (!speaker_settings) 
	return NULL;
	    
    speaker_settings = spconf_speaker_settings_clear (speaker_settings);
    g_free (speaker_settings);
    speaker_settings = NULL;
    
    return speaker_settings;
}

SpeakerSettings*
spconf_speaker_settings_copy (SpeakerSettings *dest, 
			      SpeakerSettings *source)
{
    sru_return_val_if_fail (source, NULL);

    spconf_speaker_settings_clear (dest);
	
    sru_return_val_if_fail (dest, NULL);
    
    dest->gnopernicus_speaker 	= g_strdup (source->gnopernicus_speaker);
    dest->driver_name    	= g_strdup (source->driver_name);
    dest->driver_name_descr    	= g_strdup (source->driver_name_descr);
    dest->voice_name 		= g_strdup (source->voice_name);
    dest->voice_name_descr	= g_strdup (source->voice_name_descr);
    dest->volume     		= source->volume;
    dest->rate       		= source->rate;
    dest->pitch      		= source->pitch;
    
    return dest;
}

/******************************************************************************/
SpeakerSettingsListType*
spconf_speaker_settings_list_clone (SpeakerSettingsListType *sp_list)
{
    SpeakerSettingsListType *elem;
    SpeakerSettingsListType *clone = NULL;
    
    sru_return_val_if_fail (sp_list, NULL);
    
    for (elem = sp_list; elem; elem = elem->next)
    {
	SpeakerSettings *item = (SpeakerSettings*)elem->data;
	SpeakerSettings *new_item = NULL;
	
	new_item = spconf_speaker_settings_new ();
	
	if (new_item)
	{
	    new_item->gnopernicus_speaker = g_strdup (item->gnopernicus_speaker);
	    new_item->driver_name 	  = g_strdup (item->driver_name);
	    new_item->driver_name_descr   = g_strdup (item->driver_name_descr);
	    new_item->voice_name 	  = g_strdup (item->voice_name);
	    new_item->voice_name_descr    = g_strdup (item->voice_name_descr);
	    new_item->volume     	  = item->volume;
	    new_item->rate       	  = item->rate;
	    new_item->pitch      	  = item->pitch;
	    clone = g_slist_append (clone, new_item);
	}	
    }
    return clone;
}

SpeakerSettingsListType*
spconf_speaker_settings_list_find (SpeakerSettingsListType *sp_list,
		    	        const gchar *gnopernicus_speaker)
{
    GSList *elem = NULL;
    
    if (!sp_list)
	return NULL;
    sru_return_val_if_fail (gnopernicus_speaker, NULL);
    
    for (elem = sp_list; elem ; elem = elem->next)
    {
	SpeakerSettings *item = (SpeakerSettings*)elem->data;
	if (item)
	{
	    if (!strcmp (gnopernicus_speaker, 
			 item->gnopernicus_speaker))
		return elem;
	}
    }
    
    return NULL;
}

SpeakerSettingsListType*
spconf_speaker_settings_list_add (SpeakerSettingsListType *sp_list, 
			          const gchar 	*gn_speaker,
			          const gchar 	*driver_name,
			          const gchar 	*voice_name,
			    	  gint  	volume,
			          gint  	rate,
			    	  gint  	pitch)
{
    SpeakerSettings *speaker = NULL;
    SpeakerSettingsListType *elem = NULL;
    
    sru_return_val_if_fail (voice_name, sp_list);
    sru_return_val_if_fail (gn_speaker, sp_list);

	
    elem = spconf_speaker_settings_list_find (sp_list, gn_speaker);
    
    if (elem)
	return sp_list;
    					    
    speaker = spconf_speaker_settings_new ();
	
    sru_return_val_if_fail (speaker, sp_list);

    speaker->gnopernicus_speaker = g_strdup (gn_speaker);
    speaker->driver_name 	 = g_strdup (driver_name);
    speaker->driver_name_descr 	 = spconf_driver_list_driver_descr_name_get (driver_name);
    speaker->voice_name 	 = g_strdup (voice_name);
    speaker->voice_name_descr	 = spconf_driver_list_voice_descr_name_get (voice_name); 
    speaker->volume     	 = volume;
    speaker->rate       	 = rate;
    speaker->pitch      	 = pitch;
    
    sp_list = g_slist_append (sp_list, speaker);

    return sp_list;
}

/* DEPRECATED */
SpeakerSettingsListType*
spconf_speaker_settings_list_copy (SpeakerSettingsListType *dest, 
				   SpeakerSettingsListType *source)
{
    SpeakerSettings *speaker_dest = NULL;
    SpeakerSettings *speaker_source = NULL;
    
    sru_return_val_if_fail (dest, NULL);
    sru_return_val_if_fail (source, NULL);
	
    speaker_dest   = (SpeakerSettings*) dest->data;
    speaker_source = (SpeakerSettings*) source->data;

    speaker_dest =
	spconf_speaker_settings_copy (speaker_dest, 
			              speaker_source);
    return dest;
}



SpeakerSettingsListType*
spconf_speaker_settings_list_remove (SpeakerSettingsListType *sp_list,
				     const gchar *gn_speaker)
{
    SpeakerSettingsListType *elem = NULL;
    
    sru_return_val_if_fail (sp_list, 	sp_list);
    sru_return_val_if_fail (gn_speaker, sp_list);
    
    elem = sp_list;
    
    while (elem)
    {
	SpeakerSettings *item = (SpeakerSettings*)elem->data;
	if (item)
	{
	    if (!strcmp (gn_speaker, item->gnopernicus_speaker))
	    {
		gchar *path = NULL;
		
		sp_list = g_slist_remove_link (sp_list, elem);
		    
		if (strcmp (NONE_ELEMENT, item->gnopernicus_speaker))
		{
		    path = g_strdup_printf ("%s%s", CONFIG_PATH SPEECH_VOICE_PARAM_KEY_PATH,
					    item->gnopernicus_speaker);
		    
		    gnopiconf_unset_key (path);
		    
		    spconf_speech_voice_removed (item->gnopernicus_speaker);
		    
		    g_free (path);
		}
		
		elem->data = 
		    spconf_speaker_settings_free ((SpeakerSettings*)elem->data);
		    
		g_slist_free (elem);
		elem = NULL;
		break;
	    }
	}
	elem = elem->next;
    }
    return sp_list;
}


SpeakerSettingsListType*
spconf_speaker_settings_list_free (SpeakerSettingsListType *sp_list)
{
    SpeakerSettingsListType *elem = sp_list;
    
    if (!sp_list) 
	return NULL;
    
    while (elem)
    {
        elem->data = 
		spconf_speaker_settings_free ((SpeakerSettings*)elem->data);
		
	elem = elem->next;
    }
	
    g_slist_free (sp_list);
    sp_list = NULL;
    
    return sp_list;
}

SpeakerSettingsListType *
spconf_speaker_settings_list_load_default (void)
{
    SpeakerSettingsListType   *speakers_settings = NULL;
    gint iter;
    
    for (iter = 0 ; iter < G_N_ELEMENTS (speakers); iter++)
    {
	SpeakerSettings *speaker = spconf_speaker_settings_new ();
	
	if (speaker)
	{
	    speaker->gnopernicus_speaker = g_strdup (speakers[iter].gnopernicus_speaker);
	    speaker->driver_name 	 = g_strdup (speakers[iter].driver_name);
	    speaker->driver_name_descr 	 = g_strdup (speakers[iter].driver_name_descr);
	    speaker->voice_name 	 = g_strdup (speakers[iter].voice_name);
	    speaker->voice_name_descr	 = g_strdup (speakers[iter].voice_name_descr);
	    speaker->volume 		 = speakers[iter].volume;
	    speaker->rate                = speakers[iter].rate;
	    speaker->pitch               = speakers[iter].pitch;
	    
	    spconf_speaker_settings_save (speaker);
	    
	    speakers_settings = 
		g_slist_append (speakers_settings, speaker);
	}
    }
    
    return speakers_settings;
}

SpeakerSettingsListType *
spconf_speaker_settings_list_load_from_gconf (GnopernicusSpeakerListType *gnopernicus_speakers)
{
    SpeakerSettingsListType *elem = NULL;
    SpeakerSettingsListType  *speakers_settings = NULL;

    for (elem = gnopernicus_speakers; elem ; elem = elem->next)
    {
	SpeakerSettings *speaker =
	    spconf_speaker_settings_load ((gchar*)elem->data);
	    
	if (speaker)
	{
	    speakers_settings = 
			g_slist_append (speakers_settings, speaker);
	}
	else
	    continue;
    }

    return speakers_settings;
}

void
spconf_speaker_settings_list_save (SpeakerSettingsListType *sp_list)
{
    SpeakerSettingsListType *elem = NULL;
        
    sru_return_if_fail (sp_list);
    
    for (elem = sp_list; elem ; elem = elem->next)
    {
	SpeakerSettings *item = (SpeakerSettings*)elem->data;
	
	if (item)
	    spconf_speaker_settings_save (item);
    }
}


/******************************************************************************/
GSList*
spconf_free_slist_with_char_data (GSList *list)
{
    GSList *elem = NULL;
    
    if (!list) 
	return NULL;
    
    for (elem = list; elem ; elem = elem->next)
	g_free ((gchar*)elem->data);
	
    g_slist_free (list);
    list = NULL;
    
    return list;
}


GList*
spconf_driver_list_for_combo_get (SpeechDriverListType *speech_driver_list,
				    const gchar *item)
{
    SpeechDriverListType *tmp;
    GList 		 *driver_list = NULL;
    
    if (item)
	driver_list = g_list_append (driver_list, g_strdup (item));
	
    for (tmp = speech_driver_list; tmp ; tmp = tmp->next)
    {
    	SpeechDriver *driver = (SpeechDriver *)tmp->data;
	
	driver_list = 
	    g_list_append (driver_list, g_strdup (driver->driver_name_descr));
    }
    
    return driver_list;
}

GList*
spconf_voice_list_for_combo_get (SpeechDriverListType	*speech_driver_list,
				 const gchar *driver,
				 const gchar *item)
{
    SpeechDriverListType *tmp;
    const SpeechDriver	 *sdriver;
    GList 		 *voice_list = NULL;
    
    if (item)
	voice_list = g_list_append (voice_list, g_strdup (item));

    sdriver =
	spconf_driver_list_get_driver_from_name_descr (speech_driver_list,
							driver);

    if (!sdriver)
	return voice_list;

    for (tmp = sdriver->driver_voice_list; tmp ; tmp = tmp->next)
    {
	SpeechVoice *speech_voice = (SpeechVoice*)tmp->data; 
	
	voice_list = 
	    g_list_append (voice_list, g_strdup (speech_voice->voice_name_descr));
    }
    
    return voice_list;
}

void
spconf_list_for_combo_free (GList *list)
{
    GList *tmp_list = NULL;
    
    for (tmp_list = g_list_first (list); tmp_list ; tmp_list = tmp_list->next)
	g_free (tmp_list->data);

    g_list_free (list);
}

GSList*
spconf_create_speaker_item (SpeakerSettings *item)
{
    GSList *list = NULL;
    
    sru_return_val_if_fail (item, NULL);
    
    list = g_slist_append (list, g_strdup (item->driver_name));  
    list = g_slist_append (list, g_strdup (item->voice_name));   
    list = g_slist_append (list, g_strdup_printf ("%d", item->volume));
    list = g_slist_append (list, g_strdup_printf ("%d", item->rate));
    list = g_slist_append (list, g_strdup_printf ("%d", item->pitch));
    
    return list;
}


GSList*
spconf_free_speaker_item (GSList *item)
{
     return spconf_free_slist_with_char_data (item);
}

void
spconf_set_default_voices (gboolean force)
{
    GnopernicusSpeakerListType *list = NULL;
    GnopernicusSpeakerListType *gs = NULL;
    GConfValueType type;
    gint iter;
    
    gs = spconf_gnopernicus_speakers_free (gs);

    list = gnopiconf_get_list_with_default (
		CONFIG_PATH SPEECH_VOICE_PARAM_KEY_PATH SPEECH_GNOPERNICUS_SPEAKERS,
		NULL, &type);
	
    if (!list || force)
    {
	if (force && list)
	    list = spconf_free_speaker_item (list);
	    
	for (iter = 0 ; iter < G_N_ELEMENTS(speakers); iter++)
	{
	    SpeakerSettings *speaker = spconf_speaker_settings_new ();
	    
	    gs = g_slist_append (gs, g_strdup (speakers[iter].gnopernicus_speaker));
	    
	    if (speaker)
	    {
		speaker->gnopernicus_speaker 	= g_strdup (speakers[iter].gnopernicus_speaker);
		speaker->driver_name 		= g_strdup (speakers[iter].driver_name);
		speaker->driver_name_descr	= g_strdup (speakers[iter].driver_name_descr);
		speaker->voice_name 		= g_strdup (speakers[iter].voice_name);
		speaker->voice_name_descr	= g_strdup (speakers[iter].voice_name_descr);
		speaker->volume     		= speakers[iter].volume;
		speaker->rate       		= speakers[iter].rate;
		speaker->pitch      		= speakers[iter].pitch;

		spconf_speaker_settings_save (speaker);	    
		
		spconf_speaker_settings_free (speaker);
	    }
	}

	spconf_gnopernicus_speakers_save (gs);
	gs = spconf_free_speaker_item (gs);
    }
    else
	list = spconf_free_speaker_item (list);
    
}

/*****************************************************************************/
DictionaryListType*
spconf_dictionary_load_default (void)
{
    DictionaryListType *list = NULL;
    
    list = g_slist_append (list, g_strdup (DEFAULT_SPEECH_DICTIONARY_ENTRY));
    
    spconf_dictionary_save (list);
    
    return list;
}

DictionaryListType*
spconf_dictionary_load (void)
{
    DictionaryListType *list = NULL;
    GConfValueType type;
    
    list = gnopiconf_get_list_with_default (CONFIG_PATH SPEECH_DICTIONARY_PATH SPEECH_DICTIONARY_LIST, 
				    	    NULL, &type);
    
    if (!list)
	list = spconf_dictionary_load_default ();
	
    return list;
}

gboolean
spconf_dictionary_save (DictionaryListType *list)
{
    sru_return_val_if_fail (list, FALSE);

    if (!gnopiconf_set_list (list, 
			    GCONF_VALUE_STRING, 
			    CONFIG_PATH SPEECH_DICTIONARY_PATH SPEECH_DICTIONARY_LIST))
	return FALSE;
	
    spconf_dictionary_changes ();
    
    return TRUE;
}

DictionaryListType*
spconf_dictionary_free (DictionaryListType *list)
{
    DictionaryListType *elem = NULL;
    
    if (!list)
	return NULL;
	
    for (elem = list; elem ; elem = elem->next)
    {
	g_free ((gchar*)elem->data);
	elem->data = NULL;
    }
    
    g_slist_free (list);
    list = NULL;
    
    return list;
}

gboolean
spconf_dictionary_split_entry (const gchar *entry,
			       gchar **word,
			       gchar **replace)
{
    gchar *separator = NULL;
    
    *word = NULL;
    *replace = NULL;

    sru_return_val_if_fail (entry, FALSE);
    
    separator = g_utf8_strchr (entry, -1, '<');
    
    if (separator && *(g_utf8_find_next_char (separator, NULL)) == '>')
    {
	*word     = g_strndup (entry, separator - entry);
	separator = g_utf8_find_next_char (separator, NULL);
	*replace  = g_strdup (g_utf8_find_next_char (separator,NULL));
	
	return TRUE;
    }
    
    return FALSE;
}

gchar*
spconf_dictionary_create_entry (const gchar *word,
			        const gchar *replace)
{
    sru_return_val_if_fail (word, NULL);
    sru_return_val_if_fail (replace, NULL);
    
    return g_strdup_printf ("%s<>%s", word, replace);
}

DictionaryListType*
spconf_dictionary_find_word (DictionaryListType *list,
			    const gchar *word)
{
    DictionaryListType *elem;

    sru_return_val_if_fail (word, NULL);
    sru_return_val_if_fail (list, NULL);
	
    for (elem = list; elem ; elem = elem->next)
    {
	gchar *word1 = NULL;
	gchar *word2 = NULL;

	spconf_dictionary_split_entry ((gchar*)elem->data, &word1, &word2);
	
	if (!strcmp (word1, word))
	{
	    g_free (word1);
	    g_free (word2);
	    return elem;
	}
	
	g_free (word1);
	g_free (word2);
    }
    
    return elem;
}

DictionaryListType*
spconf_dictionary_modify_word (DictionaryListType *list,
			       const gchar *word,
			       const gchar *replace)
{
    DictionaryListType *elem = NULL;
    sru_return_val_if_fail (replace, list);
    sru_return_val_if_fail (word, list);
    sru_return_val_if_fail (list, list);

    if ((elem = spconf_dictionary_find_word (list, word)) == NULL)
	return list;

    g_free (elem->data);
    elem->data = 
	spconf_dictionary_create_entry (word, replace);
    
    return list;
}

DictionaryListType*
spconf_dictionary_add_word (DictionaryListType *list,
			    const gchar *word,
			    const gchar *replace)
{
    sru_return_val_if_fail (replace, list);
    sru_return_val_if_fail (word, list);
    sru_return_val_if_fail (list, list);

    if (spconf_dictionary_find_word (list, word))
	return list;

    list = g_slist_append (list,
	    spconf_dictionary_create_entry (word, replace));
	    
    return list;
}

DictionaryListType*
spconf_dictionary_remove_word (DictionaryListType *list,
			      const gchar *word)
{
    DictionaryListType *elem = NULL;
    
    sru_return_val_if_fail (word, list);
    sru_return_val_if_fail (list, list);

    if ((elem = spconf_dictionary_find_word (list, word)) == NULL)
	return list;
	
    list = g_slist_remove_link (list, elem);
    
    spconf_dictionary_free (elem);
    
    return list;
}

/*****************************************************************************/
/**
 *
 * Speech settings callback function
 * <client> - gconf client
 * <cnxn_id> - notify id
 * <entry> - entry structure
 * <user_data> - not used
 *
**/
void
spconf_changes_cb			(GConfClient	*client,
					guint		cnxn_id,
					GConfEntry	*entry,
					gpointer	user_data)
{        
    gchar *path = NULL;

    if (!entry || 
	!entry->value)
	return;
	    
    sru_debug  ("spconf:spconf_changes_cb:Entry value type:%i",entry->value->type);
    switch (entry->value->type)
    {
    	case GCONF_VALUE_INT:    sru_debug ("spconf:spconf_changes_cb:Entry value:%in",gconf_value_get_int(entry->value));break;
    	case GCONF_VALUE_FLOAT:  sru_debug ("spconf:spconf_changes_cb:Entry value:%f",gconf_value_get_float(entry->value));break;
    	case GCONF_VALUE_STRING: sru_debug ("spconf:spconf_changes_cb:Entry value:%s",gconf_value_get_string(entry->value));break;
    	case GCONF_VALUE_BOOL:   sru_debug ("spconf:spconf_changes_cb:Entry value:%i",gconf_value_get_bool(entry->value));break;
	default:break;
    }

    path = g_path_get_basename ((gchar*) gconf_entry_get_key (entry));

    spui_voice_settings_gconf_callback (path);
    
    g_free (path);
}
