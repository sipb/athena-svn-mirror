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

/* medusa-master-db.c:  The top level database 
   that manages all of the indexes (the file index and the text index) */

#include <config.h>
#include "medusa-master-db.h"

#include <dirent.h>
#include <glib.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-ops.h>	
#include <libgnomevfs/gnome-vfs-types.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <libmedusa/medusa-index-progress.h>
#include <libmedusa/medusa-unsearched-locations.h>
#include <libmedusa/medusa-utils.h>

#include "medusa-conf.h"
#include "medusa-file-index.h"
#include "medusa-master-db-private.h"
#include "medusa-rdb-file.h"
#include "medusa-rdb-table.h"
#include "medusa-search-uri.h"
#include "medusa-text-index.h"
#include "medusa-text-index-queries.h"
#include "medusa-uri-list.h"
#include "medusa-uri-list-private.h"


#define TEXT_INDEX_ON
#define FREED_URI_MARKER -1
#define END_OF_URI_LIST_MARKER 0

typedef struct {
	gint32 *uris;
	gboolean uris_contain_content;
	int number_of_uris;
} MedusaContentResults;

typedef struct {
	MedusaMasterDB *db;
	const char *path;
	uid_t uid;
} IndexMetafileParameters;

static void                  index_directory                              (MedusaMasterDB       *master_db,
									   char                 *directory_name);
static void                  index_file                                   (MedusaMasterDB       *master_db,
									   char                 *directory_name,
									   char                 *file_name,
									   GnomeVFSFileInfo     *file_info);
static GList *               run_query_on_content_results                 (MedusaMasterDB       *master_db,
									   GList                *criteria,
									   MedusaContentResults *content_results);
static GList *               run_query_on_everything_but_content_results  (MedusaMasterDB       *master_db,
									   GList                *criteria,
									   MedusaContentResults *content_results);
static GList *               run_query_on_all_uris                        (MedusaMasterDB       *master_db,
									   GList                *criteria);
						       
/* Run a set of queries on a uri, and if the uri fits, add it to the current results */
static GList *               append_uri_to_results_if_matches             (GList                *result_list,
									   GList                *clause_closures,
									   MedusaMasterDB       *master_db,
									   int                   uri_number);
static gboolean              query_execute_from_clause_closure            (MedusaClauseClosure  *clause_closure,
									   int                   uri_number);
static MedusaContentResults *content_requests_to_uri_numbers              (MedusaMasterDB       *master_db,
									   GList                *clause_closures);
static void                  merge_new_uris_with_current_results_and_free (MedusaContentResults *old_results,
									   gint32               *new_uris,
									   int                   number_of_new_uris,
									   gboolean              new_uris_are_inclusion_uris);
static void                  index_public_metafile                        (MedusaMasterDB       *master_db,
									   const char           *directory_uri);
static void                  index_private_metafiles                      (MedusaMasterDB       *master_db);
static void                  medusa_master_db_destroy                     (MedusaMasterDB       *db);

MedusaMasterDB *
medusa_master_db_open (const char     *root_uri,
		       MedusaLogLevel  log_level,
		       const char     *uri_list,
		       const char     *file_attribute_index,
		       const char     *file_name_hash,
		       const char     *directory_name_hash,
		       const char     *mime_type_hash,
		       const char     *keyword_sets_file_name,
		       const char     *text_index_start_file,
		       const char     *text_index_location_file,
		       const char     *text_index_word_file)
{
	MedusaMasterDB *master_db;
	MedusaHash *file_names, *directory_names;

	/* Add check for all files existing here */

	master_db = g_new0 (MedusaMasterDB, 1);
	file_names = medusa_hash_open (file_name_hash,
				       FILE_HASH_BITS);

	directory_names = medusa_hash_open (directory_name_hash,
					    DIRECTORY_HASH_BITS);
	if (file_names == NULL || directory_names == NULL) {
		g_free (master_db);
		/* FIXME: log error here */
		return NULL;
	}
	master_db->uri_list = medusa_uri_list_open (root_uri, log_level, uri_list, file_names, directory_names);
	g_return_val_if_fail (master_db->uri_list != NULL, NULL);
	master_db->text_index = medusa_text_index_open (text_index_start_file,
							log_level,
							text_index_location_file,
							text_index_word_file);
	g_return_val_if_fail (master_db->text_index != NULL, NULL);
	master_db->file_system_db = medusa_file_system_db_open
		(ROOT_DIRECTORY, file_attribute_index,
		 file_names, directory_names,
		 mime_type_hash, keyword_sets_file_name);
	g_return_val_if_fail (master_db->file_system_db != NULL, NULL);
	medusa_hash_ref (file_names);
	medusa_hash_ref (directory_names);
	master_db->root_uri = g_strdup (root_uri);
	
	master_db->log_level = log_level;
	master_db->indexing_mode = FALSE;
	master_db->ref_count = 1;
	return master_db;

}

MedusaMasterDB *
medusa_master_db_new (const char *root_uri,
		      MedusaLogLevel log_level,
		      const char *uri_list,
		      const char *file_attribute_index,
		      const char *file_name_hash,
		      const char *directory_name_hash,
		      const char *mime_type_hash,
		      const char *keyword_sets_file_name,
		      const char *text_index_start_file,
		      const char *text_index_location_file,
		      const char *text_index_word_file,
		      const char *text_index_temp_file)
{
	MedusaMasterDB *master_db;
	MedusaHash *file_names, *directory_names;
	char *root_directory;

	master_db = g_new0 (MedusaMasterDB, 1);
  

	file_names = medusa_hash_new (file_name_hash,
				      FILE_HASH_BITS);
	directory_names = medusa_hash_new (directory_name_hash,
					   DIRECTORY_HASH_BITS);
	root_directory = gnome_vfs_get_local_path_from_uri (root_uri);
	master_db->indexing_mode = TRUE;
	master_db->index_progress = medusa_index_progress_new (root_directory);
	master_db->uri_list = medusa_uri_list_new (root_uri, log_level, uri_list, file_names, directory_names);
	master_db->text_index = medusa_text_index_new (text_index_start_file,
						       log_level,
						       text_index_location_file,
						       text_index_word_file,
						       text_index_temp_file);

	master_db->file_system_db = medusa_file_system_db_new
		(root_directory, file_attribute_index,
		 file_names, directory_names,
		 mime_type_hash, keyword_sets_file_name);
	g_free (root_directory);
	
	medusa_hash_ref (file_names);
	medusa_hash_ref (directory_names);
	master_db->root_uri = g_strdup (root_uri);
	
	master_db->log_level = log_level;

	master_db->ref_count = 1;

	medusa_unsearched_locations_initialize ();
	return master_db;
}	      

void
medusa_master_db_index (MedusaMasterDB *master_db)
{
	/* Make sure that the databases are empty before we start */
	g_assert (medusa_uri_list_number_of_uris (master_db->uri_list) == 0);
	g_assert (medusa_file_system_db_get_number_of_records (master_db->file_system_db) == 0);

	index_private_metafiles (master_db);
	index_directory (master_db, master_db->root_uri);
#ifdef TEXT_INDEX_ON
	medusa_text_index_finish_indexing (master_db->text_index);
#endif
	medusa_file_system_db_update_keywords (master_db->file_system_db,
					       master_db->uri_list);
}


void
medusa_master_db_update (MedusaMasterDB *master_db)
{
	/* FIXME bugzilla.eazel.com 2713: 
	   Not implemented. */
}

/* Returns a glist of uri's */
/* FIXME:  Return errors, too */
GList *
medusa_master_db_query (MedusaMasterDB *master_db,
			char *search_uri_string)
{
	MedusaParsedSearchURI *parsed_search_uri;
	GList *results;
	MedusaContentResults *content_results;
	GList *content_clauses, *non_content_clauses;


	parsed_search_uri = medusa_search_uri_parse (search_uri_string, master_db);
	if (parsed_search_uri->syntax_error_found ||
	    parsed_search_uri->query_is_always_false) {
		medusa_parsed_search_uri_free (parsed_search_uri);
		return NULL;
	}
	/* FIXME: What do we do if the query is always true? */
	/* Return nothing for uris that are invalid */
	if (parsed_search_uri->clause_closures == NULL) {
		medusa_parsed_search_uri_free (parsed_search_uri);
		return NULL;
	}

	content_clauses = medusa_g_list_partition (parsed_search_uri->clause_closures,
						   medusa_clause_closure_is_content_search,
						   NULL,
						   &non_content_clauses);
	if (content_clauses != NULL) {
		content_results = content_requests_to_uri_numbers (master_db,
								   content_clauses);
		g_assert (content_results != NULL);
		if (content_results->uris_contain_content == TRUE) {
			results = run_query_on_content_results (master_db,
								non_content_clauses,
								content_results);
		}
		else {
			results = run_query_on_everything_but_content_results (master_db,
									       non_content_clauses,
									       content_results);
		}

	}
	else {
		results = run_query_on_all_uris (master_db,
						 non_content_clauses);
	}
	/* g_list_partition consumed the original list,
	   and so we must free the clause closures out of content_clauses
	   and non_content_closures instead of out of the parsed search uri */
	medusa_clause_closure_list_free (content_clauses);
	medusa_clause_closure_list_free (non_content_clauses);
	medusa_parsed_search_uri_free (parsed_search_uri);
	return results;

}
			

static gboolean
query_execute_from_clause_closure (MedusaClauseClosure *clause_closure,
				   int uri_number)
{
	g_return_val_if_fail (clause_closure->query_func != NULL, FALSE);
	if (clause_closure->file_system_db != NULL) {
		return clause_closure->query_func (clause_closure->file_system_db,
						   medusa_rdb_record_number_to_record (clause_closure->file_system_db->file_database,
										       uri_number),
						   clause_closure->argument);
	}
	else {
		return clause_closure->query_func (clause_closure->uri_list,
						   medusa_rdb_record_number_to_record (clause_closure->uri_list->uri_names,
										       uri_number),
						   clause_closure->argument);
	}
}
				
				       

static void
index_directory (MedusaMasterDB *master_db,
		 char *directory_uri)
{
	DIR *directory_handle;
	char *directory_path;
	struct dirent *directory_entry;
	char *file_uri;
	GnomeVFSFileInfo *file_info;
	GnomeVFSResult result;
  
	directory_path = gnome_vfs_get_local_path_from_uri (directory_uri);
	g_return_if_fail (directory_path != NULL);
	
	/* If the directory is unreadable, don't bother */
	directory_handle = opendir (directory_path);
	if (directory_handle == NULL) {
		g_free (directory_path);
		return;
	}
	
	while ((directory_entry = readdir (directory_handle))) {
		if (!strcmp (directory_entry->d_name,".") || 
		    !strcmp (directory_entry->d_name,"..")) {
			continue;
		}
		
		file_uri = medusa_full_uri_from_directory_and_file (directory_uri,
								    directory_entry->d_name);
		
		if (medusa_is_unsearched_location (file_uri)) {
			if (master_db->log_level == MEDUSA_DB_LOG_ABBREVIATED ||
			    master_db->log_level == MEDUSA_DB_LOG_EVERYTHING) {
				printf ("Skipping file %s\n", file_uri);
			}
			g_free (file_uri);
			continue;
		}
		file_info = gnome_vfs_file_info_new ();
		result = gnome_vfs_get_file_info (file_uri,
						  file_info,
						  GNOME_VFS_FILE_INFO_GET_MIME_TYPE);
		if (result != GNOME_VFS_OK) {
			gnome_vfs_file_info_unref (file_info);
			g_free (file_uri);
			continue;
		}

		index_file (master_db, directory_uri, directory_entry->d_name, file_info);

		if (file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			gnome_vfs_file_info_unref (file_info);
			index_directory (master_db, file_uri);
		}
		else {
			gnome_vfs_file_info_unref (file_info);
			if (strcmp (directory_entry->d_name, MEDUSA_PUBLIC_METAFILE_NAME) == 0) {
				index_public_metafile (master_db, directory_uri);
			}
		}
    
		g_free (file_uri);
	}
	closedir (directory_handle);

	g_free (directory_path);
}


static void
index_file (MedusaMasterDB *master_db,
	    char *directory_uri,
	    char *file_name,
	    GnomeVFSFileInfo *file_info)
{
	char *uri;
	int uri_number;
	
	uri_number = medusa_uri_list_number_of_uris (master_db->uri_list);
	medusa_uri_list_index_file (master_db->uri_list, directory_uri, file_name);
	uri = medusa_uri_number_to_uri (master_db->uri_list, uri_number);
	if (master_db->log_level == MEDUSA_DB_LOG_EVERYTHING) {
		printf ("Indexing name and attributes of %s\n",uri);
	}
	if (master_db->log_level == MEDUSA_DB_LOG_ABBREVIATED) {
		printf ("%s\n",uri);
	}
	medusa_file_system_db_index_file (master_db->file_system_db, 
					  uri_number,
					  file_info);
#ifdef TEXT_INDEX_ON
	medusa_text_index_read_file (master_db->text_index,
				     uri,
				     uri_number,
				     file_info);
#endif
	medusa_index_progress_update (master_db->index_progress,
				      file_info->size);
	g_free (uri);
}

static GList *                  
run_query_on_content_results (MedusaMasterDB *master_db,
			      GList *criteria,
			      MedusaContentResults *content_results)
{
	int i;
	GList *results;

	results = NULL;
	for (i = 0; i < content_results->number_of_uris; i++) {
		if (master_db->log_level == MEDUSA_DB_LOG_EVERYTHING) {
			printf ("Trying result %d\n", content_results->uris[i]);
		}
		results = append_uri_to_results_if_matches (results,
							    criteria,
							    master_db,
							    content_results->uris[i]);
	}
	return results;
}

static GList *                  
run_query_on_everything_but_content_results  (MedusaMasterDB *master_db,
					      GList *criteria,
					      MedusaContentResults *content_results)
{
	int i, content_uri_position, number_of_uris;
	GList *results;

	results = NULL;
	content_uri_position = 0;
	number_of_uris = medusa_uri_list_number_of_uris (master_db->uri_list);
	/* Search everything but the content results */
	for (i = number_of_uris - 1; i > 0; i--) {
		if (content_results->uris[content_uri_position] == i) {
			content_uri_position++;
			continue;
		}
		results = append_uri_to_results_if_matches (results,
							    criteria,
							    master_db,
							    i);
	}
	return results;
}

static GList *                  
run_query_on_all_uris (MedusaMasterDB *master_db,
		       GList *criteria)
{
	int i, number_of_uris;
	GList *results;

	results = NULL;
	number_of_uris = medusa_uri_list_number_of_uris (master_db->uri_list);
	for (i = number_of_uris - 1; i > 0 ; i--) {
		results = append_uri_to_results_if_matches (results,
							    criteria,
							    master_db,
							    i);
	}
	return results;
}


static GList *     
append_uri_to_results_if_matches (GList *result_list,
				  GList *clause_closures,
				  MedusaMasterDB *master_db,
				  int uri_number)
{
	gboolean query_result;
	GList *next_clause;
	
	query_result = TRUE;
	/* Need two cases: the first is for non-partitioned lists, the second for 
	   parititioned lists */
	for (next_clause = clause_closures; next_clause != NULL && next_clause->data != NULL; next_clause = next_clause->next) {
		
		if (query_execute_from_clause_closure (next_clause->data, uri_number) == FALSE) {
			query_result = FALSE;
			break;
		}
	}
	if (query_result) {
		result_list = g_list_prepend (result_list, 
					      medusa_uri_number_to_uri (master_db->uri_list,
									uri_number));
	}
	return result_list;
		
}

static MedusaContentResults *    
content_requests_to_uri_numbers (MedusaMasterDB *master_db,
				 GList *clause_closures)
{
	MedusaClauseClosure *closure, *new_closure_data;
	GList *new_closure;
	gint32 *new_uris;
	int number_of_new_uris;
	MedusaContentResults *results;
	

	g_return_val_if_fail (g_list_length (clause_closures) > 0, NULL);
	closure = clause_closures->data;
	g_assert (closure->is_content_request == TRUE &&
		  (closure->return_matches == TRUE ||
		   closure->return_matches == FALSE));
	results = g_new0 (MedusaContentResults, 1);
	results->uris_contain_content = closure->return_matches;
	results->uris = medusa_text_index_get_uris_for_criterion (master_db->text_index,
								  closure->content_request,
								  closure->match_all_words,
								  &results->number_of_uris);
	/* No intersection work to do if there is only one criterion */
	if (g_list_length (clause_closures) == 1) {
		return results;
	}
	/* Otherwise, keep intersecting the results with the new results */
	for (new_closure = clause_closures->next; 
	     new_closure != NULL; 
	     new_closure = new_closure->next) {
		new_closure_data = (MedusaClauseClosure *) new_closure->data;
		new_uris = medusa_text_index_get_uris_for_criterion (master_db->text_index,
								     new_closure_data->content_request,
								     new_closure_data->match_all_words,
								     &number_of_new_uris);
		merge_new_uris_with_current_results_and_free (results,
							      new_uris,
							      number_of_new_uris,
							      new_closure_data->return_matches);
						     
	}
	return results;
}

static void
merge_new_uris_with_current_results_and_free (MedusaContentResults *old_results,
					      gint32 *new_uris,
					      int number_of_new_uris,
					      gboolean new_uris_are_inclusion_uris)
{
	gint32 *merged_results;
	int number_of_merged_results;
	/* Several ways to merge here, depending on whether the
	   content lists are for inclusion or for exclusion.
	   We assume intersection of the criteria */

	/* This function will break if these aren't the case,
	   so check now */
	g_assert (old_results->uris_contain_content == TRUE ||
		  old_results->uris_contain_content == FALSE);
	g_assert (new_uris_are_inclusion_uris == TRUE ||
		  new_uris_are_inclusion_uris == FALSE);
	
	merged_results = NULL;
	/* Case 1: both the current results and the new uris are positive 
	   content requests, or both are negative content requests.
	   Just merge the two lists.  
	   We can do this merge linearly, because the text index
	   results are in descending order */ 
	if (old_results->uris_contain_content == new_uris_are_inclusion_uris) {
	        merged_results = medusa_intersect_two_descending_integer_lists (old_results->uris,
										old_results->number_of_uris,
										new_uris,
										number_of_new_uris,
										&number_of_merged_results);
		
	}
	/* Case two, remove results, if the new results are things _not_ to be included,
	   and the old are for inclusion */
	if (old_results->uris_contain_content && !new_uris_are_inclusion_uris) {
		merged_results = medusa_difference_of_two_descending_integer_lists (old_results->uris,
										    old_results->number_of_uris,
										    new_uris,
										    number_of_new_uris,
										    &number_of_merged_results);
	}
	
	/* Case three, is the reflection of two */
	if (!old_results->uris_contain_content && new_uris_are_inclusion_uris) {
		merged_results = medusa_difference_of_two_descending_integer_lists (new_uris,
										    number_of_new_uris,
										    old_results->uris,
										    old_results->number_of_uris,
										    &number_of_merged_results);
		/* We need to change the negative inclusion to a positive one now */
		old_results->uris_contain_content = TRUE;
	}

	g_free (old_results->uris);  
	g_free (new_uris);  
	
	old_results->uris = merged_results;
	old_results->number_of_uris = number_of_new_uris;

}

void
medusa_master_db_ref (MedusaMasterDB *master_db)
{
	g_assert (master_db->ref_count > 0);
	master_db->ref_count++;

}				       
 
void
medusa_master_db_unref (MedusaMasterDB *master_db)
{
	g_assert (master_db->ref_count > 0);
	if (master_db->ref_count == 1) {
		medusa_master_db_destroy (master_db);
	}
	else {
		master_db->ref_count--;
	}
}

static void
medusa_master_db_destroy (MedusaMasterDB *master_db)
{
	g_assert (master_db->ref_count == 1);

	medusa_uri_list_unref (master_db->uri_list);
	medusa_file_system_db_free (master_db->file_system_db);
	medusa_text_index_unref (master_db->text_index);
	medusa_unsearched_locations_shutdown ();
	if (master_db->indexing_mode) {
		medusa_index_progress_destroy (master_db->index_progress);
	}
	medusa_search_uri_parse_shutdown ();
	g_free (master_db->root_uri);
	g_free (master_db);
}

static void
add_public_keywords_to_index (const char *file_name,
			      GList *keywords,
			      gpointer callback_data)
{
	IndexMetafileParameters *parameters;
	char *file_path;
	const char *keyword;
	GList *p;

	parameters = callback_data;

	file_path = medusa_full_path_from_directory_and_file
		(parameters->path, file_name);

	for (p = keywords; p != NULL; p = p->next) {
		keyword = (const char *) p->data;

		medusa_file_system_db_add_public_keyword
			(parameters->db->file_system_db,
			 file_path, keyword);
	}
	
	g_free (file_path);
}

static void
add_private_keywords_to_index (const char *file_name,
			       GList *keywords,
			       gpointer callback_data)
{
	IndexMetafileParameters *parameters;
	char *file_path;
	const char *keyword;
	GList *p;

	parameters = callback_data;

	file_path = medusa_full_path_from_directory_and_file
		(parameters->path, file_name);

	for (p = keywords; p != NULL; p = p->next) {
		keyword = (const char *) p->data;

		medusa_file_system_db_add_private_keyword
			(parameters->db->file_system_db,
			 file_path, keyword, parameters->uid);
	}
	
	g_free (file_path);
}

static void
index_public_metafile (MedusaMasterDB *db,
		       const char *directory_uri)
{
	char *directory_path, *metafile_path;
	IndexMetafileParameters parameters;

	/* For each item in the metafile, we locate any keywords and
	 * add them to an index. For now, the keywords go into a
	 * single hash table, indexed by the name of the file the
	 * keywords are attached to.
	 */

	directory_path = gnome_vfs_get_local_path_from_uri (directory_uri);
	if (directory_path == NULL) {
		return;
	}
	metafile_path = medusa_full_path_from_directory_and_file
		(directory_path, MEDUSA_PUBLIC_METAFILE_NAME);

	parameters.db = db;
	parameters.path = directory_path;

	medusa_extract_keywords_from_metafile
		(metafile_path, add_public_keywords_to_index, &parameters);

	g_free (directory_path);
	g_free (metafile_path);
}

static char *
get_path_from_private_metafile_name (const char *name)
{
	int length;
	char *name_without_xml, *uri, *local_path;

	/* Remove ".xml" at end. */
	length = strlen (name);
	if (!medusa_str_has_suffix (name, MEDUSA_PRIVATE_METAFILE_SUFFIX)) {
		return NULL;
	}
	name_without_xml = g_strndup (name, length
				      - strlen (MEDUSA_PRIVATE_METAFILE_SUFFIX));

	/* Unescape once to turn % and / back to normal. */
	uri = gnome_vfs_unescape_string (name_without_xml, NULL);
	g_free (name_without_xml);

	/* Convert to local path. */
	local_path = gnome_vfs_get_local_path_from_uri (uri);
	g_free (uri);

	return local_path;
}

static void
index_private_metafile (MedusaMasterDB *db,
			uid_t uid,
			const char *metafile_path,
			const char *target_path)
{
	GnomeVFSResult result;
	IndexMetafileParameters parameters;

	/* For each item in the metafile, we locate any keywords and
	 * add them to the keyword set tagged with this user.
	 */
	parameters.db = db;
	parameters.path = target_path;
	parameters.uid = uid;
	result = medusa_extract_keywords_from_metafile
		(metafile_path, add_private_keywords_to_index, &parameters);

	/* If there was a metafile, add this directory, since even
	 * files in this directory that have no keywords in the
	 * metafile get (the lack of) keywords from this metafile.
	 */
	if (result == GNOME_VFS_OK) {
		medusa_file_system_db_add_private_keywords_directory
			(db->file_system_db, target_path, uid);
	}
}

static void
index_private_metafiles_for_user (MedusaMasterDB *db,
				  uid_t uid,
				  const char *home_directory_path)
{
	char *metafiles_directory_path;
	DIR *dir;
	struct dirent *file;
	char *target_path, *metafile_path;

	metafiles_directory_path = g_strconcat (home_directory_path,
						"/.nautilus/metafiles",
						NULL);

	dir = opendir (metafiles_directory_path);
	if (dir == NULL) {
		g_free (metafiles_directory_path);
		return;
	}
	while ((file = readdir (dir)) != NULL) {
		target_path = get_path_from_private_metafile_name (file->d_name);
		if (target_path != NULL) {
			metafile_path = medusa_full_path_from_directory_and_file
				(metafiles_directory_path, file->d_name);
			index_private_metafile
				(db, uid, metafile_path, target_path);
			g_free (metafile_path);
			g_free (target_path);
		}
	}
	closedir (dir);

	g_free (metafiles_directory_path);
}

static void
index_private_metafiles (MedusaMasterDB *db)
{
	struct passwd *user;

	/* Index the private metafiles of all users on the system. */
	setpwent ();
	while ((user = getpwent ()) != NULL) {
		if (user->pw_dir != NULL) {
			index_private_metafiles_for_user
				(db, user->pw_uid, user->pw_dir);
		}
	}
	endpwent ();
	g_free (user);
}
