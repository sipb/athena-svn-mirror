/**********************************************************************
 *  setuser.c -- intialize Athena Login session structure
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include "al.h"
#include "etale.h"
#include <unistd.h>
#include <pwd.h>
#include <hesiod.h>

/* Fill in elements of Athena login session */

long
ALsetUser(ALsession session, char *uname, ALflag_t initial_flags)
{
  /* make com_err work */
  initialize_ale_error_table();

  session->flags = initial_flags;
  /* set flags according to noattach, nocreate, noremote, nologin */
  if (access(ALfileNOLOGIN, F_OK)==0) ALflagSet(session, ALhaveNOLOGIN);
  if (access(ALfileNOCREATE, F_OK)==0) ALflagSet(session, ALhaveNOCREATE);
  if (access(ALfileNOREMOTE, F_OK)==0) ALflagSet(session, ALhaveNOREMOTE);
  if (access(ALfileNOATTACH, F_OK)==0) ALflagSet(session, ALhaveNOATTACH);

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
