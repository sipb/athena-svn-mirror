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

static const char rcsid[] = "$Id: access_off.c,v 1.11 1998-11-06 18:52:16 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PATH_INETD_PID		"/var/athena/inetd.pid"
#define PATH_SSHD_PID		"/var/athena/sshd.pid"
#define PATH_SSHD		"/etc/athena/sshd"
#define PATH_RC_CONF		"/etc/athena/rc.conf"
#define PATH_BOOT_ENVIRON	"/var/athena/env.boot"
#define ENV_FALLBACK		{ "PATH=/etc/athena", NULL }

static char **get_boot_environment(void);

int main(int argc, char **argv)
{
  FILE *pidfile, *rcfile;
  int pid, on, running;
  char *progname, buf[1024], **env, *env_fallback[] = ENV_FALLBACK;

  progname = strrchr(argv[0], '/');
  progname = (progname) ? progname + 1 : argv[0];

  /* We're turning access off if this program was invoked as access_off;
   * otherwise we're turning access on. */
  on = (strcmp(progname, "access_off") != 0);

  /* Send SIGUSR1 or SIGUSR2 to the Athena inetd. */
  pidfile = fopen(PATH_INETD_PID, "r");
  if (!pidfile)
    {
      fprintf(stderr, "Cannot read %s.  Daemon probably not running\n",
	      PATH_INETD_PID);
      return 1;
    }
  if (fscanf(pidfile, "%d", &pid) != 1)
    {
      fprintf(stderr, "Error reading %s.\n", PATH_INETD_PID);
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
  rcfile = fopen(PATH_RC_CONF, "r");
  if (!rcfile)
    {
      fprintf(stderr, "Cannot read %s: %s", PATH_RC_CONF, strerror(errno));
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
      pidfile = fopen(PATH_SSHD_PID, "r");
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
	      fclose(rcfile);
	      /* Make ruid and saved uid equal to effective uid before exec,
	       * or sshd might notice and get confused.  Set the environment
	       * to the boot-time environment.
	       */
	      setuid(geteuid());
	      env = get_boot_environment();
	      if (!env)
		env = env_fallback;
	      execle(PATH_SSHD, PATH_SSHD, (char *) NULL, env);
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

static char **get_boot_environment(void)
{
  int fd, len, count;
  struct stat st;
  char *buf, *p, *q, **env, **ep;

  /* Open and read the stored boot environment. */
  fd = open(PATH_BOOT_ENVIRON, O_RDONLY);
  if (fd == -1)
    return NULL;
  if (fstat(fd, &st) == -1)
    {
      close(fd);
      return NULL;
    }
  buf = malloc(st.st_size);
  if (!buf)
    {
      close(fd);
      return NULL;
    }
  len = 0;
  while (len < st.st_size)
    {
      count = read(fd, buf + len, st.st_size - len);
      if (count < 0 && errno != EINTR)
	{
	  close(fd);
	  free(buf);
	  return NULL;
	}
      else if (count > 0)
	len += count;
    }
  close(fd);

  /* Allocate space for the environment pointers. */
  count = 0;
  for (p = buf; *p; p++)
    {
      if (*p == '\n')
	count++;
    }
  env = malloc((count + 1) * sizeof(char *));
  if (!env)
    {
      free(buf);
      return NULL;
    }

  /* Set the environment pointers. */
  ep = env;
  p = buf;
  while ((q = strchr(p, '\n')) != NULL)
    {
      *ep++ = p;
      *q = 0;
      p = q + 1;
    }

  /* Terminate the environment list and return it. */
  *ep = NULL;
  return env;
}
