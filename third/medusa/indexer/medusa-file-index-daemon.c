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

#include <errno.h>
#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-popt.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <popt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include <libmedusa-internal/medusa-conf.h>
#include <libmedusa-internal/medusa-lock.h>
#include <libmedusa-internal/medusa-lock-file-paths.h>
#include <libmedusa-internal/medusa-master-db.h>
#include <libmedusa/medusa-search-service-private.h>
#include <libmedusa/medusa-index-progress.h>
#include <libmedusa/medusa-index-service.h>
#include <libmedusa/medusa-index-service-private.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#ifndef NAME_MAX
#define NAME_MAX 512
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

/* Creates a totally new file index, and writes it over the old one */
static void             do_full_indexing                  (const char *root_directory);

static void             do_index_accounting               (void);

/* Sits on the indexing socket and waits for requests to do either of the
   above indexings */
void                    wait_for_indexing_signal          (void); 

static void             file_indexer_parse_transmission   (char *transmission,
							   int client_fd);

static void             register_command_line_options     (struct poptOption *medusa_options);
/* Do clean up, etc. */
static void             exit_indexer                      (void);

/* Sets up index request socket */
int                     initialize_index_server_socket   (struct sockaddr_un *daemon_address); 
/* Copies the index files over to a different location 
   for quick reindexing so that the 
   current database is usable until the updated index is ready */
void                    backup_index_files                (void);
/* Moves the ready updated/new index files
   to the place where are actually read */
void                    copy_new_index_files              (void);




extern int errno;

/* Main index loop.  
   Creates three process to wait for signals, 
   do fast reindexing, and do slow reindexing */
int
main (int argc, char *argv[])
{
        pid_t forked_process;
        poptContext popt_context;
        const char **args;
        gboolean start_listener, text_index_accounting;
        const char *root_directory;
        struct poptOption command_line_options[] = {
                { "folder", 'f', POPT_ARG_STRING, &root_directory, 
                  0, N_("Specify the root folder to start indexing at"),
                  N_("PATH") },
                { "with-listener", '\0', POPT_ARG_NONE, &start_listener,
                  0, N_("Start listener for interprocess indexing requests"),
                  NULL },
                { "do-text-accounting", '\0', POPT_ARG_NONE, &text_index_accounting,
                  0, N_("Count the space taken in the text index by file name and mime type"),
                  NULL },
                POPT_AUTOHELP
                { NULL, '\0', 0, NULL, 0, NULL, NULL }
};



        if (getuid () != 0) {
                puts ("The medusa index daemon must be run as root.  Exiting now\n");
                exit (1);
        }
        /* Set these to defaults */
        root_directory = "/";
        start_listener = FALSE;
        text_index_accounting = FALSE;

        nice (19);
        gnome_vfs_init ();


        
        register_command_line_options (command_line_options);
        popt_context = gnomelib_parse_args (argc, argv, 0);

        /* Get the remaining arguments */
        args = poptGetArgs (popt_context);
        if (args != NULL) {
                fprintf (stderr, "Invalid argument: args[0]\n");
                fprintf (stderr, "Exiting\n");
                exit_indexer ();
        }
        poptFreeContext (popt_context);

        if (start_listener) {
                forked_process = fork ();
                if (forked_process == 0) {
                        wait_for_indexing_signal ();
                }
        }
        if (text_index_accounting) {
                do_index_accounting ();
                exit_indexer ();
        }
        
        do_full_indexing (root_directory); 
        exit_indexer ();

        return 1;
}



void
wait_for_indexing_signal (void)
{
        struct sockaddr_un *daemon_address;
        guint address_length;
        int index_request_socket, client_fd;
        char transmission[MAX_LINE];
        int transmission_length;
  
        /* FIXME  bugzilla.eazel.com 2621:
           There should be some access control system here, so
           that medusa can be set up to only allow reindex requests
           from certain users */

        daemon_address = g_new0 (struct sockaddr_un, 1);
        while (1) {
                index_request_socket = initialize_index_server_socket (daemon_address);
                address_length = SUN_LEN (daemon_address);
    
                client_fd = accept (index_request_socket, 
                                    (struct sockaddr *) daemon_address,
                                    &address_length);
                /* FIXME: bugzilla.eazel.com 2645:
                   should do error handling */
                printf ("errno is %d\n", errno);
                g_return_if_fail (client_fd != -1);
    
                memset (transmission, 0, MAX_LINE - 1);
                transmission_length = read (client_fd, transmission, MAX_LINE - 1);
    
                file_indexer_parse_transmission (transmission, client_fd);
                memset (transmission, 0, MAX_LINE);
                printf ("writing to socket %d\n", client_fd);
                transmission_length = read (client_fd, transmission, MAX_LINE - 1);
                close (client_fd);
        }
}

void
do_index_accounting (void)
{
        MedusaMasterDB *master_db;
        char *root_uri;
  
        unlink (URI_ACCOUNT_INDEX);
        unlink (DIRECTORY_NAME_ACCOUNT_INDEX);
        unlink (FILE_NAME_ACCOUNT_INDEX);
        unlink (MIME_TYPE_ACCOUNT_INDEX);
        unlink (KEYWORD_SETS_ACCOUNT_INDEX);
        unlink (FILE_SYSTEM_DB_ACCOUNT_INDEX);
        unlink (TEXT_INDEX_START_ACCOUNT_INDEX);
        unlink (TEXT_INDEX_LOCATION_ACCOUNT_INDEX);
        unlink (TEXT_INDEX_WORD_ACCOUNT_INDEX);
        unlink (TEXT_INDEX_TEMP_ACCOUNT_INDEX);
        printf ("Removed old account files\n");
  
        root_uri = gnome_vfs_get_uri_from_local_path (ROOT_DIRECTORY);
        master_db = medusa_master_db_new (root_uri,
                                          MEDUSA_DB_LOG_TEXT_INDEX_DATA,
                                          URI_ACCOUNT_INDEX,
                                          FILE_SYSTEM_DB_ACCOUNT_INDEX,
                                          FILE_NAME_ACCOUNT_INDEX,
                                          DIRECTORY_NAME_ACCOUNT_INDEX,
                                          MIME_TYPE_ACCOUNT_INDEX,
                                          KEYWORD_SETS_ACCOUNT_INDEX,
                                          TEXT_INDEX_START_ACCOUNT_INDEX,
                                          TEXT_INDEX_LOCATION_ACCOUNT_INDEX,
                                          TEXT_INDEX_WORD_ACCOUNT_INDEX,
                                          TEXT_INDEX_TEMP_ACCOUNT_INDEX);
        g_free (root_uri);
        printf ("Initialized database\n");

#ifdef NOT_DEFINED
        medusa_master_db_set_accounting_on (master_db, INDEX_ACCOUNT_FILE);
#endif
        medusa_master_db_index (master_db);
        printf ("Finished indexing \n");
  
        medusa_master_db_unref (master_db);
        unlink (URI_ACCOUNT_INDEX);
        unlink (DIRECTORY_NAME_ACCOUNT_INDEX);
        unlink (FILE_NAME_ACCOUNT_INDEX);
        unlink (MIME_TYPE_ACCOUNT_INDEX);
        unlink (KEYWORD_SETS_ACCOUNT_INDEX);
        unlink (FILE_SYSTEM_DB_ACCOUNT_INDEX);
        unlink (TEXT_INDEX_START_ACCOUNT_INDEX);
        unlink (TEXT_INDEX_LOCATION_ACCOUNT_INDEX);
        unlink (TEXT_INDEX_WORD_ACCOUNT_INDEX);
        unlink (TEXT_INDEX_TEMP_ACCOUNT_INDEX);
        printf ("Removed account files\n");
}  

		     


static void
do_full_indexing (const char *root_directory)
{
        MedusaMasterDB *master_db;
        char *root_uri;
        int i;
        char *temp_index_name;
        MedusaWriteLock *write_lock;
  
        write_lock = medusa_write_lock_get (MEDUSA_FILE_INDEX_WRITE_LOCK_FILE);
        if (write_lock == NULL) {
                /* FIXME: bugzilla.eazel.com 2622 
                   Locking API should communicate the problem so we 
                   can tell the user about it */
                puts ("ERROR: failed to acquire lock file.");
                        exit (-1);
        }
        medusa_index_progress_file_clear ();
        unlink (FILE_SYSTEM_DB_FILE_NAME BACKUP_SUFFIX);
        unlink (DIRECTORY_NAME_HASH_NAME BACKUP_SUFFIX);
        unlink (FILE_NAME_HASH_NAME BACKUP_SUFFIX);
        unlink (MIME_TYPE_HASH_NAME BACKUP_SUFFIX);
        unlink (KEYWORD_SETS_FILE_NAME BACKUP_SUFFIX);
        unlink (URI_LIST_NAME BACKUP_SUFFIX);
        unlink (TEXT_INDEX_START_FILE_NAME BACKUP_SUFFIX);
        unlink (TEXT_INDEX_LOCATION_FILE_NAME BACKUP_SUFFIX);
        unlink (TEXT_INDEX_WORD_FILE_NAME BACKUP_SUFFIX);
        
        for (i = 0; i < NUMBER_OF_TEMP_INDEXES; i++) {
                temp_index_name = g_strdup_printf ("%s%s.%d", TEXT_INDEX_TEMP_FILE_NAME, BACKUP_SUFFIX, i); 
                unlink (temp_index_name);
                g_free (temp_index_name);
        }
        printf ("Removed old backup files\n");
    
        root_uri = gnome_vfs_get_uri_from_local_path (root_directory);
        if (getenv ("MEDUSA_INDEX_DEBUG") != NULL) {
                master_db = medusa_master_db_new (root_uri,
                                                  MEDUSA_DB_LOG_EVERYTHING,
                                                  URI_LIST_NAME BACKUP_SUFFIX,
                                                  FILE_SYSTEM_DB_FILE_NAME BACKUP_SUFFIX,
                                                  FILE_NAME_HASH_NAME BACKUP_SUFFIX,
                                                  DIRECTORY_NAME_HASH_NAME BACKUP_SUFFIX,
                                                  MIME_TYPE_HASH_NAME BACKUP_SUFFIX,
                                                  KEYWORD_SETS_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_START_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_LOCATION_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_WORD_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_TEMP_FILE_NAME BACKUP_SUFFIX);
        }
        else if (getenv ("MEDUSA_INDEX_LOG_ABBREVIATED") != NULL) {
                master_db = medusa_master_db_new (root_uri,
                                                  MEDUSA_DB_LOG_ABBREVIATED,
                                                  URI_LIST_NAME BACKUP_SUFFIX,
                                                  FILE_SYSTEM_DB_FILE_NAME BACKUP_SUFFIX,
                                                  FILE_NAME_HASH_NAME BACKUP_SUFFIX,
                                                  DIRECTORY_NAME_HASH_NAME BACKUP_SUFFIX,
                                                  MIME_TYPE_HASH_NAME BACKUP_SUFFIX,
                                                  KEYWORD_SETS_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_START_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_LOCATION_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_WORD_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_TEMP_FILE_NAME BACKUP_SUFFIX);
        }
        else if (getenv ("MEDUSA_INDEX_LOG_ERRORS") != NULL) {
                master_db = medusa_master_db_new (root_uri,
                                                  MEDUSA_DB_LOG_ABBREVIATED,
                                                  URI_LIST_NAME BACKUP_SUFFIX,
                                                  FILE_SYSTEM_DB_FILE_NAME BACKUP_SUFFIX,
                                                  FILE_NAME_HASH_NAME BACKUP_SUFFIX,
                                                  DIRECTORY_NAME_HASH_NAME BACKUP_SUFFIX,
                                                  MIME_TYPE_HASH_NAME BACKUP_SUFFIX,
                                                  KEYWORD_SETS_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_START_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_LOCATION_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_WORD_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_TEMP_FILE_NAME BACKUP_SUFFIX);
        }
        else {
                master_db = medusa_master_db_new (root_uri,
                                                  MEDUSA_DB_LOG_NOTHING,
                                                  URI_LIST_NAME BACKUP_SUFFIX,
                                                  FILE_SYSTEM_DB_FILE_NAME BACKUP_SUFFIX,
                                                  FILE_NAME_HASH_NAME BACKUP_SUFFIX,
                                                  DIRECTORY_NAME_HASH_NAME BACKUP_SUFFIX,
                                                  MIME_TYPE_HASH_NAME BACKUP_SUFFIX,
                                                  KEYWORD_SETS_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_START_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_LOCATION_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_WORD_FILE_NAME BACKUP_SUFFIX,
                                                  TEXT_INDEX_TEMP_FILE_NAME BACKUP_SUFFIX);
        }
        
        g_free (root_uri);
        printf ("Initialized database\n");

        /* It is faster to erase the old file and reindex, I think
           at this point */
        printf ("Indexing...\n");
        medusa_master_db_index (master_db);
        printf ("Finished indexing \n");
        
        medusa_master_db_unref (master_db);
        printf ("Wrote databases to disk\n");
    
        copy_new_index_files ();
        printf ("Done moving files to correct locations\n");
	
        printf ("Releasing lock and exiting\n");
        medusa_write_lock_release (write_lock);
        
        exit_indexer ();
}

int
initialize_index_server_socket (struct sockaddr_un *daemon_address)
{
        int index_listening_fd; 
        int bind_return_code;
        int listen_return_code;

        index_listening_fd = socket (AF_LOCAL, SOCK_STREAM, 0);
        g_return_val_if_fail (index_listening_fd != -1, -1);
  
        daemon_address->sun_family = AF_LOCAL; 
        /* FIXME bugzilla.eazel.com 2635:
           Hard coding the size of the struct seems ugly,
           but it is not clear what the more positive alternative is */
        snprintf (daemon_address->sun_path, 100, "%s", INDEX_SOCKET_PATH);
        /* If socket currently exists, delete it 
           unlink (daemon_address->sun_path); */
#ifdef INDEX_DEBUG
        g_message  ("Creating socket at %s\n", daemon_address->sun_path);
#endif
        unlink (daemon_address->sun_path);

        bind_return_code = bind (index_listening_fd, (struct sockaddr *) daemon_address, 
                                 SUN_LEN (daemon_address));
        g_return_val_if_fail (bind_return_code != -1, -1);
  
        listen_return_code = listen (index_listening_fd, 5);
        g_return_val_if_fail (listen_return_code != -1, -1);
        chmod (INDEX_SOCKET_PATH, S_IRWXU | S_IRWXG | S_IRWXO);

        return index_listening_fd;
} 


void
backup_index_files (void)
{
        /* FIXME bugzilla.eazel.com 2637: should not use cp!!!! */
        system ("/bin/cp -f "  URI_LIST_NAME " " 
                URI_LIST_NAME BACKUP_SUFFIX);
        system ("/bin/cp -f " DIRECTORY_NAME_HASH_NAME " " 
                DIRECTORY_NAME_HASH_NAME BACKUP_SUFFIX);
        system ("/bin/cp -f "  FILE_NAME_HASH_NAME " " 
                FILE_NAME_HASH_NAME BACKUP_SUFFIX);
  
        system ("/bin/cp -f " FILE_SYSTEM_DB_FILE_NAME " " 
                FILE_SYSTEM_DB_FILE_NAME BACKUP_SUFFIX);
        system ("/bin/cp -f " MIME_TYPE_HASH_NAME " " 
                MIME_TYPE_HASH_NAME BACKUP_SUFFIX);
        system ("/bin/cp -f " KEYWORD_SETS_FILE_NAME " " 
                KEYWORD_SETS_FILE_NAME BACKUP_SUFFIX);
  
        system ("/bin/cp -f "  TEXT_INDEX_START_FILE_NAME " " 
                TEXT_INDEX_START_FILE_NAME BACKUP_SUFFIX);
        system ("/bin/cp -f "  TEXT_INDEX_LOCATION_FILE_NAME " " 
                TEXT_INDEX_LOCATION_FILE_NAME BACKUP_SUFFIX);
        system ("/bin/cp -f "  TEXT_INDEX_WORD_FILE_NAME " " 
                TEXT_INDEX_WORD_FILE_NAME BACKUP_SUFFIX);
        system ("/bin/cp -f "  TEXT_INDEX_TEMP_FILE_NAME " " 
                TEXT_INDEX_TEMP_FILE_NAME BACKUP_SUFFIX);
}

void
copy_new_index_files (void)
{
        char *temp_index_name;
        int i;

        unlink (URI_LIST_NAME);
        rename (URI_LIST_NAME BACKUP_SUFFIX, 
                URI_LIST_NAME);
  
        unlink (DIRECTORY_NAME_HASH_NAME);
        rename (DIRECTORY_NAME_HASH_NAME BACKUP_SUFFIX, 
                DIRECTORY_NAME_HASH_NAME);
  
        unlink (FILE_NAME_HASH_NAME);
        rename (FILE_NAME_HASH_NAME BACKUP_SUFFIX, 
                FILE_NAME_HASH_NAME);
  
        unlink (FILE_SYSTEM_DB_FILE_NAME);
        rename (FILE_SYSTEM_DB_FILE_NAME BACKUP_SUFFIX, 
                FILE_SYSTEM_DB_FILE_NAME);

        unlink (MIME_TYPE_HASH_NAME);
        rename (MIME_TYPE_HASH_NAME BACKUP_SUFFIX, 
                MIME_TYPE_HASH_NAME);
  
        unlink (KEYWORD_SETS_FILE_NAME);
        rename (KEYWORD_SETS_FILE_NAME BACKUP_SUFFIX, 
                KEYWORD_SETS_FILE_NAME);
  
        unlink (TEXT_INDEX_START_FILE_NAME);
        rename (TEXT_INDEX_START_FILE_NAME BACKUP_SUFFIX,
                TEXT_INDEX_START_FILE_NAME);

        unlink (TEXT_INDEX_LOCATION_FILE_NAME);
        rename (TEXT_INDEX_LOCATION_FILE_NAME BACKUP_SUFFIX,
                TEXT_INDEX_LOCATION_FILE_NAME);

        unlink (TEXT_INDEX_WORD_FILE_NAME);
        rename (TEXT_INDEX_WORD_FILE_NAME BACKUP_SUFFIX,
                TEXT_INDEX_WORD_FILE_NAME);
  
        /* We don't need to keep the temp files around */
        for (i = 0; i < NUMBER_OF_TEMP_INDEXES; i++) {
                temp_index_name = g_strdup_printf ("%s%s.%d", TEXT_INDEX_TEMP_FILE_NAME, BACKUP_SUFFIX, i); 
                unlink (temp_index_name);
                g_free (temp_index_name);
        }

}

static void
file_indexer_parse_transmission (char *transmission,
				 int client_fd)
{
        MedusaWriteLock *write_lock;
  
        if (!strlen (transmission)) {
                return;
        }
  
        if (strstr (transmission, MEDUSA_REINDEX_REQUEST)) {
                /* Grab DB lock */
                write_lock = medusa_write_lock_get (MEDUSA_FILE_INDEX_WRITE_LOCK_FILE);
                if (write_lock == NULL) {
                        /* We can't index right now -- someone else has the lock */
                        write (client_fd, MEDUSA_REINDEX_REQUEST_DENIED_BUSY,
                               strlen (MEDUSA_REINDEX_REQUEST_DENIED_BUSY));
                } else {
                        write (client_fd, MEDUSA_REINDEX_REQUEST_ACCEPTED, 
                               strlen (MEDUSA_REINDEX_REQUEST_ACCEPTED));
		  
                        /* Remove db lock */
                        medusa_write_lock_release (write_lock);
		  
                        /* FIXME bugzilla.eazel.com 2639:  
                           Need another fork here to collect messages while we are 
                           indexing */
                        do_full_indexing (ROOT_DIRECTORY);
                }
        }
}

static void             
register_command_line_options (struct poptOption *medusa_options)
{
        gnomelib_register_popt_table (medusa_options, "Medusa Indexer Options");
}

static void             
exit_indexer (void)
{
        gnome_vfs_shutdown ();
        exit (0);
}



