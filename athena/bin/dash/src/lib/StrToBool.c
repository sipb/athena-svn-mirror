/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToBool.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToBool.c,v 1.1 1993-07-02 00:04:52 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"

/*
 * String to Boolean conversion
 */
int StrToBoolean(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  if (strcasecmp(address, "true") == 0 ||
      strcasecmp(address, "yes") == 0 ||
      strcasecmp(address, "on") == 0)
    {
      *((int *)((char *)where + resource->resource_offset)) = True;
      return 0;
    }

  if (strcasecmp(address, "false") == 0 ||
      strcasecmp(address, "no") == 0 ||
      strcasecmp(address, "off") == 0)
    {
      *((int *)((char *)where + resource->resource_offset)) = False;
      return 0;
    }

  *((int *)((char *)where + resource->resource_offset)) = atoi(address);
  return 0;
}
