/* Copyright 2002 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: athneteventd.c,v 1.2 2002-11-06 20:01:28 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>		/* for system() */

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 100
#endif

static void copy_file(int fd, const char *source_file)
{
  char buf[BUFSIZ];
  size_t len;
  int src;

  /* Open the source file in read-only mode. */
  src = open(source_file, O_RDONLY);
  if (src < 0)
    {
      perror("open");
      close(fd);
      return;
    }

  /* Read the source file; for each chunk, try to write it out to fd. */
  while ((len = read(src, buf, sizeof(buf))) > 0)
    {
      char *p = buf;
      size_t lenwrite;

      while (len)
	{
	  lenwrite = write(fd, p, len);
	  if (lenwrite < 0)
	    {
	      perror("write");
	      break;
	    }
	  len -= lenwrite;
	  p += lenwrite;
	}
    }

  /* Close the source file. */
  close(src);
}

static void wait_for_events(int sock)
{
  char dummy, tmpfile[BUFSIZ];
  int i, status, fd;
  pid_t pid;

  /* Read event indications (which are just newlines) from statusd. */
  while ((i = read(sock, &dummy, 1)) > 0)
    {
      /* We got an event.  Create a tempfile. */
      snprintf(tmpfile, sizeof(tmpfile), "/tmp/status-event-XXXXXX");
      fd = mkstemp(tmpfile);
      if (fd < 0)
	{
	  perror("mkstemp");
	  return;
	}

      /* Copy the status file. */
      copy_file(fd, STATUS_PATH);
      close(fd);

      /* Signal the server. */
      if (write(sock, "\n", 1) < 0)
	perror("write");

      /* Fork a client to process the events */
      pid = fork();
      switch (pid)
	{
	case -1:
	  perror("fork");
	  return;
	case 0:
	  /* In the child, run the client script (which should remove the
	   * status file).
	   */
	  execl("/bin/sh", "-c", SCRIPT_PATH, tmpfile, NULL);
	  _exit(1);

	default:
	  /* In the parent, wait for the child. */
	  status = 0;
	  waitpid(pid, &status, 0);
	}
    }

  if (i < 0)
    perror("read");
  if (i == 0)
    fprintf(stderr, "Status daemon exited.\n");
}

static int daemon_connect(char *sock_name)
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
  strncpy(sun.sun_path, sock_name, UNIX_PATH_MAX);

  if (connect(sock, (struct sockaddr *) &sun, sizeof(sun)) < 0)
    {
      perror("connect");
      exit(2);
    }

  return sock;
}

int main(int argc, char *argv[])
{
  int sock;

  /* Connect to the status daemon. */
  sock = daemon_connect(SOCK_PATH);

  /* Turn ourselves into a daemon, but do not chdir to / or redirect
   * stdout/stderr to /dev/null.
   */
  if (daemon(1, 1) != 0)
    {
      perror("daemon");
      exit(1);
    }

  /* Process events sent to us by the status daemon. */
  wait_for_events(sock);

  close(sock);
  return 0;
}
