#undef HAVE_SA_LEN
#define USE_POSIX
#define POSIX_SIGNALS
#define USE_WAITPID
#define HAVE_FCHMOD
#define NEED_PSELECT
#define SETGRENT_VOID
#define SETPWENT_VOID
#define SIOCGIFCONF_ADDR
#define HAVE_CHROOT
#define CAN_CHANGE_ID

#define PORT_NONBLOCK	O_NONBLOCK
#define PORT_WOULDBLK	EWOULDBLOCK
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
 * has had this type added.  The type number below is correct on most BSD-
 * derived systems for which AF_INET6 is defined.
 */
#ifndef AF_INET6
#define AF_INET6	24
#endif

#include <sys/types.h>
extern int gethostname(char *, size_t);

#define NEED_STRSEP
extern char *strsep(char **, const char *);

#define NEED_DAEMON
int daemon(int nochdir, int noclose);

#define NEED_UTIMES
#include <sys/time.h>
#define utimes __utimes
int __utimes(char *, struct timeval *);

#undef _ptr

/*
 * RLIM_INFINITY is in <sys/resource.h> in HP-UX 9.x but within an
 * #ifdef _KERNEL. HP-UX 10.20 has the same value for RLIM_INFINITY,
 * this time it's not within an #ifdef _KERNEL.
 */
#ifndef RLIM_INFINITY
#define RLIM_INFINITY	0x7fffffff
#endif

#define HAVE_GETRUSAGE
#include <syscall.h>
#define getrusage(w,ru) syscall(SYS_GETRUSAGE,w,ru,0,0,0,0,0,0,0)

/*
 * Even though the server uses the POSIX signal API, on HP-UX it still gets
 * "one shot" behavior with the SIGPIPE signal handler.  Defining
 * SIGPIPE_ONE_SHOT will cause the the SIGPIPE handler to be reinstalled
 * after it is caught.
 */
#define SIGPIPE_ONE_SHOT
