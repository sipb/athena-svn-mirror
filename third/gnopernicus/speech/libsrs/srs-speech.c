/* srs-speech.c
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
#include <string.h>
#include "srs-speech.h"
#include "SRMessages.h"
#include "srs-gs.h"
#include "srintl.h"


static SRSOut		  *srs_crt_out 	    	  = NULL;
#ifdef SRS_NO_MARKERS_SUPPORTED
static GSList		  *srs_unspoken_outs	  = NULL;
static gboolean		   srs_no_markers_present = FALSE;
#endif
static GSList		  *srs_text_outs_speaking = NULL;
static GHashTable 	  *srs_voices		  = NULL;
static SRSMarkersCallback srs_sp_callback	  = NULL;

typedef struct _SRSVoice
{
    gchar *id;
    SRSGSSpeaker *speaker;
}SRSVoice;

static SRSVoice*
srs_voice_new ()
{
    return g_new0 (SRSVoice, 1);
}

static void
srs_voice_terminate (SRSVoice *voice)
{
    sru_assert (voice);

    g_free (voice->id);
    srs_gs_speaker_terminate (voice->speaker);
    g_free (voice);
}


SRSVoiceInfo*
srs_voice_info_new (void)
{
    SRSVoiceInfo *rv;

    rv = g_new0 (SRSVoiceInfo, 1);
    rv->rate = rv->pitch = rv->volume = -1;

    return rv;
}

void
srs_voice_info_terminate (SRSVoiceInfo *voice)
{
    sru_assert (voice);

    g_free (voice->id);
    g_free (voice->driver);
    g_free (voice->voice);
    g_free (voice);
}

static SRSVoice*
srs_voice_find (gchar *name)
{
    sru_assert (name);

    return g_hash_table_lookup (srs_voices, name);
}

static gboolean
srs_voice_add (SRSVoice *voice)
{
    sru_assert (voice && voice->id);

    g_hash_table_insert (srs_voices, voice->id, voice);

    return TRUE;
}

#ifdef SRS_NO_MARKERS_SUPPORTED
static void
srs_check_for_callbacks (gpointer key,
			 gpointer val,
			 gpointer data)
{
    gchar *name = key;
    SRSVoice *voice = val;

    sru_assert (name && voice);

    if (!srs_no_markers_present)
	srs_no_markers_present = !srs_gs_speaker_has_callback (voice->speaker);
}
#endif

gboolean
srs_voice_update_from_info (SRSVoiceInfo *voice)
{
    SRSVoice *found;

    sru_assert (voice && voice->id);

/*fprintf (stderr, "\nVOICE:\tn=%s\td=%s\tdv=%s\tr=%d\tp=%d\tv=%d",
		voice->id,   voice->driver, voice->voice,
		voice->rate, voice->pitch,  voice->volume);
*/
    found = srs_voice_find (voice->id);
    if (found)
    	srs_gs_speaker_update (found->speaker, voice);
    else
    {
	found = srs_voice_new ();
	found->id = g_strdup (voice->id);
	found->speaker = srs_gs_speaker_new (voice);
	srs_voice_add (found);
    }

#ifdef SRS_NO_MARKERS_SUPPORTED
    srs_no_markers_present = FALSE;
    g_hash_table_foreach (srs_voices, srs_check_for_callbacks, NULL);
#endif

    return TRUE;
}

SRSTextOut*
srs_text_out_new (void)
{
    return g_new0 (SRSTextOut, 1);
}

void
srs_text_out_terminate (SRSTextOut *tout)
{
    sru_assert (tout);

    g_free (tout->text);
    g_free (tout->voice);
    g_free (tout->id);
    g_free (tout);
}

SRSOut*
srs_out_new (void)
{
    SRSOut *rv;

    rv = g_new0 (SRSOut, 1);
    rv->texts = g_ptr_array_new ();

    return rv;
}

void
srs_out_terminate (SRSOut *out)
{
    gint i;

    sru_assert (out);

    for (i = 0; i < out->texts->len; i++)
	srs_text_out_terminate (g_ptr_array_index (out->texts, i));
    g_ptr_array_free (out->texts, TRUE);
    g_free (out->id);
    g_free (out);
}

static gboolean
srs_out_terminate_idle (gpointer data)
{
    srs_out_terminate (data);

    return FALSE;
}

SRSMarker*
srs_marker_new ()
{
    SRSMarker *rv;

    rv = g_new0 (SRSMarker, 1);
    rv->offset = -1;

    return rv;
}

void
srs_marker_terminate (SRSMarker *marker)
{
    sru_assert (marker);

    g_free (marker);
}

#ifdef SRS_ENABLE_OPTIMIZATION
static gboolean
srs_has_same_voice_as (SRSTextOut *tout1,
		       SRSTextOut *tout2)
{
    SRSVoice *voice1, *voice2;
    sru_assert (tout1 && tout2);
    sru_assert (tout1->voice && tout2->voice);

    if (strcmp (tout1->voice, tout2->voice) == 0)
	return TRUE;

    voice1 = srs_voice_find (tout1->voice);
    voice2 = srs_voice_find (tout2->voice);

    sru_assert (voice1 && voice2);

    return srs_gs_speaker_same_as (voice1->speaker, voice2->speaker);
}
#endif


gboolean
srs_out_add_text_out (SRSOut     *out,
		      SRSTextOut *tout)
{
    sru_assert (out && tout);
#ifdef SRS_ENABLE_OPTIMIZATION
    if (out->texts->len > 0 && tout->spelling == SRS_SPELLING_NONE)
    {
	SRSTextOut *last = g_ptr_array_index (out->texts, out->texts->len - 1);
	if (last->spelling == SRS_SPELLING_NONE && 
				    srs_has_same_voice_as (tout, last))
	{
	    gchar *text = last->text;
	    last->text = g_strconcat (text, " ", tout->text, NULL);
	    g_free (text);
	    srs_text_out_terminate (tout);
	    out->markers &= (SRS_MARKER_OUTPUT_STARTED | SRS_MARKER_OUTPUT_ENDED);
	}
	else
	    g_ptr_array_add (out->texts, tout);
    }
    else
	g_ptr_array_add (out->texts, tout);
#else
    g_ptr_array_add (out->texts, tout);
#endif

    return TRUE;
}


gchar**
srs_sp_get_drivers (void)
{
    return srs_gs_get_drivers ();
}

gchar**
srs_sp_get_driver_voices (gchar *driver)
{
    return srs_gs_get_driver_voices (driver); 
}

static struct
{
    gunichar ch;
    gchar   *cbc; 
    gchar   *military; /* To translators: This is the military phonetic alphabet */
}srs_sp_spell_table[] =
    {
	{'a',	"a",		N_("Alpha")	},
	{'b',	"b",		N_("Bravo")	},
	{'c',	"c",		N_("Charlie")	},
	{'d',	"d",		N_("Delta")	},
	{'e',	"e",		N_("Echo")	},
	{'f',	"f",		N_("Foxtrot")	},
	{'g',	"g",		N_("Golf")	},
	{'h',	"h",		N_("Hotel")	},
	{'i',	"i",		N_("India")	},
	{'j',	"j",		N_("Juliet")	},
	{'k',	"k",		N_("Kilo")	},
	{'l',	"l",		N_("Lima")	},
	{'m',	"m",		N_("Mike")	},
	{'n',	"n",		N_("November")	},
	{'o',	"o",		N_("Oscar")	},
	{'p',	"p",		N_("Papa")	},
	{'q',	"q",		N_("Quebec")	},
	{'r',	"r",		N_("Romeo")	},
	{'s',	"s",		N_("Sierra")	},
	{'t',	"t",		N_("Tango")	},
	{'u',	"u",		N_("Uniform")	},
	{'v',	"v",		N_("Victor")	},
	{'w',	"w",		N_("Whiskey")	},
	{'x',	"x",		N_("Xray")	},
	{'y',	"y",		N_("Yankee")	},
	{'z',	"z",		N_("Zulu")	},
	{' ',	N_("space"),	N_("space")	},
	{'\t',	N_("tab"),	N_("tab")	},
	{'\n',	N_("new line"),	N_("new line")	},
	{'-',	N_("dash"),	N_("dash")	}
    };

static gint
srs_sp_letter_get_index_for_spell (gunichar letter)
{
    gint i;
    gunichar letterl;

    sru_assert (g_unichar_validate (letter));

    letterl = g_unichar_tolower (letter);
    for (i = 0; i < G_N_ELEMENTS (srs_sp_spell_table); i++)
	if (letterl == srs_sp_spell_table[i].ch)
	    return i;

    return -1;
}

static gboolean
srs_speak_text_out (SRSTextOut *tout)
{
    SRSVoice *voice;

    sru_assert (tout);

    voice = srs_voice_find (tout->voice);
    sru_assert (voice);
    srs_text_outs_speaking = g_slist_append (srs_text_outs_speaking, tout);
    if (tout->spelling == SRS_SPELLING_NONE)
        srs_gs_speaker_say (voice->speaker, tout->text, tout, -1);
    else if (tout->spelling == SRS_SPELLING_CHAR ||
	     tout->spelling == SRS_SPELLING_MILITARY)
    {
	gint i;
	gchar *crt; 

	sru_assert (tout->text && g_utf8_validate (tout->text, -1, NULL));
	for (i = 0, crt = tout->text; *crt; i++, crt = g_utf8_next_char (crt))
	{
	    gunichar letter;
	    GString *text;
	    gint index;

	    letter = g_utf8_get_char (crt);
	    sru_assert (g_unichar_validate (letter));
	    index = srs_sp_letter_get_index_for_spell (letter);

	    text = g_string_new ("");
/*	    if (g_unichar_isupper (letter)) 
	    {
		g_string_append (text, _("upper"));
		g_string_append (text, " ");
	    }*/
	    if (index >= 0)
		g_string_append (text,
			    tout->spelling == SRS_SPELLING_CHAR ? 
				    _(srs_sp_spell_table[index].cbc) : 
				    _(srs_sp_spell_table[index].military));
	    else
		g_string_append_unichar (text, letter);
	    srs_gs_speaker_say (voice->speaker, text->str, tout, i);
	    g_string_free (text, TRUE);
	}
    }
    else
	sru_assert_not_reached ();

    return TRUE;
}

gboolean
srs_sp_speak_out (SRSOut *out)
{
    gint i;

    sru_assert (out && out->texts && out->texts->len);
#ifdef SRS_NO_MARKERS_SUPPORTED
    if (srs_no_markers_present && srs_crt_out)
    {
	g_slist_append (srs_unspoken_outs, srs_crt_out);
	srs_crt_out = NULL;
    }
#endif

    sru_assert (srs_crt_out == NULL);

    srs_crt_out = out;

    for (i = 0; i < srs_crt_out->texts->len; i++)
	srs_speak_text_out (g_ptr_array_index (srs_crt_out->texts, i));

    return TRUE; 
}

gboolean
srs_sp_shutup (void)
{
    GSList *speaking, *crt;

    if (!srs_crt_out)
	return TRUE;

    srs_gs_shutup ();

    speaking = srs_text_outs_speaking;
    srs_text_outs_speaking = NULL;
    for (crt = speaking; crt; crt = crt->next)
    {
	SRSVoice *voice;
	sru_assert (crt->data);
	voice = srs_voice_find (((SRSTextOut*)crt->data)->voice);
	sru_assert (voice);
	srs_gs_speaker_shutup (voice->speaker);
    }
    g_slist_free (speaking);

    srs_out_terminate (srs_crt_out);
    srs_crt_out = NULL;
#ifdef SRS_NO_MARKERS_SUPPORTED
    if (srs_no_markers_present)
    {
	GSList *crt;
	for (crt = srs_unspoken_outs; crt; crt = crt->next)
	    srs_out_terminate (crt->data);
	g_slist_free (srs_unspoken_outs);
	srs_unspoken_outs = NULL;
    }
#endif

    return TRUE;
}

gboolean
srs_sp_pause (void)
{
    return FALSE;
}

gboolean
srs_sp_resume (void)
{
    return FALSE;
}

static gboolean
srs_sp_callback_wrap_idle (gpointer data)
{
    SRSMarker *mk;

    mk = data;
    sru_assert (mk);
    sru_assert (srs_sp_callback);
    srs_sp_callback(mk);
    srs_marker_terminate (mk);

    return FALSE;
}

static gboolean
srs_sp_callback_wrap (SRSOut        *out,
		      SRSTextOut    *tout,
		      SRSMarkerType  marker,
		      gint 	     offset)
{
    SRSMarker *mk;


    mk = srs_marker_new ();
    mk->out    = out;
    mk->tout   = tout;
    mk->type = marker;
    mk->offset = offset;

    /*g_idle_add (srs_sp_callback_wrap_idle, mk); */
    srs_sp_callback_wrap_idle (mk);
    
    return TRUE;
}

static void
srs_sp_clb (gpointer 	   id,
	    SRSMarkerType  marker,
	    gint	   offset)
{
    static gpointer last = NULL;
    SRSTextOut	   *tout = id;

#ifdef SRS_NO_MARKERS_SUPPORTED
    if (srs_no_markers_present)
	return;
#endif

    if (marker == SRS_MARKER_TEXT_ENDED && g_slist_find (srs_text_outs_speaking, tout))
    {
	sru_assert (g_slist_find (srs_text_outs_speaking, tout) == srs_text_outs_speaking); /* should be for first position */
	sru_assert (srs_crt_out);

	if (offset > 0 && offset == g_utf8_strlen (((SRSTextOut*)srs_text_outs_speaking->data)->text, -1) - 1)
	    offset = -1; /* is last char, end of speech must be simulated */
	
	if (offset < 0) /* needed because of char by char and military modes */
	{
	    if (srs_crt_out->markers & SRS_MARKER_TEXT_ENDED)
		srs_sp_callback_wrap (srs_crt_out, tout, SRS_MARKER_TEXT_ENDED, -1);
	    srs_text_outs_speaking = g_slist_remove (srs_text_outs_speaking, tout);
	    if (!srs_text_outs_speaking)
	    {
		SRSOut *tmp = srs_crt_out;
		srs_crt_out = NULL;
		if (tmp->markers & SRS_MARKER_OUTPUT_ENDED)
    		    srs_sp_callback_wrap (tmp, NULL, SRS_MARKER_OUTPUT_ENDED, -1);
		g_idle_add (srs_out_terminate_idle, tmp);
	    }
	}
    }
    else if (marker == SRS_MARKER_TEXT_STARTED && g_slist_find (srs_text_outs_speaking, tout))
    {
	sru_assert (g_slist_find (srs_text_outs_speaking, tout) == srs_text_outs_speaking); /* should be for first position */
	sru_assert (srs_crt_out);
	if (offset <= 0) /* needed because of char by char and military modes */
	{
	    if (srs_crt_out->markers & SRS_MARKER_OUTPUT_STARTED && last != srs_crt_out)
		srs_sp_callback_wrap (srs_crt_out, NULL, SRS_MARKER_OUTPUT_STARTED, -1);
	
	    if (srs_crt_out->markers & SRS_MARKER_TEXT_STARTED)
		srs_sp_callback_wrap (srs_crt_out, tout, SRS_MARKER_TEXT_STARTED,  -1);
	}
	if (offset >= 0)
	    srs_sp_callback_wrap (srs_crt_out, tout, SRS_MARKER_TEXT_PROGRESS, offset);
    }
    last = srs_crt_out;
}

gboolean
srs_sp_init (SRSMarkersCallback callback)
{
    sru_assert (callback);

    srs_crt_out 	    	= NULL;
#ifdef SRS_NO_MARKERS_SUPPORTED
    srs_unspoken_outs		= NULL;
    srs_no_markers_present	= FALSE;
#endif
    srs_text_outs_speaking	= NULL;
    srs_sp_callback		= callback;

    srs_voices = g_hash_table_new_full (g_str_hash, g_str_equal, NULL,
			(GDestroyNotify)srs_voice_terminate);

    return srs_gs_init (srs_sp_clb);
}

void
srs_sp_terminate (void)
{
    sru_assert (srs_crt_out == NULL);
#ifdef SRS_NO_MARKERS_SUPPORTED
    sru_assert (srs_unspoken_outs == NULL);
#endif
    sru_assert (srs_text_outs_speaking == NULL);

    g_hash_table_destroy (srs_voices);

    srs_gs_terminate ();
}

#ifdef SRS_NO_MARKERS_SUPPORTED
gboolean
srs_voice_has_callback (gchar *name)
{
    SRSVoice *voice;

    sru_assert (name);

    voice = srs_voice_find (name);
    sru_assert (voice);

    return srs_gs_speaker_has_callback (voice->speaker);
}
#endif
