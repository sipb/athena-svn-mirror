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

static const char rcsid[] = "$Id: dustbuster.c,v 1.1 2001-04-30 15:25:44 ghudson Exp $";

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

static pid_t child_pid = 0;

int main(int argc, char **argv)
{
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
      fprintf(stderr, "%s: error: can't open display\n", argv[0]);
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
	      argv[0], strerror(errno));
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
	  kill(child_pid, SIGHUP);
	  exit(0);
	}
      close(fd);
      sleep(5);
    }
}

/* Set up the SIGCHLD handler and start the child process.  argv is
 * the original dustbuster argument vector.  Sets child_pid.
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
      fprintf(stderr, "%s: error: can't create subprocess: %s\n", argv[0],
	      strerror(errno));
      exit(1);
    }

  /* In the child, run the specified program. */
  if (child_pid == 0)
    {
      execvp(argv[1], argv + 1);
      fprintf(stderr, "%s: error: can't execute %s: %s\n", argv[0], argv[1],
	      strerror(errno));
      _exit(1);
    }
}

/* Xlib calls this when a fatal I/O error happens (such as the display
 * closing.  Send a SIGHUP to the child pid and exit.
 */
static int xhandler(Display *dpy)
{
  kill(child_pid, SIGHUP);
  exit(0);
}

static void child_handler(int signo)
{
  int status;

  /* If our child died, we're done. */
  if (waitpid(child_pid, &status, WNOHANG) == child_pid)
    exit(WIFEXITED(status) ? WEXITSTATUS(status) : 1);
}
