/* magxml.c
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

#include "mag_ctrl.h"
#include "magxml.h"
#include "magxmlapi.h"

/*#define MAG_XML_DEBUG */

static xmlSAXHandler  *mag_ctx = NULL;

static MagParserState current_state  = MPS_IDLE;
static MagParserState previous_state = MPS_IDLE;
static int unknown_depth	     = 0       ;

static MagZoomer *current_mag_zoomer  = NULL   ;
static gboolean   found 	      = FALSE  ;
static int clear  		      = 0      ;
/* __ SAX CALLBACKS ___________________________________________________________*/

void 
mag_startDocument (void *ctx)
{
	found = FALSE;
#ifdef MAG_XML_DEBUG
	fprintf (stderr, "\nMAG: startDocument");
#endif	
}

void 
mag_endDocument (void *ctx)
{
#ifdef MAG_XML_DEBUG
	fprintf (stderr, "\nMAG: endDocument");
#endif		
}

void 
mag_startElement (void           *ctx, 
		  const xmlChar  *name, 
		  const xmlChar ** attrs)
{	
	gchar* attr_val  = NULL;
	gchar* tattr_val = NULL;
	
	found = FALSE;	
#ifdef MAG_XML_DEBUG
	fprintf (stderr, "\nMAG: startElement: %s %d", name, current_state);
#endif			
	switch (current_state)
	{
    		case MPS_IDLE:
			if (g_strcasecmp (name, "MAGOUT") == 0)
			{
				if (attrs)
				{	
					while (*attrs)
					{
						if (g_strcasecmp (*attrs, "clear") == 0)
						{
							++attrs;				
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val); /* trim the attr value*/

							clear = mag_zoomers_clear (tattr_val);

							g_free (attr_val);
							attr_val = NULL;
						}
						
						else if (g_strcasecmp(*attrs, "CursorName") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);	

							mag_zoomers_set_cursor (tattr_val, NULL, NULL);

							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "CursorSize") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);	

							mag_zoomers_set_cursor (NULL, tattr_val, NULL);

							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "CursorZoomFactor") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);	

							mag_zoomers_set_cursor (NULL, NULL, tattr_val);

							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "CursorScale") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);	

							mag_zoomers_set_cursor_scale_on_off (tattr_val);

							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "CursorColor") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);	

							mag_zoomers_set_cursor_color (tattr_val);

							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "cursor") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);	

							mag_zoomers_set_cursor_on_off (tattr_val);

							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "crosswire") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomers_set_crosswire_on_off (tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "CrosswireSize") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomers_set_crosswire_size (tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "CrosswireColor") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomers_set_crosswire_color (tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "CrosswireClip") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomers_set_crosswire_clip (tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						

						else

						{						
							/* unsupported attribute */
#ifdef MAG_XML_DEBUG
							fprintf (stderr, "\nThe MAGOUT attribute \"%s\" is not supported", *attrs);
#endif								
							++attrs;						
						}												
						++attrs;						
					}
				}
				if (!clear) current_state = MPS_OUT;
			}
			break;
			
		case MPS_OUT:
			if (g_strcasecmp (name, "ZOOMER") == 0)
			{					
				/* create a new MAG_ZOOMER object*/
				current_mag_zoomer = mag_zoomer_new();				
				/* MAG_ZOOMER ATTRIBUTES*/
				if (attrs)
				{	
					while (*attrs)
					{
					/* parse the mag zoomer attributes */
					/*
					<ZOOMER ID="magnifier1"
					visible="true"
				    	ROILeft="0"
					ROITop="100"
					ROIWidth="10"
				    	ROIHeight="50"
					ZPLeft="0"
					ZPTop="0"
					ZPWidth="300"
					ZPHeight="300"
					ZoomFactor="3.0"
					smoothing="bilinear"
					invert="false"> 
					</ZOOMER>
					*/
						if (g_strcasecmp (*attrs, "ID") == 0)
						{
							++attrs;				
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val); /* trim the attr value*/

							mag_zoomer_set_id (current_mag_zoomer, tattr_val);							
							found = mag_zoomer_find (&current_mag_zoomer);

							g_free (attr_val);
							attr_val = NULL;
		
						}
						
						else if (g_strcasecmp(*attrs, "visible") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);	

							mag_zoomer_set_params_visible (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						
						else if (g_strcasecmp(*attrs, "ROILeft") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ROI_left (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "ROITop") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ROI_top (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}
						else if (g_strcasecmp(*attrs, "ROIWidth") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ROI_width (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}
						else if (g_strcasecmp(*attrs, "ROIHeight") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ROI_height (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "ZPLeft") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ZP_extents_left (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "ZPTop") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ZP_extents_top (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "ZPWidth") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ZP_extents_width (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "ZPHeight") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ZP_extents_height (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "BorderWidth") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ZP_border_width (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						else if (g_strcasecmp(*attrs, "BorderColor") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_ZP_border_color (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;						
						}						
						
						else if (g_strcasecmp(*attrs, "ZoomFactorX") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_zoom_factor (current_mag_zoomer, tattr_val, NULL);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						
						else if (g_strcasecmp(*attrs, "ZoomFactorY") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_zoom_factor (current_mag_zoomer, NULL, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						

						else if (g_strcasecmp (*attrs, "smoothing") == 0)
						{
							++attrs;
							
							attr_val  = g_strdup (*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_smoothing_alg (current_mag_zoomer, tattr_val);
														
							g_free (attr_val);
							attr_val = NULL;												
						}						
						
						else if (g_strcasecmp(*attrs, "invert") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_invert (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "contrast") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_contrast (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "AlignmentX") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_alignment (current_mag_zoomer, tattr_val, NULL);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "AlignmentY") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_alignment (current_mag_zoomer, NULL, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "tracking") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_tracking (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "MouseTracking") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_params_mouse_tracking (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						
						else if (g_strcasecmp(*attrs, "source") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_source (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}
						else if (g_strcasecmp(*attrs, "target") == 0)
						{
							++attrs;

							attr_val  = g_strdup(*attrs);
							tattr_val = g_strstrip (attr_val);

							mag_zoomer_set_target (current_mag_zoomer, tattr_val);
							
							g_free (attr_val);
							attr_val = NULL;												
						}						

						/*more to come*/

						else
						{						
							/* unsupported attribute */
#ifdef MAG_XML_DEBUG
							fprintf (stderr, "\nThe ZOOMER attribute \"%s\" is not supported", *attrs);
#endif								
							++attrs;						
						}						
						
						++attrs;
						
					}
					
				}

				current_state = MPS_ZOOMER;
			}
			
		break;
						
		case MPS_ZOOMER:break;				
		
		case MPS_UNKNOWN:
			previous_state = current_state;
			++unknown_depth;
		break;
	}		
}

void 
mag_endElement (void          *ctx,
		const xmlChar *name)
{
#ifdef MAG_XML_DEBUG
	fprintf (stderr, "\nMAG: endElement  : %s %d", name, current_state);
#endif		
	switch (current_state)
	{
		
		case MPS_IDLE:break;
		case MPS_OUT:	
			if (g_strcasecmp(name, "MAGOUT") == 0)
			{
			    current_state = MPS_IDLE;
			}
		break;
		
		case MPS_ZOOMER: 
			if (g_strcasecmp(name, "ZOOMER") == 0)
			{
				/* give the MAG_ZOOMER to mag */
				mag_add_zoomer (current_mag_zoomer);
				
				/* free the mag zoomer */
				if (!found )
				{
				     mag_zoomer_free (current_mag_zoomer);
				     current_mag_zoomer = NULL;
				}
				
				current_state = MPS_OUT;
			}
		break;				

		case MPS_UNKNOWN:
			--unknown_depth;
			if (unknown_depth <= 0)
			{
				current_state = previous_state;
			}
		break;
	}
}

void 
mag_characters (void          *ctx,
		const xmlChar *ch, 
		int            len)
{
	gchar*		tch;
	/* make out own copy */
	tch = g_strndup(ch, len);

	switch (current_state)
	{
		
		case MPS_IDLE   : break;
		case MPS_OUT    : break;		
		case MPS_ZOOMER : break;
		case MPS_UNKNOWN: break;
	}
	g_free (tch);	
}

void 
mag_warning (void       *ctx, 
	     const char *msg,
	     ...)
{
	va_list args;

	va_start (args, msg);
	fprintf (stderr,"%s %d : warning",__FILE__,__LINE__);
	g_logv ("MAGXML", 
		 G_LOG_LEVEL_WARNING, 
		 msg, 
		 args);
	va_end (args);
	/* !!! TBI !!! current_state = MPS_IDLE */
}

void 
mag_error (void       *ctx, 
	   const char *msg, 
	   ...)
{
	va_list args;

	va_start (args, msg);
	g_logv ("MAGXML", 
		 G_LOG_LEVEL_CRITICAL, 
		 msg, 
		 args);
	va_end (args);
	/* !!! TBI !!! current_state = MPS_IDLE */	
}

void 
mag_fatalError (void       *ctx, 
		const char *msg, 
		...)
{
	va_list args;

	va_start (args, msg);
	g_logv ("MAGXML", 
		 G_LOG_LEVEL_ERROR, 
		 msg, 
		 args);
	va_end (args);
	/* !!! TBI !!! current_state = MPS_IDLE */	
}


/* __ MAGXML API ______________________________________________________________*/

int 
mag_xml_init (MagEventCB mag_event_cb)
{
	xmlInitParser();

	/* initialize SAX */
	mag_ctx = g_malloc0 (sizeof (xmlSAXHandler) );
		
	mag_ctx->startDocument = mag_startDocument;
	mag_ctx->endDocument   = mag_endDocument  ;
	mag_ctx->startElement  = mag_startElement ;
	mag_ctx->endElement    = mag_endElement   ;
	mag_ctx->characters    = mag_characters   ;

	mag_ctx->warning       = mag_warning   ;
	mag_ctx->error         = mag_error     ;
	mag_ctx->fatalError    = mag_fatalError;

	if (!mag_initialize (mag_event_cb) ) return 0;

	return 1;
}

void
mag_xml_terminate ()
{
	if (mag_ctx) 
	{
	    g_free (mag_ctx);
	    mag_ctx = NULL;
	}
	mag_terminate();
}

void 
mag_xml_output (char *buffer,
		int   len)
{
	xmlSAXParseMemory (mag_ctx, 
			   buffer, 
			   len, 
			   0); 
}

