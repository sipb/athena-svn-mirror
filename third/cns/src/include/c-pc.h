/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: IBM PC 8086
 */

#include "mit-copyright.h"

#define IBMPC
#define BITS16
#define CROSSMSDOS
#define LSBFIRST
#define	HOST_BYTE_ORDER	LSB_FIRST

typedef void sigtype;	/* Signal handler functions are declared "void".  */
