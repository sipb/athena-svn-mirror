/***********************************************************
		Copyright IBM Corporation 1988

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
/*
 * $XConsortium: Xos.h,v 1.10 88/09/06 14:30:21 jim Exp $
 * 
 * Copyright 1987 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 */

/* This is a collection of things to try and minimize system dependencies
 * in a "signficant" number of source files.
 */

#ifndef _XOS_H_
#define _XOS_H_

#if defined(_AIX) && !defined(AIX)
#define AIX
#endif

/* make sure that if a SystemV derivitive is used that SYSV is defined */
#ifdef AIX
#ifndef SYSV
#define SYSV	1
#endif /* SYSV */
#endif /* AIX */
#ifdef hpux
#ifndef SYSV
#define SYSV	1
#endif /* SYSV */
#endif /* hpux */


/*
 * Get major data types (esp. caddr_t)
 */

#ifdef CRAY
#ifndef __TYPES__
#define __TYPES__
#include <sys/types.h>			/* forgot to protect it... */
#endif /* __TYPES__ */
#else
#include <sys/types.h>
#endif /* CRAY */


/*
 * Just about everyone needs the strings routines.  For uniformity, we use
 * the BSD-style index() and rindex() in application code, so any systems that
 * don't provide them need to have #defines here.  Unfortunately, we can't
 * use #if defined() here since makedepend will get confused.
 *
 * The list of systems that currently needs System V stings includes:
 *
 *	hpux
 * 	macII
 *	CRAY
 */

#ifdef SYSV
#define SYSV_STRINGS
#endif /* SYSV */

#ifdef macII				/* since it doesn't define SYSV */
#define SYSV_STRINGS
#endif /* macII */

#ifdef SYSV_STRINGS
#include <string.h>
#define index strchr
#define rindex strrchr
#undef SYSV_STRINGS
#else
#include <strings.h>
#endif /* SYSV_STRINGS */


/*
 * Get open(2) constants
 */

#ifdef SYSV
#include <fcntl.h>
#endif /* SYSV */
#include <sys/file.h>


/*
 * Get struct timeval
 */

#ifdef macII
#include <time.h>		/* need this as well as sys/time.h */
#endif /* macII */

#ifdef SYSV
#ifdef	AIX
#include <sys/time.h>
#else
#include <time.h>
#endif
# ifdef mips
# include <bsd/sys/time.h>
# endif /* mips */
#else /* else not SYSV */
#include <sys/time.h>
#endif /* SYSV */

/*
 * More BSDisms
 */

#ifdef SYSV
#define SIGCHLD SIGCLD
#endif /* SYSV */

/* include path for syslog.h BSD vs SYSV */
#ifdef SYSV
#ifdef hpux
#include <syslog.h>
#else
#ifdef	AIX
#include <sys/syslog.h>
#else
#include <bsd/sys/syslog.h>
#endif
#endif
#else /* SYSV */
#include <syslog.h>
#endif /* SYSV */

/* VMUNIX vs. SY_B4x */
#if SY_B4x
#ifndef VMUNIX
#define	VMUNIX	1
#endif /* VMUNIX */
#endif /* SY_B4x */

/* getdtablesize() and an errno which does not seem to be defined for SYSV */
#ifdef SYSV
#define EDOM				33
#define getdtablesize()			_NFILE
#define setpriority(which,who,prio) (nice((prio)-nice(0)))
#endif /* SYSV */


/*
 * handle (BSD) flock vs. (SYSV) lockf through defines
  * osi == Operating System Independent
  * locking with blocking is not enabled in AFS - so not defined below
 */
#ifdef SYSV
#ifdef hpux
#include <unistd.h>
#else
#include <sys/lockf.h>
#endif
#define osi_ExclusiveLockNoBlock(fid)	lockf((fid), F_TLOCK, 0)
#define osi_UnLock(fid)			lockf((fid), F_ULOCK, 0)
#else /* SYSV */
#define osi_ExclusiveLockNoBlock(fid)	flock((fid), LOCK_EX | LOCK_NB)
#define osi_UnLock(fid)			flock((fid), LOCK_UN)
#endif /* SYSV */

/* handle (BSD) vfork for (AIX) which only knows fork */
#ifdef AIX
#define	osi_vfork()			fork()
#else /* AIX */
#define	osi_vfork()			vfork()
#endif

/*
 * Put system-specific definitions here
 */

#ifdef hpux
#define sigvec sigvector
#endif /* hpux */

#ifdef mips
# ifdef SYSTYPE_SYSV
# include <bsd/sys/ioctl.h>
# include <bsd/sys/file.h>
# endif /* SYSTYPE_SYSV */
#endif /* mips */


#endif /* _XOS_H_ */
