/* brlmonxml.c
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

#include "brlmonxml.h"
#include "brlmonxmlapi.h"
#include "SRMessages.h"

static xmlSAXHandler  *mon_ctx;

static MON_PARSER_STATE mon_curr_state = MON_IDLE;
static MON_PARSER_STATE mon_prev_state = MON_IDLE;
static gint mon_unknown_depth = 0;

DisplayRole display_role = ROLE_OTHER;


void 
brlmon_startDocument (void *ctx)
{
	/*fprintf (stderr, "BRL: startDocument\n"); */
}

void 
brlmon_endDocument (void *ctx)
{
	/*fprintf (stderr, "BRL: endDocument\n"); */
}

void 
brlmon_startElement (void *ctx, const xmlChar* name, const xmlChar ** attrs)
{	

	gchar* attr_val;
	gchar* tattr_val;
		
	/*fprintf (stderr, "BRL: startElement: %s\n", name);*/
	
	switch (mon_curr_state)
	{
		case MON_IDLE:
		    {								
			if (g_strcasecmp((gchar*)name, "BRLOUT") == 0)
			{	
				/* BRLOUT ATTRIBUTES */
				if (attrs)
				{
					while (*attrs)
					{
						/*fprintf (stderr, "attr_val: %s\n", *attrs); */
											
						if (g_strcasecmp((gchar*)*attrs, "language") == 0)
						{
							++attrs;						
						}
						else if (g_strcasecmp((gchar*)*attrs, "bstyle") == 0)
						{
							++attrs;
						}					
						else if (g_strcasecmp((gchar*)*attrs, "clear") == 0)
						{
							++attrs;
						}
						else
						{						
							/* unsupported attribute */
							sru_warning (_("BRLOUT attribute ""%s"" is not supported."), *attrs);
							++attrs;						
						}
						++attrs;					
					}
				}								
				mon_curr_state = MON_BRL_OUT;
			}
		    }	
			
		break;
		
		case MON_BRL_OUT:
			
			if (g_strcasecmp((gchar*)name, "BRLDISP") == 0)
			{								
				/* BRLDISP ATTRIBUTES */
				display_role = ROLE_OTHER;
				if (attrs)
				{				
					while (*attrs)
					{
						/*fprintf (stderr, "attr_val: %s\n", *attrs); */
						if (g_strcasecmp((gchar*)*attrs, "id") == 0)
						{
							++attrs;						
						}
						else if (g_strcasecmp((gchar*)*attrs, "role") == 0)
						{
							++attrs;
														
							attr_val = g_strdup((gchar*)*attrs);
							tattr_val = g_strstrip (attr_val);
							
							if (strcmp(tattr_val,"main") == 0) 	display_role = ROLE_MAIN;
							    else
							if (strcmp(tattr_val,"status") == 0) 	display_role = ROLE_STATUS;
							    else				display_role = ROLE_OTHER;
							
							g_free (attr_val);
							attr_val = NULL;
						}
						else if (g_strcasecmp((gchar*)*attrs, "dno") == 0)
						{
							++attrs;
						}					
						else if (g_strcasecmp((gchar*)*attrs, "clear") == 0)
						{
							++attrs;
						}
						else if (g_strcasecmp((gchar*)*attrs, "start") == 0)
						{
							++attrs;
						}
						else if (g_strcasecmp((gchar*)*attrs, "offset") == 0)
						{
							++attrs;
						}
						else if (g_strcasecmp((gchar*)*attrs, "cstyle") == 0)
						{
							++attrs;
						}
						else if (g_strcasecmp((gchar*)*attrs, "cursor") == 0)
						{
							
							
							++attrs;
							
							attr_val = g_strdup((gchar*)*attrs);
							tattr_val = g_strstrip (attr_val);
							
							brlmon_cursor_pos(atoi(tattr_val));
							
							g_free (attr_val);
							attr_val = NULL;

						}
						else
						{						
							/* unsupported attribute */
							fprintf (stderr, "BRLDISP attribute ""%s"" is not supported\n", *attrs);
							++attrs;						
						}
						++attrs;
					
					}
				}
				mon_curr_state = MON_BRL_DISP;
			}			
			
		break;
		
		case MON_BRL_DISP:
		
			if (g_strcasecmp((gchar*)name, "DOTS") == 0)
			{
				mon_curr_state = MON_DOTS;
			}
			
			if (g_strcasecmp((gchar*)name, "TEXT") == 0)
			{
				/* TEXT ATTRIBUTES */
				brlmon_set_typedot(DOTNONE);
				if (attrs)
				{
					while (*attrs)
					{
						/*fprintf (stderr, "attr_val: %s\n", *attrs); */
						if (g_strcasecmp((gchar*)*attrs, "language") == 0)
						{
							++attrs;						
						}
						else if (g_strcasecmp((gchar*)*attrs, "attr") == 0)
						{
						
							++attrs;
							
							attr_val = g_strdup((gchar*)*attrs);
							tattr_val = g_strstrip (attr_val);
							
							if (strcmp(tattr_val,"dot78") == 0) brlmon_set_typedot(DOT78);
							    else
							if (strcmp(tattr_val,"dot7") == 0)  brlmon_set_typedot(DOT7);
							    else
							if (strcmp(tattr_val,"dot8") == 0)  brlmon_set_typedot(DOT8);
							    else
											    brlmon_set_typedot(DOTNONE);
							
							g_free (attr_val);
						
						}
						else
						{						
							/* unsupported attribute */
							sru_warning (_("TEXT attribute ""%s"" is not supported."), *attrs);
							++attrs;						
						}
						++attrs;					
					}
				}	
				mon_curr_state = MON_TEXT;
			}

		break;
		
		case MON_BRL_SET:break;
		case MON_DOTS: break;
		case MON_TEXT: break;
		case MON_UNKNOWN:
			mon_prev_state = mon_curr_state;
			++mon_unknown_depth;
		break;
	}
		
}

void 
brlmon_endElement (void *ctx, const xmlChar* name)
{
	/*fprintf (stderr, "BRL: endElement: %s\n", name); */
	
	switch (mon_curr_state)
	{
		
		case MON_IDLE:			
		break;
		
		case MON_BRL_OUT:
		
			if (g_strcasecmp((gchar*)name, "BRLOUT") == 0)
			{
				mon_curr_state = MON_IDLE;
			}
						
		break;
		
		
		case MON_BRL_DISP:
		
			if (g_strcasecmp((gchar*)name, "BRLDISP") == 0)
			{
				mon_curr_state = MON_BRL_OUT;
			}			
			
		break;
		
		case MON_DOTS:
			if (g_strcasecmp((gchar*)name, "DOTS") == 0)
			{
				mon_curr_state = MON_BRL_DISP;
			}
		break;
		
		case MON_TEXT:
			if (g_strcasecmp((gchar*)name, "TEXT") == 0)
			{
				mon_curr_state = MON_BRL_DISP;
			}
		break;
		
		case MON_UNKNOWN:
			--mon_unknown_depth;
			if (mon_unknown_depth <= 0)
			{
				mon_curr_state = mon_prev_state;
			}
		break;
		case MON_BRL_SET:break;
	}

}

void 
brlmon_characters (void *ctx, const xmlChar *ch, gint len)
{	
    gchar	*tch;
		
    tch = g_strndup((gchar*)ch, len);
		
/*    fprintf(stderr,"TEXT:%s:end\n",tch); */

	switch (mon_curr_state)
	{
		case MON_IDLE: break;
					
		case MON_DOTS:
		{
		    gchar txt[2];
		    txt[0] = '|';
		    txt[1] = '\0';
		    brlmon_print_text_from_cur_pos(txt, display_role);
		}
		break;
		
		case MON_TEXT:
		{
		    brlmon_print_text_from_cur_pos(tch, display_role);
		}	
		break;
		
		case MON_BRL_OUT: break;
		case MON_BRL_DISP: break;
		case MON_UNKNOWN: break;		
		case MON_BRL_SET:break;
	}

    g_free(tch);
    tch = NULL;
}

void 
brlmon_warning (void *ctx, const gchar *msg, ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("BRL XML", G_LOG_LEVEL_WARNING, msg, args);
    va_end (args);
}

void 
brlmon_error (void *ctx, const gchar *msg, ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("BRL XML", G_LOG_LEVEL_CRITICAL, msg, args);
    va_end (args);
}

void 
brlmon_fatalError (void *ctx, const gchar *msg, ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("BRL XML", G_LOG_LEVEL_ERROR, msg, args);
    va_end (args);
}


void 
brlmon_xml_output (gchar* buffer, gint len)
{
    xmlSAXParseMemory (mon_ctx, buffer, len, 0); 
}


gint 
brlmon_xml_init ()
{
    xmlInitParser ();	

    mon_ctx = g_new0 (xmlSAXHandler, 1);
    
    mon_ctx->startDocument 	= brlmon_startDocument;
    mon_ctx->endDocument 	= brlmon_endDocument;
    mon_ctx->startElement 	= brlmon_startElement;
    mon_ctx->endElement 	= brlmon_endElement;
    mon_ctx->characters 	= brlmon_characters;

    mon_ctx->warning 		= brlmon_warning;
    mon_ctx->error 		= brlmon_error;
    mon_ctx->fatalError 	= brlmon_fatalError;
    
    return 1;	
}

void 
brlmon_xml_terminate ()
{
    if (mon_ctx) 
    {
	g_free(mon_ctx);	
	mon_ctx = NULL;
    }
}
