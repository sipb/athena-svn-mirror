/**********************************************************************
 *  al.h -- header file for athena login library
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#ifndef _AL_AL_H
#define _AL_AL_H

#include <krb.h>	
#include <pwd.h>
#include <sys/types.h>
#include <AL/etale.h>
#include <AL/etalw.h>

#define ALtempDotfiles		"/usr/athena/lib/prototype_tmpuser/."
#define ALtempDirPerm		(0710)
#define ALlengthErrContext	4096

typedef long ALflag_t;

typedef struct _ALgroup
{
  char gname[64];
  char gid[16];
  int gadded;			/* 0 if not added to /etc/group yet */
} ALgroupStruct, *ALgroup;

typedef struct _ALsession
{
  ALflag_t flags;
  struct passwd *pwd;		/* don't use directly; use macros */
  char context[ALlengthErrContext]; /* more information about error */
  pid_t attach_pid;
  int ngroups;
  ALgroup groups;
} ALsessionStruct, *ALsession;

/* macros to access passwd entry of ALsession */

#define ALpw_name(session)	((session)->pwd->pw_name)
#define ALpw_passwd(session)	((session)->pwd->pw_passwd)
#define ALpw_uid(session)	((session)->pwd->pw_uid)
#define ALpw_gid(session)	((session)->pwd->pw_gid)
#define ALpw_gecos(session)	((session)->pwd->pw_gecos)
#define ALpw_dir(session)	((session)->pwd->pw_dir)
#define ALpw_shell(session)	((session)->pwd->pw_shell)

/* group macros */

#define ALgroupName(s, n)	((s)->groups[(n)].gname)
#define ALgroupId(s, n)		((s)->groups[(n)].gid)
#define ALgroupAdded(s, n)	((s)->groups[(n)].gadded)
#define ALngroups(s)		((s)->ngroups)

/* other ALsession macros */

#define ALcontext(s)		((s)->context)
#define ALattach_pid(s)		((s)->attach_pid)

/* macros for manipulating flags */

#define ALflagClear(session, flag)	((session)->flags &= ~(flag))
#define ALflagSet(session, flag)	((session)->flags |= (flag))
#define ALisTrue(session, flag)		((session)->flags & (flag))
#define ALflagNone	((ALflag_t)0)

#define ALfileNOLOGIN	"/etc/nologin"
#define ALfileNOCREATE	"/etc/nocreate"
#define ALfileNOREMOTE	"/etc/noremote"
#define ALfileNOATTACH	"/etc/noattach"

#define ALfileExists(f) (access((f), F_OK) == 0)

/* flags - may be reordered after the list is more complete */

#define ALhaveNOLOGIN	((ALflag_t)(1<<0))
#define ALhaveNOCREATE	((ALflag_t)(1<<1))
#define ALhaveNOREMOTE	((ALflag_t)(1<<2))
#define ALhaveNOATTACH	((ALflag_t)(1<<3))

#define ALdidGetHesiodPasswd	((ALflag_t)(1<<4))
#define ALdidAttachHomedir	((ALflag_t)(1<<5))
#define ALdidCreateHomedir	((ALflag_t)(1<<6))

#define ALisRemoteSession	((ALflag_t)(1<<7))
#define ALhaveAuthentication	((ALflag_t)(1<<8)) /* if we have krb tkts */

/* how library functions return an error */

#define ALreturnError(session, code, string) { strncpy((session)->context, string, ALlengthErrContext-1); (session)->context[ALlengthErrContext-1]='\0'; return(code); }

#define ALisWarning(code)	((code) >= ALwarnOK && (code) <= ALwarnMax)
#define ALisError(code)		((code) && !ALisWarning((code)))

/* for ALmodifyLinesOfFile() */
#define ALmodifyNOT ((long (*)(ALsession, char[]))0)
#define ALappendNOT ((long (*)(ALsession, int))0)

/* function prototypes */
#include <AL/ptypes.h>

#endif /* _AL_AL_H */
