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

static const char rcsid[] = "$Id: access_off.c,v 1.6 1997-04-11 20:48:49 ghudson Exp $";

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
  FILE *pidfile;
  int pid, status, signo;
  char *progname;

  pidfile = fopen("/var/athena/inetd.pid", "r");
  if (!pidfile)
    {
      printf("cannot read process id file--daemon probably not running\n");
      return 1;
    }
  if (fscanf(pidfile, "%d", &pid) != 1)
    {
      printf("error reading process id file\n");
      return 1;
    }
  fclose(pidfile);

  progname = strrchr(argv[0], '/');
  progname = (progname) ? progname + 1 : argv[0];
  signo = (strcmp(progname, "access_off") == 0) ? SIGUSR2 : SIGUSR1;

  if (kill(pid, signo) < 0)
    {
      if (errno != ESRCH)
	  perror("error killing daemon");
      return 1;
    }

  return 0;
}
