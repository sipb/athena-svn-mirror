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

/* Fill in elements of Athena login session */

long
ALsetUser(ALsession session, char *uname, ALflag_t initial_flags)
{
  /* make com_err work */
  initialize_ale_error_table();
  initialize_alw_error_table();

  memset(session, 0, sizeof(*session));
  session->flags = initial_flags;
  /* set flags according to noattach, nocreate, noremote, nologin */
  if (ALfileExists(ALfileNOLOGIN)) ALflagSet(session, ALhaveNOLOGIN);
  if (ALfileExists(ALfileNOCREATE)) ALflagSet(session, ALhaveNOCREATE);
  if (ALfileExists(ALfileNOREMOTE)) ALflagSet(session, ALhaveNOREMOTE);
  if (ALfileExists(ALfileNOATTACH)) ALflagSet(session, ALhaveNOATTACH);

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
