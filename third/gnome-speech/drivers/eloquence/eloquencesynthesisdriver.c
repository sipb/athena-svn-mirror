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
 * eloquencesynthesisdriver.c: Implements the EloquenceSynthesisDriver object--
 *                    a GNOME Speech driver for SpeechWork's Eloquence TTS SDK
 *
 */

#include <libbonobo.h>
#include <glib/gmain.h>
#include <gnome-speech/gnome-speech.h>
#include "eloquencesynthesisdriver.h"
#include "eloquencespeaker.h"
#include <eci.h>
 

#define VERSION_LENGTH 20

static GObjectClass *parent_class;

static CORBA_string
eloquence__get_driverName (PortableServer_Servant servant,
			  CORBA_Environment * ev)
{
	return CORBA_string_dup ("SpeechWorks Eloquence GNOME Speech Driver");
}




static CORBA_string
eloquence__get_synthesizerName (PortableServer_Servant servant,
			       CORBA_Environment * ev)
{
	return CORBA_string_dup ("SpeechWorks Eloquence");
}



static CORBA_string
eloquence__get_driverVersion (PortableServer_Servant aservant,
			     CORBA_Environment * ev)
{
	return CORBA_string_dup ("0.2");
}



static CORBA_string
eloquence__get_synthesizerVersion (PortableServer_Servant
				  servant,
				  CORBA_Environment * ev)
{
	char version[VERSION_LENGTH];
	
	eciVersion (version);

	return CORBA_string_dup (version);
}



static GSList *
get_voice_list (void)
{
	GSList *l = NULL;
	ECIHand tmp_handle;
	int i;
  
	tmp_handle = eciNew ();
	if (tmp_handle == NULL_ECI_HAND)
		return NULL;

	for (i = 1; i <= ECI_PRESET_VOICES; i++) {
		GNOME_Speech_VoiceInfo *info;
		char tmp[ECI_VOICE_NAME_LENGTH + 1];
		int gender;
    
		info = GNOME_Speech_VoiceInfo__alloc ();
		eciGetVoiceName (tmp_handle, i, tmp);
		info->language = 0;
		info->name = CORBA_string_dup (tmp);
		gender = eciGetVoiceParam (tmp_handle, i, eciGender);
		if (gender == 0)
			info->gender = GNOME_Speech_gender_male;
		else if (gender == 1)
			info->gender = GNOME_Speech_gender_female;
		l = g_slist_prepend (l, info);
	}
	l = g_slist_reverse (l);
	eciDelete (tmp_handle);
	return l;
}



static void
voice_list_free (GSList *l)
{
	GSList *tmp = l;

	while (tmp) {
		CORBA_free (tmp->data);
		tmp = tmp->next;
	}
	g_slist_free (l);
}



static GNOME_Speech_VoiceInfoList *
voice_info_list_from_voice_list (GSList *l)
{
	int i = 0;
	GNOME_Speech_VoiceInfoList *rv = GNOME_Speech_VoiceInfoList__alloc ();
  
	if (!l)
	{
	        rv->_length = 0;
		return rv;
	}

	rv->_length = rv->_maximum = g_slist_length (l);
	rv->_buffer = GNOME_Speech_VoiceInfoList_allocbuf (rv->_length);

	while (l) {
		GNOME_Speech_VoiceInfo *info =
			(GNOME_Speech_VoiceInfo *) l->data;
		rv->_buffer[i].name = CORBA_string_dup (info->name);
		rv->_buffer[i].gender = info->gender;
		rv->_buffer[i].language = info->language;
		i++;
		l = l->next;
	}
	return rv;
}



static GNOME_Speech_VoiceInfoList *
eloquence_getVoices (PortableServer_Servant servant,
		    const GNOME_Speech_VoiceInfo *voice_spec,
		    CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	l = get_voice_list ();
	rv = voice_info_list_from_voice_list (l);
	voice_list_free (l);
	return rv;
}



static GNOME_Speech_VoiceInfoList *
eloquence_getAllVoices (PortableServer_Servant servant,
		       CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	l = get_voice_list ();
	rv = voice_info_list_from_voice_list (l);
	voice_list_free (l);
	return rv;
}



static GNOME_Speech_Speaker
eloquence_createSpeaker (PortableServer_Servant servant,
			const GNOME_Speech_VoiceInfo *voice_spec,
			CORBA_Environment *ev)
{
	EloquenceSpeaker *s = eloquence_speaker_new (voice_spec);
	return CORBA_Object_duplicate(bonobo_object_corba_objref(BONOBO_OBJECT(s)), ev);
}



static void
eloquence_synthesis_driver_init (EloquenceSynthesisDriver *driver)
{
}



static void
eloquence_synthesis_driver_finalize (GObject *obj)
{
	if (parent_class->finalize)
		parent_class->finalize (obj);
}



static void
eloquence_synthesis_driver_class_init (EloquenceSynthesisDriverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = eloquence_synthesis_driver_finalize;

	/* Initialize parent class epv table */

	klass->epv._get_driverName = eloquence__get_driverName;
	klass->epv._get_driverVersion = eloquence__get_driverVersion;
	klass->epv._get_synthesizerName = eloquence__get_synthesizerName;
	klass->epv._get_synthesizerVersion = eloquence__get_synthesizerVersion;
	klass->epv.getVoices = eloquence_getVoices;
	klass->epv.getAllVoices = eloquence_getAllVoices;
	klass->epv.createSpeaker = eloquence_createSpeaker;  
}




BONOBO_TYPE_FUNC_FULL (EloquenceSynthesisDriver,
		       GNOME_Speech_SynthesisDriver,
		       bonobo_object_get_type (),
		       eloquence_synthesis_driver);



EloquenceSynthesisDriver *
eloquence_synthesis_driver_new (void)
{
	EloquenceSynthesisDriver *driver;
	driver = g_object_new (ELOQUENCE_SYNTHESIS_DRIVER_TYPE, NULL);
	return driver;
}




int
main (int argc,
      char **argv)
{
	EloquenceSynthesisDriver *driver;
	char *obj_id;
	int ret;
  
	if (!bonobo_init (&argc, argv)) {
		g_error ("Could not initialize Bonobo Activation / Bonobo");
	}

	/* Initialize threads */  

	g_thread_init (NULL);

	obj_id = "OAFIID:GNOME_Speech_SynthesisDriver_Eloquence:proto0.2";

	driver = eloquence_synthesis_driver_new ();
	if (!driver)
		g_error ("Error creating speech synthesis driver object.\n");

	ret = bonobo_activation_active_server_register (
                obj_id,
                bonobo_object_corba_objref (bonobo_object (driver)));

	if (ret != Bonobo_ACTIVATION_REG_SUCCESS)
		g_error ("Error registering speech synthesis driver.\n");
	else
		bonobo_main ();
	return 0;
}



