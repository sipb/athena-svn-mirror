/* brlinp.c
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

#include <libxml/parser.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include "brlinp.h"

/* GLOBALS */
static xmlSAXHandler*		brl_in_ctx;

static BRLInParserStates 	brl_in_curr_state = BIPS_IDLE;
static BRLInParserStates 	brl_in_prev_state = BIPS_IDLE;
static gint 			brl_in_unknown_depth = 0;

static BRLInEvent		brl_in_event = {0, {"NULL"}};	/* "internal" event */
static BRLInCallback 		client_callback = NULL;

static gboolean                 brl_in_initialized = FALSE;


/* __ SAX CALLBACKS ___________________________________________________________ */

void 
brl_in_start_document (void *ctx)
{
/*	fprintf (stderr, "BRL IN: startDocument\n"); */
}

void 
brl_in_end_document (void *ctx)
{
/*	fprintf (stderr, "BRL IN: endDocument\n"); */
}

void 
brl_in_start_element (void          *ctx,
		      const xmlChar *name, 
		      const xmlChar **attrs)
{	
    gchar* attr_val;
    gchar* tattr_val;
		
    switch (brl_in_curr_state)
    {
	case BIPS_IDLE:			
	    if (g_strcasecmp ((gchar*)name, "BRLIN") == 0)
	    {				
		brl_in_curr_state = BIPS_BRL_IN;
	    }
	break;
		
	case BIPS_BRL_IN:		
	    if (g_strcasecmp ((gchar*)name, "KEY") == 0)
	    {
		brl_in_event.event_data.key_codes=NULL;
		brl_in_curr_state = BIPS_KEY;
	    }
	    else if (g_strcasecmp ((gchar*)name, "SENSOR") == 0)
	    {
		brl_in_event.event_data.sensor_codes=NULL;
		if (attrs)
		{
		    while (*attrs)
		    {
			/* fprintf (stderr, "attr_val: %s\n", *attrs); */
											
			if (g_strcasecmp ((gchar*)*attrs, "bank") == 0)
			{
			    ++attrs;						
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);
							
			    g_free (attr_val);
			}
			else if (g_strcasecmp ((gchar*)*attrs, "display") == 0)
			{
			    ++attrs;
			    				
			    attr_val = g_strdup((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);
										
			    g_free (attr_val);
			}					
			else if (g_strcasecmp ((gchar*)*attrs, "technology") == 0)
			{
			    ++attrs;
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);
											
			    g_free (attr_val);
			}
			else
			{						
			    /* unsupported attribute */
			    fprintf (stderr, "SENSOR attribute ""%s"" is not supported\n", *attrs);
			    ++attrs;						
			}
			++attrs;					
		    }
		}		
		brl_in_curr_state = BIPS_SENSOR;
	    }
	    else if (g_strcasecmp ((gchar*)name, "SWITCH") == 0)
	    {
		brl_in_event.event_data.switch_codes=NULL;
		brl_in_curr_state = BIPS_SWITCH;				
	    }									
	break;
				
	case BIPS_UNKNOWN:
	    brl_in_prev_state = brl_in_curr_state;
	    ++brl_in_unknown_depth;
	break;
	
	default:
	break;
    }		
}


void 
brl_in_characters (void          *ctx, 
		   const xmlChar *ch, 
		   gint          len)
{
    gchar* tch;
		
    tch = g_strndup((gchar*)ch, len);
	
    switch (brl_in_curr_state)
    {			
	case BIPS_KEY:
	    brl_in_event.event_data.key_codes=tch;
	break;
		
	case BIPS_SENSOR:
	    brl_in_event.event_data.sensor_codes=tch;
	break;
		
	case BIPS_SWITCH:	
	    brl_in_event.event_data.switch_codes=tch;
	break;
		
	case BIPS_IDLE:		
	case BIPS_BRL_IN:
	case BIPS_UNKNOWN:
	break;	
    }
	
    /* g_free (tch); !!! don't delete it here !!! */	
}

void 
brl_in_end_element (void           *ctx, 
                    const xmlChar* name)
{	
    switch (brl_in_curr_state)
    {				
	case BIPS_IDLE:
	break;
		
	case BIPS_BRL_IN:
	    if (g_strcasecmp ((gchar*)name, "BRLIN") == 0)
	    {
		brl_in_curr_state = BIPS_IDLE;
	    }
	break;
	
	case BIPS_KEY:
	    if (g_strcasecmp ((gchar*)name, "KEY") == 0)
	    {
		/* fire event */
		brl_in_event.event_type = BIET_KEY;				
		if (client_callback) 
		    client_callback (&brl_in_event);
		g_free (brl_in_event.event_data.key_codes);								
		brl_in_curr_state = BIPS_BRL_IN;
	    }
	break;
		
	case BIPS_SENSOR:
	    if (g_strcasecmp ((gchar*)name, "SENSOR") == 0)
	    {
		/* fire event */
		brl_in_event.event_type = BIET_SENSOR;
		if (client_callback) 
		    client_callback (&brl_in_event);
		g_free (brl_in_event.event_data.sensor_codes);
		brl_in_curr_state = BIPS_BRL_IN;
	    }
	break;
		
	case BIPS_SWITCH:	
	    if (g_strcasecmp ((gchar*)name, "SWITCH") == 0)
	    {
		/* fire event */
		brl_in_event.event_type = BIET_SWITCH;
		if (client_callback) 
		    client_callback (&brl_in_event);
		g_free (brl_in_event.event_data.switch_codes);
		brl_in_curr_state = BIPS_BRL_IN;
	    }
	break;
		
	case BIPS_UNKNOWN:
	    --brl_in_unknown_depth;
	    if (brl_in_unknown_depth <= 0)
	    {
		brl_in_curr_state = brl_in_prev_state;
	    }
	break;	
    }
}

void 
brl_in_warning (void        *ctx, 
	        const gchar *msg, ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("BRL XML IN", G_LOG_LEVEL_WARNING, msg, args);
    va_end (args);
}

void 
brl_in_error (void        *ctx, 
	      const gchar *msg, ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("BRL XML IN", G_LOG_LEVEL_CRITICAL, msg, args);
    va_end (args);
}

void 
brl_in_fatal_error (void        *ctx, 
		   const gchar  *msg, ...)
{
    va_list args;
    
    va_start (args, msg);
    g_logv ("BRL XML IN", G_LOG_LEVEL_ERROR, msg, args);
    va_end (args);
}

/* API */
gint 
brl_in_xml_init (BRLInCallback callback_proc)
{
    if (!brl_in_initialized)
    {
	/* store client callback */
	client_callback = callback_proc;
	
	xmlInitParser();
	
	/* initialize the XML parser */
	
	brl_in_ctx = g_malloc0(sizeof(xmlSAXHandler));
		
	brl_in_ctx->startDocument = brl_in_start_document;
	brl_in_ctx->endDocument = brl_in_end_document;
	brl_in_ctx->startElement = brl_in_start_element;
	brl_in_ctx->endElement = brl_in_end_element;
	brl_in_ctx->characters = brl_in_characters;

	brl_in_ctx->warning =   brl_in_warning;
	brl_in_ctx->error = brl_in_error;
	brl_in_ctx->fatalError = brl_in_fatal_error;
	
	brl_in_initialized = TRUE;
    }
    else
    {
	fprintf (stderr, "WARNING: brl_in_xml_init called more than once.\n");
    }
    return 1;	/* !!! TBR !!! return true only if everything went OK up there */
}

void
brl_in_xml_terminate ()
{
    if (brl_in_initialized)
    {
	if (brl_in_ctx)
	{
	    g_free (brl_in_ctx);
	}
	
	brl_in_initialized = FALSE;	
		
    }
    else
    {
	fprintf (stderr, "WARNING: brl_in_xml_terminate called more than once.\n");
    }
}

void 
brl_in_xml_parse (gchar *buffer, 
		  gint  len)
{
    xmlSAXParseMemory (brl_in_ctx, buffer, len, 0);
}
