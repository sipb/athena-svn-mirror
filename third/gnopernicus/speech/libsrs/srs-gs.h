/* srs-gs.h
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

#ifndef __SRS_GS_H__
#define __SRS_GS_H__

#include <glib.h>
#include "srs-speech.h"

typedef struct _SRSGSSpeaker SRSGSSpeaker;
typedef void (*SRSGSMarkersCallback) (gpointer id, SRSMarkerType marker, gint offset);

gboolean srs_gs_init		    (SRSGSMarkersCallback callback);
void 	 srs_gs_terminate	    (void);
gchar**  srs_gs_get_drivers	    (void);
gchar**  srs_gs_get_driver_voices   (gchar *driver);
gboolean srs_gs_shutup		    (void);

void 	      srs_gs_speaker_terminate 	(SRSGSSpeaker *speaker);
gboolean      srs_gs_speaker_update 	(SRSGSSpeaker *speaker, SRSVoiceInfo *info);
SRSGSSpeaker* srs_gs_speaker_new 	(SRSVoiceInfo *info);
gboolean      srs_gs_speaker_say 	(SRSGSSpeaker *speaker, gchar *text, gpointer idp, gint index);
gboolean      srs_gs_speaker_shutup 	(SRSGSSpeaker *speaker);
#ifdef SRS_ENABLE_OPTIMIZATION
gboolean      srs_gs_speaker_same_as	(SRSGSSpeaker *speaker1, SRSGSSpeaker *speaker2);
#endif
#ifdef SRS_NO_MARKERS_SUPPORTED
gboolean      srs_gs_speaker_has_callback (SRSGSSpeaker *speaker);
#endif

#endif /* __SRS_GS_H__ */
