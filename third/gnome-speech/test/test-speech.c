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
 * test-speech.c: Test suite for GNOME Speech
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib/gmain.h>
#include <libbonobo.h>
#include <gnome-speech/gnome-speech.h>
#include "callback.h"

static void
speaker_say_print (GNOME_Speech_Speaker speaker,
		   const char                  *format,
		   ...)
{
	va_list args;
	char *str;
	CORBA_Environment ev;
  
	va_start (args, format);

	CORBA_exception_init (&ev);
  
	str = g_strdup_vprintf (format, args);
	printf ("%s", str);
	GNOME_Speech_Speaker_say (speaker, str, &ev);
	if (BONOBO_EX (&ev))
		printf ("Exception %s writing '%s'",
			bonobo_exception_get_text (&ev), str);
	g_free (str);

	CORBA_exception_free (&ev);
	
	va_end (args);
}

static CORBA_Object
select_server (CORBA_Environment *ev)
{
  
	int i;
	Bonobo_ServerInfoList *servers;
	Bonobo_ServerInfo *info;
	char input[81];
	CORBA_Object rv;
  
	servers = bonobo_activation_query (
		"repo_ids.has ('IDL:GNOME/Speech/SynthesisDriver:0.3')",
		NULL, ev);
	if (BONOBO_EX (ev)) {
		return CORBA_OBJECT_NIL;
	}

	if (!servers)
	{
		return CORBA_OBJECT_NIL;
	}
	for (i = 0; i < servers->_length; i++)
	{
		info = &servers->_buffer[i];
		printf ("%d: %s\n", i+1, info->iid);
	}
	printf ("\nSelect a server: ");
	fgets (input, 80, stdin);
	i = atoi (input);
	i--;
	if (i < 0 || i > servers->_length)
	{
		printf ("Server not found.\n");
		CORBA_free (servers);
		return CORBA_OBJECT_NIL;
	}
	info = &servers->_buffer[i];
	printf ("Attempting to activate %s.\n", info->iid);
	rv = bonobo_activation_activate_from_id (
		(const Bonobo_ActivationID) info->iid,
		0, NULL, ev);
	if (BONOBO_EX (ev)) printf ("Error activating service: %s\n", bonobo_exception_get_text (ev));
	CORBA_free (servers);

	return rv;
}

static void
speech_interrupt_test (GNOME_Speech_Speaker s)
{
        gint i, j;
	gint delay;
	CORBA_Environment ev;
	
	CORBA_exception_init (&ev);

	for (i = 0; i < 100; i++) {
		gchar *str = g_strdup_printf ("Test %d\n", i);
		for (j = 0; j < i; j++)
		  GNOME_Speech_Speaker_say (s, str, &ev);
		g_free (str);
		delay = rand ()%1000000;
		usleep (delay);
		GNOME_Speech_Speaker_stop (s, &ev);
		fprintf (stderr, "%d\r", i);
	}
	CORBA_exception_free (&ev);
}


static void
wait_test (GNOME_Speech_Speaker s)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	GNOME_Speech_Speaker_wait (s, &ev);
	if (BONOBO_EX (&ev)) {
		speaker_say_print (s, "This driver doesn't know how to wait.\n");
		return;
	}
	speaker_say_print (s, "Speech Wait Test.\n");
	speaker_say_print (s, "Peter Piper picked a peck of pickled peppers.  If Peter Piper picked a peck of pickled peppers, how many pecks of pickled peppers did Peter Piper pick?\n");
	GNOME_Speech_Speaker_wait (s, &ev);
	CORBA_exception_free (&ev);
}


static void
parameter_tests (GNOME_Speech_Speaker speaker)
{
	GNOME_Speech_ParameterList *list;
	int i;
	gboolean done = FALSE;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	list = GNOME_Speech_Speaker_getSupportedParameters (speaker, &ev);
	if (BONOBO_EX (&ev) || list->_length == 0) {
		if (!BONOBO_EX (&ev))
			CORBA_free (list);
		speaker_say_print (speaker, "No parameters supported.\n");
		CORBA_exception_free (&ev);
		return;
	}

	speaker_say_print (speaker, "%d parameters supported.\n", list->_length);
	for (i = 0; i < list->_length; i++) {
		GNOME_Speech_Parameter *p = &(list->_buffer[i]);

		speaker_say_print (speaker, "parameter %d: %s.\n", i + 1, p->name);
		speaker_say_print (speaker, "   minimum value: %.2lf.\n", p->min);
		speaker_say_print (speaker, "   current value: %.2lf.\n", p->current);
		speaker_say_print (speaker, "   maximum value: %.2lf.\n", p->max);
		speaker_say_print (speaker, "   %s\n",
				   (p->enumerated) ? "Is enumerated."
				   : "Is not enumerated.");
	}

	while (!done) {
		gchar input[81];
		gint choice;
		gdouble new_value;
		GNOME_Speech_Parameter *p;
      
		speaker_say_print (speaker, "Parameter to change (1 through %d, "
				   "0 to exit): ", list->_length);
		fgets (input, 80, stdin);
		choice = atoi(input);
		if (choice == 0) {
			done = 1;
			break;
		}
		p = &(list->_buffer[choice-1]);
		GNOME_Speech_Speaker_stop (speaker, &ev);
		speaker_say_print (speaker, "new %s? %.2lf through %.2lf.",
				   p->name, p->min, p->max);
		fgets (input, 80, stdin);
		new_value = atof (input);
		printf ("Setting to %lf.\n", new_value);
		GNOME_Speech_Speaker_setParameterValue (speaker, p->name, new_value, &ev);
	}

	CORBA_free (list);
	CORBA_exception_free (&ev);
}

static void
callback_test (GNOME_Speech_Speaker s)
{
	Callback *cb = callback_new ();
	BonoboObject *o = BONOBO_OBJECT (cb);
	CORBA_Environment ev;
	gboolean res;
		
	CORBA_exception_init (&ev);
	res = GNOME_Speech_Speaker_registerSpeechCallback (s,
						     bonobo_object_corba_objref (o), &ev);
	if (!res || BONOBO_EX (&ev)) {
		speaker_say_print (s, "This driver does not support callbacks.\n");
		bonobo_object_unref (cb);
		CORBA_exception_free (&ev);
		return;
	}
	GNOME_Speech_Speaker_say (s, "Now is the time for all good men to come to the aid of their country.", &ev);
	bonobo_main ();
	bonobo_object_unref (cb);
	CORBA_exception_free (&ev);
}



static int
do_test (GNOME_Speech_SynthesisDriver driver, CORBA_Environment *ev)
{
	char input[81];
	int choice = -1;
	CORBA_string driver_name, driver_version;
	CORBA_string synth_name, synth_version;
	GNOME_Speech_Speaker speaker;
	GNOME_Speech_voice_gender gender;
	GNOME_Speech_VoiceInfoList *voices;
	gint i;
    
	driver_name = GNOME_Speech_SynthesisDriver__get_driverName (driver, ev);
	if (!BONOBO_EX (ev))
	{
		printf ("Driver name: %s\n", driver_name);
		CORBA_free (driver_name);
	}
	driver_version = GNOME_Speech_SynthesisDriver__get_driverVersion (driver, ev);
	if (!BONOBO_EX (ev))
	{
		printf ("Driver version: %s\n", driver_version);
		CORBA_free (driver_version);
	}
	synth_name = GNOME_Speech_SynthesisDriver__get_synthesizerName (driver, ev);
	if (!BONOBO_EX (ev))
	{
		printf ("Synthesizer name: %s\n", synth_name);
		CORBA_free (synth_name);
	}
	synth_version = GNOME_Speech_SynthesisDriver__get_synthesizerVersion (driver, ev);
	if (!BONOBO_EX (ev))
	{
		printf ("Synthesizer Version: %s\n", synth_version);
		CORBA_free (synth_version);
	}

	input[0] = '\0';
	while (*input != 'm' && *input != 'f') {
		printf ("\nEnter desired gender ('m' or 'f'): ");
		fgets (input, 80, stdin);
	}

	gender = (*input == 'f') ? GNOME_Speech_gender_female : GNOME_Speech_gender_male;

	input[0] = '\0';
	while (strlen (input) == 0) {
		printf ("\nEnter desired locale, or 'all' to display all voices: ");
		fgets (input, 80, stdin);
	}

	/* Display list of voices */

	if (!strcmp (input, "all\n") || strlen (input) == 1)
	{
	    voices = GNOME_Speech_SynthesisDriver_getAllVoices (driver, ev);
	}
	else
	{
	    GNOME_Speech_VoiceInfo *voice_spec;
	    input [strlen (input) - 1] = '\0';
	    voice_spec = GNOME_Speech_VoiceInfo__alloc ();
	    voice_spec->name = CORBA_string_dup ("");
	    voice_spec->language = CORBA_string_dup (input);
	    voice_spec->gender = GNOME_Speech_gender_male;

	    voices = GNOME_Speech_SynthesisDriver_getVoices (driver, voice_spec, ev);
	}
	if (BONOBO_EX (ev)) {
		fprintf (stderr, "Exception %s getting voice list.\n",
			 bonobo_exception_get_text (ev));
		return -1;
	}
  
	if (voices->_length < 1) {
		printf ("No voices, bailing out.\n");
		return -1;
	}
  
	for (i = 0; i < voices->_length; i++) {
		speaker = GNOME_Speech_SynthesisDriver_createSpeaker (driver,
								      &voices->_buffer[i],
								      ev);
		if (BONOBO_EX (ev)) {
			fprintf (stderr, "Exception %s creating speaker.\n",
				 bonobo_exception_get_text (ev));
			return -1;
		}
		speaker_say_print (speaker, "%d. %s (language %s)\n", i+1, voices->_buffer[i].name, voices->_buffer[i].language);
		bonobo_object_release_unref (speaker, NULL);
	}
	while (choice < 1 || choice > voices->_length) {
		printf ("\nSelect voice: ");
		fgets (input, 80, stdin);
		choice = atoi (input);
	}

	speaker = GNOME_Speech_SynthesisDriver_createSpeaker (driver,
							      &voices->_buffer[choice-1], ev);
	if (speaker == CORBA_OBJECT_NIL) {
		fprintf (stderr, "Error creating speaker.\n");
		return -1;
	}
	GNOME_Speech_Speaker_stop (speaker, ev);
	speaker_say_print (speaker, "\nPlease select a \"test\".\n\n");
	speaker_say_print (speaker, "1. Parameter test.\n");
	speaker_say_print (speaker, "2. Callback test.\n");
	speaker_say_print (speaker, "3. Speech Interrupt test.\n");
	speaker_say_print (speaker, "4. Wait test.\n");
	speaker_say_print (speaker, "0. Exit.\n");
	printf ("\nSelect test: ");
	fgets (input, 80, stdin);
	choice = atoi (input);
	switch (choice)
	{
	case 1:
		GNOME_Speech_Speaker_stop (speaker, ev);
		parameter_tests (speaker);
		GNOME_Speech_Speaker_stop (speaker, ev);
		break;
	case 2:
		GNOME_Speech_Speaker_stop (speaker, ev);
		callback_test (speaker);
		GNOME_Speech_Speaker_stop (speaker, ev);
		break;
	case 3:
		GNOME_Speech_Speaker_stop (speaker, ev);
		speech_interrupt_test (speaker);
		break;
	case 4:
		GNOME_Speech_Speaker_stop (speaker, ev);
		wait_test (speaker);
		break;
	case 0:
		GNOME_Speech_Speaker_stop (speaker, ev);
		GNOME_Speech_Speaker_say (speaker, "Goodbye.", ev);
		bonobo_object_release_unref (speaker, NULL);
		CORBA_free (voices);
		return -1;
	}
	bonobo_object_release_unref (speaker, NULL);
	CORBA_free (voices);
	return 0;
}

int
main (int argc, char **argv)
{
	CORBA_Object o;
	int done = 0;
	CORBA_Environment ev;
  
	CORBA_exception_init (&ev);
	if (!bonobo_init (&argc, argv))
	{
		g_error ("Can't initialize Bonobo...\n");
	}

	o = select_server (&ev);
	if (BONOBO_EX (&ev) || o == CORBA_OBJECT_NIL)
	{
		fprintf (stderr, "Server selection failed: %s.\n", 
			BONOBO_EX (&ev) ? bonobo_exception_get_text (&ev) : "object is NIL");
		return 0;
	}

	/* Attempt to initialize the driver */

	if (GNOME_Speech_SynthesisDriver_driverInit (o, &ev))
		while (!done) {
			if (do_test (o, &ev))
				done = 1;
		}
        else
	        fprintf (stderr, "Server could not be initialized.\n");
	bonobo_object_release_unref (o, NULL);
	CORBA_exception_free (&ev);

	return bonobo_debug_shutdown ();
}

