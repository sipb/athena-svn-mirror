/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/rpd.h,v 1.8 1990-12-02 23:10:25 lwvanels Exp $
 */

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

#ifdef KERBEROS
#include <krb.h>
#endif /* KERBEROS */

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

#define VERSION 	0
#define SYSLOG_FACILITY LOG_LOCAL6

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif


#ifdef KERBEROS
#define K_SERVICE	"olc"

#define MONITOR_ACL "/usr/lib/olc/acls/monitor.acl"

/* Acl Library */
int acl_check P((char *acl, char *principal));
void acl_canonicalize_principal P((char *principal , char *canon ));
int acl_exact_match P((char *acl , char *principal ));
#endif /* KERBEROS */


/* system */
int accept P((int s, struct sockaddr *addr, int *addrlen));
int bind P((int s, struct sockaddr *name, int namelen));
int bzero P((void *b, int length));
void *calloc P((unsigned nelem, unsigned elsize));
int close P((int d));
int exit P((int status));
int free P((void *ptr));
int fstat P((int fd, struct stat *buf));
int getdtablesize P(());
struct servent *getservbyname P((char *name, char *proto));
char *index P((char *s, int c));
char *inet_ntoa P((struct in_addr in));
void ioctl P((int d, unsigned int request, char *argp));
int listen P((int s, int backlog));
off_t lseek P((int d, off_t offset, int whence));
void *malloc P((unsigned size));
int open P((char *path, int flags, int mode));
void openlog P((char *ident, int logopt, int facility));
#ifdef aix
void perror P((char *s));
#else
int perror P((char *s));
#endif
int psignal P((unsigned sig, char *s));
int read P((int d, void *buf, int nbytes));
int setsockopt P((int s, int level, int optname, void *optval, int optlen));
int shutdown P((int s, int how));
int socket P((int domain, int type, int protocol));
int stat P((char *path, struct stat *buf));
int strcmp P((char *s1, char *s2));
char *strcpy P((char *s1, char *s2));
int strlen P((char *s));
char *strncpy P((char *s1, char *s2, int n));
void syslog P((int priority, char *message, ...));
int write P((int d, void *buf, int nbytes));

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
