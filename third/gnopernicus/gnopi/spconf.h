/* spconf.h
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

#ifndef __SPEECH__
#define __SPEECH__ 0

#include <glib.h>
#define NONE_ELEMENT 			"<none>"

typedef GSList	GnopernicusSpeakerListType; /*data type is gchar*/
typedef GSList  SpeakerSettingsListType;    /*data type is SpeakerSettings */
typedef GSList	DictionaryListType;	    /*data type is gchar*/
typedef GSList  SpeechDriverListType;	    /*data type is Driver*/

typedef struct
{
    gchar *voice_name;			/* voice name */
    gchar *voice_name_descr;		/* voice name what is showed in UI */
}SpeechVoice;

typedef struct
{
    gchar 	*driver_name;		/* driver name */
    gchar 	*driver_name_descr;	/* driver name what is showed in UI */
    GSList	*driver_voice_list;	/* list of voices */
}SpeechDriver;

typedef struct
{
/* Gnopernicus speaker(voice) name */
    gchar 	*gnopernicus_speaker; /* gnopernicus voice name */

/* Gnome_speech driver name and the selected speaker(voice)*/
    gchar 	*driver_name;	    /* driver name */
    gchar 	*driver_name_descr; /* driver name what is showed in UI */
    gchar 	*voice_name;        /* voice name */
    gchar 	*voice_name_descr;  /* voice name what is showed in UI */

/* Parameters for voice_name */
    gint  	volume;		
    gint  	rate;
    gint  	pitch;        
}SpeakerSettings;

typedef struct
{
    gchar	*count_type;
    gchar 	*punctuation_type;
    gchar 	*text_echo_type;
    gchar	*modifiers_type;
    gchar	*cursors_type;
    gchar	*spaces_type;
    gchar	*dictionary;
}Speech;


gboolean 	spconf_gconf_client_init 	(void);
Speech* 	spconf_setting_init 		(gboolean set_struct);
Speech* 	spconf_setting_new 		(void);
void 		spconf_load_default_settings 	(Speech* speech);
void 		spconf_setting_clean 		(Speech* speech);
void 		spconf_setting_free 		(Speech* speech);
void 		spconf_terminate		(Speech *speech);

/**
 * Set Methods 
**/
void 		spconf_setting_set 		(const Speech *speech);
void 		spconf_count_set 		(const gchar *count);
void 		spconf_punctuation_set 		(const gchar *punctuation);
void 		spconf_text_echo_set 		(const gchar *text_echo);
void 		spconf_modifiers_set 		(const gchar *modifiers);
void 		spconf_cursors_set 		(const gchar *cursors);
void 		spconf_spaces_set 		(const gchar *spaces);

void 		spconf_dictionary_set 		(const gchar *dictionary);
void		spconf_dictionary_changes 	(void);

/**
 * Get Methods 						
**/
gchar* 		spconf_count_get		(void);
gchar*		spconf_punctuation_get		(void);
gchar*		spconf_text_echo_get		(void);
gchar*		spconf_modifiers_get		(void);
gchar*		spconf_cursors_get		(void);
gchar*		spconf_spaces_get		(void);
gchar*		spconf_dictionary_get		(void);


SpeechDriverListType*	spconf_driver_list_init (void);
SpeechDriverListType*	spconf_driver_list_free (SpeechDriverListType *driver_list);

gchar*		    spconf_driver_list_voice_descr_name_get (const gchar *name);
const gchar*	    spconf_driver_list_get_driver_name (SpeechDriverListType *driver_list,
							const gchar *driver_name_descr);
const gchar*	    spconf_driver_list_get_driver_name_descr (SpeechDriverListType *driver_list,
							      const gchar *driver_name);
const SpeechDriver* spconf_driver_list_get_driver_from_name_descr (SpeechDriverListType *driver_list,
							     const gchar *driver_name_descr);
const SpeechDriver* spconf_driver_list_get_driver (SpeechDriverListType *driver_list,
			    			   const gchar *driver_name);
const SpeechDriver* spconf_driver_list_get_driver_by_voice (SpeechDriverListType *driver_list,
			    				    const gchar *voice_name);
const gchar*	    spconf_driver_list_get_voice_name (SpeechDriverListType *driver_list,
						       const gchar *driver_name,
			    			       const gchar *voice_name_descr);
const SpeechVoice*  spconf_driver_list_get_speech_voice (SpeechDriverListType *driver_list,
				    			const gchar *driver_name,
			    	    			const gchar *voice_name_descr);





GnopernicusSpeakerListType* spconf_gnopernicus_speakers_clone (GnopernicusSpeakerListType *gs_list);
GnopernicusSpeakerListType* spconf_gnopernicus_speakers_free (GnopernicusSpeakerListType *gs_list);
GnopernicusSpeakerListType* spconf_gnopernicus_speakers_add (GnopernicusSpeakerListType *gs_list,
				    			     const gchar *gn_speaker);
GnopernicusSpeakerListType* spconf_gnopernicus_speakers_remove (GnopernicusSpeakerListType *gs_list,
								const gchar *gn_speaker);
GnopernicusSpeakerListType* spconf_gnopernicus_speakers_find (GnopernicusSpeakerListType *gs_list,
				    			      const gchar *gn_speaker);
GnopernicusSpeakerListType* spconf_gnopernicus_speakers_load (GnopernicusSpeakerListType *gs_list,
							      SpeakerSettingsListType   **speakers_settings);
void			    spconf_gnopernicus_speakers_save (GnopernicusSpeakerListType *gs_list);
void			    spconf_gnopernicus_speakers_debug (GnopernicusSpeakerListType *gs_list);



SpeakerSettings*	spconf_speaker_settings_new (void);
SpeakerSettings*	spconf_speaker_settings_free (SpeakerSettings *speaker_settings);
void			spconf_speaker_settings_save (SpeakerSettings *speaker_settings);
SpeakerSettings*	spconf_speaker_settings_load (const gchar *key);
SpeakerSettings*	spconf_speaker_settings_copy (SpeakerSettings *dest, 
			    		    	      SpeakerSettings *source);
						      
SpeakerSettingsListType* spconf_speaker_settings_list_clone (SpeakerSettingsListType *sp_list);
SpeakerSettingsListType* spconf_speaker_settings_list_copy (SpeakerSettingsListType *dest, 
							    SpeakerSettingsListType *source);
SpeakerSettingsListType* spconf_speaker_settings_list_add (SpeakerSettingsListType *sp_list, 
			    			const gchar 	*gn_speaker,
						const gchar 	*driver_name,
			    			const gchar 	*voice_name,
			    			gint  		volume,
			    			gint  		rate,
			    			gint  		pitch);
SpeakerSettingsListType* spconf_speaker_settings_list_remove (SpeakerSettingsListType *sp_list,
							      const gchar *gn_speaker);
SpeakerSettingsListType* spconf_speaker_settings_list_free (SpeakerSettingsListType *sp_list);    
SpeakerSettingsListType* spconf_speaker_settings_list_load_default (void);
SpeakerSettingsListType* spconf_speaker_settings_list_load_from_gconf (GnopernicusSpeakerListType *gnopernicus_speakers);
void			 spconf_speaker_settings_list_save (SpeakerSettingsListType *sp_list);
SpeakerSettingsListType* spconf_speaker_settings_list_find (SpeakerSettingsListType *sp_list,
		    	    				    const gchar *gnopernicus_speaker);

GSList*		spconf_free_slist_with_char_data 	(GSList *list);

GList*		spconf_driver_list_for_combo_get 	(SpeechDriverListType	*speech_driver_list,
							const gchar *item);
GList*		spconf_voice_list_for_combo_get 	(SpeechDriverListType	*speech_driver_list,
							const gchar *driver,
							const gchar *item);
void		spconf_list_for_combo_free 		(GList *list);


GSList*		spconf_create_speaker_item 		(SpeakerSettings *item);
GSList*		spconf_free_speaker_item 		(GSList *item);
void		spconf_speaker_remove 			(const gchar *gn_speaker);
void		spconf_play_voice 			(const gchar *gnopernicus_speaker);
void		spconf_set_default_voices 		(gboolean force);
void		spconf_speech_voice_removed 		(const gchar *voice);


DictionaryListType*	spconf_dictionary_load (void);
gboolean		spconf_dictionary_save (DictionaryListType *list);
DictionaryListType*	spconf_dictionary_free (DictionaryListType *list);
gboolean		spconf_dictionary_split_entry (const gchar *entry,
			    				gchar **word,
			    				gchar **replace);
gchar*			spconf_dictionary_create_entry (const gchar *word,
							const gchar *replace);
DictionaryListType*	spconf_dictionary_find_word (DictionaryListType *list,
						    const gchar *word);
DictionaryListType*	spconf_dictionary_modify_word (DictionaryListType *list,
			    			    const gchar *word,
			    			    const gchar *replace);
DictionaryListType*	spconf_dictionary_add_word (DictionaryListType *list,
						    const gchar *word,
						    const gchar *replace);
DictionaryListType*	spconf_dictionary_remove_word (DictionaryListType *list,
			    			    const gchar *word);
DictionaryListType*	spconf_dictionary_load_default (void);

#endif
