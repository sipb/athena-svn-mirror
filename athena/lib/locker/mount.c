/* Copyright 1998 by the Massachusetts Institute of Technology.
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

/* This file is part of liblocker. It implements routines to
 * mount and unmount system-recognized filesystems.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "locker.h"
#include "locker_private.h"

static const char rcsid[] = "$Id: mount.c,v 1.3 1999-05-19 14:19:49 danw Exp $";

static int domount(locker_context context, int mount,
		   locker_attachent *at, char **argv);
static void cancel_mount(int sig);
static volatile int cancel;


int locker__mount(locker_context context, locker_attachent *at,
		  char *mountoptions)
{
  char *argv[6];

  argv[0] = MOUNT_CMD;
  argv[1] = "-o";
  argv[2] = mountoptions;
  argv[3] = at->hostdir;
  argv[4] = at->mountpoint;
  argv[5] = NULL;
  return domount(context, 1, at, argv);
}

int locker__unmount(locker_context context, locker_attachent *at)
{
  char *argv[3];
  int status;

  argv[0] = UMOUNT_CMD;
  argv[1] = at->mountpoint;
  argv[2] = NULL;
  status = domount(context, 0, at, argv);

  if (status == LOCKER_EDETACH)
    {
      /* Check if the detach failed because the filesystem was already
       * unmounted. (By seeing if the mountpoint directory isn't
       * there, or if its st_dev matches its parent's st_dev.)
       */
      struct stat st1, st2;
      char *parent, *p;

      if (stat(at->mountpoint, &st1) == -1 && errno == ENOENT)
	return LOCKER_SUCCESS;
      parent = strdup(at->mountpoint);
      if (!parent)
	return LOCKER_EDETACH;

      p =  strrchr(parent, '/');
      if (!p)
	{
	  free(parent);
	  return LOCKER_EDETACH;
	}
      if (p == parent)
	p++;
      *p = '\0';
      status = stat(parent, &st2);
      free(parent);
      if (status == -1)
	return LOCKER_EDETACH;

      if (st1.st_dev == st2.st_dev)
	return LOCKER_SUCCESS;
      else
	return LOCKER_EDETACH;
    }
  else
    return status;
}

static int domount(locker_context context, int mount,
		   locker_attachent *at, char **argv)
{
  pid_t pid;
  int status, pstat, pipefds[2], bufsiz, nread, new, oldalarm;
  char *buf = NULL;
  sigset_t omask, mask;
  struct sigaction sa, sa_chld, sa_int, sa_alrm;

  if (pipe(pipefds) == -1)
    {
      locker__error(context, "%s: Could not open pipe for mount: %s.\n",
		    at->name, strerror(errno));
      return mount ? LOCKER_EATTACH : LOCKER_EDETACH;
    }

  /* If the liblocker caller has set SIGCHLD's handler to SIG_IGN,
   * waitpid won't work. So we set its handler to SIG_DFL after
   * blocking it. This makes waitpid work but still prevents the
   * caller from seeing SIGCHLDs.
   */
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &omask);

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SIG_DFL;
  sigaction(SIGCHLD, &sa, &sa_chld);

  /* Set SIGINT and SIGALRM to interrupt us nicely and then unblock
   * them.
   */
  cancel = 0;
  sa.sa_handler = cancel_mount;
  sigaction(SIGINT, &sa, &sa_int);
  sigaction(SIGALRM, &sa, &sa_alrm);
  oldalarm = alarm(LOCKER_MOUNT_TIMEOUT);

  sigdelset(&mask, SIGALRM);
  sigdelset(&mask, SIGINT);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  switch (pid = fork())
    {
    case -1:
      close(pipefds[0]);
      close(pipefds[1]);
      if (errno == ENOMEM)
	{
	  locker__error(context, "Not enough memory to fork.\n");
	  status = LOCKER_ENOMEM;
	  goto cleanup;
	}
      else
	{
	  locker__error(context, "%s: Could not fork to exec %s: %s.\n",
			at->name, argv[0], strerror(errno));
	  status = mount ? LOCKER_EATTACH : LOCKER_EDETACH;
	  goto cleanup;
	}

    case 0:
      setuid(0);
      close(pipefds[0]);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      dup2(pipefds[1], STDOUT_FILENO);
      dup2(pipefds[1], STDERR_FILENO);

      execv(argv[0], argv);
      exit(1);

    default:
      close(pipefds[1]);
    }

  /* Need to read the data out of the pipe first, in case mount blocks
   * writing to it before it can exit.
   */
  bufsiz = nread = 0;

  new = -1;
  while (new != 0 && !cancel)
    {
      char *nbuf;

      bufsiz += 1024;
      nbuf = realloc(buf, bufsiz);
      if (nbuf)
	{
	  buf = nbuf;
	  new = read(pipefds[0], buf + nread, 1023);
	}

      if (!nbuf || new == -1)
	{
	  /* We can't finish reading the buffer, but we need to wait for
	   * the child to exit.
	   */
	  free(buf);
	  buf = NULL;
	  break;
	}
      else
	nread += new;
    }
  close(pipefds[0]);
  if (buf)
    buf[nread] = '\0';

  do
    {
      if (cancel)
	kill(pid, SIGKILL);
    }
  while (waitpid(pid, &pstat, 0) < 0 && errno == EINTR);

  if (WIFEXITED(pstat) && WEXITSTATUS(pstat) == 0)
    status = LOCKER_SUCCESS;
  else if (cancel == SIGALRM)
    {
      locker__error(context, "%s: %s timed out.\n", at->name,
		    mount ? "Mount" : "Unmount");
      status = mount ? LOCKER_EATTACH : LOCKER_EDETACH;
    }
  else if (cancel == SIGINT)
    {
      locker__error(context, "%s: %s interrupted by user.\n", at->name,
		    mount ? "Mount" : "Unmount");
      status = mount ? LOCKER_EATTACH : LOCKER_EDETACH;
    }
  else
    {
      locker__error(context, "%s: %s failed:\n%s", at->name,
		    mount ? "Mount" : "Unmount",
		    buf ? buf : "Unknown error.\n");
      status = mount ? LOCKER_EATTACH : LOCKER_EDETACH;
    }

cleanup:
  free(buf);
  sigaction(SIGCHLD, &sa_chld, NULL);
  sigaction(SIGINT, &sa_int, NULL);
  alarm(0);
  sigaction(SIGALRM, &sa_alrm, NULL);
  sigprocmask(SIG_SETMASK, &omask, NULL);
  alarm(oldalarm);

  return status;
}

static void cancel_mount(int sig)
{
  cancel = sig;
}
