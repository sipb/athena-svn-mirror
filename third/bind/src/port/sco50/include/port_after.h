#undef  HAS_SA_LEN
#define USE_POSIX
#define POSIX_SIGNALS
#define USE_WAITPID
#define WAIT_T int
#define SETGRENT_VOID
#define SETPWENT_VOID
#define HAVE_FCHMOD
#define HAVE_GETRUSAGE	/* have getrlimit(), setrlimit(),
			   but only partially emulated getrusage()
			*/
#define SIOCGIFCONF_ADDR
#define SIG_FN void
/* #define USE_UTIME /**/
#define CAN_RECONNECT /* ??? */
#define HAVE_CHROOT
#define CAN_CHANGE_ID

#define PORT_NONBLOCK	O_NONBLOCK
#define PORT_WOULDBLK	EWOULDBLOCK

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

#define NEED_PSELECT

#include <sys/resource.h>
#define getrusage(x,y) sco_getrusage(x,y)
int getrusage(int who, struct rusage *usage);
