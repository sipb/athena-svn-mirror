/*
 * c-alpha.h
 *
 * Copyright 1988, 1993 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: DEC Alpha AXP
 */

#include "mit-copyright.h"

#define KRB_INT32	int

#define BITS32
#define BITS64
#define BIG
#define LSBFIRST
#define	HOST_BYTE_ORDER	LSB_FIRST
#define BSDUNIX
#define MUSTALIGN
#define DONT_DECLARE_TIME
typedef void sigtype;	/* Signal handler functions are declared "void".  */

/* Used in various places, appl/bsd, email/POP, kadmin, lib/krb */
#define	USE_UNISTD_H	1
