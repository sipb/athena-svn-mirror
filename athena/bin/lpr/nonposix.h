/*
 * BSD functions which can't be realized in POSIX
 */

#if defined(i386) && defined(_POSIX_SOURCE)
/*
 * Missing error codes in 386BSD 0.1
 */
#define EADDRINUSE      48              /* Address already in use */
#define ECONNREFUSED    61              /* Connection refused */
#define EWOULDBLOCK     EAGAIN          /* Operation would block */

/*
 * Link specific stuff .. (used in lpd/printjob.c need some alternatives)
 */
#define	S_IFMT	 0170000		/* type of file */
#define	S_IFLNK	 0120000		/* symbolic link */
#define	S_IFDIR	 0040000		/* directory */
#endif

int chksize(int size);
int isexec(int);
