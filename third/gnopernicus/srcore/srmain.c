/* srmain.c
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

/* #define DEBUG_SRCORE  */
#include "config.h"
#include <stdio.h>
#include <gnome.h>
#include "srbrl.h"
#include "srspc.h"
#include "srmag.h"
#include "libsrconf.h"
#include "brlinp.h"
#include "libke.h"
#include "SREvent.h"
#include "srctrl.h"
#include "SRObject.h"
#include "SRLow.h"
#include "srmain.h"
#include "srpres.h"
#include "srintl.h"
#include "screen-review.h"
#include <libxml/parser.h>


static SRLClientHandle src_srl_client;
gboolean src_use_braille = FALSE;
gboolean src_use_magnifier = FALSE;
gboolean src_use_speech	= FALSE;
gboolean src_use_braille_monitor = FALSE;

extern gint src_speech_mode;
gboolean src_mouse_take;
gboolean src_mouse_click;
gboolean src_enable_format_string = FALSE; 
gint src_caret_position;

gchar *braille_device = NULL;
gint   braille_port;

extern gchar *scroll;
extern gint old_offset;

extern gboolean panning;


void
src_present_null_sro ()
{
    if (src_use_braille) 
	src_braille_show (_("No focus"));
	
    if (src_use_braille_monitor) 
	src_brlmon_show (_("No focus")); 

    if (src_use_speech)
    	src_speech_say_message (_("No accessible object is focused"));
}


static gchar *src_last_key_echo		= NULL;
static gchar *src_last_punct_echo 	= NULL;
static gchar *src_last_space_echo 	= NULL;
static gchar *src_last_modifier_echo 	= NULL;
static gchar *src_last_cursor_echo 	= NULL;

static gboolean src_process_key_  	(gchar *key);
static gboolean src_process_punct 	(gchar *punct);
static gboolean src_process_space 	(gchar *space);
static gboolean src_process_modifier 	(gchar *modifier);
static gboolean src_process_cursor 	(gchar *cursor);

static void
src_present_real_sro_for_brl_and_brlmon (SRObject *obj, gchar *reason)
{
    gchar *txt;

    srl_assert (src_use_braille || src_use_braille_monitor);

	src_caret_position = -1;
    src_enable_format_string = FALSE; 
    txt = src_presentation_sro_get_pres_for_device (obj, reason, "braille");
    if (txt)
    {
	gchar *tmp;
	gchar *cursor = NULL;
	SRObjectRoles role;
	gchar *status = NULL;
	gchar *braille_style = NULL;
	gchar *braille_cursor_style = NULL;
	gchar *braille_translation_table = NULL;
	gint  braille_offset;
	gchar offset[5];
	gchar *braille_scroll = NULL;

	braille_style = src_braille_get_style ();
	braille_cursor_style = src_braille_get_cursor_style ();
	braille_translation_table = src_braille_get_translation_table ();
	braille_offset = src_braille_get_offset();
	braille_scroll = g_strdup (scroll);
	
	sro_get_role (obj, &role, SR_INDEX_CONTAINER);
	if (role == SR_ROLE_TEXT_SL || role == SR_ROLE_TEXT_ML || role == SR_ROLE_TERMINAL || role == SR_ROLE_COMBO_BOX)
	{
	    SRLong offset;
	    SRTextAttribute *attr;
	    gchar status_[5] = "    ";

	    if (sro_text_get_caret_offset (obj, &offset, SR_INDEX_CONTAINER))
		{
			if (src_caret_position == -1)
				src_caret_position = 0;
		    cursor = g_strdup_printf (" cursor=\"%ld\" cstyle=\"%s\"", offset+src_caret_position, braille_cursor_style);
	    }
	    if (sro_text_get_text_attr_from_caret (obj, SR_TEXT_BOUNDARY_CHAR, 
				    &attr, SR_INDEX_CONTAINER))
	    {
		gchar *tmp;
		if (sra_get_attribute_value (attr[0], "bold", &tmp))
		{
		    if (strcmp (tmp, "true") == 0)
			status_[0] = 'B';
		    SR_freeString (tmp);
		}
	    	if (sra_get_attribute_value (attr[0], "italic", &tmp))
		{
		    if (strcmp (tmp, "true") == 0)
		        status_[1] = 'I';
		    SR_freeString (tmp);
		}
	    	if (sra_get_attribute_value (attr[0], "underline", &tmp))
		{
		    if (strcmp (tmp, "single") == 0 || strcmp (tmp, "double") == 0)
		        status_[2] = 'U';
		    SR_freeString (tmp);
		}
	    	if (sra_get_attribute_value (attr[0], "selected", &tmp))
		{
		    if (strcmp (tmp, "true") == 0)
		        status_[3] = 'S';
		    SR_freeString (tmp);
		}
		sra_free (attr);
		status = g_strconcat("<BRLDISP role=\"status\" start=\"0\" clear=\"true\">"
					 "<TEXT>",
					 status_,
					 "</TEXT>"
					 "</BRLDISP>", 
					 NULL);
	    }
	}
	sprintf (offset, "%d", braille_offset);
	tmp = g_strconcat ("<BRLOUT language=\"",braille_translation_table,"\" bstyle=\"", braille_style,"\" clear=\"true\">",
			"<BRLDISP role=\"main\" offset=\"", offset, "\" clear=\"true\"",
			cursor ? cursor : "",
			">",
			txt,
			NULL);
	if (braille_scroll && braille_scroll[0])
	{
	    tmp = g_strconcat (tmp, 
			       "<SCROLL mode=\"", braille_scroll, "\"></SCROLL>", 
			       "</BRLDISP>",
			       status ? status : "",
	                       "</BRLOUT>", 
			       NULL);		       
	    g_free (braille_scroll);
	}		
	else
	{
	    tmp = g_strconcat (tmp, 
			       "</BRLDISP>",
			       status ? status : "",
	                       "</BRLOUT>", 
			       NULL);
	}		
	/*fprintf (stderr, "\n%s", tmp);*/
	if (src_use_braille)
	    src_braille_send (tmp);
	if (src_use_braille_monitor)
	    src_brlmon_send (tmp);
	g_free (tmp);
	g_free (cursor);
    }
    g_free (txt);
}


static void
src_present_real_sro_for_speech (SRObject *obj, gchar *reason)
{
    SRObjectRoles role;

    srl_assert (src_use_speech);

    src_enable_format_string = TRUE; 
    sro_get_role (obj, &role, SR_INDEX_CONTAINER);
    if (strcmp (reason, "object:text-changed:insert") == 0
	    && (role == SR_ROLE_TEXT_SL || role == SR_ROLE_TEXT_ML || role == SR_ROLE_TERMINAL))
    {
	gchar *diff;
	if (sro_text_get_difference (obj, &diff, SR_INDEX_CONTAINER))
	{
	    if (diff[1] != '\0')
	    {
		if (src_last_key_echo)
		{
		    g_free (src_last_key_echo);
		    src_last_key_echo = NULL;
		}
		src_last_key_echo = diff;
		src_cmd_queue_add ("kbd key");
		src_cmd_queue_process ();
		src_last_key_echo = NULL;
	    }
	    else
	    {
		if (g_ascii_ispunct (diff[0]))
    		    src_process_punct (diff);
		else if (g_ascii_isspace (diff[0]))
    		    src_process_space (diff);
		else
    		    src_process_key_ (diff);
	    }
	    SR_freeString (diff);
	}
    }
    else
    {
	gchar *txt;
    	if (src_last_key_echo)
	{
	    g_free (src_last_key_echo);
	    src_last_key_echo = NULL;
	}
	txt = src_presentation_sro_get_pres_for_device (obj, reason, "speech");
	if (txt && txt[0])
	{
	    SRObjectRoles role;
	    SRState state;
	    sro_get_role (obj, &role, SR_INDEX_CONTAINER);
	    sro_get_state (obj, &state, SR_INDEX_CONTAINER);
	    if (state & SR_STATE_FOCUSED || strcmp (reason, "present") == 0)
	    {
		if ( role == SR_ROLE_STATUS_BAR && strcmp (reason , "focus:") == 0)
		    src_speech_send_chunk (txt, SRC_SPEECH_PRIORITY_SYSTEM, FALSE);
		else if ((role == SR_ROLE_TABLE || role == SR_ROLE_TREE_TABLE) &&
			strcmp (reason , "focus:") == 0)
		    src_speech_send_chunk (txt, SRC_SPEECH_PRIORITY_IDLE, FALSE);
		else
		    src_speech_send_chunk (txt, SRC_SPEECH_PRIORITY_IDLE, TRUE);
	    }
	    else
	    {
		src_speech_send_chunk (txt, SRC_SPEECH_PRIORITY_SYSTEM, TRUE);
	    }
	}
	g_free (txt);
    }
}

static void
src_present_real_sro_for_magnifier (SRObject *obj, gchar *reason)
{
    gchar *txt;

    srl_assert (src_use_magnifier);

    txt = src_presentation_sro_get_pres_for_device (obj, reason, "magnifier");
    if (txt)
    {
        gchar *tmp;
        tmp = g_strconcat ("<MAGOUT><ZOOMER ID=\"generic_zoomer\" tracking=\"focus\" ",
			txt,
			"></ZOOMER></MAGOUT>",
			NULL);
        if (tmp)
	    src_magnifier_send (tmp);
	g_free (tmp);
	g_free (txt);
    }
}

void
src_present_real_sro (SRObject *obj)
{
    static SRObject *old = NULL;
    gchar *reason;
    if (!obj)
	return;
    
    if (old != obj)
    {	
	src_braille_set_offset (0);
	old_offset = 0;
	if (scroll)
	{
	    g_free (scroll);
	    scroll = NULL;
        }
     }
    else
    {
	src_braille_set_offset (old_offset);
    }
    old = obj;

    if (!src_use_braille && !src_use_speech && !src_use_magnifier && !src_use_braille_monitor)
	return;
    reason = NULL;
    if (!sro_get_reason (obj, &reason))
	return;

    if (src_use_braille || src_use_braille_monitor)
	src_present_real_sro_for_brl_and_brlmon (obj, reason);
    
    if (src_use_speech)
	src_present_real_sro_for_speech (obj, reason);

    if (src_use_magnifier)
	src_present_real_sro_for_magnifier (obj, reason);

    SR_freeString (reason);
}


SRObject *src_focused_sro;
SRObject *src_crt_sro;
SRObject *src_crt_tooltip;
SRObject *src_crt_window;


gboolean
src_present_crt_sro ()
{
    if (!src_crt_sro)
	src_present_null_sro ();
    else
	src_present_real_sro (src_crt_sro);

    return TRUE;
}



gboolean
src_present_crt_window ()
{
    gchar *reason;
    sru_assert (src_crt_window);
    if (!sro_get_reason (src_crt_window, &reason))
	return FALSE;
    
    if (src_use_speech)
    {
    	gchar *txt;

	src_enable_format_string = FALSE; 
	txt = src_presentation_sro_get_pres_for_device (src_crt_window, reason, "speech");
    	if (txt && txt[0])
    	{
	    if (strcmp (reason, "window:create") == 0)
		src_speech_send_chunk (txt, SRC_SPEECH_PRIORITY_WARNING, FALSE);
	    else
		src_speech_send_chunk (txt, SRC_SPEECH_PRIORITY_SYSTEM, TRUE);
	}
	g_free (txt);
    }
    SR_freeString (reason);
    return TRUE;
}


gboolean
src_present_crt_tooltip ()
{
    gchar *reason;
    sru_assert (src_crt_tooltip);

    if (!sro_get_reason (src_crt_tooltip, &reason))
	return FALSE;
    
    if (src_use_speech)
    {
    	gchar *txt;

	src_enable_format_string = FALSE; 
	txt = src_presentation_sro_get_pres_for_device (src_crt_tooltip, reason, "speech");
    	if (txt && txt[0])
    	{
	    src_speech_send_chunk (txt, SRC_SPEECH_PRIORITY_SYSTEM, TRUE);
	}
	g_free (txt);
    }
    SR_freeString (reason);
    return TRUE;
}




gboolean
src_present_details ()
{
    gchar *name, *role, *description, *location, *message, *tmp;
    SRRectangle rect;
    
    sru_assert (src_crt_sro);
    
    role = NULL;
    if (sro_get_role_name (src_crt_sro, &tmp, SR_INDEX_CONTAINER))
    {
	role = src_xml_make_part ("TEXT", src_speech_get_voice ("role"), tmp);
	SR_freeString (tmp);
    }
    
    location = g_strdup (_("unknown location"));
    if (sro_get_location (src_crt_sro, SR_COORD_TYPE_SCREEN, 
				&rect, SR_INDEX_CONTAINER))
    {
	location = g_strdup_printf ("x %d y %d width %d height %d", rect.x, rect.y,
			    rect.width, rect.height);
    }

    tmp = location;
    location = src_xml_make_part ("TEXT", src_speech_get_voice ("location"), location);
    g_free (tmp);
    
    name = NULL;
    if (sro_get_name (src_crt_sro, &tmp, SR_INDEX_CONTAINER))
    {
	name = src_xml_make_part ("TEXT", src_speech_get_voice ("name"), tmp);
        SR_freeString (tmp);
    }
    
    description = NULL;
    if (sro_get_description (src_crt_sro, &tmp, SR_INDEX_CONTAINER))
    {
	description = src_xml_make_part ("TEXT", src_speech_get_voice ("name"), tmp);
        SR_freeString (tmp);
    }
    
    message = NULL;
    if (name || description || location || role)
    {
	message = g_strconcat (	name ? name : "",
				role ? role : "",
				description ? description : "",
				location ? location : "",
				NULL); 
    }
    else
    {
	message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), _("no details")); 
    }    
    
    if (message && message[0])
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
    
    g_free (name);
    g_free (role);
    g_free (description);
    g_free (location);
    g_free (message);
    
    return TRUE;
}


static void
src_process_layer_event (SREvent *event, 
			 unsigned long flags)
{
    gchar *key;
    
    sru_assert (event);

    sre_get_event_data (event, (void **)&key);
    if (key)
	src_ctrl_process_key (key);
}

static gchar *src_layer = NULL;

gboolean
src_present_layer_changed ()
{
    sru_assert (src_layer);
    
    if (src_use_speech)
    {    
	gchar *message, *tmp;
	
	tmp = g_strdup_printf ("%s %s", _("layer"), src_layer + (src_layer[1] == '0' ? 2 : 1));
		
	message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), tmp);
	if (message && message[0])
	    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);

	g_free (tmp);
	g_free (message);
    }
    
    return TRUE;
}

gboolean
src_present_layer_timeout ()
{
    sru_assert (src_layer);

    if (src_use_speech)
    {
	gchar *message, *tmp;
	tmp = g_strdup_printf ("%s %s", _("back to layer"), src_layer + (src_layer[1] == '0' ? 2 : 1));
	
	message = src_xml_make_part ("TEXT", src_speech_get_voice ("system"), tmp);
	if (message && message[0])
	    src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);

	g_free (tmp);
	g_free (message);
    }
    return TRUE;
}


static void
src_process_layer_changed_event (SREvent *event, 
				 unsigned long flags)
{
    gchar *layer, *cmd;
    
    sru_assert (event);
    
    sre_get_event_data (event, (void **)&layer);
    src_layer = g_strdup (layer);
    
    cmd = NULL;
    if (flags == KE_LAYER_TIME_OUT)
	cmd = "present layer timeout";
    else if (flags == KE_LAYER_CHANGED)
	cmd = "present layer changed";
    else
	sru_assert_not_reached ();    
    if (cmd)
    {
	src_cmd_queue_add (cmd);
	src_cmd_queue_process ();
    }
}


static void
src_process_sro (SRObject *obj)
{
    if (src_focused_sro)
    {
	sro_release_reference (src_focused_sro);
	src_focused_sro = NULL;
    }
    if (src_crt_sro)
    {
	sro_release_reference (src_crt_sro);
	src_crt_sro = NULL;
    }

    if (obj)
    {
        src_focused_sro = src_crt_sro = obj;
	sro_add_reference (src_focused_sro);
	sro_add_reference (src_crt_sro);
    }
    
    src_cmd_queue_add ("present current object");
    src_cmd_queue_process ();
}



static void
src_process_window (SRObject *obj)
{
    if (src_crt_window)
    	sro_release_reference (src_crt_window);
    src_crt_window = obj;

    if (obj)
	sro_add_reference (src_crt_window);
/*    {
	gchar *r = NULL, *n = NULL;
	sro_get_reason (obj, &r);
	sro_get_name (obj, &n, SR_INDEX_CONTAINER);
	fprintf (stderr, "\n%s for %s", r, n);
	SR_freeString (r);
	SR_freeString (n);
    }
*/        
    src_cmd_queue_add ("present current window");
    src_cmd_queue_process ();
}



static void
src_process_tooltip (SRObject *obj)
{

    if (src_crt_tooltip)
    	sro_release_reference (src_crt_tooltip);
    src_crt_tooltip = obj;

    if (obj)
	sro_add_reference (src_crt_tooltip);
        
    src_cmd_queue_add ("present current tooltip");
    src_cmd_queue_process ();
}


/*************************************************************/
/* GCONF CALLBACKS */

static void
src_process_config_changed_for_braille (SRConfigStructure *config)
{
    if (!config)
	return;
	
    sru_assert (config->key);
    if (strcmp (config->key, BRAILLE_DEVICE) == 0)
    {
	sru_assert (config->newvalue && config->type == CFGT_STRING);
	
	gboolean rv = src_braille_set_device ((gchar*)config->newvalue);   
	if (!rv)
	{
	    sru_message ("SR: process config changed for braille: brl_device did not change!!!");
	}
    }
    else if (strcmp (config->key, BRAILLE_PORT_NO) == 0)
    {
    	sru_assert (config->newvalue && config->type == CFGT_INT);
	
	src_braille_set_port_no (*((gint*)config->newvalue));
    }
    else
/*    if (config->key && strcmp (config->key, BRAILLE_ATTRIBUTES) == 0)
    {
    }
    else
    */
    if (strcmp (config->key, BRAILLE_STYLE) == 0)
    {
	sru_assert (config->type == CFGT_STRING && config->newvalue);

	src_braille_set_style ((gchar*)config->newvalue);
    }
    else if (strcmp (config->key, BRAILLE_CURSOR_STYLE) == 0)
    {
    	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	src_braille_set_cursor_style ((gchar*)config->newvalue);			
    }
    else if (strcmp (config->key, BRAILLE_TRANSLATION) == 0)
    {
    	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	gboolean rv = src_braille_set_translation_table ((gchar*)config->newvalue);
	
	if (!rv)
	{
	    return;
	}
    }
    else
    /*
    if (config->key && strcmp (config->key, BRAILLE_FILL_CHAR) == 0)
    {
    }
    else
    if (config->key && strcmp (config->key, BRAILLE_STATUS_CELL) == 0)
    {
    }
    else*/
    if (strcmp (config->key, BRAILLE_POSITION_SENSOR) == 0)
    {
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	src_braille_set_position_sensor ((*(gint*)config->newvalue));
    }
    else if (strcmp (config->key, BRAILLE_OPTICAL_SENSOR) == 0)
    {
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	src_braille_set_optical_sensor (*((gint*)config->newvalue));
    }	
}



static void
src_process_config_changed_for_srcore (SRConfigStructure *config)
{    
    if (!config)
	return;
	
    if (config->key && 
	config->type == CFGT_BOOL && 
	strcmp (config->key, SRCORE_EXIT_KEY) == 0)
    {
	if (config->newvalue)
	{
	    if (*(gboolean *)(config->newvalue) == TRUE)
	    {
		sru_exit_loop();
	    }
	}
    }
    if (config->key && 
	config->type == CFGT_BOOL && 
	strcmp (config->key, SRCORE_BRAILLE_ACTIVE) == 0)
    {
	if (config->newvalue)
	{
	    if (src_use_braille != *((gboolean*)config->newvalue))
	    {
		src_use_braille = *((gboolean*)config->newvalue);
		if (src_use_braille)
		{
		    src_braille_get_defaults ();
		    src_use_braille = src_braille_init ();
		}
		else
		{
		    src_braille_terminate ();
		}
	    }
	}
    }
    else
    if (config->key &&
	config->type == CFGT_BOOL &&
	strcmp (config->key, SRCORE_BRAILLE_MONITOR_ACTIVE) == 0)
    {
	if (config->newvalue)
	{
	    if (src_use_braille_monitor != *((gboolean*)config->newvalue))
	    {
		src_use_braille_monitor = *((gboolean*)config->newvalue);
	    
		if (src_use_braille_monitor)
		{
		    src_use_braille_monitor = src_braille_monitor_init ();
		}
		else
		{
		    src_braille_monitor_terminate ();
		}
	    }
	}
    }
    else
    if (config->key && 
	config->type == CFGT_BOOL && 
	strcmp (config->key, SRCORE_SPEECH_ACTIVE) == 0)
    {
	if (config->newvalue)
	{
	    if (src_use_speech != *((gboolean*)config->newvalue))
	    {
		src_use_speech = *((gboolean*)config->newvalue);
	    
		if (src_use_speech)
		{
		    src_use_speech = src_speech_init();
		}
		else
		{
		    src_speech_terminate ();
		}
	    }
	}
    }
    else	
    if (config->key && 
	config->type == CFGT_BOOL && 
	strcmp (config->key, SRCORE_MAGNIF_ACTIVE) == 0)
    {
	if (config->newvalue)
	{
	    if (src_use_magnifier != *((gboolean*)config->newvalue))
	    {
		src_use_magnifier = *((gboolean*)config->newvalue);
	    
		if (src_use_magnifier)
		{
		    src_use_magnifier = src_magnifier_init();
		    if (src_use_magnifier) 
			src_magnifier_create ();
		}
		else
		{
		    /*sleep (1); ADI: TBR */
		    src_magnifier_terminate(); 
		}
	    }
	}
    }
    else
    if (config->key && 
	config->type == CFGT_BOOL &&
	strcmp (config->key, KEYBOARD_TAKE_MOUSE) == 0)
    {
	if (config->newvalue)
	{
	    src_mouse_take = *((gboolean*)config->newvalue);
	}
    }
    else
    if (config->key && 
	config->type == CFGT_BOOL &&
	strcmp (config->key, KEYBOARD_SIMULATE_CLICK) == 0)
    {
	if (config->newvalue)
	{
	    src_mouse_click = *((gboolean*)config->newvalue);
	}
    }
    else
    if (config->key && 
	config->type == CFGT_INT &&
	strcmp (config->key, SRCORE_SCREEN_REVIEW) == 0)
    {
	if (config->newvalue)
	{
	    /* TO DO */
	}
    }
}


static void
src_process_config_changed_for_speech (SRConfigStructure *config)
{
    if (!src_use_speech)
	return;

    switch (config->module)
    {
	case CFGM_SPEECH:
	case CFGM_SPEECH_VOICE:
	    src_speech_process_config_changed (config);
	    break;
	default:
	    src_speech_process_voice_config_changed (config);
	    break;
    }
}

static void
src_process_config_changed_for_magnifier (SRConfigStructure *config)
{
    if (!src_use_magnifier)
	return;

    sru_assert (config->key);
    
    if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CURSOR) == 0)
    {
	gboolean cursor_state, rv;
	
	sru_assert (config->type == CFGT_BOOL && config->newvalue);
	
	cursor_state = *((gboolean*)config->newvalue);
	rv = src_magnifier_set_cursor_state (cursor_state);
	
	if (rv && src_use_speech)
	{	    
	    gchar *tmp;
	    tmp = g_strdup_printf (cursor_state ? _("cursor on") : _("cursor off"));
	    src_say_message (tmp);
	    g_free (tmp);    
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CURSOR_NAME) == 0)
    {
	gchar *cursor_name = NULL;
	gboolean rv;
	
	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	cursor_name = (gchar*)config->newvalue;
	rv = src_magnifier_set_cursor_name (cursor_name);
	
	if (rv && src_use_speech)
	{    
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %s", _("current cursor is"), cursor_name);
	    src_say_message (tmp);
	    g_free (tmp);
	}	
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CURSOR_SIZE) == 0)
    {
	gint cursor_size;
	gboolean rv;
	
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	cursor_size = *((gint*)config->newvalue);
	rv = src_magnifier_set_cursor_size (cursor_size);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %d", _("cursor size"), cursor_size);
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CURSOR_SCALE) == 0)
    {
	gboolean cursor_mag, rv;
	
	sru_assert (config->type == CFGT_BOOL && config->newvalue);
	
	cursor_mag = *((gboolean*)config->newvalue);
	rv = src_magnifier_set_cursor_scale (cursor_mag);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf (cursor_mag ? _("cursor magnification on") :
				    	        _("cursor magnification off"));
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CURSOR_COLOR) == 0)
    {
	gint32 cursor_color;
	gboolean rv;
	
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	cursor_color = *((gint32*)config->newvalue);
	rv = src_magnifier_set_cursor_color (cursor_color);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %u", _("cursor color"), cursor_color);
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CROSSWIRE) == 0)
    {
	gboolean crosswire, rv;
	
	sru_assert (config->type == CFGT_BOOL && config->newvalue);
	
	crosswire = *((gboolean*)config->newvalue);
	rv = src_magnifier_set_crosswire_state (crosswire);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf (crosswire ? _("crosshair on") : _("crosshair off"));
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CROSSWIRE_CLIP) == 0)
    {
	gboolean crosswire_clip, rv;
	
	sru_assert (config->type == CFGT_BOOL && config->newvalue);
	
	crosswire_clip = *((gboolean*)config->newvalue);
	rv = src_magnifier_set_crosswire_clip (crosswire_clip);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf (crosswire_clip ? _("crosshair clip on") :
				                    _("crosshair clip off"));
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CROSSWIRE_SIZE) == 0)
    {
	gint crosswire_size;
	gboolean rv;
	
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	crosswire_size = *((gint*)config->newvalue);
	rv = src_magnifier_set_crosswire_size (crosswire_size);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %d", _("crosshair size"), crosswire_size);
    	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_CROSSWIRE_COLOR) == 0)
    {
	gint32 crosswire_color;
	gboolean rv;
	
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	crosswire_color = *((gint32*)config->newvalue);
	rv = src_magnifier_set_crosswire_color (crosswire_color);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %u", _("crosshair color"), crosswire_color);
    	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ZP_LEFT) == 0)
    {
	gint zp_size;
	gboolean rv;
	
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	zp_size = *((gint*)config->newvalue);
	rv = src_magnifier_set_zp_left (zp_size);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %u", _("zoomer placement left"), zp_size);
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ZP_TOP) == 0)
    {
	gint zp_size;
	gboolean rv;
	
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	zp_size = *((gint*)config->newvalue);
	rv = src_magnifier_set_zp_top (zp_size);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %u", _("zoomer placement top"), zp_size);
    	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ZP_WIDTH) == 0)
    {
	gint zp_size;
	gboolean rv;
	
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	zp_size = *((gint*)config->newvalue);
	rv = src_magnifier_set_zp_width (zp_size);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %u", _("zoomer placement width"), zp_size);
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ZP_HEIGHT) == 0)
    {
	gint zp_size;
	gboolean rv;
	
	sru_assert (config->type == CFGT_INT && config->newvalue);
	
	zp_size = *((gint*)config->newvalue);
	rv = src_magnifier_set_zp_height (zp_size);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %u", _("zoomer placement height"), zp_size);
    	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else
/*    if (config->key && 
	strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_BORDER_WIDTH) == 0)
    {
    }
    else
    if (config->key && 
	strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_BORDER_COLOR) == 0)
    {
    }

    else*/
    if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_TARGET) == 0)
    {
	gchar *target = NULL;
	gboolean rv;
	
	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	target = (gchar*)config->newvalue;
	rv = src_magnifier_set_target (target);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %s", _("magnifier target"), target);
    	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_SOURCE) == 0)
    {
	gchar *source = NULL;
	gboolean rv;
	
	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	source = (gchar*)config->newvalue;
	rv = src_magnifier_set_source (source);
	
	if (rv & src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %s", _("magnifier source"), source);
    	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ZOOM_FACTOR_X) == 0)
    {
	gdouble zoom_factor;
	gboolean rv;
	
	sru_assert (config->type == CFGT_FLOAT && config->newvalue);
	
	zoom_factor = *((gdouble*)config->newvalue);
	rv = src_magnifier_set_zoom_factor_x (zoom_factor);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %f", _("zoom factor x"), zoom_factor);
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ZOOM_FACTOR_Y) == 0)
    {
	gdouble zoom_factor;
	gboolean rv;
	
	sru_assert (config->type == CFGT_FLOAT && config->newvalue);
	
	zoom_factor = *((gdouble*)config->newvalue);
	rv = src_magnifier_set_zoom_factor_y (zoom_factor);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %f", _("zoom factor y"), zoom_factor);
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ZOOM_FACTOR_LOCK) == 0)
    {
	gboolean zoom_factor_lock, rv;
	
	sru_assert (config->type == CFGT_BOOL && config->newvalue);

	zoom_factor_lock = *((gboolean*)config->newvalue);
	rv = src_magnifier_set_zoom_factor_lock (zoom_factor_lock);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf (zoom_factor_lock ? _("zoom factor locked") :
				            	      _("zoom factor unlocked"));
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_INVERT) == 0)
    {
	gboolean invert, rv;

	sru_assert (config->type == CFGT_BOOL && config->newvalue);
	
	invert = *((gboolean*)config->newvalue);
	rv = src_magnifier_set_invert_on_off (invert);
	
	if (rv && src_use_speech)
	{
    	    gchar *tmp;
	    tmp = g_strdup_printf (invert ? _("invert on") : _("invert off"));
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_SMOOTHING) == 0)
    {
	gchar *smoothing = NULL;
	gboolean rv;
	
	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	smoothing = (gchar*)config->newvalue;
	rv = src_magnifier_set_smoothing (smoothing);
	
	if (rv && src_use_speech)
	{
    	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %s", _("magnifier smoothing type"), smoothing);;
	    src_say_message (tmp);
	    g_free (tmp);
	}		
    }
/*    else
    if (config->key && 
	strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_TRACKING) == 0)
    {
    }*/
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_MOUSE_TRACKING) == 0)
    {
	gchar *mouse_tracking = NULL;
	gboolean rv;
	
	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	mouse_tracking = (gchar*)config->newvalue;
	rv = src_magnifier_set_mouse_tracking_mode (mouse_tracking);
	
	if (rv && src_use_speech)
	{
	    gchar *tmp;
	    tmp = g_strdup_printf ("%s %s", _("mouse tracking mode"), mouse_tracking);
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
/*    else
    if (config->key && 
	strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_VISIBLE) == 0)
    {
    }*/
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_PANNING) == 0)
    {
	gboolean panning, rv;
	
	sru_assert (config->type == CFGT_BOOL && config->newvalue);
	
	panning = *((gboolean*)config->newvalue);
	rv = src_magnifier_set_panning_on_off (panning);
	
	if (rv && src_use_speech)	    
	{
	    gchar *tmp;
	    tmp = g_strdup (panning ? _("panning on") : _("panning off"));
	    src_say_message (tmp);
	    g_free (tmp);
	}
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ALIGNMENT_X) == 0)
    {
	gchar *alignment_x = NULL;
	
	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	alignment_x = (gchar*)config->newvalue;
	src_magnifier_set_alignment_x (alignment_x);
    }
    else if (strcmp (config->key, MAGNIFIER_ACTIVE_SCHEMA MAGNIFIER_ALIGNMENT_Y) == 0)
    {
	gchar *alignment_y = NULL;
	
	sru_assert (config->type == CFGT_STRING && config->newvalue);
	
	alignment_y= (gchar*)config->newvalue;
	src_magnifier_set_alignment_y (alignment_y);    
    }
}

static void
src_process_config_changed_for_control (SRConfigStructure *config)
{
    if (config)
    {
	src_ctrl_terminate 	();
	src_ctrl_init 		();
	ke_config_changed	();
    }
    /*src_update_key ();*/
}

static void
src_process_change_for_presentation (gboolean first)
{
    gchar *name = DEFAULT_PRESENTATION_ACTIVE_SETTING_NAME;
    gchar *name2;

    srconf_get_data_with_default (PRESENTATION_ACTIVE_SETTING, CFGT_STRING, &name, name, PRESENTATION_PATH);

    name2 = g_strconcat (PRESENTATION_DIR, name, ".xml", NULL); 
    if (!first)
	src_presentation_terminate ();
    src_presentation_init (name2);
    g_free (name2);
}

static void
src_process_config_changed_for_presentation (SRConfigStructure *config)
{
    if (config)
    {
	src_process_change_for_presentation (FALSE);
    }
}

static void
src_process_config_changed_for_keyboard (SRConfigStructure *config)
{
    if (!config) 
	return;
}



static void
src_process_config_changed (SREvent *event,
			    unsigned long flags)
{
    SRConfigStructure *config;
    
    if (!sre_get_event_data (event, (void**)&config))
	return;
    if (!config)
	return;
    
/* for debuggung purpose */
#ifdef DEBUG_SRCORE
    if (config != NULL)
    {
	printf ("\n*********************************************************************\n");
	printf ("SRCORE: CONFIG_CHANGED/process_config_changed(): \n\tModule: %d \n\tKey: %s\n\tType: %i\n\tNewValue: ",
		config->module, config->key, config->type);
	switch (config->type)
	{
	    case CFGT_INT:
		printf ("%i\n", *((gint*)config->newvalue));
		break;
    	    case CFGT_FLOAT:
		printf ("%f\n", *((gdouble*)config->newvalue));
		break;
    	    case CFGT_STRING:
		printf ("%s\n", config->newvalue ? config->newvalue : " ");
		break;
    	    case CFGT_BOOL:
		printf ("%i\n", *((gboolean*)config->newvalue));
		break;
	    default:
		break;
	}
	printf ("*********************************************************************\n");
	fflush(stdout);
    }
#endif

    switch (config->module)
    {
	case CFGM_BRAILLE:
	    src_process_config_changed_for_braille (config);
	    break;
	case CFGM_KBD_MOUSE:
	    src_process_config_changed_for_keyboard (config);
	    break;
	case CFGM_GNOPI:
	    break;
    	case CFGM_MAGNIFIER:
	    src_process_config_changed_for_magnifier (config); 
	    break;
	case CFGM_SPEECH:
	case CFGM_SPEECH_VOICE:
	case CFGM_SPEECH_VOICE_PARAM:
	    src_process_config_changed_for_speech (config);
	    break;
	case CFGM_SRCORE:
	    src_process_config_changed_for_srcore (config);
	    break;
	case CFGM_KEY_PAD:
	    src_process_config_changed_for_control (config);
	    break;
	case CFGM_PRESENTATION:
	    src_process_config_changed_for_presentation (config);
	    break;
    }
}

static void
src_process_keyboard_echo (SREvent *event,
			   unsigned long flags)
{
/*
    gchar *str;
    if (!sre_get_event_data (event, (void**)&str))
	return;
    if (!str)
	return;
    
    fprintf(stderr, "PROCESS KEYBOARD ECHO: %s\n", str);
*/
}
	
static void
src_process_hotkey (SREvent *event,
		    unsigned long flags)
{			
/*
    SRHotkeyData *srhotkey_data;
    
    if (!sre_get_event_data (event, (void**)&srhotkey_data))
	return;
    if (!srhotkey_data)
	return;
    
    fprintf(stderr, "SR: process HotKey: %s%s%s%s\n", srhotkey_data->modifiers & SRHOTKEY_ALT ? "ALT + " : "",
			 srhotkey_data->modifiers & SRHOTKEY_CTRL ? "CTRL + " : "",
			 srhotkey_data->modifiers & SRHOTKEY_SHIFT ? "SHIFT + ": "",
			 srhotkey_data->keystring ? srhotkey_data->keystring : " ");
*/
}
gboolean
src_kb_key_echo ()
{
    gchar *message;
    sru_assert (src_last_key_echo);

    if (!src_use_speech)
	return FALSE;
    message = src_xml_format ("TEXT", src_speech_get_voice ("system"), src_last_key_echo);
    if (message && message[0])
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_MESSAGE, TRUE);
    g_free (message); 
    
    return TRUE;    
}

gboolean
src_kb_punct_echo ()
{
    gchar *message;
    sru_assert (src_last_punct_echo);

    if (!src_use_speech)
	return FALSE;
    message = src_xml_format ("TEXT", src_speech_get_voice ("system"), src_last_punct_echo);
    if (message && message[0])
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
    g_free (message); 
    
    return TRUE;    
}

gboolean
src_kb_space_echo ()
{
    gchar *message;
    sru_assert (src_last_space_echo);

    if (!src_use_speech)
	return FALSE;
    message = src_xml_format ("TEXT", src_speech_get_voice ("system"), src_last_space_echo);
    if (message && message[0])
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
    g_free (message); 
    
    return TRUE;    
}

gboolean
src_kb_modifier_echo ()
{
    gchar *message;
    sru_assert (src_last_modifier_echo);

    if (!src_use_speech)
	return FALSE;
    message = src_xml_format ("TEXT", src_speech_get_voice ("system"), src_last_modifier_echo);
    if (message && message[0])
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_IDLE, TRUE);
    g_free (message); 
    
    return TRUE;    
}


gboolean
src_kb_cursor_echo ()
{
    gchar *message;
    sru_assert (src_last_cursor_echo);

    if (!src_use_speech)
	return FALSE;
    message = src_xml_format ("TEXT", src_speech_get_voice ("system"), src_last_cursor_echo);
    if (message && message[0])
	src_speech_send_chunk (message, SRC_SPEECH_PRIORITY_SYSTEM, TRUE);
    g_free (message); 

    return TRUE;    
}


static gboolean
src_process_key_ (gchar *key)
{
    SRCSpeechTextEchoType  text_echo_type;
    
    sru_assert (key);
    text_echo_type = src_speech_get_text_echo_type ();

    if (text_echo_type == SRC_SPEECH_TEXT_ECHO_CHAR)
    {
	src_last_key_echo = key;
	src_cmd_queue_add ("kbd key");
	src_cmd_queue_process ();
	src_last_key_echo = NULL;	
    }
    else if (text_echo_type == SRC_SPEECH_TEXT_ECHO_WORD)
    {
	gchar *tmp = src_last_key_echo;
	
	src_last_key_echo = g_strconcat (src_last_key_echo ? src_last_key_echo : "",
					 key,
					 NULL);
	g_free (tmp);	
    }
    else if (text_echo_type == SRC_SPEECH_TEXT_ECHO_NONE)
    {	
    }
    else
	sru_assert_not_reached ();
    return TRUE;
}

#define SRC_TP_NONE	0
#define SRC_TP_ALL	1

static gboolean
src_process_punct (gchar *punct)
{
    static gint src_kb_punct_mode = SRC_TP_ALL;

    sru_assert (punct);
    
    g_free (src_last_punct_echo);
    src_last_punct_echo = NULL;
    
    if (src_last_key_echo)
	src_cmd_queue_add ("kbd key");

    
    if (src_kb_punct_mode == SRC_TP_ALL)
    {
	src_last_punct_echo = g_strdup (punct);
	src_cmd_queue_add ("kbd punct");
    }
    
    if (src_last_key_echo || src_last_punct_echo)
	src_cmd_queue_process ();
    
    g_free (src_last_key_echo);
    src_last_key_echo = NULL;
    
    return TRUE;
}


static gboolean
src_process_space (gchar *space)
{
    SRCSpeechSpacesEchoType spaces_echo_type;
    
    sru_assert (space);
    
    g_free (src_last_space_echo);
    src_last_space_echo = NULL;

    spaces_echo_type = src_speech_get_spaces_echo_type ();
    if (src_last_key_echo)
	src_cmd_queue_add ("kbd key");

    if (spaces_echo_type == SRC_SPEECH_SPACES_ECHO_ALL)
    {
	src_last_space_echo = g_strdup (space);
	src_cmd_queue_add ("kbd space");
    }

    if (src_last_key_echo || src_last_space_echo)
	src_cmd_queue_process ();

    g_free (src_last_key_echo);
    src_last_key_echo = NULL;
    return TRUE;
}


static gboolean
src_process_modifier (gchar *modifier)
{
    SRCSpeechModifiersEchoType modifiers_echo_type;

    sru_assert (modifier);
    
    if (src_last_modifier_echo)
	g_free (src_last_modifier_echo);
    src_last_modifier_echo = NULL;

    modifiers_echo_type = src_speech_get_modifiers_echo_type ();
    
    if (modifiers_echo_type == SRC_SPEECH_MODIFIERS_ECHO_ALL)
    {
	gchar *part1, *part2;
	gint pos;
    
	part1 = part2 = NULL;
	pos = 0;

	if (strncmp (modifier, "Alt", 3) == 0)
	{
	    part1 = _("alt");
	    pos = 4;
	}
	else if (strncmp (modifier, "Control", 7) == 0)
	{
	    part1 = _("control");
	    pos = 8;
	}
	else if (strncmp (modifier, "Shift", 5) == 0)
	{
	    part1 = _("shift");
	    pos = 6;
        }
	else if (strncmp (modifier, "Caps", 3) == 0)
	{
	    part1 = _("caps lock");
	}
	else if (strncmp (modifier, "Num", 3) == 0)
	{
	    part1 = _("num lock");
	}
	else
	    sru_assert_not_reached ();
    
	if (pos)
	{
	    if (modifier[pos] == 'R')
		part2 = _("right");
	    else if (modifier[pos] == 'L')
		part2 = _("left");
	    else
		sru_assert_not_reached ();
	}
	src_last_modifier_echo = g_strconcat (part1 ? part1 : "", 
					    part2 ? " " : "",
					    part2 ? part2 : "",
					    NULL);

	src_cmd_queue_add ("kbd modifier");
	src_cmd_queue_process ();
    }
    return TRUE;
}

static gboolean
src_process_cursor (gchar *cursor)
{
    SRCSpeechCursorsEchoType cursors_echo_type;
    
    sru_assert (cursor);
    
    if (src_last_cursor_echo)
	g_free (src_last_cursor_echo);
    src_last_cursor_echo = NULL;

    if (src_last_key_echo)
	src_cmd_queue_add ("kbd key");
    
    cursors_echo_type = src_speech_get_cursors_echo_type ();
    
    if (cursors_echo_type == SRC_SPEECH_CURSORS_ECHO_ALL)
    {
	if (strcmp (cursor, "Home") == 0)
	    src_last_cursor_echo = g_strdup (_("home"));
        else if (strcmp (cursor, "End") == 0)
    	    src_last_cursor_echo = g_strdup (_("end"));
	else if (strcmp (cursor, "Page_Up") == 0)
	    src_last_cursor_echo = g_strdup (_("page up"));
	else if (strcmp (cursor, "Page_Down") == 0)
	    src_last_cursor_echo = g_strdup (_("page down"));
	else if (strcmp (cursor, "Right") == 0)
	    src_last_cursor_echo = g_strdup (_("right"));
	else if (strcmp (cursor, "Left") == 0)
	    src_last_cursor_echo = g_strdup (_("left"));
	else if (strcmp (cursor, "Down") == 0)
	    src_last_cursor_echo = g_strdup (_("down"));
	else if (strcmp (cursor, "Up") == 0)
	    src_last_cursor_echo = g_strdup (_("up"));
	else if (strcmp (cursor, "BackSpace") == 0)
	    src_last_cursor_echo = g_strdup (_("backspace"));
	else if (strcmp (cursor, "Delete") == 0)
	    src_last_cursor_echo = g_strdup (_("delete"));
	else if (strcmp (cursor, "Escape") == 0)
	    src_last_cursor_echo = g_strdup (_("escape"));	    
	else
	    sru_assert_not_reached ();
	        
	src_cmd_queue_add ("kbd cursor");
    }

    if (src_last_key_echo || src_last_cursor_echo)
	src_cmd_queue_process ();

    g_free (src_last_key_echo);
    src_last_key_echo = NULL;

    return TRUE;
}



static void
src_process_key (SREvent *event,
		 unsigned long flags)
{
    SRHotkeyData *srhotkey_data;

    if (!src_use_speech)
	return;
	
    if (!sre_get_event_data (event, (void**)&srhotkey_data))
	return;
    if (!srhotkey_data)
	return;

    if (!srhotkey_data->keystring)
	return;
    if (strcmp (srhotkey_data->keystring, "Control_L") == 0)
	if (src_use_speech)
	    src_speech_shutup ();

    if (strncmp (srhotkey_data->keystring, "Alt", 3) == 0 ||
	strncmp (srhotkey_data->keystring, "Control", 7) == 0 ||
	strncmp (srhotkey_data->keystring, "Shift", 5) == 0 ||
	strncmp (srhotkey_data->keystring, "Caps", 3) == 0 ||
	strncmp (srhotkey_data->keystring, "Num", 3) == 0)
    {
    
	src_process_modifier (srhotkey_data->keystring);
    }
    else if (strcmp (srhotkey_data->keystring, "Home") == 0 ||
	     strcmp (srhotkey_data->keystring, "End") == 0 ||
	     strcmp (srhotkey_data->keystring, "Page_Up") == 0 ||
	     strcmp (srhotkey_data->keystring, "Page_Down") == 0 ||
	     strcmp (srhotkey_data->keystring, "Right") == 0 ||
	     strcmp (srhotkey_data->keystring, "Left") == 0 ||
	     strcmp (srhotkey_data->keystring, "Down") == 0 ||
	     strcmp (srhotkey_data->keystring, "Up") == 0 ||
	     strcmp (srhotkey_data->keystring, "BackSpace") == 0 ||
	     strcmp (srhotkey_data->keystring, "Delete") == 0 ||
	     strcmp (srhotkey_data->keystring, "Escape") == 0)
    {
	src_process_cursor (srhotkey_data->keystring);
    }
}

typedef enum
{
    BST_PS = 0,
    BST_HOS
}SRBrlSensorTypes;

static gboolean
src_translate_sensor_code (gchar *sensor_code,
			   SRBrlSensorTypes *sensor_type,
			   gint *sensor_index)
{
	/* !!! TBI: !!! this table will be loaded for each actual device */
	/* !!! TO DO like this !!! */
/*	static gchar *sensor_codes = {	"HMS00", "HMS01", "HMS02", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00",
					"HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00",
					"HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00",
					"HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00",
					"HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00",
					"HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00",
					"HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00",
					"HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00", "HMS00"
				};
*/

    gchar *sensor_bank = NULL;
    gboolean rv = FALSE;

    /* BAUM only or others that comply with Baum sensor naming convention */
    if (sensor_code)
    {
	sensor_bank = g_strndup(sensor_code, 3);
	/* 
	    DR: could be
	    sensor_bank = sensor_code +3; 
	    !!!!!!!! then atention to g_free
	*/
    }
    if (sensor_bank)
    {
	*sensor_index = atoi(&sensor_code[3]);
	if (g_strcasecmp(sensor_bank, "HMS") == 0)
	{
	    *sensor_type = BST_PS;
	    rv = TRUE;
	}
	else if (g_strcasecmp(sensor_bank, "HOS") == 0)
	{
	    *sensor_type = BST_HOS;
	    rv = TRUE;
	}
	else if (g_strcasecmp(sensor_bank, "LOS") == 0)
	{
	    /* fprintf(stderr, "SR: left optical sensor pressed\n"); */
	}
	else if (g_strcasecmp(sensor_bank, "ROS") == 0)
	{
	    /* fprintf(stderr, "SR: right optical sensor pressed\n"); */
	}
	else
	{
	    /* fprintf(stderr, "SR: this cannot be...\n"); */
	}
	g_free(sensor_bank);
    }
    return rv;
}

static void
src_process_braille_sensor (gchar *code)
{
    SRBrlSensorTypes type;
    gint index;

    if (!code)
	return;
	
    if (src_translate_sensor_code (code, &type, &index))
    {
	switch (type)
	{
	    case BST_PS:
		src_ctrl_position_sensor_action (index);
		break;
	    case BST_HOS:
		src_ctrl_optical_sensor_action (index);
		break;
	    default:
	    	fprintf (stderr, "SR: unsuported sensor code\n");
		break;
	}
    }
}




static void
src_process_sro_event (SREvent *event,
			unsigned long flags)
{
    SRObject *obj;

    if (sre_get_event_data (event, (void**)&obj))
    {
        if (src_use_magnifier)
	{
	    gchar *reason;
	    if (sro_get_reason (obj, &reason))
	    {
		if (strcmp (reason, "object:visible-data-changed") == 0)
		    src_present_real_sro_for_magnifier (obj, reason);
		SR_freeString (reason);
	    }	    
	}
	src_process_sro (obj);
	if (src_use_magnifier)
	    src_magnifier_start_panning (obj);
    }
}

static void
src_process_window_event (SREvent *event,
			    unsigned long flags)
{
    SRObject *obj;

    if (sre_get_event_data (event, (void**)&obj))
    {
	src_process_window (obj);
    }
}

static void
src_process_tooltip_event (SREvent *event,
			    unsigned long flags)
{
    SRObject *obj;

    if (sre_get_event_data (event, (void**)&obj))
    {
	src_process_tooltip (obj);
    }
}

static void
src_process_mouse_event (SREvent *event,
		 unsigned long flags)
{
    SRPoint *point;

    if (!src_use_magnifier)
	return;
	
    if (sre_get_event_data (event, (void**)&point))
    {
	gchar *magout;

	src_magnifier_stop_panning ();
	magout = g_strdup_printf ("<MAGOUT><ZOOMER ID =\"%s\" tracking=\"mouse\" ROILeft =\"%d\" ROITop =\"%d\" ROIWidth =\"%d\" ROIHeight=\"%d\"></ZOOMER></MAGOUT>",
				"generic_zoomer",
				point->x, point->y, point->x + 1, point->y + 1);
	if (magout)
	    src_magnifier_send (magout);
	g_free (magout);
    }
}

static void
src_event_sink(SREvent *event_, 
	       unsigned long flags)
{
    SREventType type;
    static GSList *list = NULL;
    static gboolean busy = FALSE;

    if (!event_)
	return;

    list = g_slist_append (list, event_);
    sre_add_reference (event_);

    if (busy)
	return;
    busy = TRUE;
    
    while (list)
    {
	SREvent *event;
	GSList *tmp;
	
	event = (SREvent*) list->data;
	tmp = list;
	list = list->next;
	g_slist_free_1 (tmp);
	
	if (sre_get_type(event, &type))
	{
/*	    static gboolean shutup = TRUE;

	    switch (type)
	    {
		case SR_EVENT_WINDOW:
		    src_speech_shutup_ ();
		    shutup = FALSE;
		    break;
		case SR_EVENT_MOUSE:
		case SR_EVENT_COMMAND_LAYER:
		    break;	    
		default:
		    if (shutup)
			src_speech_shutup_ ();
		    shutup = TRUE;
		    break;		 
	    }
*/	
	    switch (type)
	    {
		case SR_EVENT_SRO:
		case SR_EVENT_WINDOW:
		    src_ctrl_flat_mode_terminate ();
		    break;
		default:
		    break;
	    }
	    switch (type)
	    {
		case SR_EVENT_CONFIG_CHANGED:
		    src_process_config_changed(event, flags);
	   	    break;
		case SR_EVENT_KEYBOARD_ECHO:
		    src_process_keyboard_echo(event, flags);
	   	    break;
		case SR_EVENT_HOTKEY:
		    src_process_hotkey(event, flags);
	   	    break;
		case SR_EVENT_KEY:
	       	    src_process_key(event, flags);
	   	    break;
		case SR_EVENT_COMMAND_LAYER:
		    src_process_layer_event (event, flags);
		    break;
		case SR_EVENT_COMMAND_LAYER_CHANGED:
	    	    src_process_layer_changed_event (event, flags);
		    break;
		case SR_EVENT_SRO:
		    src_process_sro_event (event, flags);
		    break;
		case SR_EVENT_WINDOW:
		    src_process_window_event (event, flags);
		    break;
		case SR_EVENT_TOOLTIP:
		    src_process_tooltip_event (event, flags);
		    break;
		case SR_EVENT_MOUSE:
		    src_process_mouse_event (event, flags);
		    break;
		default:
		    sru_assert_not_reached ();
		    break;
	    }
	}
	sre_release_reference (event);
    }
    busy = FALSE;
}

void
brl_input_event (BRLInEvent *event)
{
    switch (event->event_type)
    {
    	case BIET_KEY:
/*	    fprintf (stderr, "SENSOR: %s\n", event->event_data.key_codes);*/
	    if (event->event_data.key_codes)
		src_ctrl_process_key (event->event_data.key_codes);
	    break;
	case BIET_SENSOR:
	    /* fprintf (stderr, "SENSOR: %s\n", brl_in_event->event_data.sensor_codes); */
	    src_process_braille_sensor (event->event_data.sensor_codes);
	    break;
    	case BIET_SWITCH:
	    /* fprintf (stderr, "SWITCH: %s\n", brl_in_event->event_data.switch_codes); */
	    break;
	default:
	    fprintf (stderr, "UNKNOWN BRAILLE EVENT");
	break;
    }
}

void
brl_xml_input_proc (char* buffer, int len)
{
    brl_in_xml_parse (buffer, len);
}


static void
src_get_defaults ()
{
    src_braille_get_defaults ();
    
    if ((gint)src_use_braille == -1)
    {
	src_use_braille = DEFAULT_SRCORE_BRAILLE_ACTIVE;
	GET_SRCORE_CONFIG_DATA_WITH_DEFAULT (SRCORE_BRAILLE_ACTIVE, CFGT_BOOL, 
	    &src_use_braille, &src_use_braille);
    }
    
    
    if ((gint)src_use_braille_monitor == -1)
    {
	src_use_braille_monitor = DEFAULT_SRCORE_BRAILLE_MONITOR_ACTIVE;
	GET_SRCORE_CONFIG_DATA_WITH_DEFAULT (SRCORE_BRAILLE_MONITOR_ACTIVE, CFGT_BOOL, 
	    &src_use_braille_monitor, &src_use_braille_monitor);
    }
    
    if ((gint)src_use_speech == -1)
    {
	src_use_speech = DEFAULT_SRCORE_SPEECH_ACTIVE;
	GET_SRCORE_CONFIG_DATA_WITH_DEFAULT (SRCORE_SPEECH_ACTIVE, CFGT_BOOL, 
					    &src_use_speech, &src_use_speech);
    }

    if ((gint)src_use_magnifier == -1)
    {
	src_use_magnifier = DEFAULT_SRCORE_MAGNIF_ACTIVE;
	GET_SRCORE_CONFIG_DATA_WITH_DEFAULT (SRCORE_MAGNIF_ACTIVE, CFGT_BOOL, 
					    &src_use_magnifier, &src_use_magnifier);
    }
    
    GET_SRCORE_CONFIG_DATA_WITH_DEFAULT (KEYBOARD_TAKE_MOUSE, CFGT_BOOL, 
	    &src_mouse_take, &src_mouse_take);
    GET_SRCORE_CONFIG_DATA_WITH_DEFAULT (KEYBOARD_SIMULATE_CLICK, CFGT_BOOL, 
	    &src_mouse_click, &src_mouse_click);
	    
}

static gchar *src_message;

gboolean 
src_say_message (const gchar *message)
{
    if (src_message)
	g_free (src_message);
    src_message = NULL;
    if (message && message[0])
	src_message = g_strdup (message);
    src_cmd_queue_add ("present last message");
    src_cmd_queue_process ();
    
    return TRUE;
}


gboolean 
src_present_last_message ()
{

    if (src_use_speech)
	if (src_message)
	    src_speech_say (src_message, TRUE);
    return TRUE;
}

void
src_set_command_lines (const gchar *mouse_tracking,
		       const gchar *focus_tracking,
		       gboolean    invert,
		       gdouble 	   zoomfactor)
{
    if (mouse_tracking)
    {
    	gchar *mt = g_ascii_strdown (mouse_tracking, -1);
	if (!strcmp (mt, "centered"))
	{
	    g_free (mt);
	    mt = g_strdup ("mouse-centered");
	}
	else if (!strcmp (mt, "proportional"))
	{
	    g_free (mt);
	    mt = g_strdup ("mouse-proportional");
	}
	else if (!strcmp (mt, "push"))
	{
	    g_free (mt);
	    mt = g_strdup ("mouse-push");
	}
	else if (!strcmp (mt, "none"))
	{
	    g_free (mt);
	    mt = g_strdup ("none");
	}
	else
	{
	    g_free (mt);
	    mt = NULL;
	}
	if (mt)
	{
	    srconf_set_data (MAGNIFIER_MOUSE_TRACKING,
			  CFGT_STRING, 
			  mt,
			  MAGNIFIER_CONFIG_PATH);
	
	}
	g_free (mt);
    }
    
    if (focus_tracking)
    {
	gchar *ft = g_ascii_strdown (focus_tracking, -1);
	if (!strcmp (ft, "none") ||
	    !strcmp (ft, "centered") ||
	    !strcmp (ft, "auto"))
	{
	    srconf_set_data (MAGNIFIER_ALIGNMENT_Y,
			  CFGT_STRING, 
			  ft,
			  MAGNIFIER_CONFIG_PATH);
    
	    srconf_set_data (MAGNIFIER_ALIGNMENT_X,
			  CFGT_STRING, 
			  ft,
			  MAGNIFIER_CONFIG_PATH);
	}
	g_free (ft);
    }
    
    srconf_set_data (MAGNIFIER_INVERT,
			  CFGT_BOOL, 
			  &invert,
			  MAGNIFIER_CONFIG_PATH);
    
    if (0 < zoomfactor && 17 > zoomfactor)
    {
    	srconf_set_data (MAGNIFIER_ZOOM_FACTOR_X,
			  CFGT_FLOAT, 
			  &zoomfactor,
			  MAGNIFIER_CONFIG_PATH);
    	srconf_set_data (MAGNIFIER_ZOOM_FACTOR_Y,
			  CFGT_FLOAT, 
			  &zoomfactor,
			  MAGNIFIER_CONFIG_PATH);
    }
}    

static void
src_init(const gchar *mouse_tracking,
	 const gchar *focus_tracking,
	 const gchar *config_source,
	 gboolean    invert,
	 gint 	     zoomfactor,
	 gboolean    horiz_split)
{
    SRLClient client;
    gchar *message = _("Welcome to Gnopernicus");
    gboolean brl_error = FALSE;
    gboolean initialize_finished = FALSE;
    gboolean default_invert_value;

    /* initialize SR Utils */
    sru_init();

    /* initialize SRConf */
    srconf_init((SRConfCB)src_event_sink, "/apps/gnopernicus", config_source);

    /* layout init */
    src_ctrl_init ();
    
/*    src_speech_count_mode = DEFAULT_SPEECH_REPEAT_TYPE;*/
    src_get_defaults ();
    
    if (braille_device)
	src_braille_set_device (braille_device);
    
    if (braille_port)
	src_braille_set_port_no (braille_port);
    /* initialize 
    
    Braille */
    if (src_use_braille)
    {
    	src_use_braille = src_braille_init ();
	if (!src_use_braille)
	    brl_error = TRUE;
    }
	
    if (src_use_braille_monitor)
	src_use_braille_monitor = src_braille_monitor_init ();
 	
    /* initialize Speech */
    if (src_use_speech)
	src_use_speech = src_speech_init ();
    
    /* check if invert is true */
    if (!invert)
    {
	default_invert_value = DEFAULT_MAGNIFIER_INVERT;
	srconf_get_data_with_default (MAGNIFIER_INVERT,
				      CFGT_BOOL,
				      (gpointer)&invert,
				      (gpointer)&default_invert_value,
				      MAGNIFIER_CONFIG_PATH);
    }
    
    /* set the command lines arguments in gconf */
    src_set_command_lines (mouse_tracking,
		    	    focus_tracking,
			    invert,
			    zoomfactor);
			    
    
    /* initialize Magnifier */
    if (src_use_magnifier)
    {
	src_use_magnifier = src_magnifier_init ();
	if (src_use_magnifier)
	{
	    src_magnifier_create ();
	    src_magnifier_set_split_screen_horizontal (horiz_split);
	}
    }
    
    
    src_process_change_for_presentation (TRUE);

    /* initialize SRLow */
    srl_init();

    client.event_proc = (SROnEventProc) src_event_sink;
    src_srl_client = srl_add_client (&client);

    /* initialize keyboard */
    ke_init ((KeyboardEchoCB)&src_event_sink);

    src_layer = NULL;
    src_focused_sro = NULL;
    src_crt_sro = NULL;
    src_crt_tooltip = NULL;
    src_crt_window = NULL;

    src_message = NULL;

    /* and Welcome to Gnopernicus... */
    if (src_use_speech)
	src_say_message (message);
    if (src_use_braille)
	src_braille_show (message);
    if (src_use_braille_monitor)
	src_brlmon_show (message);
    src_mouse_take = DEFAULT_KEYBOARD_TAKE_MOUSE;
    src_mouse_click = DEFAULT_KEYBOARD_SIMULATE_CLICK;

    src_last_key_echo		= NULL;
    src_last_punct_echo 	= NULL;
    src_last_space_echo 	= NULL;
    src_last_modifier_echo 	= NULL;
    src_last_cursor_echo 	= NULL;
    if (brl_error)
	src_say_message (_("braille device can not be initialized"));

    /* src_init finished */
    SET_SRCORE_CONFIG_DATA(SRCORE_INITIALIZATION_END, CFGT_BOOL, &initialize_finished);
    initialize_finished = !initialize_finished;
    SET_SRCORE_CONFIG_DATA(SRCORE_INITIALIZATION_END, CFGT_BOOL, &initialize_finished);    
}

static void
src_terminate()
{
    gboolean exitack = TRUE;
    gchar *message = _("Gnopernicus is now exiting. Have a nice day!");

    if (src_focused_sro)
	sro_release_reference (src_focused_sro);
    if (src_crt_sro) 
	sro_release_reference (src_crt_sro);
    if (src_crt_tooltip)
	sro_release_reference (src_crt_tooltip);
    if (src_crt_window)
	sro_release_reference (src_crt_window);

    if (src_use_braille)
	src_braille_show (message);
	
    if (src_use_braille_monitor)
	src_brlmon_show (message);
	
    if (src_use_speech)
	src_say_message (message);
    /* FIXME remus: maybe this function should not be called here, in respect to
    current design */
    /* terminate screen review in case that Exit was called and gnopernicus
    is still in flat review mode*/
    screen_review_terminate ();
	
    /* terminate SRLow */
    srl_remove_client (src_srl_client);
    srl_terminate ();

    /* terminate Keyboard */
    ke_terminate();

    /* terminate Magnifier */
    if (src_use_magnifier)
	src_magnifier_terminate (); 

    /* terminate Braille */
    if (src_use_braille)
	src_braille_terminate ();
	
    /*terminate Braille Monitor*/	
    if (src_use_braille_monitor)
	src_brlmon_terminate ();

    /* terminate Speech */
    if (src_use_speech)
	src_speech_terminate ();
	
    /* terminate SRConf */
    SET_SRCORE_CONFIG_DATA(SRCORE_EXIT_ACK_KEY, CFGT_BOOL, &exitack);
    exitack = !exitack;
    SET_SRCORE_CONFIG_DATA(SRCORE_EXIT_ACK_KEY, CFGT_BOOL, &exitack);
    srconf_terminate();

    /* end of SPI usage */
    sru_terminate();	
    src_presentation_terminate ();
    src_ctrl_terminate ();
    if (src_layer)
	g_free (src_layer);
    if (src_message)
	g_free (src_message);
    if (src_last_key_echo)
	g_free (src_last_key_echo);
    if (src_last_punct_echo)
	g_free (src_last_punct_echo);
    if (src_last_space_echo)
	g_free (src_last_space_echo);
    if (src_last_modifier_echo)
	g_free (src_last_modifier_echo);
    if (src_last_cursor_echo)
	g_free (src_last_cursor_echo);
}
    
gint
main (gint argc, 
      gchar **argv)
{
    gboolean enable_braille 	= FALSE;
    gboolean disable_braille 	= FALSE;
    gboolean enable_speech 	= FALSE;
    gboolean disable_speech 	= FALSE;
    gboolean enable_magnifier	= FALSE;
    gboolean disable_magnifier	= FALSE;
    gboolean enable_braille_monitor 	= FALSE;
    gboolean disable_braille_monitor 	= FALSE;    
    
    gchar 	*config_source 	= NULL;
    gchar 	*mouse_tracking = NULL;
    gchar	*focus_tracking = NULL;

    gint  	zoom_factor     = 0;
    gboolean 	invert 	    	= FALSE;
    gboolean    horiz_split     = FALSE;
    gboolean 	login_enabled	= FALSE;    

    
    struct poptOption poptopt[] = 
    {		
	{
	 "enable-braille",	
	 'b', 	
	 POPT_ARG_NONE, 
	 &enable_braille,		
	 0, 
	 "Enable braille service", 
	 NULL
	},
	{
	 "disable-braille",	
	 'B', 	
	 POPT_ARG_NONE, 
	 &disable_braille,		
	 0, 
	 "Disable braille service", 
	 NULL
	},
	{
	 "enable-speech",	
	 's', 	
	 POPT_ARG_NONE, 
	 &enable_speech,		
	 0, 
	 "Enable speech service", 
	 NULL
	},
	{
	 "disable-speech",	
	 'S', 	
	 POPT_ARG_NONE, 
	 &disable_speech,		
	 0, 
	 "Disable speech service", 
	 NULL
	},
	{
	 "enable-magnifier",	
	 'm', 	
	 POPT_ARG_NONE, 
	 &enable_magnifier,		
	 0, 
	 "Enable magnifier service",  
	 NULL
	},
	{
	 "disable-magnifier",	
	 'M', 	
	 POPT_ARG_NONE, 
	 &disable_magnifier,		
	 0, 
	 "Disable magnifier service",  
	 NULL
	},

	{
	 "enable-braille-monitor", 
	 'o', 
	 POPT_ARG_NONE, 
	 &enable_braille_monitor, 	
	 0, 
	 "Enable braille monitor service", 
	 NULL
	},
	{
	 "disable-braille-monitor", 
	 'O', 
	 POPT_ARG_NONE, 
	 &disable_braille_monitor, 	
	 0, 
	 "Disable braille monitor service", 
	 NULL
	},
	{
	 "braille-port", 
	 'p',   
	 POPT_ARG_INT,  
	 &braille_port, 	
	 0, 
	 "Serial port (ttyS)", 
	 "ttyS[1..4]"
	},
	{
	 "braille-device", 
	 'e', 
	 POPT_ARG_STRING, 
	 &braille_device, 	
	 0, 
	 "Braille Device", 
	 "DEVICE NAME"
	},
	{
	"settings-dir", 
	 'c', 
	 POPT_ARG_STRING, 
	 &config_source, 	
	 0, 
	 "Use gconf settings file", 
	 "PATH"
	},
	{
	"magnification-zoom", 
	 'z', 
	 POPT_ARG_INT, 
	 &zoom_factor, 	
	 0, 
	 "Set magnification zoom level", 
	 "[1..16]"
	},
	{
	"magnification-invert", 
	 'i', 
	 POPT_ARG_NONE, 
	 &invert, 	
	 0, 
	 "Magnification with reverse video", 
	 ""
	},
	
	{
	"magnification-horizontal-split", 
	 'u', 
	 POPT_ARG_NONE, 
	 &horiz_split, 	
	 0, 
	 "Magnification with horizontal-split", 
	 ""
	},
	
	{
	"login", 
	 'l', 
	 POPT_ARG_NONE, 
	 &login_enabled, 	
	 0, 
	 "Used at login time", 
	 ""
	},
	{
	"magnification-mouse-tracking", 
	 't', 
	 POPT_ARG_STRING, 
	 &mouse_tracking, 	
	 0, 
	 "How the mouse pushed the region", 
	 "[CENTERED, PROPORTION, PUSH, NONE]"
	},

	{
	"magnification-focus-tracking", 
	 'f', 
	 POPT_ARG_STRING, 
	 &focus_tracking, 	
	 0, 
	 "How the focus pushed the region", 
	 "[AUTO, CENTERED, NONE]"
	},
	{
	 NULL, 0,     0, NULL, 0
	}
    };
    
#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, GNOPERNICUSLOCALEDIR);
    textdomain (GETTEXT_PACKAGE);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
#endif    

    fprintf (stderr, "\n**********************");
    fprintf (stderr, "\n* SCREEN READER CORE *");
    fprintf (stderr, "\n**********************\n\n");
        
    gnome_program_init ("srcore", VERSION,
		    LIBGNOMEUI_MODULE,
		    argc, argv,
		    GNOME_PARAM_POPT_TABLE, poptopt,
		    GNOME_PARAM_HUMAN_READABLE_NAME, _("srcore"),
		    LIBGNOMEUI_PARAM_CRASH_DIALOG, FALSE,
		    NULL);
    sru_log_init ();
    
    login_time = FALSE;	
    src_use_braille = (disable_braille ? FALSE :
			(enable_braille ? TRUE : -1));

    src_use_braille_monitor = (disable_braille_monitor ? FALSE :
			      (enable_braille_monitor ? TRUE : -1));
    
    src_use_speech = (disable_speech ? FALSE :
		     (enable_speech ? TRUE : -1));

    src_use_magnifier = (disable_magnifier ? FALSE :
			(enable_magnifier  ? TRUE : -1));

    if (braille_port <= MIN_BRAILLE_PORT && 
	braille_port >= MAX_BRAILLE_PORT)
    {
	braille_port = DEFAULT_BRAILLE_PORT_NO;
    }
    
    if (braille_device)
    {
	gchar *tmp = NULL;
	tmp = g_utf8_strup (braille_device, -1);
	braille_device = tmp;
    }
    
    if (login_enabled)
    {
	login_time = TRUE;
    }

    src_init (mouse_tracking, focus_tracking, config_source, invert,
	      zoom_factor, horiz_split);
    	
    g_free (mouse_tracking);
    g_free (focus_tracking);
    g_free (config_source);


    sru_entry_loop ();

    src_terminate ();
    
    sru_log_terminate ();
    
    return EXIT_SUCCESS;
}

gchar*
src_xml_process_string (gchar *str_)
{
    gint len, i, pos;
    gchar *rv, *crt;
    gchar *str;
    
    if (!str_ || !str_[0])
	return NULL;

    str = src_process_string (str_, SRC_SPEECH_COUNT_AUTO, SRC_SPEECH_SPELL_AUTO);

    if (!str)
	return NULL;    
    
    len = strlen (str);
    /* 6 = maximum lengt of xml_ch din translate table */
    crt = rv = (gchar*) g_malloc ((len * 6 + 1) * sizeof (gchar));
    if (!rv)
	return NULL;
	
    for (i = 0, pos = 0; i < len; ++i)
    {
	static struct
	{
	    gchar ch;
	    gchar *xml_ch;
	}translate[] = {
		    {'<',	"&lt;"	},
		    {'>',	"&gt;"	},
		    {'&',	"&amp;"	},
		    {'\'',	"&apos;"},
		    {'\"',	"&quot;"},
		};
	gint j;
	gboolean special = FALSE;
	
	for (j = 0; j < G_N_ELEMENTS (translate); j++)
	{
	    if (str[i] == translate[j].ch)
	    {
	    	crt = g_stpcpy (crt, translate[j].xml_ch);
		special = TRUE;
	    }
	}
	if (!special)
	{
	    *crt = str[i];
	    crt++;
	}
    }
    *crt = '\0';
    g_free (str);
    return rv;
}		

gchar*
src_xml_make_part (gchar *tag,
	    	   gchar *attr,
	           gchar *text)
{
    if (!tag || !text)
	return NULL;
    return g_strconcat ("<", 
			    tag, 
			    attr ? " " : "", attr ? attr : "",
			">",
			text, 
			"</", tag, ">",
			NULL);
}

gchar*
src_xml_format (gchar *tag,
	    	gchar *attr,
	        gchar *text)
{
    gchar *tmp, *rv;;
    
    
    if (!tag || !text)
	return NULL;
    
    tmp = src_xml_process_string (text);
    rv = src_xml_make_part (tag, attr, tmp);
    g_free (tmp);

    return rv;    
}
