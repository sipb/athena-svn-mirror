/*
 *  session_gate - Keeps session alive by continuing to run
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/session/session_gate.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/session/session_gate.c,v 1.9 1993-05-20 12:52:48 miki Exp $
 *	$Author: miki $
 */

#include <signal.h>
#ifndef SOLARIS
#include <strings.h>
#else
#include <strings.h>
#include <sys/fcntl.h>
#endif
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define BUFSIZ 1024
#define MINUTE 60
#define PID_FILE_TEMPLATE "/tmp/session_gate_pid."

static char filename[80];

void main( );
int itoa( );
void cleanup( );
void logout( );
void clean_child( );
time_t check_pid_file( );
time_t create_pid_file( );
time_t update_pid_file( );
void write_pid_to_file( );


void main(argc, argv)
int argc;
char **argv;
{
    int pid;
    int parentpid;
    char buf[10];
    time_t mtime;
    int dologout = 0;

    if (argc == 2 && !strcmp(argv[1], "-logout"))
      dologout = 1;

    pid = getpid();
    parentpid = getppid();
    
    /*  Set up signal handlers for a clean exit  */

    if (dologout) {
	signal(SIGHUP, logout);
	signal(SIGINT, logout);
	signal(SIGQUIT, logout);
	signal(SIGTERM, logout);
    } else {
	signal(SIGHUP, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGTERM, cleanup);
    }
    signal(SIGCHLD, clean_child);	/* Clean up zobmied children */

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
	  clean_child();	/* In case there are any zombied children */
	  if (parentpid != getppid())
	    cleanup();
	  mtime = check_pid_file(filename, pid, mtime);
      }
}

static powers[] = {100000,10000,1000,100,10,1,0};

int itoa(x, buf)
int x;
char *buf;
{
    int i;
    int pos=0;
    int digit;

    for (i = 0; powers[i]; i++)
      {
	  digit = (x/powers[i]) % 10;
	  if ((pos > 0) || (digit != 0) || (powers[i+1] == 0))
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

void logout( )
{
    int f;

    /* Ignore Alarm in child since HUP probably arrived during
     * sleep() in main loop.
     */
    signal(SIGALRM, SIG_IGN);
    unlink(filename);
    f = open(".logout", O_RDONLY, 0);
    if (f >= 0) {
	dup2(f, 0);
	execl("/bin/athena/tcsh", "logout", 0);
    }
    exit(0);
}

/*
 * clean_child() --
 * 	Clean up any zombied children that are awaiting this process's
 * 	acknowledgement of the child's death.
 */
void clean_child( )
{
    wait3(0, WNOHANG, 0);
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
