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
 * thetaspeaker.c: Implements the Thetaspeaker object--
 *                 a GNOME Speech driver for Cepstal's Theta TTS Engine
 *
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <libbonobo.h>
#include <glib/gmain.h>
#include <glib/gthread.h>
#include <gnome-speech/gnome-speech.h>
#include <theta.h>
#include "thetaspeaker.h"
#include "thetasynthesisdriver.h"


static GObjectClass *parent_class;

static ThetaSpeaker *
theta_speaker_from_servant(PortableServer_Servant *servant)
{
	return (THETA_SPEAKER(bonobo_object_from_servant(servant)));
}


static CORBA_boolean
theta_registerSpeechCallback(PortableServer_Servant servant,
                               const GNOME_Speech_SpeechCallback callback,
                               CORBA_Environment *ev)
{
	ThetaSpeaker *s = theta_speaker_from_servant(servant);
	Speaker *speaker = SPEAKER (s);

	/* Store reference to callback */

	speaker->cb = CORBA_Object_duplicate(callback, ev);

	return TRUE;
}


static CORBA_long
theta_say(PortableServer_Servant servant,
            const CORBA_char *text,
            CORBA_Environment *ev)
{
	ThetaSpeaker *s = theta_speaker_from_servant (servant);
	Speaker *speaker = SPEAKER (s);
	ThetaSynthesisDriver *d = THETA_SYNTHESIS_DRIVER (speaker->driver);

	if (speaker_needs_parameter_refresh (speaker))
		speaker_refresh_parameters (speaker);
	return theta_synthesis_driver_say (d, s, text);
}


static CORBA_boolean
theta_stop(PortableServer_Servant servant, CORBA_Environment *ev)
{
	ThetaSpeaker *s = theta_speaker_from_servant(servant);
	Speaker *speaker = SPEAKER (s);
	ThetaSynthesisDriver *d = THETA_SYNTHESIS_DRIVER (speaker->driver);
	return theta_synthesis_driver_stop (d);
}


static CORBA_boolean
theta_isSpeaking(PortableServer_Servant servant, CORBA_Environment *ev)
{
	ThetaSpeaker *s = theta_speaker_from_servant(servant);
	Speaker *speaker = SPEAKER (s);
	ThetaSynthesisDriver *d = THETA_SYNTHESIS_DRIVER (speaker->driver);
	return theta_synthesis_driver_is_speaking (d);
}


static void
theta_speaker_init(ThetaSpeaker *speaker)
{
}


static void
theta_speaker_finalize(GObject *obj)
{
	ThetaSpeaker *s = THETA_SPEAKER (obj);
	Speaker *speaker = SPEAKER (s);

	theta_unload_voice (s->vox);
	if (speaker->driver)
		g_object_unref (speaker->driver);

	if (parent_class->finalize)
		parent_class->finalize (obj);
}


static void
theta_speaker_class_init(ThetaSpeakerClass *klass)
{
	SpeakerClass *class = SPEAKER_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = theta_speaker_finalize;

	/* Initialize parent class epv table */

	class->epv.say = theta_say;
	class->epv.stop = theta_stop;
	class->epv.isSpeaking = theta_isSpeaking;
	class->epv.registerSpeechCallback = theta_registerSpeechCallback;
}


static gboolean
theta_set_rate(Speaker *speaker, gdouble new_rate)
{
	ThetaSpeaker *s = THETA_SPEAKER (speaker);
	
	theta_set_rate_stretch (s->vox, 250/new_rate, NULL);
	return TRUE;
}



static gboolean
theta_set_pitch(Speaker *speaker, gdouble new_pitch)
{
	ThetaSpeaker *s = THETA_SPEAKER (speaker);
	gdouble pitch_shift;

	pitch_shift = log(new_pitch/100)/log(2);
	theta_set_pitch_shift (s->vox, pitch_shift, NULL);
	return TRUE;
}



static gboolean
theta_set_volume(Speaker *speaker, gdouble new_pitch)
{
	ThetaSpeaker *s = THETA_SPEAKER (speaker);
	gdouble volume;

	volume = new_pitch/100;
	theta_set_rescale (s->vox, volume, NULL);
	return TRUE;
}



BONOBO_TYPE_FUNC(ThetaSpeaker, speaker_get_type(), theta_speaker);


ThetaSpeaker *
theta_speaker_new(const GObject *driver,
		    const GNOME_Speech_VoiceInfo *voice_spec)
{
	ThetaSpeaker *speaker;
	Speaker *s;
	theta_voice_desc *vlist, *vdesc;
	
	speaker = g_object_new(THETA_SPEAKER_TYPE, NULL);
	s = SPEAKER (speaker);

	s->driver = g_object_ref (G_OBJECT(driver));
	
	/* Get a voice */

	vlist = theta_enum_voices (NULL, NULL);

	for (vdesc = vlist; vdesc; vdesc = vdesc->next) {
	if (!strcmp (vdesc->human, voice_spec->name))
		speaker->vox = theta_load_voice (vdesc);
	}
	theta_free_voicelist (vlist);

	/* Add parameters */

	speaker_add_parameter (s, "rate", 50, 250, 500,
			       theta_set_rate);
	
	speaker_add_parameter (s, "pitch", 50, 100, 400,
			       theta_set_pitch);
	
	speaker_add_parameter (s, "volume", 0, 100, 100,
			       theta_set_volume);
	
	return speaker;
}


