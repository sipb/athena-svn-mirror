/*
 * GNOME Speech - Speech services for the GNOME desktop
 *
 * Copyright 2003 Sun Microsystems Inc.
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
 * dectalkspeaker.c: Implements the DectalkSpeaker object--
 *                 a GNOME Speech driver for Fonix's DECtalk TTS SDK
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libbonobo.h>
#include <glib/gmain.h>
#include <glib/gthread.h>
#include <gnome-speech/gnome-speech.h>
#include <dtk/ttsapi.h>
#include "dectalkspeaker.h"
#include "dectalksynthesisdriver.h"


static GObjectClass *parent_class;

extern struct voiceinfo voices[MAXVOICES];


static DectalkSpeaker *
dectalk_speaker_from_servant(PortableServer_Servant *servant)
{
	return(DECTALK_SPEAKER(bonobo_object_from_servant(servant)));
}


static CORBA_boolean
dectalk_registerSpeechCallback(PortableServer_Servant servant,
                               const GNOME_Speech_SpeechCallback callback,
                               CORBA_Environment *ev)
{
	DectalkSpeaker *s = dectalk_speaker_from_servant(servant);
	Speaker *speaker = SPEAKER (s);

	/* Store reference to callback */

	speaker->cb = CORBA_Object_duplicate(callback, ev);

	return TRUE;
}


static CORBA_long
dectalk_say(PortableServer_Servant servant,
            const CORBA_char *text,
            CORBA_Environment *ev)
{
	DectalkSpeaker *s = dectalk_speaker_from_servant (servant);
	Speaker *speaker = SPEAKER (s);

	return dectalk_synthesis_driver_say (DECTALK_SYNTHESIS_DRIVER(speaker->driver),
					     s, (gchar *) text);
}


static CORBA_boolean
dectalk_stop(PortableServer_Servant servant, CORBA_Environment *ev)
{
	DectalkSpeaker *s = dectalk_speaker_from_servant(servant);
	Speaker *speaker = SPEAKER (s);
	speaker->parameter_refresh = TRUE;
	return dectalk_synthesis_driver_stop (DECTALK_SYNTHESIS_DRIVER(speaker->driver));
}


static void
dectalk_wait(PortableServer_Servant servant, CORBA_Environment *ev)
{
	DectalkSpeaker *s = dectalk_speaker_from_servant(servant);
	Speaker *speaker = SPEAKER (s);
	dectalk_synthesis_driver_wait (DECTALK_SYNTHESIS_DRIVER(speaker->driver));
}


static CORBA_boolean
dectalk_isSpeaking(PortableServer_Servant servant, CORBA_Environment *ev)
{
	DectalkSpeaker *s = dectalk_speaker_from_servant(servant);
	Speaker *speaker = SPEAKER (s);
	speaker->parameter_refresh = TRUE;
	return dectalk_synthesis_driver_is_speaking (DECTALK_SYNTHESIS_DRIVER(speaker->driver));
}


static void
dectalk_speaker_init(DectalkSpeaker *speaker)
{
	speaker->voice = NULL;
}


static void
dectalk_speaker_finalize(GObject *obj)
{
	DectalkSpeaker *s = DECTALK_SPEAKER (obj);
	Speaker *speaker = SPEAKER (s);

	if (speaker->driver)
		g_object_unref (speaker->driver);

	if (s->voice)
		g_free (s->voice);
	if (parent_class->finalize)
		parent_class->finalize (obj);
}


static void
dectalk_speaker_class_init(DectalkSpeaker *klass)
{
	SpeakerClass *class = SPEAKER_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = dectalk_speaker_finalize;

	/* Initialize parent class epv table */

	class->epv.say = dectalk_say;
	class->epv.stop = dectalk_stop;
	class->epv.wait = dectalk_wait;
	class->epv.isSpeaking = dectalk_isSpeaking;
	class->epv.registerSpeechCallback = dectalk_registerSpeechCallback;
}


BONOBO_TYPE_FUNC(DectalkSpeaker, speaker_get_type(), dectalk_speaker);


static gboolean
dectalk_set_rate(Speaker *speaker, gdouble new_rate)
{
	gchar *command_string;

	command_string = g_strdup_printf ("[:rate %ld]", (long)new_rate);
	dectalk_synthesis_driver_speak_raw (DECTALK_SYNTHESIS_DRIVER(speaker->driver),
				      (gchar *) command_string);
	g_free (command_string);
	return TRUE;
}



static gboolean
dectalk_set_pitch(Speaker *speaker, gdouble new_pitch)
{
	gchar *command_string;
	
    
	command_string = g_strdup_printf ("[:dv ap %ld]", (long)new_pitch);
	dectalk_synthesis_driver_speak_raw (DECTALK_SYNTHESIS_DRIVER(speaker->driver),
				      (gchar *) command_string);
	g_free (command_string);
	return TRUE;
}



DectalkSpeaker *
dectalk_speaker_new(const GObject *driver,
		    const GNOME_Speech_VoiceInfo *voice_spec)
{
	DectalkSpeaker *speaker;
	Speaker *s;
	guint voice_num = 0;
	gint i;
	char *voice_names[] = {
		"Paul",
		"Betty",
		"Harry",
		"Frank",
		"Dennis",
		"Kit",
		"Ursula",
		"Rita",
		"Wendy"};    

	speaker = g_object_new(DECTALK_SPEAKER_TYPE, NULL);
	s = SPEAKER (speaker);

	s->driver = g_object_ref (G_OBJECT(driver));

	/* If call specified name, set the voice to that name */

	if (voice_spec->name) {
		for (i = 0; i < MAXVOICES; i++) {
			if (!strcmp (voice_spec->name, voices[i].description)) {
				voice_num = i;
			}
		}
	}
	else {
		voice_num = 0;
	}

	/* Set speaker's voice */

	speaker->voice = g_strdup_printf ("[:name %s]", voice_names[voice_num]);

        /* Add parameters */

	speaker_add_parameter(s, "rate", 75, 250, 600,
			      dectalk_set_rate);  


	speaker_add_parameter(s, "pitch", 50, voices[voice_num].pitch, 400,
			      dectalk_set_pitch);  

	return speaker;
}


