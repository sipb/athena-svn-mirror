/*
 *  session_gate - Keeps session alive by continuing to run
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/session/session_gate.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/session/session_gate.c,v 1.1 1990-11-18 21:54:31 probe Exp $
 *	$Author: probe $
 */

#include <signal.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFSIZ 1024
#define MINUTE 60
#define PID_FILE_TEMPLATE "/tmp/session_gate_pid."

static char filename[80];

int main( );
int itoa( );
void cleanup( );
time_t check_pid_file( );
time_t create_pid_file( );
time_t update_pid_file( );
void write_pid_to_file( );


int main(argc, argv)
int argc;
char **argv;
{
    int pid;
    int parentpid;
    char buf[10];
    time_t mtime;

    pid = getpid();
    parentpid = getppid();
    
    /*  Set up signal handlers for a clean exit  */

    signal(SIGHUP, cleanup);
    signal(SIGINT, cleanup);
    signal(SIGQUIT, cleanup);
    signal(SIGTERM, cleanup);

    /*  Figure out the filename  */

    strcpy(filename, PID_FILE_TEMPLATE);
    itoa(getuid(), buf);
    strcat(filename, buf);

    /*  Put pid in file for the first time  */

    mtime = check_pid_file(filename, pid, (time_t)0);

    /*
     * Now sit and wait.  If a signal occurs, catch it, cleanup, and exit.
     * Every minute, wake up and perform the following checks:
     *   - If parent process does not exist, cleanup and exit.
     *   - If pid file has been modified or is missing, refresh it.
     */
    
    while (1)
      {
	  sleep(MINUTE);
	  if (parentpid != getppid())
	    cleanup();
	  mtime = check_pid_file(filename, pid, mtime);
      }
}


static powers[] = {10000,1000,100,10,1};

int itoa(x, buf)
int x;
char *buf;
{
    int i;
    int pos=0;
    int digit;

    for (i = 0; i < 5; i++)
      {
	  digit = (x/powers[i]) % 10;
	  if ((pos > 0) || (digit != 0) || (i == 4))
	    buf[pos++] = '0' + (char) digit;
      }
    buf[pos] = '\0';
    return pos;
}


void cleanup( )
{
    unlink(filename);
    exit(0);
}


time_t check_pid_file(filename, pid, mtime)
char* filename;
int pid;
time_t mtime;
{
    struct stat st_buf;

    if (stat(filename, &st_buf) == -1)
      return create_pid_file(filename, pid);	/*  File gone:  create  */
    else if (st_buf.st_mtime > mtime)
      return update_pid_file(filename, pid);	/*  File changed:  update  */
    else
      return mtime;				/*  File unchanged  */
}


time_t create_pid_file(filename, pid)
char* filename;
int pid;
{
    int fd;
    struct stat st_buf;

    if ((fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0)) >= 0)
      {
	  write_pid_to_file(pid, fd);
	  fchmod(fd, S_IREAD | S_IWRITE);   /* set file mode 600 */
	  close(fd);
      }

    stat(filename, &st_buf);
    return st_buf.st_mtime;
}


time_t update_pid_file(filename, pid)
char* filename;
int pid;
{
    int fd;
    char buf[BUFSIZ];
    int count;
    char* start;
    char* end;
    struct stat st_buf;

    /*  Determine if the file already contains the pid  */

    if ((fd = open(filename, O_RDONLY, 0)) >= 0)
      {
	  if ((count = read(fd, buf, BUFSIZ-1)) > 0)
	    {
		buf[count-1] = '\0';
		start = buf;
		while ((end = index(start, '\n')) != 0)
		  {
		      *end = '\0';
		      if (atoi(start) == pid)
			{
			    stat(filename, &st_buf);
			    return st_buf.st_mtime;
			}
		      start = end + 1;
		  }
	    }
	  close(fd);
      }

    /*  Append the pid to the file  */

    if ((fd = open(filename, O_WRONLY | O_APPEND, 0)) >= 0)
      {
	  write_pid_to_file(pid, fd);
	  close(fd);
      }
	  
    stat(filename, &st_buf);
    return st_buf.st_mtime;
}


void write_pid_to_file(pid, fd)
int pid;
int fd;
{
    char buf[10];

    itoa(pid, buf);
    strcat(buf, "\n");
    write(fd, buf, strlen(buf));
}
