/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/lwp/RCS/lock.h,v 2.3 1992/11/06 19:04:15 bob Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/lwp/RCS/lock.h,v $ */

#ifndef LOCK_H
#define LOCK_H

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidlock = "$Header: /afs/transarc.com/project/fs/dev/afs/rcs/lwp/RCS/lock.h,v 2.3 1992/11/06 19:04:15 bob Exp $";
#endif

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

/*******************************************************************\
* 								    *
* 	Information Technology Center				    *
* 	Carnegie-Mellon University				    *
* 								    *
* 								    *
* 								    *
\*******************************************************************/

/*
	Include file for using Vice locking routines.
*/

/* The following macros allow multi statement macros to be defined safely, i.e.
   - the multi statement macro can be the object of an if statement;
   - the call to the multi statement macro may be legally followed by a semi-colon.
   BEGINMAC and ENDMAC have been tested with both the portable C compiler and
   Hi-C.  Both compilers were from the Palo Alto 4.2BSD software releases, and
   both optimized out the constant loop code.  For an example of the use
   of BEGINMAC and ENDMAC, see the definition for ReleaseWriteLock, below.
   An alternative to this, using "if(1)" for BEGINMAC is not used because it
   may generate worse code with pcc, and may generate warning messages with hi-C.
*/

#define BEGINMAC do {
#define ENDMAC   } while (0)

/* all locks wait on excl_locked except for READ_LOCK, which waits on readers_reading */
struct Lock {
    unsigned char	wait_states;	/* type of lockers waiting */
    unsigned char	excl_locked;	/* anyone have boosted, shared or write lock? */
    unsigned char	readers_reading;	/* # readers actually with read locks */
    unsigned char	num_waiting;	/* probably need this soon */
};

#define READ_LOCK	1
#define WRITE_LOCK	2
#define SHARED_LOCK	4
/* this next is not a flag, but rather a parameter to Lock_Obtain */
#define BOOSTED_LOCK 6

/* next defines wait_states for which we wait on excl_locked */
#define EXCL_LOCKS (WRITE_LOCK|SHARED_LOCK)

#define ObtainReadLock(lock)\
	if (!((lock)->excl_locked & WRITE_LOCK) && !(lock)->wait_states)\
	    (lock) -> readers_reading++;\
	else\
	    Lock_Obtain(lock, READ_LOCK)

#define ObtainWriteLock(lock)\
	if (!(lock)->excl_locked && !(lock)->readers_reading)\
	    (lock) -> excl_locked = WRITE_LOCK;\
	else\
	    Lock_Obtain(lock, WRITE_LOCK)

#define ObtainSharedLock(lock)\
	if (!(lock)->excl_locked && !(lock)->wait_states)\
	    (lock) -> excl_locked = SHARED_LOCK;\
	else\
	    Lock_Obtain(lock, SHARED_LOCK)

#define BoostSharedLock(lock)\
	if (!(lock)->readers_reading)\
	    (lock)->excl_locked = WRITE_LOCK;\
	else\
	    Lock_Obtain(lock, BOOSTED_LOCK)

/* this must only be called with a WRITE or boosted SHARED lock! */
#define UnboostSharedLock(lock)\
	BEGINMAC\
	    (lock)->excl_locked = SHARED_LOCK; \
	    if((lock)->wait_states) \
		Lock_ReleaseR(lock); \
	ENDMAC

#ifdef notdef
/* this is what UnboostSharedLock looked like before the hi-C compiler */
/* this must only be called with a WRITE or boosted SHARED lock! */
#define UnboostSharedLock(lock)\
	((lock)->excl_locked = SHARED_LOCK,\
	((lock)->wait_states ?\
		Lock_ReleaseR(lock) : 0))
#endif /* notdef */

#define ReleaseReadLock(lock)\
	BEGINMAC\
	    if (!--(lock)->readers_reading && (lock)->wait_states)\
		Lock_ReleaseW(lock) ; \
	ENDMAC


#ifdef notdef
/* This is what the previous definition should be, but the hi-C compiler generates
  a warning for each invocation */
#define ReleaseReadLock(lock)\
	(!--(lock)->readers_reading && (lock)->wait_states ?\
		Lock_ReleaseW(lock)    :\
		0)
#endif /* notdef */

#define ReleaseWriteLock(lock)\
        BEGINMAC\
	    (lock)->excl_locked &= ~WRITE_LOCK;\
	    if ((lock)->wait_states) Lock_ReleaseR(lock);\
        ENDMAC

#ifdef notdef
/* This is what the previous definition should be, but the hi-C compiler generates
   a warning for each invocation */
#define ReleaseWriteLock(lock)\
	((lock)->excl_locked &= ~WRITE_LOCK,\
	((lock)->wait_states ?\
		Lock_ReleaseR(lock) : 0))
#endif /* notdef */

/* can be used on shared or boosted (write) locks */
#define ReleaseSharedLock(lock)\
        BEGINMAC\
	    (lock)->excl_locked &= ~(SHARED_LOCK | WRITE_LOCK);\
	    if ((lock)->wait_states) Lock_ReleaseR(lock);\
        ENDMAC

#ifdef notdef
/* This is what the previous definition should be, but the hi-C compiler generates
   a warning for each invocation */
/* can be used on shared or boosted (write) locks */
#define ReleaseSharedLock(lock)\
	((lock)->excl_locked &= ~(SHARED_LOCK | WRITE_LOCK),\
	((lock)->wait_states ?\
		Lock_ReleaseR(lock) : 0))
#endif /* notdef */
	

/* I added this next macro to make sure it is safe to nuke a lock -- Mike K. */
#define LockWaiters(lock)\
	((int) ((lock)->num_waiting))

#define CheckLock(lock)\
	((lock)->excl_locked? (int) -1 : (int) (lock)->readers_reading)

#define WriteLocked(lock)\
	((lock)->excl_locked & WRITE_LOCK)

#endif /* LOCK_H */
