/* taken directly from pth_p.h from GNU Pth.
 *
 * pth_* has been changed to cothreads_* so that we don't pollute the namespace
 */

#include <cothreads/config-public.h>


/*
 * machine context state structure
 *
 * In `jb' the CPU registers, the program counter, the stack
 * pointer and (usually) the signals mask is stored. When the
 * signal mask cannot be implicitly stored in `jb', it's
 * alternatively stored explicitly in `sigs'. The `error' stores
 * the value of `errno'.
 */

#if COTHREADS_MCTX_MTH(mcsc)
#include <ucontext.h>
#elif COTHREADS_MCTX_MTH(sjlj)
#include <signal.h>
#include <setjmp.h>
#endif

typedef struct cothreads_mctx_st cothreads_mctx_t;
struct cothreads_mctx_st {
#if COTHREADS_MCTX_MTH(mcsc)
    ucontext_t uc;
#elif COTHREADS_MCTX_MTH(sjlj)
    cothreads_sigjmpbuf jb;
#else
#error "unknown mctx method"
#endif
    sigset_t sigs;
#if COTHREADS_MCTX_DSP(sjlje)
    sigset_t block;
#endif
    int error;
};

/*
** ____ MACHINE STATE SWITCHING ______________________________________
*/

/*
 * save the current machine context
 */
#if COTHREADS_MCTX_MTH(mcsc)
#define cothreads_mctx_save(mctx) \
        ( (mctx)->error = errno, \
          getcontext(&(mctx)->uc) )
#elif COTHREADS_MCTX_MTH(sjlj) && COTHREADS_MCTX_DSP(sjlje)
#define cothreads_mctx_save(mctx) \
        ( (mctx)->error = errno, \
          sigprocmask(SIG_SETMASK, &((mctx)->block), NULL), \
          cothreads_sigsetjmp((mctx)->jb) )
#elif COTHREADS_MCTX_MTH(sjlj)
#define cothreads_mctx_save(mctx) \
        ( (mctx)->error = errno, \
          cothreads_sigsetjmp((mctx)->jb) )
#else
#error "unknown mctx method"
#endif

/*
 * restore the current machine context
 * (at the location of the old context)
 */
#if COTHREADS_MCTX_MTH(mcsc)
#define cothreads_mctx_restore(mctx) \
        ( errno = (mctx)->error, \
          (void)setcontext(&(mctx)->uc) )
#elif COTHREADS_MCTX_MTH(sjlj)
#define cothreads_mctx_restore(mctx) \
        ( errno = (mctx)->error, \
          (void)cothreads_siglongjmp((mctx)->jb, 1) )
#else
#error "unknown mctx method"
#endif

/*
 * restore the current machine context
 * (at the location of the new context)
 */
#if COTHREADS_MCTX_MTH(sjlj) && COTHREADS_MCTX_DSP(sjlje)
#define cothreads_mctx_restored(mctx) \
        sigprocmask(SIG_SETMASK, &((mctx)->sigs), NULL)
#else
#define cothreads_mctx_restored(mctx) \
        /*nop*/
#endif

/*
 * switch from one machine context to another
 */
#ifdef COTHREADS_DEBUG
#define  _cothreads_mctx_switch_debug(old,new) g_message ("switching from cothread %p to %p", (old), (new));
#else
#define  _cothreads_mctx_switch_debug(old,new) /*NOP*/
#endif
#if COTHREADS_MCTX_MTH(mcsc)
#define cothreads_mctx_switch(old,new) \
    _cothreads_mctx_switch_debug(old,new) \
    swapcontext(&((old)->uc), &((new)->uc));
#elif COTHREADS_MCTX_MTH(sjlj)
#define cothreads_mctx_switch(old,new) \
    _cothreads_mctx_switch_debug(old,new) \
    if (cothreads_mctx_save(old) == 0) \
        cothreads_mctx_restore(new); \
    cothreads_mctx_restored(old);
#else
#error "unknown mctx method"
#endif
