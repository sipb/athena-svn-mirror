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
 *  medusa-text-index-test.c: Test files in the text indexer directory 
 *
 */

#include <medusa-lock.h>
#include <medusa-lock-file-paths.h>
#include <medusa-conf.h>
#include <medusa-master-db.h>
#include <medusa-master-db-private.h>
#include <medusa-uri-list.h>
#include <medusa-text-index.h>
#include <medusa-text-index-queries.h>
#include <stdlib.h>




int main ()
{
        char next_line[1000];
        int *results;
        int number_of_results;
        int i;
        MedusaMasterDB *master_db;
        MedusaReadLock *read_lock;

        /* acquire lock */
	read_lock = medusa_read_lock_get (MEDUSA_FILE_INDEX_READ_LOCK_FILE);
        if (read_lock == NULL) {
                puts ("ERROR: can't acquire database lock");
                exit (-1);
        }
        master_db = medusa_master_db_new (ROOT_DIRECTORY,
                                          MEDUSA_DB_LOG_EVERYTHING,
                                          URI_LIST_NAME,
                                          FILE_SYSTEM_DB_FILE_NAME,
                                          FILE_NAME_HASH_NAME,
                                          DIRECTORY_NAME_HASH_NAME,
                                          MIME_TYPE_HASH_NAME,
                                          KEYWORD_SETS_FILE_NAME,
                                          TEXT_INDEX_START_FILE_NAME,
                                          TEXT_INDEX_LOCATION_FILE_NAME,
                                          TEXT_INDEX_WORD_FILE_NAME,
                                          TEXT_INDEX_TEMP_FILE_NAME);
        
        printf ("Enter words to look for in the text index\n");
        scanf ("%s", next_line);
        printf ("Looking for all the words X%sX\n", next_line);
        results = medusa_text_index_get_uris_for_criterion (master_db->text_index,
                                                            next_line,
                                                            TRUE,
                                                            &number_of_results);

        if (results == NULL) {
                printf ("There are no results for word %s\n", next_line);
                return 1;
        }
        for (i = 0; i < number_of_results; i++) {
                printf ("Next result is uri number  %d, which is %s\n", results[i], 
                        medusa_uri_number_to_uri (master_db->uri_list, results[i]));
        }
        medusa_read_lock_release (read_lock);
        printf ("Total number of results is %d\n", number_of_results);
        return 0;
        
}
