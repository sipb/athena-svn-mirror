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

#include <glib.h>
#include <stdio.h>
#include <unistd.h>

#include "medusa-file-search-parse-transmission.h"
#include "medusa-authenticate.h"
#include "medusa-search-uri.h"
#include "medusa-master-db.h"
#include "medusa-search-service-private.h"
#include "medusa-utils.h"

#include <string.h>




static void                 run_query                        (MedusaMasterDB *master_db,
                                                              char *search_uri,
                                                              int uid, 
                                                              int client_fd);
static char *               get_uid_pid_key_information      (char *line,
                                                              int *uid, 
                                                              int *pid, 
                                                              int *key);


/* Filter the results of the query to what the
   user should be able to access, and send them back to the sender */
static void                 present_query_results             (GList *results,
                                                               int client_fd,
                                                               int uid);
static char *               goto_next_line                     (char *line);
/* Free entries from the read cache hash table */
static void                 free_entries                       (gpointer key,
                                                                gpointer value,
                                                                gpointer user_data);
static gboolean             another_transmission_line_exists   (char *line);
static void                 send_cookie_creation_acknowledgement (int client_fd);

void
medusa_file_search_parse_transmission (char *transmission, int client_fd,
				       MedusaMasterDB *db)
{
        char *line;
        int uid, pid, key;

        g_return_if_fail (strlen (transmission) != 0);

        line = transmission;
        while (another_transmission_line_exists (line)) {
#ifdef SEARCH_DAEMON_DEBUG
                printf ("Received %s\n", line);
#endif
                if (medusa_authenticate_is_cookie_request (line)) {
                        medusa_authenticate_create_cookie_file (line); 
                        send_cookie_creation_acknowledgement (client_fd);
                }
                else {
                        line = get_uid_pid_key_information (line, &uid, &pid, &key);
                        if (line == NULL) {
                                g_warning ("Search daemon is trying to parse NULL transmission to its input socket\n");
                                return;
                        }
                        if (medusa_authenticate_is_correct_cookie (uid, pid, key)) {
#ifdef SEARCH_DAEMON_DEBUG
                                printf ("Key %d matched for uid %d and pid %d\n", key, uid, pid);
#endif
                                run_query (db, line, uid, client_fd);
                        }
                }
                line = goto_next_line (line);
        }
}  


static void
run_query (MedusaMasterDB *db, 
           char *search_uri,
           int uid,
           int client_fd) 
{
        GList *results;
        char *search_uri_to_send;
        
        printf ("Running query\n");
        /* Do preprocessing on the search uri, to add extra permission 
           or file type checks, if relevant */
        search_uri_to_send = medusa_search_uri_add_extra_needed_criteria (search_uri,
                                                                          uid);
        results = medusa_master_db_query (db, search_uri_to_send);
        g_free (search_uri_to_send);
        printf ("Done running query\n");
        present_query_results (results, client_fd, uid);

        
}


static char *
get_uid_pid_key_information (char *line, int *uid, int *pid, int *key)
{
        char *result;
        sscanf (line, "%d %d %d\t", uid, pid, key);
        result = strchr (line,'\t');
        if (result == NULL) {
                return result;
        }
        else {
                /* return the next meaningful character, rather than the tab */
                return result+1;
        }
}

static void
present_query_results (GList *results,
		       int client_fd, 
		       int uid)
{
        GList *iterator;
        GHashTable *read_cache;
        char send_buffer[MAX_LINE + 1];

        read_cache = g_hash_table_new (g_str_hash, g_str_equal);
        for (iterator = results ; iterator != NULL; iterator = iterator->next) {
                if (medusa_authenticate_uid_can_read ((char *) iterator->data, uid, read_cache)) {
                        memset (send_buffer, 0, MAX_LINE);
                        strncpy (send_buffer, iterator->data, MAX_LINE);
                        send_buffer [MAX_LINE] = 0;
                        write (client_fd, send_buffer, strlen (send_buffer));
                        write (client_fd, "\n", 1);
                }
        }
        g_hash_table_foreach (read_cache, free_entries, NULL);
        g_hash_table_destroy (read_cache);
        write (client_fd, SEARCH_END_TRANSMISSION, strlen (SEARCH_END_TRANSMISSION));
        write (client_fd, "\n", 1);
        medusa_g_list_free_deep (results);

        printf ("Finished sending search results\n");
        close (client_fd);
}

static void          
free_entries (gpointer key,
              gpointer value,
              gpointer user_data)
{
        char *directory_name;

        directory_name = (char *) key;
        g_free (directory_name);
}

static char *
goto_next_line (char *line)
{
        line = strchr (line, '\n');
        return line+1;
}

static gboolean      
another_transmission_line_exists (char *line)
{
        return (line - 1) != NULL && *line != 0;

}

static void                 
send_cookie_creation_acknowledgement (int client_fd)
{
        g_return_if_fail (client_fd != -1);

        write (client_fd, COOKIE_REQUEST, strlen (COOKIE_REQUEST));
}
