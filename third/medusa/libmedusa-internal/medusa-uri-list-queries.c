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
 *  medusa-uri-list-queries.c  Builds and performs common queries on a list of uri's
 *
 */

#include <glib.h>
#include <string.h>

#include "medusa-conf.h"
#include "medusa-query-clauses.h"
#include "medusa-rdb-fields.h"
#include "medusa-rdb-query.h"
#include "medusa-rdb-record.h"
#include "medusa-uri-list.h"
#include "medusa-uri-list-private.h"
#include "medusa-uri-list-queries.h"

static char *           regexp_escape               (char *string);
static char *           file_glob_to_regexp         (char *file_glob);


#define MEDUSA_URI_LIST_QUERY_CHAR_ARG(query_name, arg_name, field_name, operator, desired_result) \
gboolean \
medusa_uri_list_##query_name (MedusaURIList *uri_list, \
                              MedusaRDBRecord record, \
                              char * arg_name) \
{ \
        MedusaRDBField *field; \
        MedusaRDBQuery *query; \
        MedusaRDBFieldInfo *field_info; \
        gboolean query_result; \
   \
        g_return_val_if_fail (record != NULL, FALSE); \
\
        query = medusa_rdb_query_new (); \
        field_info = medusa_uri_list_get_field_info (uri_list); \
        field = medusa_rdb_field_get_field_structure (field_info, \
                                                      field_name); \
        query = medusa_rdb_query_add_selection_criterion (query, \
                                                          field, \
                                                          operator, \
                                                          arg_name, \
                                                          desired_result, \
                                                          uri_list); \
        query_result = medusa_rdb_query_match (query, \
                                               record, \
                                               field_info); \
        medusa_rdb_query_free (query); \
        return query_result ; \
}

MEDUSA_URI_LIST_QUERY_CHAR_ARG (is_in_directory, directory_name, MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE, MEDUSA_RDB_STRING_EQUALS, TRUE)
     
     MEDUSA_URI_LIST_QUERY_CHAR_ARG (is_named, file_name, MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE, MEDUSA_RDB_STRING_EQUALS, TRUE)

     MEDUSA_URI_LIST_QUERY_CHAR_ARG (has_name_regexp_matching, string, MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE, MEDUSA_RDB_REGEXP_MATCH, TRUE)

     MEDUSA_URI_LIST_QUERY_CHAR_ARG (has_name_not_regexp_matching, string, MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE, MEDUSA_RDB_REGEXP_MATCH, FALSE)

     MEDUSA_URI_LIST_QUERY_CHAR_ARG (is_in_directory_regexp_matching, string, MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE, MEDUSA_RDB_STRING_EQUALS, TRUE)

     MEDUSA_URI_LIST_QUERY_CHAR_ARG (is_not_in_directory_regexp_matching, string, MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE, MEDUSA_RDB_STRING_EQUALS, FALSE)
     
     gboolean
medusa_uri_list_has_name_containing (MedusaURIList *uri_list,
                                     MedusaRDBRecord record,
                                     char *string)
{
        char *escaped_string;
        gboolean result;
        
        escaped_string = regexp_escape (string);
        result = medusa_uri_list_has_name_regexp_matching (uri_list, 
                                                           record, 
                                                           escaped_string);
        g_free (escaped_string);
        return result;
}



gboolean
medusa_uri_list_has_name_not_containing (MedusaURIList *uri_list,
                                         MedusaRDBRecord record,
                                         char *string)
{
        char *escaped_string;
        gboolean result; 
        escaped_string = regexp_escape (string);
        result = medusa_uri_list_has_name_not_regexp_matching  (uri_list, 
                                                                record, 
                                                                escaped_string);
        g_free (escaped_string);
        return result;
}


gboolean
medusa_uri_list_is_in_directory_containing (MedusaURIList *uri_list,
                                            MedusaRDBRecord record,
                                            char *string)
{
        char *escaped_string;
        gboolean result;
        escaped_string = regexp_escape (string);

        result = medusa_uri_list_is_in_directory_regexp_matching  (uri_list, 
                                                                   record, 
                                                                   escaped_string);
        g_free (escaped_string);
        return result;

}


gboolean
medusa_uri_list_has_file_name_starting_with (MedusaURIList *uri_list,
                                             MedusaRDBRecord record,
                                             char *string)
{
        char *escaped_string, *pattern;
        gboolean result;
        escaped_string = regexp_escape (string);
        pattern = g_strconcat ("^", escaped_string, NULL);
        
        result = medusa_uri_list_has_name_regexp_matching  (uri_list, 
                                                            record, 
                                                            pattern);
        g_free (pattern);
        g_free (escaped_string);
        return result;
}


gboolean
medusa_uri_list_has_file_name_ending_with  (MedusaURIList *uri_list,
                                            MedusaRDBRecord record,
                                            char *string)
{
        char *escaped_string, *pattern;
        gboolean result;
        escaped_string = regexp_escape (string);
        pattern = g_strconcat (escaped_string, "$", NULL);
        
        result = medusa_uri_list_has_name_regexp_matching  (uri_list, 
                                                            record, 
                                                            pattern);
        g_free (pattern);
        g_free (escaped_string);
        return result;
}


gboolean
medusa_uri_list_is_in_directory_tree (MedusaURIList *uri_list,
                                      MedusaRDBRecord record, 
                                      char *string)
{
        char *escaped_string, *pattern;
        gboolean result;

        escaped_string = regexp_escape (string);
        pattern = g_strconcat ("^", escaped_string, NULL);
        
        result = medusa_uri_list_is_in_directory_regexp_matching  (uri_list, 
                                                                   record, 
                                                                   escaped_string);

        g_free (pattern);
        g_free (escaped_string);
        return result;
}


gboolean
medusa_uri_list_has_name_glob_matching (MedusaURIList *uri_list,
                                        MedusaRDBRecord record,
                                        char *string)
{
        char *pattern;
        gboolean result;

        pattern = file_glob_to_regexp (string);
        
        result = medusa_uri_list_has_name_regexp_matching  (uri_list, 
                                                            record, 
                                                            pattern);
        g_free (pattern);
        return result;
}

gboolean
medusa_uri_list_has_full_file_name (MedusaURIList *uri_list,
                                    MedusaRDBRecord record,
                                    char *full_file_name)
{
        MedusaRDBField *field; 
        MedusaRDBQuery *query; 
        MedusaRDBFieldInfo *field_info; 
        char *file_name, *directory_name;
        gboolean result;
   
        g_return_val_if_fail (uri_list != NULL, FALSE); 
        g_return_val_if_fail (full_file_name != NULL, FALSE); 
        
        file_name = &full_file_name[strlen (full_file_name) - 1];
        while (*file_name != '/') {
                file_name--;
        }
        file_name++;
        directory_name = g_new0 (char, (int) (file_name - full_file_name));
        strncpy (directory_name, full_file_name, (int) (file_name - full_file_name));
        directory_name[(int) (file_name - full_file_name)] = 0;

        query = medusa_rdb_query_new (); 
        field_info = medusa_uri_list_get_field_info (uri_list); 
        field = medusa_rdb_field_get_field_structure (field_info, 
                                                      MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE); 
        query = medusa_rdb_query_add_selection_criterion (query, 
                                                          field, 
                                                          MEDUSA_RDB_STRING_EQUALS,
                                                          file_name,
                                                          TRUE,
                                                          uri_list); 
        field = medusa_rdb_field_get_field_structure (field_info, 
                                                      MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE); 
        query = medusa_rdb_query_add_selection_criterion (query, 
                                                          field, 
                                                          MEDUSA_RDB_STRING_EQUALS,
                                                          directory_name,
                                                          TRUE,
                                                          uri_list); 
        result = medusa_rdb_query_match (query, 
                                         record,
                                         field_info); 
        g_free (directory_name);
        medusa_rdb_query_free (query); 
        return result;

        

}
                                

static char *           
regexp_escape (char *string)
{
        char *literal, *literal_loc;
        literal = g_new0 (char, 3 * strlen (string) + 1);
        literal_loc = literal;
        while (*string) {
                *literal_loc++ = '[';
                *literal_loc++ = *string;
                *literal_loc++ = ']';
                string++;
        }
        return literal;
}

static char *
file_glob_to_regexp (char *file_glob)
{
        char *literal, *literal_loc;

        literal = g_new0 (char, 4 * strlen (file_glob) + 1);
        /* Iterate through the file_glob, quoting '.' and changing * to .* */
        /* The cool bracket hack was the idea of engber@eazel.com */
        literal_loc = literal;
        while (*file_glob) {

                if (*file_glob == '.') {
                        *literal_loc++ = '\\';
                        *literal_loc = '.';
                }
                if (*file_glob == '*') {
                        *literal_loc++ = '.';
                        *literal_loc = '*';
                }
                else {                
                        *literal_loc++ = '[';
                        *literal_loc++ = *file_glob;
                        *literal_loc++ = ']';
                }
                file_glob++;
        }
        return literal;
}


