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

static const char rcsid[] = "$Id: homedir.c,v 1.7 1998-01-14 17:09:59 ghudson Exp $";

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
#include <dirent.h>
#include "al.h"
#include "al_private.h"

int al__setup_homedir(const char *username, struct al_record *record,
		      int havecred, int tmphomedir)
{
  struct passwd *local_pwd, *hes_pwd;
  pid_t pid, rpid;
  int status;
  char *tmpdir, *tmpfile, *saved_homedir;
  void *hescontext;
  DIR *dir;
  struct dirent *entry;

  /* If there's an existing session using a tmp homedir, we'll use that. */
  if (record->old_homedir)
    return AL_WXTMPDIR;

  /* Get local password entry. User should already have been added to
   * passwd database, so if this fails, we've already lost, so punt. */
  local_pwd = al__getpwnam(username);
  if (!local_pwd)
    return AL_WNOHOMEDIR;

  /* Get hesiod password entry. If the user has no hesiod passwd
   * entry or the listed homedir differs from the local passwd entry,
   * return AL_SUCCESS (and use the local homedir). */
  if (hesiod_init(&hescontext) != 0)
    {
      al__free_passwd(local_pwd);
      return AL_WNOHOMEDIR;
    }
  hes_pwd = hesiod_getpwnam(hescontext, username);
  if (!hes_pwd || strcmp(local_pwd->pw_dir, hes_pwd->pw_dir))
    {
      if (hes_pwd)
	hesiod_free_passwd(hescontext, hes_pwd);
      hesiod_end(hescontext);
      al__free_passwd(local_pwd);
      return AL_SUCCESS;
    }
  if (hes_pwd)
    hesiod_free_passwd(hescontext, hes_pwd);
  hesiod_end(hescontext);

  /* We want to attach a remote home directory. Make sure this is OK. */
  if (access(PATH_NOATTACH, F_OK) == 0)
    {
      al__free_passwd(local_pwd);
      return AL_WNOATTACH;
    }

  pid = fork();
  switch (pid)
    {
    case -1:
      /* If we can't fork, we just lose. */
      al__free_passwd(local_pwd);
      return AL_WNOHOMEDIR;

    case 0:
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      if (havecred)
	{
	  execl(PATH_ATTACH, "attach", "-user", username, "-quiet",
		"-nozephyr", username, (char *) NULL);
	}
      else
	{
	  execl(PATH_ATTACH, "attach", "-user", username, "-quiet",
		"-nozephyr", "-nomap", username, (char *) NULL);
	}
      _exit(1);

    default:
      while ((rpid = waitpid(pid, &status, 0)) < 0 && errno == EINTR)
	;

      if (rpid == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0 &&
	  access(local_pwd->pw_dir, F_OK) == 0)
	{
	  record->attached = 1;
	  al__free_passwd(local_pwd);
	  return AL_SUCCESS;
	}
      break;
    }

  /* attach failed somehow. Try to make a local homedir now, unless
   * the caller doesn't want that. */
  if (!tmphomedir)
    {
      al__free_passwd(local_pwd);
      return AL_WNOHOMEDIR;
    }

  /* Allocate space to hold directory names. */
  tmpdir = malloc(strlen(PATH_TMPDIRS) * 2 + strlen(username) * 2 + 8);
  if (!tmpdir)
    {
      al__free_passwd(local_pwd);
      return AL_ENOMEM;
    }

  /* If the user's temporary directory does not exist, we need to create
   * it.  PATH_TMPDIRS is not world-writable, so we don't have to be
   * paranoid about the creation of the user home directory, but we do
   * have to be careful about doing anything as root in a diretory which
   * we've already chowned to the user. */
  sprintf(tmpdir, "%s/%s", PATH_TMPDIRS, username);
  if (access(tmpdir, F_OK) == -1)
    {
      /* First make sure PATH_TMPDIRS exists. */
      if (access(PATH_TMPDIRS, F_OK) == -1)
	mkdir(PATH_TMPDIRS, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);

      /* Create and chown tmpdir. */
      if (mkdir(tmpdir, S_IRWXU) == -1
	  || chown(tmpdir, local_pwd->pw_uid, local_pwd->pw_gid) == -1)
 	{
	  rmdir(tmpdir);
 	  free(tmpdir);
 	  al__free_passwd(local_pwd);
	  return AL_WNOHOMEDIR;
 	}

      /* Copy files from PATH_TMPPROTO to the ephemeral directory. */
      dir = opendir(PATH_TMPPROTO);
      if (!dir)
	{
	  rmdir(tmpdir);
	  free(tmpdir);
	  al__free_passwd(local_pwd);
	  return AL_WNOHOMEDIR;
	}

      while ((entry = readdir(dir)) != NULL)
	{
	  if (strcmp(entry->d_name, ".") == 0
	      || strcmp(entry->d_name, "..") == 0)
	    continue;

	  /* fork to copy the file into the tmpdir. */
	  pid = fork();
	  switch (pid)
	    {
	    case -1:
	      closedir(dir);
	      rmdir(tmpdir);
	      free(tmpdir);
	      al__free_passwd(local_pwd);
	      return AL_WNOHOMEDIR;

	    case 0:
	      close(STDOUT_FILENO);
	      close(STDERR_FILENO);
	      if (setgid(local_pwd->pw_gid) == -1
		  || setuid(local_pwd->pw_uid) == -1)
		_exit(1);
	      tmpfile = malloc(strlen(PATH_TMPPROTO) + strlen(entry->d_name)
			       + 2);
	      if (!tmpfile)
		_exit(1);
	      sprintf(tmpfile, "%s/%s", PATH_TMPPROTO, entry->d_name);
	      execlp("cp", "cp", tmpfile, tmpdir, (char *) NULL);
	      _exit(1);

	    default:
	      while ((rpid = waitpid(pid, &status, 0) < 0) && errno == EINTR)
		;
	      if (rpid == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		{
		  closedir(dir);
		  rmdir(tmpdir);
		  free(tmpdir);
		  al__free_passwd(local_pwd);
		  return AL_WNOHOMEDIR;
		}
	      break;
	    }
	}
      closedir(dir);
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
      free(tmpdir);
      al__free_passwd(local_pwd);
      return AL_WNOHOMEDIR;
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

  local_pwd = al__getpwnam(username);
  if (!local_pwd)
    return AL_EPERM;

  if (record->old_homedir && !record->passwd_added)
    {
      if (al__change_passwd_homedir(username,
				    record->old_homedir) != AL_SUCCESS)
	{
	  al__free_passwd(local_pwd);
	  return AL_EPERM;
	}
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
	  close(STDOUT_FILENO);
	  close(STDERR_FILENO);
	  if (setgid(local_pwd->pw_gid) == -1
	      || setuid(local_pwd->pw_uid) == -1)
	    _exit(1);
	  execl(PATH_DETACH, "detach", "-quiet", "-nozephyr",
		username, (char *) NULL);
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
