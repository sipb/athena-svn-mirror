/**********************************************************************
 * Macros to deal with allocation properly
 *
 * $Author: probe $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/memory.h,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/memory.h,v 1.1 1992-12-07 13:29:42 probe Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#ifndef lint
static char rcsid_memory_h[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/memory.h,v 1.1 1992-12-07 13:29:42 probe Exp $";
#endif lint

#ifndef __MEMORY_MACROS__
#define __MEMORY_MACROS__

char *malloc();
char *realloc();

#define New(type) ((type *)malloc((unsigned)sizeof (type)))
#define NewArray(type, n) ((type *)malloc((unsigned)sizeof(type) * (n)))
#define BiggerArray(type,old,n) ((type *)realloc((char *)(old), \
						 (unsigned)(sizeof(type)*(n))))
#define newstring(s) malloc((unsigned)strlen(s)+1)

#endif /* __MEMORY_MACROS__ */


