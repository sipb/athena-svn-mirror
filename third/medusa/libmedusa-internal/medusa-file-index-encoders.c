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

/* medusa-file-index-encoders.c : files to encode and decode the
   various fields of the file attribute database */

#include <config.h>
#include <glib.h>

#include <medusa-file-index-encoders.h>
#include <medusa-conf.h>
#include <medusa-test.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <medusa-byte.h>
#include <sys/stat.h>
#include <fcntl.h> 


void
medusa_file_database_uri_number_encode (char *result,
                                        int *uri_number,
                                        MedusaFileSystemDB *file_system_db)
{
        memcpy (result, uri_number, sizeof (int));
        /* strncpy (result, medusa_int_to_bytes (owner, MEDUSA_FILE_INDEX_OWNER_FIELD_SIZE),
           medusa_rdb_field_get_size (file_system_db->file_database->file->field_info, 
           MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE));*/
}
                                        

void
medusa_file_database_mime_type_encode (char *result,
				       char *mime_type,
				       MedusaFileSystemDB *file_system_db)
{
        char *bytes; 

        bytes = medusa_token_to_bytes (medusa_string_to_token (file_system_db->mime_types,
                                                               mime_type),
                                       medusa_rdb_field_get_size (file_system_db->file_database->file->field_info, 
                                                                  MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE));
        memcpy (result,
                bytes,
                medusa_rdb_field_get_size (file_system_db->file_database->file->field_info, 
                                           MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE));
        g_free (bytes);
}

void
medusa_file_database_mtime_encode (char *result,
				   time_t *mtime,
                                   MedusaFileSystemDB *file_system_db)
{
        memcpy (result, (char *) mtime, sizeof (time_t));
}


void
medusa_file_database_owner_encode (char *result,
				   int *owner,
                                   MedusaFileSystemDB *file_system_db)
{
        memcpy (result, owner, sizeof (int));
        /* strncpy (result, medusa_int_to_bytes (owner, MEDUSA_FILE_INDEX_OWNER_FIELD_SIZE),
           medusa_rdb_field_get_size (file_system_db->file_database->file->field_info, 
           MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE));*/
}

void
medusa_file_database_group_encode (char *result,
				   gid_t *group,
                                   MedusaFileSystemDB *file_system_db)
	
{
        memcpy (result, group, 
                medusa_rdb_field_get_size (file_system_db->file_database->file->field_info, 
                                           MEDUSA_FILE_INDEX_GROUP_FIELD_TITLE));
}

void
medusa_file_database_permissions_encode (char *result,
					 int *permissions,
                                         MedusaFileSystemDB *file_system_db)
     
{
        memcpy (result, permissions, 
                medusa_rdb_field_get_size (file_system_db->file_database->file->field_info, 
                                           MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_TITLE));
}


void
medusa_file_database_size_encode (char *result,
				  size_t *size,
                                  MedusaFileSystemDB *file_system_db)
     
{
        memcpy (result, size,
                medusa_rdb_field_get_size (file_system_db->file_database->file->field_info, 
                                           MEDUSA_FILE_INDEX_SIZE_FIELD_TITLE));
}

void
medusa_file_database_uri_number_decode (int *uri_number,
                                        char *field,
                                        gpointer data)
{
        memcpy (uri_number, field, sizeof (int));
}


void 
medusa_file_database_mime_type_decode (char *mime_type_result,
				       char *field,
				       MedusaFileSystemDB *file_system_db)
{
        strncpy (mime_type_result, 
                 medusa_token_to_string (file_system_db->mime_types,
                                         medusa_bytes_to_token (field,
                                                                medusa_rdb_field_get_size (file_system_db->file_database->file->field_info, 
                                                                                           MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE))),
                 NAME_MAX);
}

void
medusa_file_database_mtime_decode (time_t *mtime,
				   char *field,
				   gpointer data)
{
        memcpy (mtime, field, sizeof (time_t));
}


void
medusa_file_database_owner_decode (int *owner,
				   char *field,
				   gpointer data)
{
        memcpy (owner, field, sizeof (int));
}

void
medusa_file_database_group_decode (unsigned long *group,
				   char *field,
				   gpointer data)
{
        *group = *(gid_t *) field;
}

void
medusa_file_database_permissions_decode (int *permissions,
					 char *field,
					 gpointer data)
{
        *permissions = *(int *) field;
}

void
medusa_file_database_size_decode (size_t *size,
                                  char *field,
                                  gpointer data)
{
        *size = *(size_t *) field;
}

void
medusa_file_database_keywords_encode (char *result,
                                      char *keywords,
                                      MedusaFileSystemDB *file_system_db)
{
        MedusaToken token;
        char *bytes;

        token = medusa_string_to_token (file_system_db->keyword_sets, keywords);
        bytes = medusa_token_to_bytes (token, MEDUSA_FILE_INDEX_KEYWORDS_FIELD_SIZE);
        memcpy (result, bytes, MEDUSA_FILE_INDEX_KEYWORDS_FIELD_SIZE);
        g_free (bytes);
}

void
medusa_file_database_keywords_decode (char *result,
                                      char *field,
                                      MedusaFileSystemDB *file_system_db)
{
        MedusaToken token;
        char *keywords;

        token = medusa_bytes_to_token (field, MEDUSA_FILE_INDEX_KEYWORDS_FIELD_SIZE);
        if (token == 0) {
                result[0] = '\0';
                return;
        }
        keywords = medusa_token_to_string (file_system_db->keyword_sets, token);
        /* FIXME bugzilla.eazel.com 2646: 
           Truncating the list of keywords sucks, but it's the
           * best we can do given this encoding/decoding API.
           */
        strncpy (result, keywords, NAME_MAX);
}

void 
medusa_file_index_encoders_test (void)
{
        
        char buffer[NAME_MAX];
        char *name, name_back[NAME_MAX];
        time_t time_v, time_back;
        int owner, owner_back;
        unsigned long group, group_back;
        int permissions, permissions_back;
        size_t size, size_back;
        MedusaFileSystemDB *file_system_db;
        MedusaHash *file_names, *directory_names;

        /* FIXME bugzilla.eazel.com 2647:
           These tests need to be updated */

 	file_names = medusa_hash_new (FILE_NAME_HASH_NAME,
				      FILE_HASH_BITS);
	directory_names = medusa_hash_new (DIRECTORY_NAME_HASH_NAME,
					   DIRECTORY_HASH_BITS);

        file_system_db = medusa_file_system_db_new
                ("/gnome-source",
                 FILE_SYSTEM_DB_FILE_NAME,
                 file_names,
                 directory_names,
                 MIME_TYPE_HASH_NAME,
                 KEYWORD_SETS_FILE_NAME);
        
        /* Mime Type checks */
        name = g_strdup ("this is a test mime type alkfjdsa;lkfj");
        medusa_file_database_mime_type_encode (buffer, name, file_system_db);
        medusa_file_database_mime_type_decode (name_back, buffer, file_system_db);
        MEDUSA_TEST_STRING_RESULT (name_back, name); 

        name = g_strdup ("");
        medusa_file_database_mime_type_encode (buffer, name, file_system_db);
        medusa_file_database_mime_type_decode (name_back, buffer, file_system_db);
        MEDUSA_TEST_STRING_RESULT (name_back, name);

        name = g_strdup ("Application/x-pdf");
        medusa_file_database_mime_type_encode (buffer, name, file_system_db);
        medusa_file_database_mime_type_decode (name_back, buffer, file_system_db);
        MEDUSA_TEST_STRING_RESULT (name_back, name);

        /* Mtime Checks */
        time_v = time(NULL);
        medusa_file_database_mtime_encode (buffer, &time_v, file_system_db);
        medusa_file_database_mtime_decode (&time_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (time_back, time_v);

        time_v = 0;
        medusa_file_database_mtime_encode (buffer, &time_v, file_system_db);
        medusa_file_database_mtime_decode (&time_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (time_back, time_v);

        time_v = 9999999;
        medusa_file_database_mtime_encode (buffer, &time_v, file_system_db);
        medusa_file_database_mtime_decode (&time_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (time_back, time_v);

        /* Owner checks */
        owner = 0;
        medusa_file_database_owner_encode (buffer, &owner, file_system_db);
        medusa_file_database_owner_decode (&owner_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (owner_back, owner);

        owner = 500;
        medusa_file_database_owner_encode (buffer, &owner, file_system_db);
        medusa_file_database_owner_decode (&owner_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (owner_back, owner);

        /* FIXME bugzilla.eazel.com 2648:  
           -3 doesnt work */
        owner = 27;
        medusa_file_database_owner_encode (buffer, &owner, file_system_db);
        medusa_file_database_owner_decode (&owner_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (owner_back, owner);
        
        /* Group tests */
        group = 0;
        medusa_file_database_group_encode (buffer, (gid_t *) &group, file_system_db);
        medusa_file_database_group_decode (&group_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (group_back, group);

        group = 500;
        medusa_file_database_group_encode (buffer, (gid_t *) &group, file_system_db);
        medusa_file_database_group_decode (&group_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (group_back, group);

        group = -3;
        medusa_file_database_group_encode (buffer, (gid_t *) &group, file_system_db);
        medusa_file_database_group_decode (&group_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (group_back, group);

        /* Permissions Check */
        permissions = S_IRWXU;
        medusa_file_database_permissions_encode (buffer, &permissions, file_system_db);
        medusa_file_database_permissions_decode (&permissions_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (permissions_back, permissions);

        permissions = 0;
        medusa_file_database_permissions_encode (buffer, &permissions, file_system_db);
        medusa_file_database_permissions_decode (&permissions_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (permissions_back, permissions);

        permissions = S_IRWXU | S_IRGRP | S_IWGRP | S_IXOTH;
        medusa_file_database_permissions_encode (buffer, &permissions, file_system_db);
        medusa_file_database_permissions_decode (&permissions_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (permissions_back, permissions);

        /* Size tests */

        size = 500;
        medusa_file_database_size_encode (buffer, &size, file_system_db);
        medusa_file_database_size_decode (&size_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (size_back, size);

        size = 10293833;
        medusa_file_database_size_encode (buffer, &size, file_system_db);
        medusa_file_database_size_decode (&size_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (size_back, size);

        size = 0;
        medusa_file_database_size_encode (buffer, &size, file_system_db);
        medusa_file_database_size_decode (&size_back, buffer, file_system_db);
        MEDUSA_TEST_INTEGER_RESULT (size_back, size);
}
