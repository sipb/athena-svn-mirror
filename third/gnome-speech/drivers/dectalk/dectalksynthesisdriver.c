/*
 *
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
 * dectalksynthesisdriver.c: Implements the DectalkSynthesisDriver object--
 *                    a GNOME Speech driver for Fonix's DECtalk TTS SDK
 *
 */

#include <string.h>
#include <libbonobo.h>
#include <glib/gmain.h>
#include <gnome-speech/gnome-speech.h>
#include "dectalksynthesisdriver.h"
#include <dtk/ttsapi.h>
 

#define VERSION_LENGTH 20

typedef struct {
	GNOME_Speech_SpeechCallback cb;
	GNOME_Speech_speech_callback_type type;
	gint text_id;
	gint offset;
} index_queue_entry;


static gint text_id = 0;


static GObjectClass *parent_class;

/*
 *
 * index_queue_entry functions
 *
 */

static void
index_queue_entry_destroy (index_queue_entry *e)
{
	g_return_if_fail (e);
	CORBA_Environment ev;
	CORBA_exception_init (&ev);
	CORBA_Object_release (e->cb, &ev);
	CORBA_exception_free (&ev);
	g_free (e);
}


static void
dectalk_synthesis_driver_flush_index_queues (DectalkSynthesisDriver *d)
{
	GSList *tmp;
	
	/* Flush the index queues */

	g_mutex_lock (d->mutex);
	for (tmp = d->index_queue; tmp; tmp = tmp->next) {
		index_queue_entry *e = (index_queue_entry *) tmp->data;
		index_queue_entry_destroy (e);
	}
	for (tmp = d->post_queue; tmp; tmp = tmp->next) {
		index_queue_entry *e = (index_queue_entry *) tmp->data;
		index_queue_entry_destroy (e);
	}
	g_slist_free (d->index_queue);
	g_slist_free (d->post_queue);
	d->index_queue = d->post_queue = NULL;
	g_mutex_unlock (d->mutex);
}

static DectalkSynthesisDriver *
dectalk_synthesis_driver_from_servant (PortableServer_Servant *servant)
{
	return DECTALK_SYNTHESIS_DRIVER(bonobo_object_from_servant(servant));
}


static gboolean
dectalk_idle (gpointer data)
{
	DectalkSynthesisDriver *d = DECTALK_SYNTHESIS_DRIVER (data);

	TextToSpeechSpeak (d->handle, d->text, TTS_FORCE);
	g_free (d->text);
	d->text = NULL;
	d->text_idle = 0;
	return FALSE;
}


static void
dectalk_add_string (DectalkSynthesisDriver *d,
		    gchar *text)
{
	gchar *new_string;

	if (d->text) {
		new_string = g_strconcat (d->text, text, NULL);
		g_free (d->text);
		d->text = new_string;
	}
	else
		d->text = g_strdup (text);
	if (!d->text_idle)
		d->text_idle = g_idle_add (dectalk_idle, d);
}
	

static void
dectalk_synthesis_driver_add_index(DectalkSynthesisDriver *d,
				   GNOME_Speech_SpeechCallback cb,
				   GNOME_Speech_speech_callback_type type,
				   gint text_id,
				   gint offset)
{
	index_queue_entry *e;
	CORBA_Environment ev;
	
	e = g_new(index_queue_entry, 1);
	CORBA_exception_init (&ev);
	e->cb = CORBA_Object_duplicate (cb, &ev);
	CORBA_exception_free (&ev);
	e->type = type;
	e->text_id = text_id;
	e->offset = offset;

	g_mutex_lock (d->mutex);
	d->index_queue = g_slist_append(d->index_queue, e);
	g_mutex_unlock (d->mutex);
	
	dectalk_add_string (d, " [:index mark 00]");
}


static gboolean
dectalk_timeout (void *data)
{
	DectalkSynthesisDriver *d = DECTALK_SYNTHESIS_DRIVER (data);
	CORBA_Environment ev;
	index_queue_entry *e;
	
	if (d->post_queue) {
		g_mutex_lock (d->mutex);
		e = (index_queue_entry *) d->post_queue->data;
		d->post_queue = g_slist_remove_link (d->post_queue, d->post_queue);
		g_mutex_unlock (d->mutex);
		CORBA_exception_init (&ev);
		GNOME_Speech_SpeechCallback_notify (e->cb,
						    e->type,
						    e->text_id,
						    e->offset,
						    &ev);
		CORBA_Object_release (e->cb, &ev);
		CORBA_exception_free (&ev);
		g_free (e);
	}
	return TRUE;
}


static void
dectalk_callback (LONG lParam1,
		  LONG lParam2, 
		  DWORD dwCallbackParameter, 
		  UINT uiMsg) 
{
	DectalkSynthesisDriver *d = DECTALK_SYNTHESIS_DRIVER(dwCallbackParameter);
	index_queue_entry *e = NULL;
	
	switch (uiMsg) {
	case TTS_MSG_INDEX_MARK:
		if (d && d->index_queue) {
			g_mutex_lock (d->mutex);
			e = (index_queue_entry *) d->index_queue->data;
			d->index_queue = g_slist_remove_link (d->index_queue, d->index_queue);
			d->post_queue = g_slist_append (d->post_queue, e);
			g_mutex_unlock (d->mutex);
		}
	}
	return;
}


static CORBA_string
dectalk__get_driverName(PortableServer_Servant servant,
                        CORBA_Environment * ev)
{
	return(CORBA_string_dup("Fonix DECtalk GNOME Speech Driver"));
}


static CORBA_string
dectalk__get_synthesizerName(PortableServer_Servant servant,
                             CORBA_Environment * ev)
{
	return(CORBA_string_dup("Fonix DECtalk"));
}


static CORBA_string
dectalk__get_driverVersion(PortableServer_Servant aservant,
                           CORBA_Environment * ev)
{
	return(CORBA_string_dup("0.3"));
}


static CORBA_string
dectalk__get_synthesizerVersion(PortableServer_Servant servant,
                                CORBA_Environment * ev)
{
	return(CORBA_string_dup("4.61"));
}


struct voiceinfo voices[MAXVOICES] = {
	{ PAUL, 0, "Paul", 122 },
	{ BETTY,  1, "Betty", 208 },
	{ HARRY,  0, "Harry", 89 },
	{ FRANK,  0, "Frank", 155, },
	{ DENNIS, 0, "Dennis", 110 },
	{ KIT,    0, "Kit", 306 },
	{ URSULA, 1, "Ursula", 240 },
	{ RITA,   1, "Rita", 106 },
	{ WENDY,  1, "Wendy", 200 },
};


static GSList *
get_voice_list(void)
{
	GSList *l = NULL;
	int i;

	for (i = 0; i < MAXVOICES; i++) {
		GNOME_Speech_VoiceInfo *info = GNOME_Speech_VoiceInfo__alloc();

		info->language = CORBA_string_dup("en_US");
		info->name = CORBA_string_dup(voices[i].description);
		info->gender = voices[i].gender;
		l = g_slist_prepend(l, info);
	}
	l = g_slist_reverse(l);

	return(l);
}

static gboolean
lang_matches_spec (gchar *lang, gchar *spec)
{
	gboolean lang_matches, country_matches = FALSE, encoding_matches = TRUE;

	if (!strcmp (lang, spec))
	{
		return TRUE;
	}
	else
	{
		gchar *lang_country_cp, *spec_country_cp;
		gchar *lang_encoding_cp, *spec_encoding_cp;
		lang_matches = !strncmp (lang, spec, 2);
		if (lang_matches) 
		{ 
			lang_country_cp = strchr (lang, '_');
			spec_country_cp = strchr (spec, '_');
			lang_encoding_cp = strchr (lang, '@');
			spec_encoding_cp = strchr (spec, '@');

			if (!lang_country_cp || !spec_country_cp)
			{
				country_matches = TRUE;
			}
			else
			{
				country_matches = !strncmp (lang_country_cp, spec_country_cp, 3);
			}
			if (spec_encoding_cp)
			{
				if (lang_encoding_cp)
				{
					encoding_matches = !strcmp (spec_encoding_cp, lang_encoding_cp); 
				}
				else
				{
					encoding_matches = FALSE;
				}
			}
		}
		return lang_matches && country_matches && encoding_matches;
	}
}

static void
voice_list_free(GSList *l)
{
	GSList *tmp = l;

	while (tmp) {
		CORBA_free(tmp->data);
		tmp = tmp->next;
	}
	g_slist_free(l);
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
			if (!lang_matches_spec (i->language, info->language)) {
				CORBA_free (i);
				l = g_slist_remove_link (l, cur);
				cur = next;
				continue;
			}
		if ((info->gender == GNOME_Speech_gender_male || info->gender == GNOME_Speech_gender_female) &&
		    (info->gender != i->gender)) {
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
voice_info_list_from_voice_list(GSList *l)
{
	int i = 0;
	GNOME_Speech_VoiceInfoList *rv = GNOME_Speech_VoiceInfoList__alloc();
  
	if (!l) {
		rv->_length = 0;
		return(rv);
	}

	rv->_length = rv->_maximum = g_slist_length(l);
	rv->_buffer = GNOME_Speech_VoiceInfoList_allocbuf(rv->_length);

	while (l) {
		GNOME_Speech_VoiceInfo *info = (GNOME_Speech_VoiceInfo *) l->data;

		rv->_buffer[i].name = CORBA_string_dup(info->name);
		rv->_buffer[i].gender = info->gender;
		rv->_buffer[i].language = CORBA_string_dup(info->language);
		i++;
		l = l->next;
	}

	return(rv);
}


static CORBA_boolean
dectalk_driverInit (PortableServer_Servant servant,
		    CORBA_Environment *ev)
{
	DectalkSynthesisDriver *d = dectalk_synthesis_driver_from_servant (servant);
	UINT devOptions = 0;
	int devNo = (int) WAVE_MAPPER;


	if (TextToSpeechStartup(&d->handle, devNo, devOptions,
				dectalk_callback, (gint) d) != MMSYSERR_NOERROR) {
		d->handle = NULL;
		d->initialized = FALSE;
	}

	else {

                /* Initialize the index queue */

		d->index_queue = d->post_queue = NULL;

		d->timeout_id = g_timeout_add_full (G_PRIORITY_HIGH, 50,
						    dectalk_timeout, d, NULL);
		d->initialized = TRUE;
	}
	return d->initialized;
}


static CORBA_boolean
dectalk_isInitialized (PortableServer_Servant servant,
		       CORBA_Environment *ev)
{
	DectalkSynthesisDriver *d = dectalk_synthesis_driver_from_servant (servant);
 	return d->initialized;
}


static GNOME_Speech_VoiceInfoList *
dectalk_getVoices(PortableServer_Servant servant,
                  const GNOME_Speech_VoiceInfo *voice_spec,
                  CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	l = get_voice_list();
	l = prune_voice_list (l, voice_spec);
	rv = voice_info_list_from_voice_list(l);
	voice_list_free(l);

	return(rv);
}


static GNOME_Speech_VoiceInfoList *
dectalk_getAllVoices(PortableServer_Servant servant,
                     CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	l = get_voice_list();
	rv = voice_info_list_from_voice_list(l);
	voice_list_free(l);

	return(rv);
}


static GNOME_Speech_Speaker
dectalk_createSpeaker(PortableServer_Servant servant,
                      const GNOME_Speech_VoiceInfo *voice_spec,
                      CORBA_Environment *ev)
{
	DectalkSynthesisDriver *d = dectalk_synthesis_driver_from_servant (servant);
	DectalkSpeaker *s;
	GSList *l;
	GNOME_Speech_VoiceInfo *info;

	l = get_voice_list ();
	l = prune_voice_list (l, voice_spec);
	if (l)
		info = l->data;
	else
		info = NULL;
	s = dectalk_speaker_new(G_OBJECT(d), info);

	return(CORBA_Object_duplicate(
		       bonobo_object_corba_objref(BONOBO_OBJECT(s)), ev));
}


static void
dectalk_synthesis_driver_init(DectalkSynthesisDriver *driver)
{
	driver->mutex = g_mutex_new ();
	driver->text = NULL;
	driver->text_idle = 0;
	driver->initialized = FALSE;
	driver->last_speaker = NULL;
}


static void
dectalk_synthesis_driver_finalize(GObject *obj)
{
	DectalkSynthesisDriver *d = DECTALK_SYNTHESIS_DRIVER(obj);
	
	if (d->handle != NULL) {
		TextToSpeechSync (d->handle);
		TextToSpeechShutdown(d->handle);
	}
	if (d->timeout_id > 0)
		g_source_remove (d->timeout_id);
	dectalk_synthesis_driver_flush_index_queues (d);
	if (parent_class->finalize)
		parent_class->finalize(obj);
	bonobo_main_quit ();
}


static void
dectalk_synthesis_driver_class_init(DectalkSynthesisDriverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = dectalk_synthesis_driver_finalize;

	/* Initialize parent class epv table */

	klass->epv._get_driverName = dectalk__get_driverName;
	klass->epv._get_driverVersion = dectalk__get_driverVersion;
	klass->epv._get_synthesizerName = dectalk__get_synthesizerName;
	klass->epv._get_synthesizerVersion = dectalk__get_synthesizerVersion;
	klass->epv.driverInit = dectalk_driverInit;
	klass->epv.isInitialized = dectalk_isInitialized;
	klass->epv.getVoices = dectalk_getVoices;
	klass->epv.getAllVoices = dectalk_getAllVoices;
	klass->epv.createSpeaker = dectalk_createSpeaker;  
}


BONOBO_TYPE_FUNC_FULL(DectalkSynthesisDriver, GNOME_Speech_SynthesisDriver,
                      bonobo_object_get_type(), dectalk_synthesis_driver);


DectalkSynthesisDriver *
dectalk_synthesis_driver_new(void)
{
	DectalkSynthesisDriver *driver;

	driver = g_object_new(DECTALK_SYNTHESIS_DRIVER_TYPE, NULL);

	return(driver);
}


gint
dectalk_synthesis_driver_say (DectalkSynthesisDriver *d,
			      DectalkSpeaker *s,
			      gchar *text)
{
	Speaker *speaker = SPEAKER (s);
	gchar *beginning = text;
	gchar *end, *word;
	
        /* If this speaker wasn't the last one to speak, reset the speech parameters */

	if (d->last_speaker != s || speaker_needs_parameter_refresh(speaker)) {
		dectalk_add_string (d, (char *) s->voice);
		speaker_refresh_parameters (speaker);
		d->last_speaker = s;
	}
	
	text_id++;
	if (speaker->cb != CORBA_OBJECT_NIL) {
		dectalk_synthesis_driver_add_index(d,
						   speaker->cb,
						   GNOME_Speech_speech_callback_speech_started,
						   text_id, 0);
		
		/* Add index between words */
		
		while (*beginning) {
			end = beginning;
			while (*end && !g_ascii_isspace(*end))
				end++;
			word = g_strndup (beginning, end-beginning);
			dectalk_add_string (d, word);
			g_free (word);
			dectalk_synthesis_driver_add_index(d,
						   speaker->cb,
						   GNOME_Speech_speech_callback_speech_progress,
						   text_id, end-text);
			if (!*end)
				break;
			beginning = end+1;
		}
		dectalk_synthesis_driver_add_index(d,
						   speaker->cb,
						   GNOME_Speech_speech_callback_speech_ended,
						   text_id, 0);
	}
	else
		dectalk_add_string (d, (char *) text);
	return text_id;	
}


void
dectalk_synthesis_driver_speak_raw (DectalkSynthesisDriver *d,
				    gchar *text)
{
	dectalk_add_string (d, (char *) text);
}


gboolean
dectalk_synthesis_driver_stop (DectalkSynthesisDriver *d)
{
	MMRESULT result;
	
	dectalk_synthesis_driver_flush_index_queues (d);
	if (d->text) {
		g_free (d->text);
		d->text = NULL;
	}
	if (d->text_idle) {
		g_source_remove (d->text_idle);
		d->text_idle = 0;
	}
	
	result = TextToSpeechReset(d->handle, TRUE);
	return((result == MMSYSERR_NOERROR) ? TRUE : FALSE);
}


void
dectalk_synthesis_driver_wait (DectalkSynthesisDriver *d)
{
	TextToSpeechSync (d->handle);
}


gboolean
dectalk_synthesis_driver_is_speaking (DectalkSynthesisDriver *d)
{
	gint lparam1, lparam2;

	TextToSpeechGetStatus (d->handle, &lparam1, &lparam2, STATUS_SPEAKING);
	return TRUE;
}


int
main(int argc, char **argv)
{
	DectalkSynthesisDriver *driver;
	char *obj_id;
	int ret;
  
	if (!bonobo_init(&argc, argv)) {
		g_error("Could not initialize Bonobo Activation / Bonobo");
	}

	/* If threads haven't been initialized, initialize them */

	if (!g_threads_got_initialized)
		g_thread_init (NULL);
	
	obj_id = "OAFIID:GNOME_Speech_SynthesisDriver_Dectalk:proto0.3";

	driver = dectalk_synthesis_driver_new();
	if (!driver) {
		g_error("Error creating speech synthesis driver object.\n");
	}

	ret = bonobo_activation_active_server_register(obj_id,
						       bonobo_object_corba_objref(bonobo_object(driver)));

	if (ret != Bonobo_ACTIVATION_REG_SUCCESS) {
		g_error ("Error registering speech synthesis driver.\n");
	} else {
		bonobo_main();
	}

	return(0);
}
