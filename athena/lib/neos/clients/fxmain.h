/**********************************************************************
 * File Exchange fxmain header
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxmain.h,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/fxmain.h,v 1.1 1991-06-30 16:25:05 probe Exp $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#include <mit-copyright.h>

#ifndef _fxmain_h

#include <fxcl.h>

#define VERBOSE 1
#define LISTONLY 2
#define PRESERVE 4

#define ERR_USAGE -1L

extern char fxmain_error_context[];

#ifdef __STDC__
long fxmain(int, char *[], char *, Paper *,
	    int (*)(int, char *[], int *, Paper *, int *),
	    long (*)(FX *, Paper *, int, char *));
#else /* __STDC__ */
long fxmain();
#endif /* __STDC__ */

#endif /* _fxmain_h */
