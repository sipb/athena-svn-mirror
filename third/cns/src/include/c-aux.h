/*
 * c-aux.h
 *
 * Copyright 1991 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: MAC A/UX 2.0
 */

#include "mit-copyright.h"

#define MAC 
#define BITS32
#define BIG
#define MSBFIRST
#define BSDUNIX
#define SYSV_TERMIO

#ifndef __STDC__
#ifndef NOASM
#define AUX_ASM
#endif /* no assembly */
#endif /* standard C */

typedef void sigtype;	/* Signal handler functions are declared "void".  */

/* Used in appl/bsd/login.c and kuser/ksu.c. */
#define	NO_SETPRIORITY	1

/*
 * These are not defined for at least SunOS 3.3, Ultrix 2.2, and A/UX 2.0
 */
#if defined(_AUX_SOURCE)
#define FD_ZERO(p)	((p)->fds_bits[0] = 0)
#define FD_SET(n, p)	((p)->fds_bits[0] |= (1 << (n)))
#define FD_ISSET(n, p)	((p)->fds_bits[0] & (1 << (n)))
#endif

/* These are used in lib/des/random_key.c.  */
#ifdef _AUX_SOURCE
#include <time.h>
#define random	lrand48
#define srandom	srand48
#endif

