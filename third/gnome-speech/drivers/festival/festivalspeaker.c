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
 * festivalspeaker.c.c: Implementation of the FestivalSynthesisDriver
 *                            object-- a GNOME Speech driver for the Festival
 *                            Speech Synthesis System
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libbonobo.h>
#include "festivalsynthesisdriver.h"
#include "festivalspeaker.h"
 


static GObjectClass *parent_class;

static FestivalSpeaker *
festival_speaker_from_servant (PortableServer_Servant *servant)
{
	return FESTIVAL_SPEAKER(bonobo_object_from_servant (servant));
}



static CORBA_long
festival_say (PortableServer_Servant servant,
	      const CORBA_char *text,
	      CORBA_Environment *ev)
{
	FestivalSpeaker *speaker = festival_speaker_from_servant (servant);
	Speaker *s = SPEAKER (speaker);
	FestivalSynthesisDriver *d = FESTIVAL_SYNTHESIS_DRIVER(s->driver);

	return festival_synthesis_driver_say (d, speaker, (gchar *) text);
}



static CORBA_boolean
festival_stop (PortableServer_Servant servant,
	       CORBA_Environment *ev)
{
	FestivalSpeaker *speaker = festival_speaker_from_servant (servant);

	Speaker *s = SPEAKER (speaker);
	FestivalSynthesisDriver *d = FESTIVAL_SYNTHESIS_DRIVER(s->driver);

	return festival_synthesis_driver_stop (d);
}


static CORBA_boolean
festival_registerSpeechCallback (PortableServer_Servant servant,
				 const GNOME_Speech_SpeechCallback callback,
				 CORBA_Environment *ev)
{
	return FALSE;
}


static gboolean
festival_set_rate (Speaker *speaker,
		   gdouble new_value)
{
	FestivalSpeaker *festival_speaker = FESTIVAL_SPEAKER (speaker);
	Speaker *s = SPEAKER (festival_speaker);
	FestivalSynthesisDriver *d = FESTIVAL_SYNTHESIS_DRIVER (s->driver);
	gdouble stretch;
	gchar *command;

	stretch = 200/new_value;
	command = g_strdup_printf ("(Parameter.set 'Duration_Stretch %lf)\n", stretch);
	festival_synthesis_driver_say_raw (d, command);
	g_free (command);
	return TRUE;
}



static void
festival_speaker_init (FestivalSpeaker *s)
{
	s->voice = NULL;
}



static void
festival_speaker_finalize (GObject *obj)
{
	FestivalSpeaker *s = FESTIVAL_SPEAKER (obj);
	Speaker *speaker = SPEAKER (s);

	if (speaker->driver)
		g_object_unref (speaker->driver);

	if (s->voice)
		g_free (s->voice);

	if (parent_class->finalize)
		parent_class->finalize (obj);
}



static void
festival_speaker_class_init (FestivalSpeakerClass *klass)
{
	SpeakerClass *class = SPEAKER_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = festival_speaker_finalize;

	/* Initialize parent class epv table */

	class->epv.say = festival_say;
	class->epv.stop = festival_stop;
	class->epv.registerSpeechCallback = festival_registerSpeechCallback;
}




BONOBO_TYPE_FUNC (FestivalSpeaker,
		  speaker_get_type (),
		  festival_speaker);



FestivalSpeaker *
festival_speaker_new (GObject *driver,
		      const GNOME_Speech_VoiceInfo *info)
{
	FestivalSpeaker *speaker;
	Speaker *s;
	
	speaker = g_object_new (FESTIVAL_SPEAKER_TYPE, NULL);
	s = SPEAKER (speaker);
       	s->driver = g_object_ref (driver);

	/* Add supported parameters */

	speaker_add_parameter (s,
					"rate",
					1,
					200,
					400,
					festival_set_rate);

	/* Set voice */
		
	if (!strcmp (info->name, "Kevin"))
		speaker->voice = g_strdup ("(voice_ked_diphone)\n");
	else if (!strcmp (info->name, "Kal"))
		speaker->voice = g_strdup ("(voice_kal_diphone)\n");
	return speaker;
}




