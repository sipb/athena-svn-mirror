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
 * festivalspeaker.h: Definition of the FestivalSynthesisDriver
 *                            object-- a GNOME Speech driver for the Festival
 *                            Speech Synthesis System (implementation in
 *                            festivalsynthesisdriver.c)
 *
 */

#ifndef __FESTIVAL_SPEAKER_H_
#define __FESTIVAL_SPEAKER_H_

#include <bonobo/bonobo-object.h>
#include <gnome-speech/gnome-speech.h>

#define FESTIVAL_SPEAKER_TYPE        (festival_speaker_get_type ())
#define FESTIVAL_SPEAKER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), FESTIVAL_SPEAKER_TYPE, FestivalSpeaker))
#define FESTIVAL_SPEAKER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), FESTIVAL_SPEAKER_TYPE, FestivalSpeakerClass))
#define IS_FESTIVALSPEAKER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), FESTIVAL_SPEAKER_TYPE))
#define IS_FESTIVAL_SPEAKER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), FESTIVAL_SPEAKER_TYPE))

typedef struct {
	Speaker parent;

	gchar *voice;
} FestivalSpeaker;

typedef struct {
	SpeakerClass parent_class;
} FestivalSpeakerClass;

GType
festival_speaker_get_type   (void);

FestivalSpeaker *
festival_speaker_new (GObject *driver,
		      const GNOME_Speech_VoiceInfo *spec);
 
#endif /* __FESTIVAL_SPEAKER_H_ */
