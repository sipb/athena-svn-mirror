#ifndef  PTHREAD
#define  PTHREAD
#include <thread.h>
#include <synch.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#define _POSIX_THREADS			1
#define _POSIX_THREAD_ATTR_STACKSIZE	1

typedef void 			 *pthread_addr_t;
typedef int			  pthread_t;
#define PTHREAD_NONE		(-1L)
typedef pthread_addr_t		(*pthread_startroutine_t)( pthread_addr_t);
typedef void			(*pthread_cleanup_t)( pthread_addr_t);

typedef pthread_addr_t		any_t;
typedef pthread_startroutine_t	pthread_func_t;

#define pthread_equal(a,b)	((a)==(b))
#define pthread_equal_np(a,b)	((a)==(b))

/*extern	pthread_t		pthread_self pthread_proto((void));*/

typedef struct PTHREAD_ATTR {
    long		stackSize;
    int			prio;
} PTHREAD_ATTR, *pthread_attr_t;

extern  pthread_attr_t			pthread_attr_default;
#define PTHREAD_DEFAULT_STACK		65536 /* 64 K */
#define PTHREAD_MIN_STACK		32768 /* 32 K */

#define PRI_OTHER_MIN	1
#define PRI_OTHER_MAX	127
#define PRI_OTHER_MID	((PRI_OTHER_MIN + PRI_OTHER_MAX)/2)

/*extern	void	 pthread_yield pthread_proto((void));*/

#define PTHREAD_INHERIT_SCHED 0
#define PTHREAD_DEFAULT_SCHED 1
#define SCHED_OTHER 0
#define SCHED_RR    1
#define SCHED_FIFO  2

#define PRI_FIFO_MIN 1
#define PRI_FIFO_MAX 100

/*
 * Mutexes
 */
#define MUTEX_FAST_NP			0
#define MUTEX_NONRECURSIVE_NP		1
#define MUTEX_RECURSIVE_NP		2
#define MUTEX_LAST_NP			2

typedef struct PTHREAD_MUTEX {
    int 		kind;
    pthread_t		ownerId;
    mutex_t		fastMutex;
    int			timesInside;
} PTHREAD_MUTEX, *pthread_mutex_t;

typedef void *pthread_mutexattr_t;
#define	pthread_mutexattr_default	((pthread_mutexattr_t) MUTEX_FAST_NP)

typedef cond_t	 PTHREAD_COND, *pthread_cond_t;
typedef void	*pthread_condattr_t;
#define pthread_condattr_default	((pthread_condattr_t)0)

#include <sys/time.h>

typedef struct PTHREAD_ONCE {
    int		onceInit;	/* must be first field */
    mutex_t	onceMutex;
} PTHREAD_ONCE, pthread_once_t;

#define pthread_once_init { 0 }
typedef void (*pthread_initroutine_t)(void);

typedef thread_key_t pthread_key_t;
typedef void (*pthread_destructor_t)(void *);

#define CANCEL_OFF	0
#define CANCEL_ON	1

#include "exc_handling.h"

#define pthread_cleanup_push(routine,arg)	\
    { \
    pthread_cleanup_t _pthread_XXX_proc = (pthread_cleanup_t)(routine); \
    pthread_addr_t _pthread_XXX_arg = (arg); \
    int _pthread_XXX_completed = 0; \
    TRY {

#define pthread_cleanup_pop(execute)	\
    _pthread_XXX_completed = 1;} \
    FINALLY { \
	int _pthread_XXX_execute = (execute); \
	if ((! _pthread_XXX_completed) || _pthread_XXX_execute) \
	    _pthread_XXX_proc (_pthread_XXX_arg);} \
    ENDTRY}

#ifdef	notdef
extern void atfork pthread_proto((/* IN */ void  *uState,
				  /* IN */ void (*preF)(void*),
				  /* IN */ void (*parentF)(void*),
				  /* IN */ void (*childF)(void*)));
#endif
#define fork pthread_Fork

#define sigprocmask thr_sigsetmask

extern  int	pthread_create(), pthread_join(), pthread_detach();
extern  void	pthread_exit(), pthread_abort();
extern	int	pthread_getunique_np();
extern  int	pthread_attr_create(), pthread_attr_delete(), pthread_attr_getstacksize();
extern	int	pthread_attr_setstacksize(), pthread_attr_getprio(), pthread_attr_setprio();
extern	int	pthread_attr_getsched();
extern	int	pthread_attr_setsched();
extern	int	pthread_attr_getinheritsched();
extern	int	pthread_attr_setinheritsched();
extern	int	pthread_getprio();
extern	int	pthread_setprio();
extern	int	pthread_getscheduler();
extern	int	pthread_setscheduler();
extern	int	pthread_mutex_init();
extern	int	pthread_mutex_destroy();
extern	int	pthread_mutex_lock();
extern	int	pthread_mutex_unlock();
extern	int	pthread_mutex_trylock();
extern	int	pthread_mutexattr_create();
extern	int	pthread_mutexattr_delete();
extern	int	pthread_mutexattr_getkind_np();
extern	int	pthread_mutexattr_setkind_np();
extern  int	pthread_cond_init();
extern	int	pthread_cond_destroy();
extern	int	pthread_cond_signal();
extern	int	pthread_cond_broadcast();
extern  int	pthread_cond_wait();
extern  int	pthread_cond_timedwait();
extern	int pthread_condattr_create();
extern	int pthread_condattr_delete();
extern  int	pthread_once();
extern	void	pthread_lock_global_np();
extern	void	pthread_unlock_global_np();
extern	int	pthread_get_expiration_np();
extern	int	pthread_delay_np();
extern	int	pthread_keycreate();
extern	int	pthread_setspecific();
extern	int	pthread_getspecific();
extern  int	pthread_cancel();
extern	void	pthread_testcancel();
extern	int	pthread_chlibsig_np();
extern	int	pthread_setcancel();
extern	int	pthread_setasynccancel();
extern	int	pthread_signal_to_cancel_np();

/*
#define CHT_NO_BDE
#define cht_boolean_t			int
#define cht_mutex_struct		mutex_t
#define cht_mutex_init(mP)		mutex_init(mP, USYNC_THREAD, 0)
#define cht_lock(mP)			mutex_lock(mP)
#define cht_unlock(mP)			mutex_unlock(mP)

typedef mutex_t *cht_mutex_t;
*/
#include "cht_hash.h"

#define HASH_TABLE_SIZE		128
#define GIVE_KEY(x)		((x)->myId)
#define HASH_KEY(x)		(x)
#define EQUAL_KEY(a,b)		pthread_equal(a,b)


/*----------------------------------------------------------------------*/

/*
 * Problem with programs calling pthread_self() from the destructor function of
 * the thr-spec data. The tcb is deleted when thr_exit() is invoked. Therefore,
 * the workaround is to wrap keycreate, registering "NULL" as the destructor
 * function with solaris, while calling the destructors ourselves before calling
 * thr_exit().
 *
 * Now, since there isnt any "keydestroy()" function, we can simply start
 * inserting the destructor entries in the table from slot 0 onwards. Since the
 * order in which the destructors are called is unspecified anyway, we call them
 * in the order the keys were created from myFinishRoutine
 */

typedef struct {
    void		(*destructor)(void*);
    thread_key_t	  key;
} dest_slot_t;

typedef struct {
    int		   nentries;  /* size allocated (in terms of number of slots) */
    int		   next_free; /* next free slot */
    dest_slot_t	   slot[1];
} pthread_destructor_tab_t;

/* incremental number of entries allocated each time */
#define DESTRUCTOR_TAB_CHUNK 20
#define DTAB_SIZE(size) (sizeof(pthread_destructor_tab_t) + (size)*sizeof(dest_slot_t))

typedef struct pthread_tcb_t {
    pthread_t			 myId;
    thread_t			 solarisId;
    int				 flags;
    int				 cancelAck;
    pthread_addr_t		 exitStatus;
    pthread_startroutine_t	 startRoutine;
    cond_t			 joinCond;
    unsigned long		 numJoiners;
    exc_context_t		*excStack;
    cht_link_t			 nextP;  /* for CHT hashing */
} pthread_tcb_t;

#define PTHREAD_STATE		0xff
#define PTHREAD_STATE_RUNNING	0x01
#define PTHREAD_STATE_ZOMBIE	0x02

#define PTHREAD_IS_DETACHED		0x100
#define PTHREAD_GENERAL_CANCEL_ENABLED	0x200
#define PTHREAD_ASYNC_CANCEL_ENABLED	0x400
#define PTHREAD_IS_CANCELLED		0x800
#define PTHREAD_IS_SUSPENDED		0x1000

/*
 * The idea is this:
 *
 * Async cancel should be turned off for a thread whenever it enters
 * the pthread library, and restored when it leaves. This has to be
 * done on a per-thread basis, meaning that the async-cancel flag is
 * in the thread's tcb. Note that the thread's signal mask cannot be
 * tampered with, since regular cancellation also uses the same signal.
 *
 * However, the tcb must first be retrieved to manipulate the flag.
 * Therefore, the solaris-id hash table always turns off the signal
 * when it is entered, and restores it when it leaves.
 */

#define STOP_ASYNCCANCEL(save,tcb)				\
	((save) = (tcb)->flags & PTHREAD_ASYNC_CANCEL_ENABLED,	\
	 (tcb)->flags &= ~PTHREAD_ASYNC_CANCEL_ENABLED)
#define RESTORE_ASYNCCANCEL(save,tcb)				\
  ((tcb)->flags |= ((save) & PTHREAD_ASYNC_CANCEL_ENABLED))

#define TESTCANCEL(tcb)						\
    MACRO_BEGIN							\
    if ((tcb->flags & PTHREAD_GENERAL_CANCEL_ENABLED) &&	\
	(tcb->flags & PTHREAD_IS_CANCELLED)) {			\
	tcb->flags &= ~PTHREAD_IS_CANCELLED;			\
	tcb->cancelAck += 1;					\
	RAISE( pthread_cancel_e);				\
    }								\
    MACRO_END

#define PTHREAD_CANCEL_STATUS ((pthread_addr_t)-1)

IMPORT	int	pthread_init_flag;
IMPORT	int	pthread_init(void);

#define PTHREAD_INIT(retval)					\
    MACRO_BEGIN							\
    if ( !pthread_init_flag && pthread_init() )			\
	return retval;						\
    MACRO_END


IMPORT	pthread_tcb_t	*pthread_preTestCancel(int *asave);
IMPORT	void		 pthread_postTestCancel(pthread_tcb_t *tcb, int asave);
IMPORT	pthread_tcb_t	*pthread_getSelfTcb( void);
IMPORT sigset_t pthread_cancelSigset;
IMPORT sigset_t pthread_allSignals;

#define disableCancelSignal(o) thr_sigsetmask(SIG_BLOCK,&pthread_cancelSigset,&o)
#define restoreCancelSignal(o) thr_sigsetmask(SIG_SETMASK,&o,NULL)

#define maskAllSignals(o) thr_sigsetmask(SIG_BLOCK,&pthread_allSignals,&o)
#define restoreAllSignals(o) thr_sigsetmask(SIG_SETMASK,&o,NULL)

#endif /*  PTHREAD */
