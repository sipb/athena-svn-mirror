/* Copyright 1996, 1999 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: access_off.c,v 1.13 1999-02-27 17:10:47 ghudson Exp $";

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#define PATH_INETD_PID		"/var/athena/inetd.pid"
#define PATH_SSHD_PID		"/var/athena/sshd.pid"

static int sendsig(const char *pidpath, int on);

int main(int argc, char **argv)
{
  int on, err;
  char *progname;

  /* We're turning access off if this program was invoked as access_off;
   * otherwise we're turning access on. */
  progname = strrchr(argv[0], '/');
  progname = (progname) ? progname + 1 : argv[0];
  on = (strcmp(progname, "access_off") != 0);

  err = 0;
  if (sendsig(PATH_INETD_PID, on) == -1)
    err = 1;
  if (sendsig(PATH_SSHD_PID, on) == -1)
    err = 1;
  return err;
}

static int sendsig(const char *pidpath, int on)
{
  FILE *pidfile;
  int pid;

  /* Send SIGUSR1 or SIGUSR2 to the pid contained in the requested file. */
  pidfile = fopen(pidpath, "r");
  if (!pidfile)
    {
      fprintf(stderr, "Cannot read %s.  Daemon probably not running\n",
	      pidpath);
      return -1;
    }
  if (fscanf(pidfile, "%d", &pid) != 1)
    {
      fclose(pidfile);
      fprintf(stderr, "Error reading %s.\n", pidpath);
      return -1;
    }
  fclose(pidfile);
  if (kill(pid, (on) ? SIGUSR1 : SIGUSR2) < 0)
    {
      if (errno != ESRCH)
	perror("Error killing daemon");
      return -1;
    }
  return 0;
}
