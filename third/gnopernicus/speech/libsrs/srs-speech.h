/* srs-speech.h
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

#ifndef __SRS_SPEECH_H__
#define __SRS_SPEECH_H__

#include <glib.h>

typedef enum _SRSMarkerType	SRSMarkerType;
typedef enum _SRSSpellingMode 	SRSSpellingMode;

enum _SRSMarkerType
{
    SRS_MARKER_OUTPUT_STARTED = 1 << 0,
    SRS_MARKER_OUTPUT_ENDED   = 1 << 1,
    SRS_MARKER_TEXT_STARTED   = 1 << 2,
    SRS_MARKER_TEXT_ENDED     = 1 << 3,
    SRS_MARKER_TEXT_PROGRESS  = 1 << 4
};

enum _SRSSpellingMode
{
    SRS_SPELLING_NONE,
    SRS_SPELLING_CHAR,
    SRS_SPELLING_MILITARY
};

typedef struct _SRSTextOut   SRSTextOut;
typedef struct _SRSVoiceInfo SRSVoiceInfo;
typedef struct _SRSOut       SRSOut;
typedef struct _SRSMarker    SRSMarker;


typedef void (*SRSMarkersCallback) (SRSMarker *marker);

struct _SRSVoiceInfo
{
    gchar	*id;

    gchar 	*driver;
    gchar 	*voice; /* driver voice */

    gint	rate;	/* in % */
    gint	pitch;	/* in % */
    gint	volume;	/* in % */
};

struct _SRSTextOut
{
    gchar	   *text;
    gchar 	   *voice;
    gchar	   *id;
    SRSSpellingMode spelling;
};

struct _SRSOut
{
    GPtrArray 	*texts;
    gchar	*id;
    SRSMarkerType markers;
};

struct _SRSMarker
{
    SRSMarkerType  type;
    SRSOut	  *out;
    SRSTextOut	  *tout;
    gint	   offset;
};

SRSVoiceInfo*	srs_voice_info_new 	   (void);
void       	srs_voice_info_terminate   (SRSVoiceInfo *voice);
gboolean       	srs_voice_update_from_info (SRSVoiceInfo *voice);
#ifdef SRS_NO_MARKERS_SUPPORTED
gboolean       	srs_voice_has_callback 	   (gchar *voice);
#endif
SRSTextOut*	srs_text_out_new         (void);
void 		srs_text_out_terminate   (SRSTextOut	*tout);

SRSOut*		srs_out_new    	    (void);
void 		srs_out_terminate   (SRSOut *out);
gboolean	srs_out_add_text_out(SRSOut *out, SRSTextOut *tout);

SRSMarker*	srs_marker_new	     (void);
void 		srs_marker_terminate (SRSMarker *marker);

gboolean	srs_sp_init	    (SRSMarkersCallback callback);
void		srs_sp_terminate    (void);
gchar**		srs_sp_get_drivers  (void);
gchar**		srs_sp_get_driver_voices(gchar *driver);
gboolean	srs_sp_speak_out    (SRSOut *out);
gboolean       	srs_sp_shutup	    (void);
gboolean       	srs_sp_pause	    (void);
gboolean       	srs_sp_resume	    (void);

#endif /* __SRS_SPEECH_H__ */
