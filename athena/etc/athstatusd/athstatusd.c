/* Copyright 200 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: athstatusd.c,v 1.1.2.1 2002-12-09 23:30:40 ghudson Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

static volatile int run = 1;
static volatile int new_event = 0;

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 100
#endif

/* Update the highest set fd in an fd_set, given an upper bound. */
static int find_highest(fd_set *fds, int highest)
{
  while (highest > 0)
    {
      if (FD_ISSET(highest, fds))
	return highest;
      highest--;
    }
  return highest;
}

static void send_events(fd_set *fds, int highest, int listener)
{
  char buf[BUFSIZ];
  int i;

  for (i = 0; i <= highest; i++)
    {
      if (i == listener)
	continue;

      if (FD_ISSET(i, fds))
	{
	  if (write(i, "\n", 1) < 0)
	    perror("write");
	  else
	    {
	      fd_set rfd;
	      struct timeval tv;

	      /* Give the client two seconds to copy the status file and
	       * respond.
	       */
	      FD_ZERO(&rfd);
	      FD_SET(i, &rfd);
	      tv.tv_sec = 2;
	      tv.tv_usec = 0;
	      if (select(i + 1, &rfd, NULL, NULL, &tv) > 0)
		{
		  if (read(i, buf, sizeof(buf)) < 0)
		    perror("read");
		}
	    }
	}
    }
  new_event = 0;
  unlink(LOCK_PATH);
}

static void wait_for_events (int listener)
{
  char buf[BUFSIZ];
  fd_set readfds;
  fd_set rfd;
  int highest = listener;
  int i, s;

  FD_ZERO(&readfds);
  FD_SET(listener, &readfds);

  while (run)
    {
      rfd = readfds;

      if (new_event)
	send_events(&readfds, highest, listener);

      if (select(highest + 1, &rfd, NULL, NULL, NULL) <= 0)
	continue;

      /* Find out which fds have data. */
      for (i = 0; i <= highest; i++)
	{
	  if (!FD_ISSET(i, &rfd))
	    continue;
	  if (i == listener)
	    {
	      /* We have a new client. */
	      s = accept(i, NULL, 0);
	      if (s < 0)
		perror("accept");
	      else
		{
		  FD_SET(s, &readfds);
		  if (s > highest)
		    highest = s;
		}
	    }
	  else
	    {
	      /* An old client went away. */
	      s = read(i, buf, sizeof(buf));
	      if (s < 0)
		perror("read");
	      else if (s == 0)
		{
		  close(i);
		  FD_CLR(i, &readfds);
		  highest = find_highest(&readfds, highest);
		}
	    }
	}
    }
}

static void term_handler(int arg)
{
  run = 0;
}

static void event_handler(int arg)
{
  new_event = 1;
}

static void setup_signals(void)
{
  struct sigaction sa;

  memset(&sa, 0, sizeof (sa));
  sa.sa_handler = event_handler;
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGUSR1, &sa, NULL) < 0)
    {
      perror("sigaction SIGUSR1");
      exit(1);
    }

  memset(&sa, 0, sizeof (sa));
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGPIPE, &sa, NULL) < 0)
    {
      perror("sigaction SIGPIPE");
      exit(1);
    }

  memset(&sa, 0, sizeof (sa));
  sa.sa_handler = term_handler;
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGTERM, &sa, NULL) < 0)
    {
      perror("sigaction SIGTERM");
      exit(1);
    }

  if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
      perror("sigaction SIGHUP");
      exit(1);
    }
}

static void save_pid(char *filename)
{
  pid_t pid;
  FILE *fp;

  pid = getpid();

  fp = fopen(filename, "w");
  if (!fp)
    {
      perror("fopen");
      exit(1);
    }
  fprintf(fp, "%d\n", pid);
  fclose(fp);
}

static int open_listener (const char *filename)
{
  int sock;
  struct sockaddr_un sun;

  sock = socket(PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror("socket");
      exit(1);
    }

  memset(&sun, 0, sizeof (sun));
  sun.sun_family = AF_UNIX;
  strncpy(sun.sun_path, filename, UNIX_PATH_MAX);

  if (bind(sock, (struct sockaddr *)&sun, sizeof (sun)) < 0)
    {
      perror("bind");
      unlink(filename);
      exit(1);
    }

  /* Make sure everyone can connect. */
  if (chmod(filename, 0777) < 0)
    {
      perror("chmod");
      close(sock);
      unlink(filename);
      exit(1);
    }

  listen(sock, 7);
  return sock;
}

int main (int argc, char *argv[])
{
  int listener;

  setup_signals();
  listener = open_listener(SOCK_PATH);

  /* Turn ourselves into a daemon, but do not redirect stdout/stderr
   * to /dev/null.
   */
  if (daemon(0, 1) != 0)
    {
      perror("daemon");
      exit(1);
    }

  /* Save our PID. */
  save_pid(PID_PATH);

  /* Clean up any stale lock file. */
  unlink(LOCK_PATH);

  /* Wait for clients. */
  wait_for_events(listener);

  /* Work is done.  Clean up our socket for next time. */
  close(listener);
  unlink(SOCK_PATH);
  unlink(PID_PATH);

  return 0;
}
