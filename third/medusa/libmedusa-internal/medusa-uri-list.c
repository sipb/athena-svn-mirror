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

/* medusa-uri-list.c:  The list of uris that a master index
   is keeping information about.  */ 

#include <config.h>

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <libmedusa/medusa-utils.h>

#include "medusa-conf.h"
#include "medusa-file-index.h"
#include "medusa-hash.h"
#include "medusa-rdb-table.h"
#include "medusa-uri-list.h"
#include "medusa-uri-list-encoders.h"
#include "medusa-uri-list-private.h"
#include "medusa-uri-list-queries.h"


#define URI_LIST_METAINFO_LENGTH 100

static MedusaRDBFieldInfo * uri_list_field_info               (void);
static MedusaQueryClauses * uri_list_clauses                  (void);
static void                 read_index_info_from_database     (MedusaURIList *uri_list);

static void                 write_index_info_to_database      (MedusaRDBTable *table,
							       char *version,
							       int last_indexing_time);

static void                 medusa_uri_list_destroy           (MedusaURIList *db);




MedusaURIList *
medusa_uri_list_new (const char *root_directory,
		     MedusaLogLevel log_level,
		     const char *uri_list_name,
		     MedusaHash *file_names,
		     MedusaHash *directory_names) 
{
	MedusaURIList *uri_list;
	MedusaRDBFile *uri_file;

	g_return_val_if_fail (access (uri_list_name, X_OK) == -1, NULL);
	uri_list = g_new0 (MedusaURIList, 1);
	uri_file = medusa_rdb_file_new (uri_list_name, URI_LIST_METAINFO_LENGTH);

	medusa_rdb_file_add_field (uri_file,
				   MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE,
				   MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_SIZE,
				   (MedusaRDBEncodeFunc) medusa_uri_list_directory_name_encode,
				   (MedusaRDBDecodeFunc) medusa_uri_list_directory_name_decode);
	medusa_rdb_file_add_field (uri_file,
				   MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE,
				   MEDUSA_URI_LIST_FILE_NAME_FIELD_SIZE,
				   (MedusaRDBEncodeFunc) medusa_uri_list_filename_encode,
				   (MedusaRDBDecodeFunc) medusa_uri_list_filename_decode );
	uri_list->uri_names = medusa_rdb_table_all_rows (uri_file);
	
	write_index_info_to_database (uri_list->uri_names,
				      MEDUSA_INDEX_FORMAT_VERSION,
				      time (NULL));
				  
	uri_list->uri_names = medusa_rdb_table_all_rows (uri_file);

	uri_list->file_names = file_names;
	uri_list->directory_names = directory_names;
	uri_list->root_directory = g_strdup (root_directory);
	uri_list->clauses = uri_list_clauses ();
	uri_list->log_level = log_level;
	uri_list->ref_count = 1;
	uri_list->indexing_start_time = time (NULL);
	uri_list->version = g_strdup (MEDUSA_INDEX_FORMAT_VERSION);

	return uri_list;
}

MedusaURIList *
medusa_uri_list_open (const char *root_directory,
		     MedusaLogLevel log_level,
		     const char *uri_list_name,
		     MedusaHash *file_names,
		     MedusaHash *directory_names) 
{
	MedusaURIList *uri_list;
	MedusaRDBFile *uri_file;

	g_return_val_if_fail (access (uri_list_name, R_OK) != -1, NULL);

	uri_list = g_new0 (MedusaURIList, 1);
	uri_file = medusa_rdb_file_open (uri_list_name, URI_LIST_METAINFO_LENGTH);

	uri_file->field_info = uri_list_field_info ();
	uri_list->uri_names = medusa_rdb_table_all_rows (uri_file);
	read_index_info_from_database (uri_list);

	uri_list->uri_names = medusa_rdb_table_all_rows (uri_file);

	uri_list->file_names = file_names;
	uri_list->directory_names = directory_names;
	uri_list->root_directory = g_strdup (root_directory);
	uri_list->clauses = uri_list_clauses ();
	uri_list->log_level = log_level;
	uri_list->ref_count = 1;
	uri_list->indexing_start_time = time (NULL);
	uri_list->version = g_strdup (MEDUSA_INDEX_FORMAT_VERSION);

	return uri_list;
}	      



char *
medusa_uri_number_to_uri (MedusaURIList *uri_list,
			  int uri_number)
{
	char *directory_name, *file_name, *full_file_name;
	char *record;

	directory_name = g_new0 (char, NAME_MAX);
	file_name = g_new0 (char, NAME_MAX);

	record = medusa_rdb_record_number_to_record (uri_list->uri_names, uri_number);
	medusa_rdb_record_get_field_value
		(record,
		 uri_list->uri_names->file->field_info,
		 MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE,
		 uri_list,
		 file_name);
	medusa_rdb_record_get_field_value
		(record,
		 uri_list->uri_names->file->field_info,
		 MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE,
		 uri_list,
		 directory_name);

	/* Test for bug 1667 recurring */
	g_return_val_if_fail (strlen (directory_name) > 0, NULL);
  
	full_file_name = medusa_full_uri_from_directory_and_file (directory_name, file_name);
  
	g_free (directory_name);
	g_free (file_name);

	return full_file_name;


}

MedusaRDBFieldInfo * 
medusa_uri_list_get_field_info (MedusaURIList *uri_list)
{
	return uri_list->uri_names->file->field_info;
}

MedusaQueryClauses *
medusa_uri_list_get_query_clauses (MedusaURIList *uri_list)
{
	return uri_list->clauses;
}

int 
medusa_uri_list_number_of_uris (MedusaURIList *uri_list)
{
	return medusa_rdb_table_get_number_of_records (uri_list->uri_names);
}

void
medusa_uri_list_ref (MedusaURIList *uri_list)
{
	g_assert (uri_list->ref_count > 0);
	uri_list->ref_count++;
}
				      
  
void
medusa_uri_list_unref (MedusaURIList *uri_list)
{
	g_assert (uri_list->ref_count > 0);
	if (uri_list->ref_count == 1) {
		medusa_uri_list_destroy (uri_list);
	}
	else {
		uri_list->ref_count--;
	}
  
}


void
medusa_uri_list_index_file (MedusaURIList *uri_list, 
			    char *directory_name, 
			    char *file_name)
			    
{
	MedusaRDBEncodeFunc directory_encode, file_encode;
	char *record;
  
	record = g_new0 (char, uri_list->uri_names->file->field_info->record_size);

	directory_encode = medusa_rdb_field_get_encoder (uri_list->uri_names->file->field_info,
	
							 MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE);
	directory_encode (record, directory_name, uri_list);
  
	file_encode = medusa_rdb_field_get_encoder (uri_list->uri_names->file->field_info,
						    MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE);
	file_encode (&record[MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_SIZE], file_name, uri_list);

	medusa_rdb_table_insert (uri_list->uri_names, record);

	g_free (record);
}


void
medusa_uri_list_update_file (MedusaURIList *uri_list, 
			     char *directory_name, 
			     char *file_name,
			     MedusaDBUpdateAction action)
{
  
  
}



static MedusaRDBFieldInfo *
uri_list_field_info (void)
{
	MedusaRDBFieldInfo *field_info;

	field_info = medusa_rdb_field_info_new ();
	medusa_rdb_field_add (field_info,
			      MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE,
			      MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_SIZE,
			      (MedusaRDBEncodeFunc) medusa_uri_list_directory_name_encode,
			      (MedusaRDBDecodeFunc) medusa_uri_list_directory_name_decode);

	medusa_rdb_field_add (field_info,
			      MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE,
			      MEDUSA_URI_LIST_FILE_NAME_FIELD_SIZE,
			      (MedusaRDBEncodeFunc) medusa_uri_list_filename_encode,
			      (MedusaRDBDecodeFunc) medusa_uri_list_filename_decode);
	return field_info;
}

static MedusaQueryClauses *
uri_list_clauses (void)
{
	MedusaQueryClauses *clauses;

	clauses = medusa_query_clauses_new ();
	medusa_query_clauses_add_clause (clauses,
					 "file_name",
					 "is",
					 (MedusaQueryFunc) medusa_uri_list_is_named,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "file_name",
					 "contains",
					 (MedusaQueryFunc) medusa_uri_list_has_name_containing,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "file_name",
					 "does_not_contain",
					 (MedusaQueryFunc) medusa_uri_list_has_name_not_containing,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "file_name",
					 "regexp_matches",
					 (MedusaQueryFunc) medusa_uri_list_has_name_regexp_matching,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "file_name",
					 "matches",
					 (MedusaQueryFunc) medusa_uri_list_has_name_glob_matching,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "file_name",
					 "begins_with",
					 (MedusaQueryFunc) medusa_uri_list_has_file_name_starting_with,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "file_name",
					 "ends_with",
					 (MedusaQueryFunc) medusa_uri_list_has_file_name_ending_with,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "file_name",
					 "matches_regexp",
					 (MedusaQueryFunc) medusa_uri_list_has_name_regexp_matching,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "directory_name",
					 "is",
					 (MedusaQueryFunc) medusa_uri_list_is_in_directory,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "directory_name",
					 "contains",
					 (MedusaQueryFunc) medusa_uri_list_is_in_directory_containing,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "directory_name",
					 "regexp_matches",
					 (MedusaQueryFunc) medusa_uri_list_is_in_directory_regexp_matching,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "directory_name",
					 "is_in",
					 (MedusaQueryFunc) medusa_uri_list_is_in_directory_tree,
					 MEDUSA_ARGUMENT_TYPE_STRING);
	medusa_query_clauses_add_clause (clauses,
					 "directory_name",
					 "does_not_regexp_match",
					 (MedusaQueryFunc) medusa_uri_list_is_not_in_directory_regexp_matching,
					 MEDUSA_ARGUMENT_TYPE_STRING);

	return clauses;
}

static void
medusa_uri_list_destroy (MedusaURIList *uri_list)
{

	g_assert (uri_list->ref_count == 1);
	write_index_info_to_database (uri_list->uri_names,
				      uri_list->version,
				      uri_list->indexing_start_time);
	medusa_rdb_table_free (uri_list->uri_names);
	medusa_query_clauses_unref (uri_list->clauses);
	medusa_hash_unref (uri_list->file_names);
	medusa_hash_unref (uri_list->directory_names);
	g_free (uri_list->version);
	g_free (uri_list->root_directory);
	g_free (uri_list);
  
}


static void
write_index_info_to_database (MedusaRDBTable *table, char *version,  
			      int last_index_time)
{
        char *metainfo;
        metainfo = g_new0 (char, table->file->metainfo_length);
        sprintf(metainfo,"%s%c%d%c",version,0,last_index_time,0);
	medusa_rdb_file_set_metainfo (table->file, metainfo);
        g_free (metainfo);
}



/* Gets the first record, and interprets it as db information */
static void
read_index_info_from_database (MedusaURIList *uri_list)

{
	char *metainfo, *last_index_time_location;

	metainfo = medusa_rdb_file_get_metainfo (uri_list->uri_names->file);
  
	g_return_if_fail (metainfo != NULL);
	
	uri_list->version = g_strdup (metainfo);

	last_index_time_location = metainfo;
	while (*last_index_time_location) {
		last_index_time_location++;
	}
	last_index_time_location++;

	uri_list->last_index_time = (int) strtol (last_index_time_location, NULL, 10);
	
	g_free (metainfo);

}

