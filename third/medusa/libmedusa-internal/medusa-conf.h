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
 *  
 */

/* medusa-conf.h: These are hard wired constants.  Some of them may
   need to be tuned for particular systems.  Please feel free to do
   so. */


#ifndef MEDUSA_CONF_H
#define MEDUSA_CONF_H

#include <config.h>


#define ROOT_DIRECTORY "/"
#define MEDUSA_DATABASE_DIRECTORY MEDUSA_PREFIX "/share/medusa/"


#define URI_LIST_NAME MEDUSA_DATABASE_DIRECTORY "uri-list"
#define DIRECTORY_NAME_HASH_NAME MEDUSA_DATABASE_DIRECTORY "directory-name-hash"
#define FILE_NAME_HASH_NAME MEDUSA_DATABASE_DIRECTORY "file-name-hash"

#define FILE_SYSTEM_DB_FILE_NAME MEDUSA_DATABASE_DIRECTORY "file-system-db"
#define MIME_TYPE_HASH_NAME MEDUSA_DATABASE_DIRECTORY "mime-type-hash"
#define KEYWORD_SETS_FILE_NAME MEDUSA_DATABASE_DIRECTORY "keyword-sets-hash"

#define TEXT_INDEX_START_FILE_NAME MEDUSA_DATABASE_DIRECTORY "text-index-start-file"
#define TEXT_INDEX_LOCATION_FILE_NAME MEDUSA_DATABASE_DIRECTORY "text-index-location-file"
#define TEXT_INDEX_WORD_FILE_NAME MEDUSA_DATABASE_DIRECTORY "text-index-word-file"
#define TEXT_INDEX_TEMP_FILE_NAME MEDUSA_DATABASE_DIRECTORY "text-index-temp-file"
#define NUMBER_OF_TEMP_INDEXES 11

#define URI_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "uri-list-account-db"
#define DIRECTORY_NAME_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "directory-names-account-db"
#define FILE_NAME_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "file-names-account-db"
#define FILE_SYSTEM_DB_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "file-system-db-account-db"
#define MIME_TYPE_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "mime-type-account-db"
#define KEYWORD_SETS_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "keyword-sets-account-db"
#define TEXT_INDEX_START_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "text-index-start-account-db"
#define TEXT_INDEX_LOCATION_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "text-index-location-account-db"
#define TEXT_INDEX_WORD_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "text-index-word-account-db"
#define TEXT_INDEX_TEMP_ACCOUNT_INDEX MEDUSA_DATABASE_DIRECTORY "text-index-temp-account-db"
#define INDEX_ACCOUNT_FILE "medusa-account-data"

#define BACKUP_SUFFIX ".in_progress"

#define INDEX_LOG_ERRORS

#define MEDUSA_FILE_INDEX_BACKUP_LOCK_FILE "/var/lock/medusa-db.bak"

#define DIRECTORY_HASH_BITS 16
#define FILE_HASH_BITS 18
#define MIME_HASH_BITS 10
#define KEYWORD_SETS_HASH_BITS 12
#define SEMANTIC_UNITS_BITS 21

#define MEDUSA_MAXIMUM_SEARCH_QUERY_LENGTH 1024

#define MEDUSA_INDEX_FORMAT_VERSION "0.3"
#define MEDUSA_FILENAME_INDEXING_INTERVAL 10000
#define MEDUSA_FILESYSTEM_INDEXING_INTERVAL 86400

#define MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE      "File_Name"
#define MEDUSA_URI_LIST_FILE_NAME_FIELD_SIZE       3
#define MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE "Directory_Name"
#define MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_SIZE  2

#define MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_TITLE   "URI_Number"
#define MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_SIZE    sizeof (int)
#define MEDUSA_FILE_INDEX_MTIME_FIELD_TITLE        "Modification_Time"
#define MEDUSA_FILE_INDEX_MTIME_FIELD_SIZE         sizeof (time_t)
#define MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE        "Owner"
#define MEDUSA_FILE_INDEX_OWNER_FIELD_SIZE         sizeof (int)
#define MEDUSA_FILE_INDEX_GROUP_FIELD_TITLE        "Group"
#define MEDUSA_FILE_INDEX_GROUP_FIELD_SIZE         sizeof (int)
#define MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_TITLE  "Permissions"
#define MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_SIZE   sizeof (int)
#define MEDUSA_FILE_INDEX_SIZE_FIELD_TITLE         "Size"
#define MEDUSA_FILE_INDEX_SIZE_FIELD_SIZE          sizeof (size_t)
#define MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE    "Mime_Type"
#define MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_SIZE     sizeof (int)
#define MEDUSA_FILE_INDEX_IS_DIRECTORY_FIELD_TITLE "Is_Directory"
#define MEDUSA_FILE_INDEX_IS_DIRECTORY_FIELD_SIZE  1
#define MEDUSA_FILE_INDEX_KEYWORDS_FIELD_TITLE     "keywords"
#define MEDUSA_FILE_INDEX_KEYWORDS_FIELD_SIZE      sizeof (int)

/* These aren't being used yet, but maybe we should
   use these instead of mime types to deal with problems
   of certain fields being useless for certain types of
   files, like size */
#define MEDUSA_FILE_INDEX_FILE_TYPE_FIELD_TITLE   "File_Type"
#define MEDUSA_FILE_INDEX_FILE_TYPE_FIELD_SIZE    1

#endif /* MEDUSA_CONF_H */
