/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToOrient.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToOrient.c,v 1.1 1993-07-02 00:03:51 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"

/*
 * String to Orientation conversion
 */
int StrToOrientation(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  if (strcasecmp(address, XjVertical) == 0)
    {
      *((int *)((char *)where + resource->resource_offset)) = Vertical;
      return 0;
    }

  if (strcasecmp(address, XjHorizontal) == 0)
    {
      *((int *)((char *)where + resource->resource_offset)) = Horizontal;
      return 0;
    }

  *((int *)((char *)where + resource->resource_offset)) = Vertical;

  {
    char errtext[100];
    sprintf(errtext, "bad orientation value: %s", address);
    XjWarning(errtext);
  }
  return 0;
}
