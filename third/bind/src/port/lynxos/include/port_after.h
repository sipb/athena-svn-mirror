#define BSD 43
#define USE_POSIX
#define POSIX_SIGNALS
#define NETREAD_BROKEN
#define NEED_STRSEP
#define USE_WAITPID
#define HAVE_GETRUSAGE
#define HAVE_FCHMOD
#define SPRINTF_CHAR
#define VSPRINTF_CHAR
#define CHECK_UDP_SUM
#define FIX_UDP_SUM
#define __BIND_RES_TEXT
#define HAVE_SA_LEN
#define HAVE_MINIMUM_IFREQ
#define NEED_STRCASECMP
#define NEED_MKSTEMP
#define HAVE_CHROOT
#define CAN_CHANGE_ID

#define PORT_NONBLOCK	O_NONBLOCK
#define PORT_WOULDBLK	EWOULDBLOCK
#define WAIT_T		int
#ifdef PATH_MAX
#undef PATH_MAX
#endif
#define PATH_MAX	_POSIX_PATH_MAX
#define INADDR_NONE	0xffffffff
#define KSYMS		"/vmunix"
#define KMEM		"/dev/kmem"
#define UDPSUM		"_udp_cksum"

/* #define _TIMEZONE timezone */

#ifndef MIN
# define MIN(x, y)	((x > y) ?y :x)
#endif
#ifndef MAX
# define MAX(x, y)	((x > y) ?x :y)
#endif

#define memmove(src,dst,len) bcopy(dst,src,len)

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

#include <sys/types.h>

extern char *strsep(char **, const char *);

#define NEED_PSELECT

#define vfork fork
#define nice(x)

#define NEED_DAEMON
int daemon(int nochdir, int noclose);

/* SunOS provides this but there is no include file that prototypes it. */
int getopt(int, char **, char *);
extern char *optarg;
extern int optind, opterr;

/* SunOS provides vsprintf but doesn't prototype it. */
#include <stdarg.h>

/* SunOS provides realloc, but it doesn't have ANSI C semantics */
#include "ansi_realloc.h"

#ifndef RAND_MAX
#define RAND_MAX 0x7fffffff
#endif
