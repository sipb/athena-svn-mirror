/**********************************************************************
 *  start.c -- start Athena login session
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <AL/AL.h>

/* Start Athena login session */

long
ALstart(ALsession session, char *ttyname)
{
  long code;

  /* add user to passwd file */
  if ((code=ALaddPasswdEntry(session)) != 0L) return(code);

  /* add groups to groups file */
  if ((code=ALaddToGroupsFile(session)) != 0L) return(code);

  /* attach/make home directory */
  if ((code=ALgetHomedir(session)) != 0L) return(code);

  /* create utmp entry (if ttyname != NULL) */

  return 0L;
}

long ALend(ALsession session)
{
  ALremovePasswdEntry(session);

  /* remove groups from groups file */
  /* detach home directory */
  /* remove temporary directory */
  /* remove utmp entry */

  return 0L;
}
