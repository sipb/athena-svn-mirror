/*
 * c-hp68k.h
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: HP 68000 with BSD-like Unix
 */

#include "mit-copyright.h"

#define BITS32
#define BIG
#define MSBFIRST
#define	HOST_BYTE_ORDER	MSB_FIRST
/* needed for lib/des/new_rnd_key.c */
#define BSDUNIX

typedef void sigtype;	/* Signal handler functions are declared "void".  */

/* Used in appl/bsd/kcmd.c */
#define	HIDE_RUSEROK	1

/* Used in appl/bsd/login.c and kuser/ksu.c. */
#define	NO_SETPRIORITY	1

/* Used in lib/des/random_key.h, lib/kstream/kstream_des.c */
#define random	lrand48
#define srandom	srand48
