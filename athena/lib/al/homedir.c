/* Copyright 1997 by the Massachusetts Institute of Technology.
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

/* This file is part of the Athena login library.  It implements
 * functions to set up and revert user home directories.
 */

static const char rcsid[] = "$Id: homedir.c,v 1.3 1997-10-30 23:58:55 ghudson Exp $";

#include <hesiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include "al.h"
#include "al_private.h"

static int nuketmpdir(const char *tmpdir, struct passwd *pw);

int al__setup_homedir(const char *username, struct al_record *record,
		      int havecred, int tmphomedir)
{
  struct passwd *local_pwd, *hes_pwd;
  pid_t pid;
  int status;
  char *tmpdir, *saved_homedir;
  void *hescontext;

  /* If there's an existing session using a tmp homedir, we'll use that. */
  if (record->old_homedir)
    return AL_WXTMPDIR;

  /* Get local password entry. User should already have been added to
   * passwd database, so if this fails, we've already lost, so punt. */
  local_pwd = al__getpwnam(username);
  if (!local_pwd)
    return AL_WNOTMPDIR;

  /* Get hesiod password entry. If the user has no hesiod passwd
   * entry or the listed homedir differs from the local passwd entry,
   * return AL_SUCCESS (and use the local homedir). */
  if (hesiod_init(&hescontext) != 0)
    return AL_WNOTMPDIR;
  hes_pwd = hesiod_getpwnam(hescontext, username);
  if (!hes_pwd || strcmp(local_pwd->pw_dir, hes_pwd->pw_dir))
    {
      hesiod_free_passwd(hescontext, hes_pwd);
      hesiod_end(hescontext);
      al__free_passwd(local_pwd);
      return AL_SUCCESS;
    }
  hesiod_free_passwd(hescontext, hes_pwd);
  hesiod_end(hescontext);

  /* We want to attach a remote home directory. Make sure this is OK. */
  if (access(PATH_NOATTACH, F_OK) == 0)
    return AL_WNOATTACH;

  pid = fork();
  switch (pid)
    {
    case -1:
      /* If we can't fork, we just lose. */
      al__free_passwd(local_pwd);
      return AL_WNOTMPDIR;

    case 0:
      if (havecred)
	{
	  execl(PATH_ATTACH, "attach", "-user", username, "-quiet",
		"-nozephyr", username, NULL);
	}
      else
	{
	  execl(PATH_ATTACH, "attach", "-user", username, "-quiet",
		"-nozephyr", "-nomap", username, NULL);
	}
      _exit(1);

    default:
      while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
	;

      if (WIFEXITED(status))
	{
	  /* If we succeeded, or if the remote homedir doesn't exist
	   * and we don't already have credentials (XXX consider this
	   * more), then return success. */
	  if ((WEXITSTATUS(status) == 0 &&
	       access(local_pwd->pw_dir, F_OK) == 0) ||
	      (WEXITSTATUS(status) == 26 && !havecred))
	    {
	      record->attached = 1;
	      al__free_passwd(local_pwd);
	      return AL_SUCCESS;
	    }
	}
      break;
    }

  /* attach failed somehow. Try to make a local homedir now, unless
   * the caller doesn't want that. */
  if (!tmphomedir)
    {
      al__free_passwd(local_pwd);
      return AL_WNOTMPDIR;
    }

  tmpdir = malloc(strlen(username) + 24);
  if (!tmpdir)
    {
      al__free_passwd(local_pwd);
      return AL_ENOMEM;
    }
  sprintf(tmpdir, "%s/%s", PATH_TMPDIRS, username);

  if (access(tmpdir, F_OK) == -1)
    {
      if (access(PATH_TMPDIRS, F_OK) == -1)
	mkdir(PATH_TMPDIRS, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
      if (mkdir(tmpdir, S_IRWXU) == -1)
	{
	  free(tmpdir);
	  al__free_passwd(local_pwd);
	  return AL_WNOTMPDIR;
	}
      if (chown(tmpdir, local_pwd->pw_uid, local_pwd->pw_gid) == -1)
	{
	  rmdir(tmpdir);
	  free(tmpdir);
	  al__free_passwd(local_pwd);
	  return AL_WNOTMPDIR;
	}

      pid = fork();
      switch (pid)
	{
	case -1:
	  free(tmpdir);
	  al__free_passwd(local_pwd);
	  return AL_WNOTMPDIR;

	case 0:
	  if (setgid(local_pwd->pw_gid) == -1
	      || setuid(local_pwd->pw_uid) == -1)
	    _exit(1);
	  execlp("cp", "cp", "-r", PATH_TMPPROTO, tmpdir, NULL);
	  _exit(1);

	default:
	  while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
	    ;
	  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	    {
	      free(tmpdir);
	      al__free_passwd(local_pwd);
	      return AL_WNOTMPDIR;
	    }
	  break;
	}
    }

  /* Update session records.  (malloc first so we will never have
   * to back out after calling al__change_passwd_homedir). */
  if (!record->passwd_added)
    {
      saved_homedir = malloc(strlen(local_pwd->pw_dir) + 1);
      if (!saved_homedir)
	{
	  free(tmpdir);
	  al__free_passwd(local_pwd);
	  return AL_ENOMEM;
	}
      strcpy(saved_homedir, local_pwd->pw_dir);
    }
  if (al__change_passwd_homedir(username, tmpdir) != AL_SUCCESS)
    {
      nuketmpdir(tmpdir, local_pwd);
      free(tmpdir);
      al__free_passwd(local_pwd);
      return AL_WNOTMPDIR;
    }
  if (!record->passwd_added)
    record->old_homedir = saved_homedir;

  free(tmpdir);
  al__free_passwd(local_pwd);
  return AL_WTMPDIR;
}

int al__revert_homedir(const char *username, struct al_record *record)
{
  struct passwd *local_pwd;
  pid_t pid;
  int status;
  char *tmpdir;

  local_pwd = al__getpwnam(username);
  if (!local_pwd)
    return AL_EPERM;

  if (record->old_homedir)
    {
      if (!record->passwd_added)
	{
	  if (al__change_passwd_homedir(username, record->old_homedir)
	      != AL_SUCCESS)
	    {
	      al__free_passwd(local_pwd);
	      return AL_EPERM;
	    }
	}

      tmpdir = malloc(strlen(username) + 24);
      if (!tmpdir)
	{
	  al__free_passwd(local_pwd);
	  return AL_ENOMEM;
	}
      sprintf(tmpdir, "%s/%s", PATH_TMPDIRS, username);

      if (nuketmpdir(tmpdir, local_pwd) != AL_SUCCESS)
	{
	  free(tmpdir);
	  al__free_passwd(local_pwd);
	  return AL_EPERM;
	}
      free(tmpdir);
    }

  if (record->attached)
    {
      pid = fork();
      switch (pid)
	{
	case -1:
	  al__free_passwd(local_pwd);
	  return AL_ENOMEM;

	case 0:
	  if (setgid(local_pwd->pw_gid) == -1
	      || setuid(local_pwd->pw_uid) == -1)
	    _exit(1);
	  execl(PATH_DETACH, "detach", "-quiet", "-nozephyr",
		username, NULL);
	  _exit(1);

	default:
	  while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
	    ;
	  break;
	}
    }

  al__free_passwd(local_pwd);
  return AL_SUCCESS;
}

static int nuketmpdir(const char *tmpdir, struct passwd *pw)
{
  pid_t pid;
  int status;

  pid = fork();
  switch (pid)
    {
    case -1:
      return AL_ENOMEM;

    case 0:
      if (setgid(pw->pw_gid) == -1 || setuid(pw->pw_uid) == -1)
	_exit(1);
      execlp("rm", "rm", "-rf", tmpdir, NULL);
      _exit(1);

    default:
      while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
	;
      if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	return AL_EPERM;
      break;
    }

  if (rmdir(tmpdir) == -1)
    return AL_EPERM;

  return AL_SUCCESS;
}
