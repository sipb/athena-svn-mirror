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

static const char rcsid[] = "$Id: dustbuster.c,v 1.9 2003-01-22 21:43:28 rbasch Exp $";

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
static void sessionbust(char **argv);
static void start_child(char **argv, int keep_fd);
static int xhandler(Display *dpy);
static void child_handler(int signo);
static void sig_handler(int signo);
static void kill_and_exit(int status);
static int find_signal(const char *signame);
static int tty_accessible(void);
static void usage(void);

static const char *progname;
static pid_t child_pid = 0;
static int sig = SIGHUP;
static int session_leader = 0;

int main(int argc, char **argv)
{
  const char *signame;
  struct sigaction sa;

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
	  break;

	case 'S':
	  session_leader = 1;
	  break;

	default:
	  usage();
	  break;
	}
      argv++;
    }
  if (*argv == NULL)
    usage();

  /* Set up a handler for common terminating signals to make sure
   * we kill the child before dying.  In particular, this is needed
   * to handle a SIGHUP at logout when we are invoked by a foreground
   * process, as our child will always be in a separate process group.
   */
  sa.sa_handler = sig_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  /* We conditionalize on XSESSION (set by the Athena default xsession
   * script) instead of DISPLAY, so that we won't hold open the
   * display when someone does a tty login via ssh with X forwarding.
   */
  if (getenv("XSESSION") != NULL)
    xbust(argv);
  else if (tty_accessible())
    ttybust(argv);
  else if (getenv("ATHENA_LOGIN_SESSION") != NULL)
    sessionbust(argv);
  else
    {
      fprintf(stderr, "%s: error: can't find any way to dustbust\n",
	      progname, strerror(errno));
      exit(1);
    }
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
  start_child(argv, ConnectionNumber(dpy));

  /* Wait until either the X connection dies (xhandler) or the child
   * process dies (child_handler).
   */
  while (1)
    XNextEvent(dpy, &event);
}

static void ttybust(char **argv)
{
  int fd;

  /* Start the child process. */
  start_child(argv, -1);

  /* Wait until the child process exits (child_handler) or the
   * controlling tty can no longer be opened.
   */
  while (1)
    {
      fd = open("/dev/tty", O_RDWR, 0);
      if (fd == -1)
	kill_and_exit(0);
      close(fd);
      sleep(5);
    }
}

static void sessionbust(char **argv)
{
  pid_t pid;

  pid = atoi(getenv("ATHENA_LOGIN_SESSION"));
  if (pid <= 1)
    {
      fprintf(stderr, "%s: error: invalid ATHENA_LOGIN_SESSION value %s\n",
	      progname, getenv("ATHENA_LOGIN_SESSION"));
      exit(1);
    }
  if (kill(pid, 0) == -1)
    {
      fprintf(stderr, "%s: error: login session pid %lu not running\n",
	      progname, (unsigned long) pid);
      exit(1);
    }

  start_child(argv, -1);

  /* Wait until the child process exits (child_handler) or the
   * login session pid is no longer running.
   */
  while (1)
    {
      if (kill(pid, 0) == -1)
	kill_and_exit(0);
      sleep(5);
    }
}

/* Set up the SIGCHLD handler and start the child process given by
 * argv.  Sets child_pid.  The parent closes open file descriptors,
 * except that given by keep_fd, to prevent blocking by any parent
 * reading a pipe, and redirects the standard input, output, and error
 * files to /dev/null.
 */
static void start_child(char **argv, int keep_fd)
{
  struct sigaction sa;
  sigset_t set, oset;
  int fd;

  /* Set up the SIGCHLD handler. */
  sa.sa_handler = child_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NOCLDSTOP;
  sigaction(SIGCHLD, &sa, NULL);

  /* Create the subprocess, blocking SIGCHLD and terminating signals
   * until child_pid is assigned.
   */
  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  sigaddset(&set, SIGHUP);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
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
      /* Make session leader if requested, or process group leader if not. */
      if (session_leader)
	setsid();
      else
	setpgid(getpid(), getpid());

      execvp(argv[0], argv);
      fprintf(stderr, "%s: error: can't execute %s: %s\n", progname, argv[0],
	      strerror(errno));
      _exit(1);
    }

  /* In the parent, close all open files, except the descriptor the
   * caller passed in keep_fd, and redirect stdin, stdout, and stderr
   * to /dev/null.
   */
  for (fd = sysconf(_SC_OPEN_MAX); --fd > STDERR_FILENO; )
    {
      if (fd != keep_fd)
	close(fd);
    }
  fd = open("/dev/null", O_RDWR, 0);
  if (fd != -1)
    {
      if (keep_fd != STDIN_FILENO)
	dup2(fd, STDIN_FILENO);
      if (keep_fd != STDOUT_FILENO)
	dup2(fd, STDOUT_FILENO);
      if (keep_fd != STDERR_FILENO)
	dup2(fd, STDERR_FILENO);
      if (fd > STDERR_FILENO)
	close(fd);
    }
}

/* Xlib calls this when a fatal I/O error happens (such as the display
 * closing).  Send a signal to the child's process group and exit.
 */
static int xhandler(Display *dpy)
{
  kill_and_exit(0);
}

static void child_handler(int signo)
{
  int status;

  /* If our child died, we're done. */
  if (waitpid(child_pid, &status, WNOHANG) == child_pid)
    exit(WIFEXITED(status) ? WEXITSTATUS(status) : 1);
}

/* Here when we receive a terminating signal.  Send a signal to the
 * child's process group and exit.
 */
static void sig_handler(int signo)
{
  kill_and_exit(0);
}

/* Here to kill the child's process group and exit.  We use kill() as
 * well as killpg() in case the child has not yet set its process group.
 */
static void kill_and_exit(int status)
{
  sigset_t set, oset;

  if (child_pid > 0)
    {
      /* Block SIGCHLD until we have signaled both the process and the
       * process group.
       */
      sigemptyset(&set);
      sigaddset(&set, SIGCHLD);
      sigprocmask(SIG_BLOCK, &set, &oset);
      kill(child_pid, sig);
      killpg(child_pid, sig);
      sigprocmask(SIG_SETMASK, &oset, NULL);
    }
  exit(status);
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

static int tty_accessible(void)
{
  int fd;

  fd = open("/dev/tty", O_RDWR, 0);
  if (fd == -1)
    return 0;
  close(fd);
  return 1;
}

static void usage(void)
{
  fprintf(stderr, "Usage: %s [-S] [-s signame] program args...\n", progname);
  exit(1);
}
