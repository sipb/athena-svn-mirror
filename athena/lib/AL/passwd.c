/**********************************************************************
 *  passwd.c -- add/remove user from passwd file
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <AL/AL.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* XXX do reference count */
long ALincRefCount(ALsession session)
{ return 0L; }

/* Warning to SYSV users: The Irix sigset(2) man page claims you
   should never use POSIX signal handling "with" sigset(). Right now,
   we're using sigaction() generally when we can. But it seems that
   lckpwdf() calls sigset(). I don't know if this really causes
   lossage for anyone, but in case you get weird signal problems, this
   is once place to look. */

int ALlockPasswdFile(ALsession session)
{
#ifdef SYSV
  return lckpwdf();
#else
  return ALopenLockFile(session, ALlockPASSWD);
#endif
}

int ALunlockPasswdFile(ALsession session)
{
#ifdef SYSV
  return ulckpwdf();
#else
  return ALcloseLockFile(session, ALlockPASSWD);
#endif
}

/* set error context to contents of a file */
void
ALgetErrorContextFromFile(ALsession session, char *filename)
{
  int fd, cnt;

  fd = open(filename, O_RDONLY);
  if (fd < 0)
    {
      ALcontext(session)[0]='\0';
    }
  else
    {
      /* put contents of NOCREATE file in error context */
      cnt = read(fd, ALcontext(session), ALlengthErrContext);

      /* null-terminate the string */
      if (cnt < ALlengthErrContext) ALcontext(session)[cnt] = '\0';
      else ALcontext(session)[ALlengthErrContext] = '\0';
    }
  return;
}

/* Add user to passwd file */
/* This code adds you to the password file even if you're already
   there. However, it's typically only called if you weren't there
   when we last looked. But there's a race condition. To correct the
   problem, we need to scan the password file in this routine, after
   locking it, to see if the user is there, and then add if not. Of
   course, implementing refcounts would also fix this problem.

   Note that our static pwd structure when this routine is called
   was obtained from hes_getpwnam, so we can safely call getpwnam
   without fear of clobbering it. Otherwise, we'd need to make our
   own copy of structure, which we may want to do anyway. */
long
ALaddPasswdEntry(ALsession session)
{
  long code;
  FILE *pfile;
  int cnt, fd, errnocopy;
#ifdef SYSV
  long lastchg = DAY_NOW;
#endif

  /* inc refcount */
  if ((code=ALincRefCount(session)) != 0L) return(code);

  /* if we didn't find user via hesiod, we're done */
  if (!ALisTrue(session, ALdidGetHesiodPasswd)) return 0L;

  /* if NOCREATE is set, return an error */
  if (ALisTrue(session, ALhaveNOCREATE))
    {
      ALgetErrorContextFromFile(session, ALfileNOCREATE);
      return(ALerrNOCREATE);
    }

  if (ALisTrue(session, ALisRemoteSession) && ALisTrue(session, ALhaveNOREMOTE))
    {
      ALgetErrorContextFromFile(session, ALfileNOREMOTE);
      return(ALerrNOREMOTE);
    }

  /* lock password file */
  /* Note that our traditional login code adds you to the password
     file and continues regardless of whether a lock was
     obtained. AL code currently fails out. */

  if (ALlockPasswdFile(session) == -1)
    ALreturnError(session, ALerrNoLock, "for passwd insert");

  /* append correct line */
  if((pfile=fopen(PASSWD, "a")) != NULL)
    {
      fprintf(pfile, "%s:%s:%d:%d:%s:%s:%s\n",
	      ALpw_name(session),
	      ALisTrue(session, ALhaveSHADOW) ? "x" :
	        ALisTrue(session, ALhaveNOCRACK) ? "*" :
	          ALpw_passwd(session),
	      ALpw_uid(session),
	      ALpw_gid(session),
	      ALpw_gecos(session),
	      ALpw_dir(session),
	      ALpw_shell(session));
      fclose(pfile);
    }
  else
    {
      errnocopy = errno;
      ALunlockPasswdFile(session);
      ALreturnError(session, (long) errnocopy, "appending to /etc/passwd");
    }

#ifdef SHADOW
  if (ALisTrue(session, ALhaveSHADOW))
    {
      if((pfile=fopen(SHADOW, "a")) != NULL)
	{
	  fprintf(pfile,"%s:%s:%d::::::\n",
		  ALpw_name(session),
		  ALpw_passwd(session),
		  lastchg);
	  fclose(pfile);
	}
      else
	{
	  errnocopy = errno;
	  ALunlockPasswdFile(session);
	  ALreturnError(session, (long) errnocopy, "appending to /etc/shadow");
	}
    }
#endif

  ALunlockPasswdFile(session);

  /* printf("Added %s to /etc/passwd\n", ALpw_dir(session)); */
  return 0L;
}

long
ALremovePasswdEntry(ALsession session)
{
  long code;

  /* XXX should decrement refcount */
  /* if we didn't find user via hesiod, we're done */
  if (!ALisTrue(session, ALdidGetHesiodPasswd)) return 0L;

  if (ALlockPasswdFile(session) == -1)
    ALreturnError(session, ALerrNoLock, "for passwd remove");

  /* delete user's entry */
  code = ALmodifyLinesOfFile(session, PASSWD, ALlockPASSWD,
			     ALmodifyRemoveUser, ALappendNOT);
#ifdef SHADOW
  if (!code)
    if (ALisTrue(session, ALhaveSHADOW))
      code = ALmodifyLinesOfFile(session, SHADOW, ALlockSHADOW,
				 ALmodifyRemoveUser, ALappendNOT);
#endif

  ALunlockPasswdFile(session);
  return(code);
}
