/* srs-xml.c
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

#include "config.h"	
#include <libxml/parser.h>
#include <string.h>
#include "srs-xml.h"
#include "srs-speech.h"
#include "SRMessages.h"

typedef enum
{
    SRS_PS_IDLE,
    SRS_PS_OUT,
    SRS_PS_VOICE,
    SRS_PS_TEXT,
    SRS_PS_SHUT_UP,
    SRS_PS_PAUSE,
    SRS_PS_RESUME
}SPSParserState;

static gboolean 	srs_xml_initialized  = FALSE;
static xmlSAXHandler  	*srs_ctx	     = NULL;
static SPSParserState 	srs_crt_state 	     = SRS_PS_IDLE;
static SRSOut     	*srs_crt_out 	     = NULL;
static SRSTextOut 	*srs_crt_text_out    = NULL;
static SRSVoiceInfo   	*srs_crt_voice 	     = NULL;
static SRSCallback 	srs_xml_callback     = NULL;

static gint
srs_get_new_int_val (gint old,
		     gchar *add)
{
    gint rv = old;

    sru_assert (add);

    if (add[0] != '+' && add[0] != '-')
	rv = 0;
    rv += atoi (add);

    return rv;
}

static gboolean
srs_set_attrs_for_out (SRSOut *out,
		       gchar **attrs)
{
    sru_assert (out && attrs);

    while (*attrs)
    {
	gchar *attr, *val;
	attr = *attrs;
	++attrs;
	val = *attrs;
	++attrs;
	if (g_strcasecmp (attr, "id") == 0)
	    out->id = g_strdup (val);
	else if (g_strcasecmp (attr, "markers") == 0)
	{
	    gchar **vals = g_strsplit (val, ":", -1);
	    gint i;
	    for (i = 0; vals[i]; i++)
	    {
		if (strcmp (vals[i], "out-started") == 0)
		    out->markers |= SRS_MARKER_OUTPUT_STARTED;
		else if (strcmp (vals[i], "out-ended") == 0)
		    out->markers |= SRS_MARKER_OUTPUT_ENDED;
		else if (strcmp (vals[i], "text-started") == 0)
		    out->markers |= SRS_MARKER_TEXT_STARTED;
		else if (strcmp (vals[i], "text-ended") == 0)
		    out->markers |= SRS_MARKER_TEXT_ENDED;
		else if (strcmp (vals[i], "text-progress") == 0)
		    out->markers |= SRS_MARKER_TEXT_PROGRESS;
		else
		    sru_assert_not_reached ();
	    }
	    g_strfreev (vals);
	}
	else
	    sru_assert_not_reached ();
    }

    return TRUE;
}

static gboolean
srs_set_attrs_for_text_out (SRSTextOut *tout,
		    	    gchar **attrs)
{
    sru_assert (tout && attrs);

    while (*attrs)
    {
        gchar *attr, *val;
	attr = *attrs;
	++attrs;
	val = (gchar*)*attrs;
	++attrs;
	if (g_strcasecmp (attr, "voice") == 0)
	    tout->voice = g_strdup (val);
	else if (g_strcasecmp (attr, "id") == 0)
	    tout->id = g_strdup (val);
	else if (g_strcasecmp (attr, "spelling") == 0)
	{
	    if (g_strcasecmp (val, "military") == 0)
		tout->spelling = SRS_SPELLING_MILITARY;
	    else if (g_strcasecmp (val, "char") == 0)
		tout->spelling = SRS_SPELLING_CHAR;
	    else
		sru_assert_not_reached ();
	}
	else
	    sru_assert_not_reached ();								
    }

    return TRUE;
}

static gboolean
srs_set_attrs_for_voice (SRSVoiceInfo *voice,
		    	 gchar **attrs)
{
    sru_assert (voice && attrs);

    while (*attrs)
    {
	gchar *attr, *val;
	attr = *attrs;
	++attrs;
	val = *attrs;
	++attrs;
	if (g_strcasecmp (attr, "ID") == 0)
	    voice->id = g_strdup (val);
	else if (g_strcasecmp (attr, "TTSEngine") == 0)
	    voice->driver = g_strdup (val);
	else if (g_strcasecmp (attr, "TTSVoice") == 0)
	    voice->voice = g_strdup (val);
	else if (g_strcasecmp (attr, "rate") == 0)
	    voice->rate = srs_get_new_int_val (srs_crt_voice->rate, val);
	else if (g_strcasecmp (attr, "pitch") == 0)
	    voice->pitch = srs_get_new_int_val (srs_crt_voice->pitch, val);
	else if (g_strcasecmp(attr, "volume") == 0)
	    voice->volume = srs_get_new_int_val (srs_crt_voice->volume, val);
	else
	    sru_assert_not_reached ();
    }

    return TRUE;
}

static void
srs_startElement (void           *ctx, 
		  const xmlChar  *name_,
		  const xmlChar **attrs_)
{
    gchar *name = (gchar*)name_;
    gchar **attrs = (gchar**)attrs_;

    switch (srs_crt_state)
    {
	case SRS_PS_IDLE:
	    if (g_strcasecmp (name, "SRSOUT") == 0)
	    {
		sru_assert (srs_crt_out == NULL);
		srs_crt_state = SRS_PS_OUT;
		srs_crt_out = srs_out_new ();
		if (attrs)
		    srs_set_attrs_for_out (srs_crt_out, attrs);
	    }
	    break;
	case SRS_PS_OUT:
	    if (g_strcasecmp (name, "SHUTUP") == 0)
	    	srs_crt_state = SRS_PS_SHUT_UP;
	    else if (g_strcasecmp (name, "PAUSE") == 0)
    		srs_crt_state = SRS_PS_PAUSE;
            else if (g_strcasecmp (name, "RESUME") == 0)
    		srs_crt_state = SRS_PS_RESUME;
	    else if (g_strcasecmp (name, "TEXT") == 0)
	    {
		sru_assert (srs_crt_text_out == NULL);
		srs_crt_state = SRS_PS_TEXT;
		srs_crt_text_out = srs_text_out_new ();
		if (attrs)
		    srs_set_attrs_for_text_out (srs_crt_text_out, attrs);
	    }
	    else if (g_strcasecmp(name, "VOICE") == 0)
	    {
		sru_assert (srs_crt_voice == NULL);
		srs_crt_state = SRS_PS_VOICE;
		srs_crt_voice = srs_voice_info_new ();
	    	if (attrs)
		    srs_set_attrs_for_voice (srs_crt_voice, attrs);
	    }
    	    else
		sru_assert_not_reached ();
	    break;
	case SRS_PS_TEXT:
	case SRS_PS_VOICE:
	case SRS_PS_SHUT_UP:
	case SRS_PS_PAUSE:
	case SRS_PS_RESUME:
	    break;
	default:
	    sru_assert_not_reached ();
	    break;
    }
}

static gboolean
srs_xml_callback_wrap_idle (gpointer data)
{
    GString *xml;

    xml = data;
    sru_assert (xml);
    sru_assert (srs_xml_callback);
    srs_xml_callback (xml->str, xml->len);
    g_string_free (xml, TRUE);

    return FALSE;
}

static gboolean
srs_xml_report_voice_creation_idle (gpointer data)
{
    gchar *name = data;
    GString *xml;

    sru_assert (name);
    
    xml = g_string_new ("");
#ifdef SRS_NO_MARKERS_SUPPORTED
    g_string_append_printf (xml, "<SRSIN><VOICECREATED name=\"%s\" callback=\"%s\"/></SRSIN>",
			 name, srs_voice_has_callback (name) ? "yes" : "no");
#else
    g_string_append_printf (xml, "<SRSIN><VOICECREATED name=\"%s\"/></SRSIN>", name);
#endif

    /* g_idle_add (srs_xml_callback_wrap_idle, xml); */
    srs_xml_callback_wrap_idle (xml);
    g_free (name);

    return FALSE;
}


static void
srs_endElement (void 		*ctx,
		const xmlChar	*name_)
{
    gchar *name = (gchar*)name_;

    switch (srs_crt_state)
    {
	case SRS_PS_OUT:
	    if (g_strcasecmp (name, "SRSOUT") == 0)
	    {
		if (srs_crt_out->texts->len)
		    srs_sp_speak_out (srs_crt_out);
		else
		    srs_out_terminate (srs_crt_out);
		srs_crt_out = NULL;
		srs_crt_state = SRS_PS_IDLE;
	    }
	    else
		sru_assert_not_reached ();
	    break;
	case SRS_PS_SHUT_UP:
	    if (g_strcasecmp (name, "SHUTUP") == 0)
	    {
		srs_sp_shutup ();
		srs_crt_state = SRS_PS_OUT;
	    }
	    else
		sru_assert_not_reached ();
	    break;
	case SRS_PS_PAUSE:
    	    if (g_strcasecmp (name, "PAUSE") == 0)
    	    {
    		srs_sp_pause();
    		srs_crt_state = SRS_PS_OUT;
    	    }
	    else
		sru_assert_not_reached ();
	    break;
	case SRS_PS_RESUME:
    	    if (g_strcasecmp (name, "RESUME") == 0)
    	    {
    		srs_sp_resume();
    		srs_crt_state = SRS_PS_OUT;
    	    }
	    else
		sru_assert_not_reached ();
	    break;		
	case SRS_PS_TEXT:
	    if (g_strcasecmp (name, "TEXT") == 0)
	    {				
		srs_out_add_text_out (srs_crt_out, srs_crt_text_out);
		srs_crt_text_out = NULL;
		srs_crt_state = SRS_PS_OUT;
	    }
	    else
		sru_assert_not_reached ();
	    break;
	case SRS_PS_VOICE:
	    if (g_strcasecmp (name, "VOICE") == 0)
	    {
		if (srs_voice_update_from_info (srs_crt_voice))
		{
		    sru_assert (srs_crt_voice->id);
		    /* g_idle_add (srs_xml_report_voice_creation_idle, g_strdup (srs_crt_voice->id)); */
		    srs_xml_report_voice_creation_idle (g_strdup (srs_crt_voice->id));
		}
		srs_voice_info_terminate (srs_crt_voice);
		srs_crt_voice = NULL;
		srs_crt_state = SRS_PS_OUT;
	    }
	    else
		sru_assert_not_reached ();
	    break;
	case SRS_PS_IDLE:
	    break;
	default:
	    sru_assert_not_reached ();
	    break;
    }
}

static void
srs_characters (void *ctx,
		const xmlChar *ch_,
		gint len)
{
    gchar *ch = (gchar*)ch_;

    switch (srs_crt_state)
    {
	case SRS_PS_TEXT:
	    {
		const gchar *end;
		gint count = len;
		sru_assert (srs_crt_text_out);
	        if (!g_utf8_validate (ch, len, &end))
		{
		    gchar *text = g_strndup (ch, len);
		    count = end - ch;
		    sru_warning ("invalid UTF-8 string \"%s\"", text);
		    g_free (text);
		}
		if (srs_crt_text_out->text)
		{
		    gchar *old = srs_crt_text_out->text;
		    gchar *tmp = g_strndup (ch, count);
		    srs_crt_text_out->text = g_strconcat (old, tmp, NULL);
		    g_free (tmp);
		    g_free (old);
		}
		else
		    srs_crt_text_out->text = g_strndup (ch, count);
	    }
	    break;
	case SRS_PS_IDLE:
	case SRS_PS_OUT:
	case SRS_PS_SHUT_UP:
	case SRS_PS_PAUSE:
	case SRS_PS_RESUME:
	case SRS_PS_VOICE:
	    break;
	default:
	    sru_assert_not_reached ();
	    break;
    }
}

static void
srs_warning (void *ctx,
    	     const char *msg,
	     ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("SRSXML", G_LOG_LEVEL_WARNING, msg, args);
    va_end (args);
}

static void
srs_error (void *ctx,
	   const char *msg,
	   ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("SRSXML", G_LOG_LEVEL_CRITICAL, msg, args);
    va_end (args);
}

static void
srs_fatalError (void *ctx,
		const char *msg,
		...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("SRSXML", G_LOG_LEVEL_ERROR, msg, args);
    va_end (args);
}

static void
srs_xml_markers_callback (SRSMarker *marker)
{
    GString *xml;
    
    sru_assert (marker);

    xml = g_string_new ("<SRSIN>");
    switch (marker->type)
    {
	case SRS_MARKER_OUTPUT_STARTED:
	    sru_assert (marker->out && marker->out->id);
	    g_string_append_printf (xml, "<MARKER type=\"out-started\" ido=\"%s\"/>", marker->out->id);
	    break;
	case SRS_MARKER_OUTPUT_ENDED:
	    sru_assert (marker->out && marker->out->id);
	    g_string_append_printf (xml, "<MARKER type=\"out-ended\" ido=\"%s\"/>", marker->out->id);
	    break;
	case SRS_MARKER_TEXT_STARTED:
	    sru_assert (marker->tout && marker->tout->id && marker->out && marker->out->id);
	    g_string_append_printf (xml, "<MARKER type=\"text-started\" idt=\"%s\" ido=\"%s\"/>", marker->tout->id, marker->out->id);
	    break;
	case SRS_MARKER_TEXT_ENDED:
	    sru_assert (marker->tout && marker->tout->id && marker->out && marker->out->id);
	    g_string_append_printf (xml, "<MARKER type=\"text-ended\" idt=\"%s\" ido=\"%s\"/>", marker->tout->id, marker->out->id);
	    break;
	case SRS_MARKER_TEXT_PROGRESS:
	    sru_assert (marker->tout && marker->tout->id && marker->out && marker->out->id);
	    g_string_append_printf (xml, "<MARKER type=\"text-progress\" idt=\"%s\" ido=\"%s\" offset=\"%d\"/>", marker->tout->id, marker->out->id, marker->offset);
	    break;
	default:
	    sru_assert_not_reached ();
	    break;
    }
    g_string_append (xml, "</SRSIN>");
    /* g_idle_add (srs_xml_callback_wrap_idle, xml); */
    srs_xml_callback_wrap_idle (xml);
}

static gboolean
srs_send_drivers_and_voices ()
{
    GString *xml;
    gchar **drivers;
    gint i;

    drivers = srs_sp_get_drivers ();
    if (!drivers)
	return FALSE;

    xml = g_string_new ("<SRSIN>");
    for (i = 0; drivers[i]; i++)
    {
	gchar **voices;
	gint j;

	g_string_append_printf (xml, "<DRIVER name=\"%s\">", drivers[i]);
	voices = srs_sp_get_driver_voices (drivers[i]);
	sru_assert (voices && voices[0]);
	for (j = 0; voices[j]; j++)
	    g_string_append_printf (xml, "<VOICE name=\"%s\"/>", voices[j]);
	g_string_append (xml, "</DRIVER>");
	g_strfreev (voices);
    }
    g_string_append (xml, "</SRSIN>");
    g_strfreev (drivers);

    /* g_idle_add (srs_xml_callback_wrap_idle, xml); */
    srs_xml_callback_wrap_idle (xml);

    return TRUE;
}


gboolean
srs_init (SRSCallback callback)
{
    sru_assert (srs_xml_initialized == FALSE);

    sru_assert (callback);

    srs_crt_state 	= SRS_PS_IDLE;
    srs_crt_out 	= NULL;
    srs_crt_text_out    = NULL;
    srs_crt_voice 	= NULL;
    srs_xml_callback	= callback;

    if (!srs_sp_init (srs_xml_markers_callback))
	return FALSE;
    if (!srs_send_drivers_and_voices ())
	return FALSE;

    xmlInitParser ();
    srs_ctx = g_new0 (xmlSAXHandler, 1);
    srs_ctx->startElement = srs_startElement;
    srs_ctx->endElement   = srs_endElement;
    srs_ctx->characters   = srs_characters;
    srs_ctx->warning 	  = srs_warning;
    srs_ctx->error 	  = srs_error;
    srs_ctx->fatalError	  = srs_fatalError;

    srs_xml_initialized = TRUE;

    return TRUE;
}

void 
srs_terminate ()
{
    sru_assert (srs_xml_initialized);

    g_free(srs_ctx);
    srs_sp_terminate();
    srs_xml_initialized = FALSE;
}

gboolean
srs_output (gchar* buffer,
	    gint len)
{
    xmlSAXParseMemory (srs_ctx, buffer, len, 0);

    return TRUE;
}
