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
 * eloquencesynthespeaker.h: Definition of the EloquenceSpeaker
 *                            object-- a GNOME Speech driver for SpeechWork's
 *                            Eloquence TTS SDK (implementation in
 *                            eloquencesynthesisdriver.c)
 *
 */


#ifndef __ELOQUENCE_SPEAKER_H_
#define __ELOQUENCE_SPEAKER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <bonobo/bonobo-object.h>
#include <glib/gthread.h>
#include <eci.h>
#include <gnome-speech/gnome-speech.h>



#define ELOQUENCE_SPEAKER_TYPE        (eloquence_speaker_get_type ())
#define ELOQUENCE_SPEAKER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), ELOQUENCE_SPEAKER_TYPE, EloquenceSpeaker))
#define ELOQUENCE_SPEAKER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), eloquence_SPEAKER_TYPE, EloquenceSpeakerClass))
#define ELOQUENCE_SPEAKER_GET_CLASS(obj)                 (G_TYPE_INSTANCE_GET_CLASS ((obj), ELOQUENCE_SPEAKER_TYPE, EloquenceSpeakerClass))
#define IS_ELOQUENCE_SPEAKER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), ELOQUENCE_SPEAKER_TYPE))
#define IS_ELOQUENCE_SPEAKER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), ELOQUENCE_SPEAKER_TYPE))

typedef struct {
	Speaker parent;

	ECIHand handle;
} EloquenceSpeaker;

typedef struct {
	SpeakerClass parent_class;
} EloquenceSpeakerClass;

GType
eloquence_speaker_get_type   (void);

EloquenceSpeaker *
eloquence_speaker_new (const GNOME_Speech_VoiceInfo *voice_speec);
  
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ELOQUENCE_SPEAKER_H_ */
