/**********************************************************************
 *  modify.c -- functions to modify files
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <AL/AL.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

static char *lockFiles[ALlockMAX] =
	{ PASSTEMP, SHADTEMP, "/etc/gtmp", "testlock" };

char *
ALlockFile(int lockfile)
{
  if (lockfile < 0 || lockfile > ALlockMAX)
    return NULL;

  return lockFiles[lockfile];
}

int
ALopenLockFile(ALsession session, int lockfile)
{
  int cnt;

  if (lockfile < 0 || lockfile > ALlockMAX)
    return -1;

  if (ALlock_fd(session, lockfile) == -1)
    for (cnt = 10; cnt >= 0; cnt--)
      {
	if ((ALlock_fd(session, lockfile) = open(ALlockFile(lockfile),
						 O_WRONLY|O_CREAT|O_EXCL,
						 0600)) != -1)
	  break;
	sleep(1);
      }

  return ALlock_fd(session, lockfile);
}

int
ALcloseLockFile(ALsession session, int lockfile)
{
  if (lockfile < 0 || lockfile > ALlockMAX)
    return -1;

  if (ALlock_fd(session, lockfile) == -1)
    return -1;

  /* If close returns an error, we assume that the caller must have
     closed it (and therefore takes care of the rename/unlink for us),
     so we shouldn't unlink it - it might be someone else's lock by
     now. */
  if (-1 != close(ALlock_fd(session, lockfile)))
    unlink(ALlockFile(lockfile));

  ALlock_fd(session, lockfile) = -1;
  return 0;
}

/* Use ALmodifyRemoveUser() with ALmodifyLinesOfFile() to remove
 * a username from a file
 */

long
ALmodifyRemoveUser(ALsession session, char buf[])
{
  int len;

  /* if the line doesn't start with "username:", skip it */
  len = strlen(ALpw_name(session));
  if (strncmp(buf, ALpw_name(session), len) || buf[len] != ':')
    return 0L;

  /* It does start with "username:", so remove it */
  buf[0] = '\0';
  return 0L;
}

long ALmodifyAppendPasswd(ALsession session, int fd)
{
  char buf[256];

  sprintf(buf, "%s:x:%d:%d:%s:%s:%s\n",
	  ALpw_name(session),
	  ALpw_uid(session),
	  ALpw_gid(session),
	  ALpw_gecos(session),
	  ALpw_dir(session),
	  ALpw_shell(session));

  if (write(fd, buf, strlen(buf)) <0) return((long) errno);

  return 0L;
}

/* ALmodifyLinesOfFile() is a generalized way to lock a file
 * and remove/modify all lines or a specific line
 * e.g. removing any line beginning with "brlewis:" from /etc/passwd
 */

long
ALmodifyLinesOfFile(ALsession session,
		    char *filename,
		    int lockfile,
		    long (*modify)(ALsession, char[]),
		    long (*append)(ALsession, int))
{
  long code=0L;
  int cnt, fd= -1;
  FILE *oldfile=(FILE *)0;
  char buf[1024];
  struct stat stat_buf;
  char *ctxt;			/* error context */

  /* open lock file */
  fd = ALopenLockFile(session, lockfile);
  if (fd < 0) ALreturnError(session, ALerrNoLock, ALlockFile(lockfile));

  /* open file to read from */
  if ((oldfile = fopen(filename, "r"))== (FILE *)0)
    { ctxt=filename; goto CLEANUP; }

  /* get file protection mode of file to read */
  if (stat(filename, &stat_buf) < 0)
    { ctxt=filename; goto CLEANUP; }

  /* set lockfile protection */
  if (fchmod(fd, stat_buf.st_mode) < 0)
    { ctxt=ALlockFile(lockfile); goto CLEANUP; }

  /* modify the file, line by line */
  while (fgets(buf, 1024, oldfile) != (char*)0)
    {
      if (modify)
	{
	  code = modify(session, buf);
	  if (code) { ctxt=filename; goto CLEANUP; }
	}

      /* write line to new file */
      cnt = write(fd, buf, strlen(buf));
      if (cnt < 0) { ctxt=ALlockFile(lockfile); goto CLEANUP; }
    }

  /* close file to read */
  if (fclose(oldfile)==EOF) { ctxt=filename; goto CLEANUP; }
  oldfile=(FILE *)0;

  /* append if necessary */
  if (append)
    {
      code = append(session, fd);
      if (code) { ctxt = ALlockFile(lockfile); goto CLEANUP; }
    }

  /* close lockfile */
  if (close(fd) < 0) { ctxt=ALlockFile(lockfile); goto CLEANUP; }
  fd= -1;
  ALcloseLockFile(session, lockfile);

  if (rename(ALlockFile(lockfile), filename) < 0)
    { ctxt=filename; goto CLEANUP; }


  return 0L;

 CLEANUP:
  if (!code) code = (long) errno;
  if (fd>=0) close(fd);
  if (oldfile) fclose(oldfile);
  unlink(ALlockFile(lockfile));
  ALcloseLockFile(session, lockfile);
  ALreturnError(session, code, ctxt);
}
