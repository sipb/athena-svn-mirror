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


#include <config.h>
#include "medusa-search-uri.h"

#include "medusa-master-db-private.h"
#include "medusa-uri-list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <libmedusa/medusa-utils.h>

#include "medusa-master-db.h"
#include "medusa-master-db-private.h"
#include "medusa-query-optimizations.h"
#include "medusa-uri-list.h"
#include "medusa-search-uri.h"


static gboolean              bypass_search_method                         (char **string);
static char *                get_next_root_uri                            (char **string);
static char **               get_search_criteria                          (const char *string);
static MedusaClauseClosure * clause_to_clause_closure                     (const char *clause,                       
                                                                           MedusaMasterDB *master_db);
static gboolean              clause_is_content_request                    (const char *clause);
static char *                get_verb_from_clause                         (const char *clause);
static char *                get_direct_object_from_clause                (const char *clause);
static gboolean              request_wants_all_words_to_match             (const char *verb);
static gboolean              request_is_for_positive_matches              (const char *verb);

static gboolean              search_uri_contains_content_requests         (const char *search_uri);
static gboolean              search_uri_contains_size_requests            (const char *search_uri);

static char *                search_uri_add_user_can_read                 (const char *search_uri, 
                                                                           uid_t uid);
static char *                search_uri_add_type_is_regular_file          (const char *search_uri);
static void                  medusa_clause_closure_free_cover             (gpointer data,
                                                                           gpointer user_data);
static void                  medusa_clause_closure_free                   (MedusaClauseClosure *closure);

static MedusaQueryOptimizationList *optimizations = NULL;

static void
print_optimization_result (MedusaOptimizationResult *optimization_result)
{
        int i;

        if (optimization_result->syntax_error_found) {
                g_print ("Syntax error found during optimization; aborting query.\n");
                return;
        }
        if (optimization_result->query_is_always_true) {
                g_print ("Query is TRUE in all cases\n");
                return;
        }
        if (optimization_result->query_is_always_false) {
                g_print ("Query is FALSEn");
                return;
        }
        g_print ("The optimization produced a new query\n");
        g_print ("search:[file:///]");
        for (i = 0; optimization_result->criteria[i+1] != NULL; i++) {
                g_print ("%s & ", optimization_result->criteria[i]);
        }
        g_print ("%s\n", optimization_result->criteria[i]);
}

MedusaParsedSearchURI *
medusa_search_uri_parse (char *search_uri,
                         MedusaMasterDB *master_db)
{
        char *location;
        char *root_uri;
        gboolean search_method_is_valid;
        char **criteria;
        MedusaOptimizationResult *optimization_result;
        int i;
        MedusaParsedSearchURI *parsed_structure;
        MedusaClauseClosure *clause_closure;



        g_return_val_if_fail (medusa_uri_is_search_uri (search_uri), NULL);

        
        location = strchr (search_uri, ':');
        location++;
        
        search_method_is_valid = bypass_search_method (&location);
        g_return_val_if_fail (search_method_is_valid, NULL);
        
        root_uri = get_next_root_uri (&location);

        /* We shouldn't be getting any other kinds of uri's except
         * for a single root with root = "file:///".
         */
        if (root_uri == NULL ||
            strcmp (root_uri, "file:///") ||
            get_next_root_uri (&location) != NULL) {
                return NULL;
        }
        
        g_free (root_uri);
        
        location = strchr (search_uri, ']');
        location++;
        
        /* For now we don't except or queries here */
        if (strchr (location, '|') != NULL) {
                return NULL;
        }

        parsed_structure = g_new0 (MedusaParsedSearchURI, 1);        

        g_strdown (location);
        criteria = get_search_criteria (location);
        if  (criteria == NULL) {
                parsed_structure->syntax_error_found = TRUE;
                return parsed_structure;
        }
        if (optimizations == NULL) {
                optimizations = medusa_query_optimizations_initialize ();
        }
        optimization_result = medusa_query_optimizations_perform_and_free_deep (optimizations,
                                                                                criteria);

        print_optimization_result (optimization_result);
        parsed_structure->query_is_always_true = optimization_result->query_is_always_true;
        parsed_structure->query_is_always_false = optimization_result->query_is_always_false;
        parsed_structure->syntax_error_found = optimization_result->syntax_error_found;

        if (parsed_structure->query_is_always_true ||
            parsed_structure->query_is_always_false ||
            parsed_structure->syntax_error_found) {
                return parsed_structure;
        }
        
        for (i=0; optimization_result->criteria[i] != NULL; i++) {
                clause_closure =  clause_to_clause_closure (optimization_result->criteria[i],
                                                            master_db);
                /* Detect syntax errors */
                if (clause_closure == NULL) {
                        parsed_structure->syntax_error_found = TRUE;
                        medusa_optimization_result_destroy (optimization_result);
                        return parsed_structure;
                }
                parsed_structure->clause_closures = g_list_prepend (parsed_structure->clause_closures, clause_closure);
                                                               
        }
        medusa_optimization_result_destroy (optimization_result);
        return parsed_structure;
}

void
medusa_parsed_search_uri_free (MedusaParsedSearchURI *parsed_search_uri)
{
        g_free (parsed_search_uri);
        
}

void                     
medusa_search_uri_parse_shutdown (void)
{
        if (optimizations != NULL) {
                medusa_query_optimizations_destroy (optimizations);
        }
        optimizations = NULL;
}

static MedusaClauseClosure *
clause_to_clause_closure (const char *clause,
                          MedusaMasterDB *master_db)
{
        MedusaQueryFunc evaluate;
        MedusaClauseClosure *closure;
        MedusaArgumentType type;
        char *verb, *direct_object;

        verb = get_verb_from_clause (clause);
        if (verb == NULL) {
                return NULL;
        }
        direct_object = get_direct_object_from_clause (clause);
        if (direct_object == NULL) {
                return NULL;
        }

        evaluate = medusa_query_clauses_get_function (medusa_uri_list_get_query_clauses (master_db->uri_list),
                                                      clause,
                                                      &type);

        if (evaluate != NULL) {
                closure = g_new0 (MedusaClauseClosure, 1);
                closure->argument_type = type;
                closure->query_func = evaluate;
                
                closure->uri_list = master_db->uri_list;
                closure->file_system_db = NULL;
                switch (type) {
                case MEDUSA_ARGUMENT_TYPE_STRING:
                        closure->argument = g_strdup (direct_object);
                        break;
                case MEDUSA_ARGUMENT_TYPE_INTEGER:
                        closure->argument = GINT_TO_POINTER(strtol (direct_object, NULL, 10));
                        break;
                default:
                        g_assert_not_reached ();
                        return NULL;
                }
                closure->is_content_request = FALSE;
                g_free (verb);
                g_free (direct_object);
                return closure;
        }
        
        evaluate = medusa_query_clauses_get_function (medusa_file_system_db_get_query_clauses (master_db->file_system_db),
                                                      clause,
                                                      &type);
        if (evaluate != NULL) {
                closure = g_new0 (MedusaClauseClosure, 1);
                closure->argument_type = type;
                closure->query_func = evaluate;
                closure->file_system_db = master_db->file_system_db;
                closure->uri_list = NULL;
                switch (type) {
                case MEDUSA_ARGUMENT_TYPE_STRING:
                        closure->argument = g_strdup (direct_object);
                        break;
                case MEDUSA_ARGUMENT_TYPE_INTEGER:
                        closure->argument = GINT_TO_POINTER(strtol (direct_object, NULL, 10));
                        break;
                default:
                        /* MEDUSA_ARGUMENT_TYPE_NONE */
                        g_assert_not_reached ();
                }
                closure->is_content_request = FALSE;
                g_free (verb);
                g_free (direct_object);
                return closure;
        }
        if (clause_is_content_request (clause)) {
                closure = g_new0 (MedusaClauseClosure, 1);
                closure->argument_type = MEDUSA_ARGUMENT_TYPE_NONE;
                closure->query_func = evaluate;
                closure->is_content_request = TRUE;
                closure->match_all_words = request_wants_all_words_to_match (verb);
                closure->content_request = g_strdup (direct_object);
                closure->return_matches = request_is_for_positive_matches (verb);
                g_free (verb);
                g_free (direct_object);
                return closure;
        }

        g_free (verb);
        g_free (direct_object);
        return NULL;
}

char *
medusa_search_uri_add_extra_needed_criteria (const char *search_uri,
                                             uid_t uid)
{
        char *current_uri;
        char *new_uri;

        current_uri = g_strdup (search_uri);
        if (search_uri_contains_content_requests (current_uri)) {
                new_uri = search_uri_add_user_can_read (current_uri,
                                                        uid);
                g_free (current_uri);
                current_uri = new_uri;
        }
        if (search_uri_contains_size_requests (current_uri)) {
                new_uri = search_uri_add_type_is_regular_file (current_uri);
                g_free (current_uri);
                current_uri = new_uri;
        }
        
        return current_uri;
                                                               
}

static gboolean              
search_uri_contains_content_requests (const char *search_uri)
{
        char *text_request, *text_anti_request, *file_type_is;
        text_request = strstr (search_uri,"content includes");
        text_anti_request = strstr (search_uri, "content does_not_include");
        file_type_is = strstr (search_uri, "file_type is");
        return (text_request != NULL ||
                text_anti_request != NULL ||
                file_type_is != NULL);
}

static char *                
search_uri_add_user_can_read (const char *search_uri, 
                              uid_t uid)
{
        char *new_criterion;
        char *extended_search_uri;

        new_criterion = g_strdup_printf ("permissions_to_read include_uid %ld", (long int) uid);
        if (strstr (search_uri, new_criterion)) {
                g_free (new_criterion);
                return g_strdup (search_uri);
        }
        else {
                extended_search_uri = g_strdup_printf ("%s & %s", search_uri, new_criterion);
                g_free (new_criterion);
                return extended_search_uri;
        }
}


static gboolean              
search_uri_contains_size_requests (const char *search_uri)
{
        char *size_larger, *size_smaller, *size_is;
        size_larger = strstr (search_uri,"size larger_than ");
        size_smaller = strstr (search_uri, "size smaller_than ");
        size_is = strstr (search_uri, "size is");
        return (size_larger != NULL ||
                size_smaller != NULL ||
                size_is != NULL);
}

static char *                
search_uri_add_type_is_regular_file (const char *search_uri)

{
        char *extended_search_uri;

        if (strstr (search_uri, "file_type is file")) {
                return g_strdup (search_uri);
        }
        else {
                extended_search_uri = g_strdup_printf ("%s & file_type is file", search_uri);
                return extended_search_uri;
        }
}

static void                  
medusa_clause_closure_free_cover (gpointer data,
                                  gpointer user_data)
{
        g_return_if_fail (data != NULL);

        medusa_clause_closure_free (data);
}

static void                  
medusa_clause_closure_free (MedusaClauseClosure *closure)
{
        if (closure->argument_type == MEDUSA_ARGUMENT_TYPE_STRING) {
                g_free (closure->argument);
        }

        if (closure->is_content_request) {
                g_free (closure->content_request);
        }
        
        g_free (closure);
}

void
medusa_clause_closure_list_free (GList *clause_closures)
{
        g_list_foreach (clause_closures, medusa_clause_closure_free_cover, NULL);
        g_list_free (clause_closures);
}

gboolean              
medusa_clause_closure_is_content_search (gpointer data,
                                         gpointer user_data)
{
        MedusaClauseClosure *clause_closure;

        clause_closure = (MedusaClauseClosure *) data;
        g_return_val_if_fail (clause_closure != NULL, FALSE);

        g_assert (clause_closure->is_content_request == TRUE ||
                  clause_closure->is_content_request == FALSE);

        return clause_closure->is_content_request;
}

gboolean
medusa_uri_is_search_uri (char *uri)
{
        char *trimmed_uri;
        gboolean result;

        /* Remove leading spaces */
        trimmed_uri = g_strdup (uri);
        trimmed_uri = g_strchug (trimmed_uri);
        result = FALSE;

        if (strncmp (trimmed_uri, "gnome-search:", strlen ("gnome-search:")) ==  0) {
                result = TRUE;
        }
        if (strncmp (trimmed_uri, "search:", strlen ("search:")) == 0) {
                result = TRUE;
        }
        if (strncmp (trimmed_uri, "medusa:", strlen ("medusa:")) == 0) {
                result = TRUE;
        }
        g_free (trimmed_uri);
        return result;
}

static gboolean
bypass_search_method (char **string)
{
        if (**string == '[') {
                /* There's no method, we're fine */
                return TRUE;
        }
        if (medusa_str_has_prefix (*string,
                                   "index-only")) {
                *string += strlen ("index-only");
                return TRUE;
        }
        if (medusa_str_has_prefix (*string,
                                   "index-with-backup")) {
                *string += strlen ("index-with-backup");
                return TRUE;

        }
        return FALSE;
}

/* Gets the next bracket enclosed segment.
   does no checking for uri validity */
static char *
get_next_root_uri (char **string)
{
        char *close_bracket;
        char *uri;
#ifdef SEARCH_URI_DEBUG
        printf ("Trying to parse string %s\n", *string);
#endif
        g_return_val_if_fail (*string != NULL, NULL);

        /* Return if there are no more uri's */
        if (*string[0] != '[') {
                return NULL;
        }

        close_bracket = strchr (*string, ']');

        if (close_bracket == NULL) {
                return NULL;
        }
        
        uri = g_strndup (*string + 1, close_bracket - (*string + 1));
#ifdef SEARCH_URI_DEBUG
        printf ("Next root uri is %s\n", uri);
#endif
        *string = close_bracket + 1;
        return uri;
}

static char **
get_search_criteria (const char *string)
{
        g_return_val_if_fail (string != NULL, NULL);
        return g_strsplit (string, " & ", G_MAXINT);
}


static gboolean              
clause_is_content_request (const char *clause)
{
        return medusa_str_has_prefix (clause, "content");
}

static char *                
get_verb_from_clause (const char *clause)
{
        const char *end_of_word;

        if (strchr (clause, ' ') == NULL) {
                return NULL;
        }
        for ( ; *clause != ' '; clause++);
        for ( ; *clause == ' '; clause++);
        for (end_of_word = clause; *end_of_word != ' '; end_of_word++);

        return g_strndup (clause, end_of_word - clause);

}

static char *                
get_direct_object_from_clause (const char *clause)
{
        g_assert (strchr (clause, ' ') != NULL);
        for ( ; *clause != ' '; clause++);
        for ( ; *clause == ' '; clause++);
        if (strchr (clause, ' ') == NULL) {
                return NULL;
        }
        for ( ; *clause != ' '; clause++);
        for ( ; *clause == ' '; clause++);

        return g_strdup (clause);

}

static gboolean              
request_wants_all_words_to_match (const char *verb)
{
        if (strcmp (verb, "includes_all_of") == 0 ||
            strcmp (verb, "does_not_include_all_of") == 0) {
                return TRUE;
        }
        else {
                g_return_val_if_fail (strcmp (verb, "includes_any_of") == 0 ||
                                      strcmp (verb, "does_not_include_any_of") == 0,
                                      FALSE);
                return FALSE;
        }
}

static gboolean              
request_is_for_positive_matches (const char *verb)
{
        if (strcmp (verb, "includes_all_of") == 0 ||
            strcmp (verb, "includes_any_of") == 0) {
                return TRUE;
        }
        else {
                g_return_val_if_fail (strcmp (verb, "does_not_include_all_of") == 0 ||
                                      strcmp (verb, "does_not_include_any_of") == 0,
                                      FALSE);
                return FALSE;
        }
}

