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
 * thetaspeaker.h: Definition of the ThetaSpeaker
 *                            object-- a GNOME Speech driver for Cepstral's
 *                            Theta TTS Engine  (implementation in
 *                            thetaspeaker.c)
 *
 */


#ifndef __THETA_SPEAKER_H_
#define __THETA_SPEAKER_H_

#include <bonobo/bonobo-object.h>
#include <glib/gthread.h>
#include <gnome-speech/gnome-speech.h>
#include <theta.h>

#define THETA_SPEAKER_TYPE        (theta_speaker_get_type())
#define THETA_SPEAKER(o)          (G_TYPE_CHECK_INSTANCE_CAST((o), THETA_SPEAKER_TYPE, ThetaSpeaker))
#define THETA_SPEAKER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), theta_SPEAKER_TYPE, ThetaSpeakerClass))
#define THETA_SPEAKER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), THETA_SPEAKER_TYPE, ThetaSpeakerClass))
#define IS_THETA_SPEAKER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), THETA_SPEAKER_TYPE))
#define IS_THETA_SPEAKER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), THETA_SPEAKER_TYPE))

#define MAXVOICES 9

struct voiceinfo {
	int index;		  /* taken from <dtk/ttsapi.h> include file. */
	int gender;		  /* 0 is male, 1 is female. */
	char *description;
	gint pitch;
};

typedef struct  {
	Speaker parent;

	cst_voice *vox;
} ThetaSpeaker;

typedef struct  {
	SpeakerClass parent_class;
} ThetaSpeakerClass;

GType
theta_speaker_get_type   (void);

ThetaSpeaker *
theta_speaker_new(const GObject *driver,
		    const GNOME_Speech_VoiceInfo *voice_speec);
  
#endif /* __THETA_SPEAKER_H_ */
