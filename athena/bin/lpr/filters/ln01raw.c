#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

main()
{
    struct flock lock;
    int count;
    char buf[1024];
    
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    fcntl(STDOUT_FILENO, F_SETLKW, &lock);
    while ((count = read(STDIN_FILENO, buf, 1024)) > 0)
    	write(STDOUT_FILENO, buf, count);
    lock.l_type = F_UNLCK;
    fcntl(STDOUT_FILENO, F_SETLK, &lock);
}
