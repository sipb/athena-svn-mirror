/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/rx/RCS/rx_user.h,v 2.29 1995/10/23 13:09:47 ota Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/rx/RCS/rx_user.h,v $ */

/*
****************************************************************************
*        Copyright IBM Corporation 1988, 1989 - All Rights Reserved        *
*                                                                          *
* Permission to use, copy, modify, and distribute this software and its    *
* documentation for any purpose and without fee is hereby granted,         *
* provided that the above copyright notice appear in all copies and        *
* that both that copyright notice and this permission notice appear in     *
* supporting documentation, and that the name of IBM not be used in        *
* advertising or publicity pertaining to distribution of the software      *
* without specific, written prior permission.                              *
*                                                                          *
* IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL IBM *
* BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY      *
* DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER  *
* IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING   *
* OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.    *
****************************************************************************
*/

#ifdef	VALIDATE
error-foo error-foo error-foo
#endif /* VALIDATE */
#ifndef RX_USER_INCLUDE
#define RX_USER_INCLUDE

/* rx_user.h:  definitions specific to the user-level implementation of Rx */

#include <afs/param.h>
#include <stdio.h>
#include <lwp.h>

#ifdef RXDEBUG
extern FILE *rx_debugFile;
#endif

/* These routines are no-ops in the user level implementation */
#define SPLVAR
#define NETPRI
#define USERPRI

#define AFS_GLOCK()
#define AFS_GUNLOCK()
#define ISAFS_GLOCK()
#define AFS_ASSERT_GLOCK()

#define AFS_RXGLOCK()
#define AFS_RXGUNLOCK()
#define ISAFS_RXGLOCK()
#define AFS_ASSERT_RXGLOCK()

extern void rxi_StartListener();
extern void rxi_StartServerProc();
extern void rxi_ReScheduleEvents();

/* Some "operating-system independent" stuff, for the user mode implementation */
typedef short osi_socket;
#define	OSI_NULLSOCKET	((osi_socket) -1)

#define	osi_rxSleep(x)		    rxi_Sleep(x)
#define	osi_rxWakeup(x)		    rxi_Wakeup(x)

#ifndef	AFS_AIX32_ENV

#ifndef osi_Alloc
#define	osi_Alloc(size)		    ((char *) malloc(size))
#endif

#ifndef osi_Free
#define	osi_Free(ptr, size)	    free((char *)(ptr))
#endif

#endif

#define	osi_GetTime(timevalptr)	    gettimeofday(timevalptr, 0)

/* Just in case it's possible to distinguish between relatively long-lived stuff and stuff which will be freed very soon, but which needs quick allocation (e.g. dynamically allocated xdr things) */
#define	osi_QuickFree(ptr, size)    osi_Free(ptr, size)
#define	osi_QuickAlloc(size)	    osi_Alloc(size)

extern osi_Panic();

#if  !defined(_ANSI_C_SOURCE) || defined(AFS_NEXT20_ENV) || defined(AFS_SUN_ENV)
#ifdef AFS_NEXT20_ENV
extern int fprintf(FILE *stream, const char *format, ...);
#endif
#if defined(AFS_SUN_ENV) && !defined(AFS_SUN5_ENV)
extern int fprintf();
#endif
#endif	/* ANSI_C_SOURCE */

#define	osi_Msg			    fprintf)(stderr,

#endif /* RX_USER_INCLUDE */
