/**********************************************************************
 *  al.h -- header file for athena login library
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#include <krb.h>	
#include <pwd.h>
#include <utmp.h>
#ifdef SOLARIS
#include <utmpx.h>
#include <shadow.h>
#endif

#ifdef SOLARIS
#define AL_NMAX	sizeof(utmpx.ut_name)
#define AL_HMAX	sizeof(utmpx.ut_host)
#else
#define AL_NMAX	sizeof(utmp.ut_name)
#define AL_HMAX	sizeof(utmp.ut_host)
#endif

typedef struct _ALsession
{
  long flags;
  struct passwd pwd;
  char	rusername[AL_NMAX+1], lusername[AL_NMAX+1];
  char	rpassword[AL_NMAX+1];
  char	name[AL_NMAX+1];
  char	*rhost;
  AUTH_DAT *kdata;
}

#define AL_FKRB  (1<<0)		/* True if Kerberos-authenticated login */
#define AL_FPAG  (1<<1)		/* True if we call setpag() */
#define AL_FTMPP (1<<2)		/* True if passwd entry is temporary */
#define AL_FTMPD (1<<3)		/* True if home directory is temporary */
#define AL_FINH  (1<<4)		/* inhibit account creation on the fly */
#define AL_FATOK (1<<5)		/* True if /etc/noattach doesn't exist */
#define AL_FATD  (1<<6)		/* True if homedir attached */
#define AL_FERPT (1<<7)		/* True if login error already printed */
#define AL_FNRMT (1<<8)		/* True if /etc/noremote exists */
