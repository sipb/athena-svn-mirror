/*
 * Copyright 2000, International Business Machines Corporation and others.
 * All Rights Reserved.
 * 
 * This software has been released under the terms of the IBM Public
 * License.  For details, see the LICENSE file in the top-level source
 * directory or online at http://www.openafs.org/dl/license10.html
 */

#include <afsconfig.h>
#include "../afs/param.h"

RCSID("$Header: /afs/dev.mit.edu/source/repository/third/openafs/src/afs/LINUX/osi_sleep.c,v 1.1.1.3 2004-02-13 17:52:10 zacheiss Exp $");

#include "../afs/sysincludes.h"	/* Standard vendor system headers */
#include "../afs/afsincludes.h"	/* Afs-based standard headers */
#include "../afs/afs_stats.h"   /* afs statistics */



static int osi_TimedSleep(char *event, afs_int32 ams, int aintok);
void afs_osi_Wakeup(char *event);
void afs_osi_Sleep(char *event);

static char waitV, dummyV;

void afs_osi_InitWaitHandle(struct afs_osi_WaitHandle *achandle)
{
    AFS_STATCNT(osi_InitWaitHandle);
    achandle->proc = (caddr_t) 0;
}

/* cancel osi_Wait */
void afs_osi_CancelWait(struct afs_osi_WaitHandle *achandle)
{
    caddr_t proc;

    AFS_STATCNT(osi_CancelWait);
    proc = achandle->proc;
    if (proc == 0) return;
    achandle->proc = (caddr_t) 0;   /* so dude can figure out he was signalled */
    afs_osi_Wakeup(&waitV);
}

/* afs_osi_Wait
 * Waits for data on ahandle, or ams ms later.  ahandle may be null.
 * Returns 0 if timeout and EINTR if signalled.
 */
int afs_osi_Wait(afs_int32 ams, struct afs_osi_WaitHandle *ahandle, int aintok)
{
    int code;
    afs_int32 endTime, tid;
    struct timer_list *timer = NULL;

    AFS_STATCNT(osi_Wait);
    endTime = osi_Time() + (ams/1000);
    if (ahandle)
	ahandle->proc = (caddr_t) current;

    do {
        AFS_ASSERT_GLOCK();
        code = 0;
        code = osi_TimedSleep(&waitV, ams, aintok);

        if (code) break;
	if (ahandle && (ahandle->proc == (caddr_t) 0)) {
	    /* we've been signalled */
	    break;
	}
    } while (osi_Time() < endTime);
    return code;
}




typedef struct afs_event {
    struct afs_event *next;	/* next in hash chain */
    char *event;		/* lwp event: an address */
    int refcount;		/* Is it in use? */
    int seq;			/* Sequence number: this is incremented
				   by wakeup calls; wait will not return until
				   it changes */
#if defined(AFS_LINUX24_ENV)
    wait_queue_head_t cond;
#else
    struct wait_queue *cond;
#endif
} afs_event_t;

#define HASHSIZE 128
afs_event_t *afs_evhasht[HASHSIZE];/* Hash table for events */
#define afs_evhash(event)	(afs_uint32) ((((long)event)>>2) & (HASHSIZE-1));
int afs_evhashcnt = 0;

/* Get and initialize event structure corresponding to lwp event (i.e. address)
 * */
static afs_event_t *afs_getevent(char *event)
{
    afs_event_t *evp, *newp = 0;
    int hashcode;

    AFS_ASSERT_GLOCK();
    hashcode = afs_evhash(event);
    evp = afs_evhasht[hashcode];
    while (evp) {
	if (evp->event == event) {
	    evp->refcount++;
	    return evp;
	}
	if (evp->refcount == 0)
	    newp = evp;
	evp = evp->next;
    }
    if (!newp)
	return NULL;

    newp->event = event;
    newp->refcount = 1;
    return newp;
}

/* afs_addevent -- allocates a new event for the address.  It isn't returned;
 *     instead, afs_getevent should be called again.  Thus, the real effect of
 *     this routine is to add another event to the hash bucket for this
 *     address.
 *
 * Locks:
 *     Called with GLOCK held. However the function might drop
 *     GLOCK when it calls osi_AllocSmallSpace for allocating
 *     a new event (In Linux, the allocator drops GLOCK to avoid
 *     a deadlock).
 */

static void afs_addevent(char *event)
{
    int hashcode;
    afs_event_t *newp;
    
    AFS_ASSERT_GLOCK();
    hashcode = afs_evhash(event);
    newp = osi_linux_alloc(sizeof(afs_event_t), 0);
    afs_evhashcnt++;
    newp->next = afs_evhasht[hashcode];
    afs_evhasht[hashcode] = newp;
#if defined(AFS_LINUX24_ENV)
    init_waitqueue_head(&newp->cond);
#else
    init_waitqueue(&newp->cond);
#endif
    newp->seq = 0;
    newp->event = &dummyV;  /* Dummy address for new events */
    newp->refcount = 0;
}

#ifndef set_current_state
#define set_current_state(x)            current->state = (x);
#endif

/* Release the specified event */
#define relevent(evp) ((evp)->refcount--)

/* afs_osi_Sleep -- waits for an event to be notified, ignoring signals.
 * - NOTE: that on Linux, there are circumstances in which TASK_INTERRUPTIBLE
 *   can wake up, even if all signals are blocked
 * - TODO: handle signals correctly by passing an indication back to the
 *   caller that the wait has been interrupted and the stack should be cleaned
 *   up preparatory to signal delivery
 */
void afs_osi_Sleep(char *event)
{
    struct afs_event *evp;
    int seq;
#ifdef DECLARE_WAITQUEUE
    DECLARE_WAITQUEUE(wait, current);
#else
    struct wait_queue wait = { current, NULL };
#endif

    evp = afs_getevent(event);
    if (!evp) {
        afs_addevent(event);
	evp = afs_getevent(event);
    }

    seq = evp->seq;

    add_wait_queue(&evp->cond, &wait);
    while (seq == evp->seq) {
	sigset_t saved_set;

	set_current_state(TASK_INTERRUPTIBLE);
	AFS_ASSERT_GLOCK();
	AFS_GUNLOCK();

	SIG_LOCK(current);
	saved_set = current->blocked;
	sigfillset(&current->blocked);
	RECALC_SIGPENDING(current);
	SIG_UNLOCK(current);

	schedule();

	SIG_LOCK(current);
	current->blocked = saved_set;
	RECALC_SIGPENDING(current);
	SIG_UNLOCK(current);
	AFS_GLOCK();
    }
    remove_wait_queue(&evp->cond, &wait);
    set_current_state(TASK_RUNNING);

    relevent(evp);
}

/* osi_TimedSleep
 * 
 * Arguments:
 * event - event to sleep on
 * ams --- max sleep time in milliseconds
 * aintok - 1 if should sleep interruptibly
 *
 * Returns 0 if timeout, EINTR if signalled, and EGAIN if it might
 * have raced.
 */
static int osi_TimedSleep(char *event, afs_int32 ams, int aintok)
{
    int code = 0;
    long ticks = (ams * HZ / 1000) + 1;
    struct afs_event *evp;
#ifdef DECLARE_WAITQUEUE
    DECLARE_WAITQUEUE(wait, current);
#else
    struct wait_queue wait = { current, NULL };
#endif

    evp = afs_getevent(event);
    if (!evp) {
        afs_addevent(event);
	evp = afs_getevent(event);
    }

    add_wait_queue(&evp->cond, &wait);
    set_current_state(TASK_INTERRUPTIBLE);
    /* always sleep TASK_INTERRUPTIBLE to keep load average
       from artifically increasing. */

    AFS_GUNLOCK();
    if (aintok) {
        if (schedule_timeout(ticks))
            code = EINTR;
    } else
        schedule_timeout(ticks);
    AFS_GLOCK();

    remove_wait_queue(&evp->cond, &wait);
    set_current_state(TASK_RUNNING);

    relevent(evp);

    return code;
}


void afs_osi_Wakeup(char *event)
{
    struct afs_event *evp;

    evp = afs_getevent(event);
    if (!evp)                          /* No sleepers */
	return;

    if (evp->refcount > 1) {
	evp->seq++;    
	wake_up(&evp->cond);
    }
    relevent(evp);
}