/**********************************************************************
 *  acctref.c -- reference counts for account creation
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <unistd.h>
#include <stdio.h>

/* lockfilename is something like "/etc/ptmp" */

long
ALlockfile(char *lockfilename)
{
  /* if lock file exists but is old, unlink it */
  /* try to create lock file */

  return 0L;
}

long
ALunlockfile(char *lockfilename)
{
  /* unlink lock file */

  return 0L;
}

long
ALaddRef(ALsession session)
{
  /* don't addRef twice */
  /* remember we've addRef'ed this session */
  /* lock ref file */
  /* find ref for user */
  /* if user is in password file but has no ref, do nothing */
  /* add my pid to ref list */
  /* unlock ref file */
  return 0L;
}

long
ALdelRef(ALsession session)
{
  /* lock ref file */
  /* find ref for user */
  /* if no ref, do nothing */
  /* check validity of other pids which ref user */
  /* remove invalid pids */
  /* remove my pid */
  /* unlock ref file */
  return 0L;			/* AL_WLASTREF */
}

/* ALremove("/etc/passwd", "/etc/ptmp", "brlewis:") will remove
 * any line from /etc/passwd that begins with "brlewis:"
 */

ALremoveLineFromFile(ALsession session,
		     char *filename,
		     char *lockfilename,
		     char *prefix)
{
  long code;
  int cnt, fd;
  FILE *oldfile, *lockfile;
  char buf[1024];

  /* open lock file */
  for (cnt = 10; cnt >= 0; cnt--)
    {
      if ((fd = open(lockfilename, O_WRONLY|O_CREAT|O_EXCL, 0644)) > 0) break;
      sleep(1);
    }
  if (fd < 0) ALreturnError(session, ALerrNoLock, lockfilename);

  /* open file to read from */
  if ((oldfile = fopen(filename, "r"))== (FILE *)0)
    ALreturnError(session, (long) errno, filename);

  /* copy appropriate lines */
  len = strlen(prefix);
  while (fgets(buf, 1024, oldfile) != (char*)0)
    {
      if (strncmp(buf, prefix, len))
	{
	  /* write line to new file */
	  cnt = write(fd, buf, strlen(buf));
	  if (cnt < 0) goto WRITE_ERROR;
	}
    }
  if (close(fd) < 0) goto WRITE_ERROR;

  if (rename(lockfilename, filename) < 0) goto WRITE_ERROR;

  return 0L;

 WRITE_ERROR:
  code = (long) errno;
  unlink(lockfilename);
  ALreturnError(session, code, lockfilename);
}
