/* gok-gconf.c
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <gconf/gconf-client.h>
#include "gok-log.h"

/**
 * gok_gconf_get_double:
 * @client: GConfClient to use to retreive the key.
 * @key:    GConf key to retrieve.
 * @dest:   Pointer to a gdouble to store the retrieved key, if successful.
 *
 * Retrieves the requested key and stores it in the given dest.  If an
 * error occurs during retreiving the key a message is logged using
 * gok_log_x and dest is left unchanged.
 *
 * Returns: TRUE if key retrieved successfully, FALSE if an error occured.
 **/
gboolean
gok_gconf_get_double (GConfClient *client,
		      const gchar *key,
		      gdouble *dest)
{
    gdouble a_gdouble;
    GError *err = NULL;

    a_gdouble = gconf_client_get_float (client, key, &err);
    if (err != NULL)
    {
	gok_log_x ("Error getting key %s from GConf", key);
	g_error_free (err);
	return FALSE;
    }
    else
    {
	*dest = a_gdouble;
	return TRUE;
    }
}

/**
 * gok_gconf_set_double:
 * @client:  GConfClient to use to set the value.
 * @key:     GConf key to set.
 * @value:   gdouble to store.
 *
 * Stores the given (key, value) pair if it is different from what is
 * in GConf already. If an error occurs during storing the key a
 * message is logged using gok_log_x.
 *
 * Returns: TRUE if key stored successfully, FALSE if an error occured.
 **/
gboolean
gok_gconf_set_double (GConfClient *client,
		      const gchar *key,
		      gdouble value)
{
    gdouble a_gdouble;
    gboolean successful;
    GError *err = NULL;

    /* why are we doing this double round trip?  Doesn't gconf_client_set... already check for equality? */
    if (gok_gconf_get_double (client, key, &a_gdouble) && (a_gdouble == value))
    {
        return TRUE;
    }

    successful = gconf_client_set_float (client, key, value, &err);
    if ( (!successful) || (err != NULL) )
    {
        gok_log_x ("Error setting key %s in GConf", key);
        if (err != NULL)
        {
            g_error_free (err);
        }
        return FALSE;
    }
    else
    {
        gok_log ("Set key %s in GConf", key);
        return TRUE;
    }
}

/**
 * gok_gconf_get_int:
 * @client: GConfClient to use to retreive the key.
 * @key:    GConf key to retrieve.
 * @dest:   Pointer to a gint to store the retrieved key, if successful.
 *
 * Retrieves the requested key and stores it in the given dest.  If an
 * error occurs during retreiving the key a message is logged using
 * gok_log_x and dest is left unchanged.
 *
 * Returns: TRUE if key retrieved successfully, FALSE if an error occured.
 **/
gboolean
gok_gconf_get_int (GConfClient *client,
                   const gchar *key,
                   gint *dest)
{
    gint a_gint;
    GError *err = NULL;

    a_gint = gconf_client_get_int (client, key, &err);
    if (err != NULL)
    {
	gok_log_x ("Error getting key %s from GConf", key);
	g_error_free (err);
	return FALSE;
    }
    else
    {
	*dest = a_gint;
	return TRUE;
    }
}

/**
 * gok_gconf_set_int:
 * @client:  GConfClient to use to set the value.
 * @key:     GConf key to set.
 * @value:   gint to store.
 *
 * Stores the given (key, value) pair if it is different from what is
 * in GConf already. If an error occurs during storing the key a
 * message is logged using gok_log_x.
 *
 * Returns: TRUE if key stored successfully, FALSE if an error occured.
 **/
gboolean
gok_gconf_set_int (GConfClient *client,
                   const gchar *key,
                   gint value)
{
    gint a_gint;
    gboolean successful;
    GError *err = NULL;

    if (gok_gconf_get_int (client, key, &a_gint) && (a_gint == value))
    {
        return TRUE;
    }

    successful = gconf_client_set_int (client, key, value, &err);
    if ( (!successful) || (err != NULL) )
    {
        gok_log_x ("Error setting key %s in GConf", key);
        if (err != NULL)
        {
            g_error_free (err);
        }
        return FALSE;
    }
    else
    {
        gok_log ("Set key %s in GConf", key);
        return TRUE;
    }
}

/**
 * gok_gconf_get_bool:
 * @client: GConfClient to use to retreive the key.
 * @key:    GConf key to retrieve.
 * @dest:   Pointer to a gboolean to store the retrieved key, if successful.
 *
 * Retrieves the requested key and stores it in the given dest.  If an
 * error occurs during retreiving the key a message is logged using
 * gok_log_x and dest is left unchanged.
 *
 * Returns: TRUE if key retrieved successfully, FALSE if an error occured.
 **/
gboolean
gok_gconf_get_bool (GConfClient *client,
                    const gchar *key,
                    gboolean *dest)
{
    gboolean a_gboolean;
    GError *err = NULL;

    a_gboolean = gconf_client_get_bool (client, key, &err);
    if (err != NULL)
    {
	gok_log_x ("Error getting key %s from GConf", key);
	g_error_free (err);
	return FALSE;
    }
    else
    {
	*dest = a_gboolean;
	return TRUE;
    }
}

/**
 * gok_gconf_set_bool:
 * @client:  GConfClient to use to set the value.
 * @key:     GConf key to set.
 * @value:   gboolean to store.
 *
 * Stores the given (key, value) pair if it is different from what is
 * in GConf already.  If an error occurs during storing the key a
 * message is logged using gok_log_x.
 *
 * Returns: TRUE if key stored successfully, FALSE if an error occured.
 **/
gboolean
gok_gconf_set_bool (GConfClient *client,
                    const gchar *key,
                    gboolean value)
{
    gboolean a_gboolean;
    gboolean successful;
    GError *err = NULL;

    if (gok_gconf_get_bool (client, key, &a_gboolean) && (a_gboolean == value))
    {
        return TRUE;
    }

    successful = gconf_client_set_bool (client, key, value, &err);
    if ( (!successful) || (err != NULL) )
    {
	gok_log_x ("Error setting key %s in GConf", key);
        if (err != NULL)
        {
            g_error_free (err);
        }
	return FALSE;
    }
    else
    {
        gok_log ("Set key %s in GConf", key);
	return TRUE;
    }
}

/**
 * gok_gconf_get_string:
 * @client: GConfClient to use to retreive the key.
 * @key:    GConf key to retrieve.
 * @dest:   Pointer to a pointer to a gchar to store the retrieved key, if successful.
 *
 * Retrieves the requested key and stores it in the given dest.  If an
 * error occurs during retreiving the key a message is logged using
 * gok_log_x and dest is left unchanged.
 *
 * NOTE: *dest needs to be freed when finished with.
 *
 * Returns: TRUE if key retrieved successfully, FALSE if an error occured.
 **/
gboolean
gok_gconf_get_string (GConfClient *client,
                      const gchar *key,
                      gchar **dest)
{
    gchar *a_gchar;
    GError *err = NULL;

    a_gchar = gconf_client_get_string (client, key, &err);
    if (a_gchar == NULL)
    {
        gok_log_x ("Got unexpected NULL when getting key %s from GConf", key);
        return FALSE;
    }
    if (err != NULL)
    {
	gok_log_x ("Error getting key %s from GConf", key);
	g_error_free (err);
	return FALSE;
    }
    else
    {
	*dest = a_gchar;
	return TRUE;
    }
}

/**
 * gok_gconf_set_string:
 * @client:  GConfClient to use to set the value.
 * @key:     GConf key to set.
 * @value:   gchar to store.
 *
 * Stores the given (key, value) pair if it is different from what is
 * in GConf already.  If an error occurs during storing the key a
 * message is logged using gok_log_x.
 *
 * Returns: TRUE if key stored successfully, FALSE if an error occured.
 **/
gboolean
gok_gconf_set_string (GConfClient *client,
                      const gchar *key,
                      const gchar *value)
{
    gchar *a_gchar = NULL;
    gboolean successful;
    GError *err = NULL;

    if (value != NULL)
    {
        a_gchar = gconf_client_get_string (client, key, NULL);

        if ((a_gchar != NULL) && (strcmp (a_gchar, value) == 0))
        {
            g_free (a_gchar);
            return TRUE;
        }

        if (a_gchar != NULL)
        {
            g_free (a_gchar);
        }

        successful = gconf_client_set_string (client, key, value, &err);
        if ( (!successful) || (err != NULL) )
        {
            gok_log_x ("Error setting key %s in GConf", key);
            if (err != NULL)
            {
                g_error_free (err);
            }
            return FALSE;
        }
        else
        {
            gok_log ("Set key %s in GConf", key);
            return TRUE;
        }
    }
    else
    {
        gconf_client_unset (client, key, NULL);
        return TRUE;
    }
}
