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
 * functions to add and remove a user from the system passwd database.
 */

static const char rcsid[] = "$Id: passwd.c,v 1.6 1997-11-17 22:06:19 danw Exp $";

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <hesiod.h>
#ifdef HAVE_SHADOW
#include <shadow.h>
#endif
#include "al.h"
#include "al_private.h"

/* /etc/ptmp should be mode 600 on a master.passwd system, 644 otherwise. */
#ifdef HAVE_MASTER_PASSWD
#define PTMP_MODE (S_IWUSR|S_IRUSR)
#else
#define PTMP_MODE (S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH)
#endif

static int copy_changing_cryptpw(FILE *in, FILE *out, const char *username,
				 const char *cryptpw);

/* This is an internal function.  Its contract is to lock the passwd
 * database in a manner consistent with the operating system and return
 * the file handle of a temporary file (which may or may not also be
 * the lock file) into which to write the new contents of the passwd
 * file.
 */

static FILE *lock_passwd()
{
#ifdef HAVE_LCKPWDF
  FILE *fp;

  if (lckpwdf() == -1)
    return NULL;
  fp = fopen(PATH_PASSWD_TMP, "w");
  if (fp)
    fchmod(fileno(fp), S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
  else
    ulckpwdf();
  return fp;
#else
  int i, fd = -1;
  FILE *fp;

  for (i = 0; i < 10; i++)
    {
      fd = open(PATH_PASSWD_TMP, O_RDWR|O_CREAT|O_EXCL, PTMP_MODE);
      if (fd >= 0 || errno != EEXIST)
	break;
      sleep(1);
    }
  if (fd == -1)
    return NULL;
  fp = fdopen(fd, "w");
  if (fp == NULL)
    unlink(PATH_PASSWD_TMP);
  return fp;
#endif
}

/* This is an internal function.  Its contract is to replace the passwd
 * file with the temporary file.
 */

static int update_passwd(FILE *fp)
{
#ifdef HAVE_MASTER_PASSWD
  int pstat;
  pid_t pid, rpid;
#endif
  int status;

  status = ferror(fp);
  if (fclose(fp) || status)
    {
      unlink(PATH_PASSWD_TMP);
#ifdef HAVE_LCKPWDF
      ulckpwdf();
#endif
    }

  /* Replace the passwd file with the lock file. */
#ifdef HAVE_MASTER_PASSWD
  pid = fork();
  if (pid == 0)
    {
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      execl(_PATH_PWD_MKDB, "pwd_mkdb", "-p", PATH_PASSWD_TMP, NULL);
      _exit(1);
    }
  while ((rpid = waitpid(pid, &pstat, 0)) < 0 && errno == EINTR)
    ;
  if (rpid == -1 || !WIFEXITED(pstat) || WEXITSTATUS(pstat) != 0)
    return AL_EPASSWD;
#else /* HAVE_MASTER_PASSWD */
  if (rename(PATH_PASSWD_TMP, PATH_PASSWD))
    {
      unlink(PATH_PASSWD_TMP);
#ifdef HAVE_LCKPWDF
      ulckpwdf();
#endif
      return AL_EPASSWD;
    }
#endif /* HAVE_MASTER_PASSWD */

  /* Unlock the passwd file if we're using System V style locking. */
#ifdef HAVE_LCKPWDF
  ulckpwdf();
#endif

  return AL_SUCCESS;
}

/* This is an internal function.  Its contract is to clean up after a
 * failed attempt to write the new passwd file.
 */

static void discard_passwd_lockfile(FILE *fp)
{
  fclose(fp);
  unlink(PATH_PASSWD_TMP);
#ifdef HAVE_LCKPWDF
  ulckpwdf();
#endif
  return;
}

/* This is an internal function.  Its contract is to add the user to the
 * local passwd database if appropriate, and set record->passwd_added to
 * 1 if it adds a passwd line.
 *
 * There are three implementations of the passwd database which concern
 * us:
 * 	* An /etc/passwd file
 * 	* An /etc/passwd and /etc/shadow file
 * 	* An /etc/master.passwd file and associated databases
 * For simplicity, PATH_PASSWD is defined to be "/etc/passwd" on the
 * first two types of systems and to "/etc/master.passwd" on the third
 * type.
 */

int al__add_to_passwd(const char *username, struct al_record *record,
		      const char *cryptpw)
{
  FILE *in = NULL, *out = NULL;
#ifdef HAVE_SHADOW
  FILE *shadow_in = NULL, *shadow_out = NULL;
#endif
  struct passwd *pwd, *tmppwd;
  const char *passwd;
  char buf[BUFSIZ];
  int nbytes, retval, have_nocrack, fd;
  void *hescontext = NULL;

  tmppwd = al__getpwnam(username);
  if (tmppwd)
    {
      al__free_passwd(tmppwd);
      return AL_SUCCESS;
    }

  errno = 0;
  if (hesiod_init(&hescontext) != 0)
    return (errno == ENOMEM) ? AL_ENOMEM : AL_ENOUSER;

  pwd = hesiod_getpwnam(hescontext, username);

  if (!pwd)
    {
      hesiod_end(hescontext);
      return (errno == ENOMEM) ? AL_ENOMEM : AL_ENOUSER;
    }

  /* uid must not conflict with one already in passwd file. */
  tmppwd = al__getpwuid(pwd->pw_uid);
  if (tmppwd)
    {
      al__free_passwd(tmppwd);
      hesiod_free_passwd(hescontext, pwd);
      hesiod_end(hescontext);
      return AL_EBADHES;
    }

  have_nocrack = !access(PATH_NOCRACK, F_OK);
  if (cryptpw && have_nocrack)
    passwd = cryptpw;
  else
    passwd = pwd->pw_passwd;

  out = lock_passwd();
  in = fopen(PATH_PASSWD, "r");
  if (!out || !in)
    goto cleanup;
  do
    {
      nbytes = fread(buf, sizeof(char), BUFSIZ, in);
      if (nbytes)
	{
	  if (!fwrite(buf, sizeof(char), nbytes, out))
	    goto cleanup;
	}
    }
  while (nbytes == BUFSIZ);

  fprintf(out, "%s:%s:%lu:%lu:%s:%s:%s\n",
	  pwd->pw_name,
#ifdef HAVE_SHADOW
	  "x",
#else
	  passwd,
#endif
	  (unsigned long) pwd->pw_uid, (unsigned long) pwd->pw_gid,
	  pwd->pw_gecos, pwd->pw_dir, pwd->pw_shell);

#ifdef HAVE_SHADOW
  fd = open(PATH_SHADOW_TMP, O_RDWR|O_CREAT, S_IWUSR|S_IRUSR);
  if (fd < 0)
    goto cleanup;
  shadow_out = fdopen(fd, "w");
  if (!shadow_out)
    {
      close(fd);
      unlink(PATH_SHADOW_TMP);
      goto cleanup;
    }
  shadow_in = fopen(PATH_SHADOW, "r");
  if (!shadow_in)
    goto cleanup;
  retval = copy_changing_cryptpw(shadow_in, shadow_out, username, passwd);
  if (retval == -1)
    goto cleanup;
  if (retval == 0)
    {
      fprintf(shadow_out, "%s:%s:%lu::::::\n", pwd->pw_name, passwd,
	      (unsigned long) (time(NULL) / (60 * 60 * 24)));
    }
  retval = ferror(shadow_out);
  retval = fclose(shadow_out) || retval;
  shadow_out = NULL;
  if (retval)
    goto cleanup;
  retval = fclose(shadow_in);
  shadow_in = NULL;
  if (retval)
    goto cleanup;
  if (rename(PATH_SHADOW_TMP, PATH_SHADOW))
    goto cleanup;
#endif

  retval = fclose(in);
  in = NULL;
  if (retval)
    goto cleanup;
  hesiod_free_passwd(hescontext, pwd);
  hesiod_end(hescontext);
  retval = update_passwd(out);
  if (retval == AL_SUCCESS)
    record->passwd_added = 1;
  return retval;

cleanup:
  if (hescontext)
    {
      if (pwd)
	hesiod_free_passwd(hescontext, pwd);
      hesiod_end(hescontext);
    }
  if (in)
    fclose(in);
#ifdef HAVE_SHADOW
  if (shadow_in)
    fclose(shadow_in);
  if (shadow_out)
    {
      fclose(shadow_out);
      unlink(PATH_SHADOW_TMP);
    }
#endif
  if (out)
    discard_passwd_lockfile(out);
  return AL_EPASSWD;
}

/* This is an internal function.  Its contract is to remove username from
 * the passwd file and shadow file if record->passwd_added is true.
 */

int al__remove_from_passwd(const char *username, struct al_record *record)
{
  FILE *in = NULL, *out = NULL;
#ifdef HAVE_SHADOW
  FILE *shadow_in = NULL, *shadow_out = NULL;
#endif
  char *buf = NULL;
  int bufsize = 0, retval, len, fd;

  if (!record->passwd_added)
    return AL_SUCCESS;
  
  out = lock_passwd();
  in = fopen(PATH_PASSWD, "r");
  if (!out || !in)
    goto cleanup;
  len = strlen(username);

  while ((retval = al__read_line(in, &buf, &bufsize)) == 0)
    {
      if (strncmp(username, buf, len) != 0 || buf[len] != ':')
	{
	  fputs(buf, out);
	  fputs("\n", out);
	}
    }
  if (retval == -1)
    goto cleanup;
  retval = fclose(in);
  in = NULL;
  if (retval)
    goto cleanup;

#ifdef HAVE_SHADOW
  fd = open(PATH_SHADOW_TMP, O_RDWR|O_CREAT, S_IWUSR|S_IRUSR);
  if (fd < 0)
    goto cleanup;
  shadow_out = fdopen(fd, "w");
  if (!shadow_out)
    {
      close(fd);
      unlink(PATH_SHADOW_TMP);
      goto cleanup;
    }
  shadow_in = fopen(PATH_SHADOW, "r");
  if (!shadow_in)
    goto cleanup;

  while ((retval = al__read_line(shadow_in, &buf, &bufsize)) == 0)
    {
      if (strncmp(username, buf, len) != 0 || buf[len] != ':')
	{
	  fputs(buf, shadow_out);
	  fputs("\n", shadow_out);
	}
    }
  if (retval == -1)
    goto cleanup;
  retval = fclose(shadow_out);
  shadow_out = NULL;
  if (retval)
    goto cleanup;
  retval = fclose(shadow_in);
  shadow_in = NULL;
  if (retval)
    goto cleanup;
  if (rename(PATH_SHADOW_TMP, PATH_SHADOW))
    goto cleanup;
#endif

  if (buf)
    free(buf);
  return update_passwd(out);

cleanup:
  if (buf)
    free(buf);
#ifdef HAVE_SHADOW
  if (shadow_in)
    fclose(shadow_in);
  if (shadow_out)
    {
      fclose(shadow_out);
      unlink(PATH_SHADOW_TMP);
    }
#endif
  if (in)
    fclose(in);
  if (out)
    discard_passwd_lockfile(out);
  return AL_EPASSWD;
}

/* This is an internal function.  Its contract is to edit the passwd
 * file, changing the home directory field to homedir.
 */

int al__change_passwd_homedir(const char *username, const char *homedir)
{
  FILE *in = NULL, *out = NULL;
  int bufsize = 0, retval, len, i;
  char *ptr1, *buf;

  out = lock_passwd();
  if (!out)
    return AL_EPASSWD;
  in = fopen(PATH_PASSWD, "r");
  if (!in)
    {
      discard_passwd_lockfile(out);
      return AL_EPASSWD;
    }
  len = strlen(username);

  while ((retval = al__read_line(in, &buf, &bufsize)) == 0)
    {
      if (strncmp(username, buf, len) == 0 && buf[len] == ':')
	{
	  /* Skip to colon before homedir field (the fifth colon; we start
	   * at the first one and skip four more). */
	  for (ptr1 = buf + len, i = 0; ptr1 && i < 4;
	       ptr1 = strchr(ptr1 + 1, ':'), i++)
	      ;
	  if (!ptr1)
	    continue;
	  fwrite(buf, sizeof(char), ptr1 + 1 - buf, out);
	  fputs(homedir, out);
	  ptr1 = strchr(ptr1 + 1, ':');
	  if (ptr1)
	    fputs(ptr1, out);
	  fputs("\n", out);
	}
      else
	{
	  fputs(buf, out);
	  fputs("\n", out);
	}
    }
  if (bufsize)
    free(buf);
  fclose(in);
  if (retval == -1)
    {
      discard_passwd_lockfile(out);
      return AL_EPASSWD;
    }

  return update_passwd(out);
}

/* This is an internal function.  Its contract is to update the
 * encrypted password entry for username if record->passwd_added is
 * true, cryptpw is not NULL, and /etc/nocrack is not present.  It may
 * only return AL_SUCCESS or AL_ENOMEM.
 */

int al__update_cryptpw(const char *username, struct al_record *record,
		       const char *cryptpw)
{
  FILE *in, *out, *lock;
  int have_nocrack, status;

  /* Check whether we really want to update the password field. */
  have_nocrack = !access(PATH_NOCRACK, F_OK);
  if (!record->passwd_added || cryptpw == NULL || have_nocrack)
    return AL_SUCCESS;

  /* Obtain a lock on the passwd file. */
  lock = lock_passwd();
  if (!lock)
    return AL_SUCCESS;

  /* Open the input and output files.  We want to update the shadow file
   * if there is one and the passwd file if there isn't. */
#ifdef HAVE_SHADOW
  out = fopen(PATH_SHADOW_TMP, "w");
  if (out)
    {
      fchmod(fileno(out), S_IWUSR|S_IRUSR|S_IRGRP);
      in = fopen(PATH_SHADOW, "r");
      if (!in)
	fclose(out);
    }
#else
  out = lock;
  in = fopen(PATH_PASSWD, "r");
#endif
  if (!out || !in)
    {
      discard_passwd_lockfile(lock);
      return AL_SUCCESS;
    }

  status = copy_changing_cryptpw(in, out, username, cryptpw);

  fclose(in);
#ifdef HAVE_SHADOW
  status = ferror(out) || status;
  if (fclose(out) || status)
    rename(PATH_SHADOW_TMP, PATH_SHADOW);
  else
    unlink(PATH_SHADOW_TMP);
  discard_passwd_lockfile(lock);
#else
  if (status)
    update_passwd(lock);
  else
    discard_passwd_lockfile(lock);
#endif
  return AL_SUCCESS;
}

/* Copy lines from a passwd, master.passwd, or shadow file from in to out,
 * changing the encrypted passwd field for username to cryptpw.  Return 1
 * if we found and changed one or more lines matching username, 0 if we
 * found none, and -1 if we detected an error.
 */

static int copy_changing_cryptpw(FILE *in, FILE *out, const char *username,
				 const char *cryptpw)
{
  char *line;
  int linesize = 0, len = strlen(username), found = 0, status;
  const char *p;

  /* Copy input to output, substituting cryptpw for the passwd field of
   * a line matching username. */
  while ((status = al__read_line(in, &line, &linesize)) == 0)
    {
      if (strncmp(username, line, len) == 0 && line[len] == ':')
	{
	  /* Find the field after the encrypted password. */
	  p = strchr(line + len + 1, ':');
	  if (!p)
	    continue;
	  fprintf(out, "%s:%s%s\n", username, cryptpw, p);
	  found = 1;
	}
      else
	fprintf(out, "%s\n", line);
    }

  if (linesize)
    free(line);
  return (status == -1) ? -1 : found;
}
