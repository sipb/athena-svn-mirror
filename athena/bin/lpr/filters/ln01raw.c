#ifdef _AUX_SOURCE
#include <sys/types.h>
#endif
#include <sys/file.h>

main()
{
    register int count;
    char buf[1024];
    
    flock(1, LOCK_EX);
    while ((count = read(0, buf, 1024)) > 0)
    	write(1, buf, count);
    flock(1, LOCK_UN);
}
