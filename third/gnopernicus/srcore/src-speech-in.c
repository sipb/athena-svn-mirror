 /* src-speech-in.c
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
#include <libxml/parser.h>
#include "src-speech-in.h"
#include "srspc.h"
#include "SRMessages.h"
#include <string.h>

typedef enum 
{
    SRS_MARKER_OUTPUT_STARTED = 1 << 0,
    SRS_MARKER_OUTPUT_ENDED   = 1 << 1,
    SRS_MARKER_TEXT_STARTED   = 1 << 2,
    SRS_MARKER_TEXT_ENDED     = 1 << 3,
    SRS_MARKER_TEXT_PROGRESS  = 1 << 4
}SRSMarkerType;

typedef enum
{
    SRS_IDLE,
    SRS_IN,
    SRS_DRIVER,
    SRS_DRIVER_VOICE,
    SRS_MARKER,
    SRS_VOICECREATED
}SRSParserState;

typedef struct
{
    SRSMarkerType	type;
    gchar		*idt;
    gchar		*ido;
    gint		offset;
}SRSMarker;

static xmlSAXHandler  	*srs_ctx 	= NULL;
static SRSParserState 	 srs_crt_state 	= SRS_IDLE;
static SRSDriver 	*srs_crt_driver = NULL;
static GPtrArray 	*srs_all_drivers= NULL;
static GPtrArray 	*srs_all_voices = NULL;

static SRSMarker*
srs_marker_new ()
{
    return g_new0 (SRSMarker, 1);
}

static void
srs_marker_terminate (SRSMarker *marker)
{
    sru_assert (marker);

    g_free (marker->idt);
    g_free (marker->ido);
    g_free (marker);
}


static gboolean
srs_process_marker (SRSMarker *marker)
{
    sru_assert (marker);

    switch (marker->type)
    {
	case SRS_MARKER_OUTPUT_ENDED:
	    src_speech_set_idle (marker->ido);
	    break;
	case SRS_MARKER_OUTPUT_STARTED:
	case SRS_MARKER_TEXT_STARTED:
	case SRS_MARKER_TEXT_ENDED:
	case SRS_MARKER_TEXT_PROGRESS:
	    break;
	default:
	    sru_assert_not_reached ();
    }

    return TRUE;
}

static gboolean
srs_set_attrs_for_driver (SRSDriver *driver,
			  gchar **attrs)
{
    sru_assert (driver);
    sru_assert (attrs);

    while (*attrs)
    {
	gchar *attr, *val;
	
	attr = *attrs;
	++attrs;
	val = *attrs;
	++attrs;
	if (g_strcasecmp (attr, "name") == 0)
	     driver->name = g_strdup (val);
	else
	    sru_assert_not_reached ();
    }
    return TRUE;
}

static gboolean
srs_set_attrs_for_voicecreated (gchar **attrs)
{
    gchar *name = NULL;
#ifdef SRS_NO_MARKERS_SUPPORTED
    gboolean has_callback = FALSE;
#endif
    sru_assert (attrs);

    while (*attrs)
    {
	gchar *attr, *val;
	
	attr = *attrs;
	++attrs;
	val = *attrs;
	++attrs;
	if (g_strcasecmp (attr, "name") == 0)
	     name = val;
#ifdef SRS_NO_MARKERS_SUPPORTED
	else if (g_strcasecmp (attr, "callback") == 0)
	    has_callback = g_strcasecmp (val, "yes") == 0;
#endif
	else
	    sru_assert_not_reached ();
    }
    sru_assert (name);
#ifdef SRS_NO_MARKERS_SUPPORTED
    src_speech_add_created_voice (name, has_callback);
#else
    src_speech_add_created_voice (name);
#endif

    return TRUE;
}

static gboolean
srs_set_attrs_for_voice (gchar **attrs)
{
    sru_assert (attrs);
    sru_assert (srs_all_voices);

    while (*attrs)
    {
	gchar *attr, *val;
	
	attr = *attrs;
	++attrs;
	val = *attrs;
	++attrs;
	if (g_strcasecmp (attr, "name") == 0)
	    g_ptr_array_add (srs_all_voices, g_strdup (val));
	else
	    sru_assert_not_reached ();
    }
    return TRUE;
}


static gboolean
srs_set_attrs_for_marker (SRSMarker *marker, 
			  gchar **attrs)
{
    sru_assert (marker);
    sru_assert (attrs);

    while (*attrs)
    {
	gchar *attr, *val;
	
	attr = *attrs;
	++attrs;
	val = *attrs;
	++attrs;
	if (g_strcasecmp (attr, "type") == 0)
	{
	    if (g_strcasecmp (val, "out-started") == 0)
		marker->type = SRS_MARKER_OUTPUT_STARTED;
	    else if (g_strcasecmp (val, "out-ended") == 0)
		marker->type = SRS_MARKER_OUTPUT_ENDED;
	    else if (g_strcasecmp (val, "text-started") == 0)
		marker->type = SRS_MARKER_TEXT_STARTED;
	    else if (g_strcasecmp (val, "text-ended") == 0)
		marker->type = SRS_MARKER_TEXT_ENDED;
	    else if (g_strcasecmp (val, "text-progress") == 0)
		marker->type = SRS_MARKER_TEXT_PROGRESS;
	    else
		sru_assert_not_reached ();
	}
	else if (g_strcasecmp (attr, "idt") == 0)   
	    marker->idt = g_strdup (val);
	else if (g_strcasecmp (attr, "ido") == 0)   
	    marker->ido = g_strdup (val);
	else if (g_strcasecmp (attr, "offset") == 0)   
	    marker->offset = atoi (val);
    	else
	    sru_assert_not_reached ();
    }
    return TRUE;
}

static void
srs_startElement (void *ctx, 
		  const xmlChar *name_, 
		  const xmlChar **attrs_)
{	
	gchar *name = (gchar*)name_;
	gchar **attrs = (gchar**)attrs_;
		
	switch (srs_crt_state)
	{
	    case SRS_IDLE:
		if (g_strcasecmp (name, "SRSIN") == 0)
		{
		    sru_assert (srs_all_drivers == NULL);
		    srs_crt_state = SRS_IN;
		}
		else
		    sru_assert_not_reached ();
		break;
	    case SRS_IN:
		if (g_strcasecmp (name, "MARKER") == 0)
		{
		    SRSMarker *marker = srs_marker_new ();
		    if (attrs)
		    	srs_set_attrs_for_marker (marker, attrs);
		    srs_process_marker (marker);
		    srs_marker_terminate (marker);
		    srs_crt_state = SRS_MARKER;
		}
		else if (g_strcasecmp (name, "DRIVER") == 0)
		{
		    sru_assert (srs_crt_driver == NULL);
		    sru_assert (srs_all_voices == NULL);
		    srs_all_voices = g_ptr_array_new ();
		    if (!srs_all_drivers)
			srs_all_drivers = g_ptr_array_new ();
		    srs_crt_driver = srs_driver_new ();					
		    if (attrs)
		    	srs_set_attrs_for_driver (srs_crt_driver, attrs);
		    srs_crt_state = SRS_DRIVER;
		}
		else if (g_strcasecmp (name, "VOICECREATED") == 0)
		{
		    if (attrs)
		    	srs_set_attrs_for_voicecreated (attrs);
		    srs_crt_state = SRS_VOICECREATED;
		}
		else
		    sru_assert_not_reached ();					
		break;
	    case SRS_DRIVER:
		if (g_strcasecmp (name, "VOICE") == 0)
		{
		    if (attrs)
			srs_set_attrs_for_voice (attrs);
    		    srs_crt_state = SRS_DRIVER_VOICE;
		}
		break;
	    case SRS_MARKER:
	    case SRS_DRIVER_VOICE:
	    	break;
	    default:
		sru_assert_not_reached ();
		break;
	}
}


static void
srs_endElement (void *ctx,
		const xmlChar* name_)
{
    gchar *name = (gchar*)name_;

    switch (srs_crt_state)
    {
	case SRS_IDLE:
	    break;
	case SRS_IN:
	    if (g_strcasecmp (name, "SRSIN") == 0)
	    {
		if (srs_all_drivers)
		{
		    src_speech_process_drivers (srs_all_drivers);
		    srs_all_drivers = NULL; /* srs_all_drivers will be freed in src_speech_process_drivers */
		}
		srs_crt_state = SRS_IDLE;
	    }
	    break;
	case SRS_DRIVER:
	    if (g_strcasecmp (name, "DRIVER") == 0)
	    {
		sru_assert (srs_all_drivers);
		sru_assert (srs_crt_driver);
		sru_assert (srs_all_voices);
		srs_crt_state = SRS_IN;
		g_ptr_array_add (srs_all_voices, NULL);
		srs_crt_driver->voices = (gchar**)g_ptr_array_free (srs_all_voices, FALSE);
		g_ptr_array_add (srs_all_drivers, srs_crt_driver);
		srs_crt_driver = NULL;
		srs_all_voices = NULL;
	    }			
	    break;
	case SRS_MARKER:
	    if (g_strcasecmp (name, "MARKER") == 0)
	    {
		srs_crt_state = SRS_IN;
	    }			
	    break;
	case SRS_DRIVER_VOICE:
	    if (g_strcasecmp(name, "VOICE") == 0)
	    {
		srs_crt_state = SRS_DRIVER;
	    }
	    break;
	case SRS_VOICECREATED:
	    if (g_strcasecmp (name, "VOICECREATED") == 0)
		srs_crt_state = SRS_IN;
	    break;
	default:
	    sru_assert_not_reached ();
    }
}

static void 
srs_warning (void *ctx,
	     const gchar *msg,
	     ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("SRSXML", G_LOG_LEVEL_WARNING, msg, args);
    va_end (args);
}

static void 
srs_error (void *ctx,
	   const gchar *msg,
	   ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("SRS XML", G_LOG_LEVEL_CRITICAL, msg, args);
    va_end (args);
}

static void 
srs_fatalError (void *ctx,
		const gchar *msg,
		...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("SRS XML", G_LOG_LEVEL_ERROR, msg, args);
    va_end (args);
}


void 
srcs_in_parse_input (gchar* buffer,
		     gint len)
{
    sru_assert (buffer);
    sru_assert (len == strlen (buffer));

    /* fprintf (stderr, "\n%s", buffer); */
    xmlSAXParseMemory (srs_ctx, buffer, len, 0); 
}

gboolean
srcs_in_init ()
{
    xmlInitParser ();	
    
    srs_all_drivers = NULL;
    srs_all_voices  = NULL;
    srs_crt_driver  = NULL;
    srs_crt_state   = SRS_IDLE;

    srs_ctx = g_new0 (xmlSAXHandler, 1);
    
    srs_ctx->startElement 	= srs_startElement;
    srs_ctx->endElement 	= srs_endElement;

    srs_ctx->warning 		= srs_warning;
    srs_ctx->error 		= srs_error;
    srs_ctx->fatalError 	= srs_fatalError;
    
    return TRUE;
}

void 
srcs_in_terminate ()
{
    sru_assert (srs_ctx);
    sru_assert (srs_all_drivers == NULL);
    sru_assert (srs_all_voices == NULL);
    sru_assert (srs_crt_driver == NULL);
 
    g_free (srs_ctx);
}
