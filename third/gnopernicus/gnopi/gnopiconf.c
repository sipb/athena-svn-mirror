/* gnopiconf.c
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
#include "gnopiconf.h"
#include "SRMessages.h"
#include "srintl.h"

#define SAVE_SCHEMA_TEMPLATE
#undef SAVE_SCHEMA_TEMPLATE

#ifdef SAVE_SCHEMA_TEMPLATE
#include <string.h>
#endif

GConfClient *gnopernicus_client = NULL;

gboolean
gnopiconf_gconf_client_init (void)
{    
    gnopernicus_client = gconf_client_get_default ();
 
    if (gnopernicus_client == NULL) 
    {
	sru_warning (_("Failed to init gconf client."));
	return FALSE;
    }
    
    return TRUE;
}

gboolean
gnopiconf_client_add_dir (const gchar *path)
{
    GError *error = NULL;
    
    gconf_client_add_dir (gnopernicus_client,
			 path, 
			 GCONF_CLIENT_PRELOAD_NONE, 
			 &error);

    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
	return FALSE;
    }
    
    return TRUE;
}

#ifdef SAVE_SCHEMA_TEMPLATE
static void
gnopiconf_create_schema_entry (const gchar *key,
			       const gchar *type,
			       const gchar *value,
			       GSList *list)
{
    if (strcmp (type, "list"))
    {
    fprintf (stderr,"\n<schema>"
    		    "\n<key>/schemas%s</key>"
    		    "\n<applyto>%s</applyto>"
    		    "\n<owner>gnopernicus</owner>"
    		    "\n<type>%s</type>"
    		    "\n<default>%s</default>"
    		    "\n<locale name=\"C\"></locale>"
    		    "\n</schema>",key, key, type, value);
    }
    else
    {
    GSList *elem = list;
    gchar  *rv = g_strdup (elem->data);
    elem = elem->next;
    while (elem)
    {
	rv = g_strconcat (rv,",",elem->data,NULL);
	elem = elem->next;
    }
    fprintf (stderr,"\n<schema>"
    		    "\n<key>/schemas%s</key>"
    		    "\n<applyto>%s</applyto>"
    		    "\n<owner>gnopernicus</owner>"
    		    "\n<type>%s</type>"
		    "\n<list_type>string</list_type>"
    		    "\n<default>[%s]</default>"
    		    "\n<locale name=\"C\"></locale>"
    		    "\n</schema>",key, key, type, rv);
	g_free (rv);
    }
}
#endif


void
gnopiconf_value_change (const gchar *key, 
			const gchar *val)
{
    GConfValue *value;
    GError     *error = NULL;
    sru_assert (gnopernicus_client);
    value = gconf_value_new (GCONF_VALUE_STRING);
    sru_assert (value);
    gconf_value_set_string (value, val);
    gconf_client_set (gnopernicus_client, key, value, &error);
    gconf_value_free (value);
    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
    }
    

}

static gboolean
gnopiconf_check_type (const gchar* key, 
		      GConfValue* val, 
		      GConfValueType t, 
		      GError** err)
{
    if (val->type != t)
    {
        g_set_error (err, GCONF_ERROR, GCONF_ERROR_TYPE_MISMATCH,
	  	   _("Expected key: %s"),
                   key);
	      
        return FALSE;
    }
    else
	return TRUE;
}


gboolean
gnopiconf_set_int (gint val, 
		   const gchar *key)
{
    GError *error = NULL;
    gboolean  ret = TRUE;
    
    sru_return_val_if_fail ( key != NULL, FALSE );
    sru_return_val_if_fail ( gnopernicus_client != NULL, FALSE );
    sru_return_val_if_fail ( gconf_client_key_is_writable (gnopernicus_client, key, NULL), FALSE);

#ifdef SAVE_SCHEMA_TEMPLATE
    gnopiconf_create_schema_entry (key, "int", g_strdup_printf ("%d",val), NULL);
#endif

    ret  = gconf_client_set_int (gnopernicus_client, key, val, &error);

    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
    }
    
    return ret;
}

gboolean
gnopiconf_set_double (gdouble val, 
		      const gchar *key)
{
    GError *error = NULL;
    gboolean  ret = TRUE;
    
    sru_return_val_if_fail ( key != NULL, FALSE );
    sru_return_val_if_fail ( gnopernicus_client != NULL, FALSE );
    sru_return_val_if_fail ( gconf_client_key_is_writable (gnopernicus_client, key, NULL), FALSE);

#ifdef SAVE_SCHEMA_TEMPLATE
    gnopiconf_create_schema_entry (key, "float", g_strdup_printf ("%f",val), NULL);
#endif

    ret  = gconf_client_set_float (gnopernicus_client, key, val, &error);

    g_message ("setting key %s to float %f", key, val);

    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
    }
    
    return ret;
}

gboolean
gnopiconf_set_double_in_section (gdouble val, 
				 const gchar *section, 
				 const gchar *key)
{
    gchar *path = NULL;
    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (section != NULL, FALSE);
    path = gconf_concat_dir_and_key (section, key);
    gnopiconf_set_double (val, path);
    g_free (path);
    return TRUE;
}

gboolean
gnopiconf_set_int_in_section (gint val, 
			      const gchar *section, 
			      const gchar *key)
{
    gchar *path = NULL;
    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (section != NULL, FALSE);
    path = gconf_concat_dir_and_key (section, key);
    gnopiconf_set_int (val, path);
    g_free (path);
    return TRUE;
}

gboolean
gnopiconf_set_string (const gchar *val, 
		      const gchar *key)
{
    GError *error = NULL;
    gboolean ret  = TRUE;
    
    sru_return_val_if_fail (key != NULL, FALSE );
    sru_return_val_if_fail (gnopernicus_client != NULL, FALSE );
    sru_return_val_if_fail (gconf_client_key_is_writable (gnopernicus_client, key, NULL), FALSE);
    
#ifdef SAVE_SCHEMA_TEMPLATE
    gnopiconf_create_schema_entry (key, "string", val, NULL);
#endif
    
    if (!val)
	val = "";
	
    ret = gconf_client_set_string (gnopernicus_client, key, val, &error);

    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
    }
    return ret;
}

gboolean
gnopiconf_set_string_in_section (const gchar *val, 
				 const gchar *section, 
				 const gchar *key)
{
    gchar *path = NULL;
    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (section != NULL, FALSE);
    path = gconf_concat_dir_and_key ( section, key);
    gnopiconf_set_string (val, path);
    g_free (path);
    return TRUE;
}


gboolean
gnopiconf_set_bool (gboolean val, 
		    const gchar *key)
{
    GError *error = NULL;
    gboolean  ret = TRUE;
    
    sru_return_val_if_fail (key != NULL, FALSE );
    sru_return_val_if_fail (gnopernicus_client != NULL, FALSE );
    sru_return_val_if_fail (gconf_client_key_is_writable (gnopernicus_client, key, NULL), FALSE);

#ifdef SAVE_SCHEMA_TEMPLATE        
    gnopiconf_create_schema_entry (key, "bool", (val?"true":"false"), NULL);
#endif    
    ret  = gconf_client_set_bool (gnopernicus_client, key, val, &error);

    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
    }
    
    return ret;
}

gboolean
gnopiconf_set_bool_in_section (	gboolean    val, 
				const gchar *section, 
				const gchar *key)
{
    gchar *path = NULL;
    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (section != NULL, FALSE);
    path = gconf_concat_dir_and_key ( section, key);
    gnopiconf_set_bool (val, path);
    g_free (path);
    return TRUE;
}


/**
 * Get Methods
**/
gint 
gnopiconf_get_int_with_default (const gchar *key, 
				gint default_value)
{
    GError *error = NULL;
    GConfValue *value = NULL;
    gint ret_val;
    gchar *key_error = NULL;
    
    sru_return_val_if_fail (key != NULL, default_value );
    sru_return_val_if_fail (gnopernicus_client != NULL, default_value );
    if (!gconf_valid_key (key, &key_error))
    {
	sru_error (key_error);
	g_free (key_error);
	return FALSE;
    }    

    value = gconf_client_get (gnopernicus_client, key, &error);
    
    ret_val = default_value;
    
    if (value != NULL && error == NULL)
    {	
	if (gnopiconf_check_type (key, value, GCONF_VALUE_INT, &error))
    	    ret_val = gconf_value_get_int (value);
        else
	{
    	    sru_warning (_("Invalid type of key: %s."), key);
	    if (!gnopiconf_set_int (ret_val, key))
	        sru_warning (_("Failed to set value: %d."), ret_val);
	}
	    
	gconf_value_free (value);
		
        return ret_val;
    }
    else
    {
	if (error != NULL)
	{
	    sru_warning (_(error->message));
	    g_error_free (error);
	    error = NULL;
	}

	    
	if (!gnopiconf_set_int (default_value, key))
	    sru_warning (_("Failed to set value: %d."), default_value);
		
        return default_value;
    }    
}

gdouble
gnopiconf_get_double_with_default (const gchar *key, 
				   gdouble default_value)
{
    GError *error = NULL;
    GConfValue *value = NULL;
    gdouble ret_val;
    gchar *key_error = NULL;
    
    sru_return_val_if_fail (key != NULL, default_value );
    sru_return_val_if_fail (gnopernicus_client != NULL, default_value );
    if (!gconf_valid_key (key, &key_error))
    {
	sru_error (key_error);
	g_free (key_error);
	return FALSE;
    }    

    value = gconf_client_get (gnopernicus_client, key, &error);
    
    ret_val = default_value;
    
    if (value != NULL && error == NULL)
    {	
	if (gnopiconf_check_type (key, value, GCONF_VALUE_FLOAT, &error))
    	    ret_val = gconf_value_get_float (value);
        else
	{
    	    sru_warning (_("Invalid type of key: %s."), key);
	    if (!gnopiconf_set_double (ret_val, key))
	        sru_warning (_("Failed to set value: %f."), ret_val);
	}
	    
	gconf_value_free (value);
		
        return ret_val;
    }
    else
    {
	if (error != NULL)
	{
	    sru_warning (_(error->message));
	    g_error_free (error);
	    error = NULL;
	}

	    
	if (!gnopiconf_set_double (default_value, key))
	    sru_warning (_("Failed to set value: %f."), default_value);
		
        return default_value;
    }    
}

gdouble
gnopiconf_get_double_from_section_with_default (const gchar *section,
						const gchar *key, 
						gdouble default_value)
{
    gchar *path = NULL;
    gint  return_value;
    sru_return_val_if_fail (key != NULL, default_value);
    sru_return_val_if_fail (section != NULL, default_value);
    path = gconf_concat_dir_and_key (section, key);
    return_value = gnopiconf_get_double_with_default (path, default_value);
    g_free (path);
    return return_value;
}


gint 
gnopiconf_get_int_from_section_with_default (const gchar *section,
					     const gchar *key, 
					     gint default_value)
{
    gchar *path = NULL;
    gint  return_value;
    sru_return_val_if_fail (key != NULL, default_value);
    sru_return_val_if_fail (section != NULL, default_value);
    path = gconf_concat_dir_and_key (section, key);
    return_value = gnopiconf_get_int_with_default (path, default_value);
    g_free (path);
    return return_value;
}


gchar* 
gnopiconf_get_string_with_default (const gchar *key, 
				   const gchar *default_value)
{
    GError *error = NULL;
    gchar *retval = NULL;
    gchar *key_error = NULL;
    
    sru_return_val_if_fail (key != NULL, g_strdup (default_value) );
    sru_return_val_if_fail (gnopernicus_client != NULL, g_strdup (default_value) );
    if (!gconf_valid_key (key, &key_error))
    {
	sru_error (key_error);
	g_free (key_error);
	return FALSE;
    }    

    retval = gconf_client_get_string (gnopernicus_client, key, &error);

    if (retval == NULL)
    {    
	if (error != NULL)
	{
	    sru_warning (_(error->message));
	    g_error_free (error);
	    error = NULL;
	}
	
	if (!gnopiconf_set_string (default_value, key))
	    sru_warning (_("Failed to set value: %s."), default_value);
	
	return g_strdup (default_value);
    }
    else
    {
	if (error != NULL)
	{
	    sru_warning(_(error->message));
	    g_error_free (error);
	    error = NULL;
		
	    if (!gnopiconf_set_string (default_value, key))
		sru_warning (_("Failed to set value: %s."), retval);
	    return g_strdup (default_value);
	}
	
	return retval;
    }
}

gchar*
gnopiconf_get_string_from_section_with_default (const gchar *section,
					        const gchar *key, 
					        const gchar *default_value)
{
    gchar *path = NULL;
    gchar *return_value;
    sru_return_val_if_fail (key != NULL, g_strdup (default_value));
    sru_return_val_if_fail (section != NULL, g_strdup (default_value));
    path = gconf_concat_dir_and_key (section, key);
    return_value = gnopiconf_get_string_with_default (path, default_value);
    g_free (path);
    return return_value;
}


gboolean
gnopiconf_get_bool_with_default (const gchar* key,
				 gboolean default_value)
{
    GError* error = NULL;
    GConfValue* val = NULL;
    gchar *key_error = NULL;

    sru_return_val_if_fail (key != NULL, default_value );
    sru_return_val_if_fail (gnopernicus_client != NULL, default_value );
    if (!gconf_valid_key (key, &key_error))
    {
	sru_error (key_error);
	g_free (key_error);
	return FALSE;
    }    

    val = gconf_client_get (gnopernicus_client, key, &error);

    if (val != NULL && error == NULL)
    {
	gboolean retval = default_value;
      
        if (gnopiconf_check_type (key, val, GCONF_VALUE_BOOL, &error))
    	    retval = gconf_value_get_bool (val);
        else
	{
    	    sru_warning (_("Invalid type of key: %s."), key);
	    if (!gnopiconf_set_bool (retval, key))
	        sru_warning (_("Failed to set value: %d."), retval);
	}
	
        gconf_value_free (val);
        return retval;
    }
    else
    {
	if (error != NULL)
	{
	    sru_warning (_(error->message));
	    g_error_free (error);
	    error = NULL;
	}
	    
	if (!gnopiconf_set_bool (default_value, key))
		sru_warning (_("Failed to set value: %d."), default_value);
        return default_value;
    }
}

gboolean
gnopiconf_get_bool_from_section_with_default (const gchar *section,
					      const gchar *key, 
					      gboolean default_value)
{
    gchar *path = NULL;
    gboolean return_value;
    sru_return_val_if_fail (key != NULL, default_value);
    sru_return_val_if_fail (section != NULL, default_value);
    path = gconf_concat_dir_and_key (section, key);
    return_value = gnopiconf_get_bool_with_default (path, default_value);
    g_free (path);
    return return_value;
}


gboolean
gnopiconf_get_bool (const gchar* key,
		    GError **error)
{
    GConfValue* val = NULL;
    gchar *key_error = NULL;

    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (gnopernicus_client != NULL, FALSE);
    
    gconf_client_add_dir (gnopernicus_client,
			 "/desktop/gnome/interface", 
			 GCONF_CLIENT_PRELOAD_NONE, 
			 error);

    if (*error != NULL)
	return FALSE;
    
    if (!gconf_valid_key (key, &key_error))
    {
	sru_error (key_error);
	g_free (key_error);
	return FALSE;
    }    

    val = gconf_client_get (gnopernicus_client, key, error);

    
    if (*error)
	return FALSE;
	
      
    if (gnopiconf_check_type (key, val, GCONF_VALUE_BOOL, error))
    	return gconf_value_get_bool (val);
    
    return FALSE;
}




gboolean
gnopiconf_set_list (GSList *list, 
		    GConfValueType list_type, 
		    const gchar *key)
{
    GError *error = NULL;
    gboolean  ret = TRUE;
    
    sru_return_val_if_fail (key != NULL, FALSE );
    sru_return_val_if_fail (gnopernicus_client != NULL, FALSE );
    sru_return_val_if_fail (gconf_client_key_is_writable (gnopernicus_client, key, NULL), FALSE);

#ifdef SAVE_SCHEMA_TEMPLATE            
    gnopiconf_create_schema_entry (key, "list", "NULL", list);
#endif

    ret  = gconf_client_set_list (gnopernicus_client, key, list_type, list, &error);

    if (error != NULL)
    {
	sru_warning (_(error->message));
	g_error_free (error);
	error = NULL;
    }
    
    return ret;
}

static GSList*
gnopiconf_get_list_from_gconf_list (GSList *list, 
				    GConfValueType type)
{
    GSList *retlist = NULL;
    
    if (!list) 
	return NULL;
    
    while (list)
    {
	GConfValue *value;
	value = list->data;
	switch (type)
	{
	    case GCONF_VALUE_STRING:
		retlist = g_slist_append (retlist, g_strdup((gchar*)gconf_value_get_string (value)));
		break;
	    case GCONF_VALUE_INT:
		retlist = g_slist_append (retlist, (gint*)gconf_value_get_int (value));
		break;
	    case GCONF_VALUE_BOOL:
	    case GCONF_VALUE_FLOAT:
	    default:
		break;
	}
	list = list->next;
    }
    return retlist;
}

GSList*
gnopiconf_get_list_with_default (const gchar* key, 
				 GSList *default_list, 
				 GConfValueType *type)
{
    GError* error = NULL;
    GConfValue* val = NULL;
    gchar *key_error = NULL;

    sru_return_val_if_fail (key != NULL, default_list );
    sru_return_val_if_fail (gnopernicus_client != NULL, default_list );
    if (!gconf_valid_key (key, &key_error))
    {
	sru_error (key_error);
	g_free (key_error);
	return FALSE;
    }    


    val = gconf_client_get (gnopernicus_client, key, &error);

    if (val != NULL && error == NULL)
    {
        GSList *retval = default_list;
        GSList *retlist = NULL;
      
        if (gnopiconf_check_type (key, val, GCONF_VALUE_LIST, &error))
        {
    	    retval = gconf_value_get_list (val);
	    *type = gconf_value_get_list_type (val);
	    retlist = gnopiconf_get_list_from_gconf_list (retval, *type);
        }	
        else
    	{
    	    sru_warning (_("Invalid type of key: %s."), key);
	    if (!gnopiconf_set_list (retval, *type, key))
		    sru_warning (_("Failed to set value."));
	}
	
        gconf_value_free (val);
        return retlist;
    }
    else
    {
	if (error != NULL)
	{
	    sru_warning (_(error->message));
	    g_error_free (error);
	    error = NULL;
	}
		    
        return NULL;
    }
}

gboolean
gnopiconf_unset_key (const gchar *key)
{
    GError *error = NULL;
    GConfValue *val;
    gboolean retval = FALSE;
    gchar *key_error = NULL;
    
    sru_return_val_if_fail ( key != NULL, FALSE);
    
    if (!gconf_valid_key (key, &key_error))
    {
	sru_error (key_error);
	g_free (key_error);
	return FALSE;
    }    

    
    val = gconf_client_get (gnopernicus_client, key, &error);
    
    if (val && !error)
    {    
	gconf_value_free (val);
	
	retval = gconf_client_unset (gnopernicus_client, key , &error);
        
	if (error != NULL)
	{
	    sru_warning(_(error->message));
	    g_error_free(error);
	    error = NULL;
	}
	else
	    return retval = TRUE;
    }
    return retval;
}


gboolean
gnopiconf_remove_dir (gchar *dir)
{
    GError *err = NULL;
    
    sru_return_val_if_fail (dir != NULL, FALSE);
    sru_return_val_if_fail (gnopernicus_client != NULL, FALSE );
    
    
    gconf_client_remove_dir ( gnopernicus_client, dir, &err);
    
    if (err)
    {
	sru_warning(_(err->message));
	g_error_free (err);
	return FALSE;
    }
	
    return TRUE;
}

GSList*
gnopiconf_get_all_entries (const gchar *path)
{
    GSList *rv 	  = NULL;
    GError *error = NULL;
    
    sru_return_val_if_fail (path != NULL, NULL);
    sru_return_val_if_fail (gnopernicus_client != NULL, NULL);

    
    
    rv = gconf_client_all_entries (gnopernicus_client, path, &error);
    
    if (error)
    {
        sru_warning (_(error->message));
        g_error_free (error);
        error = NULL;
	return NULL;
    }
    return rv;
}

