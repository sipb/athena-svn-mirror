#ifndef _RX_MACHDEP_
#define _RX_MACHDEP_

/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/rx/RCS/rx_machdep.h,v 1.51 1996/04/17 17:28:00 cdc Exp $ */
/*
****************************************************************************
*        Copyright IBM Corporation 1988, 1989 - All Rights Reserved        *
*        Copyright Transarc Corporation 1993  - All Rights Reserved        *
*                                                                          *
* Permission to use, copy, modify, and distribute this software and its    *
* documentation for any purpose and without fee is hereby granted,         *
* provided that the above copyright notice appear in all copies and        *
* that both that copyright notice and this permission notice appear in     *
* supporting documentation, and that neither the name of IBM nor the name  *
* of Transarc be used in advertising or publicity pertaining to            *
* distribution of the software without specific, written prior permission. *
*                                                                          *
* IBM AND TRANSARC DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,   *
* INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO   *
* EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL     *
* DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR    *
* PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS  *
* ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF   *
* THIS SOFTWARE.                                                           *
****************************************************************************
*/

#include <afs/param.h>
#if defined(AFS_SGIMP_ENV)
#include "sys/sema.h"
#ifndef mutex_tryenter
#define mutex_tryenter(m) cpsema(m)
#endif /* nutex_tryenter */
#endif


/* Include the following to use the lock data base. */
/* #define RX_LOCKS_DB 1 */
/* The lock database uses a file id number and the line number to identify
 * where in the code a lock was obtained. Each file containing locks
 * has a separate file id called: rxdb_fileID.
 */
#define RXDB_FILE_RX        1	/* rx.c */
#define RXDB_FILE_RX_EVENT  2	/* rx_event.c */
#define RXDB_FILE_RX_PACKET 3	/* rx_packet.c */
#define RXDB_FILE_RX_RDWR   4	/* rx_rdwr.c */

#ifdef AFS_GLOBAL_RXLOCK_KERNEL
#undef AFS_GLOBAL_RXLOCK_KERNEL
#endif /* AFS_GLOBAL_RXLOCK_KERNEL */

#if	defined(AFS_AIX41_ENV) && defined(KERNEL)
#undef	LOCK_INIT
#include <sys/lockl.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/sleep.h>
#define	RX_ENABLE_LOCKS		1
#define AFS_GLOBAL_RXLOCK_KERNEL
/*
 * `condition variables' -- well, not really.  these just map to the
 * AIX event-list routines.  Thus, if one signals a condition prior to
 * a process waiting for it, the signal gets lost.
 * Note:
 *	`t' in `cv_timedwait' has different interpretation
 */
#ifndef	CV_DEFAULT
#define	CV_DEFAULT	0
#endif
#ifndef	MUTEX_DEFAULT
#define	MUTEX_DEFAULT	0
#endif
#define CV_INIT(cv, a,b,c)	(*(cv) = EVENT_NULL)
#define CV_WAIT(_cv, _lck) 	e_sleep_thread((_cv), (_lck), LOCK_SIMPLE)
#define CV_TIMEDWAIT(cv,lck,t)	e_mpsleep((cv), (t), (lck), LOCK_SIMPLE)
#define CV_SIGNAL(_cv)		e_wakeup_one(_cv)
#define CV_BROADCAST(_cv)	e_wakeup(_cv)
typedef simple_lock_data kmutex_t;
typedef int kcondvar_t;
#define	osi_rxWakeup(cv)	e_wakeup(cv)
#endif
#if	defined(AFS_SUN5_ENV) && defined(KERNEL) 

#if            defined(AFS_FINEGR_SUNLOCK)
#define	RX_ENABLE_LOCKS	1
#else
#undef	RX_ENABLE_LOCKS
#endif

#include <sys/tiuser.h>
#ifndef	AFS_NCR_ENV
#include <sys/t_lock.h>
#include <sys/mutex.h>
#endif

#endif	/* SUN5 && KERNEL */

#if defined(AFS_SGI53_ENV) && defined(MP) && defined(KERNEL)
#define	RX_ENABLE_LOCKS		1
#ifndef	CV_DEFAULT
#define	CV_DEFAULT	0
#endif
#ifndef	MUTEX_DEFAULT
#define	MUTEX_DEFAULT	0
#endif
#endif /* SGI53 && KERNEL && MP */

#ifndef	AFS_AOS_ENV
#define	ADAPT_PERF
#ifndef	AFS_SUN5_ENV
#define MISCMTU
#define ADAPT_MTU
#endif
#if defined(AFS_SUN5_ENV) && !defined(KERNEL)
#define MISCMTU
#define ADAPT_MTU
#include <sys/sockio.h>
#include <sys/fcntl.h>
#endif
#endif

#if	defined(AFS_AIX32_ENV) && defined(KERNEL)
#define PIN(a, b) pin(a, b);
#define UNPIN(a, b) unpin(a, b);
#else 
#define PIN(a, b) ;
#define UNPIN(a, b) ;
#endif

#if	defined(AFS_GLOBAL_SUNLOCK) && defined(KERNEL)
#ifndef AFS_GLOBAL_RXLOCK_KERNEL
#define	AFS_GLOBAL_RXLOCK_KERNEL	1
#endif
#endif

/* Conditionally re-defined below */
#define	MUTEX_ISMINE(a)	(1)
#define	osirx_AssertMine(a, msg)

#ifdef RX_ENABLE_LOCKS

#ifdef RX_REFCOUNT_CHECK
/* RX_REFCOUNT_CHECK is used to test for call refcount leaks by event
 * type.
 */
int rx_callHoldType = 0;
#define CALL_HOLD(call, type) do { \
				 call->refCount++; \
				 call->refCDebug[type]++; \
				 if (call->refCDebug[type] > 50)  {\
				     rx_callHoldType = type; \
				     osi_Panic("Huge call refCount"); \
							       } \
			     } while (0)
#define CALL_RELE(call, type) do { \
				 call->refCount--; \
				 call->refCDebug[type]--; \
				 if (call->refCDebug[type] > 50) {\
				     rx_callHoldType = type; \
				     osi_Panic("Negative call refCount"); \
							      } \
			     } while (0)
#else /* RX_REFCOUNT_CHECK */
#define CALL_HOLD(call, type) 	 call->refCount++
#define CALL_RELE(call, type)	 call->refCount--
#endif  /* RX_REFCOUNT_CHECK */

/*
 * MUTEX primitives for different platforms
 */

#ifdef AFS_FINEGR_SUNLOCK
extern kmutex_t afs_termStateLock;
extern kcondvar_t afs_termStateCv;
#endif /* AFS_FINEGR_SUNLOCK */

#ifdef	AFS_AIX41_ENV

#define	LOCK_INIT(a, b)		lock_alloc((void*)(a), LOCK_ALLOC_PIN, 1, 1), \
				simple_lock_init((void *)(a))
#define MUTEX_INIT(a,b,c,d)	lock_alloc((void*)(a), LOCK_ALLOC_PIN, 1, 1), \
				simple_lock_init((void *)(a))
#define MUTEX_DESTROY(a)	lock_free((void*)(a))

#ifdef RX_LOCKS_DB

#undef CV_WAIT
#define CV_WAIT(_cv, _lck)	rxdb_droplock((void *)(_lck), thread_self(), \
					      rxdb_fileID, __LINE__), \
				e_sleep_thread((_cv), (_lck), LOCK_SIMPLE), \
				rxdb_grablock((void *)(_lck), thread_self(), \
					      rxdb_fileID, __LINE__)


#define MUTEX_ENTER(a)		simple_lock((void *)(a)), \
				rxdb_grablock((void *)(a), thread_self(),rxdb_fileID,\
					      __LINE__)

#define MUTEX_TRYENTER(a)	(simple_lock_try((void *)(a)) ?\
				rxdb_grablock((void *)(a), thread_self(), rxdb_fileID,\
					      __LINE__), 1 : 0)

#define MUTEX_EXIT(a)  		rxdb_droplock((void *)(a), thread_self(), rxdb_fileID,\
					      __LINE__), \
				simple_unlock((void *)(a))


#define RXObtainWriteLock(a)    simple_lock((void *)(a)), \
				rxdb_grablock((void *)(a), thread_self(),rxdb_fileID,\
					      __LINE__)
	
#define RXReleaseWriteLock(a)	rxdb_droplock((void *)(a), thread_self(), rxdb_fileID,\
					      __LINE__), \
				simple_unlock((void *)(a))

#else /* RX_LOCK_DB */

#define MUTEX_ENTER(a) 		simple_lock((void *)(a))
#define MUTEX_TRYENTER(a)	simple_lock_try((void *)(a))
#define MUTEX_EXIT(a) 		simple_unlock((void *)(a))
#define RXObtainWriteLock(a) 	simple_lock((void *)(a))
#define RXReleaseWriteLock(a)	simple_unlock((void *)(a))

#endif /* RX_LOCK_DB */

#define	MUTEX_DEFAULT	0

#undef MUTEX_ISMINE
#define MUTEX_ISMINE(a)	(lock_mine((void *)(a)))

#undef osirx_AssertMine
extern void osirx_AssertMine(void *lockaddr, char *msg);
#else	/* AIX41 */
#ifdef	AFS_OSF30_ENV

#define LOCK_INIT(a,b)		usimple_lock_init(a)
#define MUTEX_INIT(a,b,c,d)	usimple_lock_init(a)
#define MUTEX_DESTROY(a)	usimple_lock_terminate(a)
#define MUTEX_ENTER(a)		usimple_lock(a)
#define MUTEX_TRYENTER(a)	usimple_lock_try(a)
#define MUTEX_EXIT(a)		usimple_unlock(a)

#undef MUTEX_ISMINE
#define MUTEX_ISMINE(a)		simple_lock_holder(a)

#undef osirx_AssertMine
extern void osirx_AssertMine(void *lockaddr, char *msg);

#else	/* OSF30 */
#if defined(AFS_SGI53_ENV) && defined(MP) && defined(KERNEL)
#define MUTEX_INIT(a,b,c,d)  mutex_init(a,b,c,d)
#define MUTEX_DESTROY(a) mutex_destroy(a)
#undef MUTEX_ISMINE
#define MUTEX_ISMINE(a)	1
#define CV_INIT(cv, a,b,c)	cv_init(cv, a, b, c)
#define CV_SIGNAL(_cv)		cv_signal(_cv)
#define CV_BROADCAST(_cv)	cv_broadcast(_cv)
#define CV_DESTROY(_cv)		cv_destroy(_cv)
#undef osirx_AssertMine
extern void osirx_AssertMine(void *lockaddr, char *msg);
#ifdef RX_LOCKS_DB
#define MUTEX_ENTER(a)		do { \
				     mutex_enter(a); \
				     rxdb_grablock((a), osi_ThreadUnique(), \
						   rxdb_fileID, __LINE__); \
				 } while(0)
#define MUTEX_TRYENTER(a)	(mutex_tryenter(a) ? \
				     (rxdb_grablock((a), osi_ThreadUnique(), \
						   rxdb_fileID, __LINE__), 1) \
						   : 0)
#define MUTEX_EXIT(a)  		do { \
				     rxdb_droplock((a), osi_ThreadUnique(), \
						   rxdb_fileID, __LINE__); \
				     mutex_exit(a); \
				 } while(0)
#define CV_WAIT(_cv, _lck) 	do { \
				     AFS_ASSERT_GLOCK(); \
				     AFS_GUNLOCK(); \
				     rxdb_droplock((_lck), \
						   osi_ThreadUnique(), \
						   rxdb_fileID, __LINE__); \
				     cv_wait(_cv, _lck); \
				     rxdb_grablock((_lck), \
						   osi_ThreadUnique(), \
						   rxdb_fileID, __LINE__); \
				     MUTEX_EXIT(_lck); \
				     AFS_GLOCK(); \
				     MUTEX_ENTER(_lck); \
				 } while (0)
#define CV_TIMEDWAIT(cv,lck,t)	do { \
				     AFS_ASSERT_GLOCK(); \
				     AFS_GUNLOCK(); \
				     rxdb_droplock((_lck), \
						   osi_ThreadUnique(), \
						   rxdb_fileID, __LINE__); \
				     cv_timedwait(_cv, _lock, t); \
				     rxdb_grablock((_lck), \
						   osi_ThreadUnique(), \
						   rxdb_fileID, __LINE__); \
				     MUTEX_EXIT(_lck); \
				     AFS_GLOCK(); \
				     MUTEX_ENTER(_lck); \
				 } while (0)
#else /* RX_LOCKS_DB */
#define MUTEX_ENTER(a) mutex_enter(a)
#define MUTEX_TRYENTER(a) mutex_tryenter(a)
#define MUTEX_EXIT(a)  mutex_exit(a)
#define CV_WAIT(_cv, _lck) 	do { \
					AFS_ASSERT_GLOCK(); \
					AFS_GUNLOCK(); \
					cv_wait(_cv, _lck); \
					MUTEX_EXIT(_lck); \
					AFS_GLOCK(); \
					MUTEX_ENTER(_lck); \
				    } while (0)
#define CV_TIMEDWAIT(cv,lck,t)	do { \
					AFS_ASSERT_GLOCK(); \
					AFS_GUNLOCK(); \
					cv_timedwait(_cv, _lock, t); \
					MUTEX_EXIT(_lck); \
					AFS_GLOCK(); \
					MUTEX_ENTER(_lck); \
				    } while (0)
#endif /* RX_LOCKS_DB */
#else /* AFS_SGI53_ENV */
/* SunOS 5 version, but disabled at present (12/21/95) -- BWL */
#define MUTEX_DESTROY(a) mutex_destroy(a)
#define MUTEX_ENTER(a) mutex_enter(a)
#define MUTEX_TRYENTER(a) mutex_tryenter(a)
#define MUTEX_EXIT(a)  mutex_exit(a)
#define MUTEX_INIT(a,b,c,d)  mutex_init(a,b,c,d)

#undef MUTEX_ISMINE
#define MUTEX_ISMINE(a)	(mutex_owned((kmutex_t *)(a)))

#undef osirx_AssertMine
extern void osirx_AssertMine(void *lockaddr, char *msg);
#endif /* AFS_SGI53_ENV */
#endif	/* AFS_OSF30_ENV */
#endif	/* AFS_AIX41_ENV */

#else /* RX_ENABLE_LOCKS etc. */

#define CALL_HOLD(call, type)
#define CALL_RELE(call, type)
#define RXObtainWriteLock(a) AFS_ASSERT_RXGLOCK()
#define RXReleaseWriteLock(a)

#define MUTEX_DESTROY(a)
#define MUTEX_ENTER(a)
#define MUTEX_TRYENTER(a) 1
#define MUTEX_EXIT(a)  
#define MUTEX_INIT(a,b,c,d) 
#define CV_INIT(a,b,c,d)
#endif /* RX_ENABLE_LOCKS etc. */

#if !defined(AFS_AIX32_ENV) && !defined(AFS_OSF_ENV)
#define IFADDR2SA(f) (&((f)->ifa_addr))
#else /* AFS_AIX32_ENV */
#define IFADDR2SA(f) ((f)->ifa_addr)
#endif


#endif /* _RX_MACHDEP_ */
