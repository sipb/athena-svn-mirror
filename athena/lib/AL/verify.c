/**********************************************************************
 *  verify.c -- verify username and password
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include "al.h"
#include <unistd.h>
#include <pwd.h>

/* Fill in elements of Athena login session */

long
ALsetUser(ALsession session, char *uname)
{
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
      session->pwd = hes_getpwnam(uname);
      
  /* if uid is 0, no reason to disallow login */

  return 0L;
}

long
ALprePassword(ALsession session, char *uname)
{
  /* request kerberos tickets */

  return 0L;
}

long
ALverifyPassword(ALsession session, char *password)
{
  /* verify password with kerberos */
  /* verify password locally */
  /* destroy all traces of password */

  return 0L;
}
