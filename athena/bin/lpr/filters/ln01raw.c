#ifdef _AUX_SOURCE
#include <sys/types.h>
#endif
#include <sys/file.h>
#ifdef SOLARIS
/*
 * flock operations.
 */
#define LOCK_SH               1       /* shared lock */
#define LOCK_EX               2       /* exclusive lock */
#define LOCK_NB               4       /* don't block when locking */
#define LOCK_UN               8       /* unlock */
#endif

main()
{
    register int count;
    char buf[1024];
    
    flock(1, LOCK_EX);
    while ((count = read(0, buf, 1024)) > 0)
    	write(1, buf, count);
    flock(1, LOCK_UN);
}
