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

int
ALopenLockFile(char *filename)
{
  int cnt, fd;

  for (cnt = 10; cnt >= 0; cnt--)
    {
      if ((fd = open(filename, O_WRONLY|O_CREAT|O_EXCL, 0644)) > 0) break;
      sleep(1);
    }
  return fd;
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

/* ALmodifyLinesOfFile() is a generalized way to lock a file
 * and remove/modify all lines or a specific line
 * e.g. removing any line beginning with "brlewis:" from /etc/passwd
 */

long
ALmodifyLinesOfFile(ALsession session,
		    char *filename,
		    char *lockfilename,
		    long (*modify)(ALsession, char[]),
		    long (*append)(ALsession, int))
{
  long code=0L;
  int cnt, fd= -1;
  FILE *oldfile=(FILE *)0;
  char buf[1024];
  char *ctxt;			/* error context */

  /* open lock file */
  fd = ALopenLockFile(lockfilename);
  if (fd < 0) ALreturnError(session, ALerrNoLock, lockfilename);

  /* open file to read from */
  if ((oldfile = fopen(filename, "r"))== (FILE *)0)
    ALreturnError(session, (long) errno, filename);

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
      if (cnt < 0) { ctxt=lockfilename; goto CLEANUP; }
    }

  /* close file to read */
  if (fclose(oldfile)==EOF) { ctxt=filename; goto CLEANUP; }
  oldfile=(FILE *)0;

  /* append if necessary */
  if (append)
    {
      code = append(session, fd);
      if (code) { ctxt = lockfilename; goto CLEANUP; }
    }

  /* close lockfile */
  if (close(fd) < 0) { ctxt=lockfilename; goto CLEANUP; }
  fd= -1;

  if (rename(lockfilename, filename) < 0) { ctxt=filename; goto CLEANUP; }

  return 0L;

 CLEANUP:
  if (!code) code = (long) errno;
  if (fd>=0) close(fd);
  if (oldfile) fclose(oldfile);
  unlink(lockfilename);
  ALreturnError(session, code, ctxt);
}
