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

/* medusa-master-db.h:  The top level db
   that requests items from the file index and the text index */

#ifndef MEDUSA_MASTER_DB_H
#define MEDUSA_MASTER_DB_H

#include <libgnomevfs/gnome-vfs-types.h>

#include "medusa-enums.h"
#include "medusa-rdb-query.h"
#include "medusa-rdb-query-private.h"
#include "medusa-text-index.h"

typedef struct MedusaMasterDB MedusaMasterDB;


/* Open, if it exists */
MedusaMasterDB *medusa_master_db_open  (const char     *root_uri,
					MedusaLogLevel  log_level,
					const char     *uri_list,
					const char     *file_attribute_index,
					const char     *file_name_hash,
					const char     *directory_name_hash,
					const char     *mime_type_hash,
					const char     *keyword_sets_file_name,
					const char     *text_index_start_file,
					const char     *text_index_location_file,
					const char     *text_index_word_file);

/* Create new structure  */
MedusaMasterDB *medusa_master_db_new    (const char     *root_uri,
					 MedusaLogLevel  log_level,
					 const char     *uri_list,
					 const char     *file_attribute_index,
					 const char     *file_name_hash,
					 const char     *directory_name_hash,
					 const char     *mime_type_hash,
					 const char     *keyword_sets_file_name,
					 const char     *text_index_start_file,
					 const char     *text_index_location_file,
					 const char     *text_index_word_file,
					 const char     *text_index_temp_file);
/* Do full indexing */
void            medusa_master_db_index  (MedusaMasterDB *db);

/* Do quick update */
void            medusa_master_db_update (MedusaMasterDB *db);

/* Returns a list of URIs */
GList *         medusa_master_db_query  (MedusaMasterDB *db,
					 char           *search_uri);
void            medusa_master_db_ref    (MedusaMasterDB *db);
void            medusa_master_db_unref  (MedusaMasterDB *db);


typedef GList MedusaQueryOrList;
typedef GList MedusaQueryAndList;



union {
  MedusaRDBQueryCriterion *criterion;
  MedusaQueryOrList *or_list;
} MedusaBooleanExpression;



#endif /* MEDUSA_MASTER_DB_H */
