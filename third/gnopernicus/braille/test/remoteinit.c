/* remoteinit.c
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

#include "remoteinit.h"

#define REMOTE_CONFIG_PATH "/apps/gnopernicus/remote"

GConfClient *brlrm_client;

gboolean 
brl_gconf_client_init()
{
    GError *error = NULL;
    brlrm_client = gconf_client_get_default();
     
    if (brlrm_client == NULL) 
    {
	g_error("Failed to init GConf client:\n");
	return FALSE;
     }
        	
    gconf_client_add_dir(brlrm_client,
	                 REMOTE_CONFIG_PATH,GCONF_CLIENT_PRELOAD_NONE,&error);

    if (error != NULL)
    {
	g_error("Failed to add directory:\n");
	g_error_free(error);
	error = NULL;
	brlrm_client = NULL;
	return FALSE;
    }
		
    return TRUE;
}

static gboolean
brl_check_type(const gchar*   key, 
	       GConfValue*    val, 
	       GConfValueType t, 
	       GError**       err)
{
    if (val->type != t)
    {
	g_set_error (err, GCONF_ERROR, GCONF_ERROR_TYPE_MISMATCH,
	  	     "Expected key: %s",key);
	return FALSE;
    }
    else
	return TRUE;
}


gboolean
brl_set_int(gint        val,
	    const gchar *key)
{
    GError *error = NULL;
    gboolean  ret = TRUE;
    gchar *path   = NULL;
    
    g_return_val_if_fail ( key != NULL, FALSE );
    g_return_val_if_fail ( brlrm_client != NULL, FALSE );
    
    path = gconf_concat_dir_and_key(REMOTE_CONFIG_PATH,key);
    
    g_return_val_if_fail ( gconf_client_key_is_writable (brlrm_client,path,NULL), FALSE);

    ret  = gconf_client_set_int(brlrm_client,path,val,&error);

    if (error != NULL)
    {
	g_warning("Remote:Failed to set value:%d\n",val);
	g_warning("Recommended to delete ~/.gconf/apps/gnopernicus directories");
	g_error_free(error);
	error = NULL;
    }
    
    g_free(path);    
    
    return ret;
}

gboolean
brl_set_string(const gchar *val,
	       const gchar *key)
{
    GError *error = NULL;
    gboolean ret  = TRUE;
    gchar *path   = NULL;
    
    g_return_val_if_fail ( val != NULL, FALSE );
    g_return_val_if_fail ( key != NULL, FALSE );
    g_return_val_if_fail ( brlrm_client != NULL, FALSE );
    
    path = gconf_concat_dir_and_key(REMOTE_CONFIG_PATH,key);
    
    g_return_val_if_fail ( gconf_client_key_is_writable (brlrm_client,path,NULL), FALSE);
        
    ret = gconf_client_set_string(brlrm_client,path,val,&error);

    if (error != NULL)
    {
	g_warning("Remote:Failed to set value:%s\n",val);
	g_warning("Recommended to delete ~/.gconf/apps/gnopernicus directories");
	g_error_free(error);
	error = NULL;
    }
        
    g_free(path);
    
    return ret;
}

/**
 * Get Methods
**/
gint 
brl_get_int_with_default(const gchar *key,
                         gint        default_value)
{
    GError *error = NULL;
    GConfValue *value = NULL;
    gint ret_val;
    gchar *path = NULL;
    
    g_return_val_if_fail ( key != NULL, default_value );
    g_return_val_if_fail ( brlrm_client != NULL, default_value );
    
    path = gconf_concat_dir_and_key(REMOTE_CONFIG_PATH,key);
    
    value = gconf_client_get(brlrm_client,path,&error);
    
    ret_val = default_value;
    
    if (value != NULL && error == NULL)
    {	
	if (brl_check_type (key, value, GCONF_VALUE_INT, &error))
    	    ret_val = gconf_value_get_int(value);
        else
	{
    	    g_warning("Remote:Invalid type of key %s\n",key);
	    g_warning("Recommended to delete ~/.gconf/apps/gnopernicus directories");
	    if (!brl_set_int(ret_val,key))
		g_warning("Remote:Failed to set value %d\n",ret_val);
	}
	    
	gconf_value_free (value);
		
	g_free(path);
	
        return ret_val;
    }
    else
    {
	if (error != NULL)
	{
	    g_warning("Remote:Failed to get value %s\n",key);
	    g_warning("Recommended to delete ~/.gconf/apps/gnopernicus directories");
	}
	    
	if (!brl_set_int(default_value,key))
	    g_warning("Remote:Failed to set value %d\n",default_value);
	
	g_free(path);
		
        return default_value;
    }    
}

gchar* 
brl_get_string_with_default(const gchar *key,
			    gchar       *default_value)
{
    GError *error = NULL;
    gchar *retval = NULL;
    gchar *path   = NULL;
    
    g_return_val_if_fail ( key != NULL, default_value );
    g_return_val_if_fail ( brlrm_client != NULL, default_value );
    
    path = gconf_concat_dir_and_key(REMOTE_CONFIG_PATH,key);

    retval = gconf_client_get_string(brlrm_client,path,&error);

    if (retval == NULL)
    {   
	if (error != NULL)
	{
	    g_warning("Remote:Failed return string value %s\n",key);
	    g_warning("Recommended to delete ~/.gconf/apps/gnopernicus directories");
	    g_error_free(error);
	    error = NULL;
	}
	
	if (!brl_set_string(default_value,key))
	    g_warning("Remote:Failed to set value %s\n",default_value);
	
	g_free(path);
	return g_strdup(default_value);
    }
    else
    {
	if (error != NULL)
	{
	    g_warning("Remote:Failed to get value %s\n",key);
	    g_warning("Recommended to delete ~/.gconf/apps/gnopernicus directories");
	    g_error_free(error);
	    error = NULL;
		
	    if (!brl_set_string(default_value,key))
		g_warning("Remote:Failed to set value %s\n",retval);
	}
	g_free(path);
	return retval;
    }
}
