/* libsrconf.c
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
#include "libsrconf.h"
#include <gconf/gconf-client.h>
#include <stdio.h>
#include <string.h>
#include "SRMessages.h"
#include "srintl.h"


/**
 * srconf_fnc:
 *
 * The callback function used for communication with caller, is set during
 * the init process.
 *
**/
static SRConfCB srconf_fnc;

/**
 * SRConfStatusEnum:
 *
 * Define the state of the library
 *    SRCONF_IDLE    - library not initialized
 *    SRCONF_RUNNING - initialized and running
**/
typedef enum {
    SRCONF_IDLE,
    SRCONF_RUNNING
} SRConfStatusEnum;

/**
 * srconf_status:
 *
 * The library status
 *
**/
static SRConfStatusEnum srconf_status = SRCONF_IDLE;

/**
 * gconf_client:
 *
 * the client used to connect to GConf. Gconf is used for setting
 * library parameters.
 *
**/
GConfClient *gconf_client=NULL;
GConfEngine *gconf_engine=NULL;

/**
 * gconf_root_dir_path:
 *
 & the root directory in Gconf
 *
**/
static gchar *gconf_root_dir_path = NULL;

static gboolean use_config_settings = FALSE;
/**
 * DEFAULT_ROOT_DIR_PATH:
 *
 * used when gconf_root_dir_path is not specified in srconf_init
**/
#define DEFAULT_ROOT_DIR_PATH "/apps/gnopernicus"

#define NUM_OF_CONFIGURABLES (10)
/**
 * srconf_notify_directories
 *
 * Configuration directories.
**/
struct _srconf_notify_directories
{
    gchar 		*notify_path;
    SRConfigurablesEnum	 module;
    guint		 cnx_id;
} srconf_notify_directories[NUM_OF_CONFIGURABLES] = 
{
    {"/braille", 				CFGM_BRAILLE,		-1},
    {"/gnopi", 					CFGM_GNOPI,		-1},
    {"/kbd_mouse", 				CFGM_KBD_MOUSE,		-1},
    {"/magnifier", 				CFGM_MAGNIFIER,		-1},
    {"/srcore", 				CFGM_SRCORE,		-1},
    {"/speech/settings",			CFGM_SPEECH,		-1},
    {"/speech/voices_parameters",		CFGM_SPEECH_VOICE_PARAM,-1},
    {"/speech/voice",				CFGM_SPEECH_VOICE,	-1},
    {"/command_map/keyboard/changes_end", 	CFGM_KEY_PAD,		-1},
    {"/presentation/changes_end",		CFGM_PRESENTATION,	-1}
};

/**
 * suspend_notify:
 *
 * Suspend the notifies from gconf, if not 0 notification suspended
 *
**/



static void 
srconf_free_list(gpointer data,gpointer user_data)
{
    g_free((gchar*)data);
    data = NULL;
}

gboolean
srconf_free_slist (GSList *list)
{
    sru_return_val_if_fail (list, FALSE);

    g_slist_foreach(list, srconf_free_list, NULL);
    g_slist_free   (list);
    list = NULL;
    
    return TRUE;
}

/**
 *
 * Convert SRConfigTypesEnum to GConfValueType
 *
**/
GConfValueType 
srconf_convert_SRConfigTypesEnum_to_GConfValueType(SRConfigTypesEnum type)
{
    switch(type)
    {
     case CFGT_INT:
    	return GCONF_VALUE_INT;
	break;
     case CFGT_STRING:
        return GCONF_VALUE_STRING;
	break;
     case CFGT_BOOL:
        return GCONF_VALUE_BOOL;
	break;
    case CFGT_FLOAT:
        return GCONF_VALUE_FLOAT;
	break;
    case CFGT_LIST:
	return GCONF_VALUE_LIST;
	break;
    default:
	return GCONF_VALUE_INVALID;
	break;
    }
}

/**
 *
 * Convert GConfValueType to SRConfigTypesEnum
 *
**/
SRConfigTypesEnum
srconf_convert_GConfValueType_to_SRConfigTypesEnum(GConfValueType type)
{
    switch(type)
    {
     case GCONF_VALUE_INT:
    	return CFGT_INT;
	break;
     case GCONF_VALUE_STRING:
        return CFGT_STRING;
	break;
     case GCONF_VALUE_BOOL:
        return CFGT_BOOL;
	break;
    case GCONF_VALUE_FLOAT:
        return CFGT_FLOAT;
	break;
    case GCONF_VALUE_LIST:
	return CFGT_LIST;
	break;
    default:
	return -1;
	break;
    }
}


/**
 * sr_config_structure_destructor:
 *
 * This function will be used to destroy data allocated in SREvent's data member.
**/
static void 
sr_config_structure_destructor (gpointer data)
{
    SRConfigStructure *config = (SRConfigStructure *)data;

    sru_return_if_fail (config);

    g_free (config->key);
    if (config->type == CFGT_LIST)
    {
	GSList *tmp = NULL;
	for (tmp = (GSList*)config->newvalue; tmp ; tmp = tmp->next)
	{
	    if (tmp->data)
		g_free (tmp->data);
	}
	g_slist_free (config->newvalue);
	config->newvalue = NULL;
    }
    else
	g_free (config->newvalue);
    g_free (config);
    config = NULL;
}

/**
 * sr_config_changed_callback
 *
 * callback for notification of configuration changes
 *
**/
static void 
sr_config_changed_callback (guint cnxn_id, 
			    GConfEntry *entry, 
			    gpointer user_data)
{
    SREvent *evnt = NULL;
    SRConfigStructure *SRstruct = NULL;

    sru_return_if_fail (entry);
    if (!entry->value)
	return;
    
    SRstruct = (SRConfigStructure*) g_new0 (SRConfigStructure, 1);
    sru_assert (SRstruct);

    sru_debug("Entry key:%s", gconf_entry_get_key (entry));
    if (entry->value)
    {
	sru_debug("Entry value type:%i",entry->value->type);
	switch(entry->value->type)
	{	
    	    case GCONF_VALUE_INT:    
		sru_debug("Entry value:%i",gconf_value_get_int(entry->value));
		break;
    	    case GCONF_VALUE_FLOAT:  
		sru_debug("Entry value:%f",gconf_value_get_float(entry->value));
		break;
    	    case GCONF_VALUE_STRING: 
		sru_debug ("Entry value:%s",gconf_value_get_string(entry->value));
		break;
    	    case GCONF_VALUE_BOOL:   
		sru_debug ("Entry value:%i",gconf_value_get_bool(entry->value));
		break;
    	    case GCONF_VALUE_LIST: 
	    	sru_debug ("list");
		break;
	    case GCONF_VALUE_INVALID:
    	    default:
	    	sru_debug ("unset");
		break;
	}
    }

    evnt = sre_new ();
    sru_assert (evnt);
    
    evnt->type = SR_EVENT_CONFIG_CHANGED;
    SRstruct->module = srconf_notify_directories[(gint)user_data].module;
    if (strlen (gconf_entry_get_key (entry)) >
	strlen (gconf_root_dir_path) +
	strlen (srconf_notify_directories[(guint)user_data].notify_path))
    {    
	SRstruct->key = g_strdup (gconf_entry_get_key (entry) +
			    	  strlen (gconf_root_dir_path) +
			          strlen (srconf_notify_directories[(guint)user_data].notify_path) + 1);
    }
    else
	SRstruct->key = g_path_get_basename (gconf_entry_get_key(entry));
	
    if (entry->value)
    {
	switch(entry->value->type)
	{
    	    case GCONF_VALUE_INT:
		SRstruct->type = CFGT_INT;
		SRstruct->newvalue = (gpointer)g_new0 (gint, 1);
		*((gint*)SRstruct->newvalue) = gconf_value_get_int (entry->value);
		break;
    	    case GCONF_VALUE_FLOAT:
		SRstruct->type = CFGT_FLOAT;
		SRstruct->newvalue = (gpointer)g_new0 (gdouble, 1);
		*((gdouble*)SRstruct->newvalue) = gconf_value_get_float (entry->value);
		break;
    	    case GCONF_VALUE_STRING:
		SRstruct->type = CFGT_STRING;
		SRstruct->newvalue = (gpointer)g_strdup (gconf_value_get_string (entry->value));
		break;
    	    case GCONF_VALUE_BOOL:
		SRstruct->type = CFGT_BOOL;
		SRstruct->newvalue = (gpointer)g_new0 (gboolean, 1);
		*((gboolean*)SRstruct->newvalue) = gconf_value_get_bool (entry->value);
		break;
	    case GCONF_VALUE_LIST: 
	    {
		GSList	*iter     = NULL;
		GSList  *new_list = NULL;
		SRConfigTypesEnum type;
		
		type = srconf_convert_GConfValueType_to_SRConfigTypesEnum (gconf_value_get_list_type (entry->value));
		
		if (type != CFGT_STRING)
		    break;
		    		
		for(iter = gconf_value_get_list (entry->value) ; iter ; iter = iter->next)
		{
		    GConfValue *v_data = iter->data;
		    				
		    new_list = g_slist_append (new_list,
						g_strdup((gchar*)gconf_value_get_string(v_data)));
		}
		
		SRstruct->type = CFGT_LIST;
		SRstruct->newvalue = (gpointer)new_list;
		
		break;
	    }
	    case GCONF_VALUE_INVALID:
		break;
	    default:
		break;
	}
    }
    else
    {
	SRstruct->type = CFGT_UNSET;
	SRstruct->newvalue = NULL;
    }
		
    evnt->data = SRstruct;
    evnt->data_destructor = sr_config_structure_destructor;

    if (srconf_fnc) 
        srconf_fnc (evnt,0);

    sre_release_reference (evnt);
    sru_debug ("SendEventSrCore");
}

static void 
sr_config_client_changed_callback (GConfClient *client, 
			    guint cnxn_id, 
			    GConfEntry *entry, 
			    gpointer user_data)
{
    sr_config_changed_callback (cnxn_id, entry, user_data);
}
static void 
sr_config_engine_changed_callback (GConfEngine *engine, 
			    guint cnxn_id, 
			    GConfEntry *entry, 
			    gpointer user_data)
{
    sr_config_changed_callback (cnxn_id, entry, user_data);
}

/**
 * srconf_init:
 *
 * This function initialize Gconf and register the notifications
 *	srconfcb - the callback function used to send data to caller
 *	gconf_root_dir_path - the root directory of GConf
**/
gboolean 
srconf_init (SRConfCB srconfcb, 
	     const gchar *_gconf_root_dir_path,
	     const gchar *config_source)
{
    gint i;
    GError *error = NULL;    
    gchar *path = NULL;
    
    sru_return_val_if_fail (srconf_status == SRCONF_IDLE, FALSE);
    sru_return_val_if_fail (srconfcb != NULL, FALSE);
    
    srconf_fnc = srconfcb;

    gconf_root_dir_path = g_strdup (_gconf_root_dir_path?_gconf_root_dir_path: DEFAULT_ROOT_DIR_PATH);
    
    sru_return_val_if_fail (gconf_root_dir_path != NULL, FALSE);

    /*set a default gconf client */
    if (!config_source)
    {	
	use_config_settings = FALSE;
        gconf_client = gconf_client_get_default ();
	
	/*adding the directory to be watched*/
	gconf_client_add_dir (gconf_client, gconf_root_dir_path, GCONF_CLIENT_PRELOAD_ONELEVEL, &error);
	if (error != NULL)
	{
	    sru_warning(_("Failed to add directory."));
	    sru_warning(_(error->message));
	    g_error_free(error);
	    error = NULL;
	}
	
	for( i=0; i < NUM_OF_CONFIGURABLES; i++ )
	{
    	    path = g_strdup_printf ("%s%s", gconf_root_dir_path, srconf_notify_directories[i].notify_path);
    	    srconf_notify_directories[i].cnx_id = gconf_client_notify_add (gconf_client, path, sr_config_client_changed_callback, (gpointer)(i), NULL, &error);
	
	    if (error != NULL)
	    {
		sru_warning (_("Failed to add notify."));
		sru_warning (_(error->message));
		g_error_free (error);
		error = NULL;
	    }

    	    g_free (path);
    	    path = NULL;
	}

    }
    else
    {
	path = g_strdup_printf ("xml:readwrite:%s", config_source);
    	gconf_engine = gconf_engine_get_for_address (path, &error);
	g_free (path);
	
	use_config_settings = TRUE;
/*      conf = gconf_engine_get_local (config_source, &error);*/
	
	if (gconf_engine == NULL)
	{
    	    sru_assert (error != NULL);
    	    sru_warning(_("Failed to access configuration source(s): %s\n"), error->message);
    	    g_error_free (error);
    	    error = NULL;
    	    return FALSE;
	}
	
	for( i=0; i < NUM_OF_CONFIGURABLES; i++ )
	{
    	    path = g_strdup_printf ("%s%s", gconf_root_dir_path, srconf_notify_directories[i].notify_path);
    	    srconf_notify_directories[i].cnx_id = gconf_engine_notify_add (gconf_engine, path, sr_config_engine_changed_callback, (gpointer)(i),  &error);
	
	    if (error != NULL)
	    {
		sru_warning(_("Failed to add notify: %s"), error->message);
		sru_warning(_(error->message));
		g_error_free (error);
		error = NULL;
	    }	

    	    g_free (path);
    	    path = NULL;
	}
	
	gconf_client = gconf_client_get_for_engine (gconf_engine);
	
	gconf_engine_unref (gconf_engine);
    }
        
    
    srconf_status = SRCONF_RUNNING;
    sru_debug ("SRConf running.");
            
    return TRUE;
}
/**
 * srconf_terminate:
 *
 * Deregister notifications and destroy additional objects
 *
**/
void 
srconf_terminate()
{
    int i;
    GError *error = NULL;
    sru_return_if_fail (srconf_status == SRCONF_RUNNING);
    
    if (!use_config_settings)
    {
	/* removing notification callbacks */
	for( i=0; i < NUM_OF_CONFIGURABLES; i++ )
    	    gconf_client_notify_remove (gconf_client, srconf_notify_directories[i].cnx_id);
    	
	/* remove the watching directory */
	gconf_client_remove_dir (gconf_client, gconf_root_dir_path, &error);
    }
    else
    {
	/* removing notification callbacks */
	for( i=0; i < NUM_OF_CONFIGURABLES; i++ )
    	    gconf_engine_notify_remove (gconf_engine, srconf_notify_directories[i].cnx_id);
	    
	gconf_engine_unref (gconf_engine);
    }
    
    if (error != NULL)
    {
	sru_warning (_("Failed to remove directory."));
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
    }

    g_free (gconf_root_dir_path);
    gconf_root_dir_path = NULL;

    srconf_fnc = NULL;

    srconf_status = SRCONF_IDLE;
    sru_debug ("SRConf idle.");
}



/**
 * srconf_get_config_data_with_default
 *
 * function used to get configuration information
 * 	key - <in>, the name of the configuration information to be get
 * 	conftype - <in>, the type of config data
 *	data - <out>, a pointer to a memory location where srconf_get_config_data_with_default
 *		will put the config data. The CFGT_STRINGS must be freed by user.
 *		At CFGT_LIST type if used default value the returned pointer is same 
 *		with default_data
 *	default_data - returned value if the key not exist
 *	confmodule - <in>, which module's configuration directory to be used
 *	return  - return TRUE if use default value.
**/
gboolean 
srconf_get_config_data_with_default(const gchar *key, SRConfigTypesEnum conftype, gpointer data,gpointer default_data,SRConfigurablesEnum confmodule)
{
    GError 	*error = NULL;
    gchar 	*path = NULL;
    GConfValue	*value = NULL;
    gboolean	used_default;
    
    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (gconf_client != NULL, FALSE);
    sru_return_val_if_fail (srconf_status == SRCONF_RUNNING, FALSE );
    sru_return_val_if_fail ((confmodule >= 1) && (confmodule < NUM_OF_CONFIGURABLES), FALSE);
    
    path = g_strdup_printf("%s%s/%s",gconf_root_dir_path,srconf_notify_directories[confmodule-1].notify_path,key);
    sru_return_val_if_fail (path != NULL, FALSE);
    sru_debug ("srconf_get_config_data_with_default:Path:%s", path);        
    value = gconf_client_get (gconf_client, path, &error);
    
    g_free (path);

    if (value != NULL && error == NULL)
	{
	    if (value->type == srconf_convert_SRConfigTypesEnum_to_GConfValueType(conftype))
	    {
		used_default = TRUE;
		
	        switch(conftype)
		{
		    case CFGT_UNSET:
			break;
		    case CFGT_BOOL:
			*((gboolean *)data) = gconf_value_get_bool(value);
			sru_debug ("srconf_get_config_data_with_default:Data:%s", *((gboolean *)data)?"TRUE":"FALSE");    
			break;
		    case CFGT_INT:
			*((gint *)data) = gconf_value_get_int(value);
			sru_debug ("srconf_get_config_data_with_default:Data:%d", *((gint *)data));    
			break;
		    case CFGT_FLOAT:
			*((gdouble *)data) = gconf_value_get_float(value);
			sru_debug ("srconf_get_config_data_with_default:Data:%lf", *((gdouble *)data));    
			break;
		    case CFGT_STRING:
			(*((gchar **)data)) = g_strdup(gconf_value_get_string(value));
			sru_debug ("srconf_get_config_data_with_default:Data:%s", *((gchar **)data));    
			break;
		    case CFGT_LIST:
			{
			    GSList *iter = NULL;
			    GSList *list = NULL;
			    SRConfigTypesEnum type;
			    			    
			    type = srconf_convert_GConfValueType_to_SRConfigTypesEnum (gconf_value_get_list_type (value));
			    
			    for (iter = gconf_value_get_list (value) ; iter ; iter = iter->next)
			    {
				GConfValue *v_data = iter->data;
				
				if (type == CFGT_STRING)
				{
				    list = g_slist_append (list,
							g_strdup((gchar*)gconf_value_get_string(v_data)));
				}
				else
				    sru_debug ("Unsuported list type");
			    }
			    *((GSList**)data) = list;
			    break;
			}
		}
	    }
	    else
	    {
		*((gpointer*)data) = NULL;
	        used_default = FALSE;
	    }
	    
	    gconf_value_free (value);
	}
	else
	{
	    used_default = TRUE;
	
	    if (default_data)
	    {    
		switch(conftype)
		{
		    case CFGT_STRING:
			(*((gchar **)data)) = g_strdup((gchar*)default_data);
			break;
		    case CFGT_INT:
			*((gint*)data) = *((gint*)default_data);
			break;
		    case CFGT_BOOL:
			*((gboolean*)data) = *((gboolean*)default_data);
			break;
		    case CFGT_FLOAT:
			*((gfloat*)data) = *((gfloat*)default_data);
			break;
		    case CFGT_LIST:
			*((GSList**)data) = (GSList*)default_data; 
			break;
		    case CFGT_UNSET:
			break;
		}
		
		if (!srconf_set_config_data(key, conftype, default_data, confmodule)) 
		    return FALSE;
	    }
	    else
		*((gpointer*)data) = NULL;
	}
    
    return used_default;
}

/**
 * srconf_get_data_with_default:
 *
 * function used to get configuration information
 * 	key - <in>, the name of the configuration information to be get
 * 	conftype - <in>, the type of config data
 *	data - <out>, a pointer to a memory location where srconf_get_data_with_default
 *		will put the config data. The CFGT_STRINGs must be freed by user
 *		At CFGT_LIST type if used default value the returned pointer is same 
 *		with default_data
 *	default_data - returned value if the key is not exist
 *	section - <in>, which section want to write (format Section/Section1)
  *	return  - return TRUE if use default value.
**/
gboolean 
srconf_get_data_with_default(const gchar *key, SRConfigTypesEnum conftype, gpointer data,gpointer default_data , const gchar *section)
{
    GError 	*error = NULL;
    GConfValue	*value = NULL;
    gchar 	*path = NULL;
    gboolean	used_default;
        
    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (section != NULL, FALSE);
    sru_return_val_if_fail (gconf_client != NULL, FALSE);
    sru_return_val_if_fail (srconf_status == SRCONF_RUNNING, FALSE );
    
    path = g_strdup_printf("%s/%s/%s",gconf_root_dir_path,section,key);
    sru_return_val_if_fail (path != NULL, FALSE);
    sru_debug ("srconf_get_data_with_default:Path:%s", path);        
    value = gconf_client_get(gconf_client,path,&error);
    
    g_free(path);
        
    if (value != NULL && error == NULL)
	{
	    if (value->type == srconf_convert_SRConfigTypesEnum_to_GConfValueType(conftype))
	    {
		used_default = TRUE;
		
	        switch(conftype)
		{
		    case CFGT_UNSET:
			break;
		    case CFGT_BOOL:
			*((gboolean *)data) = gconf_value_get_bool(value);
			sru_debug ("srconf_get_data_with_default:Data:%s", *((gboolean *)data)?"TRUE":"FALSE");    
			break;
		    case CFGT_INT:
			*((gint *)data) = gconf_value_get_int(value);
			sru_debug ("srconf_get_data_with_default:Data:%d", *((gint *)data));    
			break;
		    case CFGT_FLOAT:
			*((gdouble *)data) = gconf_value_get_float(value);
			sru_debug ("srconf_get_data_with_default:Data:%lf", *((gdouble *)data));    
			break;
		    case CFGT_STRING:
			(*((gchar **)data)) = g_strdup(gconf_value_get_string(value));
			sru_debug ("srconf_get_data_with_default:Data:%s", *((gchar **)data));    
			break;
		    case CFGT_LIST:
			{
			    GSList *iter = NULL;
			    GSList *list = NULL;
			    SRConfigTypesEnum type;
			    
			    type = srconf_convert_GConfValueType_to_SRConfigTypesEnum (gconf_value_get_list_type (value));
			    
			    for(iter = gconf_value_get_list (value) ; iter ; iter = iter->next)
			    {
				GConfValue *v_data = iter->data;
								
				if (type == CFGT_STRING)
				{
				    list = g_slist_append (list,
							g_strdup((gchar*)gconf_value_get_string(v_data)));
				}
				else
				    sru_debug ("Unsuported list type");
			    }
			    *((GSList**)data) = list;
			    break;
			}
		}
	    }
	    else
	    {
		*((gpointer*)data) = NULL;
	        used_default = FALSE;
	    }
	    
	    gconf_value_free (value);
	}
	else
	{
	
	    used_default = TRUE;
	    if (default_data)
	    {
		switch(conftype)
		{
		    case CFGT_STRING:
			(*((gchar **)data)) = g_strdup((gchar*)default_data);
			break;
		    case CFGT_INT:
			*((gint*)data) = *((gint*)default_data);
			break;
		    case CFGT_BOOL:
			*((gboolean*)data) = *((gboolean*)default_data);
			break;
		    case CFGT_FLOAT:
			*((gfloat*)data) = *((gfloat*)default_data);
			break;
		    case CFGT_LIST:
			*((GSList**)data) = (GSList*)default_data; 
			break;
		    case CFGT_UNSET:
			break;
		}
		
		if (!srconf_set_data(key, conftype, default_data, section)) 
		    return FALSE;
	    }
	    else
		*((gpointer*)data) = NULL;
	}
    
    return used_default;

}

/**
 * srconf_set_config_data:
 *
 * function used to set configuration information
 * 	key - <in>, the name of the configuration information to be set
 * 	conftype - <in>, the type of config data
 *	data - <in>, a pointer to a memory location where srconf_set_config_data
 *		can find the config data. This function do not free anything.
 *	confmodule - <in>, which module's configuration directory to be used
**/
gboolean 
srconf_set_config_data(const gchar *key, SRConfigTypesEnum conftype, const gpointer data,SRConfigurablesEnum confmodule)
{
    GError *error = NULL;
    gchar *path = NULL;
    gboolean ret = TRUE;
        
    sru_return_val_if_fail (srconf_status == SRCONF_RUNNING, FALSE);
    sru_return_val_if_fail ((confmodule >= 1) && (confmodule < NUM_OF_CONFIGURABLES), FALSE);    
    sru_return_val_if_fail (key != NULL, FALSE);
    
    path = g_strdup_printf("%s%s/%s",gconf_root_dir_path,srconf_notify_directories[confmodule-1].notify_path,key);
    sru_return_val_if_fail (path != NULL, FALSE );
    sru_return_val_if_fail (gconf_client_key_is_writable (gconf_client,path,NULL), FALSE);
    switch(conftype)
    {
    case CFGT_BOOL:
	sru_debug ("srconf_set_config_data:Path:%s:Data:%s", path, *((gboolean *)data)?"TRUE":"FALSE");    
	ret = gconf_client_set_bool(gconf_client, path, *((gboolean *)data), &error);
	break;
    case CFGT_INT:
	sru_debug ("srconf_set_config_data:Path:%s:Data:%d", path, *((gint *)data));    
	ret = gconf_client_set_int(gconf_client, path, *((gint *)data), &error);
	break;
    case CFGT_FLOAT:
    	sru_debug ("srconf_set_config_data:Path:%s:Data:%lf", path, *((gdouble *)data));    
	ret = gconf_client_set_float(gconf_client, path, *((gdouble *)data), &error);
	break;
    case CFGT_STRING:
    	sru_debug ("srconf_set_config_data:Path:%s:Data:%s", path, (gchar *)data);    
	ret = gconf_client_set_string(gconf_client, path, (gchar *)data, &error);
	break;
    case CFGT_LIST:
	{
	    GSList *tmp = (GSList*)data;	    
	    sru_debug ("srconf_set_config_data:Path:%s:list length%d",path, g_slist_length(tmp));
	    for ( ; tmp ; tmp = tmp->next)
		sru_debug("Val:%s",(gchar*)tmp->data);

	    ret = gconf_client_set_list(gconf_client, 
					path, 
					GCONF_VALUE_STRING,
					(GSList*)data, 
					&error);
	    
	    break;
	}
    default:
	break;
    }
        
    g_free(path);
    path = NULL;
    
    if (error != NULL)
    {
        sru_warning (_("Failed to set configdata."));
        sru_warning (_(error->message));
        g_error_free (error);
        error = NULL;
        ret = FALSE;
    }

    return ret;
}

/**
 * srconf_set_data:
 *
 * function used to set configuration information
 * 	key - <in>, the name of the configuration information to be set
 * 	conftype - <in>, the type of config data
 *	data - <in>, a pointer to a memory location where srconf_set_config_data
 *		can find the config data. This function do not free anything.
 *	section - <in>, which section want to write (format Section/Section1)
**/
gboolean 
srconf_set_data(const gchar *key, SRConfigTypesEnum conftype, const gpointer data,const gchar *section)
{
    gchar *path = NULL;
    GError *error = NULL;
    gboolean ret = TRUE;
    
    sru_return_val_if_fail (srconf_status == SRCONF_RUNNING, FALSE);
    sru_return_val_if_fail (section != NULL, FALSE);
    sru_return_val_if_fail (key != NULL, FALSE);
    
    path = g_strdup_printf("%s/%s/%s",gconf_root_dir_path,section,key);
    sru_return_val_if_fail (path != NULL, FALSE);
    sru_return_val_if_fail (gconf_client_key_is_writable (gconf_client,path,NULL), FALSE);
    switch(conftype)
    {
    case CFGT_BOOL:
	sru_debug ("srconf_set_data:Path:%s:Data:%s", path, *((gboolean *)data)?"TRUE":"FALSE");    
	ret = gconf_client_set_bool(gconf_client, path, *((gboolean *)data), &error);
	break;
    case CFGT_INT:
    	sru_debug ("srconf_set_data:Path:%s:Data:%d", path, *((gint *)data));    
	ret = gconf_client_set_int(gconf_client, path, *((gint *)data), &error);
	break;
    case CFGT_FLOAT:
    	sru_debug ("srconf_set_data:Path:%s:Data:%lf", path, *((gdouble *)data));    
	ret = gconf_client_set_float(gconf_client, path, *((gdouble *)data), &error);
	break;
    case CFGT_STRING:
    	sru_debug ("srconf_set_data:Path:%s:Data:%s", path, (gchar *)data);    
	ret = gconf_client_set_string(gconf_client, path, (gchar *)data, &error);
	break;
    case CFGT_LIST:
	{
	    GSList *tmp = (GSList*)data;	    
	    sru_debug ("srconf_set_data:Path:%s:list length%d",path, g_slist_length(tmp));
	    for ( ; tmp ; tmp = tmp->next)
		sru_debug("Val:%s",(gchar*)tmp->data);	    

	    ret = gconf_client_set_list(gconf_client, 
					path, 
					GCONF_VALUE_STRING,
					(GSList*)data, 
					&error);
	    break;
	}
    default:break;
    }
        
    g_free(path);
    path = NULL;
    
    if (error != NULL)
    {
        sru_warning (_("Failed set data."));
        sru_warning (_(error->message));
        g_error_free(error);
        error = NULL;
        ret = FALSE;
    }
	
    return ret;
}

/**
 *
 * Unset a specified key.
 * @key - key what it unset
 *
**/
gboolean
srconf_unset_key (const gchar *key,const gchar *section)
{
    GError *error = NULL;
    GConfValue *val = NULL;
    gchar *path = NULL;
    gboolean retval = TRUE;
    
    sru_return_val_if_fail (section != NULL, FALSE);
    sru_return_val_if_fail (key != NULL, FALSE);
    path = g_strdup_printf("%s/%s/%s", gconf_root_dir_path, section,key);
    sru_return_val_if_fail (gconf_client_key_is_writable (gconf_client,path,NULL), FALSE);
    sru_debug ("srconf_unset_key:Path:%s", path);        
    val = gconf_client_get (gconf_client, path, NULL);
    
    if (val)
    {    
	gconf_value_free (val);
	
	retval = gconf_client_unset (gconf_client, path , &error);
        
	if (error != NULL)
	{
	    sru_warning ("Failed unset key.");
	    g_error_free(error);
	    error = NULL;
	}
    }
	
    g_free (path);
    
    return retval;
}

gchar*
srconf_presentationi_get_chunk (const gchar *device_role_event)
{
    gchar *active = NULL;
    gchar *chunk  = NULL;
    
    sru_assert (device_role_event);

    srconf_get_data_with_default(PRESENTATION_ACTIVE_SETTING, CFGT_STRING, &active ,DEFAULT_PRESENTATION_ACTIVE_SETTING_NAME , PRESENTATION_PATH);
    if (active)
    {
	gchar *key = g_strconcat (active, "/", device_role_event, NULL);    
	srconf_get_data_with_default (key, CFGT_STRING, &chunk ,DEFAULT_PRESENTATION_ACTIVE_SETTING_NAME , PRESENTATION_PATH);   
	g_free (key);	
    }

    return chunk;
}
