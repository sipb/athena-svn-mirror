/* brlconf.c
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
#include "brlconf.h"
#include "brlui.h"
#include "SRMessages.h"
#include "gnopiconf.h"
#include "cmdmapconf.h"

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>
#include "srintl.h"

#define BRL_INVALID_DEV_INDEX 	-1

extern GSList *braille_keys_list;

/**
 *
 * Braille setting gconf client listener				
 *
**/
extern GConfClient *gnopernicus_client;

/**
 *
 * Braille config setting
 *
**/
extern Braille 	*braille_setting;
gint     br_notify_id;
GList 	*brldev_id_list = NULL;
GList	*brldev_description_list = NULL;	

void	brlconf_changes_cb		(GConfClient	*client,
					guint		cnxn_id,
					GConfEntry	*entry,
					gpointer	user_data);


CmdFctType cmd_brl_cmds[]=
{
	{"DK00",   {"goto parent", 		NULL}},	
	{"DK02",   {"goto child", 		NULL}},
	{"DK03",   {"goto previous", 		NULL}},	
	{"DK05",   {"goto next", 		NULL}},	
	{"DK04",   {"repeat last", 		NULL}},	
	{"DK01",   {"goto focus", 		NULL}},
	{"DK01DK02",{"do default action", 	NULL}},
	{NULL,	   {NULL}}
};

/**
 *
 * Create a new gconf client for braille setting
 * <return> - gconf client
 *
**/
gboolean
brlconf_gconf_client_init (void)
{
    GError *error = NULL;
    
    sru_return_val_if_fail (gnopiconf_client_add_dir (CONFIG_PATH BRAILLE_PATH), FALSE);
    
    br_notify_id = gconf_client_notify_add (gnopernicus_client,
					    CONFIG_PATH BRAILLE_PATH,
					    brlconf_changes_cb,
					    NULL,NULL,&error);
    if (error != NULL)
    {
        sru_warning ("Failed to add notify.");
	sru_warning (error->message);
        g_error_free (error);
        error = NULL;
        return FALSE;
    }


    	
    return TRUE;
}

/**
 *
 * Load braille config setting
 * <return> - braille setting
 *
**/
Braille* 
brlconf_setting_init (gboolean set_struct)
{
    Braille *value= NULL;
    value = brlconf_setting_new ();

    sru_assert (value);
    
    value->braille_style	= brlconf_style_get ();
    value->cursor_style 	= brlconf_cursor_style_get ();
    value->attributes   	= brlconf_attributes_get ();
    value->translation_table	= brlconf_translation_table_get ();
    value->port_no		= brlconf_port_no_get ();
    value->status_cell		= brlconf_status_cell_get ();
    value->optical	 	= brlconf_optical_sensor_get ();
    value->position	 	= brlconf_position_sensor_get ();
    value->fill_char		= brlconf_fill_char_get ();
    
    cmdconf_set_defaults_for_table (CMDMAP_BRAILLE_KEYS_LIST_PATH, cmd_brl_cmds, FALSE);
    cmdconf_changes_end_event 	();
    
    if (!set_struct)
	return value;
	
    brlconf_setting_free (braille_setting);
    braille_setting = value;
    
    return value;
}

/**
 *
 * Set with default value the param Braille setting
 * <value> - braille setting
 *
**/
void 
brlconf_load_default_settings (Braille* braille)
{
    GConfValue  *value = NULL;
    
    sru_return_if_fail (braille);    
    
    brlconf_setting_clean (braille);
    
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_STYLE, 
						NULL);
    
    if (value)
    {
    	braille->braille_style = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->braille_style 	= g_strdup (DEFAULT_BRAILLE_STYLE);
	
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_CURSOR_STYLE, 
						NULL);
    
    if (value)
    {
    	braille->cursor_style = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->cursor_style = g_strdup (DEFAULT_BRAILLE_CURSOR_STYLE);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_ATTRIBUTES, 
						NULL);    
    if (value)
    {
    	braille->attributes = gconf_value_get_int (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->attributes = DEFAULT_BRAILLE_ATTRIBUTES;
	
    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_OPTICAL_SENSOR, 
						NULL);
    if (value)
    {
    	braille->optical = gconf_value_get_int (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->optical = DEFAULT_BRAILLE_OPTICAL_SENSOR;

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_POSITION_SENSOR, 
						NULL);
    if (value)
    {
    	braille->position = gconf_value_get_int (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->position = DEFAULT_BRAILLE_POSITION_SENSOR;

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_PORT_NO, 
						NULL);
    if (value)
    {
    	braille->port_no = gconf_value_get_int (value);
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->port_no = DEFAULT_BRAILLE_PORT_NO;

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_TRANSLATION, 
						NULL);
    if (value)
    {
    	braille->translation_table = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->translation_table = g_strdup (DEFAULT_BRAILLE_TRANSLATION);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_FILL_CHAR, 
						NULL);
    if (value)
    {
    	braille->fill_char = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->fill_char = g_strdup (DEFAULT_BRAILLE_FILL_CHAR);

    value = gconf_client_get_default_from_schema (gnopernicus_client, 
						CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_STATUS_CELL, 
						NULL);
    if (value)
    {
    	braille->status_cell = g_strdup (gconf_value_get_string (value));
	gconf_value_free (value);
	value = NULL;
    }
    else
	braille->status_cell = g_strdup (DEFAULT_BRAILLE_STATUS_CELL);
	
    cmdconf_set_defaults_for_table (CMDMAP_BRAILLE_KEYS_LIST_PATH, cmd_brl_cmds, TRUE);
    cmdconf_changes_end_event 	();
}

/**
 *
 * Create a new Braille structure
 *
**/
Braille* 
brlconf_setting_new (void)
{
    Braille *new_braille = NULL ;
    
    new_braille = ( Braille * ) g_new0 ( Braille , 1);
    
    if (!new_braille)	
	sru_error (_("Unable to allocate memory."));
    
    return new_braille ;
}

/**
 *
 * Clean BRAILLE structure
 * <braille> - braille structure
 *
**/
void 
brlconf_setting_clean (Braille* braille)
{
    sru_return_if_fail (braille);    
    
    g_free (braille->status_cell);
    g_free (braille->translation_table);
    g_free (braille->fill_char);
    g_free (braille->cursor_style);
    g_free (braille->braille_style);
    braille->status_cell = NULL;
    braille->translation_table = NULL;
    braille->fill_char = NULL;
}

/**
 *
 * Free BRAILLE structure
 * <braille> - braille structure
 *
**/
void 
brlconf_setting_free (Braille* braille)
{
     if (!braille)
        return;
    
    brlconf_setting_clean (braille);    
    g_free (braille);
    braille = NULL;
}

void
brlconf_device_list_free (void)
{
    GList *elem	= NULL;
    
    for (elem	= brldev_id_list; elem ; elem = elem->next)
	g_free (elem->data);
	
    g_list_free (brldev_id_list);
    brldev_id_list = NULL;
    
    for (elem = brldev_description_list; elem ; elem = elem->next)
	g_free (elem->data);
	
    g_list_free (brldev_description_list);
    brldev_description_list = NULL;
}

/**
 *
 * Terminate Braille configure. Free Braille setting. Remove listeners from 
 * directory.
 * <braille> - braille structure
 *
**/
void 
brlconf_terminate (Braille *braille)
{
    brlconf_setting_free (braille);
    brlconf_device_list_free ();
    braille_setting = NULL;
    
    gconf_client_notify_remove (gnopernicus_client, br_notify_id);
    
    if (gconf_client_dir_exists (gnopernicus_client, CONFIG_PATH BRAILLE_PATH, NULL) == TRUE)
	gconf_client_remove_dir (gnopernicus_client, CONFIG_PATH BRAILLE_PATH, NULL);
}


/**
 *
 * Set Methods
 *
**/
void 
brlconf_setting_set (const Braille *braille)
{
    brlconf_cursor_style_set	(braille->cursor_style);
    brlconf_port_no_set		(braille->port_no);
    brlconf_attributes_set	(braille->attributes);
    brlconf_style_set		(braille->braille_style);
    brlconf_translation_table_set(braille->translation_table);
    brlconf_optical_sensor_set 	(braille->optical);
    brlconf_position_sensor_set (braille->position);
    brlconf_fill_char_set 	(braille->fill_char);
    brlconf_status_cell_set 	(braille->status_cell);
}

void 
brlconf_style_set (const gchar *style)
{
    if (!gnopiconf_set_string (style, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_STYLE))
    {
	sru_warning (_("Failed to set braille style: %s."), style);
    }
}

void 
brlconf_cursor_style_set (const gchar *cursor)
{
    if (!gnopiconf_set_string (cursor, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_CURSOR_STYLE))
    {
        sru_warning (_("Failed to set braille cursor style: %s."), cursor);
    }
}

void 
brlconf_attributes_set (gint dots)
{
    if (!gnopiconf_set_int (dots, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_ATTRIBUTES))
    {
	sru_warning (_("Failed to set attribute settings: %d."), dots);
    }
}

void 
brlconf_device_set (const gchar *device)
{
    if (!gnopiconf_set_string (device, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_DEVICE))
    {
        sru_warning (_("Failed to set braille device: %s."), device);
    }
}

void 
brlconf_fill_char_set (const gchar *texture)
{
    if (!gnopiconf_set_string (texture, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_FILL_CHAR))
    {
        sru_warning (_("Failed to set fill character: %s."), texture);
    }
}

void
brlconf_port_no_set (gint port_no)
{
    if (!gnopiconf_set_int (port_no, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_PORT_NO))
    {
        sru_warning (_("Failed to set serial port number: %d."), port_no);
    }
}


void 
brlconf_translation_table_set (const gchar *table)
{
    if (!gnopiconf_set_string (table, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_TRANSLATION))
    {
        sru_warning (_("Failed to set translation table: %s."), table);
    }
}

void 
brlconf_optical_sensor_set (gint sensor)
{
    if (!gnopiconf_set_int (sensor, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_OPTICAL_SENSOR))
    {
        sru_warning (_("Failed to set optical sensor: %d."), sensor);
    }
}

void 
brlconf_position_sensor_set (gint sensor)
{
    if (!gnopiconf_set_int (sensor, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_POSITION_SENSOR))
    {
        sru_warning (_("Failed to set position sensor: %d."), sensor);
    }
}

void 
brlconf_status_cell_set (const gchar *status)
{
    if (!gnopiconf_set_string (status, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_STATUS_CELL))
    {
        sru_warning (_("Failed to set status cell: %s."), status);
    }
}


/**
 *
 * Get Methods
 *
**/
gchar* 
brlconf_style_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_STYLE, 
					   DEFAULT_BRAILLE_STYLE);
}

gchar* 
brlconf_cursor_style_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_CURSOR_STYLE, 
					      DEFAULT_BRAILLE_CURSOR_STYLE);
}

gchar* 
brlconf_translation_table_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_TRANSLATION, 
					    DEFAULT_BRAILLE_TRANSLATION);
}

gint 
brlconf_attributes_get (void)
{
    return gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_ATTRIBUTES, 
					DEFAULT_BRAILLE_ATTRIBUTES);
}

gchar* 
brlconf_device_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_DEVICE, 
					    DEFAULT_BRAILLE_DEVICE);
}

void
brlconf_active_device_list_get (void)
{

    gint count;
    count = gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_DEVICE_COUNT,
					    DEFAULT_BRAILLE_DEVICE_COUNT);
    if (count > 0)
    {
	gint iter;
	for (iter = 0; iter < count ; iter++)
	{
	    gchar *path;	
	    gchar *id = NULL;
	    gchar *descrip = NULL;
	    path    = g_strdup_printf ("%s%sbrldev_%d_ID", CONFIG_PATH, BRAILLE_KEY_PATH, iter);
	    id 	= gnopiconf_get_string_with_default (path, NULL);
	    g_free (path);
	    path    = g_strdup_printf ("%s%sbrldev_%d_description", CONFIG_PATH, BRAILLE_KEY_PATH, iter);
	    descrip = gnopiconf_get_string_with_default (path, NULL);
	    g_free (path);
	
	    if (id && descrip)
	    {
		brldev_id_list = 
		    g_list_append (brldev_id_list, id);
		brldev_description_list = 
		    g_list_append (brldev_description_list, descrip);
	    }
	    else
	    {
		g_free (id);
		g_free (descrip);
	    }
	}
    }
    else
    {
	brldev_id_list = 
	    g_list_append (brldev_id_list, g_strdup(""));
	brldev_description_list = 
	    g_list_append (brldev_description_list, g_strdup (_("<none>")));
    }
}

gchar* 
brlconf_fill_char_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_FILL_CHAR, 
					    DEFAULT_BRAILLE_FILL_CHAR);
}

gint 
brlconf_port_no_get (void)
{
    return gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_PORT_NO, 
					DEFAULT_BRAILLE_OPTICAL_SENSOR);
}

gint 
brlconf_optical_sensor_get (void)
{
    return gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_OPTICAL_SENSOR, 
					DEFAULT_BRAILLE_PORT_NO );
}

gint 
brlconf_position_sensor_get (void)
{
    return gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_POSITION_SENSOR, 
					DEFAULT_BRAILLE_POSITION_SENSOR);
}

gchar* 
brlconf_status_cell_get (void)
{
    return gnopiconf_get_string_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_STATUS_CELL, 
					    DEFAULT_BRAILLE_STATUS_CELL);
}

static gint
brlconf_default_device_get (void)
{
    return gnopiconf_get_int_with_default (CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_DEFAULT_DEVICE, 
					    DEFAULT_BRAILLE_DEVICE_INDEX);
}

const gchar*
brlconf_device_id_return (const gchar *device)
{
    GList *crt = NULL;
    gint index = 0;
    
    sru_return_val_if_fail (device, NULL);    
    sru_return_val_if_fail ((brldev_id_list && brldev_description_list), NULL);    
	
    for (crt = brldev_description_list, index = 0 ; crt ; crt = crt->next , index++)
	if (!strcmp (device, (gchar*)crt->data)) 
	    break;
	
    if (!crt)
	index = BRL_INVALID_DEV_INDEX;
    if (index > BRL_INVALID_DEV_INDEX)
	return g_list_nth_data (brldev_id_list, index);
	
    return NULL;
}

const gchar*
brlconf_return_active_device (void)
{
    gchar *device = brlconf_device_get ();
    
    if (device && 
	brldev_id_list && 
	brldev_description_list)
    {
	GList *crt = NULL;
	gint index = 0;
	
	for (crt = brldev_id_list, index = 0 ; crt ; crt = crt->next , index++)
	    if (!strcmp (device, (gchar*)crt->data)) break;
	
	if (!crt)
	    index = BRL_INVALID_DEV_INDEX;
	    
	if (index > BRL_INVALID_DEV_INDEX)
	    return g_list_nth_data (brldev_description_list, index);
	else
	{
	    brlconf_device_set (g_list_nth_data (brldev_id_list, 
				brlconf_default_device_get ())
				);
	    return g_list_nth_data (brldev_description_list, 
				 brlconf_default_device_get ());
	}
    }
    if (brldev_id_list)
    {
        brlconf_device_set (g_list_nth_data (brldev_id_list, 
			    brlconf_default_device_get ())
			    );
	return g_list_nth_data (brldev_description_list, 
				brlconf_default_device_get ());
    }
    return NULL;
}



void
brlconf_changes_cb			(GConfClient	*client,
					guint		cnxn_id,
					GConfEntry	*entry,
					gpointer	user_data)
{    
    gchar *key = NULL;
    
    if (!entry || 
	!entry->value)
	return;
	    
    sru_debug  ("spconf:spconf_changes_cb:Entry value type:%i",entry->value->type);
    switch (entry->value->type)
    {
    	case GCONF_VALUE_INT:    sru_debug ("spconf:spconf_changes_cb:Entry value:%in",gconf_value_get_int(entry->value));break;
    	case GCONF_VALUE_FLOAT:  sru_debug ("spconf:spconf_changes_cb:Entry value:%f",gconf_value_get_float(entry->value));break;
    	case GCONF_VALUE_STRING: sru_debug ("spconf:spconf_changes_cb:Entry value:%s",gconf_value_get_string(entry->value));break;
    	case GCONF_VALUE_BOOL:   sru_debug ("spconf:spconf_changes_cb:Entry value:%i",gconf_value_get_bool(entry->value));break;
	default:break;
    }

    key = (gchar*) gconf_entry_get_key (entry);
#if 0
    if (strcmp (key, CONFIG_PATH BRAILLE_KEY_PATH BRAILLE_TRANSLATION) == 0)
    {
        if (entry->value->type == GCONF_VALUE_STRING)
        {
	    if (!strcmp (braille_setting->translation_table, 
		gconf_value_get_string(entry->value)))
		return;
		
	    g_free (braille_setting->translation_table);	
	    braille_setting->translation_table = g_strdup ((gchar*) gconf_value_get_string (entry->value));
        }
    }
    else
	if (strcmp (key, BRAILLE_KEY_PATH BRAILLE_PORT_NO) == 0)
	{
	    if (entry->value->type == GCONF_VALUE_INT)
	    {
		braille_setting->port_no = gconf_value_get_int (entry->value);
		brlui_braille_device_value_add_to_widgets (braille_setting);
	    }
	}
#endif
}
