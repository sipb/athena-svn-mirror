/*
 * This file is stolen from the Athena OLH system
 * It contains the definition of strcasecmp, which isn't present on the
 * *^&)(&* broken PS/2's.
 *
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/strcasecmp.c,v $
 * $Id: strcasecmp.c,v 1.1 1993-10-12 03:04:16 probe Exp $
 * $Author: probe $
 *
 */

#if defined(_I386) && defined(_BSD)
#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/lib/strcasecmp.c,v 1.1 1993-10-12 03:04:16 probe Exp $";
#endif
#endif

#include <stdio.h>

int strcasecmp(str1,str2)
     char *str1,*str2;
{
  int a;

  if (str1 == NULL) {
    if (str2 == NULL)
      return(0);
    else
      return(-1);
  }
  if (str2 == NULL)
    return(1);
  while ((*str1 != '\0') && (*str2 != '\0')) {
    a = (*str1++ - *str2++);
    if ((a != 0) && (a != 'A' - 'a') && (a != 'a' -'A')) return(a);
  }
  if (*str1 == *str2) return(0);
  if (*str1 ==  '\0') return(-1); else return(1);
}

#endif /* defined(_I386) && defined(_BSD) */
