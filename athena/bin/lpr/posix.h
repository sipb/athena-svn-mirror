/*
 * BSD functions in POSIX
 */

#define   LOCK_SH   1    /* shared lock */
#define   LOCK_EX   2    /* exclusive lock */
#define   LOCK_NB   4    /* don't block when locking */
#define   LOCK_UN   8    /* unlock */

int
flock(int fd, int operation);

typedef void    Sigfunc(int);   /* for signal handlers */

		/* 4.3BSD Reno <signal.h> doesn't define SIG_ERR */
#if     defined(SIG_IGN) && !defined(SIG_ERR)
#define SIG_ERR ((Sigfunc *)-1)
#endif

Sigfunc *
signal(int signo, Sigfunc *func);
