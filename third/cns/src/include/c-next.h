/*
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

#define	HAVE_GETENV
