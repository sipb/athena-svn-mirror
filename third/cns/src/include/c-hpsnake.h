/*
 * c-hpsnake.h
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: HP "snake" (HP 9000/series 700, hpux 8)
 */
 
#include "mit-copyright.h"
 
#define BITS32
#define BIG
#define MSBFIRST
#define	HOST_BYTE_ORDER	MSB_FIRST
#define BSDUNIX
#define MUSTALIGN

typedef void sigtype;	/* Signal handler functions are declared "void".  */

/* Used in appl/bsd/kcmd.c */
#define	HIDE_RUSEROK	1

/* Used in appl/bsd/login.c, kuser/ksu.c */
#define	NO_SETPRIORITY	1

/* Used in lib/des/random_key.h, lib/kstream/kstream_des.c */
#define random	lrand48
#define srandom	srand48
