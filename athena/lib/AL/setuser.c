/**********************************************************************
 *  setuser.c -- intialize Athena Login session structure
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <AL/AL.h>
#include <AL/etale.h>
#include <unistd.h>
#include <pwd.h>
#include <hesiod.h>
#include <string.h>
#ifdef SYSV
#include <shadow.h>
#endif

#if defined(sgi) && defined(SHADOW)
extern int _getpwent_no_shadow;
#endif

/* Fill in elements of Athena login session */
long
ALinitUser(ALsession session, ALflag_t initial_flags)
{
  int i;

  ALflagClear(session, ALflagAll);
  ALflagSet(session, initial_flags);

  /* set flags according to noattach, nocreate, noremote, nologin */
  if (ALfileExists(ALfileNOLOGIN)) ALflagSet(session, ALhaveNOLOGIN);
  if (ALfileExists(ALfileNOCREATE)) ALflagSet(session, ALhaveNOCREATE);
  if (ALfileExists(ALfileNOREMOTE)) ALflagSet(session, ALhaveNOREMOTE);
  if (ALfileExists(ALfileNOATTACH)) ALflagSet(session, ALhaveNOATTACH);
  if (ALfileExists(ALfileNOCRACK)) ALflagSet(session, ALhaveNOCRACK);

#ifdef SHADOW
  /* Under Irix, we need to support the shadow password file if and
     only if it alrady exists, so we set a flag to keep track of that
     information here. To keep the code cleaner elsewhere, we go ahead
     and set this flag regardless of whether we're using Irix. */
  if (ALfileExists(SHADOW))
    {
      ALflagSet(session, ALhaveSHADOW);
#ifdef sgi
      /* Irix tries to be helpful and return the password field from
	 /etc/shadow when getpwnam is used. Since other OS's don't do
	 this, it might be rather unexpected, so we shut off this
	 behavior to keep from shooting ourselves in the foot later. */
      _getpwent_no_shadow = 1;
#endif
    }
#endif

  for (i = 0; i < ALlockMAX; i++)
    ALlock_fd(session, i) = -1;

  return 0L;
}

long
ALsetUser(ALsession session, char *uname)
{
  /* find user in passwd file */
  session->pwd = getpwnam(uname);

#ifndef NO_HESIOD
  if (!(session->pwd))
    {
      /* get passwd information from hesiod */
      if (!(session->pwd = hes_getpwnam(uname)))
	ALreturnError(session, ALerrUserUnknown, uname);

      /* disallow hesiod entries with uid==0 */
      if (!ALpw_uid(session))
	ALreturnError(session, ALerrIllegalHesiod, uname);

      /* Remember that we got info through hesiod */
      ALflagSet(session, ALdidGetHesiodPasswd);
    }
#endif /* NO_HESIOD */

  return 0L;
}
