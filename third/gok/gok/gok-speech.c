/*
* gok-speech.c
*
* Copyright 2004 Sun Microsystems, Inc.,
* Copyright 2004 University Of Toronto
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <libbonobo.h>
#include <locale.h>
#include <gnome-speech/gnome-speech.h>
#include "gok-speech.h"
#include "gok-log.h"

static GNOME_Speech_Speaker speaker = CORBA_OBJECT_NIL;
static GNOME_Speech_SynthesisDriver driver = CORBA_OBJECT_NIL;
static Bonobo_ServerInfoList *servers;
static GNOME_Speech_VoiceInfoList *voices;

/**
 * gok_speech_initialize:
 *
 * Initialises gok-speech.
 */
void gok_speech_initialize ()
{
    CORBA_Environment ev;

    gok_log_enter ();
    CORBA_exception_init (&ev);

    servers = bonobo_activation_query (
	"repo_ids.has ('IDL:GNOME/Speech/SynthesisDriver:0.3')",
	NULL, &ev);

    if (servers && servers->_length > 0 && !BONOBO_EX (&ev))
    {
	driver = bonobo_activation_activate_from_id (
	    (const Bonobo_ActivationID) servers->_buffer[0].iid,
	    0, NULL, &ev);
	if (!BONOBO_EX (&ev))
	{
	    GNOME_Speech_VoiceInfo spec;
	    spec.language = setlocale (LC_MESSAGES, NULL);
	    voices = GNOME_Speech_SynthesisDriver_getVoices (driver, &spec, &ev);
	    if (voices->_length > 0)
	    {
		speaker = GNOME_Speech_SynthesisDriver_createSpeaker (driver, 
								      &voices->_buffer[0], 
								      &ev);
		if (BONOBO_EX (&ev))
		{
		    gok_log_x ("gnome-speech error: could not create a speaker.");
		    /* TODO: free the exception */
		}
	    }
	    else
	    {
		gok_log_x ("gnome-speech error: no voices matching locale \"%s\" were found.");
	    }
	}
	else
	{
	    gok_log_x ("gnome-speech error activating speech server.");
	}
    }
    else
    {
	gok_log_x ("gnome-speech error: could not locate any speech servers.");
    }
    gok_log_leave ();
}

/**
 * gok_speech_shutdown:
 *
 * Shuts down gok-speech and frees any loaded speechs.
 */
void gok_speech_shutdown ()
{
    CORBA_Environment ev;
    gok_log_enter ();

    CORBA_exception_init (&ev);

    CORBA_free (voices);
    CORBA_free (servers);

    if (speaker)
	Bonobo_Unknown_unref (speaker, &ev);
    speaker = NULL;
    if (driver)
	Bonobo_Unknown_unref (driver, &ev);
    driver = NULL;

    gok_log_leave ();
}

/**
 * gok_speech_speak:
 * @text: The text to say.
 *
 * Speaks the text @text.
 */
void gok_speech_speak (gchar *text)
{
    CORBA_Environment ev;
    gok_log_enter ();

    CORBA_exception_init (&ev);

    gok_log ("SPEAKING: %s", text);

    if (speaker != CORBA_OBJECT_NIL) {
	GNOME_Speech_Speaker_say (speaker, text, &ev);
	if (BONOBO_EX (&ev))
	    gok_log_x ("gnome-speech error speaking string %s", text);
    }
    else
    {
	gok_log_x ("gnome-speech error: could not speak string %s (no speaker)", text);
    }

    gok_log_leave ();
}
