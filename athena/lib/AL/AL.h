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
#include <utmp.h>
#include <limits.h>
#ifdef SYSV
#include <shadow.h>
#include <utmpx.h>
#endif
#include <sys/types.h>
#include <AL/etale.h>
#include <AL/etalw.h>

#ifndef PASSWD
#define PASSWD "/etc/passwd"
#endif
#ifndef PASSTEMP
#define PASSTEMP "/etc/ptmp"
#define SHADTEMP "/etc/stmp"
#endif

#define ATHENA_NGROUPS_MAX 16

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

#define ALlockPASSWD 0
#define ALlockSHADOW 1
#define ALlockGROUP 2
#define ALlockTEST 3
#define ALlockMAX 4

typedef struct _ALsession
{
  ALflag_t flags;
  struct passwd *pwd;		/* don't use directly; use macros */
  char context[ALlengthErrContext]; /* more information about error */
  pid_t attach_pid;
  int lock_fd[ALlockMAX];
  int ngroups;
  ALgroup groups;

  ALflag_t utFlags;
  short ut_type;
#ifdef UTMPX_FILE
  struct utmpx ut;
#else
  struct utmp ut;
#endif
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

/* utmp handling */

typedef struct _ALut
{
  char *user;
  char *host;
  char *line;
  char *id;
  short type;
} ALut;

/* utmp flags for copying */
#define ALutUSER	((ALflag_t)(1<<0))
#define ALutHOST	((ALflag_t)(1<<1))
#define ALutLINE	((ALflag_t)(1<<2))
#define ALutID		((ALflag_t)(1<<3))
#define ALutTYPE	((ALflag_t)(1<<4))

#define ALut_flags(session)		((session)->utFlags)
#define ALutFlagClear(session, flag)	((session)->utFlags &= ~(flag))
#define ALutFlagSet(session, flag)	((session)->utFlags |= (flag))
#define ALutIsSet(session, flag)	((session)->utFlags & (flag))

#ifdef LOGIN_PROCESS
#define ALutLOGIN_PROC		LOGIN_PROCESS
#define ALutUSER_PROC		USER_PROCESS
#define ALutDEAD_PROC		DEAD_PROCESS
#else
#define ALutLOGIN_PROC		1
#define ALutUSER_PROC		2
#define ALutDEAD_PROC		3
#endif

/* all platforms */
#define ALut_line(session)	((session)->ut.ut_line)
#define ALut_user(session)	((session)->ut.ut_name)
#define ALut_host(session)	((session)->ut.ut_host)
#define ALut_time(session)	((session)->ut.ut_time) /* !SYSV */

/* RS/6000 and SYSV */
#define ALut_id(session)	((session)->ut.ut_id)
#define ALut_type(session)	((session)->ut.ut_type)
#define ALut_pid(session)	((session)->ut.ut_pid)

/* SYSV only */
#define ALut_syslen(session)	((session)->ut.ut_syslen)
#define ALut_session(session)	((session)->ut.ut_session)
#define ALut_tv(session)	((session)->ut.ut_tv)

/* minor hack */
#define ALut_auxtype(session)	((session)->ut_type)

/* other ALsession macros */

#define ALcontext(s)		((s)->context)
#define ALattach_pid(s)		((s)->attach_pid)
#define ALlock_fd(s,n)		((s)->lock_fd[n])

/* macros for manipulating flags */

#define ALflagClear(session, flag)	((session)->flags &= ~(flag))
#define ALflagSet(session, flag)	((session)->flags |= (flag))
#define ALisTrue(session, flag)		((session)->flags & (flag))
#define ALflagNone	((ALflag_t)0)
#define ALflagAll	((ALflag_t)LONG_MAX)

#define ALfileNOLOGIN	"/etc/nologin"
#define ALfileNOCREATE	"/etc/nocreate"
#define ALfileNOREMOTE	"/etc/noremote"
#define ALfileNOATTACH	"/etc/noattach"
#define ALfileNOCRACK	"/etc/nocrack"

#define ALfileExists(f) (access((f), F_OK) == 0)

/* flags - may be reordered after the list is more complete */

#define ALhaveNOLOGIN	((ALflag_t)(1<<0))
#define ALhaveNOCREATE	((ALflag_t)(1<<1))
#define ALhaveNOREMOTE	((ALflag_t)(1<<2))
#define ALhaveNOATTACH	((ALflag_t)(1<<3))
#define ALhaveNOCRACK	((ALflag_t)(1<<4))
#define ALhaveSHADOW	((ALflag_t)(1<<5))

#define ALdidGetHesiodPasswd	((ALflag_t)(1<<8))
#define ALdidAttachHomedir	((ALflag_t)(1<<9))
#define ALdidCreateHomedir	((ALflag_t)(1<<10))

#define ALisRemoteSession	((ALflag_t)(1<<11))
#define ALhaveAuthentication	((ALflag_t)(1<<12)) /* if we have krb tkts */

/* how library functions return an error */

#define ALreturnError(session, code, string) \
  { strncpy((session)->context, string, ALlengthErrContext-1); \
    (session)->context[ALlengthErrContext-1]='\0'; \
    return(code); }

#define ALisWarning(code)	((code) >= ALwarnOK && (code) <= ALwarnMax)
#define ALisError(code)		((code) && !ALisWarning((code)))

/* for ALmodifyLinesOfFile() */
#define ALmodifyNOT ((long (*)(ALsession, char[]))0)
#define ALappendNOT ((long (*)(ALsession, int))0)

/* function prototypes */
#include <AL/ptypes.h>

#endif /* _AL_AL_H */
