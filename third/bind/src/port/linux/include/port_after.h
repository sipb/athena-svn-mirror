#define CAN_RECONNECT
#define USE_POSIX
#define POSIX_SIGNALS
#define USE_UTIME
#define USE_WAITPID
#define HAVE_GETRUSAGE
#define HAVE_FCHMOD
#define USE_SETSID
#define SETGRENT_VOID
#define SIOCGIFCONF_ADDR

#if __GLIBC__ >= 2
#  define dprintf bind_dprintf
#else
#  define NEED_PSELECT
#  define NEED_DAEMON
int daemon(int nochdir, int noclose);
#endif

#undef _PATH_NAMED
#define _PATH_NAMED	"/usr/sbin/named"
#undef _PATH_XFER
#define _PATH_XFER	"/usr/sbin/named-xfer"

#define SPURIOUS_ECONNREFUSED	/* XXX is this still needed for 2.0 kernels? */
#define _TIMEZONE timezone

#define PORT_NONBLOCK	O_NONBLOCK
#define PORT_WOULDBLK	EAGAIN
#define WAIT_T		int

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
 * has had this type added.  The type number below is correct for Linux
 * systems.
 */
#ifndef AF_INET6
#define AF_INET6	10
#endif
