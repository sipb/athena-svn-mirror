 /* srspc.c
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
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <ctype.h>

#include "srmain.h"
#include "srspc.h"
#include "srs-xml.h"
#include "SREvent.h"
#include "srpres.h"
#include "libsrconf.h"
#include "srintl.h"

#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "src-speech-in.h"

#define STR(X) (X == NULL? "" : X)

static gboolean src_speech_initialized     = FALSE;

static GHashTable *src_dictionary_hash 	= NULL;

typedef struct 
{
    SRCSpeechCountMode     	  count_mode;
    SRCSpeechSpellingMode      	  spelling_mode;
    SRCSpeechTextEchoType  	  text_echo_type;
    SRCSpeechModifiersEchoType 	  modifiers_echo_type;
    SRCSpeechSpacesEchoType    	  spaces_echo_type;
    SRCSpeechCursorsEchoType   	  cursors_echo_type;
    SRCSpeechDictionaryType    	  dictionary_type;
    
}SRCSpeech;

typedef struct
{
     gchar 	**engine_voices;
     gchar 	**engine_drivers;
     gchar 	**voices;

}SRCVoices;

typedef struct
{
    gchar *voice;
    gchar *driver;
    gchar *id;
    gchar *rate;
    gchar *pitch;
    gchar *volume;
}SRCVoice;

typedef struct _SRCSOut
{
    gchar *chunk;
    SRCSpeechPriority priority;
    gboolean preempt;
    gchar *ido;
}SRCSOut;

static GPtrArray     	*src_voice_params = NULL;
SRCSpeech 	 	*src_speech = NULL;
static SRCVoices 	*src_voices = NULL;

static gboolean src_speech_idle = TRUE;
static GSList *srcs_outs = NULL;
static SRCSOut *last_out = NULL;
#ifdef SRS_NO_MARKERS_SUPPORTED
static gboolean src_speech_callback = TRUE;
static GSList  *src_speech_unspoken = NULL;
#endif


static SRCSOut*
srcs_out_new ()
{
    return g_new0 (SRCSOut, 1);
}

static void
srcs_out_terminate (SRCSOut *out)
{
    sru_assert (out);
    g_free (out->chunk);
    g_free (out->ido);
    g_free (out);
}


static gboolean
src_speech_valid_engine_voice ( gchar *driver,
				gchar *voice)
{
    gint i;
    gboolean valid_driver = FALSE;

    sru_assert (src_speech_initialized);
    sru_assert (src_voices->engine_voices);
    sru_assert (src_voices->engine_drivers);
    sru_assert (voice);

    for (i = 0; src_voices->engine_drivers[i]; i++)
    {
	if (strcmp (driver, src_voices->engine_drivers[i]) == 0)
	    valid_driver = TRUE;
    }

    if (!valid_driver)
	return FALSE;
	
    for (i = 0; src_voices->engine_voices[i]; i++)
    {
	if (strcmp (voice, src_voices->engine_voices[i]) == 0)
	    return TRUE;
    }
    return FALSE;
}


static gchar**
src_speech_get_voices ()
{
    gchar **rv = NULL;
    gint cnt, index;
    GSList *voices = NULL;

    sru_assert (src_speech_initialized);

    srconf_get_data_with_default (SPEECH_GNOPERNICUS_SPEAKERS, CFGT_LIST,
				&voices, NULL, SPEECH_PARAMETER_SECTION);

    cnt = voices ? g_slist_length (voices) + 2 : 2;
    rv = g_new (gchar*, cnt);
    index = 0;

    if (voices)
    {
	GSList *crt = NULL;
	for (crt = voices; crt; crt = crt->next)
	{
	    gchar *voice = crt->data;
	    if (voice && voice[0])
	    {
		rv[index] = voice;
		crt->data = NULL;
/*		 fprintf (stderr, "\nGNOPERNICUS VOICE[%d]=%s", index, rv[index]);*/
	        index++;
	    }
	}
    }
    g_slist_free (voices);
    rv[index] = g_strdup ("default");
    rv[index + 1] = NULL;

    return rv;
}


static gboolean
src_speech_valid_voice (gchar *voice)
{
    gint i;

    sru_assert (src_speech_initialized);
    sru_assert (src_voices->voices);
    sru_assert (voice);

    for (i = 0; src_voices->voices[i]; i++)
    {
	if (strcmp (voice, src_voices->voices[i]) == 0)
	    return TRUE;
    }    
    return FALSE;
}


#define src_speech_set_string_from_list_element(list_el, string)	      \
	    {								      \
		if (list_el && list_el->data && ((gchar*)(list_el->data))[0]) \
		{							      \
		    *string = list_el->data;				      \
		    list_el->data = NULL;				      \
		}							      \
		else							      \
		{							      \
		    *string = NULL;					      \
		}							      \
	    }
static SRCVoice* 
src_voice_new ()
{
    return g_new0 (SRCVoice, 1);
}

static gboolean
src_speech_get_voice_parameters_from_gconf (gchar *voice)
{
    GSList *voices = NULL;
    SRCVoice *voice_ = NULL;

    sru_assert (src_speech_initialized);
    sru_assert (voice);

    voice_ = src_voice_new ();
    srconf_get_data_with_default (voice, CFGT_LIST, &voices, 
				  NULL, SPEECH_PARAMETER_SECTION);
    
    if (voices)
    {
	GSList *crt = voices;
	
	voice_->voice  = g_strdup (voice);
	voice_->driver = g_strdup (crt->data);
	if (crt)
	    crt = crt->next;
	voice_->id = g_strdup (crt->data);

	if (crt)
	    crt = crt->next;
	voice_->volume = g_strdup (crt->data);

	if (crt)
	    crt = crt->next;
	voice_->rate = g_strdup (crt->data);
	
	if (crt)
	    crt = crt->next;
	voice_->pitch = g_strdup (crt->data);
	
	srconf_free_slist (voices);
	g_ptr_array_add (src_voice_params, voice_);
    }
    else
    {
	voice_->driver 	= NULL;
	voice_->id 	= NULL;
	voice_->volume 	= NULL;
	voice_->pitch 	= NULL;
	voice_->rate 	= NULL;
	
    }
    return TRUE;
}

static void
src_speech_voices_parameters_free ()
{
    g_ptr_array_free (src_voice_params, TRUE);
}

static gboolean
src_speech_update_voices_parameters ()
{
    gchar ** voices;
    
    sru_assert (src_speech_initialized);
    
    src_speech_voices_parameters_free ();
    src_voice_params = g_ptr_array_new ();

    voices = src_voices->voices;
    if (voices)
    {
	gint i;
	for (i = 0; voices[i]; i++)
	   src_speech_get_voice_parameters_from_gconf (voices[i]);
    }
    
    return TRUE;
}

static gboolean
src_speech_get_voice_parameters (gchar *voice,
				 gchar **driver, 
				 gchar **name,
				 gchar **volume,
				 gchar **rate,
				 gchar **pitch)
{
    SRCVoice *voice_ = NULL;
    gint i;    

    sru_assert (src_speech_initialized);
    sru_assert (voice && src_voice_params);
    
    for (i = 0; i < src_voice_params->len; i++)
    {
	voice_ = g_ptr_array_index (src_voice_params, i);

	if (voice_)
	{
	    if (strcmp (voice_->voice, voice) == 0)
	    {
		    if (driver)
			*driver = g_strdup (voice_->driver);
		    if (name)
			*name   = g_strdup (voice_->id);
		    if (volume)
			*volume = g_strdup (voice_->volume);
		    if (rate)
			*rate   = g_strdup (voice_->rate);
		    if (pitch)
			*pitch  = g_strdup (voice_->pitch);

		break;
	    }
	}
    }

    return TRUE;
}

static gboolean
src_speech_set_voice_parameters (gchar *voice,
				 gchar *driver,
				 gchar *name,
				 gchar *volume,
				 gchar *rate,
				 gchar *pitch)
{
    SRCVoice *voice_ = NULL;
    gint i;
    
    sru_assert (src_speech_initialized);
    sru_assert (voice);

    for (i = 0; i < src_voice_params->len; i++)
    {
	voice_ = g_ptr_array_index (src_voice_params, i);

	if (voice_)
	{
	    if (strcmp (voice_->voice, voice) == 0)
	    {
		
		if (driver)
		{
		    g_free (voice_->driver);
		    voice_->driver 	= g_strdup (driver);
		}
		if (name)
		{
		    g_free (voice_->id);
		    voice_->id 		= g_strdup (name);
		}
		if (volume)
		{
		    g_free (voice_->volume);
		    voice_->volume 	= g_strdup (volume);
		}
		if (rate)
		{
		    g_free (voice_->rate);
		    voice_->rate 	= g_strdup (rate);
		}
		if (pitch)
		{
		    g_free (voice_->pitch);
		    voice_->pitch 	= g_strdup (pitch);
		}
		break;
	    }
	}
    }

    return TRUE;
}

static gboolean
src_speech_set_voice_parameters_to_gconf (gchar *voice,
					  gchar *driver_,
					  gchar *name_,
					  gchar *volume_,
					  gchar *rate_,
					  gchar *pitch_)
{
    gchar *driver, *name, *volume, *rate, *pitch;
    gboolean rv = FALSE;

    sru_assert (src_speech_initialized);
    sru_assert (voice && voice[0]);

    driver = name = volume = rate = pitch = NULL;
    src_speech_get_voice_parameters (voice, &driver, &name, &volume, &rate, &pitch);

    if (driver_)
    {
	g_free (driver);
	driver = g_strdup (driver_);
    }
    if (name_)
    {
	g_free (name);
	name = g_strdup (name_);
    }
    if (volume_)
    {
	g_free (volume);
	volume = g_strdup (volume_);
    }
    if (rate_)
    {
	g_free (rate);
	rate = g_strdup (rate_);
    }
    if (pitch_)
    {
	g_free (pitch);
	pitch = g_strdup (pitch_);
    }
    if (! (driver && driver[0] && name && name[0] && volume && volume[0] && 
	    rate && rate[0] && pitch && pitch[0]))
    {
	sru_warning (_("Incorrect parameters for voice \"%s%s\""), driver, voice); 
    }
    else
    {
        GSList *list = NULL;
	list = g_slist_append (list, driver);
	list = g_slist_append (list, name);
	list = g_slist_append (list, volume);
	list = g_slist_append (list, rate);
	list = g_slist_append (list, pitch);
	srconf_set_data (voice, CFGT_LIST, list, SPEECH_PARAMETER_SECTION);
	srconf_free_slist (list);
	src_speech_set_voice_parameters (voice, driver_, name_, volume_,
					 rate_, pitch_);
	rv = TRUE;
    }    
    
    return rv;
}


static gboolean
src_speech_send (gchar *spcoutput)
{
    gboolean rv = FALSE;
    /*FIXME: busy must not be here */
    static gboolean busy = FALSE;

    sru_assert (src_speech_initialized);
    sru_assert (spcoutput && spcoutput[0]);
    
    if (busy)
	return FALSE;
    busy = TRUE;
    srs_output (spcoutput, strlen(spcoutput)); 
    rv = TRUE;
    
/*     fprintf (stderr, "\n%s", spcoutput); */

    busy = FALSE;

    if (!rv)
	sru_warning (_("failed to send to speech next string \"%s\""),
					    spcoutput);

    return rv;
}

static gboolean
src_speech_create_voice (gchar *voice,
			 gchar *driver_, 
			 gchar *name_,
			 gchar *volume_,
			 gchar *rate_,
			 gchar *pitch_)
{
    gchar *voice_;
    gchar *name, *volume, *rate, *pitch;
    gchar *tts_drv, *tts_voice;
    gboolean rv = FALSE;

    sru_assert (src_speech_initialized);
    sru_assert (voice);
    name = volume = rate = pitch = NULL;
    if (name_ && driver_)
    {
	sru_assert (src_speech_valid_engine_voice (driver_, name_));
	name = g_strconcat (" TTSVoice=\"", name_, "\"", NULL);
    }
    tts_drv = tts_voice = NULL;
    if (name_)
    {
	gchar *startd = strstr (name_, "-");
	gchar *startv = strstr (name_, " ");
	if (startd && startv)
	{
	    gchar *tmp = g_strndup (startv + 1, startd - startv - 2);
	    tts_voice = g_strconcat (" TTSVoice=\"", tmp, "\"", NULL);
	    g_free (tmp);
	}
	if (startd)
	    tts_drv = g_strconcat (" TTSEngine=\"", startd + 2, "\"", NULL);
    }
    if (rate_)
	rate = g_strconcat (" rate=\"", rate_, "\"", NULL);
    if (volume_)
	volume = g_strconcat (" volume=\"", volume_, "\"", NULL);
    if (pitch_)
	pitch = g_strconcat (" pitch=\"", pitch_, "\"", NULL);

    voice_ = g_strconcat ("<SRSOUT>"
		    	    "<VOICE"
			    " ID=\"", voice, "\"",
			    /*" TTSEngine=\"gnome-speech\"",*/
			    /* " priority=\"0\"" */
			    /* " preempt=\"yes\"", */
			    /*name ? name : "",*/
			    tts_drv ? tts_drv : "",
			    tts_voice ? tts_voice : "",
			    rate ? rate : "",
			    pitch ? pitch : "",
			    volume ? volume : "",
			    "></VOICE>"
			    "</SRSOUT>",
			    NULL);
    if (voice_)
	rv = src_speech_send (voice_);
    if (!rv)
        sru_warning (_("could not create/modify voice \"%s\""), voice);
    g_free (voice_);
    g_free (name);
    g_free (rate);
    g_free (pitch);
    g_free (volume);
    g_free (tts_drv);
    g_free (tts_voice);

    return rv;
}


gboolean
src_speech_init_voice (gchar *voice)
{
    gchar *driver, *name, *volume, *rate, *pitch;
    gboolean rv = FALSE;

    sru_assert (src_speech_initialized);
    sru_assert (voice && voice[0]);
    
    driver = name = volume = rate = pitch = NULL;
    if (src_speech_get_voice_parameters (voice, &driver, &name, &volume, &rate, &pitch))
    {
	
	if (!name || !driver ||
	    !src_speech_valid_engine_voice (driver, name))
	{
	    g_free (name);
	    g_free (driver);
	    name = g_strdup (src_voices->engine_voices[0]);
	    driver = g_strdup (src_voices->engine_drivers[0]);
	    src_speech_set_voice_parameters_to_gconf (voice, driver, name, volume, rate, pitch);
	}
	
	
	rv = src_speech_create_voice (voice, driver, name, volume, rate, pitch);
    }
    g_free (driver);
    g_free (name);
    g_free (volume);
    g_free (rate);
    g_free (pitch);

    return rv;
}

/*FIXME: remove _ from function name */
static gboolean 
src_speech_say_ (gchar *text_,
		gchar *voice,
		SRCSpeechSpellingMode smode,
		SRCSpeechPriority priority)
{
    gchar *text, *attr;
    gboolean rv = FALSE;

    sru_assert (src_speech_initialized);
    sru_assert (text_ && text_[0] && voice);

    text = attr = NULL;
    attr = src_speech_get_text_voice_attributes (voice, smode);
    if  (attr)
	text = src_xml_format ("TEXT", attr, text_);
    if (text)
	rv = src_speech_send_chunk (text, priority, TRUE);
    g_free (text);
    g_free (attr);

    return rv;
}


gboolean 
src_speech_say_message (gchar *message)
{
    gboolean rv = FALSE;

    sru_assert (src_speech_initialized);
    sru_assert (message && message[0]);

    rv = src_speech_say_ (message, "message", SRC_SPEECH_SPELL_AUTO, 
				SRC_SPEECH_PRIORITY_MESSAGE);

    return rv;
}


static gboolean
src_speech_add_voice (gchar *voice)
{
    gchar **voices;
    gint i;

    sru_assert (src_speech_initialized);
    sru_assert (voice);
    sru_assert (src_voices->voices);

    for (i = 0; src_voices->voices[i]; i++)
	if (strcmp (voice, src_voices->voices[i]) == 0)
	    return TRUE;
    voices = src_voices->voices;
    src_voices->voices = g_new (gchar*, i + 2);
    for (i = 0; voices[i]; i++)
	src_voices->voices[i] = voices[i];
    src_voices->voices[i] = g_strdup (voice);
    src_voices->voices[i + 1] = NULL;
    g_free (voices);

    return TRUE;
}


static gboolean
src_speech_remove_voice (gchar *voice)
{
    gboolean rv = FALSE;
    gint i;

    sru_assert (src_speech_initialized);
    sru_assert (voice);
    sru_assert (src_voices->voices);

    for (i = 0; src_voices->voices[i]; i++)
	if (strcmp (voice, src_voices->voices[i]) == 0)
	    break;
    if (src_voices->voices[i])
	rv = TRUE;
    for ( ; src_voices->voices[i]; i++)
	src_voices->voices[i] = src_voices->voices[i + 1];

    return rv;
}


gboolean
src_speech_test_voice (gchar *voice)
{
    gchar *driver, *name, *volume, *rate, *pitch;
    gboolean rv = FALSE;

    sru_assert (src_speech_initialized);
    sru_assert (voice && voice[0]);
    sru_assert (src_voices->voices);
    
    if (!src_speech_add_voice ("test"))
	return FALSE;

    name = volume = rate = pitch = NULL;
    if (src_speech_get_voice_parameters (voice, &driver, &name, &volume, &rate, &pitch))
    {
	if (name && driver && 
	    src_speech_valid_engine_voice (driver, name))
	    rv = src_speech_create_voice ("test", driver, name, volume, rate, pitch);	    
    }

    if (rv)
    {
	gchar *test;
	test = g_strdup_printf (_("voice \"%s\" will sound this way"), voice);
	rv = src_speech_say_ (test, "test", SRC_SPEECH_SPELL_NORMAL, TRUE);
	g_free (test);
    }
    if (!rv)
	sru_warning (_("could not test voice \"%s\""), voice);

    src_speech_remove_voice ("test");
    g_free (driver);
    g_free (name);
    g_free (volume);
    g_free (rate);
    g_free (pitch);

    return rv;
}


static gboolean
src_speech_voices_init ()
{
    gchar ** voices;

    sru_assert (src_speech_initialized);

    voices = src_voices->voices;
    if (voices)
    {
	gint i;
	for (i = 0; voices[i]; i++)
	{
	   src_speech_get_voice_parameters_from_gconf (voices[i]);
	   src_speech_init_voice (voices[i]);
	}
    }
    
    return TRUE;
}


static void
src_speech_voices_terminate ()
{
    sru_assert (src_speech_initialized);
}


gboolean
src_speech_set_spelling_mode (SRCSpeechSpellingMode mode)
{
    sru_assert (src_speech_initialized);
    sru_assert (mode  != SRC_SPEECH_SPELL_AUTO);

    src_speech->spelling_mode = mode;

    return TRUE;
}


SRCSpeechSpellingMode
src_speech_get_spelling_mode ()
{
    sru_assert (src_speech_initialized);

    return src_speech->spelling_mode;
}


gboolean
src_speech_set_count_mode (SRCSpeechCountMode mode)
{
    sru_assert (src_speech_initialized);
    sru_assert (mode  != SRC_SPEECH_COUNT_AUTO);

    src_speech->count_mode = mode;
    
    return TRUE;
}


static gboolean
src_speech_set_count_mode_from_string (gchar *mode)
{
    gboolean rv = TRUE;
    SRCSpeechCountMode mode_ = src_speech->count_mode;

    sru_assert (mode);

    if (strcmp (mode, "ALL") == 0)
	mode_ = SRC_SPEECH_COUNT_ALL;
    else if (strcmp (mode, "NONE") == 0)
	mode_ = SRC_SPEECH_COUNT_NONE;
    else
    {
	sru_warning (_("value not supported for speech count mode \"%s\""), mode);
	rv = FALSE;
    }

    if (rv)
	rv = src_speech_set_count_mode (mode_);

    return rv;
}


static gboolean
src_speech_set_modifiers_echo_type (SRCSpeechModifiersEchoType type)
{
    sru_assert (src_speech_initialized);

    src_speech->modifiers_echo_type = type;
    
    return TRUE;
}


static gboolean
src_speech_set_modifiers_echo_type_from_string (gchar *type)
{
    gboolean rv = TRUE;
    SRCSpeechModifiersEchoType type_ = src_speech->modifiers_echo_type;

    sru_assert (type);

    if (strcmp (type, "ALL") == 0)
	type_ = SRC_SPEECH_MODIFIERS_ECHO_ALL;
    else if (strcmp (type, "NONE") == 0)
	type_ = SRC_SPEECH_MODIFIERS_ECHO_NONE;
    else
    {
	sru_warning (_("value not supported for speech modifiers type \"%s\""), type);
	rv = FALSE;
    }

    if (rv)
	rv = src_speech_set_modifiers_echo_type (type_);

    return rv;
}


static gboolean
src_speech_set_cursors_echo_type (SRCSpeechCursorsEchoType type)
{
    sru_assert (src_speech_initialized);

    src_speech->cursors_echo_type = type;
    
    return TRUE;
}


static gboolean
src_speech_set_cursors_echo_type_from_string (gchar *type)
{
    gboolean rv = TRUE;
    SRCSpeechCursorsEchoType type_ = src_speech->cursors_echo_type;

    sru_assert (type);

    if (strcmp (type, "ALL") == 0)
	type_ = SRC_SPEECH_CURSORS_ECHO_ALL;
    else if (strcmp (type, "NONE") == 0)
	type_ = SRC_SPEECH_CURSORS_ECHO_NONE;
    else
    {
	sru_warning (_("value not supported for speech cursors type \"%s\""), type);
	rv = FALSE;
    }

    if (rv)
	rv = src_speech_set_cursors_echo_type (type_);

    return rv;
}


static gboolean
src_speech_set_spaces_echo_type (SRCSpeechSpacesEchoType type)
{
    sru_assert (src_speech_initialized);

    src_speech->spaces_echo_type = type;
    
    return TRUE;
}


static gboolean
src_speech_set_spaces_echo_type_from_string (gchar *type)
{
    gboolean rv = TRUE;
    SRCSpeechSpacesEchoType type_ = src_speech->spaces_echo_type;

    sru_assert (type);

    if (strcmp (type, "ALL") == 0)
	type_ = SRC_SPEECH_SPACES_ECHO_ALL;
    else if (strcmp (type, "NONE") == 0)
	type_ = SRC_SPEECH_SPACES_ECHO_NONE;
    else
    {
	sru_warning (_("value not supported for speech spaces type \"%s\""), type);
	rv = FALSE;
    }

    if (rv)
	rv = src_speech_set_spaces_echo_type (type_);

    return rv;
}


static gboolean
src_speech_set_dictionary_type (SRCSpeechDictionaryType type)
{
    sru_assert (src_speech_initialized);

    src_speech->dictionary_type = type;
    
    return TRUE;
}


static gboolean
src_speech_set_dictionary_type_from_string (gchar *type)
{
    gboolean rv = TRUE;
    SRCSpeechDictionaryType type_ = src_speech->dictionary_type;

    sru_assert (type);

    if (strcmp (type, "YES") == 0)
	type_ = SRC_SPEECH_DICTIONARY_YES;
    else if (strcmp (type, "NO") == 0)
	type_ = SRC_SPEECH_DICTIONARY_NO;
    else
    {
	sru_warning (_("value not supported for speech dictionary type \"%s\""), type);
	rv = FALSE;
    }

    if (rv)
	rv = src_speech_set_dictionary_type (type_);

    return rv;
}

static gboolean
src_speech_set_text_echo_type (SRCSpeechTextEchoType type)
{
    sru_assert (src_speech_initialized);
    
    src_speech->text_echo_type = type;
    
    return TRUE;
}


static gboolean
src_speech_set_text_echo_type_from_string (gchar *type)
{
    gboolean rv = TRUE;
    SRCSpeechTextEchoType type_ = src_speech->text_echo_type;

    sru_assert (type);

    if (strcmp (type, "WORD") == 0)
	type_ = SRC_SPEECH_TEXT_ECHO_WORD;
    else if (strcmp (type, "CHARACTER") == 0)
	type_ = SRC_SPEECH_TEXT_ECHO_CHAR;
    else if (strcmp (type, "NONE") == 0)
	type_ = SRC_SPEECH_TEXT_ECHO_NONE;	
    else
    {
	sru_warning (_("value not supported for speech text echo type \"%s\""), type);
	rv = FALSE;
    }

    if (rv)
	rv = src_speech_set_text_echo_type (type_);

    return rv;
}


static struct
{
    gchar  character;
    gchar *singular;
    gchar *plural;
    gboolean translate;
}src_speech_dictionary[]=
{
    {'~',	N_("Tilde"),		N_("Tildaes"),		FALSE},
    {'!',	N_("Exclamation"),	N_("Exclamations"),	FALSE},
    {'@',	N_("At"),		N_("Ats"),		FALSE},
    {'#',	N_("Hash"),		N_("Hashes"),		FALSE},
    {'$',	N_("Dollar"),		N_("Dollars"),		FALSE},
    {'%',	N_("Percent"),		N_("Percents"),		FALSE},
    {'^',	N_("Caret"),		N_("Carets"),		FALSE},
    {'&',	N_("And"),		N_("Ands"),		FALSE},
    {'*',	N_("Asterisk"),		N_("Asterisks"),	FALSE},
    {'(',	N_("Left Paranthesis"),N_("Left Paranthesis"),	FALSE},
    {')',	N_("Right Paranthesis"),N_("Right Paranthesis"),FALSE},
    {'_',	N_("Underscore"),	N_("Underscores"),	FALSE},
    {'+',	N_("Plus"),		N_("Pluses"),		FALSE},
    {'=',	N_("Equal"),		N_("Equals"),		FALSE},
    {'-',	N_("Minus"),		N_("Minuses"),		FALSE},
    {'`',	N_("Quote"),		N_("Quotes"),		FALSE},
    {'[',	N_("Left Bracket"),	N_("Left Brackets"),	FALSE},
    {']',	N_("Right Bracket"),	N_("Right Brackets"),	FALSE},
    {'{',	N_("Left Brace"),	N_("Left Braces"),	FALSE},
    {'}',	N_("Right Brace"),	N_("Right Braces"),	FALSE},
    {';',	N_("Semicolon"),	N_("Semicolons"),	FALSE},
    {':',	N_("Colon"),		N_("Colons"),		FALSE},
    {'"',	N_("Double quote"),	N_("Double quotes"),	FALSE},
    {'\'',	N_("Apostrophe"),	N_("Apostrophes"),	FALSE},
    {'\\',	N_("Back Slash"),	N_("Back Slashes"),	FALSE},
    {'|',	N_("Bar"),		N_("Bars"),		FALSE},
    {'>',	N_("Greater"),		N_("Greaters"),		FALSE},
    {'<',	N_("Less"),		N_("Lesses"),		FALSE},
    {',',	N_("Comma"),		N_("Commas"),		FALSE},
    {'.',	N_("Dot"),		N_("Dots"),		FALSE},
    {'/',	N_("Slash"),		N_("Slashes"),		FALSE},
    {'?',	N_("Question"),		N_("Questions"),	FALSE}
};

static gboolean
src_speech_set_translate_for (gchar *set)
{
    gint i;

    sru_assert (set);

    for (i = 0; set[i]; i++)
    {
	gint j;
	for (j = 0; j < G_N_ELEMENTS (src_speech_dictionary); j++)
	{
	    if (src_speech_dictionary[j].character == set[i])
	    {
		src_speech_dictionary[j].translate = TRUE;
		break;
	    }
	}
    }

    return TRUE;
}

static gboolean
src_speech_set_punctuation_type (SRCSpeechPunctuationType type)
{
    gint i;
    sru_assert (src_speech_initialized);

    for (i = 0; i < G_N_ELEMENTS (src_speech_dictionary); i++)
	src_speech_dictionary[i]. translate = FALSE;

    switch (type)
    {
	case SRC_SPEECH_PUNCTUATION_IGNORE:
	    src_speech_set_translate_for ("");
	    break;
	case SRC_SPEECH_PUNCTUATION_SOME:
	    src_speech_set_translate_for ("<>(){}*-+=");
	    break;
	case SRC_SPEECH_PUNCTUATION_MOST:
	    src_speech_set_translate_for ("<>(){}*-+=.,!?\"'");
	    break;
	case SRC_SPEECH_PUNCTUATION_ALL:
	    src_speech_set_translate_for ("<>(){}*-+=.,!?\"'&_[]|\\/");
	    break;

    }

    return TRUE;
}


static gboolean
src_speech_set_punctuation_type_from_string (gchar *type)
{
    gboolean rv = TRUE;
    SRCSpeechPunctuationType type_ = SRC_SPEECH_PUNCTUATION_SOME;

    sru_assert (type);

    if (strcmp (type, "IGNORE") == 0)
	type_ = SRC_SPEECH_PUNCTUATION_IGNORE;
    else if (strcmp (type, "SOME") == 0)
	type_ = SRC_SPEECH_PUNCTUATION_SOME;
    else if (strcmp (type, "MOST") == 0)
	type_ = SRC_SPEECH_PUNCTUATION_MOST;
    else if (strcmp (type, "ALL") == 0)
	type_ = SRC_SPEECH_PUNCTUATION_ALL;
    else
    {
	sru_warning (_("value not supported for speech punctuation type \"%s\""), type);
	rv = FALSE;
    }

    if (rv)
	rv = src_speech_set_punctuation_type (type_);

    return rv;
}


void
src_speech_shutup ()
{
    sru_assert (src_speech_initialized);
#ifdef SRS_NO_MARKERS_SUPPORTED
    {
	GSList *crt, *tmp = src_speech_unspoken;
	src_speech_unspoken = NULL;
	for (crt = tmp; crt; crt = crt->next)
	    srcs_out_terminate (crt->data);
	g_slist_free (tmp);
    }
#endif
    if (last_out)
    {
        src_speech_idle = TRUE;
        srcs_out_terminate (last_out);
	last_out = NULL;
    }

    src_speech_send ("<SRSOUT><SHUTUP/></SRSOUT>");
}


void
src_speech_pause ()
{
    sru_assert (src_speech_initialized);

    src_speech_send ("<SRSOUT><PAUSE/></SRSOUT>");
}


void
src_speech_resume ()
{
    sru_assert (src_speech_initialized);

    src_speech_send ("<SRSOUT><RESUME/></SRSOUT>");
}


static gboolean
src_speech_send_from_queue ()
{
    gboolean rv;
    gchar *output;

    sru_assert (srcs_outs);
#ifdef SRS_NO_MARKERS_SUPPORTED
    if (!src_speech_callback)
    {
	sru_assert (g_slist_length (srcs_outs) == 1);
	if (last_out)
	{
	    src_speech_unspoken = g_slist_append (src_speech_unspoken, last_out);
	    last_out = NULL;
	    src_speech_idle = TRUE;
	}
    }
#endif

    sru_assert (last_out == NULL);
    last_out = srcs_outs->data;
    srcs_outs = g_slist_remove_link (srcs_outs, srcs_outs);
    sru_assert (src_speech_idle);
    output = g_strdup_printf ("<SRSOUT markers=\"out-started:out-ended\" id=\"%xp\">%s</SRSOUT>", (unsigned int)last_out->chunk, last_out->chunk);
    src_speech_idle = FALSE;
    rv = src_speech_send (output);
    g_free (output);

    return rv;
}

static gboolean
srcs_out_can_shutup_out (SRCSOut *crt,
			 SRCSOut *new_)
{
    sru_assert (crt && new_);
    return  (crt->priority < new_->priority ||
		(crt->priority == new_->priority && crt->preempt));
}

gboolean
src_speech_send_chunk (gchar *chunk,
		       SRCSpeechPriority priority,
		       gboolean preempt)
{
    SRCSOut *out;
    GSList *crt, *tmp;
	
    sru_assert (src_speech_initialized);
    sru_assert (chunk && chunk[0]);
		
    out = srcs_out_new ();
    out->chunk = g_strdup (chunk);
    out->ido = g_strdup_printf ("%xp",(unsigned int) out->chunk);
    out->priority = priority;
    out->preempt = preempt;
	
    for (crt = srcs_outs; crt; crt = crt->next)
    {
        if (srcs_out_can_shutup_out (crt->data, out))
	    break;
    }
    tmp = crt;
    for (; crt; crt = crt->next)
	srcs_out_terminate (crt->data);
    if (tmp)
    {
        g_slist_free (tmp->next);
	tmp->next = NULL;
	srcs_outs = g_slist_remove_link (srcs_outs, tmp);
    }
										        
    srcs_outs = g_slist_append (srcs_outs, out);
    if (last_out && srcs_out_can_shutup_out (last_out, out))
    {
	src_speech_shutup ();
    }

#ifdef SRS_NO_MARKERS_SUPPORTED
    if (!src_speech_callback)
	src_speech_send_from_queue ();
    else
#endif
    if (src_speech_idle)
        src_speech_send_from_queue ();

    return TRUE;
}

static gboolean
src_dictionary_hash_init ()
{
    src_dictionary_hash = g_hash_table_new (g_str_hash , g_str_equal);
    if (!src_dictionary_hash)
	return FALSE;
    return TRUE;
}

static gboolean
src_dictionary_hash_terminate ()
{
    sru_assert (src_dictionary_hash != NULL);
    g_hash_table_destroy (src_dictionary_hash);
    src_dictionary_hash = NULL;
    return TRUE;
}

static gboolean
src_dictinary_add_item (gchar *key,
			gchar *value)
{
    sru_assert (src_dictionary_hash != NULL);
/*    fprintf (stderr,"%s-%s",key,value);*/
    g_hash_table_insert (src_dictionary_hash, 
			key,
			value);
    return TRUE;
}

/* DO NOT free the returned element*/
static gchar*
src_dictionary_lookup (gchar *key)
{
    gchar *word;

    sru_assert (key);
    sru_assert (src_dictionary_hash != NULL);
    word = g_hash_table_lookup (src_dictionary_hash, key);
    if (word)
	return word;
	
    return NULL;
}

static gboolean
src_dictionary_entry_split (const gchar *entry,
			    gchar **word,
			    gchar **replace)
{
    gchar *separator = NULL;
    
    *word = NULL;
    *replace = NULL;

    if (!entry)
	return FALSE;
    
    separator = g_utf8_strchr (entry, -1, '<');
    if (separator && *(g_utf8_find_next_char (separator, NULL)) == '>')
    {
	*word = g_strndup (entry, separator - entry);
	separator = g_utf8_find_next_char (separator, NULL);
	*replace  = g_strdup (g_utf8_find_next_char (separator,NULL));
	return TRUE;
    }
    return FALSE;
}

static gboolean
src_dictionary_load ()
{
    GSList *crt = NULL;
    GSList *dictionary_list = NULL;

    if (!srconf_get_data_with_default (SPEECH_DICTIONARY_LIST, CFGT_LIST,
				  &dictionary_list, NULL, 
				  SPEECH_DICTIONARY_SECTION))
    return FALSE;

    
    crt = dictionary_list;
    
    while (crt)
    {
	gchar *key   = NULL;
	gchar *value = NULL;

	src_dictionary_entry_split ((const gchar *)crt->data,
				    &key, &value);
		
	src_dictinary_add_item (key, value);
	
	crt = crt->next;
    }
 
    if (dictionary_list)
	srconf_free_slist (dictionary_list);
    
    return TRUE;
}

static gboolean
src_dictionary_terminate ()
{
    src_dictionary_hash_terminate ();
    return TRUE;
}

static gboolean
src_dictionary_init ()
{

    if (!src_dictionary_hash_init ())
    {
	sru_warning (_("Unable to initialize hashtable"));
	return FALSE;
    }
    
    if (!src_dictionary_load ())
    {
	sru_warning (_("Unable to load dictionary from gconf"));
	return FALSE;
    }
    return TRUE;
}


static gboolean
src_speech_init_default_voice ()
{
    gboolean rv = FALSE;
    gchar *driver, *name, *volume, *rate, *pitch;
 
    sru_assert (src_speech_initialized);
    sru_assert (src_voices->engine_voices);
   
    driver = g_strdup (src_voices->engine_drivers[0]);
    name   = g_strdup (src_voices->engine_voices[0]);
    volume = g_strdup_printf ("%d", DEFAULT_SPEECH_VOLUME);
    rate   = g_strdup_printf ("%d", DEFAULT_SPEECH_RATE);
    pitch  = g_strdup_printf ("%d", DEFAULT_SPEECH_PITCH);
    rv = src_speech_set_voice_parameters_to_gconf ("default", driver, name, volume, rate, pitch);
    
    g_free (driver);
    g_free (name);
    g_free (rate);
    g_free (volume);
    g_free (pitch);	

    return rv;
}

static gboolean
src_speech_load_defaults ()
{
    gchar *tmp;

    sru_assert (src_speech_initialized);

    tmp = NULL;
    GET_SPEECH_CONFIG_DATA_WITH_DEFAULT (SPEECH_COUNT, CFGT_STRING, 
							    &tmp, NULL);
    if (tmp)
	src_speech_set_count_mode_from_string (tmp);
    else
        src_speech_set_count_mode (SRC_SPEECH_COUNT_NONE);
    g_free (tmp);

    tmp = NULL;
    GET_SPEECH_CONFIG_DATA_WITH_DEFAULT (SPEECH_PUNCTUATION, CFGT_STRING, 
							    &tmp, NULL);
    if (tmp)
	src_speech_set_punctuation_type_from_string (tmp);
    else
        src_speech_set_punctuation_type (SRC_SPEECH_PUNCTUATION_SOME);
    g_free (tmp);

    tmp = NULL;
    GET_SPEECH_CONFIG_DATA_WITH_DEFAULT (SPEECH_TEXT_ECHO, CFGT_STRING, 
							    &tmp, NULL);
    if (tmp)
	src_speech_set_text_echo_type_from_string (tmp);
    else
        src_speech_set_text_echo_type (SRC_SPEECH_TEXT_ECHO_CHAR);
    g_free (tmp);

    tmp = NULL;
    GET_SPEECH_CONFIG_DATA_WITH_DEFAULT (SPEECH_MODIFIERS, CFGT_STRING, 
							    &tmp, NULL);
    if (tmp)
	src_speech_set_modifiers_echo_type_from_string (tmp);
    else
        src_speech_set_modifiers_echo_type (SRC_SPEECH_MODIFIERS_ECHO_ALL);
    g_free (tmp);

    tmp = NULL;
    GET_SPEECH_CONFIG_DATA_WITH_DEFAULT (SPEECH_CURSORS, CFGT_STRING, 
							    &tmp, NULL);
    if (tmp)
	src_speech_set_cursors_echo_type_from_string (tmp);
    else
        src_speech_set_cursors_echo_type (SRC_SPEECH_CURSORS_ECHO_ALL);
    g_free (tmp);

    tmp = NULL;
    GET_SPEECH_CONFIG_DATA_WITH_DEFAULT (SPEECH_SPACES, CFGT_STRING, 
							    &tmp, NULL);
    if (tmp)
	src_speech_set_spaces_echo_type_from_string (tmp);
    else
        src_speech_set_spaces_echo_type (SRC_SPEECH_SPACES_ECHO_ALL);
    g_free (tmp);

    tmp = NULL;
    GET_SPEECH_CONFIG_DATA_WITH_DEFAULT (SPEECH_DICTIONARY_ACTIVE, CFGT_STRING, 
								   &tmp, NULL);
    if (tmp)
	src_speech_set_dictionary_type_from_string (tmp);
    else
        src_speech_set_dictionary_type (SRC_SPEECH_DICTIONARY_NO);
    g_free (tmp);

    return TRUE;
}

static SRCSpeech*
src_speech_src_speech_new ()
{
    SRCSpeech *speech;
    
    speech = g_new0 (SRCSpeech, 1);
    
    return speech;
}

static SRCVoices*
src_speech_src_voices_new ()
{
    SRCVoices *voices;
    
    voices = g_new0 (SRCVoices, 1);
    
    return voices;
}

void
src_speech_set_default_values (SRCSpeech *src_speech)
{
    src_speech->text_echo_type     = SRC_SPEECH_TEXT_ECHO_CHAR;
    src_speech->modifiers_echo_type= SRC_SPEECH_MODIFIERS_ECHO_ALL;
    src_speech->spaces_echo_type   = SRC_SPEECH_SPACES_ECHO_ALL;
    src_speech->cursors_echo_type  = SRC_SPEECH_CURSORS_ECHO_ALL;
    src_speech->dictionary_type    = SRC_SPEECH_DICTIONARY_YES;
    src_speech->spelling_mode      = SRC_SPEECH_SPELL_NORMAL;
    src_speech->count_mode         = SRC_SPEECH_COUNT_NONE;
}

gboolean
src_speech_init ()
{
    sru_assert (!src_speech_initialized);
    
    src_speech_idle = TRUE;
    srcs_outs = NULL;
    last_out = NULL;
    src_voice_params = g_ptr_array_new ();
    
    src_voices = src_speech_src_voices_new ();
    src_speech = src_speech_src_speech_new ();
    
    src_speech_set_default_values (src_speech);
    
    srcs_in_init ();
    src_speech_initialized = srs_init (srcs_in_parse_input);
#ifdef SRS_NO_MARKERS_SUPPORTED
    src_speech_callback = TRUE;
    src_speech_unspoken = NULL;
#endif

    if (src_speech_initialized)
    {
	src_voices->voices 	  = src_speech_get_voices ();
    }
    
    if (src_speech_initialized && 
     src_voices->engine_voices && 
     src_voices->voices)
    {
	src_speech_initialized = src_speech_init_default_voice () &&
				 src_speech_voices_init ();
    }
    else
    {
	src_speech_initialized = FALSE;
    }

    if (src_speech_initialized)
	src_dictionary_init ();

    if (src_speech_initialized)
    {
	src_speech_set_spelling_mode (SRC_SPEECH_SPELL_NORMAL); /*FIXME: read from gconf????*/
	src_speech_load_defaults ();
    }

    srconf_unset_key (SRCORE_SPEECH_SENSITIVE, SRCORE_PATH);/*FIXME:why this line */
    SET_SRCORE_CONFIG_DATA (SRCORE_SPEECH_SENSITIVE, CFGT_BOOL, 
				&src_speech_initialized);/*FIXME: must be in speech gconf directory */

    if (src_speech_initialized)
    {
    	sru_message (_("speech initialization succeded"));
    }
    else
    {
    	sru_message (_("speech initialization failed"));
    }
    if (!src_speech_initialized)
    {
	g_strfreev (src_voices->voices);
	g_strfreev (src_voices->engine_voices);
    }

    return src_speech_initialized;
}


void
src_speech_terminate ()
{
    sru_assert (src_speech_initialized);

    src_speech_shutup ();
#ifdef SRS_NO_MARKERS_SUPPORTED
    sru_assert (src_speech_unspoken == NULL);
#endif
    srcs_in_terminate ();
    srs_terminate();
    src_speech_voices_terminate ();
    src_dictionary_terminate ();
    src_speech_initialized = FALSE;    
    g_strfreev (src_voices->voices);
    g_strfreev (src_voices->engine_voices);
    g_strfreev (src_voices->engine_drivers);
}

#define src_speech_is_separator(x) (( x == '.' || x == ';' || x == '?' || x == ',' || x == '!' ) ? TRUE : FALSE)
static gchar*
src_dictionary_process_string (gchar *str)
{
    gchar *crt = str;
    gchar *word_start;
    gchar *rv = NULL;    
    gchar *word_key = NULL;
    gchar *word= NULL;
    gchar *tmp = NULL;

    
    g_assert (str != NULL);

    if (!str)
	return g_strdup (str);
	
    word_start = crt;
    
    while (*crt)
    {
	if (g_unichar_isspace (*crt) ||
	    g_unichar_ispunct (*crt))
	{
	    gchar *word_key = NULL;
	    gchar *word= NULL;
	    gchar *tmp = NULL;
	    
	    word_key = g_strndup (word_start, crt - word_start);

	    if ((word = src_dictionary_lookup (word_key)) == NULL)
		word = word_key;
	
	    tmp = g_strdup_printf ("%s%s%c", STR(rv), word, *crt);		
	    g_free (word_key);
	    g_free (rv);
	    rv = tmp;
	    word_start = g_utf8_next_char (crt);
	}

	crt = g_utf8_next_char (crt);
    }
    
    word_key = g_strndup (word_start, crt - word_start);
    
    if ((word = src_dictionary_lookup (word_key)) == NULL)
	word = word_key;
	
    tmp = g_strdup_printf ("%s%s%c", STR(rv), word, *crt);
		
    g_free (word_key);
    g_free (rv);
    rv = tmp;

    if (!rv)
	return g_strdup (str);
	        
    return rv;
}


static gint
src_speech_get_index_for_symbol (gchar c)
{
    gint i;
    
    sru_assert (src_speech_initialized);
    
    for (i = 0 ; i < G_N_ELEMENTS (src_speech_dictionary); i++)
    {
	if (src_speech_dictionary [i].character == c)
	{
	    return i;
	}
    }
    
    return -1;
}


static gchar*
src_speech_translate_symbol (gint index)
{
    gchar *rv = NULL;

    sru_assert (src_speech_initialized);
    sru_assert (index >= 0 && index < G_N_ELEMENTS (src_speech_dictionary));

    if (src_speech_dictionary[index].translate)
    {
	if (src_speech_dictionary[index].character == '.' ||
	    src_speech_dictionary[index].character == '!' ||
	    src_speech_dictionary[index].character == '?')
	{
	    rv = g_strconcat ("  ", src_speech_dictionary[index].singular, " ", NULL);
	    rv[0] = src_speech_dictionary[index].character;
	}
	else
	    rv = g_strconcat (" ", src_speech_dictionary[index].singular, " ", NULL);
    }
    else
    {
	rv = g_strdup (" ");
	rv[0] = src_speech_dictionary[index].character;
    }
    return rv;
}		


static gchar*
src_speech_count_symbol (gchar *start,
			 gchar **new_pos)
{
    gchar *crt = start + 1;
    gchar count[10];
    gchar *rv = NULL;
    gint cnt, index;

    sru_assert (src_speech_initialized);
    sru_assert (start && new_pos);
    index = src_speech_get_index_for_symbol (start [0]);
    sru_assert (index >= 0);

    while (*start == *crt)
	crt++;
    count [0] = '\0';
    cnt = crt- start;
    if (cnt > 1)
	sprintf (count, " %d ", crt - start);
    if (cnt == 1 && src_speech_is_separator (start[0]))
	rv = g_strndup (start, 1);
    else
	rv = g_strconcat (count, " ",
	    cnt == 1 ? src_speech_dictionary[index].singular : 
					src_speech_dictionary[index].plural,
	    " ", NULL);
    *new_pos = crt;
    return rv;
}
			 

extern gboolean src_enable_format_string; /*FIXME: must be removed */
gchar*
src_speech_process_string (const gchar   *text,
			   SRCSpeechCountMode cmode,
			   SRCSpeechSpellingMode smode)
{
    gchar *crt = (gchar*) text;
    gchar *rv  = NULL;
    gchar *tmp = NULL;
    
    sru_assert (src_speech_initialized);
    sru_assert (text);

    if (cmode == SRC_SPEECH_COUNT_AUTO)
	cmode = src_speech->count_mode;
    if (smode == SRC_SPEECH_SPELL_AUTO)
	smode = src_speech->spelling_mode;

    if (!src_enable_format_string)
	return g_strdup (text);
	
    if (text[0] == ' ' && text[1] == '\0' && smode == SRC_SPEECH_SPELL_NORMAL)
	return g_strdup (_("space"));
    if (text[0] == '\n' && text[1] == '\0' && smode == SRC_SPEECH_SPELL_NORMAL)
	return g_strdup (_("enter"));

    if (text[0] && !text[1])
    {
        gint index = src_speech_get_index_for_symbol (crt[0]);
	if (index != -1)
	    return g_strdup (src_speech_dictionary[index].singular);
	else
	    return g_strdup ((gchar*)text);
    }

    if (src_speech->dictionary_type == SRC_SPEECH_DICTIONARY_YES)
    {
	crt = src_dictionary_process_string (crt);
	tmp = crt;
    }
    
    rv = g_strdup ("");
    while (*crt)
    {
	gchar *next = g_utf8_next_char (crt);
	gchar *add = NULL;

	if (next - crt == 1)
	{
	    gint index = src_speech_get_index_for_symbol (crt[0]);

	    if (index != -1)
	    {
		if (smode == SRC_SPEECH_SPELL_NORMAL && cmode == SRC_SPEECH_COUNT_NONE)
		    add = src_speech_translate_symbol (index);
		if (cmode == SRC_SPEECH_COUNT_ALL)
		    add = src_speech_count_symbol (crt, &next);
	    }
	    else
	    {
		add = g_strndup (crt, next - crt);
	    }
	}
	else
	{
	    add = g_strndup (crt, next - crt);
	}
	if (add)
	{
	    gchar *tmp = rv;
	    rv = g_strconcat (rv, add, NULL);
	    g_free (tmp);
	    g_free (add); 
	}
	crt = next;
    }
    
    
    if (src_speech->dictionary_type == SRC_SPEECH_DICTIONARY_YES)
	g_free (tmp);
    	
    return rv[0] ? rv : NULL;
}


gchar*
src_speech_get_text_voice_attributes (gchar *voice,
				      SRCSpeechSpellingMode smode)
{
    SRCSpeechSpellingMode mode = smode;
    gchar *spelling;

    sru_assert (src_speech_initialized);
    sru_assert (voice);
    
    if (smode == SRC_SPEECH_SPELL_AUTO)
	mode = src_speech->spelling_mode;

    if (mode == SRC_SPEECH_SPELL_MILITARY)
	spelling = " spelling='military'";
    else if (mode == SRC_SPEECH_SPELL_CHAR_BY_CHAR)
	spelling = " spelling='char'";
    else
	spelling = "";

    if (!src_speech_valid_voice (voice))
    {
	sru_warning (_("can not find \"%s\" voice. use default voice"), voice);
	voice = "default";
    }

    return g_strconcat ("voice=\"", voice, "\"", spelling, NULL);
}


gboolean
src_speech_modify_pitch (SRCModifyMode mode)
{
    gchar ** voices;
    gchar *message;
    
    sru_assert (src_speech_initialized);
    
    if (mode == SRC_MODIF_DEFAULT)
	message = _("voices pitch will be set to default value");
    else if (mode == SRC_MODIF_DECREASE)
	message = _("voices pitch will be decreased");
    else
	message = _("voices pitch will be increased");
	
    src_speech_say_message (message);

    voices = src_voices->voices;
    if (voices)
    {
	gint i;
	for (i = 0; voices[i]; i++)
	{
	    gchar *pitch_ = NULL;
	    gint   pitch = DEFAULT_SPEECH_PITCH;
	
	    if (mode != SRC_MODIF_DEFAULT)
	    {
		if (src_speech_get_voice_parameters (voices[i], NULL, NULL, NULL, NULL, &pitch_))
		{
		    pitch = atoi (pitch_);
		    g_free (pitch_);
		    pitch += (mode == SRC_MODIF_DECREASE) ? 
			    -SPEECH_PITCH_STEP : SPEECH_PITCH_STEP;
		}	
	    }

	    pitch = MIN (MAX (pitch, MIN_SPEECH_PITCH), MAX_SPEECH_PITCH);
	    pitch_ = g_strdup_printf ("%d", pitch);
	    if (src_speech_set_voice_parameters_to_gconf (voices[i], NULL, NULL, NULL, NULL, pitch_))
		src_speech_create_voice (voices[i], NULL, NULL, NULL, NULL, pitch_);
	    g_free (pitch_);	    
	}
    }
    return TRUE;
}


gboolean
src_speech_modify_rate (SRCModifyMode mode)
{
    gchar ** voices;
    gchar *message;
    
    sru_assert (src_speech_initialized);
    
    if (mode == SRC_MODIF_DEFAULT)
	message = _("voices rate will be set to default value");
    else if (mode == SRC_MODIF_DECREASE)
	message = _("voices rate will be decreased");
    else
	message = _("voices rate will be increased");
	
    src_speech_say_message (message);

    voices = src_voices->voices;
    if (voices)
    {
	gint i;
	for (i = 0; voices[i]; i++)
	{
	    gchar *rate_ = NULL;
	    gint rate = DEFAULT_SPEECH_RATE;
	
	    if (mode != SRC_MODIF_DEFAULT)
	    {
		if (src_speech_get_voice_parameters (voices[i], NULL, NULL, NULL, &rate_, NULL))
		{
		    rate = atoi (rate_);
		    g_free (rate_);
		    rate += (mode == SRC_MODIF_DECREASE) ? 
			    -SPEECH_RATE_STEP : SPEECH_RATE_STEP;
		}	
	    }

	    rate = MIN (MAX (rate, MIN_SPEECH_RATE), MAX_SPEECH_RATE);
	    rate_ = g_strdup_printf ("%d", rate);
	    if (src_speech_set_voice_parameters_to_gconf (voices[i], NULL, NULL, NULL, rate_, NULL))
		src_speech_create_voice (voices[i], NULL, NULL, NULL, rate_, NULL);
	    g_free (rate_);	    
	}
    }

    return TRUE;
}


gboolean
src_speech_modify_volume (SRCModifyMode mode)
{
    gchar ** voices;
    gchar *message;
    
    sru_assert (src_speech_initialized);
    
    if (mode == SRC_MODIF_DEFAULT)
	message = _("voices volume will be set to default value");
    else if (mode == SRC_MODIF_DECREASE)
	message = _("voices volume will be decreased");
    else
	message = _("voices volume will be increased");
	
    src_speech_say_message (message);

    voices = src_voices->voices;
    if (voices)
    {
	gint i;
	for (i = 0; voices[i]; i++)
	{
	    gchar *volume_ = NULL;
	    gint volume = DEFAULT_SPEECH_VOLUME;
	
	    if (mode != SRC_MODIF_DEFAULT)
	    {
		if (src_speech_get_voice_parameters (voices[i], NULL, NULL, &volume_, NULL, NULL))
		{
		    volume = atoi (volume_);
		    g_free (volume_);
		    volume += (mode == SRC_MODIF_DECREASE) ? 
			    -SPEECH_VOLUME_STEP : SPEECH_VOLUME_STEP;
		}
	    }

	    volume = MIN (MAX (volume, MIN_SPEECH_VOLUME), MAX_SPEECH_VOLUME);
	    volume_ = g_strdup_printf ("%d", volume);
	    if (src_speech_set_voice_parameters_to_gconf (voices[i], NULL, NULL, volume_, NULL, NULL))
		src_speech_create_voice (voices[i], NULL, NULL, volume_, NULL, NULL);
	    g_free (volume_);	    
	}
    }

    return TRUE;
}

gboolean
src_speech_process_voice_config_changed (SRConfigStructure *config)
{
    gboolean rv = TRUE;
    
    sru_assert (src_speech_initialized);
    sru_assert (config && config->key);
    sru_assert (config->type == CFGT_LIST);
    sru_assert (config->newvalue);
	
    if (!strcmp (config->key, SPEECH_GNOPERNICUS_SPEAKERS))
	    return rv;
	
    rv = src_speech_add_voice (config->key)  &&
	 src_speech_init_voice (config->key) &&
	 src_speech_update_voices_parameters();
    
    
    return rv;
}

gboolean
src_speech_process_config_changed (SRConfigStructure *config)
{
    gboolean rv = TRUE;
    sru_assert (src_speech_initialized);
    sru_assert (config && config->key);

    if (strcmp (config->key, SPEECH_COUNT) == 0)
    {
	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_set_count_mode_from_string (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_PUNCTUATION) == 0)
    {
	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_set_punctuation_type_from_string (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_VOICE_TEST) == 0)
    {
    	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_test_voice (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_VOICE_REMOVED) == 0)
    {
	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_remove_voice (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_TEXT_ECHO) == 0)
    {
	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_set_text_echo_type_from_string (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_MODIFIERS) == 0)
    {
	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_set_modifiers_echo_type_from_string (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_SPACES) == 0)
    {
	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_set_spaces_echo_type_from_string (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_CURSORS) == 0)
    {
	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_set_cursors_echo_type_from_string (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_DICTIONARY_ACTIVE) == 0)
    {
	sru_assert (config->type == CFGT_STRING);
	sru_assert (config->newvalue);
	rv = src_speech_set_dictionary_type_from_string (config->newvalue);
    }
    else if (strcmp (config->key, SPEECH_DICTIONARY_CHANGES) == 0)
    {
	sru_assert (config->type == CFGT_BOOL);
	sru_assert (config->newvalue);
	if (src_dictionary_terminate ())
	    rv = src_dictionary_init ();
    }
    else
    {
	sru_warning (_("no support for speech key \"%s\""), config->key);
	rv = FALSE;
    }

    return rv;
}



/*FIXME: Next functions must be removed */
gchar*
src_speech_get_voice(gchar *voice)
{
    return src_speech_get_text_voice_attributes (voice, SRC_SPEECH_SPELL_AUTO);
}

gboolean 
src_speech_say (gchar *message, gboolean shutup)
{
    return src_speech_say_message (message);
}

gchar*
src_process_string (const gchar   *text,
		    SRCSpeechCountMode  cmode,
		    SRCSpeechSpellingMode smode)
{
    if (!src_speech_initialized)
	return g_strdup (text);
    return src_speech_process_string (text, cmode, smode);
}

SRCSpeechTextEchoType
src_speech_get_text_echo_type ()
{
    sru_assert (src_speech &&
		src_speech_initialized);
    
    return src_speech->text_echo_type;
}

SRCSpeechSpacesEchoType
src_speech_get_spaces_echo_type ()
{
    sru_assert (src_speech &&
		src_speech_initialized);
    
    return src_speech->spaces_echo_type;
}

SRCSpeechModifiersEchoType
src_speech_get_modifiers_echo_type ()
{
    sru_assert (src_speech &&
		src_speech_initialized);
    
    return src_speech->modifiers_echo_type;
}

SRCSpeechCursorsEchoType
src_speech_get_cursors_echo_type ()
{
    sru_assert (src_speech &&
		src_speech_initialized);
		
    return src_speech->cursors_echo_type;
}
/*FIXME end*/

gboolean
src_speech_set_idle (gchar *ido)
{
    sru_assert (ido);

    if (last_out && strcmp (ido, last_out->ido) == 0)
    {
        src_speech_idle = TRUE;
        srcs_out_terminate (last_out);
	last_out = NULL;
	if (srcs_outs)
	    src_speech_send_from_queue ();
    }
    return TRUE;
}

gboolean
src_speech_add_created_voice (gchar *name
#ifdef SRS_NO_MARKERS_SUPPORTED
    			    , gboolean has_callback
#endif
			    )
{
    
#ifdef SRS_NO_MARKERS_SUPPORTED
    if (src_speech_callback)
    {
	src_speech_callback = has_callback;
	if (!src_speech_callback)
	    sru_warning ("You are using at least one voice witout markers");
    }
#endif

    return TRUE;
}

SRSDriver*
srs_driver_new ()
{
    return g_new0 (SRSDriver, 1);
}

static void
srs_driver_terminate (SRSDriver *driver)
{
    sru_assert (driver);

    g_free (driver->name);
    g_strfreev (driver->voices);
    g_free (driver);
}

static void
srs_drivers_terminate (GPtrArray *drivers)
{
    gint i;

    sru_assert (drivers);
    for (i = 0; i < drivers->len - 1; i++)
	srs_driver_terminate (g_ptr_array_index (drivers, i));
    g_ptr_array_free (drivers, TRUE);
}

static gchar*
srs_normalize_driver_name (gchar *driver_name)
{
    gchar *new_driver_name;

    sru_assert (driver_name);
    new_driver_name = g_strdup (driver_name);
    new_driver_name = g_strdelimit (new_driver_name, "|> <", '_');

    return new_driver_name;
}

static gboolean
srs_write_driver_info_to_gconf (gchar *driver_name,
			       GSList *voices)
{
    gchar *name;
    sru_assert (driver_name);
    sru_assert (voices);

    name = srs_normalize_driver_name (driver_name);
    srconf_set_data (name, CFGT_LIST, voices, SPEECH_DRIVERS_SECTION);
    g_free (name);

    return TRUE;
}

static gboolean
srs_write_drivers_to_gconf (GSList *drivers)
{
    sru_assert (drivers);

    srconf_set_data (SPEECH_ENGINE_DRIVERS, CFGT_LIST, drivers,
			    SPEECH_DRIVERS_SECTION);
    return TRUE;
}

static gboolean
srs_write_voice_count_to_gconf (gint voice_cnt)
{
    srconf_set_data (SPEECH_VOICE_COUNT, CFGT_INT, &voice_cnt,
			SPEECH_DRIVERS_SECTION);
    return TRUE;
}

typedef void (*FreeType) (gpointer data);

static void
srs_slist_free (GSList *list, FreeType func)
{
     GSList *crt;
     sru_assert (list);
     sru_assert (func);
     for (crt = list; crt; crt = crt->next)
        func (crt->data);
    g_slist_free (list);
}

gboolean
src_speech_process_drivers (GPtrArray *drivers)
{
    gint i, j, voice_cnt = 0;
    GSList *drivers_ = NULL;
    GPtrArray *driver_names;
    GPtrArray *voice_names;

    driver_names = g_ptr_array_new ();
    voice_names = g_ptr_array_new ();
    for (i = 0; i < drivers->len; i++)
    {
	GSList *temp = NULL;
	SRSDriver *driver;

	driver = g_ptr_array_index (drivers, i);
	sru_assert (driver);
	drivers_ = g_slist_append (drivers_, 
			       srs_normalize_driver_name (driver->name));
	 g_ptr_array_add (driver_names, srs_normalize_driver_name (driver->name));
        /* fprintf (stderr, "\nDRIVER:%s",  srs_normalize_driver_name (driver->name)); */
        for (j = 0; driver->voices[j]; j++)
	{
	    gchar *voice_name;
	    voice_name = g_strdup_printf ("V%d %s - %s", j, driver->voices[j],
					    	      driver->name);
	    temp = g_slist_append (temp, voice_name);
	    g_ptr_array_add (voice_names, g_strdup (voice_name));
	    /* fprintf (stderr, "\n\tVOICE:%s", voice_name); */
	    voice_cnt++;
	}
	srs_write_driver_info_to_gconf (driver->name, temp);
	srs_slist_free (temp, g_free);
    }
    srs_write_voice_count_to_gconf (voice_cnt);
    srs_write_drivers_to_gconf (drivers_);
    srs_slist_free (drivers_, g_free);

    g_ptr_array_add (driver_names, NULL);
    g_ptr_array_add (voice_names, NULL);
    src_voices->engine_drivers = (gchar**) g_ptr_array_free (driver_names, FALSE);
    src_voices->engine_voices  = (gchar**) g_ptr_array_free (voice_names, FALSE);

    srs_drivers_terminate (drivers);

    return TRUE;
}
