/*
 * c-ibm370.h
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: IBM 370
 */

#include "mit-copyright.h"

/* What else? */
#define BIG
#define NONASCII
#define SHORTNAMES
#define	MSBFIRST
#define	HOST_BYTE_ORDER	MSB_FIRST

typedef int sigtype;	/* Signal handler functions are declared "int".  */
