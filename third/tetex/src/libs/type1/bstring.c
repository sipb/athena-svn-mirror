/*
 * A simple memset() in case your ANSI C does not provide it
 */

#include "c-auto.h"

#ifndef HAVE_MEMSET
memset(void *s, int c, int length)
{  char *p = s;
  
   while (length--) *(p++) = c;
}
#endif
