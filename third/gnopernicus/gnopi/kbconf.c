/* kbconf.c
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
#include "kbconf.h"
#include "kbui.h"
#include "SRMessages.h"
#include "coreconf.h"
#include "gnopiconf.h"
#include "libsrconf.h"
#include "srintl.h"

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

extern Keyboard *keyboard_setting;

#if 0
/**
 * Notify handler						
**/
void		kbconf_changes_cb		(GConfClient	*client,
						guint		cnxn_id,
						GConfEntry	*entry,
						gpointer	user_data);
#endif
/**
 *
**/
static guint kbconf_notify_id;

/**
 *
 * Keyboard setting client listener				
 *
**/
extern GConfClient *gnopernicus_client;


/**
 *
 * Create a new gconf client for keyboard and mouse settings
 * <return> - gconf client
 *
**/
gboolean
kbconf_gconf_client_init (void)
{
/*    GError *error = NULL;*/
	 
    sru_return_val_if_fail (gnopiconf_client_add_dir (CONFIG_PATH KEYBOARD_PATH), FALSE);

#if 0
    kbconf_notify_id = gconf_client_notify_add (gnopernicus_client, 
						CONFIG_PATH KEYBOARD_PATH,
    						kbconf_changes_cb, 
						NULL, NULL, &error);
	    
    if (error != NULL)
    {
	sru_warning (_("Failed to add notify."));
	g_error_free (error);
	error = NULL;
	return FALSE;
    }
#endif
    return TRUE;
}

/**
 *
 * Load value for keyboard setting
 *
**/
Keyboard* 
kbconf_setting_init (gboolean set_struct)
{
    Keyboard *value = NULL;
    
    value = kbconf_setting_new ();

    sru_assert (value);    
    value->take_mouse 	= kbconf_take_mouse_get ();
    value->simulate_click 	= kbconf_simulate_click_get ();
    
    if (!set_struct)
	return value;
	
    kbconf_setting_free (keyboard_setting);
    keyboard_setting = value;
    
    return value;
}

/**
 *
 * Create a new Keyboard structure
 *
**/
Keyboard* 
kbconf_setting_new (void)
{
    Keyboard *new_keyboard = NULL ;
    
    new_keyboard = ( Keyboard * ) g_new0 ( Keyboard, 1);
    
    if (!new_keyboard) 
	sru_error (_("Unable to allocate memory."));
    
    return new_keyboard ;
}

/**
 *
 * Set with default value the param Keyboard setting
 * <value> - braille setting
 *
**/
void 
kbconf_load_default_settings (Keyboard* keyboard)
{
    GConfValue *value = NULL;
    sru_return_if_fail (keyboard != NULL);
    
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SRCORE_KEY_PATH KEYBOARD_TAKE_MOUSE,
						NULL);
    if (value)
    {
    	keyboard->take_mouse = gconf_value_get_bool (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
        keyboard->take_mouse = DEFAULT_KEYBOARD_TAKE_MOUSE;

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH SRCORE_KEY_PATH KEYBOARD_SIMULATE_CLICK,
						NULL);
    if (value)
    {
    	keyboard->simulate_click = gconf_value_get_bool (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	keyboard->simulate_click = DEFAULT_KEYBOARD_SIMULATE_CLICK;
}

/**
 *
 * Free Keyboard structure
 *
**/
void
kbconf_setting_free (Keyboard* keyboard)
{
    if (!keyboard) return;    
    g_free (keyboard);
    keyboard = NULL;
}

/**
 *
 * Terminate Keyboard configure					
 *
**/
void 
kbconf_terminate (Keyboard *keyboard)
{
    kbconf_setting_free (keyboard);
    
    gconf_client_notify_remove (gnopernicus_client, kbconf_notify_id);
    	
    if (gconf_client_dir_exists (gnopernicus_client, CONFIG_PATH KEYBOARD_PATH, NULL))
	gconf_client_remove_dir (gnopernicus_client, CONFIG_PATH KEYBOARD_PATH, NULL);
}


/**
 *
 * Set Methods 							
 *
**/
void    	
kbconf_setting_set (void)
{
    kbconf_take_mouse_set (keyboard_setting->take_mouse);
    kbconf_simulate_click_set (keyboard_setting->simulate_click);
}

void
kbconf_take_mouse_set (gboolean mouse_move)
{
    srcore_take_mouse_set (mouse_move);
}

void
kbconf_simulate_click_set (gboolean simulate)
{
    srcore_simulate_click_set (simulate);
}


/**
 *
 * Get Methods
 *
**/
gboolean
kbconf_take_mouse_get (void)
{
    return srcore_take_mouse_get ();
}

gboolean
kbconf_simulate_click_get (void)
{
    return srcore_simulate_click_get ();
}
/**
 *
 * Keyboard settings callback function
 * <client> - gconf client
 * <cnxn_id> - notify id
 * <entry> - entry structure
 * <user_data> - not used
 *
**/
#if 0
void
kbconf_changes_cb	(GConfClient	*client,
			guint		cnxn_id,
			GConfEntry	*entry,
			gpointer	user_data)
{
	gchar *key = NULL;
	key = (gchar*) gconf_entry_get_key(entry);
	
	sru_debug ("Entry key:%s\n",gconf_entry_get_key(entry));
	sru_debug ("Entry value type:%i\n",entry->value->type);
	switch(entry->value->type)
	{
    	    case GCONF_VALUE_INT:    sru_debug ("Entry value:%i\n",gconf_value_get_int(entry->value));break;
    	    case GCONF_VALUE_FLOAT:  sru_debug ("Entry value:%f\n",gconf_value_get_float(entry->value));break;
    	    case GCONF_VALUE_STRING: sru_debug ("Entry value:%s\n",gconf_value_get_string(entry->value));break;
    	    case GCONF_VALUE_BOOL:   sru_debug ("Entry value:%i\n",gconf_value_get_bool(entry->value));break;
	    default:break;
	}
}
#endif
