/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToJust.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/StrToJust.c,v 1.1 1993-07-02 00:04:08 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"

/*
 * String to Justify conversion
 */
int StrToJustify(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  if (strcasecmp(address, XjLeftJustify) == 0  ||
      strcasecmp(address, XjTopJustify) == 0)
    {
      *((int *)((char *)where + resource->resource_offset)) = Left;
      return 0;
    }

  if (strcasecmp(address, XjCenterJustify) == 0)
    {
      *((int *)((char *)where + resource->resource_offset)) = Center;
      return 0;
    }

  if (strcasecmp(address, XjRightJustify) == 0  ||
      strcasecmp(address, XjBottomJustify) == 0)
    {
      *((int *)((char *)where + resource->resource_offset)) = Right;
      return 0;
    }

  *((int *)((char *)where + resource->resource_offset)) = Center;

  {
    char errtext[100];
    sprintf(errtext, "bad justify value: %s", address);
    XjWarning(errtext);
  }
  return 0;
}
