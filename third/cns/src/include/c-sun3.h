/*
 * c-sun3.h
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: 68000 with BSD Unix, e.g. SUN
 */

#include "mit-copyright.h"

#define BITS32
#define BIG
#define MSBFIRST
#define	HOST_BYTE_ORDER	MSB_FIRST
#define BSDUNIX

typedef void sigtype;	/* Signal handler functions are declared "void".  */

/* Used in various places, appl/bsd, email/POP, kadmin, lib/krb */
#define	USE_UNISTD_H	1

/*
 * These are not defined for at least SunOS 3.3, Ultrix 2.2, and A/UX 2.0
 */
#if (defined(SunOS) && SunOS < 40)
#define FD_ZERO(p)	((p)->fds_bits[0] = 0)
#define FD_SET(n, p)	((p)->fds_bits[0] |= (1 << (n)))
#define FD_ISSET(n, p)	((p)->fds_bits[0] & (1 << (n)))
#endif
