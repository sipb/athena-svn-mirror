/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: VAX
 */

#include "mit-copyright.h"

#ifndef VAX
#define VAX
#endif
#define BITS32
#define BIG
#define LSBFIRST
#define	HOST_BYTE_ORDER	LSB_FIRST
#define BSDUNIX

#ifndef __STDC__
#ifndef NOASM
#define VAXASM
#endif /* no assembly */
#endif /* standard C */

typedef void sigtype;	/* Signal handler functions are declared "void".  */
