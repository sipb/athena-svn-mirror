/* srs-gs.c
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

#include <glib.h>
#include "srs-gs.h"
#include "srs-gs-wrap.h"
#include <string.h>
#include "SRMessages.h"


struct _SRSGSSpeaker
{
    gchar		*voice;  
    gchar		*driver;
    SRSGSWrapSpeaker	 speaker;
    
    gint		 rate;
    gint		 pitch;
    gint		 volume;
    gboolean		 has_callback;
};

typedef struct _SRSGSOut
{
    gpointer 	   idp;
    gint	   index;
    SRSGSWrapLong  idl;
}SRSGSOut;

static SRSGSMarkersCallback	 srs_gs_callback_to_speech = NULL;
static GSList			*srs_gs_outs 	 = NULL;
static gboolean			 srs_gs_start_marker_waiting = FALSE;


static SRSGSOut*
srs_gs_out_new ()
{
    SRSGSOut *rv;

    rv = g_new0 (SRSGSOut, 1);
    rv->index = -1;

    return rv;
}

static void
srs_gs_out_terminate (SRSGSOut *out)
{
    sru_assert (out);

    g_free (out);
}

static gboolean
srs_gs_out_terminate_idle (gpointer data)
{
    srs_gs_out_terminate (data);

    return FALSE;
}

static gboolean
srs_gs_speaker_init (SRSGSSpeaker *speaker)
{
    sru_assert (speaker);

    speaker->speaker = NULL;
    speaker->driver  = NULL;
    speaker->voice   = NULL;
    speaker->rate    = -1;
    speaker->pitch   = -1;
    speaker->volume  = -1;

    return TRUE;
}

static SRSGSSpeaker*
srs_gs_speaker_new_ ()
{
    SRSGSSpeaker *speaker = NULL;
    
    speaker = g_new0 (SRSGSSpeaker, 1);
    srs_gs_speaker_init (speaker);

    return speaker;
}

static void
srs_gs_speaker_clean (SRSGSSpeaker *speaker)
{
    sru_assert (speaker);
    
    srs_gs_wrap_speaker_terminate (speaker->speaker);
    g_free (speaker->voice);
    g_free (speaker->driver);
}

void
srs_gs_speaker_terminate (SRSGSSpeaker *speaker)
{
    srs_gs_speaker_clean (speaker);
    g_free (speaker);
}

gboolean
srs_gs_speaker_update (SRSGSSpeaker *speaker,
		       SRSVoiceInfo *voice)
{
    sru_assert (voice && speaker);

    if ((speaker->driver && voice->driver && 
			(strcmp (speaker->driver, voice->driver) != 0 )) ||
	 (speaker->voice && voice->voice &&
			strcmp (speaker->voice, voice->voice) != 0))
    {
        srs_gs_speaker_clean (speaker);
	srs_gs_speaker_init (speaker);
    }

    if ((voice->driver && 
	    (!speaker->driver || strcmp (speaker->driver, voice->driver) != 0 ))||
	 (voice->voice &&
	    (!speaker->voice || strcmp (speaker->voice, voice->voice) != 0)))
    {
	g_free (speaker->driver);
	sru_assert (voice->driver);
	speaker->driver = g_strdup (voice->driver);
	g_free (speaker->voice);
	sru_assert (voice->voice);
	speaker->voice = g_strdup (voice->voice);
	sru_assert (speaker->speaker == NULL);
#ifdef SRS_NO_MARKERS_SUPPORTED
	speaker->speaker = srs_gs_wrap_speaker_new (speaker->driver, speaker->voice, &speaker->has_callback);
#else
	speaker->speaker = srs_gs_wrap_speaker_new (speaker->driver, speaker->voice);
#endif
    }

    if (speaker->pitch != voice->pitch && voice->pitch != -1)
    {
	speaker->pitch = voice->pitch;
	srs_gs_wrap_speaker_set_pitch (speaker->speaker, speaker->pitch);
    }

    if (speaker->rate != voice->rate && voice->rate != -1)
    {
	speaker->rate = voice->rate;
	srs_gs_wrap_speaker_set_rate (speaker->speaker, speaker->rate);
    }

    if (speaker->volume != voice->volume && voice->volume != -1)
    {
	speaker->volume = voice->volume;
	srs_gs_wrap_speaker_set_volume (speaker->speaker, speaker->volume);
    }

    return TRUE;
}

SRSGSSpeaker*
srs_gs_speaker_new (SRSVoiceInfo *voice)
{
    SRSGSSpeaker *speaker;

    sru_assert (voice);

    speaker = srs_gs_speaker_new_ ();
    srs_gs_speaker_update (speaker, voice);

    return speaker;
}

static void
srs_gs_generate_callback (SRSGSWrapLong id,
			  SRSMarkerType marker,
			  SRSGSWrapLong offset)
{
    SRSGSOut *out;

    sru_assert (srs_gs_outs);
    sru_assert (srs_gs_callback_to_speech);
#ifdef SRS_GS_DEBUG
    {
	GSList *crt;
	for (crt = srs_gs_outs->next; crt; crt = crt->next)
	{
	    SRSGSOut *tmp = crt->data;
	    if (tmp->idl == id)
		sru_assert_not_reached (); /* should be ALWAYS on first position */
	}
    }
#endif

    out = srs_gs_outs->data;
    if (id == out->idl)
	srs_gs_callback_to_speech (out->idp, marker,
		out->index >= 0 ? out->index : offset);
}

gboolean
srs_gs_speaker_say (SRSGSSpeaker *speaker,
		    gchar *text,
		    gpointer idp,
		    gint index)
{
    SRSGSWrapLong id;

    sru_assert (speaker && speaker->speaker);

    id = srs_gs_wrap_speaker_say (speaker->speaker, text);
    if (id != -1)
    {
	SRSGSOut *add;

	add = srs_gs_out_new ();
	add->idl = id;
	add->idp = idp;
	add->index = index;

        srs_gs_outs = g_slist_append (srs_gs_outs, add);

	if (srs_gs_start_marker_waiting)
	{
	    srs_gs_start_marker_waiting = FALSE;
	    srs_gs_generate_callback (id, SRS_MARKER_TEXT_STARTED, -1);
	}
    }

    return id != -1;
}

gboolean
srs_gs_speaker_shutup (SRSGSSpeaker *speaker)
{
    sru_assert (speaker && speaker->speaker);
    
    return srs_gs_wrap_speaker_shutup (speaker->speaker);    
}

static void
srs_gs_callback (SRSGSWrapLong 		id,
		 SRSGSWrapMarkerType 	type,
		 SRSGSWrapLong		offset)
{
    static gboolean busy = FALSE;

    sru_assert (srs_gs_callback_to_speech);
    sru_assert (busy == FALSE);

    busy = TRUE;
    switch (type)
    {
	case SRSGSWrapSpeechStarted: /* semms to came before returning from .._say */
	    if (!srs_gs_outs) /* before ..._say return */
		srs_gs_start_marker_waiting = TRUE;
	    else
		srs_gs_generate_callback (id, SRS_MARKER_TEXT_STARTED, offset);
	    break;
	case SRSGSWrapSpeechEnded:
	    if (srs_gs_outs)
	    {
		srs_gs_generate_callback (id, SRS_MARKER_TEXT_ENDED, offset);

		g_idle_add (srs_gs_out_terminate_idle, srs_gs_outs->data);
		srs_gs_outs = g_slist_delete_link (srs_gs_outs, srs_gs_outs);
	    }
	    break;
	default:
	    sru_warning ("Marker unknown");
	    break;
    }
    busy = FALSE;
}

gchar**
srs_gs_get_drivers (void)
{
    return srs_gs_wrap_get_drivers ();
}

gchar**
srs_gs_get_driver_voices (gchar *driver)
{
    return srs_gs_wrap_get_driver_voices (driver);
}


gboolean
srs_gs_init (SRSGSMarkersCallback callback)
{
    sru_assert (callback);

    srs_gs_callback_to_speech = callback;
    srs_gs_outs = NULL;
    srs_gs_start_marker_waiting = FALSE;
    
    return srs_gs_wrap_init (srs_gs_callback);
}

void
srs_gs_terminate ()
{
    sru_assert (srs_gs_outs == NULL);
/*    sru_assert (srs_gs_start_marker_waiting == FALSE);*/

    srs_gs_wrap_terminate ();
}

gboolean
srs_gs_shutup (void)
{
    GSList *crt, *tmp;

    tmp = srs_gs_outs;
    srs_gs_outs = NULL;

    for (crt = tmp; crt; crt = crt->next)
    	srs_gs_out_terminate (crt->data);

    g_slist_free (tmp);

    return TRUE;
}

#ifdef SRS_ENABLE_OPTIMIZATION
gboolean
srs_gs_speaker_same_as (SRSGSSpeaker *speaker1,
			SRSGSSpeaker *speaker2)
{
    sru_assert (speaker1 && speaker2);

    return (strcmp (speaker1->driver, speaker2->driver) == 0 &&
	    strcmp (speaker1->voice,  speaker2->voice)  == 0 &&
	    speaker1->rate   == speaker2->rate &&
	    speaker1->pitch  == speaker2->pitch &&
	    speaker1->volume == speaker2->volume);
}
#endif

#ifdef SRS_NO_MARKERS_SUPPORTED
gboolean
srs_gs_speaker_has_callback (SRSGSSpeaker *speaker)
{
    sru_assert (speaker);

    return speaker->has_callback;
}
#endif
