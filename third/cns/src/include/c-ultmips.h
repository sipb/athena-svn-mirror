/*
 * c-ultmips.h
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: DECstation 3100 (MIPS R2000)
 */
 
#include "mit-copyright.h"
 
#define MIPS2
#define BITS32
#define BIG
#define LSBFIRST
#define	HOST_BYTE_ORDER	LSB_FIRST
#define BSDUNIX
#define MUSTALIGN
typedef void sigtype;	/* Signal handler functions are declared "void".  */

/*
 * These are not defined for at least SunOS 3.3, Ultrix 2.2, and A/UX 2.0
 */
#if defined(ULTRIX022)
#define FD_ZERO(p)	((p)->fds_bits[0] = 0)
#define FD_SET(n, p)	((p)->fds_bits[0] |= (1 << (n)))
#define FD_ISSET(n, p)	((p)->fds_bits[0] & (1 << (n)))
#endif

