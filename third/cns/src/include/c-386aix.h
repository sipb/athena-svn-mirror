/*
 * Copyright 1991 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: IBM AIX on 386 PC hardware.
 */

#include "mit-copyright.h"

#define BITS32
#define BIG
#define LSBFIRST
#define	HOST_BYTE_ORDER	LSB_FIRST
#define BSDUNIX         /* kerberos really uses this to test conditions which
                           have little to do with BSD vs. not BSD */

typedef int sigtype;	/* Signal handler functions are declared "int".  */

