/* brlxml.c
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
#include "braille.h"
#include "brlxml.h"
#include "brlxmlapi.h"

static xmlSAXHandler  *brl_ctx;

static BPSParserState brl_curr_state = BPS_IDLE;
static BPSParserState brl_prev_state = BPS_IDLE;
static gint brl_unknown_depth = 0;

gint old_offset = 0;

static BRLDisp	*tbrl_disp = NULL;
static BRLOut	*tbrl_out = NULL;

static GHashTable* translation_table_cache;
static BRLXmlInputProc xml_client_callback = NULL;

static gboolean brl_xml_initialized = FALSE;


/* __ TRANSLATION TABLE CACHE _________________________________________________ */

void 
ttc_key_destroy (gpointer key)
{
    g_free (key);
}

void
ttc_value_destroy (gpointer value)
{
    g_free (value);
}

void 
ttc_init()
{
    translation_table_cache = g_hash_table_new (g_str_hash, g_str_equal);
}

void
ttc_enum (gpointer key,
	  gpointer value, 
	  gpointer user_data)
{
    g_free (key);
    g_free (value);
}

void
ttc_terminate()
{
    g_hash_table_foreach (translation_table_cache, ttc_enum, NULL);
    g_hash_table_destroy (translation_table_cache);
}

guint8*
ttc_get_translation_table (const gchar* language)
{
	
    gchar*	key 	= NULL;
    guint8*	value  	= NULL;
	
    gchar*	fn	= NULL;
    gchar*	dir_fn 	= NULL;
    FILE* 	fp	= NULL;
	
    /* search the key in the table first */
    value = (guint8*) g_hash_table_lookup (translation_table_cache, language);
		
    if (!value)
    {
	/* the table is not in the cache, try to load it from the TT file */
			
	/* derive the TT file name from the language name */
	fn = g_strdup_printf ("%s.a2b", language);
	/* open the file */
        if (g_file_test (fn, G_FILE_TEST_EXISTS)) 
	{
    	    fp = fopen (fn, "rb");
	}
	else
	{
	    dir_fn = g_strconcat (BRAILLE_TRANS_TABLES_DIR,fn,NULL);
	    fp = fopen (dir_fn,"rb");
	    g_free (dir_fn);
	    dir_fn = NULL;
	}
	if (fp)
	{	
	    /* load translation table in memory */
	    key = g_strdup (language);
	    value = g_malloc0 (TT_SIZE);			
	    fread (value, 1, TT_SIZE, fp);	
			
	    g_hash_table_insert (translation_table_cache, key, value);
			
	    fclose (fp);	
	}	
	else
	{
	    fprintf(stderr,"brlxml : opening file error\n");
	}
	g_free (fn);
    }
	
    return value;
}

/* __ HELPERS _________________________________________________________________ */

guint8
dotstr_to_bits (gchar* dotstr)
{
    guint8 rv = 0;
    gint i, n, dot;
	
    /* DOT  ?	      1	     2        3      4        5       6       7       8  */
    guint8 dot_to_bit [] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	
    n = strlen (dotstr);	
    if (n > 3 && g_strncasecmp(dotstr, "dot", 3) == 0)
    {
	/* DOTxxx format */
	for (i = 3; i < n; ++i)
	{
	    dot = dotstr[i] - '0';
	    if (dot > 0 && dot <= 8)
	    {
		 rv |= dot_to_bit[dot];
	    }
	}						
    }
    else
    {
	/* assume hex format */
	sscanf (dotstr, "%2x", &i);
	rv = i & 0xFF;
    }
	
    return rv;
}

/* __ BrlDisp METHODS _________________________________________________________ */

BRLDisp*
brl_disp_new ()
{
	
    BRLDisp *brl_disp;
	
    brl_disp = g_malloc0(sizeof(BRLDisp));
    brl_disp->dots = g_byte_array_new();
    brl_disp->cursor_position = -1;	/* no cursor */
    brl_disp->cursor_style = BRL_CS_UNDERLINE;
    brl_disp->cursor_mask = 0xC0;
    brl_disp->attribute = 0x00;
    brl_disp->role = NULL;
    brl_disp->translation_table = NULL;

    return brl_disp;
}

void
brl_disp_free (BRLDisp *brl_disp)
{
    g_free (brl_disp->role);
    g_byte_array_free (brl_disp->dots, TRUE);
    g_free (brl_disp);
}

BRLDisp*
brl_disp_copy (BRLDisp *brl_disp)
{
    BRLDisp* bd;
		
    bd = g_malloc0(sizeof(BRLDisp));
	
    /* copy all non pointer fields */
    memcpy (bd, brl_disp, sizeof(BRLDisp));
	
    /* copy the role string */
    if (brl_disp->role)
    {
	bd->role = g_strdup(brl_disp->role);
    }

    /* copy the dots array */
    bd->dots = g_byte_array_new();
    g_byte_array_append (bd->dots, brl_disp->dots->data, brl_disp->dots->len);
	
    return bd;
}

void
brl_disp_set_id (BRLDisp *brl_disp, gchar *id)
{
    brl_disp->id = atoi(id);
}

void 
brl_disp_set_role (BRLDisp *brl_disp,
		   gchar   *role)
{
    brl_disp->role = g_strdup (role);
}

void
brl_disp_set_disp_no (BRLDisp *brl_disp, gchar *dno)
{
	brl_disp->id = atoi(dno);	/* !!! TBR !!! don't store disp no in the ID... */
}

void
brl_disp_set_clear_mode (BRLDisp *brl_disp,
                         gchar   *clear)
{
    if ((g_strcasecmp (clear, "true") == 0) ||
	(g_strcasecmp (clear, "on") == 0) ||
	(g_strcasecmp(clear, "1") == 0))
    {
	brl_disp->clear_display = TRUE;
    }
    else
    {
	brl_disp->clear_display = FALSE;
    }
}

void
brl_disp_set_start (BRLDisp *brl_disp,
		    gchar   *start)
{
    brl_disp->start = atoi(start);
}

void 
brl_disp_set_offset (BRLDisp *brl_disp,
		     gchar   *offset)
{
    brl_disp->offset = atoi(offset);
}

void
brl_disp_set_cursor_style (BRLDisp *brl_disp, 
			   gchar* style)
{			
    if (g_strcasecmp(style, "underline") == 0)
    {
	brl_disp->cursor_style = BRL_CS_UNDERLINE;			
	brl_disp->cursor_mask = 0xC0;
    	brl_disp->cursor_pattern = 0xC0;
    }
    else if (g_strcasecmp(style, "block") == 0)
    {
	brl_disp->cursor_style = BRL_CS_BLOCK;			
	brl_disp->cursor_mask = 0xFF;
	brl_disp->cursor_pattern = 0xFF;
    }
    else if (g_strcasecmp(style, "user") == 0)
    {
	brl_disp->cursor_style = BRL_CS_USER_DEFINED;			
    }
    else
    {
	/* for unknown attr value, we use the default */
	brl_disp->cursor_style = BRL_CS_UNDERLINE;			
	brl_disp->cursor_mask = 0xC0;
	brl_disp->cursor_pattern = 0xC0;
    }
}

void
brl_disp_set_cursor (BRLDisp *brl_disp,
		     gchar   *cursor)
{
    brl_disp->cursor_position = atoi(cursor);
}

void 
brl_disp_set_text_attr (BRLDisp *brl_disp,
			gchar   *attr)
{	
    brl_disp->attribute = dotstr_to_bits(attr);	
}

void
brl_disp_load_translation_table (BRLDisp *brl_disp,
				 gchar*  language)
{
	
    brl_disp->translation_table = ttc_get_translation_table (language);
}


void 
brl_disp_add_dots (BRLDisp *brl_disp, 
		   guint8*  dots, 
		   gint     len)
{
    g_byte_array_append (brl_disp->dots, dots, len);
}

/* __ BrlOut  METHODS _________________________________________________________ */

BRLOut* 
brl_out_new ()
{
    BRLOut* brl_out;
    brl_out = g_malloc0 (sizeof(BRLOut));
    brl_out->displays = g_array_new (FALSE, FALSE, sizeof (BRLDisp*));
    return brl_out;
}

void 
brl_out_free (BRLOut *brl_out)
{
    gint i;
    BRLDisp*	brl_disp;

    for (i = 0; i < brl_out->displays->len ; ++i)
    {
	brl_disp = g_array_index (brl_out->displays, BRLDisp*,i);
	brl_disp_free(brl_disp);
    }
	
    g_array_free (brl_out->displays, TRUE);
    g_free (brl_out);
}

void
brl_out_add_display (BRLOut  *brl_out,
		     BRLDisp *brl_disp)
{
    BRLDisp * bd_copy;
		
    /* make a copy of the brl_disp (incl. role and dots) */
    bd_copy = brl_disp_copy (brl_disp);
		
    g_array_append_vals (brl_out->displays, &bd_copy, 1);	
}
	
void
brl_out_load_translation_table (BRLOut *brl_out,
				gchar  *language)
{
    brl_out->translation_table = ttc_get_translation_table (language);
}

void 
brl_out_set_brl_style (BRLOut *brl_out,
		       gchar  *style)
{			
    switch (atoi(style))
    {
	case 0:
	case 8:
	    brl_out->braille_style = BRL_8_DOTS;
	break;
				
	case 1:
	case 6:
	    brl_out->braille_style = BRL_6_DOTS;
	break;
				
	default:
	    brl_out->braille_style = BRL_8_DOTS;
	break;
    }
}

void 
brl_out_set_clear_mode (BRLOut *brl_out,
			gchar  *clear)
{			
    if ((g_strcasecmp(clear, "true") == 0) ||
	(g_strcasecmp(clear, "on") == 0) ||
	(g_strcasecmp(clear, "1") == 0))
    {
	brl_out->clear_all_cells = TRUE;
    }
    else
    {
	brl_out->clear_all_cells = FALSE;
    }	
}

void 
brl_out_to_driver (BRLOut *brl_out)
{	
    gint    i, n;
    BRLDisp *brl_disp;
    gshort  did = 0;	
    	
    if (brl_out->clear_all_cells)
    {
	brl_clear_all();
    }
	
    for (i = 0; i < brl_out->displays->len; ++i)
    {
	/* get the BrlDisp */
	brl_disp = g_array_index (brl_out->displays, BRLDisp*, i);
						
	/* find the BRL_ID  from the Role and the No  */
	did = brl_get_disp_id (brl_disp->role, brl_disp->id);
	if (did >= 0 )
	{
    	    if (brl_disp->clear_display)
	    {
		brl_clear_display (did);
	    }
		
	    /* consider the cursor */
	    /* fprintf (stderr, "CP:%d, M:%02X, P:%02X\n", brl_disp->cursor_position, brl_disp->cursor_mask, brl_disp->cursor_pattern); */
			
	    if (brl_disp->cursor_position >= 0 &&
		brl_disp->cursor_position < 1024) /* to avoid ill values eating too much mem ... */
	    {
		/* the display has a potentially visible cursor */
		if (brl_disp->cursor_position >= brl_disp->dots->len )
		{
		    /* cursor outside the current dots, pad with spaces */
		    n = brl_disp->cursor_position - brl_disp->dots->len + 1;					
		    brl_disp_add_dots (brl_disp, g_malloc0(n), n);
		}
				
		/* now we are sure that the cursor is in range */
		/* we merge it to the existing dots, according to it's mask and patern */
		brl_disp->dots->data[brl_disp->cursor_position] &= ~brl_disp->cursor_mask;	
		brl_disp->dots->data[brl_disp->cursor_position] |= brl_disp->cursor_pattern & brl_disp->cursor_mask;
	    }
			
	    /* do the out */			
 	    /* fprintf(stderr, "BRL: set dots: did %d, start %d, len %d, off %d\n",
			did, brl_disp->Start, brl_disp->Dots->len, brl_disp->Offset); */
			
	    brl_set_dots (did, brl_disp->start, &brl_disp->dots->data[0], brl_disp->dots->len, brl_disp->offset, brl_disp->cursor_position);	
	}	
    }
	
    brl_update_dots(1);	/* update all dots now (change to 0 for non-blocking update) */	
}

/* __ SAX CALLBACKS ___________________________________________________________ */

void 
brl_start_document (void *ctx)
{
    /* fprintf (stderr, "BRL: startDocument\n"); */
}

void
brl_end_document (void *ctx)
{
    /* fprintf (stderr, "BRL: endDocument\n"); */
}

void
brl_start_element (void            *ctx, 
                  const xmlChar*   name, 
		  const xmlChar ** attrs)
{	
    gchar* attr_val;
    gchar* tattr_val;
		
    /*fprintf (stderr, "BRL: startElement: %s\n", name); */
	
    switch (brl_curr_state)
    {
	case BPS_IDLE:
	    if (g_strcasecmp ((gchar*)name, "BRLOUT") == 0)
	    {
		/* create a new BrlOut object */
		tbrl_out = brl_out_new();				
				
		/* BRLOUT ATTRIBUTES */
		if (attrs)
		{
		    while (*attrs)
			{
			    /* fprintf (stderr, "attr_val: %s\n", *attrs); */
											
			    if (g_strcasecmp ((gchar*)*attrs, "language") == 0)
			    {
				++attrs;						
							
				attr_val = g_strdup ((gchar*)*attrs);
				tattr_val = g_strstrip (attr_val);

				brl_out_load_translation_table (tbrl_out, tattr_val);						
							
				g_free (attr_val);
							
			    }
			    else if (g_strcasecmp ((gchar*)*attrs, "bstyle") == 0)
			    {
				++attrs;
							
				attr_val = g_strdup ((gchar*)*attrs);
				tattr_val = g_strstrip (attr_val);
							
				brl_out_set_brl_style (tbrl_out, tattr_val);
							
				g_free (attr_val);
			    }					
			    else if (g_strcasecmp ((gchar*)*attrs, "clear") == 0)
			    {
				++attrs;
							
				attr_val = g_strdup ((gchar*)*attrs);
				tattr_val = g_strstrip (attr_val);
							
				brl_out_set_clear_mode (tbrl_out, tattr_val);
							
				g_free (attr_val);
			    }
			    else
			    {						
				/* unsupported attribute */
				fprintf (stderr, "Attribute ""%s"" is not supported in the BRLOUT tag.\n", *attrs);
				++attrs;						
			    }
			    ++attrs;					
			}
		    }
								
		    brl_curr_state = BPS_BRL_OUT;
		}	
	break;
		
	case BPS_BRL_OUT:		
	    if (g_strcasecmp ((gchar*)name, "BRLDISP") == 0)
	    {
		/* create a new BrlDisp object */
		tbrl_disp = brl_disp_new();
								
		/* BRLDISP ATTRIBUTES */
		if (attrs)
		{				
		    while (*attrs)
		    {
			/* fprintf (stderr, "attr_val: %s\n", *attrs); */
			if (g_strcasecmp ((gchar*)*attrs, "id") == 0)
			{
			    ++attrs;						
			    				
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);

			    brl_disp_set_id (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}
			else if (g_strcasecmp ((gchar*)*attrs, "role") == 0)
			{
			    ++attrs;
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);

			    brl_disp_set_role (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}
			else if (g_strcasecmp ((gchar*)*attrs, "dno") == 0)
			{
			    ++attrs;
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);

			    brl_disp_set_disp_no (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}					
			else if (g_strcasecmp ((gchar*)*attrs, "clear") == 0)
			{
			    ++attrs;
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);

			    brl_disp_set_clear_mode (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}
			else if (g_strcasecmp ((gchar*)*attrs, "start") == 0)
			{
			    ++attrs;
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);
							
			    brl_disp_set_start (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}
			else if (g_strcasecmp ((gchar*)*attrs, "offset") == 0)
			{
			    	++attrs;
							
				attr_val = g_strdup ((gchar*)*attrs);
				tattr_val = g_strstrip (attr_val);
							
				brl_disp_set_offset (tbrl_disp, tattr_val);
							
				g_free (attr_val);
			}
			else if (g_strcasecmp ((gchar*)*attrs, "cstyle") == 0)
			{
			    ++attrs;
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);

			    brl_disp_set_cursor_style (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}
			else if (g_strcasecmp ((gchar*)*attrs, "cursor") == 0)
			{
			    ++attrs;
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);

			    brl_disp_set_cursor (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}
			else
			{						
			    /* unsupported attribute */
			    fprintf (stderr, "Attribute ""%s"" is not supported in the BRLDISP tag.\n", *attrs);
			    ++attrs;						
			}
		    	++attrs;
		    }
		}
		brl_curr_state = BPS_BRL_DISP;
	    }					
	break;
		
	case BPS_BRL_DISP:	
	    if (g_strcasecmp ((gchar*)name, "DOTS") == 0)
	    {
		brl_curr_state = BPS_DOTS;
	    }
	    if (g_strcasecmp ((gchar*)name, "TEXT") == 0)
	    {							
		/* reset display attributes */
		tbrl_disp->attribute = 0;
				
		/* reset language to default from BrlOut */
		tbrl_disp->translation_table = tbrl_out->translation_table;
				
		/* TEXT ATTRIBUTES */
		if (attrs)
		{
		    while (*attrs)
		    {
			/* fprintf (stderr, "attr_val: %s\n", *attrs); */
			if (g_strcasecmp ((gchar*)*attrs, "language") == 0)
			{
			    ++attrs;						
			    				
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);
							
			    brl_disp_load_translation_table (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}
			else if (g_strcasecmp ((gchar*)*attrs, "attr") == 0)
			{
			    ++attrs;
							
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strstrip (attr_val);

			    brl_disp_set_text_attr (tbrl_disp, tattr_val);
							
			    g_free (attr_val);
			}
			else
			{						
			    /* unsupported attribute */
			    fprintf (stderr, "Attribute ""%s"" is not supported in the TEXT tag.\n", *attrs);
			    ++attrs;						
			}
			++attrs;					
		    }
		}	
		brl_curr_state = BPS_TEXT;
	    }
	    /* SCROLL tag allows to scroll with display left/right*/
	    if (g_strcasecmp ((gchar*)name, "SCROLL") == 0)
	    {
		/* SCROLL attributes*/
		if (attrs)
		{
		    while (*attrs)
		    {
			/* fprintf (stderr, "\n attr_val; %s", *attrs); */
			if (g_strcasecmp ((gchar*)*attrs, "mode") == 0)
			{
			    ++attrs;
			    gint16 new_offset = 0;
			    gshort sign = 1;
					
			    attr_val = g_strdup ((gchar*)*attrs);
			    tattr_val = g_strdup (attr_val);
			    		
			    /* scroll left or right? */
			    if (tattr_val[0] == '-')
			    {
				tattr_val++;
				sign = -1;
			    }
					
			    if (g_ascii_isdigit (*tattr_val)) /* no of cells*/
			    {
				new_offset = tbrl_disp->offset + atoi (tattr_val)  *sign;    
			    }
			    else                             /*display width */
			    {
				gshort display_width = brl_get_display_width (tbrl_disp->id);
				if (display_width >= 0)
				{
				    new_offset = tbrl_disp->offset + display_width * sign;
				}
			    }
					
			    if (new_offset < 0)
			    new_offset = 0;
			    		
			    tbrl_disp->offset = new_offset;
			    old_offset = new_offset;
					
			    g_free (attr_val);
			}
			else
			{
			    /* unsupported attribute */
			    fprintf (stderr, "Attribute ""%s"" is not supported in the SCROLL tag.\n", *attrs);
			    ++attrs;
			}
			++attrs;
		    }
		}
		brl_curr_state = BPS_SCROLL;
	    }
	break;
		
	case BPS_DOTS: 
	case BPS_TEXT:
	case BPS_SCROLL:
	break;
		
	case BPS_UNKNOWN:
	    brl_prev_state = brl_curr_state;
	    ++brl_unknown_depth;
	break;
    }		
}

void 
brl_end_element (void           *ctx,
                const xmlChar* name)
{
    /* fprintf (stderr, "BRL: endElement: %s\n", name); */
	
    switch (brl_curr_state)
    {		
	case BPS_IDLE:			
	break;
		
	case BPS_BRL_OUT:	
	    if (g_strcasecmp ((gchar*)name, "BRLOUT") == 0)
	    {
		/* output packet done, make the driver calls */
		brl_out_to_driver (tbrl_out);
				
		/* free it */
		brl_out_free (tbrl_out);
		brl_curr_state = BPS_IDLE;
	    }					
	break;
			
	case BPS_BRL_DISP:	
	    if (g_strcasecmp ((gchar*)name, "BRLDISP") == 0)
	    {							
		/* finished with the current BrlDisp, add it to the BrlOut.Displays */
		brl_out_add_display (tbrl_out, tbrl_disp); 
		brl_disp_free (tbrl_disp);
		tbrl_disp = NULL;
								
		brl_curr_state = BPS_BRL_OUT;
	    }					
	break;
		
	case BPS_DOTS:
	    if (g_strcasecmp ((gchar*)name, "DOTS") == 0)
	    {
		brl_curr_state = BPS_BRL_DISP;
	    }
	break;
		
	case BPS_TEXT:
	    if (g_strcasecmp ((gchar*)name, "TEXT") == 0)
	    {
		brl_curr_state = BPS_BRL_DISP;
	    }
	break;

	case BPS_SCROLL:
	    if (g_strcasecmp ((gchar*)name, "SCROLL") == 0)
	    {
		brl_curr_state = BPS_BRL_DISP;
	    }
	break;	
		
	case BPS_UNKNOWN:
	    --brl_unknown_depth;
	    if (brl_unknown_depth <= 0)
	    {
		brl_curr_state = brl_prev_state;
	    }
	break;
    }
}

void 
brl_characters (void          *ctx,
                const xmlChar *ch, 
		gint          len)
{
    gint 	i;
    gchar*	tch;
    gchar**	tokens;
    guint8	tdots;
    guint8*	dotbuff;
		
    tch = g_strndup ((gchar*)ch, len);
    tch = g_strstrip(tch);
    switch (brl_curr_state)
    {
	case BPS_IDLE:
	break;
						
	case BPS_DOTS:	
	    /* add dots to tbrl_disp */
	    tokens = g_strsplit (tch, " ", 0);
	    for (i = 0; tokens[i] != NULL; ++i)
	    {								
		tdots = dotstr_to_bits(tokens[i]);				
		brl_disp_add_dots (tbrl_disp, &tdots, 1);
	    }
	    g_strfreev (tokens);	
	break;
		
	case BPS_TEXT:
	{
	    gchar  *str_utf = NULL;
	    gchar  *str_crt = NULL;
	    glong  str_len;
	    str_utf = g_strndup (ch, len);
	    str_len = g_utf8_strlen (str_utf, -1);
	    /* convert text to braille dots */
	    dotbuff = malloc (str_len);			
	    str_crt = str_utf;					
			
	    for (i = 0; i < str_len; ++i)
	    {
		/* translate char to dots */
		if (tbrl_disp->translation_table)
		{
		    if (TT_SIZE > g_utf8_get_char (str_crt))
		    {
			/* use the translation table */
			dotbuff[i] = tbrl_disp->translation_table[g_utf8_get_char (str_crt)];
		    }
		    else
		    {
			/* use the last simbol from translation table, 
			when the character code is grant then the 
			number of simbols in translation table*/
			dotbuff[i] = tbrl_disp->translation_table[TT_SIZE - 1];
		    }
		}
		else
		{
		    /* no table, no dots */
		    dotbuff[i] = 0;
		}
						/* consider 6/8 dot Braille style */
		if (tbrl_out->braille_style == BRL_6_DOTS)
		{
		    dotbuff[i] &= 0x3F;	/* clear dot 78 */
		}
				
		/* merge the Braille attributes				 */
		dotbuff[i] |= tbrl_disp->attribute;
		str_crt = g_utf8_find_next_char (str_crt, NULL);
	    }
							
	    brl_disp_add_dots (tbrl_disp, dotbuff, str_len );
			
	    free (dotbuff);
	    free (str_utf);	
	break;
	}
	case BPS_BRL_OUT:
	case BPS_BRL_DISP:
	case BPS_SCROLL:
	case BPS_UNKNOWN:
	break;	
    	}
	
    g_free (tch);	
}

void 
brl_warning (void        *ctx,
             const gchar *msg, ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("BRL XML", G_LOG_LEVEL_WARNING, msg, args);
    va_end (args);	
}

void
brl_error (void        *ctx,
	   const gchar *msg, ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("BRL XML", G_LOG_LEVEL_CRITICAL, msg, args);
    va_end (args);
}

void 
brl_fatal_error (void        *ctx,
		const gchar *msg, ...)
{
    va_list args;

    va_start (args, msg);
    g_logv ("BRL XML", G_LOG_LEVEL_ERROR, msg, args);
    va_end (args);
}


void
brl_xml_output (gchar *buffer,
		gint  len)
{
    if (brl_xml_initialized)
    {
	xmlSAXParseMemory (brl_ctx, buffer, len, 0); 
    }
    else
    {
	fprintf (stderr, "ERROR: brl_xml_output called before brl_xml_init.\n");
    }	
}

/* __ BRLXML API ______________________________________________________________ */

void 
brl_braille_events (BRLEventCode event_code, BRLEventData *event_data)
{
	
    gint  ix;
    gchar xml_out[1024];

    if (!xml_client_callback) 
	return;

    switch (event_code)
    {
	case BRL_EVCODE_RAW_BYTE:
	case BRL_EVCODE_KEY_BITS:	/* <KEYSTATE></KEYSTATE> ?		 */
	    return;
	break;
	case BRL_EVCODE_KEY_CODES:
	case BRL_EVCODE_SWITCH_PAD:
	case BRL_EVCODE_SENSOR:
	default:
	break;
    }
    	
    ix = 0;	
    ix += sprintf (&xml_out[ix], "<BRLIN>\n");	/* BRLIN entry tag */
    switch (event_code)
    {	
	case BRL_EVCODE_KEY_CODES:
	    ix += sprintf (&xml_out[ix], "<KEY>%s</KEY>\n", event_data->key_codes);			
	break;
			
	case BRL_EVCODE_SENSOR:
	    ix += sprintf (&xml_out[ix], "<SENSOR bank=\"%d\" display=\"%d\" technology=\"%d\">%s</SENSOR>\n",			
			   event_data->sensor.bank,
	                   event_data->sensor.associated_display,
			   event_data->sensor.technology,
			   event_data->sensor.sensor_codes);			
	break;
			
	case BRL_EVCODE_SWITCH_PAD:
	    ix += sprintf (&xml_out[ix], "<SWITCH>%s</SWITCH>\n",				
			   event_data->switch_pad.switch_codes);						
	break;
		
/*	case bec_sensor:
	    fprintf (stderr, "\nSENSOR: value:%d, bank:%d, disp:%d, tech:%d",
		     event_data->sensor.value,
		     event_data->sensor.bank,
		     event_data->sensor.associated_display,
		     event_data->sensor.technology);
	break;
*/

	default:
	    fprintf (stderr, "brlxml: unsupported input event\n");
	break;
    }
	
    ix += sprintf (&xml_out[ix], "</BRLIN>\n");	/* BRLIN exit tag */
    xml_client_callback (xml_out, ix);
}


gint
brl_xml_init (gchar           *device_name,
	      gint            port,
	      BRLXmlInputProc input_callback_proc)
{
    gint rv = 1;
	
    if (!brl_xml_initialized)
    {
	/* initialize the low level Braille stuff */
	
	brl_init();		
	if (!brl_open_device (device_name, port, brl_braille_events) )
	    return FALSE;
		
	xml_client_callback = input_callback_proc;
	xmlInitParser();
	
	/* initialize translation table cache */
	ttc_init();
	
	/* initialize the XML parser */
	brl_ctx = g_malloc0(sizeof(xmlSAXHandler));
	
	brl_ctx->startDocument = brl_start_document;
	brl_ctx->endDocument = brl_end_document;
	brl_ctx->startElement = brl_start_element;
	brl_ctx->endElement = brl_end_element;
	brl_ctx->characters = brl_characters;

	brl_ctx->warning =   brl_warning;
	brl_ctx->error = brl_error;
	brl_ctx->fatalError = brl_fatal_error;
	
	
	brl_xml_initialized = TRUE;
    }
    else
    {
	fprintf (stderr, "WARNING: brl_xml_init called more than once.\n");
    }

    return rv;	
}

void
brl_xml_terminate ()
{
    if (brl_xml_initialized )
    {
	if (brl_ctx) 
	    g_free(brl_ctx);
	
	ttc_terminate();
	brl_terminate();
	
	brl_xml_initialized = FALSE;
    }
    else
    {
	fprintf (stderr, "WARNING: brl_xml_terminate called more than once.\n");
    }	
}


