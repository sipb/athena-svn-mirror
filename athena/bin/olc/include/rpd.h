/*
 * $Id: rpd.h,v 1.22 1999-06-28 22:52:28 ghudson Exp $
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#include <mit-copyright.h>
#include <server_defines.h>
#include "nl_requests.h"
#include "system.h"

#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <limits.h>

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif

#ifdef   HAVE_SYSLOG_H
#include   <syslog.h>
#ifndef    LOG_CONS
#define      LOG_CONS 0  /* if LOG_CONS isn't defined, just ignore it */
#endif     /* LOG_CONS */
#endif /* HAVE_SYSLOG_H */

/* Note: cachesize must be a power of two.  Also, the maximum limit on open */
/* file descriptors in a process should be taken into account. */
#define CACHESIZE    256       /* file descriptor cache size */
#define CACHEWIDTH     8       /* log_2(CACHESIZE) */

#define HASHMUL 1148159605     /* multiplying factor for the hash function */
#define HASHROLL 5             /* cyclic shift size for the hash function */

struct 	entry {
  int fd;		/* file descriptor */
  char *filename;
  time_t last_mod;	/* Time log last modified */
  ino_t inode;		/* Inode of the file */
  int length;		/* Length of the question */
  char *question;	/* pointer to buffer containing question */
  short int use;	/* mark for the clock hand */
  struct entry *next;	/* next entry in the chain */
  struct entry *prev;	/* prev entry in the chain */
};

#define SYSLOG_FACILITY LOG_LOCAL6
#define LOG_DIRECTORY   OLXX_QUEUE_DIR

#define OLC_PROTOCOL   "tcp"

#ifdef HAVE_KRB4
#define K_SERVICE	"olc"

#define SRVTAB      OLXX_CONFIG_DIR "/srvtab"
#define MONITOR_ACL OLXX_ACL_DIR "/monitor.acl"

/* Acl Library */
int acl_check (char *acl, char *principal);
void acl_canonicalize_principal (char *principal , char *canon );
int acl_exact_match (char *acl , char *principal );
#endif /* HAVE_KRB4 */

/* fdcache.c */
void init_cache (void );
char *get_queue (int *result);
char *get_log (char *username , int instance , int *result , int censored);
char *get_file_uncached (char *filename, int *result);
char *get_file_cached (char *filename, int *result);
unsigned get_bucket_index (char *name);
int allocate_entry (void );
void delete_entry (struct entry *ent );

/* get_nm.c */
char *get_nm (char *username , int instance , int *result , int nuke );

/* handle_request.c */
void handle_request (int fd, struct sockaddr_in from);
void punt_connection (int fd, struct sockaddr_in from);

/* io.c */
int sread (int fd , void *buf , int nbytes );
int swrite (int fd , void *buf , int nbytes );
