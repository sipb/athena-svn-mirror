/*
 *
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
 * festivalsynthesisdriver.c: Implementation of the FestivalSynthesisDriver
 *                            object-- a GNOME Speech driver for the Festival
 *                            Speech Synthesis System
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <libbonobo.h>
#include "festivalsynthesisdriver.h"
#include "festivalspeaker.h"
 


static gint text_id = 0;
static GObjectClass *parent_class;

static gboolean festival_server_exists = FALSE;

static gchar *
festival_get_version (void)
{
	gchar *version;
	char buf[81];
	gchar *start, *end;
	FILE *festival;

	festival = popen ("festival --version", "r");
	if (!festival)
		return NULL;
	fgets (buf, 80, festival);
	pclose (festival);
	start = strstr (buf, "System: ");
	if (!start)
		return NULL;
	end = strstr (start+8, ":");
	version = g_strndup (start+8, end-start-8);
	return version;
}


static gboolean
festival_start (FestivalSynthesisDriver *d)
{
	int festival_input[2];
	int festival_output[2];
	gchar *s = NULL;
	
	if ((s = g_find_program_in_path ("festival")) != NULL)
	{
	    g_free (s);
	    festival_server_exists = TRUE;
	}
	else
	{
	    festival_server_exists = FALSE;
	    return FALSE;
	}

	/* Create pipes */

	pipe (festival_input);
	pipe (festival_output);

        /* Fork festival process */

	d->pid = fork ();
	if (!d->pid) {
		close (0);
		close (1);
		close (2);
		dup (festival_input[0]);
		close (festival_input[1]);
		dup (festival_output[1]);
		dup (festival_output[1]);
		close (festival_output[0]);
		execlp ("festival", "festival",  "--pipe", NULL);
	}
	
	close (festival_input[0]);
	close (festival_output[1]);

	d->festival_output = festival_output[0];
	d->festival_input = festival_input[1];
	
	/* Set things up for async mode */

	if (festival_server_exists)
	    festival_synthesis_driver_say_raw (d, "(audio_mode 'async)\n");

	return festival_server_exists;
}


static void
festival_stop (FestivalSynthesisDriver *d)
{
	close (d->festival_input);
	close (d->festival_output);
	d->festival_input = d->festival_output = -1;

	kill (d->pid, SIGTERM);
	waitpid (d->pid, NULL, 0);
	d->pid = 0;
}


static FestivalSynthesisDriver *
festival_synthesis_driver_from_servant (PortableServer_Servant *servant)
{
	return FESTIVAL_SYNTHESIS_DRIVER(bonobo_object_from_servant(servant));
}


static CORBA_string
festival__get_driverName (PortableServer_Servant servant,
			  CORBA_Environment * ev)
{
	return CORBA_string_dup ("Festival GNOME Speech Driver");  
}




static CORBA_string
festival__get_synthesizerName (PortableServer_Servant servant,
			       CORBA_Environment * ev)
{
	return CORBA_string_dup ("Festival Speech Synthesis System");
}



static CORBA_string
festival__get_driverVersion (PortableServer_Servant aservant,
			     CORBA_Environment * ev)
{
	return CORBA_string_dup ("0.3");
}



static CORBA_string
festival__get_synthesizerVersion (PortableServer_Servant servant,
				  CORBA_Environment * ev)
{
	FestivalSynthesisDriver *d = festival_synthesis_driver_from_servant (servant);
	return CORBA_string_dup (d->version);
}



static GSList *
get_voice_list (void)
{
	GSList *l = NULL;
	GNOME_Speech_VoiceInfo *info;	

	if (festival_server_exists)
	{
	    info = GNOME_Speech_VoiceInfo__alloc ();
	    info->language = CORBA_string_dup("en_US");
	    info->name = CORBA_string_dup ("Kevin");
	    info->gender = GNOME_Speech_gender_male;
	    l = g_slist_prepend (l, info);
	    info = GNOME_Speech_VoiceInfo__alloc ();
	    info->language = CORBA_string_dup("en_US");
	    info->name = CORBA_string_dup ("Kal");
	    info->gender = GNOME_Speech_gender_male;
	    l = g_slist_prepend (l, info);
	    l = g_slist_reverse (l);
	}
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



static GSList *
prune_voice_list (GSList *l,
		  const GNOME_Speech_VoiceInfo *info)
{
	GSList *cur, *next;
	GNOME_Speech_VoiceInfo *i;

	cur = l;
	while (cur) {
		i = (GNOME_Speech_VoiceInfo *) cur->data;
		next = cur->next;
		if (strlen(info->name))
			if (strcmp (i->name, info->name)) {
				CORBA_free (i);
				l = g_slist_remove_link (l, cur);
				cur = next;
				continue;
			}
		if (strlen(info->language))
			if (strcmp (i->language, info->language)) {
				CORBA_free (i);
				l = g_slist_remove_link (l, cur);
				cur = next;
				continue;
			}
		cur = next;
	}
	return l;
}


static GNOME_Speech_VoiceInfoList *
voice_info_list_from_voice_list (GSList *l)
{
	int i = 0;
	GNOME_Speech_VoiceInfoList *rv = GNOME_Speech_VoiceInfoList__alloc ();
  
	if (!l) {
		rv->_length = rv->_maximum = 0;
		return rv ;
	}

	rv->_length = rv->_maximum = g_slist_length (l);
	rv->_buffer = GNOME_Speech_VoiceInfoList_allocbuf (rv->_length);

	while (l) {
		GNOME_Speech_VoiceInfo *info =
			(GNOME_Speech_VoiceInfo *) l->data;
		rv->_buffer[i].name = CORBA_string_dup (info->name);
		rv->_buffer[i].gender = info->gender;
		rv->_buffer[i].language = CORBA_string_dup(info->language);
		i++;
		l = l->next;
	}
	return rv;
}



static GNOME_Speech_VoiceInfoList *
festival_getVoices (PortableServer_Servant servant,
		    const GNOME_Speech_VoiceInfo *voice_spec,
		    CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	l = get_voice_list ();
	l = prune_voice_list (l, voice_spec);
	rv = voice_info_list_from_voice_list (l);
	voice_list_free (l);
	return rv;
}



static GNOME_Speech_VoiceInfoList *
festival_getAllVoices (PortableServer_Servant servant,
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
festival_createSpeaker (PortableServer_Servant servant,
			const GNOME_Speech_VoiceInfo *voice_spec,
			CORBA_Environment *ev)
{
	FestivalSynthesisDriver *d = festival_synthesis_driver_from_servant (servant);
	FestivalSpeaker *s;
	GNOME_Speech_VoiceInfo *info;
	GSList *l;
	
	l = get_voice_list ();
	l = prune_voice_list (l, voice_spec);
	
	if (l)
		info = l->data;
	else
		info = NULL;

	s = festival_speaker_new (G_OBJECT(d), info);
	return CORBA_Object_duplicate(bonobo_object_corba_objref (BONOBO_OBJECT(s)), ev);
}



static CORBA_boolean
festival_driverInit (PortableServer_Servant servant,
		     CORBA_Environment *ev)
{
	FestivalSynthesisDriver *d = festival_synthesis_driver_from_servant (servant);

	if (d->initialized)
		return TRUE;

	d->version = festival_get_version ();
	if (!d->version)
		return FALSE;
	else
	if (!festival_start (d)) {
		festival_stop (d);
		return FALSE;
	}

	d->initialized = TRUE;
	return TRUE;
}


static CORBA_boolean
festival_isInitialized (PortableServer_Servant servant,
			CORBA_Environment *ev)
{
	FestivalSynthesisDriver *d = festival_synthesis_driver_from_servant (servant);

	return d->initialized;
}


static void
festival_synthesis_driver_init (FestivalSynthesisDriver *d)
{
	d->festival_input  = d->festival_output = -1;
	d->version = NULL;
	d->last_speaker = NULL;
	d->initialized = FALSE;
}




static void
festival_synthesis_driver_finalize (GObject *obj)
{
	FestivalSynthesisDriver *d = FESTIVAL_SYNTHESIS_DRIVER (obj);

	if (d->version)
		g_free (d->version);
	if (d->festival_input) {

		/* Tell the festival binary we're done */

		festival_synthesis_driver_say_raw (d, "(exit)\n");
		
		close (d->festival_input);
		close (d->festival_output);
	}
	waitpid (d->pid, NULL, 0);
	if (parent_class->finalize)
		parent_class->finalize (obj);
	printf ("Festival driver finalized.\n");
	bonobo_main_quit ();
}


static void
festival_synthesis_driver_class_init (FestivalSynthesisDriverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = festival_synthesis_driver_finalize;
	
	/* Initialize epv table */

	klass->epv._get_driverName = festival__get_driverName;
	klass->epv._get_driverVersion = festival__get_driverVersion;
	klass->epv._get_synthesizerName = festival__get_synthesizerName;
	klass->epv._get_synthesizerVersion = festival__get_synthesizerVersion;
	klass->epv.getVoices = festival_getVoices;
	klass->epv.getAllVoices = festival_getAllVoices;
	klass->epv.createSpeaker = festival_createSpeaker;
	klass->epv.driverInit = festival_driverInit;
	klass->epv.isInitialized = festival_isInitialized;
}



void
festival_synthesis_driver_say_raw (FestivalSynthesisDriver *d,
			       gchar *text)
{
	int l = strlen (text);
	write (d->festival_input, text, l);
}


gint
festival_synthesis_driver_say (FestivalSynthesisDriver *d,
			       FestivalSpeaker *s,
			       gchar *text)
{
	gchar *escaped_string;
	gchar *ptr1, *ptr2;
	
	escaped_string = g_malloc (strlen (text)*2+1);
	ptr1 = text;
	ptr2 = escaped_string;
	while (ptr1 && *ptr1) {
		if (*ptr1 == '\"')
			*ptr2++ = '\\';
		*ptr2++ = *ptr1++;
	}
	*ptr2 = 0;

	/* Refresh if needded */

	if (d->last_speaker != s || speaker_needs_parameter_refresh (SPEAKER(s))) {
		if (!d->last_speaker || strcmp (d->last_speaker->voice, s->voice))
			festival_synthesis_driver_say_raw (d, s->voice);
		speaker_refresh_parameters (SPEAKER(s));
		d->last_speaker = s;
	}
	festival_synthesis_driver_say_raw (d, "(SayText \"");
	festival_synthesis_driver_say_raw (d, escaped_string);
	festival_synthesis_driver_say_raw (d, "\")\r\n");
	if (escaped_string)
		g_free (escaped_string);
	return text_id++;
}


gboolean
festival_synthesis_driver_stop (FestivalSynthesisDriver *d)
{
	festival_synthesis_driver_say_raw (d, "(audio_mode 'shutup)\n");
	return TRUE;
}


BONOBO_TYPE_FUNC_FULL (FestivalSynthesisDriver,
		       GNOME_Speech_SynthesisDriver,
		       bonobo_object_get_type (),
		       festival_synthesis_driver);



FestivalSynthesisDriver *
festival_synthesis_driver_new (void)
{
	FestivalSynthesisDriver *driver;
	driver = g_object_new (FESTIVAL_SYNTHESIS_DRIVER_TYPE, NULL);
	return driver;
}




int
main (int argc,
      char **argv)
{
	FestivalSynthesisDriver *driver;
	char *obj_id;
	int ret;

	if (!bonobo_init (&argc, argv))
	{
		g_error ("Could not initialize Bonobo Activation / Bonobo");
	}

	obj_id = "OAFIID:GNOME_Speech_SynthesisDriver_Festival:proto0.3";

	driver = festival_synthesis_driver_new ();

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



