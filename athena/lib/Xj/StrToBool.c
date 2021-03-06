/*
 * $Id: StrToBool.c,v 1.2 1999-01-22 23:16:58 ghudson Exp $
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Id: StrToBool.c,v 1.2 1999-01-22 23:16:58 ghudson Exp $";
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
