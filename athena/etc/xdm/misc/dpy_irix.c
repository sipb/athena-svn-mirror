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
#include "dpy_irix.h"

#define CHKCONFIG "/etc/chkconfig "
#define WS "windowsystem "
#define KILLALL "/etc/killall "

#define CONSDEV "/dev/tport"
#define XDM "axdm"
#define XDMCMD "/etc/athena/" XDM " -config /etc/athena/login/xdm/xdm-config"
extern int debug;

#define USE_GETTY

static int run(char *what)
{
  char cmd[1024], *args[20], *ptr;
  int i = 0, status;
  sigset_t mask, omask;
  pid_t pid;

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
      sigprocmask(SIG_SETMASK, &omask, NULL);
      execv(args[0], args);
      exit(-1);
    }

  if (pid == -1)
    {
      sigprocmask(SIG_SETMASK, &omask, NULL);
      return -1;
    }

  while (pid != waitpid(pid, &status, 0));
  sigprocmask(SIG_SETMASK, &omask, NULL);

  if (WIFEXITED(status))
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

#ifdef USE_GETTY
/* Return the device on which the login will run when we go to
   console mode. The dpy_ code directly or indirectly handles
   utmp, this should return NULL. */
char *dpy_consDevice(dpy_state *dpy)
{
  return CONSDEV;
}
#else
char *dpy_consDevice(dpy_state *dpy)
{
  return NULL;
}
#endif

int dpy_startX(dpy_state *dpy)
{
  int ret;

  if (dpy == NULL || dpy->mode != DPY_NONE)
    return 1;

  if (run(CHKCONFIG WS "on"))
    return 1;

  if (ret = run(KILLALL "-HUP " XDM))
    {
      if (debug > 3)
	syslog(LOG_ERR, "killall -hup " XDM " returned %d", ret);

      if (ret = run(XDMCMD))
	{
	  if (debug > 3)
	    syslog(LOG_ERR, XDM " returned %d", ret);
	  return 1;
	}
    }

  dpy->mode = DPY_X;
  return 0;
}

int dpy_stopX(dpy_state *dpy)
{
  if (dpy == NULL || dpy->mode != DPY_X)
    return 1;

  if (1 != run(CHKCONFIG WS "off"))
    return 1;

  run(KILLALL "Xsgi"); /* don't know enough about the exit status...
			  it might not have found a server, which is
			  fine, or something strange might have happened,
			  which isn't */
  dpy->mode = DPY_NONE;
  return 0;
}
#ifdef USE_GETTY
int dpy_startCons(dpy_state *dpy)
{
  int fd, on = 1;

  if (dpy == NULL || dpy->mode != DPY_NONE)
    return 1;

  if ((dpy->login = fork()) == 0)
    {
      /* remove DISPLAY... */
      execl("/sbin/getty", "getty", "tport", "co_9600", 0);
      exit(1);
    }

  if (dpy->login == -1)
    return 1;

  dpy->mode = DPY_CONS;
  return 0;
}
#else
int dpy_startCons(dpy_state *dpy)
{
  int fd, on = 1;

  if (dpy == NULL || dpy->mode != DPY_NONE)
    return 1;

  if ((dpy->login = fork()) == 0)
    {
      close(0);
      close(1);
      close(2);
      setsid();
      fd = open(CONSDEV, O_RDWR, 0);
      ioctl(fd, TIOCCONS, (char *) &on);
      dup2(fd, 0);
      dup2(fd, 1);
      dup2(fd, 2);

      execl("/bin/login", "login", 0); /* remove DISPLAY... */
      exit(1);
    }

  if (dpy->login == -1)
    return 1;

  dpy->mode = DPY_CONS;
  return 0;
}
#endif

int dpy_stopCons(dpy_state *dpy)
{
  if (dpy == NULL || dpy->mode != DPY_CONS)
    return 1;

  /* I don't think we can actually do this. login does not appear
     to take any cleanup signals, so we can't usefully kill it off
     to get rid of the user under it. */
  return 1;
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
