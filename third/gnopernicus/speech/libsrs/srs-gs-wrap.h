/* srs-gs-wrap.h
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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

#ifndef __SRS_GS_WRAP_H__
#define __SRS_GS_WRAP_H__

#include <glib.h>
#include "srs-speech.h"
#include <gnome-speech/gnome-speech.h>

#define SRSGSWrapSpeaker 	GNOME_Speech_Speaker
#define SRSGSWrapLong	 	CORBA_long
#define SRSGSWrapMarkerType	GNOME_Speech_speech_callback_type
#define	SRSGSWrapSpeechStarted  GNOME_Speech_speech_callback_speech_started
#define SRSGSWrapSpeechEnded  	GNOME_Speech_speech_callback_speech_ended

typedef void (*SRSGSWrapCallback) (SRSGSWrapLong id, SRSGSWrapMarkerType type, SRSGSWrapLong offset);

gboolean srs_gs_wrap_init 	  (SRSGSWrapCallback callback);
void 	 srs_gs_wrap_terminate    ();

gchar**  srs_gs_wrap_get_drivers 	 ();
gchar**  srs_gs_wrap_get_driver_voices (gchar *driver);
#ifdef SRS_NO_MARKERS_SUPPORTED
SRSGSWrapSpeaker srs_gs_wrap_speaker_new (gchar *driver, gchar *voice, gboolean *has_callback);
#else
SRSGSWrapSpeaker srs_gs_wrap_speaker_new (gchar *driver, gchar *voice);
#endif
void 	 srs_gs_wrap_speaker_terminate   (SRSGSWrapSpeaker speaker);
gboolean srs_gs_wrap_speaker_set_pitch   (SRSGSWrapSpeaker speaker, gint pitch);
gboolean srs_gs_wrap_speaker_set_rate    (SRSGSWrapSpeaker speaker, gint rate);
gboolean srs_gs_wrap_speaker_set_volume  (SRSGSWrapSpeaker speaker, gint volume);
gboolean srs_gs_wrap_speaker_shutup 	 (SRSGSWrapSpeaker speaker);
SRSGSWrapLong srs_gs_wrap_speaker_say    (SRSGSWrapSpeaker speaker, gchar *text);

#endif /* __SRS_GS_WRAP_H__ */
