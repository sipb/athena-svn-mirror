/* Copyright 1996 by the Massachusetts Institute of Technology.
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

/* rep -- run a program repeatedly using curses.
 *
 *	Usage: rep [-n] command
 *
 *	Permits the user to watch a program like 'w' or 'finger' change with
 *	time.  Given an argument '-x' will cause a repetion every x seconds.
 *
 * ORIGINAL
 * Ray Lubinsky University of Virginia, Dept. of Computer Science
 *	     uucp: decvax!mcnc!ncsu!uvacs!rwl
 * MODIFIED 3/85
 * Bruce Karsh, Univ. Wisconsin, Madison.  Dept. of Geophysics.
 *           uucp: uwvax\!geowhiz\!karsh
 */

static const char rcsid[] = "$Id: rep.c,v 1.4 1997-01-11 19:13:41 ghudson Exp $";

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <curses.h>

static void interrupt(int signo);
static void usage(const char *progname);
static FILE *run_program(char **args, const char *progname, pid_t *pid_return);
static void end_program(FILE *stream, pid_t pid, const char *progname);

/* Set by interupt trap routine */
static int interrupted = 0;

int main(int argc, char **argv)
{
    FILE *input;
    char *progname, **args, buf[BUFSIZ], interval = 1;
    int	line;
    struct sigaction sa;
    pid_t pid;

    progname = argv[0];
    if (strrchr(progname, '/'))
      progname = strrchr(progname, '/') + 1;

    if (argc < 2)
	usage(progname);

    if (*argv[1] == '-')
      {
	interval = atoi(argv[1] + 1);
	if (argc < 3 || interval == 0)
	  usage(progname);
	args = argv + 2;
      }
    else
      args = argv + 1;

    /* Set up signal handler for SIGINT. */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = interrupt;
    sigaction(SIGINT, &sa, NULL);

    /* Initialize curses window. */
    initscr();
    crmode();
    nonl();
    clear();

    while (1)
      {
	if (interrupted)
	  break;

	/* Start up an instance of the program. */
	input = run_program(args, progname, &pid);
	if (!input)
	  break;

	line = 0;
	while (fgets(buf, sizeof(buf), input) != NULL)
	  {
	    mvaddstr(line, 0, buf);
	    clrtoeol();
	    line++;
	    if (line >= LINES || interrupted)
	      break;
	  }
	if (interrupted)
	  break;
	end_program(input, pid, progname);
	input = NULL;
	if (interrupted)
	  break;
	clrtobot();
	refresh();
	sleep(interval);
      }

    /* Clean up and exit. */
    if (input)
	end_program(input, pid, progname);
    clear();
    refresh();
    endwin();
    exit(0);
}

static void interrupt(int signo)
{
    interrupted = 1;
}

static void usage(const char *progname)
{
    fprintf(stderr,"Usage: %s [-<interval>] command\n", progname);
    exit(1);
}

static FILE *run_program(char **args, const char *progname, pid_t *pid_return)
{
  pid_t pid;
  int fds[2];
  FILE *stream;

  /* Create a pipe for the program output. */
  if (pipe(fds) < 0)
    {
      fprintf(stderr, "%s: pipe: %s\n", progname, strerror(errno));
      exit(1);
    }

  /* Fork a child process. */
  pid = fork();
  if (pid < 0)
    {
      fprintf(stderr, "%s: fork: %s\n", progname, strerror(errno));
      exit(1);
    }

  /* In the child, set up file descriptors and exec. */
  if (pid == 0)
    {
      if (dup2(fds[1], STDOUT_FILENO) < 0)
	{
	  fprintf(stderr, "%s: dup2: %s\n", progname, strerror(errno));
	  exit(1);
	}
      if (dup2(fds[1], STDERR_FILENO) < 0)
	{
	  fprintf(stderr, "%s: dup2: %s\n", progname, strerror(errno));
	  exit(1);
	}
      close(fds[0]);
      execvp(*args, args);
      fprintf(stderr, "%s: can't exec %s: %s\n", progname, *args,
	      strerror(errno));
      exit(1);
    }

  /* In the parent, close the writing side of the pipe and create a stream. */
  close(fds[1]);
  stream = fdopen(fds[0], "r");
  if (!stream)
    {
      close(fds[0]);
      fprintf(stderr, "%s: Can't fdopen: %s\n", progname, strerror(errno));
      exit(1);
    }

  *pid_return = pid;
  return stream;
}

static void end_program(FILE *stream, pid_t pid, const char *progname)
{
  pid_t result;

  while (1)
    {
      result = waitpid(pid, NULL, 0);
      if (result == -1 && errno != EINTR)
	{
	  fprintf(stderr, "%s: waitpid() failed: %s\n", progname,
		  strerror(errno));
	  exit(1);
	}
      if (result == pid)
	break;
    }
  fclose(stream);
}
