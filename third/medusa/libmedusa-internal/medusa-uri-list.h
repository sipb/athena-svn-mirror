/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

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

/* medusa-uri-list.h:  The list of uris that a master index
   is keeping information about.  */ 


#ifndef MEDUSA_URI_LIST_H
#define MEDUSA_URI_LIST_H

#include <glib.h>

#include "medusa-query-clauses.h"
#include "medusa-db-update.h"
#include "medusa-enums.h"
#include "medusa-rdb-record.h"
#include "medusa-rdb-fields.h"

typedef struct MedusaURIList MedusaURIList;

MedusaURIList *         medusa_uri_list_new                   (const char           *root_directory,
							       MedusaLogLevel        log_lebel,
							       const char           *uri_list_name,
							       MedusaHash           *file_names,
							       MedusaHash           *directory_names);

MedusaURIList *         medusa_uri_list_open                  (const char           *root_directory,
							       MedusaLogLevel        log_lebel,
							       const char           *uri_list_name,
							       MedusaHash           *file_names,
							       MedusaHash           *directory_names);

/* Do full indexing */
void                    medusa_uri_list_index_file            (MedusaURIList        *db,
							       char                 *directory_name,
							       char                 *file_name);
/* Do quick update -- not implemented yet */
void                    medusa_uri_list_update_file           (MedusaURIList        *uri_list,
							       char                 *directory_name,
							       char                 *file_name,
							       MedusaDBUpdateAction  action); 
char *                  medusa_uri_number_to_uri              (MedusaURIList        *uri_list,
							       int                   uri_number);
MedusaRDBFieldInfo *    medusa_uri_list_get_field_info        (MedusaURIList        *uri_list);
MedusaQueryClauses *    medusa_uri_list_get_query_clauses     (MedusaURIList        *uri_list); 
int                     medusa_uri_list_number_of_uris        (MedusaURIList        *uri_list);
void                    medusa_uri_list_ref                   (MedusaURIList        *db);
void                    medusa_uri_list_unref                 (MedusaURIList        *db);


#endif /* MEDUSA_URI_LIST_H */
