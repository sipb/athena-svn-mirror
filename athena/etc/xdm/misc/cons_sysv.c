#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

int cons_status(cons_state *c)
{
  if (c == NULL)
    return CONS_DOWN;

  return c->state;
}

int cons_fd(cons_state *c)
{
  if (c == NULL || c->gotpty == 0)
    return -1;

  return c->pty_fd;
}

int cons_fd_tty(cons_state *c)
{
  if (c == NULL || c->gotpty == 0)
    return -1;

  return c->tty_fd;
}

char *cons_name(cons_state *c)
{
  if (c == NULL || c->gotpty == 0)
    return NULL;

  return c->ttydev;
}

void cons_io(cons_state *c)
{
  static char buf[512];
  int len;

  if (c == NULL || c->gotpty == 0)
    return;

  len = read(c->pty_fd, buf, sizeof(buf));
  write(c->fd[1], buf, len);
}

int cons_getpty(cons_state *c)
{
  char *ttyname;

  if (c == NULL || c->gotpty == 1)
    return 1;

#ifdef notdef
  /* This code seems to buy you a "/dev/pts/#" device. I'd like our
     pty to look like xterm's, so I stole the block outside of this
     ifdef. Other code in this routine was adapted from xconsole. */
  if ((c->pty_fd = open ("/dev/ptmx", O_RDWR)) < 0)
    return 1;
  strcpy(c->ttydev, ptsname(c->pty_fd));
#endif

  ttyname = _getpty(&(c->pty_fd), O_RDWR, 0622, 0);
  if (ttyname == NULL)
    return 1;
  strcpy(c->ttydev, ttyname);

  grantpt(c->pty_fd);
  unlockpt(c->pty_fd);
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

  c->state = CONS_UP;

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
  c->state = state;
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
  struct sigaction sigact, osigact;
  pid_t ret;

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

  sigact.sa_flags = 0;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_handler = cons_alarm;
  sigaction(SIGALRM, &sigact, &osigact);

  alarm(5);
  while ((ret = waitpid(c->pid, &status, 0)) == -1)
    {
      if (errno == EINTR && cons_ding)
	{
	  kill(c->pid, SIGKILL);
	  cons_cleanup(c, CONS_DOWN);
	  break;
	}
      /* ouch! */
    }

  if (ret == c->pid)
    {
      if ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
	  (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL))
	cons_cleanup(c, CONS_DOWN);
      else
	cons_cleanup(c, CONS_FROZEN);
    }

  alarm(0);
  cons_ding = 0;
  sigaction(SIGALRM, &osigact, NULL);

  return 0;
}

int cons_child(cons_state *c, pid_t pid, void *s)
{
  int status;

  status = *(int *)s;

  if (c == NULL || c->pid != pid)
    return 1;

  c->exitStatus = status;

  if ((WIFEXITED(status) && WEXITSTATUS(status) == 0) ||
      (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL))
    cons_cleanup(c, CONS_DOWN);
  else
    cons_cleanup(c, CONS_FROZEN);

  return 0;
}

void *cons_exitStatus(cons_state *c)
{
  if (c == NULL)
    return NULL;

  return (void *)&(c->exitStatus);
}

void cons_close(cons_state *c)
{
  if (c == NULL)
    return;

  if (c->state == CONS_UP)
    cons_stop(c);

  close(c->fd[0]);
  close(c->fd[1]);

  if (c->gotpty)
    {
      close(c->tty_fd);
      close(c->pty_fd);
    }

  free(c);
}
