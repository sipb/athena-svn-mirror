/**********************************************************************
 * Macros to deal with allocation properly
 *
 * $Author: vrt $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/memory.h,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/memory.h,v 1.3 1993-04-27 17:17:51 vrt Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#ifndef sun /* gcc barfs on this. */
#ifndef lint
static char rcsid_memory_h[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/include/memory.h,v 1.3 1993-04-27 17:17:51 vrt Exp $";
#endif
#endif /* SOLARIS */
#ifndef __MEMORY_MACROS__
#define __MEMORY_MACROS__

#define New(type) ((type *)malloc((unsigned)sizeof (type)))
#define NewArray(type, n) ((type *)malloc((unsigned)sizeof(type) * (n)))
#define BiggerArray(type,old,n) ((type *)realloc((char *)(old), \
						 (unsigned)(sizeof(type)*(n))))
#define newstring(s) malloc((unsigned)strlen(s)+1)

#endif /* __MEMORY_MACROS__ */


