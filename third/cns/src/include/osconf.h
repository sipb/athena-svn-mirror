/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Kerberos OS-dependent configuration.
 *
 * When adding new config files, call them c-*.h if they are a configuration
 * for a specific environment, or cc-*.h if they are included in multiple
 * environments (e.g. they configure features or feature sets, not whole
 * environments).
 */

#ifndef	OSCONF_H_INCLUDED
#define	OSCONF_H_INCLUDED

#include "mit-copyright.h"

#ifdef sgi
#include "c-sgi.h"
#include "cc-unix.h"
#define KRBTYPE sgi
#endif

#ifdef tahoe
#include "c-tahoe.h"
#include "cc-unix.h"
#define KRBTYPE bsdtahoe
#endif

#ifdef vax
#include "c-vax.h"
#include "cc-unix.h"
#define KRBTYPE bsdvax
#endif

#if defined(mips) && defined(ultrix)
#include "c-ultmips.h"
#include "cc-unix.h"
#define KRBTYPE ultmips2
#endif /* !Ultrix MIPS-2 */

#ifdef __pyrsoft
#ifdef MIPSEB
#include "c-pyramid.h"
#include "cc-unix.h"
#define KRBTYPE pyramid
#endif
#endif

#ifdef __DGUX__
#ifdef __m88k__
#include "c-dgux88.h"
#include "cc-unix.h"
#define KRBTYPE pyramid
#endif
#endif

#ifdef ibm032
#include "c-rtpc.h"
#include "cc-unix.h"
#define KRBTYPE bsdibm032
#endif /* !ibm032 */

#ifdef apollo
#include "c-apollo.h"
#include "cc-unix.h"
#define KRBTYPE bsdapollo
#endif /* !apollo */

#ifdef sun
#ifdef sparc
#ifdef __svr4__
#include "c-sol2.h"
#include "cc-unix.h"
#define KRBTYPE sol20sparc
#else /* sun, sparc, not solaris */
#include "c-sunos4.h"
#include "cc-unix.h"
#define KRBTYPE bsdsparc
#endif /* sun, sparc */
#else /* sun but not sparc */
#ifdef i386
#include "c-sun386i.h"
#include "cc-unix.h"
#define KRBTYPE bsd386i
#else /* sun but not (sparc or 386i) */
#include "c-sun3.h"
#include "cc-unix.h"
#define KRBTYPE bsdm68k
#endif /* i386 */
#endif /* sparc */
#endif /* !sun */

#ifdef pyr
#include "c-pyr.h"
#include "cc-unix.h"
#define KRBTYPE pyr
#endif

#ifdef _AUX_SOURCE
#include "c-aux.h"
#include "cc-unix.h"
#define KRBTYPE aux
#endif

#ifdef _AIX
#ifdef _IBMR2
#include "c-rs6000.h"
#include "cc-unix.h"
#define KRBTYPE aixrios
#endif /* risc/6000 */
#ifdef i386
#include "c-386aix.h"
#include "cc-unix.h"
#define KRBTYPE aixps2
#endif
#endif

#ifdef hpux
#ifdef hppa
#include "c-hpsnake.h"
#include "cc-unix.h"
#define KRBTYPE hpsnake
#endif
#ifdef hp9000s300
#include "c-hp68k.h"
#include "cc-unix.h"
#define KRBTYPE hp68k
#endif
#endif

#ifdef NeXT
#include "c-next.h"
#include "cc-unix.h"
#define KRBTYPE next
#endif

#ifdef __SCO__
#include "c-386sco.h"
#include "cc-unix.h"
#define KRBTYPE sco386
#endif

#ifdef linux
#include "c-386linux.h"
#include "cc-unix.h"
#define KRBTYPE linux386
#endif

#ifdef __386BSD__
#include "c-386bsd.h"
#include "cc-unix.h"
#define KRBTYPE 386bsd
#endif

#ifdef __NetBSD__
#ifdef i386
#ifndef __386BSD__
/* close enough, I think */
#include "c-386bsd.h"
#include "cc-unix.h"
#define KRBTYPE 386bsd
#endif
#endif
#endif

#ifdef __i960__
#include "c-i960vx.h"
#include "cc-unix.h"
#define KRBTYPE i960vx
#endif

#ifdef __alpha
#include "c-alpha.h"
#include "cc-unix.h"
#define KRBTYPE alpha
#endif

#ifdef _WINDOWS
#include "c-windows.h"
#define	KRBTYPE	windows
#endif

#ifdef	macintosh
#include "c-mac.h"
#define	KRBTYPE	mac
#endif

#ifndef KRBTYPE
#ifdef __svr4__
#include "c-svr4.h"
#include "cc-unix.h"
#define KRBTYPE svr4
#endif
#endif

#ifndef KRBTYPE
/*#*/ error "configuration failed to recognize system type"
#endif

/* Define some macros for signal handling.  If USE_SIGPROCMASK is
   defined, we use the POSIX functions.  Otherwise we use the BSD
   functions.

   sigmasktype: type of mask argument to following macros.
   SIGBLOCK(MASK, SIG): block SIG, setting MASK to the old signal mask
   SIGBLOCK2(MASK, SIG1, SIG2): Like SIGBLOCK, but block both SIG1 and SIG2
   SIGBLOCK3(MASK, SIG1, SIG2, SIG3): Same again, but three signals.
   SIGUNBLOCK(MASK, SIG): unblock SIG, setting MASK to the old signal mask
   SIGSETMASK(MASK): set the signal mask to MASK

   These macros evalaute MASK multiple times.  */

#ifndef USE_SIGPROCMASK
#define sigmasktype int
#define SIGBLOCK(MASK, SIG) (MASK = sigblock (sigmask (SIG)))
#define SIGBLOCK2(MASK, SIG1, SIG2) \
  (MASK = sigblock (sigmask (SIG1) | sigmask (SIG2)))
#define SIGBLOCK3(MASK, SIG1, SIG2, SIG3) \
  (MASK = sigblock (sigmask (SIG1) | sigmask (SIG2) | sigmask (SIG3)))
#define SIGUNBLOCK(MASK, SIG) \
  (MASK = sigblock (0), sigsetmask (MASK &~ sigmask (SIG)))
#define SIGSETMASK(MASK) sigsetmask (MASK)
#else
#define sigmasktype sigset_t
#define SIGBLOCK(MASK, SIG) \
  (sigemptyset (&(MASK)), \
   sigaddset (&(MASK), (SIG)), \
   sigprocmask (SIG_BLOCK, &(MASK), &(MASK)))
#define SIGBLOCK2(MASK, SIG1, SIG2) \
  (sigemptyset (&(MASK)), \
   sigaddset (&(MASK), (SIG1)), \
   sigaddset (&(MASK), (SIG2)), \
   sigprocmask (SIG_BLOCK, &(MASK), &(MASK)))
#define SIGBLOCK3(MASK, SIG1, SIG2, SIG3) \
  (sigemptyset (&(MASK)), \
   sigaddset (&(MASK), (SIG1)), \
   sigaddset (&(MASK), (SIG2)), \
   sigaddset (&(MASK), (SIG3)), \
   sigprocmask (SIG_BLOCK, &(MASK), &(MASK)))
#define SIGUNBLOCK(MASK, SIG) \
  (sigemptyset (&(MASK)), \
   sigaddset (&(MASK), (SIG)), \
   sigprocmask (SIG_UNBLOCK, &(MASK), &(MASK)))
#define SIGSETMASK(MASK) sigprocmask (SIG_SETMASK, &(MASK), NULL)
#endif

#endif /* OSCONF_H_INCLUDED */
