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
 * dectalksynthespeaker.h: Definition of the DectalkSpeaker
 *                            object-- a GNOME Speech driver for Fonix's
 *                            DECtalk TTS SDK (implementation in
 *                            dectalksynthesisdriver.c)
 *
 */


#ifndef __DECTALK_SPEAKER_H_
#define __DECTALK_SPEAKER_H_

#include <bonobo/bonobo-object.h>
#include <glib/gthread.h>
#include <gnome-speech/gnome-speech.h>

#define DECTALK_SPEAKER_TYPE        (dectalk_speaker_get_type())
#define DECTALK_SPEAKER(o)          (G_TYPE_CHECK_INSTANCE_CAST((o), DECTALK_SPEAKER_TYPE, DectalkSpeaker))
#define DECTALK_SPEAKER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), dectalk_SPEAKER_TYPE, DectalkSpeakerClass))
#define DECTALK_SPEAKER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), DECTALK_SPEAKER_TYPE, DectalkSpeakerClass))
#define IS_DECTALK_SPEAKER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), DECTALK_SPEAKER_TYPE))
#define IS_DECTALK_SPEAKER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), DECTALK_SPEAKER_TYPE))

#define MAXVOICES 9

struct voiceinfo {
	int index;		  /* taken from <dtk/ttsapi.h> include file. */
	int gender;		  /* 0 is male, 1 is female. */
	char *description;
	gint pitch;
};

typedef struct _DectalkSpeaker DectalkSpeaker;
typedef struct _DectalkSpeakerClass DectalkSpeakerClass;

struct _DectalkSpeaker {
	Speaker parent;

	/* The string to set the speaker's voice */

	gchar *voice;
};

struct _DectalkSpeakerClass {
	SpeakerClass parent_class;
};

GType
dectalk_speaker_get_type   (void);

DectalkSpeaker *
dectalk_speaker_new(const GObject *driver,
		    const GNOME_Speech_VoiceInfo *voice_speec);
  
#endif /* __DECTALK_SPEAKER_H_ */
