/**********************************************************************
 *  passwd.c -- add/remove user from passwd file
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include "al.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#ifdef SOLARIS
#include <shadow.h>
#endif

/* XXX do reference count */
long ALincRefCount(ALsession session) { return 0L; }

/* Add user to passwd file */

long
ALaddPasswdEntry(ALsession session)
{
  long code;
  FILE *pfile;
  int cnt, fd;
#ifdef SOLARIS
  long lastchg = DAY_NOW;
#endif

  /* inc refcount */
  if ((code=ALincRefCount(session)) != 0L) return(code);

  /* if we didn't find user via hesiod, we're done */
  if (!ALisTrue(session, ALdidGetHesiodPasswd)) return 0L;

  /* if NOCREATE is set, return an error */
  if (ALisTrue(session, ALhaveNOCREATE))
    {
      fd = open(ALfileNOCREATE, O_RDONLY);
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
      return(ALerrNOCREATE);
    }

  /* lock password file */
  for (cnt=10; cnt >= 0; cnt--)
    {
      if ((fd = open("/etc/ptmp", O_WRONLY|O_CREAT|O_EXCL, 0644)) > 0) break;
      sleep(1);
    }
  if (fd < 0) ALreturnError(session, ALerrNoLock, "/etc/ptmp for insert");

  /* append correct line */
#ifndef SOLARIS
    if((pfile=fopen("/etc/passwd", "a")) != NULL) {
        fprintf(pfile, "%s:%s:%d:%d:%s:%s:%s\n",
                ALpw_name(session),
                ALpw_passwd(session),
                ALpw_uid(session),
                ALpw_gid(session),
                ALpw_gecos(session),
                ALpw_dir(session),
                ALpw_shell(session));
        fclose(pfile);
    }
#else
   if((pfile=fopen("/etc/shadow", "a")) != NULL) {
   fprintf(pfile,"%s:%s:%d::::::\n",
            ALpw_name(session),
            ALpw_passwd(session),
            lastchg);
        fclose(pfile);
    }
    if((pfile=fopen("/etc/passwd", "a")) != NULL) {
        fprintf(pfile, "%s:%s:%d:%d:%s:%s:%s\n",
                ALpw_name(session),
                "x",
                ALpw_uid(session),
                ALpw_gid(session),
                ALpw_gecos(session),
                ALpw_dir(session),
                ALpw_shell(session));
        fclose(pfile);
    }
#endif
    close(fd);
    unlink("/etc/ptmp");

  return 0L;
}
