/**********************************************************************
 * Macros to deal with allocation properly
 *
 * $Id: memory.h,v 1.1 1999-09-28 22:07:23 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#ifndef __MEMORY_MACROS__
#define __MEMORY_MACROS__

#define New(type) ((type *)malloc((unsigned)sizeof (type)))
#define NewArray(type, n) ((type *)malloc((unsigned)sizeof(type) * (n)))
#define BiggerArray(type,old,n) ((type *)realloc((char *)(old), \
						 (unsigned)(sizeof(type)*(n))))
#define newstring(s) malloc((unsigned)strlen(s)+1)

#endif /* __MEMORY_MACROS__ */


