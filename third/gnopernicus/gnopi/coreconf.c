/* coreconf.c
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
#include "coreconf.h"
#include "SRMessages.h"
#include "genui.h"
#include "findui.h"
#include "gnopiconf.h"
/*#include "langui.h"*/
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include "srintl.h"
#include "libsrconf.h"

/**
 *
 * Instance of General structure used in gnopi				
 *
**/
extern General *general_setting;

void	srcore_changes_cb	(GConfClient	*client,
				guint		cnxn_id,
				GConfEntry	*entry,
				gpointer	user_data);



gboolean sensitive_list [4] = {TRUE,TRUE,TRUE,TRUE};

/**
 *
 * SRCORE gcon client listener				
 *
**/
extern GConfClient 	*gnopernicus_client;
gint srcore_notify_id;
/**
 *
 * Exit flag
 *
**/
gboolean 	exitackget;

gboolean 	init_finish = FALSE;
/**
 *
 * Create a new gconf client for SRCORE part
 * <return> - gconf client
 *
**/
gboolean
srcore_gconf_client_init (void)
{
    GError *error=NULL;
    
    exitackget = FALSE;
    
    sru_return_val_if_fail (gnopiconf_client_add_dir (CONFIG_PATH SRCORE_PATH), FALSE);

    srcore_notify_id =
	gconf_client_notify_add (gnopernicus_client, CONFIG_PATH SRCORE_PATH,
				srcore_changes_cb, NULL, NULL, &error);
	
    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
	return FALSE;
    }
	    
    return TRUE;
}

/**
 *
 * Load General setting value
 * <return> - general setting
 *
**/
General* 
srcore_general_setting_init (gboolean set_struct)
{
    General *value= NULL;
    
    value = srcore_general_setting_new ();
    
    sru_return_val_if_fail (value, NULL);
    
    value->magnifier		= srcore_magnifier_status_get ();
    value->speech		= srcore_speech_status_get ();
    value->braille   		= srcore_braille_status_get ();
    value->braille_monitor 	= srcore_braille_monitor_status_get ();
    value->minimize		= srcore_minimize_get ();
    
    if (set_struct)
    {
	srcore_general_setting_free (general_setting);
	general_setting = value;
    }
    
    gnopiconf_unset_key (CONFIG_PATH SRCORE_KEY_PATH SRCORE_FIND_TEXT);
    
    return value;
}

/**
 *
 * Create a new General structure
 *
**/
General* 
srcore_general_setting_new (void)
{
    General *new_general = NULL ;
    
    new_general = (General *) g_new0 (General, 1);
    
    if (!new_general) 
	sru_error (_("Unable to allocate memory."));
    
    return new_general ;
}

/**
 *
 * Load default values for general structure					
 *
**/
void 
srcore_load_default_settings (General* general)
{
    GConfValue  *value = NULL;
    
    sru_return_if_fail (general);
    
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SRCORE_KEY_PATH SRCORE_SPEECH_ACTIVE, 
						NULL);
    if (value)
    {
    	general->speech = gconf_value_get_bool (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
        general->speech	= DEFAULT_SRCORE_SPEECH_ACTIVE;
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SRCORE_KEY_PATH SRCORE_MAGNIF_ACTIVE, 
						NULL);
    if (value)
    {
    	general->magnifier = gconf_value_get_bool (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	general->magnifier = DEFAULT_SRCORE_MAGNIF_ACTIVE;
    
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_ACTIVE, 
						NULL);
    if (value)
    {
    	general->braille = gconf_value_get_bool (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	general->braille  = DEFAULT_SRCORE_BRAILLE_ACTIVE;
	
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_MONITOR_ACTIVE, 
						NULL);
    if (value)
    {
    	general->braille_monitor = gconf_value_get_bool (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	general->braille_monitor = DEFAULT_SRCORE_BRAILLE_MONITOR_ACTIVE;

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SRCORE_MINIMIZE_PATH, 
						NULL);
    if (value)
    {
    	general->minimize = gconf_value_get_bool (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	general->minimize = DEFAULT_SRCORE_MINIMIZE;

}

void 
srcore_load_default_screen_review (void)
{
    srcore_screen_review_set (DEFAULT_SRCORE_SCREEN_REVIEW);
}

/**
 *
 * Free General structure
 *
**/
void
srcore_general_setting_free (General* general)
{
    if (!general)
	return;
    g_free (general);
    general = NULL;
}

/**
 *
 * Emite exit signal to sr_core
 *
**/
void
srcore_exit_all (gboolean exit_val)
{
    if (!gnopiconf_set_bool (exit_val, CONFIG_PATH SRCORE_KEY_PATH SRCORE_EXIT_KEY))
    {
	sru_warning (_("Failed to set exit val: %d."), exit_val);
    }
}

/**
 *
 * Terminate SRCORE configure
 *
**/
void
srcore_terminate (General *general)
{
    srcore_general_setting_free (general);

    gconf_client_notify_remove (gnopernicus_client, srcore_notify_id);    
    
    /* remove the watching directory */
    if (gconf_client_dir_exists (gnopernicus_client, CONFIG_PATH SRCORE_PATH, NULL))
	gconf_client_remove_dir (gnopernicus_client, CONFIG_PATH SRCORE_PATH, NULL);
}

/**
 *
 * Set Methods.
 *
**/
void
srcore_general_setting_set (General *general)
{
    srcore_speech_status_set	(general->speech);
    srcore_braille_status_set	(general->braille);
    srcore_braille_monitor_status_set	(general->braille_monitor);
    srcore_magnifier_status_set	(general->magnifier);
    srcore_minimize_set		(general->minimize);
}


void
srcore_take_mouse_set (gboolean mouse_move)
{
    if (!gnopiconf_set_bool (mouse_move, CONFIG_PATH SRCORE_KEY_PATH KEYBOARD_TAKE_MOUSE))
    {
	sru_warning (_("Failed to set move mouse: %d."), mouse_move);
    }
}

void
srcore_simulate_click_set (gboolean simulate)
{
    if (!gnopiconf_set_bool (simulate, CONFIG_PATH SRCORE_KEY_PATH KEYBOARD_SIMULATE_CLICK))
    {
        sru_warning (_("Failed to set simulate click: %d."), simulate);
    }
}

void
srcore_find_text_set (gchar *text)
{
    if (!gnopiconf_set_string (text, CONFIG_PATH SRCORE_KEY_PATH SRCORE_FIND_TEXT))
    {
	sru_warning (_("Failed to set find text: %s."), text);
    }
}


void
srcore_magnifier_status_set (gboolean status)
{
    if (!gnopiconf_set_bool (status, CONFIG_PATH SRCORE_KEY_PATH SRCORE_MAGNIF_ACTIVE))
    {
        sru_warning (_("Failed to set magnifier status: %d."), status);
    }
}

void
srcore_speech_status_set (gboolean status)
{
    if (!gnopiconf_set_bool (status, CONFIG_PATH SRCORE_KEY_PATH SRCORE_SPEECH_ACTIVE))
    {
	sru_warning (_("Failed to set speech status: %d."), status);
    }
}

void
srcore_braille_status_set (gboolean status)
{
    if (!gnopiconf_set_bool (status, CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_ACTIVE))
    {
	sru_warning (_("Failed to set braille status: %d."), status);
    }
}

void
srcore_braille_monitor_status_set (gboolean status)
{
    if (!gnopiconf_set_bool (status, CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_MONITOR_ACTIVE))
    {
	sru_warning (_("Failed to set braille monitor status: %d."), status);
    }
}

void
srcore_minimize_set (gboolean minimize)
{
    if (!gnopiconf_set_bool (minimize, CONFIG_PATH SRCORE_MINIMIZE_PATH))
    {
	sru_warning (_("Failed to set minimize flag: %d."), minimize);
    }
}

void
srcore_exit_ack_set (gboolean ack)
{
    if (!gnopiconf_set_bool (ack, CONFIG_PATH SRCORE_KEY_PATH SRCORE_EXIT_ACK_KEY))
    {
	sru_warning (_("Failed to set exit ack: %d."), ack);
    }
}

void
srcore_language_set (gchar *id)
{
    if (!gnopiconf_set_string (id, CONFIG_PATH SRCORE_KEY_PATH SRCORE_LANGUAGE))
    {
	sru_warning (_("Failed to set language: %s."), id);
    }
}

void
srcore_screen_review_set (gint flags)
{
    if (!gnopiconf_set_int (flags, CONFIG_PATH SRCORE_KEY_PATH SRCORE_SCREEN_REVIEW))
    {
	sru_warning (_("Failed to set screen review: %d."), flags);
    }
}



/**
 *
 * Get Methods.
 *
**/
gchar*
srcore_find_text_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SRCORE_KEY_PATH SRCORE_FIND_TEXT, 
					    DEFAULT_SRCORE_FIND_TEXT);
}

gboolean
srcore_take_mouse_get (void)
{
    return gnopiconf_get_bool_with_default (CONFIG_PATH SRCORE_KEY_PATH KEYBOARD_TAKE_MOUSE, 
					DEFAULT_KEYBOARD_TAKE_MOUSE);
}

gboolean
srcore_simulate_click_get (void)
{
    return gnopiconf_get_bool_with_default (CONFIG_PATH SRCORE_KEY_PATH KEYBOARD_SIMULATE_CLICK, 
					DEFAULT_KEYBOARD_SIMULATE_CLICK);
}


gboolean
srcore_magnifier_status_get (void)
{
    return gnopiconf_get_bool_with_default (CONFIG_PATH SRCORE_KEY_PATH SRCORE_MAGNIF_ACTIVE, 
					DEFAULT_SRCORE_MAGNIF_ACTIVE);
}

gboolean
srcore_speech_status_get (void)
{
    return gnopiconf_get_bool_with_default (CONFIG_PATH SRCORE_KEY_PATH SRCORE_SPEECH_ACTIVE, 
					DEFAULT_SRCORE_SPEECH_ACTIVE);
}

gboolean
srcore_braille_status_get (void)
{
    return gnopiconf_get_bool_with_default (CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_ACTIVE, 
					DEFAULT_SRCORE_BRAILLE_ACTIVE);
}

gboolean
srcore_braille_monitor_status_get (void)
{
    return gnopiconf_get_bool_with_default (CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_MONITOR_ACTIVE, 
					DEFAULT_SRCORE_BRAILLE_MONITOR_ACTIVE);
}

gboolean
srcore_minimize_get (void)
{
    return gnopiconf_get_bool_with_default (CONFIG_PATH SRCORE_MINIMIZE_PATH, 
					DEFAULT_SRCORE_MINIMIZE);
}

gchar*		
srcore_language_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SRCORE_KEY_PATH SRCORE_LANGUAGE, 
					    DEFAULT_SRCORE_LANGUAGE);
}

gint		
srcore_screen_review_get (void)
{
    return gnopiconf_get_int_with_default (CONFIG_PATH SRCORE_KEY_PATH SRCORE_SCREEN_REVIEW, 
					    DEFAULT_SRCORE_SCREEN_REVIEW);
}

gchar*		
srcore_ip_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH SRCORE_BRLMON_IP, 
					    DEFAULT_BRLMON_IP);
}

/**
 *
 * SRCore callback function
 * <client> - gconf client
 * <cnxn_id> - notify id
 * <entry> - entry structure
 * <user_data> - not used
 *
**/
void
srcore_changes_cb (GConfClient	*client,
		   guint	cnxn_id,
		   GConfEntry	*entry,
		   gpointer	user_data)
{
    gchar *key = NULL;
    
    sru_return_if_fail (entry);
    if (!entry->value || !general_setting)
	return;
    
    key = (gchar*) gconf_entry_get_key (entry);
    
    if (!strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_EXIT_ACK_KEY))
    {
	exitackget = gconf_value_get_bool (entry->value);
	return;
    }    
    
    if (!strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_INITIALIZATION_END))
    {
	init_finish = TRUE;
	return;
    }    
    
    
    sru_debug ("coreconf:srcore_changes_cb:Entry key:%s",gconf_entry_get_key(entry));
    sru_debug ("coreconf:srcore_changes_cb:Entry value type:%i",entry->value->type);
    switch(entry->value->type)
    {
    	    case GCONF_VALUE_INT:    sru_debug ("coreconf:srcore_changes_cb:Entry value:%i",gconf_value_get_int(entry->value));break;
    	    case GCONF_VALUE_FLOAT:  sru_debug ("coreconf:srcore_changes_cb:Entry value:%f",gconf_value_get_float(entry->value));break;
    	    case GCONF_VALUE_STRING: sru_debug ("coreconf:srcore_changes_cb:Entry value:%s",gconf_value_get_string(entry->value));break;
    	    case GCONF_VALUE_BOOL:   sru_debug ("coreconf:srcore_changes_cb:Entry value:%i",gconf_value_get_bool(entry->value));break;
	    default: sru_debug ("coreconf:srcore_changes_cb:Other value");break;
    }

	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_MONITOR_SENSITIVE) == 0)
	{ 
	    if (entry->value->type == GCONF_VALUE_BOOL)
	    {
		if (sensitive_list [BRAILLE_MONITOR] != gconf_value_get_bool (entry->value))
		{
		    sensitive_list [BRAILLE_MONITOR] = gconf_value_get_bool (entry->value);
		    genui_set_sensitive_for (BRAILLE_MONITOR, sensitive_list [BRAILLE_MONITOR]);
		}
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_SENSITIVE) == 0)
	{ 
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_BOOL)
	    {
	    
		if (sensitive_list [BRAILLE] != gconf_value_get_bool (entry->value))
		{
		    sensitive_list [BRAILLE] = gconf_value_get_bool (entry->value);
		    genui_set_sensitive_for (BRAILLE, sensitive_list [BRAILLE]);
		}
	    }
	    }
	    else
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_SPEECH_SENSITIVE) == 0)
	{		
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_BOOL)
	    {
		if (sensitive_list [SPEECH] != gconf_value_get_bool (entry->value))
		{
		    sensitive_list [SPEECH] = gconf_value_get_bool (entry->value);
		    genui_set_sensitive_for (SPEECH, sensitive_list [SPEECH]);
		}
	    }
	    }
	else
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_MAGNIF_SENSITIVE) == 0)
	{
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_BOOL)
	    {
		if (sensitive_list [MAGNIFIER] != gconf_value_get_bool (entry->value))
		{
		    sensitive_list [MAGNIFIER] = gconf_value_get_bool (entry->value);
		    genui_set_sensitive_for (MAGNIFIER, sensitive_list [MAGNIFIER]);
		}
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_UI_COMMAND) == 0)
	{
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_STRING)
	    {
		gchar *command;
	        command = g_strdup ((gchar*) gconf_value_get_string (entry->value));
		if (!strcmp (command, "find_set"))
		    fnui_load_find ();
		g_free (command);
	    }
	}
	else	
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_MAGNIF_ACTIVE) == 0)
	{
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_BOOL)
	    {
		general_setting->magnifier = gconf_value_get_bool (entry->value);
		genui_value_add_to_widgets (general_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_SPEECH_ACTIVE) == 0)
	{
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_BOOL)
	    {
	    	general_setting->speech = gconf_value_get_bool (entry->value);
		genui_value_add_to_widgets (general_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_ACTIVE) == 0)
	{
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_BOOL)
	    {
	    	general_setting->braille = gconf_value_get_bool (entry->value);
		genui_value_add_to_widgets (general_setting);
	    }
	}
	else
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_BRAILLE_MONITOR_ACTIVE) == 0)
	{
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_BOOL)
	    {
	    	general_setting->braille_monitor = gconf_value_get_bool (entry->value);
		genui_value_add_to_widgets (general_setting);
	    }
	}


/*
    	else
	if (strcmp (key, CONFIG_PATH SRCORE_KEY_PATH SRCORE_LANGUAGE) == 0)
	{
	    if (entry && entry->value && entry->value->type == GCONF_VALUE_STRING)
	    {
		gchar *value;
	        value = g_strdup ((gchar*) gconf_value_get_string (entry->value));
		lng_set_envirom  (value);
		g_free (value);
	    }
	}
*/
}


