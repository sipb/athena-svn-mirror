/* $Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_63/include/lwp.h,v 1.1.2.1 1997-11-04 18:46:05 ghudson Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_63/include/lwp.h,v $ */

#ifndef __LWP_INCLUDE_
#define	__LWP_INCLUDE_	1
#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidlwp = "$Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_63/include/lwp.h,v 1.1.2.1 1997-11-04 18:46:05 ghudson Exp $";
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
\*******************************************************************/

#include <afs/param.h>

#define LWP_SUCCESS	0
#define LWP_EBADPID	-1
#define LWP_EBLOCKED	-2
#define LWP_EINIT	-3
#define LWP_EMAXPROC	-4
#define LWP_ENOBLOCK	-5
#define LWP_ENOMEM	-6
#define LWP_ENOPROCESS	-7
#define LWP_ENOWAIT	-8
#define LWP_EBADCOUNT	-9
#define LWP_EBADEVENT	-10
#define LWP_EBADPRI	-11
#define LWP_NO_STACK	-12
/* These two are for the signal mechanism. */
#define LWP_EBADSIG	-13		/* bad signal number */
#define LWP_ESYSTEM	-14		/* system call failed */
/* These are for the rock mechanism */
#define LWP_ENOROCKS	-15	/* all rocks are in use */
#define LWP_EBADROCK	-16	/* the specified rock does not exist */

#if	defined(USE_PTHREADS) || defined(USE_SOLARIS_THREADS)
#ifdef	USE_SOLARIS_THREADS
#include <thread.h>
typedef int pthread_t;
typedef void *pthread_addr_t;
typedef void *pthread_condattr_t;
typedef void (*pthread_destructor_t)(void *);
typedef pthread_addr_t (*pthread_startroutine_t)(pthread_addr_t);
#define	pthread_mutex_lock	mutex_lock
#define	pthread_mutex_unlock	mutex_unlock
#define	pthread_getspecific	thr_getspecific
#define	pthread_setspecific	thr_setspecific
#define	pthread_yield		thr_yield
/*typedef mutex_t pthread_mutex_t;*/
typedef thread_key_t pthread_key_t;
typedef cond_t   PTHREAD_COND, *pthread_cond_t;

#define PTHREAD_DEFAULT_SCHED 1
#define SCHED_FIFO  2
#define MUTEX_FAST_NP              0
#define PTHREAD_DEFAULT_STACK           65536 /* 64 K */
#define PRI_OTHER_MIN   1
#define PRI_OTHER_MAX   127
#define PRI_OTHER_MID   ((PRI_OTHER_MIN + PRI_OTHER_MAX)/2)
#define DESTRUCTOR_TAB_CHUNK 20
#define maskAllSignals(o) thr_sigsetmask(SIG_BLOCK,&pthread_allSignals,&o)
#define restoreAllSignals(o) thr_sigsetmask(SIG_SETMASK,&o,NULL)

typedef struct PTHREAD_MUTEX {
    int                 kind;
    pthread_t           ownerId;
    mutex_t             fastMutex;
    int                 timesInside;
} PTHREAD_MUTEX, *pthread_mutex_t;


typedef struct PTHREAD_ATTR {
    long                stackSize;
    int                 prio;
} PTHREAD_ATTR, *pthread_attr_t;

typedef struct {
    void                (*destructor)(void*);
    thread_key_t          key;
} dest_slot_t;

typedef struct {
    int            nentries;  /* size allocated (in terms of number of slots) */
    int            next_free; /* next free slot */
    dest_slot_t    slot[1];
} pthread_destructor_tab_t;
define DTAB_SIZE(size) (sizeof(pthread_destructor_tab_t) + (size)*sizeof(dest_slot_t))

#else
#include "pthread.h"
#endif
#include <assert.h>

#define LWP_MAX_PRIORITY	0
#define LWP_NORMAL_PRIORITY	0
#define LWP_NO_PRIORITIES
/*
 * We define PROCESS as a pointer to this struct, rather than simply as
 * a pthread_t  since some applications test for a process handle being
 * non-zero. This can't be done on a pthread_t.
 */
typedef struct lwp_process {
    pthread_t handle;			/* The pthreads handle */
    struct lwp_process *next;		/* Next LWP process */
    char *name;				/* LWP name of the process */
    pthread_startroutine_t ep;		/* Process entry point */
    pthread_addr_t arg;			/* Initial parameter */
} * PROCESS;

struct rock
    {/* to hide things associated with this LWP under */
    int  tag;		/* unique identifier for this rock */
    char *value;	/* pointer to some arbitrary data structure */
    };

#define MAXROCKS	4	/* max no. of rocks per LWP */

#define DEBUGF 		0

#ifndef BDE_THREADS
/*#define CMA_DEBUG 1*/
#endif

#ifdef CMA_DEBUG
#define LWP_CHECKSTUFF(msg)	lwp_checkstuff(msg)
#else
#define LWP_CHECKSTUFF(msg)
#endif

#if DEBUGF
#define debugf(m) printf m
#else
#define debugf(m)
#endif

#define IOMGR_Poll() LWP_DispatchProcess()

/*
 * These two macros can be used to enter/exit the LWP context in a CMA
 * program. They simply acquire/release the global LWP mutex .
 */
extern pthread_mutex_t lwp_mutex;
#define LWP_EXIT_LWP_CONTEXT() pthread_mutex_unlock(&lwp_mutex)
#define LWP_ENTER_LWP_CONTEXT() pthread_mutex_lock(&lwp_mutex)
#else
/* Maximum priority permissible (minimum is always 0) */
#define LWP_MAX_PRIORITY 4	/* changed from 1 by Satya, 22 Nov. 85 */

/* Usual priority used by user LWPs */
#define LWP_NORMAL_PRIORITY (LWP_MAX_PRIORITY-2)

/* Initial size of eventlist in a PCB; grows dynamically  */ 
#define EVINITSIZE  5

typedef struct lwp_pcb *PROCESS;

struct lwp_context {	/* saved context for dispatcher */
    char *topstack;	/* ptr to top of process stack */
#ifdef sparc
#ifdef	save_allregs
    int globals[7+1+32+2+32+2];    /* g1-g7, y reg, f0-f31, fsr, fq, c0-c31, csr, cq. */
#else
    int globals[8];    /* g1-g7 and y registers. */
#endif
#endif
};

struct rock
    {/* to hide things associated with this LWP under */
    int  tag;		/* unique identifier for this rock */
    char *value;	/* pointer to some arbitrary data structure */
    };

#define MAXROCKS	4	/* max no. of rocks per LWP */

struct lwp_pcb {			/* process control block */
  char		name[32];		/* ASCII name */
  int		rc;			/* most recent return code */
  char		status;			/* status flags */
  char		blockflag;		/* if (blockflag), process blocked */
  char		eventlistsize;		/* size of eventlist array */
  char          padding;                /* force 32-bit alignment */
  char		**eventlist;		/* ptr to array of eventids */
  int		eventcnt;		/* no. of events currently in eventlist array*/
  int		wakevent;		/* index of eventid causing wakeup */
  int		waitcnt;		/* min number of events awaited */
  int		priority;		/* dispatching priority */
  struct lwp_pcb *misc;			/* for LWP internal use only */
  char		*stack;			/* ptr to process stack */
  int		stacksize;		/* size of stack */
  int		stackcheck;		/* first word of stack for overflow checking */
  int		(*ep)();		/* initial entry point */
  char		*parm;			/* initial parm for process */
  struct lwp_context
		context;		/* saved context for next dispatch */
  int		rused;			/* no of rocks presently in use */
  struct rock	rlist[MAXROCKS];	/* set of rocks to hide things under */
  struct lwp_pcb *next, *prev;		/* ptrs to next and previous pcb */
  int		level;			/* nesting level of critical sections */
  struct IoRequest	*iomgrRequest;	/* request we're waiting for */
  int           index;                  /* LWP index: should be small index; actually is
                                           incremented on each lwp_create_process */ 
  };

extern int lwp_nextindex;                      /* Next lwp index to assign */


#ifndef LWP_KERNEL
#define LWP_ActiveProcess	(lwp_cpptr+0)
#define LWP_Index() (LWP_ActiveProcess->index)
#define LWP_HighestIndex() (lwp_nextindex - 1)
#ifndef	AFS_SUN5_ENV	/* Actual functions for solaris */
#define LWP_SignalProcess(event)	LWP_INTERNALSIGNAL(event, 1)
#define LWP_NoYieldSignal(event)	LWP_INTERNALSIGNAL(event, 0)
#endif

extern
#endif
  struct lwp_pcb *lwp_cpptr;	/* pointer to current process pcb */

struct	 lwp_ctl {			/* LWP control structure */
    int		processcnt;		/* number of lightweight processes */
    char	*outersp;		/* outermost stack pointer */
    struct lwp_pcb *outerpid;		/* process carved by Initialize */
    struct lwp_pcb *first, last;	/* ptrs to first and last pcbs */
#ifdef __hp9000s800
    double	dsptchstack[100];	/* stack for dispatcher use only */
					/* force 8 byte alignment        */
#else
    char	dsptchstack[800];	/* stack for dispatcher use only */
#endif
};

#ifndef LWP_KERNEL
extern
#endif
       char lwp_debug;		/* ON = show LWP debugging trace */

/* 
 * Under hpux, any stack size smaller than 16K seems prone to
 * overflow problems.
 */
#if defined(AFS_HPUX_ENV) || defined(AFS_NEXT_ENV) || defined(AFS_AIX_ENV) || defined(AFS_OSF_ENV)
#define AFS_LWP_MINSTACKSIZE	(24 * 1024)
#else
#define AFS_LWP_MINSTACKSIZE	(16 * 1024)
#endif /* defined(AFS_HPUX_ENV) */

/* Action to take on stack overflow. */
#define LWP_SOQUIET	1		/* do nothing */
#define LWP_SOABORT	2		/* abort the program */
#define LWP_SOMESSAGE	3		/* print a message and be quiet */
extern int lwp_overflowAction;

/* Tells if stack size counting is enabled. */
extern int lwp_stackUseEnabled;
extern int lwp_MaxStackSeen;
#endif	/* USE_PTHREADS */
#ifndef	AFS_AIX32_ENV
#define	LWP_CreateProcess2(a, b, c, d, e, f)	LWP_CreateProcess((a), (b), (c), (d), (e), (f))
#endif
#endif /* __LWP_INCLUDE_ */
