/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: SPARC with BSD Unix, e.g. SUN-4
 */

#include "mit-copyright.h"

#define BITS32
#define BIG
#define MSBFIRST
#define	HOST_BYTE_ORDER	MSB_FIRST
#define BSDUNIX
#define MUSTALIGN
#define DES_SHIFT_SHIFT
typedef void sigtype;	/* Signal handler functions are declared "void".  */
#define	BSD42	1
#define	ATHENA	1
#define	HAVE_VSPRINTF	1
#define	HAS_STRDUP	1
#define	UIDGID_T	1
#define	VARARGS	1
#define	NEED_SYS_ERRLIST	1
#define	HOST_BYTE_ORDER	MSB_FIRST
#define	RPROGS_IN_USR_UCB	1
#define	MOVE_WITH_BCOPY	1

/* Used in various places, appl/bsd, email/POP, kadmin, lib/krb */
#define	USE_UNISTD_H	1
