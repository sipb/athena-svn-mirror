/*
 *  session_gate - Keeps session alive by continuing to run
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/session/session_gate.c,v $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/session/session_gate.c,v 1.12 1995-05-25 22:17:31 cfields Exp $
 *	$Author: cfields $
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

#ifdef SGISession
#include <stdio.h>
#include <errno.h>
#include <X11/Xlib.h>

Display *dpy;
Window root;
Atom sessionAtom, wmAtom;
int logoutsignal = 0;
int SGISession_debug = 0;

#define SGISession_TIMEOUT 1
#define SGISession_ATHENALOGOUT 2
#define SGISession_SGILOGOUT 3

#define SGISession_NOBODY 1
#define SGISession_WMONLY 2
#define SGISession_EVERYONE 3

void flaglogout();
void SGISession_EndSession(), SGISession_Debug();
#endif

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
#ifdef SGISession
    int logoutStarted = 0;


    while (*argv)
      {
	if (!strcmp(*argv, "-logout"))
	  dologout = 1;

	if (!strcmp(*argv, "-debug"))
	  SGISession_debug = 1;

	argv++;
      }
#else
    if (argc == 2 && !strcmp(argv[1], "-logout"))
      dologout = 1;
#endif

    pid = getpid();
    parentpid = getppid();
    
    /*  Set up signal handlers for a clean exit  */

#ifdef SGISession
    	signal(SIGHUP, flaglogout);
	signal(SIGINT, flaglogout);
	signal(SIGQUIT, flaglogout);
	signal(SIGTERM, flaglogout);
#else
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
#endif

#ifdef SYSV
    sigset(SIGCHLD, clean_child);	/* Clean up zobmied children */
#else
    signal(SIGCHLD, clean_child);	/* Clean up zobmied children */
#endif

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

#ifdef SGISession
#define goodbye() if (dologout) logout(); else cleanup()

    if (0 == SGISession_Initialize())
      {
	SGISession_Debug("Gating an SGI session\n");
	while (1)
	  {
	    switch(SGISession_Wait())
	      {
	      case SGISession_TIMEOUT: /* MINUTE passed */
		SGISession_Debug("chime\n");
		clean_child();	/* In case there are any zombied children */
		if (parentpid != getppid())
		  cleanup();
		mtime = check_pid_file(filename, pid, mtime);
		break;

	      case SGISession_ATHENALOGOUT: /* HUP signal */
		SGISession_Debug("Received hangup\n");
		logoutsignal = 0;
		if (logoutStarted)
		  {
		    /* This is our second signal. Something bad must
		       have happened the first time. Just log the user
		       out. */
		    SGISession_Debug("Second hangup; doing Athena logout\n");
		    goodbye();
		  }

		logoutStarted = 1;
		switch(SGISession_WhoCares())
		  {
		  case SGISession_NOBODY:
		    SGISession_Debug("Nobody cares; doing Athena logout\n");
		    goodbye();
		    break;
		  case SGISession_WMONLY:
		    SGISession_Debug(
"Window manager about cares session but reaper was not run;\n\
calling endsession and doing Athena logout\n");
		    SGISession_EndSession();
		    goodbye();
		    break;
		  case SGISession_EVERYONE:
		    SGISession_Debug(
		     "starting endsession and waiting for window property\n");
		    SGISession_EndSession();
		    break;
		  }
		break;

	      case SGISession_SGILOGOUT: /* property deleted */
		SGISession_Debug(
			 "Window property deleted; doing Athena logout\n");
		goodbye();
		break;
	      }
	  }
      }
    else
 SGISession_Debug("Gating a normal session; SGISession_Initialize failed\n");
#endif
    
    while (1)
      {
	  sleep(MINUTE);
	  clean_child();	/* In case there are any zombied children */
	  if (parentpid != getppid())
	    cleanup();
	  mtime = check_pid_file(filename, pid, mtime);
      }
}

#ifdef SGISession
int SGISession_Initialize()
{
  int screen;

  dpy = XOpenDisplay(NULL);

    if (dpy == NULL)
      return -1;

  screen = DefaultScreen(dpy);
  root = RootWindow(dpy, screen);

  sessionAtom = XInternAtom(dpy, "_SGI_SESSION_PROPERTY", False);
  wmAtom = XInternAtom(dpy, "_SGI_TELL_WM", False);

  XSelectInput(dpy, root, PropertyChangeMask);
  return 0;
}

void flaglogout( )
{
  logoutsignal++;
}

int SGISession_Wait()
{
  static int initialized = 0;
  static fd_set read;
  static struct timeval timeout;
  int selret;
  XEvent event;

  if (!initialized)
    {
      FD_ZERO(&read);
      timeout.tv_sec = MINUTE;
      timeout.tv_usec = 0;
      initialized++;
    }

  while (1)
    {
      while (XPending(dpy) > 0)
	{
	  if (logoutsignal)
	    return SGISession_ATHENALOGOUT;

	  XNextEvent(dpy, &event);
	  if (event.type == PropertyNotify &&
	      event.xproperty.atom == sessionAtom &&
	      event.xproperty.state == PropertyDelete)
	    return SGISession_SGILOGOUT;
	}

      FD_SET(ConnectionNumber(dpy), &read);

      if (logoutsignal)
	return SGISession_ATHENALOGOUT;

      selret = select(ConnectionNumber(dpy) + 1,
		      &read, NULL, NULL, &timeout);
      switch(selret)
	{
	case -1:
	  if (errno == EINTR && logoutsignal)
	    return SGISession_ATHENALOGOUT;
	  if (errno != EINTR) /* !SIGCHLD, basically */
	    fprintf(stderr,
		    "session_gate: Unexpected error %d from select\n",
		    errno);
	  break;
	case 0:
	  return SGISession_TIMEOUT;
	  break;
	default:
	  if (!FD_ISSET(ConnectionNumber(dpy), &read))
	    {
	      /* This can't happen. */
	      fprintf(stderr,
		      "session_gate: select returned data for unknown fd\n");
	      exit(0);
	    }
	  break;
	}
    }
}

int SGISession_WhoCares()
{
  Atom *properties;
  int i, numprops;
  int wmcares = 0, wecare = 0;

  properties = XListProperties(dpy, root, &numprops);
  if (properties)
    {
      for (i = 0; i < numprops; i++)
	if (properties[i] == sessionAtom)
	  wecare = 1;
	else
	  if (properties[i] == wmAtom)
	    wmcares = 1;
    }
  else
    return SGISession_NOBODY;

  XFree(properties);

  if (wmcares && wecare)
    return SGISession_EVERYONE;

  if (wmcares)
    return SGISession_WMONLY;

  return SGISession_NOBODY;
}

void SGISession_EndSession()
{
  system("/usr/bin/X11/endsession -f");
}

void SGISession_Debug(message)
     char *message;
{
  if (SGISession_debug)
    fprintf(stderr, "session_gate debug: %s", message);
}
#endif


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
#ifdef POSIX
    waitpid((pid_t)-1, 0, WNOHANG);
#else
    wait3(0, WNOHANG, 0);
#endif
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
