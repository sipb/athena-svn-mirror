/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 * 
 *  medusa-indexed-search.c -- Connect to the medusa search daemon and 
 *  request results
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
 *  Authors: Maciej Stachowiak <mjs@eazel.com> 
 *  
 */

#include "medusa-indexed-search.h"
#include "medusa-search-service-private.h"
#include "medusa-service-private.h"
#include "medusa-utils.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


struct MedusaIndexedSearch {
        int socket_fd;
        guint cookie;

        gboolean busy;
        char *buffer;
};



static GnomeVFSResult
authenticate_connection (MedusaIndexedSearch *connection) {
	char *request;
        char *file_name;
        char acknowledgement_buffer[13];
        int cookie_fd, key;
        int read_result, write_result;

	/* Send request for cookie */
	request = g_strdup_printf ("%s\t%lu\t%lu\n", COOKIE_REQUEST, (unsigned long) getuid (), (unsigned long) getpid());

	write_result = write (connection->socket_fd, request, strlen (request));

        g_free (request);

        if (write_result == -1) {
                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
        }
        
        read_result = read (connection->socket_fd, acknowledgement_buffer, 12);
        if (read_result == -1) {
                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
        }

        if (medusa_str_has_prefix (acknowledgement_buffer, SEARCH_INDEX_ERROR_TRANSMISSION)) {
                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
        }


        /* Go look for cookie */
        file_name = g_strdup_printf ("%s/%lu_%lu", COOKIE_PATH, (unsigned long) getuid (), (unsigned long) getpid ());
        
        cookie_fd = open (file_name, O_RDONLY);
        
        /* FIXME bugzilla.eazel.com 3000: 
           maybe have search daemon send response once cookie
           file is created instead of this loop */
        
        /* Keep looking if cookie file isn't created yet */
        while (cookie_fd == -1) {
                cookie_fd = open (file_name, O_RDONLY);
        }
        
        g_free (file_name);
        
        read (cookie_fd, &key, sizeof (int));
        close (cookie_fd);
  
        connection->cookie = key;

        return GNOME_VFS_OK;
}


GnomeVFSResult
medusa_indexed_search_is_available ()
{
        GnomeVFSResult result;
        MedusaIndexedSearch *connection;
        /* FIXME bugzilla.eazel.com 3001: This is inefficient. */
        connection = medusa_indexed_search_new (&result);
        if (connection != NULL) {
	        medusa_indexed_search_destroy (connection);
        }

        return result;
        
}


MedusaIndexedSearch *
medusa_indexed_search_new (GnomeVFSResult *result)
{
        MedusaIndexedSearch *connection;

        connection = g_new0 (MedusaIndexedSearch, 1);
        
        connection->socket_fd = medusa_initialize_socket_for_requests (SEARCH_SOCKET_PATH);

        if (connection->socket_fd == -1) {
                *result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
                return NULL;
        }


        *result = authenticate_connection (connection);

        return connection;
}


static void
send_search_request (MedusaIndexedSearch *connection, 
                     const char *search_uri)
{
        char *request;

        request = g_strdup_printf ("%lu %lu %d\t%s", (unsigned long) getuid(), (unsigned long) getpid(), connection->cookie, search_uri);

#ifdef DEBUG_SEARCH_API
        printf ("about to send %s\n", request);
#endif

        /* FIXME bugzilla.eazel.com 2999: check error code */
        write (connection->socket_fd, request, strlen(request));

        g_free (request);

}



GnomeVFSResult           
medusa_indexed_search_start_search (MedusaIndexedSearch *connection,
				    const char *search_uri)
{
        if (connection->busy) {
                return GNOME_VFS_ERROR_IN_PROGRESS;
        } else {
                send_search_request (connection, search_uri);
                connection->busy = TRUE;
                return GNOME_VFS_OK;
        }
}


GnomeVFSResult medusa_indexed_search_read_search_result (MedusaIndexedSearch *connection,
							 char **result)
{
        char tmpbuf[513];
        const char *newline_pos;
        char *result_uri;
        int len;
        char *buffer;

        if (!connection->busy) {
                *result = NULL;
                return GNOME_VFS_ERROR_EOF;
        }
        if (connection->buffer == NULL) {
                connection->buffer = g_strdup ("");
        }

        while (strchr (connection->buffer, '\n') == NULL) {
                len = read (connection->socket_fd, &tmpbuf, 512 * sizeof (char)); 

#ifdef DEBUG_SEARCH_API
                puts ("read from socket");
#endif

                tmpbuf[len] = '\0';
                buffer = g_strconcat (connection->buffer, tmpbuf, NULL);
                g_free (connection->buffer);
                connection->buffer = buffer;
        }



        /* grab a result */
        newline_pos = strchr (connection->buffer, '\n');
        result_uri = g_strndup (connection->buffer, newline_pos - connection->buffer);
        g_return_val_if_fail (result_uri != NULL, GNOME_VFS_ERROR_NOT_FOUND);

        /* move buffer forward */
        buffer = g_strdup (newline_pos + 1);
        g_free (connection->buffer);
        connection->buffer = buffer;


        if (strcmp (result_uri, SEARCH_END_TRANSMISSION) == 0) {
                g_free (result_uri);
                *result = NULL;
                connection->busy = FALSE;
                return GNOME_VFS_ERROR_EOF;
        } else {
                *result = result_uri;
                return GNOME_VFS_OK;
        }
}



void medusa_indexed_search_destroy (MedusaIndexedSearch *connection)
{
        close (connection->socket_fd);
        g_free (connection->buffer);
        g_free (connection);
}
