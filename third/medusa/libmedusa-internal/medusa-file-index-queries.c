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
 *  medusa-file-queries.c  Builds and performs common queries on a file index
 *
 */

#include <config.h>
#include "medusa-file-index-queries.h"

#include "medusa-conf.h"
#include "medusa-keyword-set.h"
#include "medusa-query-clauses.h"
#include "medusa-rdb-query.h"
#include "medusa-rdb-record.h"
#include <glib.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <libmedusa/medusa-file-info-utilities.h>
#include <libmedusa/medusa-utils.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


#define QUERY_NO_ARG(query_name, field_name, operator, value, desired_result) \
gboolean \
medusa_file_index_##query_name (MedusaFileSystemDB *file_system_db, \
                                MedusaRDBRecord record, \
                                gpointer arg) \
{ \
	return simple_query (file_system_db, record, \
                             MEDUSA_FILE_INDEX_##field_name##_FIELD_TITLE, \
                             MEDUSA_RDB_##operator, \
                             value, desired_result); \
}

#define QUERY_CHAR_ARG(query_name, field_name, operator, desired_result) \
gboolean \
medusa_file_index_##query_name (MedusaFileSystemDB *file_system_db, \
                                MedusaRDBRecord record, \
                                const char *arg) \
{ \
        return simple_query (file_system_db, record, \
                             MEDUSA_FILE_INDEX_##field_name##_FIELD_TITLE, \
                             MEDUSA_RDB_##operator, \
                             arg, desired_result); \
}

#define QUERY_INT_ARG(query_name, field_name, operator, desired_result) \
gboolean \
medusa_file_index_##query_name (MedusaFileSystemDB *file_system_db, \
                                MedusaRDBRecord record, \
                                int arg) \
{ \
        return simple_query (file_system_db, record, \
                             MEDUSA_FILE_INDEX_##field_name##_FIELD_TITLE, \
                             MEDUSA_RDB_##operator, \
                             &arg, desired_result); \
} 

static gboolean
simple_query (MedusaFileSystemDB *file_system_db,
              MedusaRDBRecord record,
              const char *field_name,
              MedusaRDBOperator operator,
              MedusaRDBOperand operand,
              gboolean result_desired)
{
        MedusaRDBField *field;
        MedusaRDBQuery *query;
        MedusaRDBFieldInfo *field_info;
        gboolean query_result;
        
        g_return_val_if_fail (record != NULL, FALSE);
        g_return_val_if_fail (file_system_db != NULL, FALSE);
        
        query = medusa_rdb_query_new ();
        field_info = medusa_file_system_db_get_field_info (file_system_db);
        field = medusa_rdb_field_get_field_structure (field_info, field_name);
        query = medusa_rdb_query_add_selection_criterion
                (query, field, operator,
                 operand, result_desired, file_system_db);
        query_result = medusa_rdb_query_match
                (query, record, field_info);
        medusa_rdb_query_free (query);
        return query_result;
}

gboolean
medusa_file_index_is_of_type (MedusaFileSystemDB *file_system_db,
                              MedusaRDBRecord record,
                              const char *type)
{
        if (strcasecmp (type, "file") == 0) {
                return medusa_file_index_is_file (file_system_db,
                                                  record,
                                                  NULL);
        }

        if (strcasecmp (type, "text_file") == 0) {
                return medusa_file_index_is_text_file (file_system_db,
                                                       record,
                                                       NULL);
        }
        if (strcasecmp (type, "application") == 0) {
                return medusa_file_index_is_application (file_system_db,
                                                         record,
                                                         NULL);
        }

        if (strcasecmp (type, "music") == 0) {
                return medusa_file_index_is_music (file_system_db,
                                                   record,
                                                   NULL);
        }
        if (strcasecmp (type, "directory") == 0) {
                return medusa_file_index_is_directory (file_system_db,
                                                       record,
                                                       NULL);
        }

        return FALSE;
}

gboolean
medusa_file_index_is_not_of_type (MedusaFileSystemDB *file_system_db,
                                  MedusaRDBRecord record,
                                  const char *type)
{
        return !(medusa_file_index_is_of_type (file_system_db,
                                               record,
                                               type));
}

QUERY_NO_ARG (is_file, MIME_TYPE, REGEXP_MATCH, "^x\\-special|^x\\-directory", FALSE)
     QUERY_NO_ARG (is_text_file, MIME_TYPE, REGEXP_MATCH, "^text/", TRUE)
     QUERY_NO_ARG (is_application, MIME_TYPE, REGEXP_MATCH, "^application/", TRUE)
     QUERY_NO_ARG (is_music, MIME_TYPE, REGEXP_MATCH, "^audio/", TRUE)
     QUERY_NO_ARG (is_directory, MIME_TYPE, REGEXP_MATCH, "^x\\-directory/", TRUE)
     QUERY_INT_ARG (has_uid, OWNER, NUMBER_EQUALS, TRUE)
     QUERY_INT_ARG (has_gid, OWNER, NUMBER_EQUALS, TRUE)
     QUERY_INT_ARG (does_not_have_uid, OWNER, NUMBER_EQUALS, FALSE)
     QUERY_INT_ARG (does_not_have_gid, OWNER, NUMBER_EQUALS, FALSE)

     /* FIXME bugzilla.eazel.com 2649: These should be time_t and not ints */
     QUERY_INT_ARG (is_modified_before_time, MTIME, LESS_THAN, TRUE)
     QUERY_INT_ARG (is_modified_after_time, MTIME, GREATER_THAN, TRUE)

     QUERY_INT_ARG (is_larger_than, SIZE, GREATER_THAN, TRUE)
     QUERY_INT_ARG (is_smaller_than, SIZE, LESS_THAN, TRUE)
     QUERY_INT_ARG (is_of_size, SIZE, NUMBER_EQUALS, TRUE)


     /* Special functions that require more processing */





gboolean
medusa_file_index_uid_can_read_file (MedusaFileSystemDB *file_system_db,
                                     MedusaRDBRecord record,
                                     int uid)
{
        MedusaRDBFieldInfo *field_info;
        int owner_uid, owner_gid;
        GnomeVFSFilePermissions permissions;
        
        g_return_val_if_fail (record != NULL, FALSE); 
        g_return_val_if_fail (file_system_db != NULL, FALSE); 
        /* We can skip all of this stuff if it's root */
        if (uid == 0) {
                printf ("user is root, it's ok to read.\n");
                return TRUE;
        }
        
        field_info = medusa_file_system_db_get_field_info (file_system_db);

        medusa_rdb_record_get_field_value
                (record,
                 field_info,
                 MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_TITLE,
                 file_system_db,
                 &permissions);
        
        if (permissions & S_IROTH) {
#ifdef PERMISSIONS_CHECK_DEBUG
                printf ("Other can read the file, it is readable to everyone\n");
#endif
                return TRUE;
        }
        
        medusa_rdb_record_get_field_value
                (record,
                 field_info,
                 MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE,
                 file_system_db,
                 &owner_uid);
#ifdef PERMISSIONS_CHECK_DEBUG
        printf ("File has uid %d\n", owner_uid);
#endif
        if ((permissions & S_IRUSR) &&
            owner_uid == uid) {
#ifdef PERMISSIONS_CHECK_DEBUG
                printf ("File is readable by owner, and owner is the uid here. returning\n");
#endif
                return TRUE;
        }

        medusa_rdb_record_get_field_value
                (record,
                 field_info,
                 MEDUSA_FILE_INDEX_GROUP_FIELD_TITLE,
                 file_system_db,
                 &owner_gid);
#ifdef PERMISSIONS_CHECK_DEBUG
        printf ("File has gid %d\n", owner_gid);
#endif
        if ((permissions & S_IRGRP) &&
            medusa_group_contains (owner_gid, uid)) {
#ifdef PERMISSIONS_CHECK_DEBUG
                printf ("File is readable by group, and owner is in the uid here. returning\n");
#endif
                return TRUE;
        }
#ifdef PERMISSIONS_CHECK_DEBUG
        printf ("Hmm, done of the above. returning false\n");
#endif
        return FALSE;
        
}

gboolean
medusa_file_index_marked_with_keyword (MedusaFileSystemDB *db,
                                       MedusaRDBRecord record,
                                       const char *keyword)
{
        MedusaRDBFieldInfo *field_info;
        char keywords_string[NAME_MAX];
        MedusaKeywordSet *keyword_set;
        gboolean marked;

        /* Extract the keywords for this record. */
        field_info = medusa_file_system_db_get_field_info (db);
        medusa_rdb_record_get_field_value
                (record, field_info,
                 MEDUSA_FILE_INDEX_KEYWORDS_FIELD_TITLE,
                 db, keywords_string);

        /* Convert into a keyword set and od the match check. */
        keyword_set = medusa_keyword_set_new_from_string (keywords_string);
        marked = medusa_keyword_set_has_keyword
                (keyword_set, getuid (), keyword);
        medusa_keyword_set_destroy (keyword_set);

        return marked;
}

gboolean
medusa_file_index_not_marked_with_keyword (MedusaFileSystemDB *db,
                                           MedusaRDBRecord record,
                                           const char *keyword)
{
        return !medusa_file_index_marked_with_keyword (db, record, keyword);
}
