/* Copyright 1990, 1999 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: timeout.c,v 1.2 1999-12-22 17:27:48 danw Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

int app_pid;
volatile int app_running;
int app_exit_status;

void child(int);
void wakeup(int);

int main(int argc, char **argv)
{
  int maxidle, ttl;
  char *name, msg[512];
  struct stat stbuf;
  struct timeval now, start;
  struct sigaction sigact;

  name = *argv++;
  if (argc < 3)
    {
      fprintf(stderr, "usage: %s max-idle-time application command line...\n",
	      name);
      exit(1);
    }
  maxidle = atoi(*argv++);
  argc -= 2;

  app_running = TRUE;

  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = 0;
  sigact.sa_handler = child;
  sigaction(SIGCHLD, &sigact, NULL);
  sigact.sa_handler = wakeup;
  sigaction(SIGALRM, &sigact, NULL);

  /* Launch application. */
  app_pid = fork();
  switch (app_pid)
    {
    case 0:
      execvp(argv[0], argv);
      sprintf(msg, "%s: failed to exec application %s\n", name, argv[0]);
      perror(msg);
      exit(1);
    case -1:
      sprintf(msg, "%s: failed to fork to create application\n", name);
      perror(msg);
      exit(1);
    default:
      break;
    }

  gettimeofday(&start, NULL);
  ttl = maxidle;

  /* Wait for application to exit or idle-time to be reached. */
  while (app_running)
    {
      alarm(ttl);
      sigpause(0);
      if (!app_running)
	break;

      fstat(1, &stbuf);
      gettimeofday(&now, NULL);
      ttl = start.tv_sec + maxidle - now.tv_sec;
      /* Only check idle time if we've been running at least that long. */
      if (ttl <= 0)
	ttl = stbuf.st_atime + maxidle - now.tv_sec;
      if (ttl <= 0)
	{
	  fprintf(stderr, "\nMAX IDLE TIME REACHED.\n");
	  kill(app_pid, SIGINT);
	  exit(0);
	}
    }
  exit(app_exit_status);
}

void child(int sig)
{
  int status;
  int pid;

  pid = waitpid(-1, &status, WNOHANG);
  if (pid != app_pid || WIFSTOPPED(status))
    return;
  app_running = FALSE;
  if (WIFEXITED(status))
    app_exit_status =  WEXITSTATUS(status);
  else
    app_exit_status = WTERMSIG(status);
}

void wakeup(int sig)
{
}
