/**********************************************************************
 * File Exchange fxmain header
 *
 * $Id: fxmain.h,v 1.1 1999-09-28 22:10:58 danw Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef _fxmain_h

#include <fx/fxcl.h>

#define VERBOSE 1
#define LISTONLY 2
#define PRESERVE 4
#define ONE_AUTHOR 8

#define ERR_USAGE -1L

extern char fxmain_error_context[];

long fxmain(int, char *[], char *, Paper *,
	    int (*)(int, char *[], int *, Paper *, int *),
	    long (*)(FX *, Paper *, int, char *));

#endif /* _fxmain_h */
