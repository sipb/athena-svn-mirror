/* This top portion comes from the HPUX port and is mostly unaltered. */

#undef WANT_IRS_NIS
#undef WANT_IRS_PW
#define SIG_FN void

#include <limits.h>	/* _POSIX_PATH_MAX */

/* This bottom portion is MPE specific. */

#include <sys/types.h>

#define bind		__bind_mpe_bind
#define fsync		__bind_mpe_fsync
#define ftruncate	__bind_mpe_ftruncate
#define nice		__bind_mpe_nice
#define fcntl		__bind_mpe_fcntl
#define recvfrom	__bind_mpe_recvfrom
#define setgid		__bind_mpe_setgid
#define setuid		__bind_mpe_setuid
#define initgroups	__bind_mpe_initgroups
#define endgrent	__bind_mpe_endgrent	
#define endpwent	__bind_mpe_endpwent	

struct timespec { 
        time_t  tv_sec;         /* seconds */ 
        long    tv_nsec;        /* nanoseconds */
};
