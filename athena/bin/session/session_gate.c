/*
 *  session_gate - Keeps session alive by continuing to run
 *
 *	$Id: session_gate.c,v 1.19 2000-04-20 14:32:20 tb Exp $
 */

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef SGISession
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

int SGISession_Initialize(void);
void flaglogout(void);
int SGISession_Wait(void);
int SGISession_WhoCares(void);
void SGISession_EndSession(void);
void SGISession_Debug(char *message);
#endif

#define MINUTE 60
#define PID_FILE_TEMPLATE "/tmp/session_gate_pid."

static char filename[80];

void cleanup(void);
void logout(void);
void clean_child(void);
time_t check_pid_file(char *filename, pid_t pid, time_t mtime);
time_t create_pid_file(char *filename, pid_t pid);
time_t update_pid_file(char* filename, pid_t pid);
void write_pid_to_file(pid_t pid, int fd);

int main(int argc, char **argv)
{
    pid_t pid, parentpid;
    time_t mtime;
    int dologout = 0;
    struct sigaction sa;
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
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

#ifdef SGISession
    sa.sa_handler = flaglogout;
#else
    if (dologout)
      sa.sa_handler = logout;
    else
      sa.sa_handler = cleanup;
#endif
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Clean up zobmied children */
    sa.sa_handler = clean_child;
    sigaction(SIGCHLD, &sa, NULL);

    /*  Figure out the filename  */

    sprintf (filename, "%s%d", PID_FILE_TEMPLATE, getuid());

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
int SGISession_Initialize(void)
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

void flaglogout(void)
{
  logoutsignal++;
}

int SGISession_Wait(void)
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

int SGISession_WhoCares(void)
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

void SGISession_EndSession(void)
{
  system("/usr/bin/X11/endsession -f");
}

void SGISession_Debug(char *message)
{
  if (SGISession_debug)
    fprintf(stderr, "session_gate debug: %s", message);
}
#endif

void cleanup(void)
{
    unlink(filename);
    exit(0);
}

void logout(void)
{
    int f;
    struct sigaction sa;

    /* Ignore Alarm in child since HUP probably arrived during
     * sleep() in main loop.
     */
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;
    sigaction(SIGALRM, &sa, NULL);
    unlink(filename);
    f = open(".logout", O_RDONLY, 0);
    if (f >= 0) {
	char buf[2];

	if (read(f, buf, 2) == -1)
	    exit(0);
	lseek(f, 0, SEEK_SET);
	if (!memcmp(buf, "#!", 2))
	    execl(".logout", ".logout", 0);
	/* if .logout wasn't executable, silently fall through to
	   old behavior */
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
void clean_child(void)
{
    waitpid((pid_t)-1, 0, WNOHANG);
}

time_t check_pid_file(char *filename, pid_t pid, time_t mtime)
{
    struct stat st_buf;

    if (stat(filename, &st_buf) == -1)
      return create_pid_file(filename, pid);	/*  File gone:  create  */
    else if (st_buf.st_mtime > mtime)
      return update_pid_file(filename, pid);	/*  File changed:  update  */
    else
      return mtime;				/*  File unchanged  */
}


time_t create_pid_file(char *filename, pid_t pid)
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


time_t update_pid_file(char* filename, pid_t pid)
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
		while ((end = strchr(start, '\n')) != 0)
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


void write_pid_to_file(pid_t pid, int fd)
{
    char buf[50];

    sprintf (buf, "%d\n", pid);
    write(fd, buf, strlen(buf));
}
