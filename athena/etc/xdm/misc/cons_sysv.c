#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <termios.h>
#include <sys/stropts.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include "cons_sysv.h"

#define DAEMON 1

char *conspid = "/etc/athena/console.pid";
char *conslog = "/usr/tmp/console.log";

cons_state *cons_init(void)
{
  cons_state *c;
  int file;

  c = malloc(sizeof(cons_state));
  if (c)
    {
      c->gotpty = 0;
      c->state = CONS_DOWN;

      if (pipe(c->fd))
	{
	  free(c);
	  return NULL;
	}

      /* Create console log file owned by daemon */
      if (access(conslog, F_OK) != 0) {
	file = open(conslog, O_CREAT, 0644);
	close(file);
      }
      chown(conslog, DAEMON, 0);
    }

  return c;
}

int cons_getpty(cons_state *c)
{
  if (c == NULL || c->gotpty == 1)
    return 1;

  if ((c->pty_fd = open ("/dev/ptmx", O_RDWR)) < 0)
    return 1;

  grantpt(c->pty_fd);
  unlockpt(c->pty_fd);
  strcpy(c->ttydev, ptsname(c->pty_fd));
  if ((c->tty_fd = open(c->ttydev, O_RDWR)) >= 0)
    {
      (void)ioctl(c->tty_fd, I_PUSH, "ttcompat");
      c->gotpty = 1;
      return 0;
    }
  if (c->pty_fd >= 0)
    close (c->pty_fd);

  /* We were unable to allocate a pty master!  Return an error
   * condition and let our caller terminate cleanly.
   */
  c->gotpty = 0;
  return(1);
}

int cons_grabcons(cons_state *c)
{
  int on = 1;

  if (c == NULL || c->gotpty == 0)
    return 1;

  if (ioctl(c->tty_fd, TIOCCONS, (char *) &on) == -1)
    return 1;

  return 0;
}

int cons_start(cons_state *c)
{
  FILE *pidfile;

  if (c == NULL || c->gotpty == 0)
    return 1;

  if (c->state == CONS_UP)
    return 0;

  c->pid = fork();
  if (c->pid == -1)
    {
      c->state = CONS_FROZEN;
      return 1;
    }

  if (c->pid == 0)			/* child */
    {
      close(0);
      close(c->fd[1]);
      dup2(c->fd[0], 0);
      setgid(DAEMON);
      setuid(DAEMON);
      execl("/etc/athena/console", "console", "-nosession", 0);
      exit(1);
    }

  pidfile = fopen(conspid, "w");
  if (pidfile == NULL)
    {
      return 0;
    }
  fprintf(pidfile, "%d\n", c->pid);
  fclose(pidfile);

  return 0;
}

static int cons_ding = 0;

static void cons_alarm(int sig, int code, struct sigcontext *sc)
{
  cons_ding = 1;
}

static void cons_cleanup(cons_state *c, int state)
{
  c->state = CONS_DOWN;
  unlink(conspid);
}

/*
 * Shut down the console cleanly. If it doesn't go down cleanly,
 * make sure it dies. The idea is to make sure the console is down
 * before this routine returns, but definitely return someday.
 */
int cons_stop(cons_state *c)
{
  int status;
  void *handler;

  if (c == NULL)
    return 1;

  if (c->state != CONS_UP)
    return 0;

  if (kill(c->pid, SIGHUP))
    {
      /* This should only be ESRCH; assuming console already down */
      cons_cleanup(c, CONS_DOWN);
      return 0;
    }

  alarm(0);
  handler = sigset(SIGALRM, cons_alarm);
  alarm(5);
  if (waitpid(c->pid, &status, 0) == -1)
    {
      if (errno == EINTR && cons_ding)
	{
	  kill(c->pid, SIGKILL);
	  cons_cleanup(c, CONS_DOWN);
	}
      /* ouch! */
    }
  else
    {
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
	cons_cleanup(c, CONS_DOWN);
      else
	cons_cleanup(c, CONS_FROZEN);
    }

  alarm(0);
  cons_ding = 0;
  sigset(SIGALRM, handler);

  return 0;
}

int cons_child(cons_state *c, void *s, pid_t pid)
{
  int status;

  status = *(int *)s;

  if (c == NULL || c->pid != pid)
    return 1;

  if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    cons_cleanup(c, CONS_DOWN);
  else
    cons_cleanup(c, CONS_FROZEN);

  return 0;
}
