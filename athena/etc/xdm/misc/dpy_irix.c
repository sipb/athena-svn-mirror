#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <termios.h>
#include <syslog.h>
#include <errno.h>
#include "dpy_irix.h"

#define CHKCONFIG "/etc/chkconfig "
#define WS "windowsystem "
#define KILLALL "/etc/killall "

#define CONSLINE "tport"
#define XDM "axdm"
#define XDMCMD "/etc/athena/" XDM " -config /etc/athena/login/xdm/xdm-config"
extern int debug;

static int run(char *what, int newpag)
{
  char cmd[1024], *args[20], *ptr;
  int i = 0, status;
  sigset_t mask, omask;
  pid_t pid, ret;

  if (what == NULL)
    return -1;

  strcpy(cmd, what);
  ptr = cmd;
  while (*ptr != '\0')
    {
      args[i++] = ptr;
      while (*ptr != '\0' && *ptr != ' ') ptr++;
      if (*ptr == ' ')
	*ptr++ = '\0';
    }
  args[i] = NULL;

  /* Guard against kidnapping. */
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &omask);

  if ((pid = fork()) == 0)
    {
      if (newpag)
	setpag();
      sigprocmask(SIG_SETMASK, &omask, NULL);
      execv(args[0], args);
      exit(-1);
    }

  if (pid == -1)
    {
      sigprocmask(SIG_SETMASK, &omask, NULL);
      return -1;
    }

  while (((ret = waitpid(pid, &status, 0)) == -1) && (errno == EINTR))
    ;
  sigprocmask(SIG_SETMASK, &omask, NULL);

  if ((ret == pid) && WIFEXITED(status))
    return WEXITSTATUS(status);

  return -2;
}

dpy_state *dpy_init(void)
{
  dpy_state *dpy;

  dpy = malloc(sizeof(dpy_state));
  dpy->mode = DPY_NONE;

  return dpy;
}

/* Wake up xdm, starting it if necessary. */
static int poke_xdm()
{
  int ret;

  ret = run(KILLALL "-HUP " XDM, 0);
  if (ret != 0)
    {
      if (debug > 3)
	syslog(LOG_ERR, "killall -hup " XDM " returned %d", ret);

      /*
       * We make xdm's PAG unique - we don't want it shared with the
       * default root PAG, since user's tokens will be flying through
       * it. But xdm does need to see the user's tokens, because it
       * tries to stat their home directory before making it their
       * HOME and logging it in. (Otherwise they get HOME=/.)
       * Therefore, xlogin does not to a setpag() under this OS. Note:
       * this means that we cannot currently use xdm to drive multiple
       * sessions without getting into race conditions.
       */

      ret = run(XDMCMD, 1);
      if (ret != 0)
	{
	  if (debug > 3)
	    syslog(LOG_ERR, XDM " returned %d", ret);
	  return 1;
	}
    }
  return 0;
}

int dpy_startX(dpy_state *dpy)
{
  if (dpy == NULL || dpy->mode != DPY_NONE)
    return 1;

  if (run(CHKCONFIG WS "on", 0))
    return 1;

  if (poke_xdm() != 0)
    return 1;

  dpy->mode = DPY_X;
  return 0;
}

int dpy_restartX(dpy_state *dpy)
{
  if (dpy == NULL || dpy->mode != DPY_X)
    return 1;

  /* Kill the X server, wait a bit, then ensure xdm is alive. */
  run(KILLALL "Xsgi", 0);
  sleep(2);

  if (poke_xdm() != 0)
    return 1;

  return 0;
}

int dpy_stopX(dpy_state *dpy)
{
  if (dpy == NULL || dpy->mode != DPY_X)
    return 1;

  if (1 != run(CHKCONFIG WS "off", 0))
    return 1;

  /* Kill xdm to work around bug in IRIX 6.5 xdm, which loops trying
   * to restart Xsgi when the latter exits.
   */
  run(KILLALL "-TERM " XDM, 0);

  run(KILLALL "Xsgi", 0); /* don't know enough about the exit status...
			     it might not have found a server, which is
			     fine, or something strange might have happened,
			     which isn't */
  dpy->mode = DPY_NONE;
  return 0;
}

int dpy_startCons(dpy_state *dpy)
{
  int fd, on = 1;

  if (dpy == NULL || dpy->mode != DPY_NONE)
    return 1;

  if ((dpy->login = fork()) == 0)
    {
      /* remove DISPLAY... */
      execl("/sbin/getty", "getty", CONSLINE, "co_9600", 0);
      exit(1);
    }

  if (dpy->login == -1)
    return 1;

  dpy->mode = DPY_CONS;
  return 0;
}

int dpy_stopCons(dpy_state *dpy)
{
  if (dpy == NULL || dpy->mode != DPY_CONS)
    return 1;

  /* I don't think we can actually do this. login does not appear
     to take any cleanup signals, so we can't usefully kill it off
     to get rid of the user under it. */
  return 1;
}

/* Return the name of the console tty line in /dev */
char *dpy_consline(dpy_state *dpy)
{
  return CONSLINE;
}

int dpy_status(dpy_state *dpy)
{
  if (dpy)
    return dpy->mode;
  else
    return 0;
}

int dpy_child(dpy_state *dpy, pid_t child, void *status)
{
  if (dpy == NULL || child != dpy->login)
    return 1;

  dpy->mode = DPY_NONE;
  return 0;
}
