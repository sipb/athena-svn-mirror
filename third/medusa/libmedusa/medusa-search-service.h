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
 *           Maciej Stachowiak <mjs@eazel.com> 
 *  
 */


/* medusa-search-service.h -- API for externals users of the medusa
   search service to make requests and receive results */

#ifndef MEDUSA_SEARCH_SERVICE_H
#define MEDUSA_SEARCH_SERVICE_H

#include <libgnomevfs/gnome-vfs-types.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#ifndef NAME_MAX
#define NAME_MAX 512
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

/* FIXME bugzilla.eazel.com 3008:  Is anyone using this type? */

typedef struct MedusaSearchHandle MedusaSearchHandle;

typedef struct MedusaSearchServiceConnection MedusaSearchServiceConnection;

typedef enum {
        MEDUSA_SEARCH_METHOD_INDEX_ONLY,
        MEDUSA_SEARCH_METHOD_FULL_SEARCH,
        MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY
} MedusaSearchMethod;


MedusaSearchServiceConnection *medusa_search_service_connection_new (const char *uri,
                                                                     GnomeVFSResult *result);

GnomeVFSResult medusa_search_service_connection_is_available_for_uri       (GnomeVFSURI *uri);


GnomeVFSResult medusa_search_service_connection_start_search       (MedusaSearchServiceConnection *connection);

GnomeVFSResult medusa_search_service_connection_read_search_result (MedusaSearchServiceConnection *connection,
                                                                    char **result);

void           medusa_search_service_connection_destroy            (MedusaSearchServiceConnection *connection);

#endif /* MEDUSA_SEARCH_SERVICE_H */


