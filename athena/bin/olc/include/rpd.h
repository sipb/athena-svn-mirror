/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/rpd.h,v 1.10 1991-01-08 16:47:49 lwvanels Exp $
 */

#include "requests.h"
#include "system.h"

#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <syslog.h>

struct 	entry {
  int fd;		/* file descriptor */
  char username[9];
  int instance;
  char filename[128];
  time_t last_mod;	/* Time log last modified */
  ino_t inode;		/* Inode of the file */
  int length;		/* Length of the question */
  char *question;	/* pointer to buffer containing question */
  short int use;	/* mark for the clock hand */
  struct entry *next;	/* next entry in the chain */
  struct entry *prev;	/* prev entry in the chain */
};

#define SYSLOG_FACILITY LOG_LOCAL6
#define LOG_DIRECTORY "/usr/spool/olc"


#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif


#ifdef KERBEROS
#define K_SERVICE	"olc"

#define MONITOR_ACL "/usr/lib/olc/acls/monitor.acl"

/* system */
char *inet_ntoa P((struct in_addr in));
int accept P((int s, struct sockaddr *addr, int *addrlen));
int bind P((int s, struct sockaddr *name, int namelen));
int connect P((int s, struct sockaddr *name, int namelen));
int fstat P((int fd, struct stat *buf));
int stat P((char *path, struct stat *buf));


/* Acl Library */
int acl_check P((char *acl, char *principal));
void acl_canonicalize_principal P((char *principal , char *canon ));
int acl_exact_match P((char *acl , char *principal ));
#endif /* KERBEROS */

/* rpd.c */
int clean_up P((int signal));

/* fdcache.c */
void init_cache P((void ));
char *get_log P((char *username , int instance , int *result ));
int get_bucket_index P((char *username , int instance ));
int allocate_entry P((void ));
void delete_entry P((struct entry *ent ));

/* get_nm.c */
char *get_nm P((char *username , int instance , int *result , int nuke ));

/* handle_request.c */
void handle_request P((int fd, struct sockaddr_in from));
void punt_connection P((int fd, struct sockaddr_in from));

#ifdef KERBEROS
/* kopt.c */
int krb_set_key P((char *key , int cvt ));
int krb_rd_req P((KTEXT authent , char *service , char *instance , long from_addr , AUTH_DAT *ad , char *fn ));
int krb_get_lrealm P((char *r , int n ));
#endif /* KERBEROS */

/* io.c */
int sread P((int fd , void *buf , int nbytes ));
int swrite P((int fd , void *buf , int nbytes ));

#undef P
