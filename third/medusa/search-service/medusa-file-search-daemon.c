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
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <glib.h>
#include <libgnomevfs/gnome-vfs-init.h>

#include <medusa-lock.h>
#include <medusa-lock-file-paths.h>
#include <medusa-conf.h>
#include <medusa-search-service-private.h>
#include <medusa-master-db.h>


#include "medusa-authenticate.h"
#include "medusa-file-search-parse-transmission.h"

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

static MedusaMasterDB *master_db; 
time_t index_validity_time;

static void             ensure_index_validity                       (MedusaMasterDB **master_db); 
static int              initialize_socket                           (struct sockaddr_un *daemon_address);
#ifdef MEDUSA_HANDLE_SIGPIPE
static void             handle_interruption_of_results_transmission (int signal_number);
#endif
static gboolean         index_files_have_changed                    (time_t last_index_time);
static void             query_loop                                  (int search_listening_fd,
                                                                     struct sockaddr_un *daemon_address,
                                                                     int address_length);

static void             
exit_search_service (void)
{
        medusa_master_db_unref (master_db);
        gnome_vfs_shutdown ();
        exit (0);
}

static void
daemon_init (void)
{
        pid_t parent_process;
        
        parent_process = fork ();
        /* check if fork failed */
        if (parent_process == -1) {
                puts ("Forking a process to set up the daemon failed.  Exiting now\n");
                exit (1);
        }
        /* only keep the child process */
        if (parent_process != 0) {
                exit (0);
        }
        
        setsid ();
        chdir ("/");
        umask (0);
        
        
}

int main (int argc, char *argv[])
{
	int search_listening_fd;
	struct sockaddr_un *daemon_address;
	int address_length;


        if (getuid () != 0) {
                puts ("The medusa search daemon must be run as root. Exiting now\n");
                exit (1);
        }
        
        /* init taken from Advanced Programming in the UNIX Environment */
        daemon_init ();
        
	
	gnome_vfs_init ();

	/* Block pipe signals so we can survive if a 
	   client interrupts receipt of search results */
        if (signal (SIGPIPE, SIG_IGN) == SIG_ERR) {
                puts ("There has been a problem with the interrupt handler for the search daemon.   Please restart the search daemon");
                exit (-1);
	}


        /* Finish setting up all of the socket internals */
	daemon_address = g_new0 (struct sockaddr_un, 1);
	search_listening_fd = initialize_socket  (daemon_address);
	chmod (SEARCH_SOCKET_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
	address_length = SUN_LEN (daemon_address);

	master_db = NULL;	
        query_loop (search_listening_fd,
                    daemon_address,
                    address_length);

	/* Just here for completeness */
	medusa_master_db_unref (master_db); 
	return 0;
}


static void
query_loop (int search_listening_fd,
            struct sockaddr_un *daemon_address,
            int address_length)
{
	MedusaReadLock *read_lock;
	char transmission[MAX_LINE];
	int transmission_length;
        int client_fd;
        int search_count = 0;

        g_print  ("Ready to receive queries\n");
	for (; ;) {
                /* Receive request */
                client_fd = accept (search_listening_fd, (struct sockaddr *) daemon_address, 
			  	    &address_length);
		g_return_if_fail (client_fd != -1);
                
		/* acquire lock */
		read_lock = medusa_read_lock_get (MEDUSA_FILE_INDEX_READ_LOCK_FILE);
		if (read_lock == NULL) {
			puts ("ERROR: can't acquire database lock");
			exit (-1);
		}
		
    		if (master_db == NULL) {
            		master_db = medusa_master_db_open (ROOT_DIRECTORY,
#ifdef SEARCH_DEBUG
                                                           MEDUSA_DB_LOG_EVERYTHING
#elif defined(SEARCH_LOG_SMALL)
                                                           MEDUSA_DB_LOG_ABBREVIATED,
#elif defined(SEARCH_LOG_ERRORS)
                                                           MEDUSA_DB_LOG_ERRORS,
#else
                                                           MEDUSA_DB_LOG_NOTHING,
#endif
                                                           
                                                           URI_LIST_NAME,
                                                           FILE_SYSTEM_DB_FILE_NAME,
                                                           FILE_NAME_HASH_NAME,
                                                           DIRECTORY_NAME_HASH_NAME,
                                                           MIME_TYPE_HASH_NAME,
                                                           KEYWORD_SETS_FILE_NAME,
                                                           TEXT_INDEX_START_FILE_NAME,
                                                           TEXT_INDEX_LOCATION_FILE_NAME,
                                                           TEXT_INDEX_WORD_FILE_NAME);
                        if (master_db == NULL) {
                                g_warning ("Your index files seem to be missing or invalid.  Searches will not be performed\n");
                                write (client_fd, SEARCH_INDEX_ERROR_TRANSMISSION, strlen (SEARCH_INDEX_ERROR_TRANSMISSION));
                                close (client_fd);
                                continue;
                        }

                        index_validity_time = time (NULL);
    		}
                ensure_index_validity (&master_db); 

		/* Read all of the data and act on it */
		memset (transmission, 0, MAX_LINE - 1);
		transmission_length = read (client_fd, transmission, MAX_LINE - 1);
		while (transmission_length > 0) {
      			medusa_file_search_parse_transmission (transmission, client_fd, master_db);
                        search_count++;
      			memset (transmission, 0, MAX_LINE);
      			transmission_length = read (client_fd, transmission, MAX_LINE - 1);
		}      

		/* release lock */
		medusa_read_lock_release (read_lock);
                if (getenv ("MEDUSA_SEARCH_ONCE") != NULL) {
                        exit_search_service ();
                }
                if (getenv ("MEDUSA_SEARCH_TEN") != NULL &&
                    search_count == 10) {
                        exit_search_service ();
                }
                                                
	}

}

static void
ensure_index_validity (MedusaMasterDB **master_db) 
{
        if (index_files_have_changed (index_validity_time)) {
                medusa_master_db_unref (*master_db);
                *master_db = medusa_master_db_open (ROOT_DIRECTORY,
#ifdef SEARCH_DEBUG
                                                    MEDUSA_DB_LOG_EVERYTHING
#elif defined(SEARCH_LOG_SMALL)
                                                    MEDUSA_DB_LOG_ABBREVIATED,
#elif defined(SEARCH_LOG_ERRORS)
                                                    MEDUSA_DB_LOG_ERRORS,
#else
                                                    MEDUSA_DB_LOG_NOTHING,
#endif
                                                    
                                                    URI_LIST_NAME,
                                                    FILE_SYSTEM_DB_FILE_NAME,
                                                    FILE_NAME_HASH_NAME,
                                                    DIRECTORY_NAME_HASH_NAME,
                                                    MIME_TYPE_HASH_NAME,
                                                    KEYWORD_SETS_FILE_NAME,
                                                    TEXT_INDEX_START_FILE_NAME,
                                                    TEXT_INDEX_LOCATION_FILE_NAME,
                                                    TEXT_INDEX_WORD_FILE_NAME);
                
                if (master_db == NULL) {
                        g_warning ("Could not open the new database.  Please reindex now by typing \"medusa-reindex-now\"\n");
                        sleep (10);
                }

  }
}

static gboolean
index_files_have_changed (time_t last_index_time)
{
        struct stat statbuf;

        /* We could probably get away with stating
           only a few of these files, but unless there are 
           performance issues, it seems best to be safe */
        if (lstat (URI_LIST_NAME, &statbuf) == -1 ||
            statbuf.st_mtime > last_index_time ||
            lstat (FILE_SYSTEM_DB_FILE_NAME, &statbuf) == -1 ||
            statbuf.st_mtime > last_index_time ||
            lstat (FILE_NAME_HASH_NAME, &statbuf) == -1 ||
            statbuf.st_mtime > last_index_time ||
            lstat (DIRECTORY_NAME_HASH_NAME, &statbuf) == -1 ||
            statbuf.st_mtime > last_index_time ||
            lstat (KEYWORD_SETS_FILE_NAME, &statbuf) == -1 ||
            statbuf.st_mtime > last_index_time ||
            lstat (TEXT_INDEX_START_FILE_NAME, &statbuf) == -1 ||
            statbuf.st_mtime > last_index_time ||
            lstat (TEXT_INDEX_LOCATION_FILE_NAME, &statbuf) == -1 ||
            statbuf.st_mtime > last_index_time ||
            lstat (TEXT_INDEX_WORD_FILE_NAME, &statbuf) == -1 ||
            statbuf.st_mtime > last_index_time)
                {
                        return TRUE;
                }
        else {
                return FALSE;
        }
        
}

static int
initialize_socket (struct sockaddr_un *daemon_address)
{
        int search_listening_fd; 

        search_listening_fd = socket (AF_LOCAL, SOCK_STREAM, 0);
        g_return_val_if_fail (search_listening_fd != -1, -1);

        daemon_address->sun_family = AF_LOCAL;
        /* FIXME bugzilla.eazel.com 2635:  
           This number (108) sucks, but it has no #define in the header.
           What to do? (POSIX requires 100 bytes at least, here)  */
        strncpy (daemon_address->sun_path, SEARCH_SOCKET_PATH, 100);

        /* If socket currently exists, delete it */
        unlink (daemon_address->sun_path);


        g_return_val_if_fail (bind (search_listening_fd, (struct sockaddr *) daemon_address, 
                                    SUN_LEN (daemon_address)) != -1, -1);
  
        g_return_val_if_fail (listen (search_listening_fd, 5) != -1, -1);

        return search_listening_fd;
} 


#ifdef MEDUSA_HANDLE_SIGPIPE
static void             
handle_interruption_of_results_transmission (int signal_number)
{
        int search_listening_fd;
        struct sockaddr_un *daemon_address;
        int address_length;

	daemon_address = g_new0 (struct sockaddr_un, 1);
	search_listening_fd = initialize_socket  (daemon_address);
	chmod (SEARCH_SOCKET_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
	
	/* Initialize file system db access */
	master_db = NULL;
#ifdef SEARCH_DAEMON_DEBUG
	printf ("Done initializing database\n");
#endif
	address_length = SUN_LEN (daemon_address);

        query_loop (search_listening_fd,
                    daemon_address,
                    address_length);
}
#endif
