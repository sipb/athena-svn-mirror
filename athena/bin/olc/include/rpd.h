/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/rpd.h,v 1.4 1990-11-27 14:09:55 lwvanels Exp $
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>

#ifdef KERBEROS
#include <krb.h>
#endif /* KERBEROS */

struct 	entry {
  int fd;		/* file descriptor */
  char username[9];
  int instance;
  char filename[128];
  time_t last_mod;	/* Time log last modified */
  int length;		/* Length of the question */
  char *question;	/* pointer to buffer containing question */
  short int use;	/* mark for the clock hand */
  struct entry *next;	/* next entry in the chain */
  struct entry *prev;	/* prev entry in the chain */
};

#define VERSION 	0


#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif


#ifdef KERBEROS
#define K_SERVICE	"olc"

/* Kerberos Library */
int krb_rd_req P((KTEXT authent, char *service, char *instance,
		  u_long from_addr, AUTH_DAT *ad, char *fn));

#define MONITOR_ACL "/usr/lib/olc/acls/monitor.acl"
/* Acl Library */
int acl_check P((char *acl, char *principal));

#endif /* KERBEROS */



/* system */
int socket P((int domain, int type, int protocol));
int perror P((char *s));
struct servent *getservbyname P((char *name, char *proto));
int bzero P((void *b, int length));
int bind P((int s, struct sockaddr *name, int namelen));
int listen P((int s, int backlog));
int accept P((int s, struct sockaddr *addr, int *addrlen));
int read P((int d, void *buf, int nbytes));
int close P((int d));
int open P((char *path, int flags, int mode));
int write P((int d, void *buf, int nbytes));
int fstat P((int fd, struct stat *buf));
void *malloc P((unsigned size));
int free P((void *ptr));
int shutdown P((int s, int how));
char *inet_ntoa P((struct in_addr in));
int psignal P((unsigned sig, char *s));

/* rpd.c */
int clean_up P((int signal));

/* fdcache.c */
void init_cache P((void ));
char *get_log P((char *username , int instance , int *result ));
int get_bucket_index P((char *username , int instance ));
int allocate_entry P((void ));
void delete_entry P((struct entry *ent ));

/* handle_request.c */
void handle_request P((int fd, struct sockaddr_in from));
void punt_connection P((int fd, struct sockaddr_in from));

/* io.c */
int sread P((int fd , void *buf , int nbytes ));
int swrite P((int fd , void *buf , int nbytes ));

#undef P
