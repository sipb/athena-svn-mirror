#undef HAS_SA_LEN
#define POSIX_SIGNALS
#define USE_WAITPID
#define WAIT_T int
#define SETGRENT_VOID
#define SETPWENT_VOID
#define USE_UTIME

#define PORT_NONBLOCK	O_NONBLOCK
#define PORT_WOULDBLK	EWOULDBLOCK

# define _S_IFREG S_IFREG

#undef _PATH_NAMED
#define _PATH_NAMED	"/etc/named"
#undef _PATH_XFER
#define _PATH_XFER	"/etc/named-xfer"
#undef _PATH_DUMPFILE
#define _PATH_DUMPFILE	"/usr/tmp/named_dump.db"
#undef _PATH_PIDFILE
#define _PATH_PIDFILE	"/etc/named.pid"

/*
 * We need to know the IPv6 address family number even on IPv4-only systems.
 * Note that this is NOT a protocol constant, and that if the system has its
 * own AF_INET6, different from ours below, all of BIND's libraries and
 * executables will need to be recompiled after the system <sys/socket.h>
 * has had this type added.  The type number below is correct on most BSD-
 * derived systems for which AF_INET6 is defined.
 */
#ifndef AF_INET6
#define AF_INET6	24
#endif

#define NEED_STRSEP
extern char *strsep(char **, const char *);

#define NEED_DAEMON
int daemon(int nochdir, int noclose);

#define	NEED_MKSTEMP
int mkstemp(char *);

#include <sys/uio.h>
#define NEED_WRITEV
#define writev __writev
int __writev(int, const struct iovec*, int);

#define NEED_READV
#define readv __readv
int __readv(int, const struct iovec*, int);

#define NEED_UTIMES
#include <sys/time.h>
#define utimes __utimes
int __utimes(char *, struct timeval *);

#define ftruncate __ftruncate
int __ftruncate(int, long);

#define vfork fork

extern char *optarg;
extern int optind, opterr;

/* SCO3.2v4.2 provides realloc(), but it doesn't have ANSI C semantics. */
#include "ansi_realloc.h"


/* SCO3.2v4.2 provides gettimeofday(), but it has some problems. */
#include "sco_gettime.h"
