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
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 *          Maciej Stachowiak <mjs@eazel.com>
 */

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <medusa-search-service.h>
#include <unistd.h>
#include <stdio.h>





int
main (int argc, char *argv[])
{
        MedusaSearchServiceConnection *connection;
        char *result;
        GnomeVFSResult connection_opened_result;

        gnome_vfs_init ();
        if (argc < 2) {
                puts ("usage: msearch SEARCH_URI");
                return -1;
        }
        
        connection = medusa_search_service_connection_new (argv[1],
                                                           &connection_opened_result);
        if (connection_opened_result == GNOME_VFS_ERROR_INVALID_URI) {
                puts ("The URI you entered was invalid.  If you are unsure of how to form a search uri, consult the search_uri_rfc, that is part of medusa's documentation.\n");
                return -1;
        }
        if (connection_opened_result != GNOME_VFS_OK) {
                puts ("The search service is unavailable right now.  It can be started by running /gnome/bin/medusa-searchd as root.  \n\nYour index may also be missing or corrupt, in which case you should rerun the medusa indexer.  The indexer can be restarted by running /gnome/bin/medusa-indexd as root.");
                return -1;
        }
        
        medusa_search_service_connection_start_search (connection);

        while (medusa_search_service_connection_read_search_result (connection, &result) == GNOME_VFS_OK) {
                puts (result);
        }
        
        medusa_search_service_connection_destroy (connection);
        
        return 0;
}











