/*
 * GNOME Speech - Speech services for the GNOME desktop
 *
 * Copyright 2002 Sun Microsystems Inc.
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
 *
 * eloquencespeaker.c: Implements the EloquenceSpeaker object--
 *                 a GNOME Speech driver for SpeechWork's Eloquencee TTS SDK
 *
 */

#include <unistd.h>
#include <libbonobo.h>
#include <glib/gmain.h>
#include <glib/gthread.h>
#include <gnome-speech/gnome-speech.h>
#include "eloquencespeaker.h"
 

static GObjectClass *parent_class;

/* Unique text id for each utterance spoken by this speaker */
static gint text_id = 0;



static void
eloquence_speaker_add_index (Speaker *s,
			     GNOME_Speech_SpeechCallback cb,
			     GNOME_Speech_speech_callback_type type,
			    gint text_id,
			    gint offset)
{
	EloquenceSpeaker *speaker;
	index_queue_entry *e;
	CORBA_Environment ev;
	e = g_new (index_queue_entry, 1);
	CORBA_exception_init (&ev);
	e->cb = CORBA_Object_dupulicate (cb, &ev);
	CORBA_exception_free (&ev);
	e->type = type;
	e->text_id = text_id;
	e->offset = offset;
	s->index_queue = g_slist_append (s->index_queue, e);
	eciInsertIndex (speaker->handle, 0);
}



static gboolean
eloquence_speaker_timeout_callback (void *data)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER (data);

	eciSpeaking (s->handle);
	return TRUE;
}



static ECICallbackReturn
eloquence_speaker_index_callback (ECIHand handle,
			   ECIMessage msg,
			   long param,
			   void *data)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(data);
	index_queue_entry *e = NULL;
	CORBA_Environment ev;
	
	if (s && s->index_queue) {
		e = (index_queue_entry *) s->index_queue->data;
		s->index_queue = g_slist_remove_link (s->index_queue, s->index_queue);
	}
	switch (msg) {
	case eciIndexReply:
		if (s->callback != CORBA_OBJECT_NIL && e) {
			CORBA_exception_init (&ev);
			GNOME_Speech_SpeechCallback_notify (s->callback,
							    e->type,
							    e->text_id,
							    e->offset,
							    &ev);
			g_free (e);
			e = NULL;
		}
		
		break;
	case eciWaveformBuffer:
	case eciPhonemeBuffer:
	case eciPhonemeIndexReply:
		break;
	}
	if (e)
		g_free (e);
	return eciDataProcessed;
}



static EloquenceSpeaker *
eloquence_speaker_from_servant (PortableServer_Servant *servant)
{
	return ELOQUENCE_SPEAKER(bonobo_object_from_servant (servant));
}



static void
eloquence_add_parameter (EloquenceSpeaker *eloquence_speaker,
			ECIVoiceParam param,
			const gchar *parameter_name,
			gdouble min,
			gdouble max,
			parameter_set_func func)
{
	Speaker *speaker = SPEAKER(eloquence_speaker);
	gdouble current;
	current = (gdouble)
		eciGetVoiceParam (eloquence_speaker->handle, 0, param);
	speaker_add_parameter (speaker,
			       parameter_name,
			       min,
			       current,
			       max,
			       func);
}



static CORBA_boolean
eloquence_registerSpeechCallback (PortableServer_Servant servant,
				 const GNOME_Speech_SpeechCallback callback,
				 CORBA_Environment *ev)
{
	EloquenceSpeaker *s = eloquence_speaker_from_servant (servant);

	/* Store reference to callback */

	s->callback = CORBA_Object_duplicate (callback, ev);

	/* Add a timeout callback to poke this instance of Eloquence */

	s->timeout_id = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, 100,
			    eloquence_speaker_timeout_callback, s, NULL);

        /* Set up ECI callback */

	eciRegisterCallback (s->handle,
			     eloquence_speaker_index_callback,
			     s);

	return TRUE;
}



static CORBA_long
eloquence_say (PortableServer_Servant servant,
	      const CORBA_char *text,
	      CORBA_Environment *ev)
{
	EloquenceSpeaker *speaker = eloquence_speaker_from_servant (servant);
  
	g_return_val_if_fail (speaker, FALSE);
	g_return_val_if_fail (speaker->handle != NULL_ECI_HAND, FALSE);

	text_id++;
	eciAddText (speaker->handle, text);
	eciSynthesize (speaker->handle);
	return text_id;
}



static CORBA_boolean
eloquence_stop (PortableServer_Servant servant,
	       CORBA_Environment *ev)
{
	EloquenceSpeaker *speaker = eloquence_speaker_from_servant (servant);
	g_return_val_if_fail (speaker->handle != NULL_ECI_HAND, FALSE);
	eciStop (speaker->handle);
	return TRUE;
}



static void
eloquence_speaker_init (EloquenceSpeaker *speaker)
{
	speaker->handle = NULL_ECI_HAND;
}



static void
eloquence_speaker_finalize (GObject *obj)
{
	EloquenceSpeaker *speaker = ELOQUENCE_SPEAKER (obj);

        /* Destroy the ECI instance */

	if (speaker->handle != NULL_ECI_HAND) 
	{
		eciDelete (speaker->handle);
	}

	/* Remove timeout */

	if (speaker->timeout_id >= 0)
		g_source_remove (speaker->timeout_id);

	if (parent_class->finalize)
		parent_class->finalize (obj);
}



static void
eloquence_speaker_class_init (EloquenceSpeaker *klass)
{
	SpeakerClass *class = SPEAKER_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = eloquence_speaker_finalize;

	/* Initialize parent class epv table */

	class->epv.say = eloquence_say;
	class->epv.stop = eloquence_stop;
	class->epv.registerSpeechCallback = eloquence_registerSpeechCallback;

	/* Add Index function */

	class->add_index = eloquence_speaker_add_index;
}




BONOBO_TYPE_FUNC (EloquenceSpeaker,
		  speaker_get_type (),
		  eloquence_speaker);



static gboolean
eloquence_set_gender (Speaker *speaker,
		     gdouble new_gender)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciSetVoiceParam (s->handle, 0, eciGender, (int) new_gender);
	return TRUE;
}



static gboolean
eloquence_set_head_size (Speaker *speaker,
			gdouble new_head_size)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciSetVoiceParam (s->handle, 0, eciHeadSize, (int) new_head_size);
	return TRUE;
}



static gboolean
eloquence_set_pitch (Speaker *speaker,
		    gdouble new_pitch)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciSetVoiceParam (s->handle, 0, eciPitchBaseline, (int) new_pitch);
	return TRUE;
}



static gboolean
eloquence_set_pitch_fluctuation (Speaker *speaker,
				gdouble new_pitch_fluctuation)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciSetVoiceParam (s->handle, 0, eciPitchFluctuation, (int) new_pitch_fluctuation);
	return TRUE;
}



static gboolean
eloquence_set_roughness (Speaker *speaker,
			gdouble new_roughness)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciSetVoiceParam (s->handle, 0, eciRoughness, (int) new_roughness);
	return TRUE;
}



static gboolean
eloquence_set_breathiness (Speaker *speaker,
			  gdouble new_breathiness)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciSetVoiceParam (s->handle, 0, eciBreathiness, (int) new_breathiness);
	return TRUE;
}



static gboolean
eloquence_set_rate (Speaker *speaker,
		   gdouble new_rate)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciSetVoiceParam (s->handle, 0, eciSpeed, (int) new_rate);
	return TRUE;
}



static gboolean
eloquence_set_volume (Speaker *speaker,
		     gdouble new_volume)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciSetVoiceParam (s->handle, 0, eciVolume, (int) new_volume);
	return TRUE;
}



static gboolean
eloquence_set_voice (Speaker *speaker,
		    gdouble new_voice)
{
	EloquenceSpeaker *s = ELOQUENCE_SPEAKER(speaker);
	eciCopyVoice (s->handle,
		      (int) new_voice,
		      0);
	return TRUE;
}



EloquenceSpeaker *
eloquence_speaker_new (const GNOME_Speech_VoiceInfo *voice_spec)
{
	EloquenceSpeaker *speaker;
	ECIHand handle;
	char name[ECI_VOICE_NAME_LENGTH];
	gint i;
	speaker = g_object_new (ELOQUENCE_SPEAKER_TYPE, NULL);
  
	/* Create a handle and initialize it */

	handle = eciNew ();

	/* Initialize the index queue  */

	speaker->index_queue = NULL;
	speaker->timeout_id = -1;
	
        /* Find the specified voice */

	for (i = 1; i <= ECI_PRESET_VOICES; i++) {
		char name[30];

		eciGetVoiceName (handle, i, name);
		if (!g_strcasecmp (voice_spec->name, name))
			eciCopyVoice (handle, i, 0);
	}

	speaker->handle = handle;

	/* Add parameters */

	eloquence_add_parameter (speaker,
				eciGender,
				"gender",
				0,
				1,
				eloquence_set_gender);
	eloquence_add_parameter (speaker,
				eciHeadSize,
				"head size",
				0,
				100,
				eloquence_set_head_size);
	eloquence_add_parameter (speaker,
				eciPitchBaseline,
				"pitch",
				0,
				100,
				eloquence_set_pitch);  
	eloquence_add_parameter (speaker,
				eciPitchFluctuation,
				"pitch fluctuation",
				0,
				100,
				eloquence_set_pitch_fluctuation);
	eloquence_add_parameter (speaker,
				eciRoughness,
				"roughness",
				0,
				100,
				eloquence_set_roughness);
	eloquence_add_parameter (speaker,
				eciBreathiness,
				"breathiness",
				0,
				100,
				eloquence_set_breathiness);  
	eloquence_add_parameter (speaker,
				eciSpeed,
				"rate",
				0,
				100,
				eloquence_set_rate);  
	eloquence_add_parameter (speaker,
				eciVolume,
				"volume",
				0,
				100,
				eloquence_set_volume);  

	/* Add the voice parameter */

	speaker_add_parameter (SPEAKER(speaker),
			       "voice",
			       1,
			       1,
			       ECI_PRESET_VOICES,
			       eloquence_set_voice);

	/* Add the voice names to the parameter */

	for (i = 1; i <= ECI_PRESET_VOICES; i++) {
		eciGetVoiceName (handle, i, name);
		speaker_add_parameter_value_description (SPEAKER(speaker),
							 "voice",
							 (gdouble) i,
							 name);
	}
  
	return speaker;
}




