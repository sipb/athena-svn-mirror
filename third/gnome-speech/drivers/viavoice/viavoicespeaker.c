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
 * viavoicespeaker.c: Implements the ViavoiceSpeaker object--
 *                            a GNOME Speech driver for IBM's Viavoice TTS RTK
 *
 */

#include <unistd.h>
#include <libbonobo.h>
#include <glib/gmain.h>
#include <gnome-speech/gnome-speech.h>
#include <eci.h>
#include "viavoicesynthesisdriver.h"
#include "viavoicespeaker.h"



static GObjectClass *parent_class;

static ViavoiceSpeaker *
viavoice_speaker_from_servant (PortableServer_Servant *servant)
{
	return VIAVOICE_SPEAKER(bonobo_object_from_servant (servant));
}



static void
viavoice_add_parameter (ECIHand handle,
			ViavoiceSpeaker *viavoice_speaker,
			enum ECIVoiceParam param,
			const gchar *parameter_name,
			gdouble min,
			gdouble max,
			parameter_set_func func)
{
	Speaker *speaker = SPEAKER(viavoice_speaker);
	gdouble current;
	current = (gdouble)
		eciGetVoiceParam (handle, 0, param);
	speaker_add_parameter (speaker,
			       parameter_name,
			       min,
			       current,
			       max,
			       func);
}



static CORBA_boolean
viavoice_registerSpeechCallback (PortableServer_Servant servant,
				 const GNOME_Speech_SpeechCallback callback,
				 CORBA_Environment *ev)
{
	ViavoiceSpeaker *s = viavoice_speaker_from_servant (servant);
	Speaker *speaker = SPEAKER (s);
	
	/* Store reference to callback */

	speaker->cb = CORBA_Object_duplicate (callback, ev);

	return TRUE;
}



static CORBA_long
viavoice_say (PortableServer_Servant servant,
	      const CORBA_char *text,
	      CORBA_Environment *ev)
{
	ViavoiceSpeaker *speaker = viavoice_speaker_from_servant (servant);
	Speaker *s = SPEAKER (speaker);
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER(s->driver);

	return viavoice_synthesis_driver_say (d, speaker, (gchar *) text);
}



static CORBA_boolean
viavoice_stop (PortableServer_Servant servant,
	       CORBA_Environment *ev)
{
	ViavoiceSpeaker *speaker = viavoice_speaker_from_servant (servant);
	Speaker *s = SPEAKER (speaker);
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER(s->driver);

	return viavoice_synthesis_driver_stop (d);
}


static CORBA_boolean
viavoice_isSpeaking (PortableServer_Servant servant,
		     CORBA_Environment *ev)
{
	ViavoiceSpeaker *speaker = viavoice_speaker_from_servant (servant);
	Speaker *s = SPEAKER (speaker);
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER(s->driver);
	return viavoice_synthesis_driver_is_speaking (d);
}


static void
viavoice_wait (PortableServer_Servant servant,
	       CORBA_Environment *ev)
{
	ViavoiceSpeaker *speaker = viavoice_speaker_from_servant (servant);
	Speaker *s = SPEAKER (speaker);
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER(s->driver);
	viavoice_synthesis_driver_wait (d);
}


static void
viavoice_speaker_init (ViavoiceSpeaker *speaker)
{
}



static void
viavoice_speaker_finalize (GObject *obj)
{
	ViavoiceSpeaker *s = VIAVOICE_SPEAKER (obj);
	Speaker *speaker = SPEAKER (s);

	if (speaker->driver)
		g_object_unref (speaker->driver);

	if (parent_class->finalize)
		parent_class->finalize (obj);
}



static void
viavoice_speaker_class_init (ViavoiceSpeaker *klass)
{
	SpeakerClass *class = SPEAKER_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = viavoice_speaker_finalize;

	/* Initialize parent class epv table */

	class->epv.say = viavoice_say;
	class->epv.stop = viavoice_stop;
	class->epv.wait = viavoice_wait;
	class->epv.isSpeaking = viavoice_isSpeaking;
	class->epv.registerSpeechCallback = viavoice_registerSpeechCallback;
}




BONOBO_TYPE_FUNC (ViavoiceSpeaker,
		  speaker_get_type (),
		  viavoice_speaker);



static gboolean
viavoice_set_gender (Speaker *speaker,
		     gdouble new_gender)
{
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER (speaker->driver);

	viavoice_synthesis_driver_set_voice_param (d, eciGender, (int) new_gender);
	return TRUE;
}



static gboolean
viavoice_set_head_size (Speaker *speaker,
			gdouble new_head_size)
{
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER (speaker->driver);

	viavoice_synthesis_driver_set_voice_param (d, eciHeadSize, (int) new_head_size);
	return TRUE;
}



static gboolean
viavoice_set_pitch (Speaker *speaker,
		    gdouble new_pitch)
{
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER (speaker->driver);

	viavoice_synthesis_driver_set_voice_param (d, eciPitchBaseline, (int) new_pitch);
	return TRUE;
}



static gboolean
viavoice_set_pitch_fluctuation (Speaker *speaker,
				gdouble new_pitch_fluctuation)
{
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER (speaker->driver);

	viavoice_synthesis_driver_set_voice_param (d, eciPitchFluctuation, (int) new_pitch_fluctuation);
	return TRUE;
}



static gboolean
viavoice_set_roughness (Speaker *speaker,
			gdouble new_roughness)
{
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER (speaker->driver);

	viavoice_synthesis_driver_set_voice_param (d, eciRoughness, (int) new_roughness);
	return TRUE;
}



static gboolean
viavoice_set_breathiness (Speaker *speaker,
			  gdouble new_breathiness)
{
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER (speaker->driver);

	viavoice_synthesis_driver_set_voice_param (d, eciBreathiness, (int) new_breathiness);
	return TRUE;
}



static gboolean
viavoice_set_rate (Speaker *speaker,
		   gdouble new_rate)
{
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER (speaker->driver);

	viavoice_synthesis_driver_set_voice_param (d, eciSpeed, (int) new_rate);
	return TRUE;
}



static gboolean
viavoice_set_volume (Speaker *speaker,
		     gdouble new_volume)
{
	ViavoiceSynthesisDriver *d = VIAVOICE_SYNTHESIS_DRIVER (speaker->driver);

	viavoice_synthesis_driver_set_voice_param (d, eciVolume, (int) new_volume);
	return TRUE;
}



ViavoiceSpeaker *
viavoice_speaker_new (GObject *d,
		      const GNOME_Speech_VoiceInfo *voice_spec)
{
	ViavoiceSpeaker *speaker;
	Speaker *s;
	ECIHand handle;
	gint i;
	gint voice_num = 1;
	speaker = g_object_new (VIAVOICE_SPEAKER_TYPE, NULL);
	s = SPEAKER(speaker);

	/* Create a handle and initialize it */

	handle = eciNew ();

	s->driver = g_object_ref (d);

        /* Find the specified voice */

	for (i = 1; i <= ECI_PRESET_VOICES; i++) {
		char name[30];

		eciGetVoiceName (handle, i, name);
		if (!g_strcasecmp (voice_spec->name, name))
			voice_num = i;
	}

	/* Set temporary handle to that voice */

	eciCopyVoice (handle, voice_num, 0);

        /* Add parameters */

	viavoice_add_parameter (handle, speaker,
				eciGender,
				"gender",
				0,
				1,
				viavoice_set_gender);
	viavoice_add_parameter (handle, speaker,
				eciHeadSize,
				"head size",
				0,
				100,
				viavoice_set_head_size);
	viavoice_add_parameter (handle, speaker,
				eciPitchBaseline,
				"pitch",
				0,
				100,
				viavoice_set_pitch);  
	viavoice_add_parameter (handle, speaker,
				eciPitchFluctuation,
				"pitch fluctuation",
				0,
				100,
				viavoice_set_pitch_fluctuation);
	viavoice_add_parameter (handle, speaker,
				eciRoughness,
				"roughness",
				0,
				100,
				viavoice_set_roughness);
	viavoice_add_parameter (handle, speaker,
				eciBreathiness,
				"breathiness",
				0,
				100,
				viavoice_set_breathiness);  
	viavoice_add_parameter (handle, speaker,
				eciSpeed,
				"rate",
				0,
				100,
				viavoice_set_rate);  
	viavoice_add_parameter (handle, speaker,
				eciVolume,
				"volume",
				0,
				100,
				viavoice_set_volume);  

	eciDelete (handle);
	return speaker;
}




