#include "os_version.h"

#undef HAS_SA_LEN
#define USE_POSIX
#define POSIX_SIGNALS
#define NETREAD_BROKEN
#define USE_WAITPID
#define HAVE_FCHMOD
#define NEED_PSELECT
#define SETGRENT_VOID
#define SETPWENT_VOID
#define SIOCGIFCONF_ADDR
#define IP_OPT_BUF_SIZE 40
#define HAVE_CHROOT
#define CAN_CHANGE_ID

#define PORT_NONBLOCK	O_NONBLOCK
#define PORT_WOULDBLK	EWOULDBLOCK
#define WAIT_T		int
#define INADDR_NONE	0xffffffff

#ifndef MIN
# define MIN(x, y)	((x > y) ?y :x)
#endif
#ifndef MAX
# define MAX(x, y)	((x > y) ?x :y)
#endif

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

/*
 * Prior to 2.6, Solaris needs a prototype for gethostname().
 */
#if (OS_MAJOR == 5 && OS_MINOR < 6)
#include <sys/types.h>
extern int gethostname(char *, size_t);
#endif

#define NEED_STRSEP
extern char *strsep(char **, const char *);

#define NEED_DAEMON
int daemon(int nochdir, int noclose);

/*
 * Solaris defines this in <netdb.h> instead of in <sys/param.h>.  We don't
 * define it in our <netdb.h>, so we define it here.
 */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

/*
 * Solaris 2.5 and later have getrlimit(), setrlimit() and getrusage().
 */
#if (OS_MAJOR > 5 || (OS_MAJOR == 5 && OS_MINOR >= 5))
#include <sys/resource.h>
#define HAVE_GETRUSAGE
#define RLIMIT_TYPE rlim_t
#define RLIMIT_FILE_INFINITY
#endif
