/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
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
 *  
 */


/* medusa-index-service.c -- API for externals users of the medusa
   indexing service. */

#include "medusa-index-service.h"
#include "medusa-index-service-private.h"
#include "medusa-service-private.h"

#include <libmedusa-internal/medusa-conf.h>

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>



static time_t        get_last_index_update_time_by_hack                 (void);


MedusaIndexingRequestResult 
medusa_index_service_request_reindex (void)
{
        int index_socket;
        int read_length;
        char *request;
        char response[512];

        index_socket = medusa_initialize_socket_for_requests (INDEX_SOCKET_PATH);

        if (index_socket == -1) {
                g_warning ("couldn't open socket\n");
                return MEDUSA_INDEXER_ERROR_NO_INDEXER_PRESENT;
        }

        request = g_strdup (MEDUSA_REINDEX_REQUEST);
        write (index_socket, request, strlen (request));
        g_free (request);
        
        read_length = 1;
        while (read_length > 0 && strchr (response, '\n') == NULL) {
                read_length = read (index_socket, response, 512 * sizeof (char));
                
        }
        close (index_socket);

        if (strstr (response, MEDUSA_REINDEX_REQUEST_ACCEPTED)) {
                return  MEDUSA_INDEXER_REQUEST_OK;
        }
        if (strstr (response, MEDUSA_REINDEX_REQUEST_DENIED_BUSY)) {
                return MEDUSA_INDEXER_ERROR_BUSY;
        }



        return MEDUSA_INDEXER_ERROR_NO_RESPONSE;
}

time_t                      
medusa_index_service_get_last_index_update_time  (void)
{
        return get_last_index_update_time_by_hack ();
}


static time_t
get_last_index_update_time_by_hack (void)
{
        struct stat index_file_info;
        int stat_return_code;

        stat_return_code = stat (TEXT_INDEX_LOCATION_FILE_NAME, &index_file_info);

        return index_file_info.st_mtime;
}
