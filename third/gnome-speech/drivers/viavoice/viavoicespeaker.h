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
 * viavoicespeaker.h: Definition of the ViavoiceSpeaker
 *                            object-- a GNOME Speech driver for IBM's
 *                            Viavoice TTS RTK (implementation in
 *                            viavoicesynthesisdriver.c)
 *
 */


#ifndef __VIAVOICE_SPEAKER_H_
#define __VIAVOICE_SPEAKER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <bonobo/bonobo-object.h>
#include <glib/gthread.h>
#include <eci.h>
#include <gnome-speech/gnome-speech.h>



#define VIAVOICE_SPEAKER_TYPE        (viavoice_speaker_get_type ())
#define VIAVOICE_SPEAKER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), VIAVOICE_SPEAKER_TYPE, ViavoiceSpeaker))
#define VIAVOICE_SPEAKER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), viavoice_SPEAKER_TYPE, ViavoiceSpeakerClass))
#define VIAVOICE_SPEAKER_GET_CLASS(obj)                 (G_TYPE_INSTANCE_GET_CLASS ((obj), VIAVOICE_SPEAKER_TYPE, ViavoiceSpeakerClass))
#define IS_VIAVOICE_SPEAKER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), VIAVOICE_SPEAKER_TYPE))
#define IS_VIAVOICE_SPEAKER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), VIAVOICE_SPEAKER_TYPE))

typedef struct {
	Speaker parent;
} ViavoiceSpeaker;

typedef struct {
	SpeakerClass parent_class;
} ViavoiceSpeakerClass;

GType
viavoice_speaker_get_type   (void);

ViavoiceSpeaker *
viavoice_speaker_new (GObject *d,
		      const GNOME_Speech_VoiceInfo *voice_speec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VIAVOICE_SPEAKER_H_ */
