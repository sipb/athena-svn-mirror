/**********************************************************************
 * Macros to deal with allocation properly
 *
 * $Id: memory.h,v 1.4 1999-01-22 23:17:46 ghudson Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/

#ifndef sun /* gcc barfs on this. */
#ifndef lint
static char rcsid_memory_h[] = "$Id: memory.h,v 1.4 1999-01-22 23:17:46 ghudson Exp $";
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


