/* Copyright 2001 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

static const char rcsid[] = "$Id: dustbuster.c,v 1.2 2001-05-30 02:15:28 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <X11/Xlib.h>

static void xbust(char **argv);
static void ttybust(char **argv);
static void start_child(char **argv);
static int xhandler(Display *dpy);
static void child_handler(int signo);
static int find_signal(const char *signame);
static void usage(void);

static const char *progname;
static pid_t child_pid = 0;
static int sig = SIGHUP;

int main(int argc, char **argv)
{
  const char *signame;

  progname = strrchr(argv[0], '/');
  progname = (progname != NULL) ? progname + 1 : argv[0];

  /* Parse arguments.  Can't use getopt() because glibc's getopt permutes,
   * and we really don't want to confuse our options with the subcommand
   * options.
   */
  argv++;
  while (*argv != NULL && **argv == '-')
    {
      if ((*argv)[1] == '-')
	{
	  argv++;
	  break;
	}
      switch ((*argv)[1])
	{
	case 's':
	  if ((*argv)[2] != '\0')	/* -sSIGNAME */
	    signame = *argv + 2;
	  else if (argv[1] == NULL)
	    usage();
	  else
	    signame = *++argv;		/* -s SIGNAME */
	  sig = find_signal(signame);
	  argv++;
	  break;

	default:
	  usage();
	  break;
	}
    }
  if (*argv == NULL)
    usage();

  /* We conditionalize on XSESSION (set by the Athena default xsession
   * script) instead of DISPLAY, so that we won't hold open the
   * display when someone does a tty login via ssh with X forwarding.
   */
  if (getenv("XSESSION") != NULL)
    xbust(argv);
  else
    ttybust(argv);
  return 0;
}

static void xbust(char **argv)
{
  Display *dpy;
  XEvent event;

  /* Open the display. */
  dpy = XOpenDisplay(NULL);
  if (dpy == NULL)
    {
      fprintf(stderr, "%s: error: can't open display\n", progname);
      exit(1);
    }
  XSetIOErrorHandler(xhandler);

  /* Start the child process. */
  start_child(argv);

  /* Wait until either the X connection dies (xhandler) or the child
   * process dies (child_handler).
   */
  while (1)
    XNextEvent(dpy, &event);
}

static void ttybust(char **argv)
{
  int fd;

  /* Make sure controlling tty can be opened to start with. */
  fd = open("/dev/tty", O_RDWR, 0);
  if (fd == -1)
    {
      fprintf(stderr, "%s: error: can't open controlling tty: %s\n",
	      progname, strerror(errno));
      exit(1);
    }
  close(fd);

  /* Start the child process. */
  start_child(argv);

  /* Wait until the child process exits (child_handler) or the
   * controlling tty can no longer be opened.
   */
  while (1)
    {
      fd = open("/dev/tty", O_RDWR, 0);
      if (fd == -1)
	{
	  kill(child_pid, sig);
	  exit(0);
	}
      close(fd);
      sleep(5);
    }
}

/* Set up the SIGCHLD handler and start the child process given by
 * argv.  Sets child_pid.
 */
static void start_child(char **argv)
{
  struct sigaction sa;
  sigset_t set, oset;

  /* Set up the SIGCHLD handler. */
  sa.sa_handler = child_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDSTOP;
  sigaction(SIGCHLD, &sa, NULL);

  /* Create the subprocess, blocking SIGCHLD until child_pid is assigned. */
  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  sigprocmask(SIG_BLOCK, &set, &oset);
  child_pid = fork();
  sigprocmask(SIG_SETMASK, &oset, NULL);
  if (child_pid == -1)
    {
      fprintf(stderr, "%s: error: can't create subprocess: %s\n", progname,
	      strerror(errno));
      exit(1);
    }

  /* In the child, run the specified program. */
  if (child_pid == 0)
    {
      execvp(argv[0], argv);
      fprintf(stderr, "%s: error: can't execute %s: %s\n", progname, argv[0],
	      strerror(errno));
      _exit(1);
    }
}

/* Xlib calls this when a fatal I/O error happens (such as the display
 * closing.  Send a SIGHUP to the child pid and exit.
 */
static int xhandler(Display *dpy)
{
  kill(child_pid, sig);
  exit(0);
}

static void child_handler(int signo)
{
  int status;

  /* If our child died, we're done. */
  if (waitpid(child_pid, &status, WNOHANG) == child_pid)
    exit(WIFEXITED(status) ? WEXITSTATUS(status) : 1);
}

static int find_signal(const char *signame)
{
  struct {
    char *name;
    int sig;
  } signames[] = {
    { "ABRT",	SIGABRT },
    { "ALRM",	SIGALRM },
    { "FPE",	SIGFPE },
    { "HUP",	SIGHUP },
    { "ILL",	SIGILL },
    { "INT",	SIGINT },
    { "KILL",	SIGKILL },
    { "PIPE",	SIGPIPE },
    { "QUIT",	SIGQUIT },
    { "SEGV",	SIGSEGV },
    { "TERM",	SIGTERM },
    { "USR1",	SIGUSR1 },
    { "USR2",	SIGUSR2 },
    { "CHLD",	SIGCHLD },
    { "CONT",	SIGCONT },
    { "STOP",	SIGSTOP },
    { "TSTP",	SIGTSTP },
    { "TTIN",	SIGTTIN },
    { "TTOU",	SIGTTOU },
    { "BUS",	SIGBUS }
  };
  int i;

  /* Allow numbers. */
  if (isdigit(*signame))
    return atoi(signame);

  /* Allow leading "SIG". */
  if (strncasecmp(signame, "SIG", 3) == 0)
    signame += 3;

  /* Now find signal name. */
  for (i = 0; i < sizeof(signames) / sizeof(*signames); i++)
    {
      if (strcasecmp(signames[i].name, signame) == 0)
	return signames[i].sig;
    }

  fprintf(stderr, "%s: invalid signal name %s\n", progname, signame);
  exit(1);
}

static void usage(void)
{
  fprintf(stderr, "Usage: %s [-s signame] program args...\n", progname);
  exit(1);
}
