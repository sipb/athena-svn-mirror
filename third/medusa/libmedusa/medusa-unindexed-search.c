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
 *  
 */


/* medusa-unindexed-search.h -- Service to run a find that backs up an
 indexed search */

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <glib.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "medusa-file-info-utilities.h"
#include "medusa-service-private.h"
#include "medusa-unindexed-search.h"
#include "medusa-unsearched-locations.h"
#include "medusa-utils.h"

typedef int MedusaEmblemData;

#define UNINDEXED_RESULTS_FILE "/tmp/unindexed-results-"
#define END_OF_UNINDEXED_SEARCH_RESULTS "--End--\n"

struct MedusaUnindexedSearch {
        /* FIXME bugzilla.eazel.com 2988:  Perhaps we only need the
           partitioned criteria? */
        /* Parsed list of criteria */
        GList *criteria;
        /* Criteria that are easy to match */
        GList *name_criteria;
        /* Criteria that require stat info */
        GList *inode_criteria;
        /* Criteria that require mime type */
        GList *mime_type_criteria;
        /* Criteria that require Nautilus keywords information */
        GList *keywords_criteria;

        /* Root directory for the search */
        char *root_directory;

        /* Place to put results */
        char *output_file_name;
        int output_fd;
        int read_fd;
        char *buffer;

        /* Keep a boolean for a bad / illogical search,
           so we know not to bother looking */
        gboolean invalid_search;

        /* Keep field about whether we should
           try and open everything as a directory,
           or whether the search will stat the files and tell us */
        gboolean search_discovers_file_type;
        
        gboolean search_is_running;
        
};


static char *                  get_root_directory_from_uri    (const char *uri);
static char *                  uri_to_search_string           (const char *uri);
static void                    assume_file_is_directory_unless_a_link (const char *uri,
                                                                       gboolean *file_is_directory);
static GList *                 search_string_to_criteria      (char *search_string);
static void                    search_directory               (char *directory, 
                                                               MedusaUnindexedSearch *search);
static gboolean                file_matches_criteria          (const char *file_uri,
                                                               MedusaUnindexedSearch *search,
                                                               gboolean *file_is_directory,
                                                               GHashTable *keywords_hash_table,
                                                               gboolean directory_has_keywords) ;

static gboolean                matches_all_name_criteria      (GList *name_criteria,
                                                               const char *file_uri,
                                                               MedusaUnindexedSearch *search);
static gboolean                matches_all_file_info_criteria (GList *name_criteria,
                                                               const char *file_uri,
                                                               MedusaUnindexedSearch *search,
                                                               gboolean *file_is_directory);

static gboolean                matches_all_mime_criteria      (GList *name_criteria,
                                                               const char *file_uri,
                                                               MedusaUnindexedSearch *search,
                                                               gboolean *file_is_directory);

static gboolean                matches_all_keywords_criteria  (GList *name_criteria,
                                                               const char *file_uri,
                                                               GHashTable *keywords_hash_table,
                                                               gboolean directory_has_keywords);

static gboolean                matches_one_name                 (const char *criterion,
                                                               const char *file_uri);
static gboolean                matches_one_file_info            (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                matches_one_mime_type            (const char *criterion,
                                                               const char *file_uri,
                                                               const char *mime_type);
static gboolean                matches_one_keywords_criterion (const char *criterion,
                                                               const char *file_uri,
                                                               GHashTable *keywords_hash_table,
                                                               gboolean keywords_hash_table_exists_for_directory);
static gboolean                matches_date_modified_criterion  (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                matches_owner_criterion          (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                matches_group_criterion          (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                matches_size_criterion           (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                criterion_is_name_criterion    (gpointer criterion_string,
                                                               gpointer user_data);
static gboolean                criterion_is_inode_criterion   (gpointer criterion_string,
                                                               gpointer user_data);
static gboolean                criterion_is_mime_criterion    (gpointer criterion_string,
                                                               gpointer user_data);
static gboolean                criterion_is_keywords_criterion(gpointer criterion_string,
                                                               gpointer user_data);
static gboolean                criterion_field_is             (const char *criterion,
                                                               const char *field_name);   
static int                     initialize_results_file_for_writing  (char *file_name);
static int                     initialize_results_file_for_reading  (char *file_name);
/* FIXME bugzilla.eazel.com 3003: 
   Perhaps we should use the gnome vfs functions that 
   do the same things */
static char *                  uri_to_file_name               (const char *uri);
static char *                  uri_to_path_name               (const char *uri);

static char *                  match_string_to_date           (const char *match_string);

static const char *            second_word_of                 (const char *criterion);
static const char *            last_word_of                   (const char *criterion);

MedusaUnindexedSearch *
medusa_unindexed_search_new (GnomeVFSResult *result,
                             const char *uri)
{
        char *search_string;
        GList *other_criteria;
        MedusaUnindexedSearch *search;
  
        search = g_new0 (MedusaUnindexedSearch, 1);
        search->root_directory = get_root_directory_from_uri (uri);
        search_string = uri_to_search_string (uri);
        /* Avoid case issues now */
        g_strdown (search_string);
        search->criteria = search_string_to_criteria (search_string);
        g_free (search_string);
        /* FIXME bugzilla.eazel.com 3004: 
           We should do criteria preprocessing here,
           instead of processing each criterion for each file */
        /* Partition criteria into various difficulties so
           we can look at the easiest ones first */
        search->name_criteria = medusa_g_list_partition (search->criteria,
                                                         criterion_is_name_criterion,
                                                         NULL,
                                                         &other_criteria);
        search->inode_criteria = medusa_g_list_partition (other_criteria,
                                                          criterion_is_inode_criterion,
                                                          NULL,
                                                          &other_criteria);
        search->mime_type_criteria = medusa_g_list_partition (other_criteria,
                                                              criterion_is_mime_criterion,
                                                              NULL,
                                                              &other_criteria);
        search->keywords_criteria = medusa_g_list_partition (other_criteria,
                                                             criterion_is_keywords_criterion,
                                                             NULL,
                                                             &other_criteria);
        search->search_discovers_file_type = ((search->inode_criteria != NULL) ||
                                              (search->mime_type_criteria != NULL));
        /* For now, we can tell a search won't work
           if it contains criteria we don't know about */
        search->invalid_search = (g_list_length (other_criteria) > 0);

        /* Initialize other side of the search connection to
           write results to */
        search->output_file_name = g_strdup_printf ("%s.%lu", UNINDEXED_RESULTS_FILE, (unsigned long) getpid ());
        search->output_fd = initialize_results_file_for_writing (search->output_file_name);
        search->read_fd = initialize_results_file_for_reading (search->output_file_name);
        if (search->read_fd == -1 || search->output_fd == -1) {
                *result = GNOME_VFS_ERROR_INTERNAL;
        }
        else {
                *result = GNOME_VFS_OK;
        }

        medusa_unsearched_locations_initialize ();
        return search;
        
}

GnomeVFSResult
medusa_unindexed_search_start_search (MedusaUnindexedSearch *search)

{
        pid_t result_process;
        
        result_process = fork ();
        if (result_process == 0) {
                search_directory (search->root_directory, search);
                printf ("Search process is complete\n");
                write (search->output_fd, 
                       END_OF_UNINDEXED_SEARCH_RESULTS,
                       strlen (END_OF_UNINDEXED_SEARCH_RESULTS));
                medusa_unindexed_search_destroy (search);
                exit (1);
        }
        close (search->output_fd);
        return GNOME_VFS_OK;
}

GnomeVFSResult         
medusa_unindexed_search_read_search_result (MedusaUnindexedSearch *search,
                                            char **result)
{ 
        char tmpbuf[513];
        const char *newline_pos;
        char *result_uri;
        int len;
        char *buffer;

        
        g_return_val_if_fail (search != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

        if (search->buffer == NULL) {
                search->buffer = g_strdup ("");
        }

        while (strchr (search->buffer, '\n') == NULL) {
                len = read (search->read_fd, &tmpbuf, 512 * sizeof (char)); 
                if (len == -1) {
                        *result = NULL;
                        return GNOME_VFS_ERROR_DIRECTORY_BUSY;
                }
                tmpbuf[len] = '\0';
                buffer = g_strconcat (search->buffer, tmpbuf, NULL);
                g_free (search->buffer);
                search->buffer = buffer;
        }

        
        /* grab a result */
        newline_pos = strchr (search->buffer, '\n');
        result_uri = g_strndup (search->buffer, newline_pos - search->buffer);
        g_return_val_if_fail (result_uri != NULL, GNOME_VFS_ERROR_NOT_FOUND);

        /* move buffer forward */
        buffer = g_strdup (newline_pos + 1);
        g_free (search->buffer);
        search->buffer = buffer;

        if (strcmp (result_uri, END_OF_UNINDEXED_SEARCH_RESULTS) == 0) {
                return GNOME_VFS_ERROR_EOF;
        }
        *result = result_uri;
        return GNOME_VFS_OK;
}

void                   
medusa_unindexed_search_destroy (MedusaUnindexedSearch *search)
{
        medusa_unsearched_locations_shutdown ();
        medusa_g_list_free_deep (search->criteria);
        g_free (search->root_directory);
        g_free (search);
}


static void                    
search_directory (char *directory_name, 
                  MedusaUnindexedSearch *search)
{
        DIR *directory_table;
        struct dirent *directory_entry;
        char *plain_directory_name;
        char *file_uri;
        gboolean file_is_directory;
        ssize_t write_result;
        GnomeVFSResult keywords_hash_table_result;
        gboolean directory_has_keywords;
        GHashTable *keywords_hash_table;

        /* FIXME bugzilla.eazel.com 3006:  
           This doesn't work to prevent searches 
           of files that aren't directories */
        if (medusa_is_unsearched_location (directory_name)) {
                return;
        }

        plain_directory_name = gnome_vfs_get_local_path_from_uri (directory_name);
        g_return_if_fail (plain_directory_name != NULL);
	directory_table = opendir (plain_directory_name);
	/* If the directory is unreadable, don't bother */
        if (directory_table == NULL) {
                /* This could be a whole slew of problems,
                   from not being able to read it to the directory
                   having being erased before we got here.  Let's
                   just ditch */
                g_free (plain_directory_name);
                return ;
        }
        if (search->keywords_criteria != NULL) {
                /* We need to collect the whole directory's
                   keywords before going through the directory */
                keywords_hash_table_result =
                        medusa_keyword_hash_table_create_for_directory (&keywords_hash_table,
                                                                        directory_name);
                directory_has_keywords = keywords_hash_table_result == GNOME_VFS_OK;
        }
        else {
                directory_has_keywords = FALSE;
        }

	while ((directory_entry = readdir (directory_table)) != NULL) {
		if (!strcmp (directory_entry->d_name,".") || 
		    !strcmp (directory_entry->d_name,"..")) {
			continue;
		}
		file_uri = medusa_full_uri_from_directory_and_file (directory_name,
                                                                    directory_entry->d_name);
                if (file_matches_criteria (file_uri, 
                                           search, 
                                           &file_is_directory,
                                           keywords_hash_table,
                                           directory_has_keywords)) {
#ifdef UNINDEXED_SEARCH_DEBUG
                        printf ("writing uri %s to the file\n", file_uri);
#endif
                        
                        write_result = write (search->output_fd, file_uri, strlen (file_uri));
                        
                        if (write_result == 0) {
                                /* FIXME bugzilla.eazel.com 3007: 
                                   We need to determine
                                   the correct error behavior in this case */
                                g_warning ("The results aren't getting written to the socket\n");
                        }
                        write_result = write (search->output_fd, "\n", strlen ("\n"));
                }
                /* Don't follow links because we don't want to get 
                   stuck in circular ones */
                if (file_is_directory) {
                        search_directory (file_uri, search);
                }
                g_free (file_uri);
        }
        closedir (directory_table);
        if (directory_has_keywords) {
                medusa_keyword_hash_table_destroy (keywords_hash_table);
        }
        
        g_free (plain_directory_name);
}

static gboolean
file_matches_criteria (const char *file_uri,
                       MedusaUnindexedSearch *search,
                       gboolean *file_is_directory,
                       GHashTable *keywords_hash_table,
                       gboolean directory_has_keywords) 
{

        /* If we don't need to check that the file is a directory,
           we just report that it is a directory, unless
           it's a symbolic link.  That way we don't
           follow circular links */
        /* First check file name criteria */
        if (search->name_criteria != NULL &&
            !matches_all_name_criteria (search->name_criteria,
                                        file_uri,
                                        search)) {

                assume_file_is_directory_unless_a_link (file_uri,
                                                        file_is_directory);
                return FALSE;
        }
        if (search->keywords_criteria != NULL &&
            !matches_all_keywords_criteria (search->keywords_criteria,
                                            file_uri,
                                            keywords_hash_table,
                                            directory_has_keywords)) {
                assume_file_is_directory_unless_a_link (file_uri,
                                                        file_is_directory);
                return FALSE;
        }

        if (search->inode_criteria != NULL &&
            !matches_all_file_info_criteria (search->inode_criteria,
                                             file_uri,
                                             search,
                                             file_is_directory)) {
                return FALSE;
        }
        if (search->mime_type_criteria != NULL &&
            !matches_all_mime_criteria (search->mime_type_criteria,
                                        file_uri,
                                        search,
                                        file_is_directory)) {
                return FALSE;
        }
        if (!search->search_discovers_file_type) {
                assume_file_is_directory_unless_a_link (file_uri,
                                                        file_is_directory);
        }
        return TRUE;
}

static gboolean
matches_all_name_criteria (GList *name_criteria,
                           const char *file_uri,
                           MedusaUnindexedSearch *search)
{
        GList *current_criterion;
        char *criterion_string;
        for (current_criterion = name_criteria; 
             current_criterion != NULL;
             current_criterion = current_criterion->next) {
                criterion_string = (char *) current_criterion->data;
                if (!matches_one_name (criterion_string, file_uri)) {
                        return FALSE;
                }
        }
        return TRUE;
}


static gboolean
matches_all_file_info_criteria (GList *name_criteria,
                                const char *file_uri,
                                MedusaUnindexedSearch *search,
                                gboolean *file_is_directory)
{
        GList *current_criterion;
        GnomeVFSFileInfo *file_info;
        GnomeVFSResult result;
        char *criterion_string;

        file_info = gnome_vfs_file_info_new ();
        result = gnome_vfs_get_file_info (file_uri,
                                          file_info,
                                          GNOME_VFS_FILE_INFO_DEFAULT);
        *file_is_directory = (file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY);
        if (result != GNOME_VFS_OK) {
                return FALSE;
        }

        
        for (current_criterion = name_criteria; 
             current_criterion != NULL;
             current_criterion = current_criterion->next) {
                criterion_string = (char *) current_criterion->data;
                if (!matches_one_file_info (criterion_string, file_uri, file_info)) {
                        gnome_vfs_file_info_unref (file_info);
                        return FALSE;
                }
        }
        gnome_vfs_file_info_unref (file_info);
        return TRUE;
}


static gboolean
matches_all_mime_criteria (GList *name_criteria,
                           const char *file_uri,
                           MedusaUnindexedSearch *search,
                           gboolean *file_is_directory)
{
        GList *current_criterion;
        GnomeVFSFileInfo *file_info;
        GnomeVFSResult result;
        char *criterion_string;

        file_info = gnome_vfs_file_info_new ();
        result = gnome_vfs_get_file_info (file_uri,
                                          file_info,
                                          GNOME_VFS_FILE_INFO_GET_MIME_TYPE);
        if (result != GNOME_VFS_OK) {
                return FALSE;
        }
        *file_is_directory = (file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY);
        for (current_criterion = name_criteria; 
             current_criterion != NULL;
             current_criterion = current_criterion->next) {
                criterion_string = (char *) current_criterion->data;
                if (!matches_one_mime_type (criterion_string, 
                                          file_uri, 
                                          file_info->mime_type)) {
                        gnome_vfs_file_info_unref (file_info);
                        return FALSE;
                }
        }
        gnome_vfs_file_info_unref (file_info);
        return TRUE;
}

static gboolean 
matches_all_keywords_criteria (GList *keywords_criteria,
                               const char *file_uri,
                               GHashTable *keywords_hash_table,
                               gboolean directory_has_keywords)
{
        GList *current_criterion;
        const char *criterion_string;

        for (current_criterion = keywords_criteria;
             current_criterion != NULL;
             current_criterion = current_criterion->next) {
                criterion_string = (char *) current_criterion->data;
                if (!matches_one_keywords_criterion (criterion_string,
                                                     file_uri,
                                                     keywords_hash_table,
                                                     directory_has_keywords)) {
                        return FALSE;
                }
                
        }
        return TRUE;
}


static gboolean
matches_one_name (const char *criterion,
                const char *file_uri)
            
{
        char *name;
        const char *match_string;
        const char *match_relation;
        static GHashTable *regex_patterns = NULL;
        regex_t *pattern_data;

        if (criterion_field_is (criterion, MEDUSA_FILE_NAME_CRITERION)) {
                name = uri_to_file_name (file_uri);
        }
        else {
                name = uri_to_path_name (file_uri);
        }
        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);
        if (medusa_str_has_prefix (match_relation, "is")) {
                return (strcmp (match_string, name) == 0);
        }
        if (medusa_str_has_prefix (match_relation, "is_not")) {
                return (strcmp (match_string, name) != 0);
        }
        if (medusa_str_has_prefix (match_relation, "contains")) {
                return (strstr (name, match_string) != NULL);
        }
        if (medusa_str_has_prefix (match_relation, "does_not_contain")) {
                return (strstr (name, match_string) == NULL);
        }
        if (medusa_str_has_prefix (match_relation, "begins_with")) {
                return medusa_str_has_prefix (name, match_string);
        }
        if (medusa_str_has_prefix (match_relation, "ends_with")) {
                return medusa_str_has_suffix (name, match_string);
        }
        if (medusa_str_has_prefix (match_relation, "matches")) {
                return (fnmatch (match_string, name, 0));
        }
        if (medusa_str_has_prefix (match_relation, "regexp_matches")) {
                if (regex_patterns == NULL) {
                        regex_patterns = g_hash_table_new (g_str_hash, g_str_equal);
                }
                pattern_data = g_hash_table_lookup (regex_patterns,
                                                    match_string);
                if (pattern_data == NULL) {
                        pattern_data = g_new0 (regex_t, 1);
                        regcomp (pattern_data, match_string, REG_ICASE | REG_NOSUB);
                        g_hash_table_insert (regex_patterns,
                                             g_strdup (match_string),
                                             pattern_data);
                        return (regexec (pattern_data, name, 0, NULL, 0) == 0);
                }
        }

        return FALSE;
}

static gboolean
matches_one_file_info (const char *criterion,
                     const char *file_uri,
                     GnomeVFSFileInfo *file_info)
{
        if (criterion_field_is (criterion, MEDUSA_MODIFIED_CRITERION)) {
                return matches_date_modified_criterion (criterion, file_uri, file_info);
        }
        if (criterion_field_is (criterion, MEDUSA_OWNER_CRITERION)) {
                return matches_owner_criterion (criterion, file_uri, file_info);
        }
        if (criterion_field_is (criterion, MEDUSA_GROUP_CRITERION)) {
                return matches_group_criterion (criterion, file_uri, file_info);
        }
        if (criterion_field_is (criterion, MEDUSA_SIZE_CRITERION)) {
                return matches_size_criterion (criterion, file_uri, file_info);
        }

        return FALSE;
}

static gboolean
matches_date_modified_criterion (const char *criterion,
                                 const char *file_uri,
                                 GnomeVFSFileInfo *file_info)
{
        const char *match_string;
        const char *match_relation;
        char *date;
        gboolean result;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);

        date = match_string_to_date (match_string);
        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */
        if (medusa_str_has_prefix (match_relation, "is ")) {
                result = (file_info->mtime > medusa_file_info_get_first_unix_time_occurring_on_date (date)) && 
                        (file_info->mtime <= medusa_file_info_get_last_unix_time_occurring_on_date (date));
        } else if (medusa_str_has_prefix (match_relation, "is_not ")) {
                result = (file_info->mtime < medusa_file_info_get_first_unix_time_occurring_on_date (date)) ||
                        (file_info->mtime >= medusa_file_info_get_last_unix_time_occurring_on_date (date));
        } else if (medusa_str_has_prefix (match_relation, "is_before ")) {
                result = file_info->mtime < medusa_file_info_get_first_unix_time_occurring_on_date (date);
        } else if (medusa_str_has_prefix (match_relation, "is_after ")) {
                result = file_info->mtime > medusa_file_info_get_last_unix_time_occurring_on_date (date);
        } else if (medusa_str_has_prefix (match_relation, "is_within_a_week_of ")) {
                result = (file_info->mtime > medusa_file_info_get_unix_time_a_week_before_date (date)) &&
                        (file_info->mtime < medusa_file_info_get_unix_time_a_week_after_date (date));
        } else if (medusa_str_has_prefix (match_relation, "is_within_a_month_of ")) {
                result = (file_info->mtime > medusa_file_info_get_unix_time_a_month_before_date (date)) &&
                        (file_info->mtime < medusa_file_info_get_unix_time_a_month_after_date (date));
        } else {
		result = FALSE;
        }

        g_free (date);
        return result;
        
}

static gboolean
matches_owner_criterion (const char *criterion,
                       const char *file_uri,
                       GnomeVFSFileInfo *file_info)
{
        const char *match_string;
        const char *match_relation;
        long user_id;
        uid_t uid;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);

        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */
        if (medusa_str_has_prefix (match_relation, "is ")) {
                return (medusa_username_to_uid (match_string, &uid) &&
                        file_info->uid == uid);
        }
        if (medusa_str_has_prefix (match_relation, "is_not ")) {
                return (!medusa_username_to_uid (match_string, &uid) ||
                        file_info->uid != uid);
        }
        if (medusa_str_has_prefix (match_relation, "has_uid ")) {
                user_id = strtol (match_string, NULL, 10);
                return (file_info->uid == (uid_t) user_id);
        }
        if (medusa_str_has_prefix (match_relation, "does_not_have_uid ")) {
                user_id = strtol (match_string, NULL, 10);
                return (file_info->uid != (uid_t) user_id);
        }               
        return FALSE;
}

static gboolean
matches_group_criterion (const char *criterion,
                       const char *file_uri,
                       GnomeVFSFileInfo *file_info)
{
        const char *match_string;
        const char *match_relation;
        long group_id;
        gid_t gid;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);
        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */
        if (medusa_str_has_prefix (match_relation, "is ")) {
                return (medusa_group_to_gid (match_string, &gid) &&
                        file_info->gid == gid);
        }
        if (medusa_str_has_prefix (match_relation, "is_not ")) {
                return (!medusa_group_to_gid (match_string, &gid) ||
                        file_info->gid != gid);
        }
        if (medusa_str_has_prefix (match_relation, "has_uid ")) {
                group_id = strtol (match_string, NULL, 10);
                return (file_info->uid == (gid_t) group_id);
        }
        if (medusa_str_has_prefix (match_relation, "does_not_have_uid ")) {
                group_id = strtol (match_string, NULL, 10);
                return (file_info->uid != (gid_t) group_id);
        }               
        return FALSE;
}

static gboolean
matches_size_criterion (const char *criterion,
                      const char *file_uri,
                      GnomeVFSFileInfo *file_info)
{
        const char *match_string;
        const char *match_relation;
        long size;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);

        size = strtol (match_string, NULL, 10);

        /* If the file has no valid size, then we should return false */
        if (!(file_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)) {
                return FALSE;
        }
        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */
        if (medusa_str_has_prefix (match_relation, "larger_than ")) {
                return (file_info->size > size);
        }
        if (medusa_str_has_prefix (match_relation, "smaller_than ")) {
                return (file_info->size < size);
        }
        if (medusa_str_has_prefix (match_relation, "is ")) {
                return (file_info->size == size);
        }
        g_warning ("Invalid size request\n");
        return FALSE;
}

static gboolean                
matches_one_mime_type (const char *criterion,
                     const char *file_uri,
                     const char *mime_type)
{
        const char *match_string;
        const char *match_relation;
        gboolean is_negative_query;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);
        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */        
        g_return_val_if_fail (medusa_str_has_prefix (match_relation, "is ") ||
                              medusa_str_has_prefix (match_relation, "is_not "),
                              FALSE);
        is_negative_query = medusa_str_has_prefix (match_relation, "is_not  ");

        if (strcmp (match_string, "application") == 0) {
                return medusa_str_has_prefix (mime_type, "application/") ^
                        is_negative_query;
        }
        if (strcmp (match_string, "music") == 0) {
                return medusa_str_has_prefix (mime_type, "audio/") ^
                        is_negative_query;
        }
        if (strcmp (match_string, "text_file") == 0) {
                return medusa_str_has_prefix (mime_type, "text/") ^
                        is_negative_query;
        }
        if (strcmp (match_string, "file") == 0) {
                return !medusa_str_has_prefix (mime_type, "x-directory/") ^
                        is_negative_query;
        }
        if (strcmp (match_string, "directory") == 0) {
                return medusa_str_has_prefix (mime_type, "x-directory/") ^
                        is_negative_query;
        }
        g_warning ("Invalid file type query\n");
        return FALSE;
        
}

static gboolean                
matches_one_keywords_criterion (const char *criterion,
                                const char *file_uri,
                                GHashTable *keywords_hash_table,
                                gboolean keywords_hash_table_exists_for_directory)
{
        const char *match_string;
        const char *match_relation;
        char *file_name;
        gboolean is_negative_query, query_result;
        GnomeVFSURI *vfs_uri;
        
        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);

        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */        
        g_return_val_if_fail (medusa_str_has_prefix (match_relation, "include ") ||
                              medusa_str_has_prefix (match_relation, "do_not_include "),
                              FALSE);
        is_negative_query = medusa_str_has_prefix (match_relation, "do_not_include  ");

        if (!keywords_hash_table_exists_for_directory) {
                return is_negative_query;
        }
        else {
                vfs_uri = gnome_vfs_uri_new (file_uri);
                file_name = gnome_vfs_uri_extract_short_name (vfs_uri);
                query_result =  is_negative_query ^ 
                        medusa_keyword_hash_table_file_has_keyword (keywords_hash_table, 
                                                                    file_name,
                                                                    match_string);
                g_free (file_name);
                gnome_vfs_uri_unref (vfs_uri);
                return query_result;
        }
}
        



static char *  
get_root_directory_from_uri (const char *uri)
{
        char *root_directory;
        
        g_return_val_if_fail (strchr (uri, '[') != NULL, NULL);
        root_directory = strchr (uri, '[') + 1;
        g_return_val_if_fail (strchr (root_directory, ']') != NULL, NULL);
        return g_strndup (root_directory, 
                          strchr (root_directory, ']') - root_directory);
       
}

static char *  
uri_to_search_string (const char *uri)
{
        char *search_string;

        search_string = strchr (uri, '[');
        g_return_val_if_fail (search_string != NULL, NULL);
        search_string = strchr (search_string, ']');
        g_return_val_if_fail (search_string != NULL, NULL);
        search_string++;
        /* For now, we can have only one root directory */
        g_return_val_if_fail (strchr (search_string, '[') == NULL, NULL);
        return g_strdup (search_string);
        
}

static void                    
assume_file_is_directory_unless_a_link (const char *file_uri,
                                        gboolean *file_is_directory)
{
        char *local_path;
        char link_placeholder[1];
        
        local_path = gnome_vfs_get_local_path_from_uri (file_uri);
        *file_is_directory = (readlink (local_path, link_placeholder, 1) == -1);
        g_free (local_path);

}

static GList *
search_string_to_criteria (char *search_string)
{
        char **split_array;
        int i;
        GList *criteria;
        split_array = g_strsplit (search_string, " & ", 0);
        criteria = NULL;
        for (i = 0; split_array[i] != NULL; i++) {
                criteria = g_list_prepend (criteria,
                                           split_array[i]);
        }
        g_free (split_array);
        return criteria;
} 


static gboolean                
criterion_is_name_criterion (gpointer criterion_string,
                             gpointer user_data)
{
        char *criterion;

        criterion = (char *) criterion_string;
        return (criterion_field_is (criterion, MEDUSA_FILE_NAME_CRITERION) ||
                criterion_field_is (criterion, MEDUSA_DIRECTORY_NAME_CRITERION));
}

static gboolean                
criterion_is_inode_criterion (gpointer criterion_string,
                              gpointer user_data)
{
        char *criterion;
        
        criterion = (char *) criterion_string;
        return  (criterion_field_is (criterion_string, MEDUSA_MODIFIED_CRITERION) ||
                 criterion_field_is (criterion_string, MEDUSA_OWNER_CRITERION) ||
                 criterion_field_is (criterion_string, MEDUSA_GROUP_CRITERION) ||
                 criterion_field_is (criterion_string, MEDUSA_SIZE_CRITERION));


}

static gboolean                
criterion_is_mime_criterion (gpointer criterion_string,
                             gpointer user_data)
{
        char *criterion;
        
        criterion = (char *) criterion_string;
        return (criterion_field_is (criterion_string, MEDUSA_FILE_TYPE_CRITERION));
}

static gboolean                
criterion_is_keywords_criterion (gpointer criterion_string,
                                 gpointer user_data)
{
        char *criterion;
        criterion = (char *) criterion_string;

        return (criterion_field_is (criterion_string, MEDUSA_KEYWORDS_CRITERION));

}

static gboolean                
criterion_field_is (const char *criterion,
                    const char *field_name)
{
        return medusa_str_has_prefix (criterion, field_name);
}


static char *                  
uri_to_file_name (const char *uri)
{
        const char *location;

        g_return_val_if_fail (uri != NULL, NULL);
        g_return_val_if_fail (strchr (uri, '/') != NULL, NULL);
        
        location = uri;
        location += strlen (uri);
        while (*location != '/') {
                location--;
        }
        return g_strdup (location);
}

static char *                  
uri_to_path_name (const char *uri)
{
        const char *location;

        g_return_val_if_fail (uri != NULL, NULL);
        g_return_val_if_fail (strchr (uri, '/') != NULL, NULL);

        location = uri;
        location += strlen (uri);
        while (*location != '/') {
                location--;
        }
        return g_strndup (uri, location - uri);

}

static char *                  
match_string_to_date (const char *match_string)
{
        struct tm  *time_struct;
        time_t relevant_time;

        if (strcmp (match_string, "today") == 0) {
                relevant_time = time (NULL);
                time_struct = localtime (&relevant_time);
                return g_strdup_printf ("%d/%d/%d", time_struct->tm_mon + 1,
                                        time_struct->tm_mday, time_struct->tm_year + 1900);
        }
        if (strcmp (match_string, "yesterday") == 0) {
                relevant_time = time (NULL) - 86400;
                time_struct = localtime (&relevant_time);
                return g_strdup_printf ("%d/%d/%d", time_struct->tm_mon + 1,
                                        time_struct->tm_mday, time_struct->tm_year + 1900);
        }
        return g_strdup (match_string);
}

static const char *            
second_word_of (const char *criterion)
{
        const char *location;

        location = criterion;
        while (!isspace (*location)) {
                location++;
        }

        return location + 1;

}

static const char *            
last_word_of (const char *criterion)
{
        const char *location;

        location = criterion + strlen (criterion);
        while (!isspace (*location)) {
                location--;
        }
        return location+1;
}



static int
initialize_results_file_for_writing (char *file_name)
{
        return open (file_name, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | O_SYNC);

} 

static int
initialize_results_file_for_reading (char *file_name)
{
        return open (file_name, O_RDONLY);
}
