#define WANT_IRS_NIS
#undef WANT_IRS_PW
#define SIG_FN void
#define SYSV
#define _POSIX_SOURCE
#include <sys/types.h>

#include <limits.h>	/* _POSIX_PATH_MAX */
#include <sys/times.h>
#include <sys/time.h>
struct timespec {
	long	tv_sec;
	long	tv_nsec;
};
