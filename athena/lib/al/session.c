#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "al.h"
#include "al_private.h"

/* These variables are to be treated as constants except by
 * test programs.
 */

char *al__session_dir = PATH_SESSIONS;


/* This is an internal function.  Its contract is to zero out the
 * informational fields in a record structure.
 */
void zero_record(struct al_record *r)
{
  r->exists = r->passwd_added = r->attached = r->ngroups = r->npids = 0;
  r->old_homedir = NULL;
  r->groups = NULL;
  r->pids = NULL;
}

/* This is an internal function.  Its contract is to open the session
 * record, lock it, and parse its contents into record.
 */
int al__get_session_record(const char *username,
			   struct al_record *record)
{
  int fd, bufsize = 0, retval = AL_SUCCESS, i;
  char *session_file, *buf = NULL, *ptr1;
  struct flock fl;
  sigset_t smask;

  /* No POSIX limit on username size; allocate space for filename. */
  session_file = malloc(strlen(al__session_dir) + strlen(username) + 2);
  if (!session_file)
    return AL_ENOMEM;
  sprintf(session_file, "%s/%s", al__session_dir, username);

  /* Zero the fields that correspond to state saved on disk. */
  zero_record(record);

  /* Open and lock the session record. */
  fd = open(session_file, O_CREAT|O_RDWR, 0600);
  if (fd == -1)
    return AL_ESESSION;
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fcntl(fd, F_SETLKW, &fl);
  record->fp = fdopen(fd, "r+");

  /* Get the first line (0 or 1; passwd created). */
  switch (al__read_line(record->fp, &buf, &bufsize))
    {
    case -1:			/* error */
      retval = AL_ESESSION;
      goto cleanup;

    case 1:			/* EOF */
      return AL_SUCCESS;

    default:			/* got line */
      if (!strcmp(buf, "0") || !strcmp(buf, "1"))
	record->passwd_added = buf[0] - '0';
      else
	return AL_WBADSESSION;
      break;
    }

  /* Get the second line (0 or 1; homedir attached). */
  switch (al__read_line(record->fp, &buf, &bufsize))
    {
    case -1:			/* error */
      retval = AL_ESESSION;
      goto cleanup;

    case 1:			/* EOF */
      zero_record(record);
      return AL_WBADSESSION;

    default:			/* got line */
      if (!strcmp(buf, "0") || !strcmp(buf, "1"))
	record->attached = buf[0] - '0';
      else
	{
	  zero_record(record);
	  return AL_WBADSESSION;
	}
      break;
    }

  /* Get the third line (0 or 1old_homedir). */
  switch (al__read_line(record->fp, &buf, &bufsize))
    {
    case -1:			/* error */
      retval = AL_ESESSION;
      goto cleanup;

    case 1:			/* EOF */
      zero_record(record);
      return AL_WBADSESSION;

    default:			/* got line */
      if (!strcmp(buf, "0") || (buf[0] == '1' && strlen(buf) > 1))
	{
	  record->attached = buf[0] - '0';
	  if (record->attached)
	    {
	      record->old_homedir = strdup(buf + 1);
	      if (!record->old_homedir)
		{
		  retval = AL_ESESSION;
		  goto cleanup;
		}
	    }
	}
      else
	{
	  zero_record(record);
	  return AL_WBADSESSION;
	}
      break;
    }

  /* Get the fourth line (gid1:gid2:...gidn:). */
  switch (al__read_line(record->fp, &buf, &bufsize))
    {
    case -1:			/* error */
      retval = AL_ESESSION;
      goto cleanup;

    case 1:			/* EOF */
      zero_record(record);
      return AL_WBADSESSION;

    default:			/* got line */
      /* Make sure it's a list of numbers each followed by a colon. */
      ptr1 = buf;
      record->ngroups = 0;
      while (*ptr1)
	{
	  if (!isdigit(*ptr1) || NULL == (ptr1 = strchr(ptr1, ':')))
	    {
	      retval = AL_WBADSESSION;
	      goto cleanup;
	    }
	  record->ngroups++;
	  ptr1++;
	}
      /* Parse the numbers into an array of gid_t. */
      record->groups = malloc(record->ngroups * sizeof(gid_t));
      if (!record->groups)
	{
	  retval = AL_ESESSION;
	  goto cleanup;
	}
      i = 0;
      for (ptr1 = buf; *ptr1; ptr1 = 1 + strchr(ptr1, ':'))
	record->groups[i++] = atoi(ptr1);
    }

  /* Get the fifth line (pid1:pid2:...pidn:). */
  switch (al__read_line(record->fp, &buf, &bufsize))
    {
    case -1:			/* error */
      retval = AL_ESESSION;
      goto cleanup;

    case 1:			/* EOF */
      zero_record(record);
      return AL_WBADSESSION;

    default:			/* got line */
      /* Make sure it's a list of numbers each followed by a colon. */
      ptr1 = buf;
      record->npids = 0;
      while (*ptr1)
	{
	  if (!isdigit(*ptr1) || NULL == (ptr1 = strchr(ptr1, ':')))
	    {
	      retval = AL_WBADSESSION;
	      goto cleanup;
	    }
	  record->npids++;
	  ptr1++;
	}
      /* Parse the numbers into an array of pid_t. */
      record->pids = malloc(record->npids * sizeof(pid_t));
      if (!record->pids)
	{
	  retval = AL_ESESSION;
	  goto cleanup;
	}
      i = 0;
      for (ptr1 = buf; *ptr1; ptr1 = 1 + strchr(ptr1, ':'))
	record->pids[i++] = atoi(ptr1);
    }

  /* Block signals that might kill process while record info on disk
   * doesn't match reality.
   */
  sigemptyset(&smask);
  sigaddset(&smask, SIGHUP);
  sigaddset(&smask, SIGINT);
  sigaddset(&smask, SIGQUIT);
  sigaddset(&smask, SIGTSTP);
  sigaddset(&smask, SIGALRM);
  sigaddset(&smask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &smask, &(record->mask));

  record->exists = 1;
  return AL_SUCCESS;

cleanup:
  if (retval == AL_ESESSION)
    {
      /* Relinquish the lock in case this OS violates POSIX.1 B.6.5.2
       * by not automatically relinquishing it when the fd is closed.
       */
      fl.l_type = F_UNLCK;
      fcntl(fileno(record->fp), F_SETLKW, &fl);

      if (record->fp)
	fclose(record->fp);
      else
	close(fd);
    }

  if (record->old_homedir)
    free(record->old_homedir);
  if (record->groups)
    free(record->groups);
  if (record->pids)
    free(record->pids);
  zero_record(record);
  return retval;
}

/* This is an internal function.  Its contract is to write out a new
 * session record according to what's in record, drop the fcntl lock,
 * and close the file descriptor.
 */
int al__put_session_record(struct al_record *record)
{
  int i;
  struct flock fl;

  rewind(record->fp);

  if (record->exists)
    {
      fprintf(record->fp, "%d\n%d\n%d%s\n",
	      record->passwd_added, record->attached,
	      (record->old_homedir != NULL),
	      (record->old_homedir != NULL) ? record->old_homedir : "");
      for (i = 0; i < record->ngroups; i++)
	fprintf(record->fp, "%lu:", (unsigned long) record->groups[i]);
      fputs("\n", record->fp);
      for (i = 0; i < record->npids; i++)
	fprintf(record->fp, "%lu:", (unsigned long) record->pids[i]);
      fputs("\n", record->fp);
      ftruncate(fileno(record->fp), ftell(record->fp));
    }
  else
    ftruncate(fileno(record->fp), 0);

  /* Relinquish the lock in case this OS violates POSIX.1 B.6.5.2
   * by not automatically relinquishing it when the fd is closed.
   */
  fl.l_type = F_UNLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;
  fcntl(fileno(record->fp), F_SETLKW, &fl);

  fclose(record->fp);

  if (record->old_homedir)
    free(record->old_homedir);
  if (record->groups)
    free(record->groups);
  if (record->pids)
    free(record->pids);

  /* Restore the signal mask in record->mask. */
  sigprocmask(SIG_SETMASK, &(record->mask), 0);

  return AL_SUCCESS;
}
