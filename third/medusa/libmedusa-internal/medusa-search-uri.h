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
 */

#ifndef MEDUSA_SEARCH_URI_H
#define MEDUSA_SEARCH_URI_H

#include <sys/types.h>

#include "medusa-master-db.h"
#include "medusa-file-index.h"
#include "medusa-uri-list.h"
#include "medusa-query-clauses.h"

#define MEDUSA_FILE_NAME_CRITERION "file_name"
#define MEDUSA_DIRECTORY_NAME_CRITERION "file_name"
#define MEDUSA_FILE_TYPE_CRITERION "file_type"
#define MEDUSA_MODIFIED_CRITERION "modified"
#define MEDUSA_OWNER_CRITERION "owner"
#define MEDUSA_GROUP_CRITERION "group"
#define MEDUSA_SIZE_CRITERION "size"
#define MEDUSA_READ_PERMISSIONS_CRITERION "permissions_to_read"
#define MEDUSA_CONTENTS_CRITERION "contents"
#define MEDUSA_EMBLEMS_CRITERION "emblems"

typedef struct MedusaParsedSearchURI MedusaParsedSearchURI;
typedef struct MedusaClauseClosure MedusaClauseClosure;


struct MedusaClauseClosure {
	MedusaQueryFunc query_func;
        MedusaFileSystemDB *file_system_db;
        MedusaURIList *uri_list;

        MedusaArgumentType argument_type;
        gpointer argument;

        gboolean is_content_request;
        char *content_request;
        gboolean return_matches;
        gboolean match_all_words;
};

struct MedusaParsedSearchURI {
        GList *clause_closures;
        gboolean query_is_always_true;
        gboolean query_is_always_false;
        gboolean syntax_error_found;
};


gboolean                 medusa_uri_is_search_uri                    (char                *uri);
MedusaParsedSearchURI *  medusa_search_uri_parse                     (char                *search_uri,
                                                                      MedusaMasterDB      *master_db);
void                     medusa_search_uri_parse_shutdown            (void);
void                     medusa_parsed_search_uri_free               (MedusaParsedSearchURI *parsed_search_uri);
gboolean                 medusa_clause_closure_is_content_search     (gpointer             data,
                                                                      gpointer             use_data);
void                     medusa_clause_closure_list_free             (GList *clause_closures);

/* Add extra criterion for content requests to
   make sure content requests have the appropriate
   security criteria, and size requests only
   return regular files */
char *   medusa_search_uri_add_extra_needed_criteria (const char          *search_uri,
                                                      uid_t               uid);

#endif /* MEDUSA_SEARCH_URI_H */
