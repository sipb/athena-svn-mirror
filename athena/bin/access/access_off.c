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

static const char rcsid[] = "$Id: access_off.c,v 1.8 1998-04-16 22:15:51 ghudson Exp $";

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  FILE *pidfile, *rcfile;
  int pid, status, on, running;
  char *progname, buf[1024];

  progname = strrchr(argv[0], '/');
  progname = (progname) ? progname + 1 : argv[0];

  /* We're turning access off if this program was invoked as access_off;
   * otherwise we're turning access on. */
  on = (strcmp(progname, "access_off") != 0);

  /* Send SIGUSR1 or SIGUSR2 to the Athena inetd. */
  pidfile = fopen("/var/athena/inetd.pid", "r");
  if (!pidfile)
    {
      fprintf(stderr, "Cannot read /var/athena/inetd.pid."
	      "  Daemon probably not running\n");
      return 1;
    }
  if (fscanf(pidfile, "%d", &pid) != 1)
    {
      fprintf(stderr, "Error reading /var/athena/inetd.pid.\n");
      return 1;
    }
  fclose(pidfile);
  if (kill(pid, (on) ? SIGUSR1 : SIGUSR2) < 0)
    {
      if (errno != ESRCH)
	perror("Error killing daemon");
      return 1;
    }

  /* If SSHD is set to "switched" in /etc/athena/rc.conf, then start or stop
   * sshd if it's not already in the state we want. */
  rcfile = fopen("/etc/athena/rc.conf", "r");
  if (!rcfile)
    {
      fprintf(stderr, "Cannot read /etc/athena/rc.conf: %s", strerror(errno));
      return 1;
    }
  while (fgets(buf, sizeof(buf), rcfile) != NULL)
    {
      /* Ignore lines which aren't setting SSHD. */
      if (strncmp(buf, "SSHD=", 5) != 0)
	continue;

      /* If SSHD is not set to "switched" then we do nothing. */
      if (strncmp(buf + 5, "switched", 8) != 0)
	break;

      /* Read the sshd pidfile and send a signal 0 to the contained pid
       * to determine if sshd is running. */
      pidfile = fopen("/var/athena/sshd.pid", "r");
      if (pidfile && fscanf(pidfile, "%d", &pid) == 1 && kill(pid, 0) == 0)
	running = 1;
      else
	running = 0;
      if (pidfile)
	fclose(pidfile);

      if (on && !running)
	{
	  /* ssh is not running and we'd like it to be running.  Start it. */
	  pid = fork();
	  if (pid == -1)
	    {
	      perror("Error forking to run sshd");
	      return 1;
	    }
	  else if (pid == 0)
	    {
	      /* Make ruid and saved uid equal to effective uid before exec,
	       * or sshd might notice and get confused. */
	      setuid(geteuid());
	      execl("/etc/athena/sshd", "sshd", (char *) NULL);
	      perror("Error running sshd");
	    }
	}
      else if (!on && running)
	{
	  /* ssh is running and we'd like it not to be running.  Kill it.
	   * We can be sure that we read its pid in this case. */
	  kill(pid, SIGTERM);
	}

      break;
    }
  fclose(rcfile);

  return 0;
}
