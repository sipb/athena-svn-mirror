/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 * 
 *  medusa.h: general header for programs that link with medusa
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Rebecca Schulman <rebecka@eazel.com>
 *           Maciej Stachowiak <mjs@eazel.com>
 *  
 */

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libmedusa/medusa-indexed-search.h>
#include <libmedusa/medusa-search-service.h>
#include <libmedusa/medusa-unindexed-search.h>
#include <libmedusa/medusa-search-service-private.h>
#include <libmedusa/medusa-service-private.h>
#include <libmedusa/medusa-utils.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef enum {
        SEARCH_MODE_UNINDEXED,
        SEARCH_MODE_INDEXED
} SearchMode;

struct MedusaSearchServiceConnection {
        char *uri;
        MedusaIndexedSearch *indexed_search;
        MedusaUnindexedSearch *unindexed_search;

        MedusaSearchMethod method;
        SearchMode mode;
};

static gboolean               uri_has_valid_header          (const char *uri);

static MedusaSearchMethod     gnome_vfs_uri_to_search_method          (GnomeVFSURI *uri);
static MedusaSearchMethod     uri_to_search_method                    (const char *uri);

static char *                 uri_to_string_remove_extra_slashes      (const GnomeVFSURI *uri);
GnomeVFSResult
medusa_search_service_connection_is_available_for_uri (GnomeVFSURI *uri)
{
        MedusaSearchMethod method;

        method = gnome_vfs_uri_to_search_method (uri);

        /* FIXME bugzilla.eazel.com 3002:  
           Should we claim searches are available 
           for full searches if the index is not? */
        if (method == MEDUSA_SEARCH_METHOD_INDEX_ONLY) {
                return medusa_indexed_search_is_available ();
        }
        g_assert ((method == MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY) || 
                  (method == MEDUSA_SEARCH_METHOD_FULL_SEARCH));
        /* We can always do an unindexed search */
        return GNOME_VFS_OK;
}


MedusaSearchServiceConnection *
medusa_search_service_connection_new (const char *uri,
                                      GnomeVFSResult *result)
{
        MedusaSearchServiceConnection *connection;
        
        if (!uri_has_valid_header (uri)) {
                *result = GNOME_VFS_ERROR_INVALID_URI;
                return NULL;
        }

        connection = g_new0 (MedusaSearchServiceConnection, 1);
        connection->uri = g_strdup (uri);
        connection->method = uri_to_search_method (uri);
        if (connection->method == MEDUSA_SEARCH_METHOD_INDEX_ONLY) {
                connection->indexed_search = medusa_indexed_search_new (result);
                connection->unindexed_search = NULL;
                connection->mode = SEARCH_MODE_INDEXED;
                return connection;
        }
        if (connection->method == MEDUSA_SEARCH_METHOD_FULL_SEARCH) {
                connection->mode = SEARCH_MODE_INDEXED;
                connection->indexed_search = medusa_indexed_search_new (result);
                if (*result != GNOME_VFS_OK) {
                        return connection;
                }
                connection->unindexed_search = medusa_unindexed_search_new (result, uri);

                return connection;
        }
        if (connection->method == MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY) {
                connection->indexed_search = NULL;
                connection->unindexed_search = medusa_unindexed_search_new (result, uri);
                connection->mode = SEARCH_MODE_UNINDEXED;
                return connection;
        }
        g_assert_not_reached ();
        return NULL;
}


GnomeVFSResult           
medusa_search_service_connection_start_search (MedusaSearchServiceConnection *connection)
{
        GnomeVFSResult result;

	result = GNOME_VFS_OK;

        if (connection->method != MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY) {
                result = medusa_indexed_search_start_search (connection->indexed_search,
                                                             connection->uri);
        }

        if (connection->method != MEDUSA_SEARCH_METHOD_INDEX_ONLY) {
                medusa_unindexed_search_start_search (connection->unindexed_search);
        }

        return result;
}


GnomeVFSResult 
medusa_search_service_connection_read_search_result (MedusaSearchServiceConnection *connection,
                                                     char **result)
{
        GnomeVFSResult read_result;

        if (connection->mode == SEARCH_MODE_INDEXED) {
                read_result = medusa_indexed_search_read_search_result (connection->indexed_search,
                                                                        result);
                /* If we're doing a full search and we're out of 
                   results, switch gears into an unindexed search */
                if (read_result == GNOME_VFS_ERROR_EOF &&
                    connection->method == MEDUSA_SEARCH_METHOD_FULL_SEARCH) {
                        connection->mode = SEARCH_MODE_UNINDEXED;
                        return medusa_search_service_connection_read_search_result (connection, result);
                } else {
                        return read_result;
                }
        }
        read_result = medusa_unindexed_search_read_search_result (connection->unindexed_search,
                                                                  result);
        if (read_result == GNOME_VFS_ERROR_DIRECTORY_BUSY) {
                sleep (1);
                read_result = medusa_unindexed_search_read_search_result (connection->unindexed_search,
                                                                          result);
        }
        return read_result;
}



void medusa_search_service_connection_destroy (MedusaSearchServiceConnection *connection)
{
        g_free (connection->uri);
        if (connection->method != MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY) {
                medusa_indexed_search_destroy (connection->indexed_search);
        }
        if (connection->method != MEDUSA_SEARCH_METHOD_INDEX_ONLY) {
                medusa_unindexed_search_destroy (connection->unindexed_search);
        }

        g_free (connection);
}

static gboolean               
uri_has_valid_header (const char *uri)
{
        const char *method_string;

        if (!(medusa_str_has_prefix (uri,"search:") ||
              medusa_str_has_prefix (uri,"gnome-search:") ||
              medusa_str_has_prefix (uri,"medusa:"))) {
                return FALSE;
        }
        
        method_string = strchr (uri, ':') + 1;
        if (!(medusa_str_has_prefix (method_string, "index-only") ||
              medusa_str_has_prefix (method_string, "index-with-backup") ||
              medusa_str_has_prefix (method_string, "unindexed-only") ||
              medusa_str_has_prefix (method_string, "[file:///]"))) {
                return FALSE;
        }

        return TRUE;
}

static MedusaSearchMethod
gnome_vfs_uri_to_search_method (GnomeVFSURI *gnome_vfs_uri)
{
        char *uri;
        char *unescaped_uri;
        MedusaSearchMethod method;

        uri = uri_to_string_remove_extra_slashes (gnome_vfs_uri);
        unescaped_uri = gnome_vfs_unescape_string (uri, NULL);

        method = uri_to_search_method (unescaped_uri);
        g_free (unescaped_uri);
        return method;

}

static MedusaSearchMethod      
uri_to_search_method (const char *uri)
{
        const char *method_string;
        
        /* We should assert these, because we checked that the uri was valid
           before sending it here */
        g_assert (medusa_str_has_prefix (uri, "search:") ||
                  medusa_str_has_prefix (uri, "gnome-search:") ||
                  medusa_str_has_prefix (uri, "medusa:"));
        method_string = strchr (uri, ':') + 1;
        
        if (medusa_str_has_prefix (method_string, "index-only")) {
                return MEDUSA_SEARCH_METHOD_INDEX_ONLY;
        }
        if (medusa_str_has_prefix (method_string, "index-with-backup")) {
                return MEDUSA_SEARCH_METHOD_FULL_SEARCH;
        } 
        if (medusa_str_has_prefix (method_string, "unindexed-only")) {
                return MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY;
        }
        /* Check also for the case of no stated method, and return the default */
        if (*method_string == '[') {
                return MEDUSA_SEARCH_METHOD_INDEX_ONLY;
        }
        
        g_assert_not_reached ();
        return MEDUSA_SEARCH_METHOD_INDEX_ONLY;
}


/* FIXME bugzilla.eazel.com 2615: 
   This function works around a problem in gnome-vfs where you
 * can't get the original URI without added slashes. That problem
 * should be fixed and then this can be removed.
 */
static char *
uri_to_string_remove_extra_slashes (const GnomeVFSURI *uri)
{
	char *uri_text, *past_colon, *result;

	/* Remove the "//" after the ":" in this URI.
	 * It's safe to assume there there is a ":".
	 */
	uri_text = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	past_colon = strchr (uri_text, ':') + 1;
	if (strncmp (past_colon, "//", 2) == 0) {
		result = g_new (char, strlen (uri_text) - 2 + 1);
		memcpy (result, uri_text, past_colon - uri_text);
		strcpy (result + (past_colon - uri_text), past_colon + 2);
		g_free (uri_text);
	} else {
		result = uri_text;
	}

	return result;
}
