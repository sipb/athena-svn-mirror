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
 * thetasynthesisdriver.c: Implements the ThetaSynthesisDriver object--
 *                    a GNOME Speech driver for Cepstral's Theta TTS engine
 *
 */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libbonobo.h>
#include <glib/gmain.h>
#include <gnome-speech/gnome-speech.h>
#include <theta.h>
#include "thetasynthesisdriver.h"
 

#define VERSION_LENGTH 20

typedef struct {
	cst_voice *vox;
	theta_async_t t;
	Speaker *speaker;
	gchar *text;
	gint text_id;
} utterance;


static gint text_id = 0;


static GObjectClass *parent_class;

static void
utterance_destroy (utterance *u)
{
	g_return_if_fail (u);
	if (u->text)
		g_free (u->text);
	if (u->speaker)
		g_object_unref (u->speaker);
	g_free (u);
}


static void
utterance_list_destroy (GSList *l)
{
	GSList *tmp;
	
	for (tmp = l; tmp; tmp = tmp->next)
		utterance_destroy ((utterance *) tmp->data);
	g_slist_free (l);
}


static gboolean
theta_timeout (gpointer data)
{
	ThetaSynthesisDriver *d = THETA_SYNTHESIS_DRIVER (data);
	utterance *u;
	CORBA_Environment ev;
	CORBA_long offset = 0;
	gint rv;
	
	/* Storeage for current utterance values */

	Speaker *s;
	gint text_id;

	CORBA_exception_init (&ev);

        /* Check to see if the speech process has sent us any notifiations */

	rv = read (d->status_pipe[0], &offset, sizeof(gint));
	if (rv == sizeof(gint)) {
		u = (utterance *) d->utterances->data;
		if (u->speaker->cb != CORBA_OBJECT_NIL) {
			GNOME_Speech_SpeechCallback_notify (u->speaker->cb,
							    GNOME_Speech_speech_callback_speech_progress,
							    u->text_id,
							    offset,
							    &ev);
		}
	}
	
	if (d->speaking) {
		u = (utterance *) d->utterances->data;
		if (theta_query_async (u->t))
			return TRUE;
		d->utterances = g_slist_remove (d->utterances, u);
		theta_close_async (u->t);
		text_id = u->text_id;
		s = g_object_ref (u->speaker);
		utterance_destroy (u);
		d->speaking = FALSE;
		if (s->cb != CORBA_OBJECT_NIL) {
			GNOME_Speech_SpeechCallback_notify (s->cb,
							    GNOME_Speech_speech_callback_speech_ended,
							    text_id,
							    0,
							    &ev);
		}
		g_object_unref (s);
	}
	if (!d->utterances) {
		d->timeout_id = 0;
		return FALSE;
	}
	u = (utterance *) d->utterances->data;
	u->t = theta_tts_async (u->text, u->vox);
	d->speaking = TRUE;
	if (u->speaker->cb != CORBA_OBJECT_NIL)
		GNOME_Speech_SpeechCallback_notify (u->speaker->cb,
						    GNOME_Speech_speech_callback_speech_started,
						    u->text_id,
						    0,
						    &ev);
	CORBA_exception_free (&ev);
	return TRUE;
}


static void
theta_synthesis_driver_set_timeout (ThetaSynthesisDriver *d)
{
	if (!d->timeout_id)
		d->timeout_id = g_timeout_add_full (G_PRIORITY_HIGH,
						    50,
						    theta_timeout,
						    (gpointer) d, NULL);
}


static void
theta_event_func (int spos,
		  int type,
		  const char *text,
		  void *data)
{
	ThetaSynthesisDriver *d = THETA_SYNTHESIS_DRIVER (data);
	utterance *u;
	gchar *location;
	gint offset, l;


	g_return_if_fail (d->utterances);
	u = (utterance *) d->utterances->data;
	switch (type) {
	case THETA_START_UTT:
		break;
	case THETA_END_UTT:
		break;
	case THETA_WORD_BOUNDARY:
		location = strstr (u->text, text);
		if (location) {
			l = strlen (text);
			offset = location+l-u->text;
			write (d->status_pipe[1], &offset, sizeof(gint));
		}
		break;
	}
}


static ThetaSynthesisDriver *
theta_synthesis_driver_from_servant (PortableServer_Servant *servant)
{
	return THETA_SYNTHESIS_DRIVER(bonobo_object_from_servant(servant));
}


static CORBA_string
theta__get_driverName(PortableServer_Servant servant,
                        CORBA_Environment * ev)
{
	return(CORBA_string_dup("Theta GNOME Speech Driver"));
}


static CORBA_string
theta__get_synthesizerName(PortableServer_Servant servant,
                             CORBA_Environment * ev)
{
	return(CORBA_string_dup("Cepstral Theta"));
}


static CORBA_string
theta__get_driverVersion(PortableServer_Servant aservant,
                           CORBA_Environment * ev)
{
	return(CORBA_string_dup("0.3"));
}


static CORBA_string
theta__get_synthesizerVersion(PortableServer_Servant servant,
                                CORBA_Environment * ev)
{
	return(CORBA_string_dup("2.3"));
}

static GSList *
get_voice_list(void)
{
	GSList *l = NULL;
	theta_voice_desc *vlist, *vdesc;

	vlist = theta_enum_voices (NULL, NULL);
	for (vdesc = vlist; vdesc; vdesc = vdesc->next) {
		const char *lang;
		GNOME_Speech_VoiceInfo *info = GNOME_Speech_VoiceInfo__alloc();

		/*
		 * Convert the language we get from Theta to the one
		 * gnome-speech clients expect
		*/

		lang = "en_US";
		info->language = CORBA_string_dup (lang);
		info->name = CORBA_string_dup(vdesc->human);
		if (!strcmp (vdesc->gender, "male"))
			info->gender = GNOME_Speech_gender_male;
		else if (!strcmp (vdesc->gender, "female"))
			info->gender = GNOME_Speech_gender_female;
		else
			info->gender = -1;
		info->gender = 0;
		l = g_slist_prepend(l, info);
	}
	l = g_slist_reverse(l);
	theta_free_voicelist (vlist);
	return l;
}


static void
voice_list_free(GSList *l)
{
	GSList *tmp = l;

	while (tmp) {
		CORBA_free (tmp->data);
		tmp = tmp->next;
	}
	g_slist_free(l);
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
voice_info_list_from_voice_list (GSList *l)
{
	int i = 0;
	GNOME_Speech_VoiceInfoList *rv = GNOME_Speech_VoiceInfoList__alloc();
  
	if (!l) {
		rv->_length = 0;
		return rv;
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

	return rv;
}


static CORBA_boolean
theta_driverInit (PortableServer_Servant servant,
		    CORBA_Environment *ev)
{
 	ThetaSynthesisDriver *d = theta_synthesis_driver_from_servant (servant);

	if (d->initialized)
		return TRUE;
	if (pipe (d->status_pipe))
		return FALSE;
	fcntl (d->status_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl (d->status_pipe[1], F_SETFL, O_NONBLOCK);
	
	theta_init (NULL);
	d->initialized = TRUE;
	return TRUE;
}


static CORBA_boolean
theta_isInitialized (PortableServer_Servant servant,
		       CORBA_Environment *ev)
{
	ThetaSynthesisDriver *d = theta_synthesis_driver_from_servant (servant);
 	return d->initialized;
}


static GNOME_Speech_VoiceInfoList *
theta_getVoices(PortableServer_Servant servant,
                  const GNOME_Speech_VoiceInfo *voice_spec,
                  CORBA_Environment *ev)
{
	GNOME_Speech_VoiceInfoList *rv;
	GSList *l;

	l = get_voice_list ();
	l = prune_voice_list (l, voice_spec);
	rv = voice_info_list_from_voice_list(l);
	voice_list_free (l);

	return rv;
}


static GNOME_Speech_VoiceInfoList *
theta_getAllVoices(PortableServer_Servant servant,
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
theta_createSpeaker(PortableServer_Servant servant,
                      const GNOME_Speech_VoiceInfo *voice_spec,
                      CORBA_Environment *ev)
{
	ThetaSynthesisDriver *d = theta_synthesis_driver_from_servant (servant);
	ThetaSpeaker *s;
	GSList *l;
	GNOME_Speech_VoiceInfo *info;

	l = get_voice_list ();
	l = prune_voice_list (l, voice_spec);
	
	if (l)
		info = l->data;
	else
		info = NULL;

	s = theta_speaker_new(G_OBJECT(d), info);
	theta_set_eventfunc (s->vox, theta_event_func, d);
	
	return(CORBA_Object_duplicate(
		       bonobo_object_corba_objref(BONOBO_OBJECT(s)), ev));
}


static void
theta_synthesis_driver_init(ThetaSynthesisDriver *driver)
{
	driver->utterances = NULL;
	driver->speaking = FALSE;
	driver->status_pipe[0] = driver->status_pipe[1] = -1;
	driver->timeout_id = 0;
	driver->initialized = FALSE;
}


static void
theta_synthesis_driver_finalize(GObject *obj)
{
	ThetaSynthesisDriver *d = THETA_SYNTHESIS_DRIVER (obj);

	if (d->status_pipe[0] != -1)
		close (d->status_pipe[0]);
	if (d->status_pipe[1] != -1)
		close (d->status_pipe[1]);
	if (parent_class->finalize)
		parent_class->finalize(obj);
	bonobo_main_quit ();
}


static void
theta_synthesis_driver_class_init(ThetaSynthesisDriverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
  
	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = theta_synthesis_driver_finalize;

	/* Initialize parent class epv table */

	klass->epv._get_driverName = theta__get_driverName;
	klass->epv._get_driverVersion = theta__get_driverVersion;
	klass->epv._get_synthesizerName = theta__get_synthesizerName;
	klass->epv._get_synthesizerVersion = theta__get_synthesizerVersion;
	klass->epv.driverInit = theta_driverInit;
	klass->epv.isInitialized = theta_isInitialized;
	klass->epv.getVoices = theta_getVoices;
	klass->epv.getAllVoices = theta_getAllVoices;
	klass->epv.createSpeaker = theta_createSpeaker;  
}


gint
theta_synthesis_driver_say (ThetaSynthesisDriver *d,
			    ThetaSpeaker *s,
			    const gchar *text)
{
	utterance *u;

	theta_synthesis_driver_set_timeout (d);
	u = g_new (utterance, 1);
	u->vox  = s->vox;
	u->t = NULL;
	u->text_id = text_id++;
	u->text = g_strdup (text);
	u->speaker = g_object_ref (s);
	d->utterances = g_slist_append (d->utterances, u);
	return u->text_id;
}


gboolean
theta_synthesis_driver_stop (ThetaSynthesisDriver *d)
{
	utterance *u;
	gint offset;

	while (read (d->status_pipe[0], &offset, sizeof(gint)) == sizeof(gint));
	if (!d->utterances)
		return FALSE;
	if (d->speaking) {
		u = (utterance *) d->utterances->data;
		theta_stop_async (u->t);
	}
	utterance_list_destroy (d->utterances);
	d->utterances = NULL;
	d->speaking = FALSE;
	if (d->timeout_id) {
		g_source_remove (d->timeout_id);
		d->timeout_id = 0;
	}
	return TRUE;
}


gboolean
theta_synthesis_driver_is_speaking (ThetaSynthesisDriver *d)
{
	if (d->utterances) {
		utterance *u = (utterance *) d->utterances->data;
		return theta_query_async (u->t);
	}
	else
		return FALSE;
}


BONOBO_TYPE_FUNC_FULL(ThetaSynthesisDriver, GNOME_Speech_SynthesisDriver,
                      bonobo_object_get_type(), theta_synthesis_driver);


ThetaSynthesisDriver *
theta_synthesis_driver_new(void)
{
	ThetaSynthesisDriver *driver;

	driver = g_object_new(THETA_SYNTHESIS_DRIVER_TYPE, NULL);

	return driver;
}


int
main(int argc, char **argv)
{
	ThetaSynthesisDriver *driver;
	char *obj_id;
	int ret;
  
	if (!bonobo_init(&argc, argv)) {
		g_error("Could not initialize Bonobo Activation / Bonobo");
	}

	/* Initialize threads */  

	obj_id = "OAFIID:GNOME_Speech_SynthesisDriver_Theta:proto0.3";

	driver = theta_synthesis_driver_new();
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

	return 0;
}
